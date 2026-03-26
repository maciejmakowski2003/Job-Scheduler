include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
)
FetchContent_MakeAvailable(googletest)

add_executable(scheduler-tests
    MpscChannelTest.cpp
    ThreadPoolTest.cpp
)

target_link_libraries(scheduler-tests PRIVATE scheduler-lib GTest::gtest_main)

include(CTest)
include(GoogleTest)
gtest_discover_tests(scheduler-tests)
