add_executable(ln_core_test RingBuffer_test.cpp StaticList_test.cpp)
target_link_libraries(ln_core_test PRIVATE Catch2::Catch2WithMain ln::core
                                           ln::port::stdcpp)
catch_discover_tests(ln_core_test)
