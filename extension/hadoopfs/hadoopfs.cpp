#include "hadoopfs.hpp"

#include "duckdb/common/atomic.hpp"
#include "duckdb/common/printer.hpp"
#include "duckdb/common/file_opener.hpp"
#include "duckdb/common/http_state.hpp"
#include "duckdb/common/thread.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/common/types/hash.hpp"
#include "duckdb/function/scalar/strftime_format.hpp"
#include "duckdb/function/scalar/string_functions.hpp"
#include "duckdb/main/client_context.hpp"
#include "duckdb/main/database.hpp"

#include <chrono>
#include <string>
#include <thread>

#include <map>

namespace duckdb {


    void HDFSEnvironmentSettingsProvider::SetExtensionOptionValue(string key, const char *env_var_name) {
        static char *evar;

        if ((evar = std::getenv(env_var_name)) != NULL) {
            if (StringUtil::Lower(evar) == "false") {
                this->config.SetOption(key, Value(false));
            } else if (StringUtil::Lower(evar) == "true") {
                this->config.SetOption(key, Value(true));
            } else {
                this->config.SetOption(key, Value(evar));
            }
        }
    }

    void HDFSEnvironmentSettingsProvider::SetAll() {
        this->SetExtensionOptionValue(HDFSParams::HDFS_DEFAULT_NAMENODE, this->HDFS_DEFAULT_NAMENODE);
        this->SetExtensionOptionValue(HDFSKerberosParams::HDFS_PRINCIPAL, this->HDFS_PRINCIPAL);
        this->SetExtensionOptionValue(HDFSKerberosParams::HDFS_KEYTAB_FILE, this->HDFS_KEYTAB_FILE);
    }

    HDFSParams HDFSParams::ReadFrom(DatabaseInstance &instance) {
        string default_namenode = "default";
        Value value;

        if (instance.TryGetCurrentSetting(HDFSParams::HDFS_DEFAULT_NAMENODE, value)) {
            default_namenode = value.ToString();
        }

        return {default_namenode};
    }

    HDFSParams HDFSParams::ReadFrom(FileOpener *opener, FileOpenerInfo &info) {
        string default_namenode = "default";
        Value value;

        if (FileOpener::TryGetCurrentSetting(opener, HDFSParams::HDFS_DEFAULT_NAMENODE, value, info)) {
            default_namenode = value.ToString();
        }

        return {default_namenode};
    }

    HDFSKerberosParams HDFSKerberosParams::ReadFrom(DatabaseInstance &instance) {
        string principal = "";
        string keytab_file = "";
        Value value;

        if (instance.TryGetCurrentSetting(HDFSKerberosParams::HDFS_PRINCIPAL, value)) {
            principal = value.ToString();
        }

        if (instance.TryGetCurrentSetting(HDFSKerberosParams::HDFS_KEYTAB_FILE, value)) {
            keytab_file = value.ToString();
        }

        return {principal, keytab_file};
    }

    HDFSKerberosParams HDFSKerberosParams::ReadFrom(FileOpener *opener, FileOpenerInfo &info) {
        string principal = "";
        string keytab_file = "";
        Value value;

        if (FileOpener::TryGetCurrentSetting(opener, HDFSKerberosParams::HDFS_PRINCIPAL, value, info)) {
            principal = value.ToString();
        }

        if (FileOpener::TryGetCurrentSetting(opener, HDFSKerberosParams::HDFS_KEYTAB_FILE, value, info)) {
            keytab_file = value.ToString();
        }

        return {principal, keytab_file};
    }

    HadoopFileHandle::HadoopFileHandle(FileSystem &fs, string path, uint8_t flags, hdfsFS hdfs)
            : FileHandle(fs, path), flags(flags), hdfs(hdfs), length(0), file_offset(0) {
    }

    void HadoopFileHandle::Initialize(FileOpener *opener) {

    }

    void HadoopFileHandle::Close() {
        if (hdfs_file) {
            hdfsCloseFile(hdfs, hdfs_file);
        }
        if (hdfs) {
            hdfsDisconnect(hdfs);
        }
    }

