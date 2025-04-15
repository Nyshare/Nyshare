file(GLOB SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

add_library(http ${SRC})

target_include_directories(http PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/database/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/external
)

target_link_libraries(http PUBLIC
  util
  database
)