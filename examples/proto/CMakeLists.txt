set(PROJECT_NAME ExampleProto)
set(PROJECT_FILES "main.cpp" "scr/tcp.cpp")

message(STATUS "Project ${PROJECT_NAME}, files: ${PROJECT_FILES}")

add_executable(${PROJECT_NAME} ${PROJECT_FILES})

target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

find_package(Protobuf REQUIRED)
message(STATUS "Protobuf_VERSION: ${Protobuf_VERSION}")
message(STATUS "Protobuf_INCLUDE_DIRS: ${PROTOBUF_INCLUDE_DIRS}")
message(STATUS "Protobuf_LIBRARIES: ${PROTOBUF_LIBRARIES}")
message(STATUS "Protobuf_PROTOC_EXECUTABLE: ${Protobuf_PROTOC_EXECUTABLE}")

target_link_libraries(${PROJECT_NAME} ${PROTOBUF_LIBRARIES})
include_directories(${PROTOBUF_INCLUDE_DIRS})

if (ENABLE_COTIRE AND NOT MSVC)
    cotire(${PROJECT_NAME})
endif ()
