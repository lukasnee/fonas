add_executable(ln_ringbuffer_test RingBuffer_test.cpp)
target_link_libraries(ln_ringbuffer_test PRIVATE Catch2::Catch2WithMain
                                                 ln::core ln::port::stdcpp)
catch_discover_tests(ln_ringbuffer_test)
