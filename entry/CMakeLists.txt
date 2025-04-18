file(GLOB SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

add_executable(Nyshare ${SRC_DIR})

target_include_directories(Nyshare PUBLIC
  ${PROJECT_SOURCE_DIR}/http/include
  ${PROJECT_SOURCE_DIR}/network/include
)

target_link_libraries(Nyshare PUBLIC
  http
  network
)
