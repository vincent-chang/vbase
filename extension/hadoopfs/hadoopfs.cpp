#include "hadoopfs.hpp"

#include "duckdb/common/atomic.hpp"
#include "duckdb/common/printer.hpp"
#include "duckdb/common/file_opener.hpp"
#include "duckdb/common/http_state.hpp"
#include "duckdb/common/thread.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/common/types/hash.hpp"
#include "duckdb/function/scalar/strftime_format.hpp"
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
        string default_namenode = "";
        Value value;

        if (instance.TryGetCurrentSetting(HDFSParams::HDFS_DEFAULT_NAMENODE, value)) {
            default_namenode = value.ToString();
        }

        return {default_namenode};
    }

    HDFSParams HDFSParams::ReadFrom(FileOpener *opener, FileOpenerInfo &info) {
        string default_namenode = "";
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
        Printer::Print("HadoopFileHandle Close." );
        /*if (hdfs_stream_builder) {
            Printer::Print("HadoopFileHandle hdfsStreamBuilderFree." );
            hdfsStreamBuilderFree(hdfs_stream_builder);
        }*/
        if (hdfs_file) {
            Printer::Print("HadoopFileHandle hdfsCloseFile." );
            hdfsCloseFile(hdfs, hdfs_file);
        }
        if (hdfs) {
            Printer::Print("HadoopFileHandle hdfsDisconnect.");
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
            Printer::Print("HadoopFileSystem hdfsDisconnect." );
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

        Printer::Print("Begin get file info: " + path);
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
        Printer::Print("End get file info: " + path);

        int hdfs_flag = 0;
        if ((flags & FileFlags::FILE_FLAGS_READ) &&
            ((flags & FileFlags::FILE_FLAGS_WRITE) || (flags & FileFlags::FILE_FLAGS_APPEND))) {
            hdfs_flag |= O_RDWR;
        } else if (flags & FileFlags::FILE_FLAGS_READ) {
            hdfs_flag |= O_RDONLY;
        } else if ((flags & FileFlags::FILE_FLAGS_WRITE) || (flags & FileFlags::FILE_FLAGS_APPEND)) {
            hdfs_flag |= O_WRONLY;
        }
        Printer::Print("Begin hdfsStreamBuilderAlloc: " + path);
        auto hdfs_stream_builder = hdfsStreamBuilderAlloc(hadoop_file_handle->hdfs, path.c_str(), hdfs_flag);
        if (!hdfs_stream_builder) {
            throw IOException("Failed to allocate stream builder.");
        }
        Printer::Print("End hdfsStreamBuilderAlloc: " + path);
        Printer::Print("Begin hdfsStreamBuilderBuild: " + path);
        hadoop_file_handle->hdfs_file = hdfsStreamBuilderBuild(hdfs_stream_builder);
        if (!hadoop_file_handle->hdfs_file) {
            throw IOException("Failed to open file.");
        }
        Printer::Print("End hdfsStreamBuilderBuild: " + path);
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
