add_library(ln_port_FreeRTOS)
add_library(ln::port::FreeRTOS ALIAS ln_port_FreeRTOS)

target_sources(ln_port_FreeRTOS PRIVATE src/port.cpp)
target_include_directories(ln_port_FreeRTOS PUBLIC include)
target_link_libraries(ln_port_FreeRTOS PUBLIC ln::core FreeRTOS-Cpp)
