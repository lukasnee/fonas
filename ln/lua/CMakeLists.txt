add_library(ln_lua)
target_link_libraries(ln INTERFACE ln::lua)

add_library(ln::lua ALIAS ln_lua)
target_link_libraries(ln_lua PUBLIC lua ln::core FreeRTOS-Cpp)
target_sources(ln_lua PRIVATE src/VM.cpp)
target_include_directories(ln_lua PUBLIC include)
