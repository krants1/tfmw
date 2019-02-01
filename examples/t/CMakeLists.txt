set(PROJECT_NAME Examples)
set(PROJECT_FILES main.cpp)
list(APPEND PROJECT_FILES 
		"tcp.cpp"				
        "log.cpp"
        "threads.cpp"
        "time_utils.cpp"
        "ini_files.cpp"
        "packet.cpp"
        "counters.cpp"
		"buffers_stack.cpp")

message(STATUS "Project ${PROJECT_NAME}, files: ${PROJECT_FILES}")

add_executable(${PROJECT_NAME} ${PROJECT_FILES})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

if (ENABLE_COTIRE AND NOT MSVC)    
    cotire(${PROJECT_NAME})
endif ()
