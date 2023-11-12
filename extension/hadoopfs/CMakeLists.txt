cmake_minimum_required(VERSION 2.8.12)

project(HadoopFsExtension)

add_extension_definitions()

include_directories(include ../../third_party/libhdfs3)

build_static_extension(hadoopfs hadoopfs.cpp
                       hadoopfs_extension.cpp)
set(PARAMETERS "-warnings")
build_loadable_extension(hadoopfs ${PARAMETERS} hadoopfs.cpp
                         hadoopfs_extension.cpp)

list(APPEND CMAKE_PREFIX_PATH ../../third_party/libhdfs3)
find_package(LIBHDFS3 REQUIRED)
include_directories(${LIBHDFS3_INCLUDE_DIRS})
target_link_libraries(hadoopfs_loadable_extension ${LIBHDFS3_LIBRARIES})
target_link_libraries(hadoopfs_extension ${LIBHDFS3_LIBRARIES})

install(
  TARGETS hadoopfs_extension
  EXPORT "${DUCKDB_EXPORT_SET}"
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}")
