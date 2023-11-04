#define DUCKDB_EXTENSION_MAIN

#include "duckdb.hpp"
#include "hadoopfs_extension.hpp"

namespace duckdb {

static void LoadInternal(DatabaseInstance &instance) {
	auto &fs = instance.GetFileSystem();

	fs.RegisterSubSystem(make_uniq<HadoopFileSystem>());

	auto &config = DBConfig::GetConfig(instance);

	// Global HTTP config
	config.AddExtensionOption("namenode", "namenode", LogicalType::VARCHAR);
	config.AddExtensionOption("keytab_file", "keytab file", LogicalType::VARCHAR);
	config.AddExtensionOption("principal", "principal", LogicalType::VARCHAR);


	//auto provider = make_uniq<AWSEnvironmentCredentialsProvider>(config);
	//provider->SetAll();
}

void HadoopfsExtension::Load(DuckDB &db) {
	LoadInternal(*db.instance);
}
std::string HadoopfsExtension::Name() {
	return "hadoopfs";
}

} // namespace duckdb

extern "C" {

DUCKDB_EXTENSION_API void hadoopfs_init(duckdb::DatabaseInstance &db) {
	LoadInternal(db);
}

DUCKDB_EXTENSION_API const char *hadoopfs_version() {
	return duckdb::DuckDB::LibraryVersion();
}
}

#ifndef DUCKDB_EXTENSION_MAIN
#error DUCKDB_EXTENSION_MAIN not defined
#endif
