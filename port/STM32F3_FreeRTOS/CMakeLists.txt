add_library(ln_port_STM32F3_FreeRTOS)
add_library(ln::port::STM32F3_FreeRTOS ALIAS ln_port_STM32F3_FreeRTOS)

target_sources(ln_port_STM32F3_FreeRTOS PRIVATE src/port.cpp)
target_include_directories(ln_port_STM32F3_FreeRTOS PUBLIC include)
target_link_libraries(
  ln_port_STM32F3_FreeRTOS PUBLIC ln::core STM32F3xx_HAL_Driver
                                  ln::port::FreeRTOS fmt::fmt)
