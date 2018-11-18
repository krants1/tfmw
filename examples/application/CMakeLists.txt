set(PROJECT_NAME ExampleApp)
set(PROJECT_FILES main.cpp)

message(STATUS "Project ${PROJECT_NAME}, files: ${PROJECT_FILES}")

add_executable(${PROJECT_NAME} ${PROJECT_FILES})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

if (ENABLE_COTIRE AND NOT MSVC)    
    cotire(${PROJECT_NAME})
endif ()