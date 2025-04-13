# 收集所有源文件
file(GLOB SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# 设置构建输出目录（可选，库不一定需要）
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

# 添加静态库 target
add_library(database STATIC ${SRC})

# 添加头文件搜索路径（让其他模块能 include 到）
target_include_directories(database
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include # 本模块头文件
  ${PROJECT_SOURCE_DIR}/util/include # util模块头文件
  ${PROJECT_SOURCE_DIR}/external/nlohmann # nlohmann JSON头文件
)

# 查找 mysqlclient 静态库
find_library(MYSQLCLIENT_LIB mysqlclient PATHS /usr/lib64/mysql)

# 链接需要的库
target_link_libraries(database
  PUBLIC
  util # 依赖 util 模块
  pthread # 多线程库
  ${MYSQLCLIENT_LIB} # MySQL 库
)

# 添加一个选项，用于是否构建测试程序
option(BUILD_DATABASE_TEST "Build the test executable for database module" OFF)

# 如果启用了测试，则编译 test/main.cpp
if(BUILD_DATABASE_TEST)
  add_executable(database_test ${CMAKE_CURRENT_SOURCE_DIR}/test/main.cpp)
  target_link_libraries(database_test PRIVATE database)
  target_include_directories(database_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
endif()
