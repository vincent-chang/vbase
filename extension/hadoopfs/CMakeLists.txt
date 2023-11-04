cmake_minimum_required(VERSION 2.8.12)

project(HadoopFsExtension)

add_extension_definitions()

include_directories(include ../parquet/include)

build_static_extension(hadoopfs hadoopfs.cpp
                       hadoopfs_extension.cpp)
set(PARAMETERS "-warnings")
build_loadable_extension(hadoopfs ${PARAMETERS} hadoopfs.cpp
                         hadoopfs_extension.cpp)

if(MINGW)
  set(OPENSSL_USE_STATIC_LIBS TRUE)
endif()

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
target_link_libraries(hadoopfs_loadable_extension duckdb_mbedtls ${OPENSSL_LIBRARIES})
target_link_libraries(hadoopfs_extension duckdb_mbedtls ${OPENSSL_LIBRARIES})

list(APPEND CMAKE_PREFIX_PATH "./")
find_package(HDFS REQUIRED)
# message("HDFS include: ${HDFS_INCLUDE_DIRS}")
# message("HDFS library: ${HDFS_LIBRARIES}")
include_directories(${HDFS_INCLUDE_DIRS})
target_link_libraries(hadoopfs_loadable_extension ${HDFS_LIBRARIES})
target_link_libraries(hadoopfs_extension ${HDFS_LIBRARIES})

if(MINGW)
  find_package(ZLIB)
  target_link_libraries(hadoopfs_loadable_extension ZLIB::ZLIB -lcrypt32)
  target_link_libraries(hadoopfs_extension ZLIB::ZLIB -lcrypt32)
endif()

install(
  TARGETS hadoopfs_extension
  EXPORT "${DUCKDB_EXPORT_SET}"
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}")
