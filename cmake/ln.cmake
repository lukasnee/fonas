include_guard()

function(ln_generate_firmware_output_files fw_target)
  target_link_options(
    ${fw_target} PUBLIC -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${fw_target}.map
    -Wl,--cref -Wl,--no-warn-rwx-segment)

  file(RELATIVE_PATH dir ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
  add_custom_command(
    TARGET ${fw_target}
    POST_BUILD
    COMMAND arm-none-eabi-size ${fw_target}
    COMMAND echo "Generating firmware artifacts:"
    COMMAND echo "${dir}/${fw_target}"
    COMMAND ${CMAKE_OBJCOPY} -O binary ${fw_target} ${fw_target}.bin
    COMMAND echo "${dir}/${fw_target}.bin"
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${fw_target} ${fw_target}.hex
    COMMAND echo "${dir}/${fw_target}.hex"
    COMMAND echo "${dir}/${fw_target}.map"
    COMMAND ${CMAKE_OBJDUMP} -S -t ${fw_target} > ${fw_target}.dump
    COMMAND echo "${dir}/${fw_target}.dump"
    COMMAND ${CMAKE_NM} ${fw_target} -C -n -S -s > ${fw_target}.address-sort.nm
    COMMAND echo "${dir}/${fw_target}.address-sort.nm"
    COMMAND ${CMAKE_NM} ${fw_target} -C -S -s --size-sort >
            ${fw_target}.size-sort.nm
    COMMAND echo "${dir}/${fw_target}.size-sort.nm"
    COMMAND ${CMAKE_NM} -lnC ${fw_target} > ${fw_target}.symbols
    COMMAND echo "${dir}/${fw_target}.symbols")
endfunction()

function(ln_add_firmware target_name linker_script openocd_cfg)

  add_executable(${target_name})

  target_link_options(${target_name} PRIVATE -T${linker_script})
  set_property(TARGET ${target_name} PROPERTY LINK_DEPENDS ${linker_script})

  execute_process(
    COMMAND git describe --always --dirty --match "NOT A TAG"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE git_hash
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  configure_file(${CMAKE_CURRENT_FUNCTION_LIST_DIR}/config/build.hpp.in
                 ${CMAKE_BINARY_DIR}/include/ln/build.hpp @ONLY)
  target_include_directories(fibsys INTERFACE ${CMAKE_BINARY_DIR}/include)

  ln_generate_firmware_output_files(${target_name})

  add_custom_target(
    flash_${target_name}
    COMMAND
      openocd -f ${openocd_cfg} -c
      "program ${PROJECT_BINARY_DIR}/${target_name} preverify verify reset exit"
    DEPENDS ${target_name}
    VERBATIM USES_TERMINAL # to get live output from openocd
    COMMENT "Flashing ${target_name} firmware...")

  add_custom_target(
    reset_${target_name}
    COMMAND openocd -f ${openocd_cfg} -c "init;reset;shutdown"
    VERBATIM
    COMMENT "Resetting target...")
endfunction()
