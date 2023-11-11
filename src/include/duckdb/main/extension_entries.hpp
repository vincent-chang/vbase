//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/main/extension_entries.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/unordered_map.hpp"

// NOTE: this file is generated by scripts/generate_extensions_function.py. Check out the check-load-install-extensions
//       job in .github/workflows/LinuxRelease.yml on how to use it

namespace duckdb {

struct ExtensionEntry {
	char name[48];
	char extension[48];
};

static constexpr ExtensionEntry EXTENSION_FUNCTIONS[] = {
    {"->>", "json"},
    {"array_to_json", "json"},
    {"create_fts_index", "fts"},
    {"current_localtime", "icu"},
    {"current_localtimestamp", "icu"},
    {"dbgen", "tpch"},
    {"drop_fts_index", "fts"},
    {"dsdgen", "tpcds"},
    {"excel_text", "excel"},
    {"from_json", "json"},
    {"from_json_strict", "json"},
    {"from_substrait", "substrait"},
    {"from_substrait_json", "substrait"},
    {"fuzz_all_functions", "sqlsmith"},
    {"fuzzyduck", "sqlsmith"},
    {"get_substrait", "substrait"},
    {"get_substrait_json", "substrait"},
    {"host", "inet"},
    {"iceberg_metadata", "iceberg"},
    {"iceberg_scan", "iceberg"},
    {"iceberg_snapshots", "iceberg"},
    {"icu_calendar_names", "icu"},
    {"icu_sort_key", "icu"},
    {"json", "json"},
    {"json_array", "json"},
    {"json_array_length", "json"},
    {"json_contains", "json"},
    {"json_deserialize_sql", "json"},
    {"json_execute_serialized_sql", "json"},
    {"json_extract", "json"},
    {"json_extract_path", "json"},
    {"json_extract_path_text", "json"},
    {"json_extract_string", "json"},
    {"json_group_array", "json"},
    {"json_group_object", "json"},
    {"json_group_structure", "json"},
    {"json_keys", "json"},
    {"json_merge_patch", "json"},
    {"json_object", "json"},
    {"json_quote", "json"},
    {"json_serialize_sql", "json"},
    {"json_structure", "json"},
    {"json_transform", "json"},
    {"json_transform_strict", "json"},
    {"json_type", "json"},
    {"json_valid", "json"},
    {"load_aws_credentials", "aws"},
    {"make_timestamptz", "icu"},
    {"parquet_metadata", "parquet"},
    {"parquet_scan", "parquet"},
    {"parquet_schema", "parquet"},
    {"pg_clear_cache", "postgres_scanner"},
    {"pg_timezone_names", "icu"},
    {"postgres_attach", "postgres_scanner"},
    {"postgres_query", "postgres_scanner"},
    {"postgres_scan", "postgres_scanner"},
    {"postgres_scan_pushdown", "postgres_scanner"},
    {"read_json", "json"},
    {"read_json_auto", "json"},
    {"read_json_objects", "json"},
    {"read_json_objects_auto", "json"},
    {"read_ndjson", "json"},
    {"read_ndjson_auto", "json"},
    {"read_ndjson_objects", "json"},
    {"read_parquet", "parquet"},
    {"reduce_sql_statement", "sqlsmith"},
    {"row_to_json", "json"},
    {"scan_arrow_ipc", "arrow"},
    {"sql_auto_complete", "autocomplete"},
    {"sqlite_attach", "sqlite_scanner"},
    {"sqlite_scan", "sqlite_scanner"},
    {"sqlsmith", "sqlsmith"},
    {"st_area", "spatial"},
    {"st_area_spheroid", "spatial"},
    {"st_asgeojson", "spatial"},
    {"st_ashexwkb", "spatial"},
    {"st_astext", "spatial"},
    {"st_aswkb", "spatial"},
    {"st_boundary", "spatial"},
    {"st_buffer", "spatial"},
    {"st_centroid", "spatial"},
    {"st_collect", "spatial"},
    {"st_collectionextract", "spatial"},
    {"st_contains", "spatial"},
    {"st_containsproperly", "spatial"},
    {"st_convexhull", "spatial"},
    {"st_coveredby", "spatial"},
    {"st_covers", "spatial"},
    {"st_crosses", "spatial"},
    {"st_difference", "spatial"},
    {"st_dimension", "spatial"},
    {"st_disjoint", "spatial"},
    {"st_distance", "spatial"},
    {"st_distance_spheroid", "spatial"},
    {"st_drivers", "spatial"},
    {"st_dump", "spatial"},
    {"st_dwithin", "spatial"},
    {"st_dwithin_spheroid", "spatial"},
    {"st_endpoint", "spatial"},
    {"st_envelope", "spatial"},
    {"st_envelope_agg", "spatial"},
    {"st_equals", "spatial"},
    {"st_extent", "spatial"},
    {"st_exteriorring", "spatial"},
    {"st_flipcoordinates", "spatial"},
    {"st_geometrytype", "spatial"},
    {"st_geomfromgeojson", "spatial"},
    {"st_geomfromhexewkb", "spatial"},
    {"st_geomfromhexwkb", "spatial"},
    {"st_geomfromtext", "spatial"},
    {"st_geomfromwkb", "spatial"},
    {"st_intersection", "spatial"},
    {"st_intersection_agg", "spatial"},
    {"st_intersects", "spatial"},
    {"st_intersects_extent", "spatial"},
    {"st_isclosed", "spatial"},
    {"st_isempty", "spatial"},
    {"st_isring", "spatial"},
    {"st_issimple", "spatial"},
    {"st_isvalid", "spatial"},
    {"st_length", "spatial"},
    {"st_length_spheroid", "spatial"},
    {"st_linemerge", "spatial"},
    {"st_linestring2dfromwkb", "spatial"},
    {"st_list_proj_crs", "spatial"},
    {"st_makeenvelope", "spatial"},
    {"st_makeline", "spatial"},
    {"st_makepolygon", "spatial"},
    {"st_ngeometries", "spatial"},
    {"st_ninteriorrings", "spatial"},
    {"st_normalize", "spatial"},
    {"st_npoints", "spatial"},
    {"st_numgeometries", "spatial"},
    {"st_numinteriorrings", "spatial"},
    {"st_numpoints", "spatial"},
    {"st_overlaps", "spatial"},
    {"st_perimeter", "spatial"},
    {"st_perimeter_spheroid", "spatial"},
    {"st_point", "spatial"},
    {"st_point2d", "spatial"},
    {"st_point2dfromwkb", "spatial"},
    {"st_point3d", "spatial"},
    {"st_point4d", "spatial"},
    {"st_pointn", "spatial"},
    {"st_pointonsurface", "spatial"},
    {"st_polygon2dfromwkb", "spatial"},
    {"st_reverse", "spatial"},
    {"st_read", "spatial"},
    {"st_readosm", "spatial"},
    {"st_reduceprecision", "spatial"},
    {"st_removerepeatedpoints", "spatial"},
    {"st_simplify", "spatial"},
    {"st_simplifypreservetopology", "spatial"},
    {"st_startpoint", "spatial"},
    {"st_touches", "spatial"},
    {"st_transform", "spatial"},
    {"st_union", "spatial"},
    {"st_union_agg", "spatial"},
    {"st_within", "spatial"},
    {"st_x", "spatial"},
    {"st_xmax", "spatial"},
    {"st_xmin", "spatial"},
    {"st_y", "spatial"},
    {"st_ymax", "spatial"},
    {"st_ymin", "spatial"},
    {"stem", "fts"},
    {"text", "excel"},
    {"to_arrow_ipc", "arrow"},
    {"to_json", "json"},
    {"tpcds", "tpcds"},
    {"tpcds_answers", "tpcds"},
    {"tpcds_queries", "tpcds"},
    {"tpch", "tpch"},
    {"tpch_answers", "tpch"},
    {"tpch_queries", "tpch"},
    {"visualize_diff_profiling_output", "visualizer"},
    {"visualize_json_profiling_output", "visualizer"},
    {"visualize_last_profiling_output", "visualizer"},
}; // END_OF_EXTENSION_FUNCTIONS

static constexpr ExtensionEntry EXTENSION_SETTINGS[] = {
    {"azure_storage_connection_string", "azure"},
    {"binary_as_string", "parquet"},
    {"calendar", "icu"},
    {"force_download", "httpfs"},
    {"http_retries", "httpfs"},
    {"http_retry_backoff", "httpfs"},
    {"http_retry_wait_ms", "httpfs"},
    {"http_timeout", "httpfs"},
    {"pg_debug_show_queries", "postgres_scanner"},
    {"pg_use_binary_copy", "postgres_scanner"},
    {"pg_experimental_filter_pushdown", "postgres_scanner"},
    {"pg_connection_limit", "postgres_scanner"},
    {"pg_pages_per_task", "postgres_scanner"},
    {"pg_array_as_varchar", "postgres_scanner"},
    {"s3_access_key_id", "httpfs"},
    {"s3_endpoint", "httpfs"},
    {"s3_region", "httpfs"},
    {"s3_secret_access_key", "httpfs"},
    {"s3_session_token", "httpfs"},
    {"s3_uploader_max_filesize", "httpfs"},
    {"s3_uploader_max_parts_per_file", "httpfs"},
    {"s3_uploader_thread_limit", "httpfs"},
    {"s3_url_compatibility_mode", "httpfs"},
    {"hdfs_default_namenode", "hadoopfs"},
    {"hdfs_principal", "hadoopfs"},
    {"hdfs_keytab_file", "hadoopfs"},
    {"s3_url_style", "httpfs"},
    {"s3_use_ssl", "httpfs"},
    {"sqlite_all_varchar", "sqlite_scanner"},
    {"timezone", "icu"},
}; // END_OF_EXTENSION_SETTINGS

// Note: these are currently hardcoded in scripts/generate_extensions_function.py
// TODO: automate by passing though to script via duckdb
static constexpr ExtensionEntry EXTENSION_COPY_FUNCTIONS[] = {{"parquet", "parquet"},
                                                              {"json", "json"}}; // END_OF_EXTENSION_COPY_FUNCTIONS

// Note: these are currently hardcoded in scripts/generate_extensions_function.py
// TODO: automate by passing though to script via duckdb
static constexpr ExtensionEntry EXTENSION_TYPES[] = {
    {"json", "json"}, {"inet", "inet"}, {"geometry", "spatial"}}; // END_OF_EXTENSION_TYPES

// Note: these are currently hardcoded in scripts/generate_extensions_function.py
// TODO: automate by passing though to script via duckdb
static constexpr ExtensionEntry EXTENSION_COLLATIONS[] = {
    {"af", "icu"},    {"am", "icu"},    {"ar", "icu"},     {"ar_sa", "icu"}, {"as", "icu"},    {"az", "icu"},
    {"be", "icu"},    {"bg", "icu"},    {"bn", "icu"},     {"bo", "icu"},    {"br", "icu"},    {"bs", "icu"},
    {"ca", "icu"},    {"ceb", "icu"},   {"chr", "icu"},    {"cs", "icu"},    {"cy", "icu"},    {"da", "icu"},
    {"de", "icu"},    {"de_at", "icu"}, {"dsb", "icu"},    {"dz", "icu"},    {"ee", "icu"},    {"el", "icu"},
    {"en", "icu"},    {"en_us", "icu"}, {"eo", "icu"},     {"es", "icu"},    {"et", "icu"},    {"fa", "icu"},
    {"fa_af", "icu"}, {"ff", "icu"},    {"fi", "icu"},     {"fil", "icu"},   {"fo", "icu"},    {"fr", "icu"},
    {"fr_ca", "icu"}, {"fy", "icu"},    {"ga", "icu"},     {"gl", "icu"},    {"gu", "icu"},    {"ha", "icu"},
    {"haw", "icu"},   {"he", "icu"},    {"he_il", "icu"},  {"hi", "icu"},    {"hr", "icu"},    {"hsb", "icu"},
    {"hu", "icu"},    {"hy", "icu"},    {"id", "icu"},     {"id_id", "icu"}, {"ig", "icu"},    {"is", "icu"},
    {"it", "icu"},    {"ja", "icu"},    {"ka", "icu"},     {"kk", "icu"},    {"kl", "icu"},    {"km", "icu"},
    {"kn", "icu"},    {"ko", "icu"},    {"kok", "icu"},    {"ku", "icu"},    {"ky", "icu"},    {"lb", "icu"},
    {"lkt", "icu"},   {"ln", "icu"},    {"lo", "icu"},     {"lt", "icu"},    {"lv", "icu"},    {"mk", "icu"},
    {"ml", "icu"},    {"mn", "icu"},    {"mr", "icu"},     {"ms", "icu"},    {"mt", "icu"},    {"my", "icu"},
    {"nb", "icu"},    {"nb_no", "icu"}, {"ne", "icu"},     {"nl", "icu"},    {"nn", "icu"},    {"om", "icu"},
    {"or", "icu"},    {"pa", "icu"},    {"pa_in", "icu"},  {"pl", "icu"},    {"ps", "icu"},    {"pt", "icu"},
    {"ro", "icu"},    {"ru", "icu"},    {"sa", "icu"},     {"se", "icu"},    {"si", "icu"},    {"sk", "icu"},
    {"sl", "icu"},    {"smn", "icu"},   {"sq", "icu"},     {"sr", "icu"},    {"sr_ba", "icu"}, {"sr_me", "icu"},
    {"sr_rs", "icu"}, {"sv", "icu"},    {"sw", "icu"},     {"ta", "icu"},    {"te", "icu"},    {"th", "icu"},
    {"tk", "icu"},    {"to", "icu"},    {"tr", "icu"},     {"ug", "icu"},    {"uk", "icu"},    {"ur", "icu"},
    {"uz", "icu"},    {"vi", "icu"},    {"wae", "icu"},    {"wo", "icu"},    {"xh", "icu"},    {"yi", "icu"},
    {"yo", "icu"},    {"yue", "icu"},   {"yue_cn", "icu"}, {"zh", "icu"},    {"zh_cn", "icu"}, {"zh_hk", "icu"},
    {"zh_mo", "icu"}, {"zh_sg", "icu"}, {"zh_tw", "icu"},  {"zu", "icu"}}; // END_OF_EXTENSION_COLLATIONS

// Note: these are currently hardcoded in scripts/generate_extensions_function.py
// TODO: automate by passing though to script via duckdb
static constexpr ExtensionEntry EXTENSION_FILE_PREFIXES[] = {
    {"http://", "httpfs"}, {"https://", "httpfs"}, {"s3://", "httpfs"},
    {"hdfs://", "hadoopfs"}
    //    {"azure://", "azure"}
}; // END_OF_EXTENSION_FILE_PREFIXES

// Note: these are currently hardcoded in scripts/generate_extensions_function.py
// TODO: automate by passing though to script via duckdb
static constexpr ExtensionEntry EXTENSION_FILE_POSTFIXES[] = {
    {".parquet", "parquet"}, {".json", "json"},    {".jsonl", "json"}, {".ndjson", "json"},
    {".shp", "spatial"},     {".gpkg", "spatial"}, {".fgb", "spatial"}}; // END_OF_EXTENSION_FILE_POSTFIXES

// Note: these are currently hardcoded in scripts/generate_extensions_function.py
// TODO: automate by passing though to script via duckdb
static constexpr ExtensionEntry EXTENSION_FILE_CONTAINS[] = {{".parquet?", "parquet"},
                                                             {".json?", "json"},
                                                             {".ndjson?", ".jsonl?"},
                                                             {".jsonl?", ".ndjson?"}}; // EXTENSION_FILE_CONTAINS

static constexpr const char *AUTOLOADABLE_EXTENSIONS[] = {
    //    "azure",
    "arrow",
    "aws",
    "autocomplete",
    "excel",
    "fts",
    "httpfs",
    // "inet",
    // "icu",
    "json",
    "parquet",
    "postgres_scanner",
    // "spatial", TODO: table function isnt always autoloaded so test fails
    "sqlsmith",
    "sqlite_scanner",
    "tpcds",
    "tpch",
    "visualizer",
}; // END_OF_AUTOLOADABLE_EXTENSIONS

} // namespace duckdb
