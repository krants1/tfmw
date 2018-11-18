set(PROJECT_NAME Tests)
set(PROJECT_FILES "main.cpp" "packet.cpp")

enable_testing()
find_package(GTest REQUIRED)

add_executable(${PROJECT_NAME} ${PROJECT_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE GTest::GTest GTest::Main ${Boost_LIBRARIES})

add_test(${PROJECT_NAME} ${PROJECT_FILES})