set(PROJECT_NAME ExampleProto)
set(PROJECT_FILES "main.cpp" "scr/tcp.cpp")

message(STATUS "Project ${PROJECT_NAME}, files: ${PROJECT_FILES}")
find_package(Protobuf REQUIRED)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS api.proto)
message(STATUS "Protobuf_SRC_ROOT_FOLDER: ${Protobuf_SRC_ROOT_FOLDER}")	
message(STATUS "PROTO_SRCS: ${PROTO_SRCS}")	
message(STATUS "PROTO_HDRS: ${PROTO_HDRS}")	

add_executable(${PROJECT_NAME} ${PROJECT_FILES} ${PROTO_SRCS} ${PROTO_HDRS})

target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

#Protobuf
message(STATUS "Protobuf_VERSION: ${Protobuf_VERSION}")
message(STATUS "Protobuf_INCLUDE_DIRS: ${PROTOBUF_INCLUDE_DIRS}")
message(STATUS "Protobuf_LIBRARIES: ${PROTOBUF_LIBRARIES}")
message(STATUS "Protobuf_PROTOC_EXECUTABLE: ${Protobuf_PROTOC_EXECUTABLE}")
include_directories(${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(${PROJECT_NAME} ${PROTOBUF_LIBRARIES})
include_directories(${PROTOBUF_INCLUDE_DIRS})

if (ENABLE_COTIRE AND NOT MSVC)
    cotire(${PROJECT_NAME})
endif ()
