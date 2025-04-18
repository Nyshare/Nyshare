file(GLOB SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

add_library(network ${SRC})

target_include_directories(network PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/util/include
)

target_link_libraries(network PUBLIC
  util
  pthread
)