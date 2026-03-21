include(FetchContent)

set(FMT_OS
    OFF
    CACHE BOOL "" FORCE)
set(FMT_UNICODE
    OFF
    CACHE BOOL "" FORCE)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG 12.1.0)
FetchContent_MakeAvailable(fmt)

# Options tailored for embedded systems (size optimization, no exceptions, etc.)
target_compile_definitions(
  fmt
  PUBLIC NDEBUG=0
         # FMT_THROW(s)=abort() # This saves 2-3Kb of flash but CMake does not
         # allow defining function-like macros with arguments... I did not found
         # a way to configure this cleanly without doing modifations to fmt's
         # source code. As an alternative, we use FMT_CUSTOM_ASSERT_FAIL=1
         FMT_CUSTOM_ASSERT_FAIL=1
         FMT_USE_FLOAT=0
         FMT_USE_DOUBLE=0
         FMT_USE_LONG_DOUBLE=0
         FMT_NO_LOCALTIME=1
         FMT_USE_INT128=0
         FMT_USE_LOCALE=1 # TODO: investigate necessity
         FMT_BUILTIN_TYPES=0
         FMT_OPTIMIZE_SIZE=2
         FMT_USE_EXCEPTIONS=0
         FMT_USE_FCNTL=0
         FMT_USE_FALLBACK_FILE=1
         FMT_USE_WRITE_CONSOLE=0)
target_compile_options(fmt PRIVATE -Os)

target_link_libraries(fmt PRIVATE ln::core)
target_sources(fmt PRIVATE ${CMAKE_CURRENT_LIST_DIR}/fmt_custom.cpp)
set_target_properties(fmt PROPERTIES CXX_CLANG_TIDY "")
