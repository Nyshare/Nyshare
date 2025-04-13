# 收集源文件
file(GLOB SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# 设置库的输出路径（静态库 .a 文件）
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

# 添加静态库 target（也可以改成 SHARED 动态库）
add_library(util STATIC ${SRC})

# 设置头文件搜索路径，让外部模块可以 include
target_include_directories(util
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 链接系统库，比如 pthread
target_link_libraries(util
  PUBLIC pthread
)
