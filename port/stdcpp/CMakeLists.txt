add_library(ln_port_stdcpp)
add_library(ln::port::stdcpp ALIAS ln_port_stdcpp)

target_sources(ln_port_stdcpp PRIVATE src/port.cpp)
target_include_directories(ln_port_stdcpp PUBLIC include)
target_link_libraries(ln_port_stdcpp PUBLIC ln::core)
