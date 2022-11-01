cmake_minimum_required(VERSION 3.5)

# set up the Microchip cross toolchain
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/external/cmake-microchip/toolchain.cmake)

# set the default MCU model
set(MICROCHIP_MCU PIC18F87J50)

# FIXME: get someone who knows CMake to review this
if(DEFINED CODEOFFSET)
    message("codeoffset: overriding ${CODEOFFSET}")
else()
    # Allow user ovveriding to 0 for no bootloader
    message("codeoffset: default")
    set(CODEOFFSET 1800)
endif()

project(open-tl866 C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/dist)


add_library(core INTERFACE)

target_sources(core INTERFACE
    ${CMAKE_SOURCE_DIR}/main.c

    ${CMAKE_SOURCE_DIR}/arglib.c
    ${CMAKE_SOURCE_DIR}/at89.c
    ${CMAKE_SOURCE_DIR}/comlib.c
    ${CMAKE_SOURCE_DIR}/configuration_bits.c
    ${CMAKE_SOURCE_DIR}/ezzif.c
    ${CMAKE_SOURCE_DIR}/io.c
    ${CMAKE_SOURCE_DIR}/stock_compat.c
    ${CMAKE_SOURCE_DIR}/usb/usb_descriptors.c
)

# On Windows, xc8 can't find io.h, and _only_ io.h, without this path added. Compiler bug?
target_include_directories(core INTERFACE ${CMAKE_SOURCE_DIR})

# set linker options to support the bootloader
target_link_libraries(core INTERFACE
    # reserve the bootloader code area
    "--codeoffset=${CODEOFFSET}"
    # reserve the bootloader data area
    "--rom=default,-1fc00-1ffff"
    # set the firmware magic number
    "--serial=55aaa55a@1fbfc"
)

# include M-Stack USB stack with CDC ACM serial
find_package(MStack)
target_link_libraries(core INTERFACE MStack_CDC)
target_include_directories(core
    # lets MStack find usb_config.h
    INTERFACE ${CMAKE_SOURCE_DIR}/usb
)


function(add_tl866_mode name)
    add_executable(${name} ${ARGN})

    target_link_libraries(${name} PRIVATE
        # include the rest of the firmware
        core

        # select linker output files
        "--output=intel,elf"
        "--asmlist"
        "-m" # memory map
    )
endfunction()

# <RESERVED FOR THIRD PARTY>
# </RESERVED FOR THIRD PARTY>

add_tl866_mode(tl866-bitbang
    ${CMAKE_SOURCE_DIR}/modes/bitbang/main.c
)

add_tl866_mode(tl866-ezzif
    ${CMAKE_SOURCE_DIR}/modes/ezzif/main.c
)

add_tl866_mode(tl866-at89
    ${CMAKE_SOURCE_DIR}/modes/at89/main.c
)

add_tl866_mode(tl866-epromv
    ${CMAKE_SOURCE_DIR}/modes/epromv/main.c
)

add_tl866_mode(tl866-mcs48
    ${CMAKE_SOURCE_DIR}/modes/mcs48/main.c
)