    void HadoopFileSystem::ParseUrl(const string &url, string &path_out, string &proto_host_port_out) {
        if (url.rfind("hdfs://", 0) != 0) {
            throw IOException("URL needs to start with hdfs://");
        }
        auto slash_pos = url.find('/', 8);
        if (slash_pos == string::npos) {
            throw IOException("URL needs to contain a '/' after the host");
        }
        proto_host_port_out = url.substr(0, slash_pos);

        path_out = url.substr(slash_pos);

        if (path_out.empty()) {
            throw IOException("URL needs to contain a path");
        }
    }

    static bool Match(vector<string>::const_iterator key, vector<string>::const_iterator key_end,
                      vector<string>::const_iterator pattern, vector<string>::const_iterator pattern_end) {

        while (key != key_end && pattern != pattern_end) {
            if (*pattern == "**") {
                if (std::next(pattern) == pattern_end) {
                    return true;
                }
                while (key != key_end) {
                    if (Match(key, key_end, std::next(pattern), pattern_end)) {
                        return true;
                    }
                    key++;
                }
                return false;
            }
            //Printer::PrintF("Match: %s, %d -- %s, %d",
            //                key->data(), key->length(), pattern->data(), pattern->length());
            if (!LikeFun::Glob(key->data(), key->length(), pattern->data(), pattern->length())) {
                return false;
            }
            key++;
            pattern++;
        }
        return key == key_end && pattern == pattern_end;
    }

    bool HadoopFileSystem::ListFiles(const string &directory,
                                     const std::function<void(const string &, bool)> &callback,
                                     FileOpener *opener) {
        //Printer::Print("ListFiles: " + directory);
        int num_entries;
        hdfsFileInfo *file_info = hdfsListDirectory(hdfs, directory.c_str(), &num_entries);
        if (file_info == nullptr) {
            return false;
        }

        for (int i = 0; i < num_entries; ++i) {
            //Printer::PrintF("File: %s, Kind: %d", file_info[i].mName, file_info[i].mKind);
            callback(file_info[i].mName, file_info[i].mKind == kObjectKindDirectory);
        }

        hdfsFreeFileInfo(file_info, num_entries);

        return true;
    }

    vector<string> HadoopFileSystem::Glob(const string &glob_pattern, FileOpener *opener) {
        if (opener == nullptr) {
            throw InternalException("Cannot HDFS Glob without FileOpener");
        }

        FileOpenerInfo info = {glob_pattern};

        // matches on prefix, not glob pattern, so we take a substring until the first wildcard char
        auto first_wildcard_pos = glob_pattern.find_first_of("*[\\");
        if (first_wildcard_pos == string::npos) {
            return {glob_pattern};
        }

        auto first_slash_pos = glob_pattern.find('/', 7);
        if (first_slash_pos == string::npos) {
            return {glob_pattern};
        }

        auto first_slash_before_wildcard = glob_pattern.rfind('/', first_wildcard_pos);
        if (first_slash_before_wildcard == string::npos) {
            return {glob_pattern};
        }

        string shared_path = glob_pattern.substr(0, first_slash_before_wildcard);
        string shared_pattern = glob_pattern.substr(first_slash_before_wildcard + 1);

        //Printer::Print("Shared path: " + shared_path);
        //Printer::Print("Shared pattern: " + shared_pattern);

        auto pattern_list = StringUtil::Split(shared_pattern, "/");
        vector<string> file_list;
        vector<string> path_list;
        path_list.push_back(shared_path);
        while (!path_list.empty()) {
            string current_path = path_list.back();
            path_list.pop_back();
            //Printer::Print("Current path: " + current_path);
            ListFiles(current_path, [&](const string &fname, bool is_directory) {
                auto match_path_list = StringUtil::Split(fname.substr(first_slash_before_wildcard + 1), "/");
                if (is_directory && Match(match_path_list.begin(), match_path_list.end(),
                                          pattern_list.begin(), pattern_list.begin() + match_path_list.size())) {
                    //Printer::Print("Push dir: " + fname);
                    path_list.push_back(fname);
                } else if (Match(match_path_list.begin(), match_path_list.end(), pattern_list.begin(),
                                 pattern_list.end())) {
                    //Printer::Print("Push file: " + fname);
                    file_list.push_back(fname.substr(first_slash_pos));
                }
            }, opener);
        }

        //for(duckdb::idx_t idx  = 0; idx < file_list.size(); idx++){
        //    Printer::PrintF("Glob %s: %s", glob_pattern, file_list[idx]);
        //}

        return file_list;
    }

