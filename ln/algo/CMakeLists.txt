add_library(ln_algo INTERFACE)
target_include_directories(ln_algo INTERFACE include)
target_link_libraries(ln_algo INTERFACE ln::core)

add_library(ln::algo ALIAS ln_algo)
target_link_libraries(ln INTERFACE ln::algo)

if(BUILD_TESTING AND PROJECT_IS_TOP_LEVEL)
  add_subdirectory(tests)
endif()
