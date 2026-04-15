add_executable(ln_algo_test StaticDAG_test.cpp)
target_link_libraries(ln_algo_test PRIVATE Catch2::Catch2WithMain ln::algo)
catch_discover_tests(ln_algo_test)