    HadoopFileSystem::HadoopFileSystem(DatabaseInstance &instance) : instance(instance) {

        auto hdfs_param = HDFSParams::ReadFrom(instance);
        auto hdfs_kerberos_param = HDFSKerberosParams::ReadFrom(instance);

        auto hdfs_builder = hdfsNewBuilder();
        hdfsBuilderSetNameNode(hdfs_builder, hdfs_param.default_namenode.c_str());

        if (!hdfs_kerberos_param.principal.empty()) {
            hdfsBuilderSetUserName(hdfs_builder, hdfs_kerberos_param.principal.c_str());
        }

        if (!hdfs_kerberos_param.keytab_file.empty()) {
            hdfsBuilderSetKerbTicketCachePath(hdfs_builder, hdfs_kerberos_param.keytab_file.c_str());
        }

        hdfs = hdfsBuilderConnect(hdfs_builder);
        if (!hdfs) {
            throw IOException("Unable to connect to HDFS: " + hdfs_param.default_namenode);
        }

    }

    HadoopFileSystem::~HadoopFileSystem() {
        if (hdfs) {
            hdfsDisconnect(hdfs);
        }
    }

    unique_ptr<HadoopFileHandle> HadoopFileSystem::CreateHandle(const string &path, uint8_t flags, FileLockType lock,
                                                                FileCompressionType compression, FileOpener *opener) {
        FileOpenerInfo info = {path};
        auto hdfs_params = HDFSParams::ReadFrom(opener, info);
        auto hdfs_kerberos_params = HDFSKerberosParams::ReadFrom(opener, info);

        hdfsBuilder *builder = hdfsNewBuilder();
        hdfsBuilderSetNameNode(builder, path.c_str());
        if (!hdfs_kerberos_params.principal.empty()) {
            hdfsBuilderSetUserName(builder, hdfs_kerberos_params.principal.c_str());
        }
        if (!hdfs_kerberos_params.keytab_file.empty()) {
            hdfsBuilderSetKerbTicketCachePath(builder, hdfs_kerberos_params.keytab_file.c_str());
        }
        hdfsFS fs = hdfsBuilderConnect(builder);
        if (!fs) {
            throw IOException("Unable to connect to HDFS: " + path);
        }

        auto hadoop_file_handle = duckdb::make_uniq<HadoopFileHandle>(*this, path, flags, fs);

        string path_out, proto_host_port;
        HadoopFileSystem::ParseUrl(path, path_out, proto_host_port);

        hdfsFileInfo *file_info = hdfsGetPathInfo(hadoop_file_handle->hdfs, path.c_str());
        if (!file_info) {
            throw IOException("Unable to get file info: " + path);
        }
        if (file_info->mKind == kObjectKindDirectory) {
            hadoop_file_handle->file_type = FileType::FILE_TYPE_DIR;
        } else if (file_info->mKind == kObjectKindFile) {
            hadoop_file_handle->file_type = FileType::FILE_TYPE_REGULAR;
        } else {
            hadoop_file_handle->file_type = FileType::FILE_TYPE_INVALID;
        }
        hadoop_file_handle->length = file_info->mSize;
        hadoop_file_handle->last_modified = file_info->mLastMod;
        hdfsFreeFileInfo(file_info, 1);

        int hdfs_flag = 0;
        if ((flags & FileFlags::FILE_FLAGS_READ) &&
            ((flags & FileFlags::FILE_FLAGS_WRITE) || (flags & FileFlags::FILE_FLAGS_APPEND))) {
            hdfs_flag |= O_RDWR;
        } else if (flags & FileFlags::FILE_FLAGS_READ) {
            hdfs_flag |= O_RDONLY;
        } else if ((flags & FileFlags::FILE_FLAGS_WRITE) || (flags & FileFlags::FILE_FLAGS_APPEND)) {
            hdfs_flag |= O_WRONLY;
        }
        auto hdfs_stream_builder = hdfsStreamBuilderAlloc(hadoop_file_handle->hdfs, path.c_str(), hdfs_flag);
        if (!hdfs_stream_builder) {
            throw IOException("Failed to allocate stream builder.");
        }
        hadoop_file_handle->hdfs_file = hdfsStreamBuilderBuild(hdfs_stream_builder);
        if (!hadoop_file_handle->hdfs_file) {
            throw IOException("Failed to open file.");
        }
        return hadoop_file_handle;
    }

