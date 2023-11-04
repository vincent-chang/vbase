#include "hadoopfs.hpp"

#include "duckdb/common/atomic.hpp"
#include "duckdb/common/file_opener.hpp"
#include "duckdb/common/http_state.hpp"
#include "duckdb/common/thread.hpp"
#include "duckdb/common/types/hash.hpp"
#include "duckdb/function/scalar/strftime_format.hpp"
#include "duckdb/main/client_context.hpp"
#include "duckdb/main/database.hpp"

#include <chrono>
#include <string>
#include <thread>

#include <map>

namespace duckdb {

    void HadoopFileSystem::ParseUrl(string &url, string &path_out, string &proto_host_port_out) {
        if (url.rfind("http://", 0) != 0 && url.rfind("https://", 0) != 0) {
            throw IOException("URL needs to start with http:// or https://");
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

    HadoopFileHandle::HadoopFileHandle(FileSystem &fs, string path, uint8_t flags)
            : FileHandle(fs, path), flags(flags), length(0), buffer_available(0), buffer_idx(0),
              file_offset(0), buffer_start(0), buffer_end(0) {
    }

    unique_ptr<HadoopFileHandle> HadoopFileSystem::CreateHandle(const string &path, uint8_t flags, FileLockType lock,
                                                                FileCompressionType compression, FileOpener *opener) {
        D_ASSERT(compression == FileCompressionType::UNCOMPRESSED);
        return duckdb::make_uniq<HadoopFileHandle>(*this, path, flags);
    }

    unique_ptr<FileHandle> HadoopFileSystem::OpenFile(const string &path, uint8_t flags, FileLockType lock,
                                                      FileCompressionType compression, FileOpener *opener) {
        D_ASSERT(compression == FileCompressionType::UNCOMPRESSED);

        auto handle = CreateHandle(path, flags, lock, compression, opener);
        handle->Initialize(opener);
        return std::move(handle);
    }

// Buffered read from http file.
// Note that buffering is disabled when FileFlags::FILE_FLAGS_DIRECT_IO is set
    void HadoopFileSystem::Read(FileHandle &handle, void *buffer, int64_t nr_bytes, idx_t location) {
        auto &hfh = (HadoopFileHandle &) handle;
        //D_ASSERT(hfh.);

    }

    int64_t HadoopFileSystem::Read(FileHandle &handle, void *buffer, int64_t nr_bytes) {
        auto &hfh = (HadoopFileHandle &) handle;
        idx_t max_read = hfh.length - hfh.file_offset;
        nr_bytes = MinValue<idx_t>(max_read, nr_bytes);
        Read(handle, buffer, nr_bytes, hfh.file_offset);
        return nr_bytes;
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
        auto &sfh = (HadoopFileHandle &) handle;
        return sfh.last_modified;
    }

    bool HadoopFileSystem::FileExists(const string &filename) {
        try {
            auto handle = OpenFile(filename.c_str(), FileFlags::FILE_FLAGS_READ);
            auto &sfh = (HadoopFileHandle & ) * handle;
            if (sfh.length == 0) {
                return false;
            }
            return true;
        } catch (...) {
            return false;
        };
    }

    bool HadoopFileSystem::CanHandleFile(const string &fpath) {
        return fpath.rfind("hdfs://", 0) == 0;
    }

    void HadoopFileSystem::Seek(FileHandle &handle, idx_t location) {
        auto &sfh = (HadoopFileHandle &) handle;
        sfh.file_offset = location;
    }

    idx_t HadoopFileSystem::SeekPosition(FileHandle &handle) {
        auto &sfh = (HadoopFileHandle &) handle;
        return sfh.file_offset;
    }

    void HadoopFileHandle::Initialize(FileOpener *opener) {
        InitializeClient();
        auto &hfs = (HadoopFileSystem &) file_system;

    }

    void HadoopFileHandle::InitializeClient() {
        string path_out, proto_host_port;
        //HTTPFileSystem::ParseUrl(path, path_out, proto_host_port);
        //http_client = HTTPFileSystem::GetClient(this->http_params, proto_host_port.c_str());
    }

    HadoopFileHandle::~HadoopFileHandle() = default;
} // namespace duckdb
