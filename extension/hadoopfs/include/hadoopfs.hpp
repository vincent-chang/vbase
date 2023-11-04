#pragma once

#include "duckdb/common/file_system.hpp"
#include "duckdb/common/pair.hpp"
#include "duckdb/common/unordered_map.hpp"
#include "duckdb/common/case_insensitive_map.hpp"
#include "duckdb/main/client_data.hpp"
#include "hdfs.h"

namespace duckdb {

    using HeaderMap = case_insensitive_map_t<string>;

    class HadoopFileHandle : public FileHandle {
    public:
        HadoopFileHandle(FileSystem &fs, string path, uint8_t flags);

        ~HadoopFileHandle() override;

        // This two-phase construction allows subclasses more flexible setup.
        virtual void Initialize(FileOpener *opener);

        // We keep an http client stored for connection reuse with keep-alive headers

        // File handle info
        uint8_t flags;
        idx_t length;
        time_t last_modified;

        // Read info
        idx_t buffer_available;
        idx_t buffer_idx;
        idx_t file_offset;
        idx_t buffer_start;
        idx_t buffer_end;

        // Read buffer
        duckdb::unique_ptr<data_t[]> read_buffer;
        constexpr static idx_t READ_BUFFER_LEN = 1000000;

    public:
        void Close() override {
        }

    protected:
        virtual void InitializeClient();
    };

    class HadoopFileSystem : public FileSystem {
    public:
        static void ParseUrl(string &url, string &path_out, string &proto_host_port_out);

        duckdb::unique_ptr<FileHandle> OpenFile(const string &path, uint8_t flags, FileLockType lock = DEFAULT_LOCK,
                                                FileCompressionType compression = DEFAULT_COMPRESSION,
                                                FileOpener *opener = nullptr) final;

        vector<string> Glob(const string &path, FileOpener *opener = nullptr) override {
            return {path}; // FIXME
        }

        void Read(FileHandle &handle, void *buffer, int64_t nr_bytes, idx_t location) override;

        int64_t Read(FileHandle &handle, void *buffer, int64_t nr_bytes) override;

        void Write(FileHandle &handle, void *buffer, int64_t nr_bytes, idx_t location) override;

        int64_t Write(FileHandle &handle, void *buffer, int64_t nr_bytes) override;

        void FileSync(FileHandle &handle) override;

        int64_t GetFileSize(FileHandle &handle) override;

        time_t GetLastModifiedTime(FileHandle &handle) override;

        bool FileExists(const string &filename) override;

        void Seek(FileHandle &handle, idx_t location) override;

        idx_t SeekPosition(FileHandle &handle) override;

        bool CanHandleFile(const string &fpath) override;

        bool CanSeek() override {
            return true;
        }

        bool OnDiskFile(FileHandle &handle) override {
            return false;
        }

        bool IsPipe(const string &filename) override {
            return false;
        }

        string GetName() const override {
            return "HadoopFileSystem";
        }

        string PathSeparator(const string &path) override {
            return "/";
        }

        static void Verify();


    protected:
        virtual duckdb::unique_ptr<HadoopFileHandle> CreateHandle(const string &path, uint8_t flags, FileLockType lock,
                                                                  FileCompressionType compression, FileOpener *opener);
    };

} // namespace duckdb