    FileType HadoopFileSystem::GetFileType(FileHandle &handle) {
        auto &hfh = (HadoopFileHandle &) handle;
        return hfh.file_type;
    }

    unique_ptr<FileHandle> HadoopFileSystem::OpenFile(const string &path, uint8_t flags, FileLockType lock,
                                                      FileCompressionType compression, FileOpener *opener) {

        auto handle = CreateHandle(path, flags, lock, compression, opener);
        handle->Initialize(opener);
        return std::move(handle);
    }

    void HadoopFileSystem::Read(FileHandle &handle, void *buffer, int64_t nr_bytes, idx_t location) {
        //auto &hfh = (HadoopFileHandle &) handle;
        Seek(handle, location);
        Read(handle, buffer, nr_bytes);
    }

    int64_t HadoopFileSystem::Read(FileHandle &handle, void *buffer, int64_t nr_bytes) {
        auto &hfh = (HadoopFileHandle &) handle;
        return hdfsRead(hfh.hdfs, hfh.hdfs_file, buffer, nr_bytes);
    }

    void HadoopFileSystem::Write(FileHandle &handle, void *buffer, int64_t nr_bytes, idx_t location) {
        throw NotImplementedException("Writing to HDFS files not implemented");
    }

    int64_t HadoopFileSystem::Write(FileHandle &handle, void *buffer, int64_t nr_bytes) {
        auto &hfh = (HadoopFileHandle &) handle;
        Write(handle, buffer, nr_bytes, hfh.file_offset);
        return nr_bytes;
    }

    void HadoopFileSystem::FileSync(FileHandle &handle) {
        throw NotImplementedException("FileSync for HDFS files not implemented");
    }

    int64_t HadoopFileSystem::GetFileSize(FileHandle &handle) {
        auto &sfh = (HadoopFileHandle &) handle;
        return sfh.length;
    }

    time_t HadoopFileSystem::GetLastModifiedTime(FileHandle &handle) {
        auto &hfh = (HadoopFileHandle &) handle;
        return hfh.last_modified;
    }

    bool HadoopFileSystem::FileExists(const string &filename) {
        try {
            if (hdfsExists(hdfs, filename.c_str()) == 0) {
                return true;
            }
            return false;
        } catch (...) {
            return false;
        };
    }

    bool HadoopFileSystem::CanHandleFile(const string &fpath) {
        return fpath.rfind("hdfs://", 0) == 0;
    }

    void HadoopFileSystem::Seek(FileHandle &handle, idx_t location) {
        auto &hfh = (HadoopFileHandle &) handle;
        hdfsSeek(hfh.hdfs, hfh.hdfs_file, location);
        hfh.file_offset = location;
    }

    idx_t HadoopFileSystem::SeekPosition(FileHandle &handle) {
        auto &hfh = (HadoopFileHandle &) handle;
        return hfh.file_offset;
    }


} // namespace duckdb
