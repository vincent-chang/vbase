# 定义HDFS库的位置
set(HDFS_INCLUDE_DIRS "${HADOOP_HOME}/include")
set(HDFS_LIBRARY_DIRS "${HADOOP_HOME}/lib/native")
set(HDFS_LIBRARIES "${HDFS_LIBRARY_DIRS}/libhdfs.so")

# 检查文件是否存在
if(NOT EXISTS "${HDFS_LIBRARIES}")
    message(FATAL_ERROR "HDFS library not found at ${HDFS_LIBRARIES}")
endif()

# 创建导入库目标
add_library(HDFS::HDFS STATIC IMPORTED)
set_target_properties(HDFS::HDFS PROPERTIES
        IMPORTED_LOCATION "${HDFS_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${HDFS_INCLUDE_DIRS}"
        )
