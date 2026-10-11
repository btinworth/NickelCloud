cmake_minimum_required(VERSION 3.21)

project(NickelCloud LANGUAGES C CXX)

option(BUILD_TESTING "Build NickelCloud unit tests" OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

find_package(Qt5 REQUIRED COMPONENTS Core Gui)

execute_process(
    COMMAND git describe --tags --always --dirty
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_VARIABLE NICKELCLOUD_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)
if(NOT NICKELCLOUD_VERSION)
    set(NICKELCLOUD_VERSION dev)
endif()

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/NickelHook")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/NickelHook/nhplugin.json"
    "{\"Keys\":[\"nh.nickelcloud\"]}\n")

file(GLOB NICKELCLOUD_SOURCES CONFIGURE_DEPENDS
    "src/*.cc"
    "src/*.h"
    "NickelHook/*.c"
    "NickelHook/*.h"
)
add_library(nickelcloud SHARED
    ${NICKELCLOUD_SOURCES}
)
set_target_properties(nickelcloud PROPERTIES
    BUILD_RPATH "/usr/local/Kobo;/usr/local/Qt-5.2.1-arm/lib"
    INSTALL_RPATH "/usr/local/Kobo;/usr/local/Qt-5.2.1-arm/lib"
)
target_include_directories(nickelcloud PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/NickelHook"
    "${CMAKE_CURRENT_BINARY_DIR}/NickelHook"
)
target_compile_definitions(nickelcloud PRIVATE "NH_VERSION=\"${NICKELCLOUD_VERSION}\"")

set(NICKELCLOUD_WARNINGS
    -Wall -Wextra -Werror -Wformat=2
)
set(NICKELCLOUD_CXX_WARNINGS
    -Wno-missing-field-initializers
    -Wnon-virtual-dtor -Woverloaded-virtual -Wcast-qual
    -Wredundant-decls -Wlogical-op -Wfloat-equal -Wundef
)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(nickelcloud PRIVATE
        -O2 -pthread ${NICKELCLOUD_WARNINGS}
        -Wno-error=unused-result
    )
elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
    target_compile_options(nickelcloud PRIVATE -O2 -pthread ${NICKELCLOUD_WARNINGS})
else()
    message(FATAL_ERROR "NickelCloud requires a GNU or Clang C compiler")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND NICKELCLOUD_CXX_WARNINGS -Wlogical-op)
elseif(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "NickelCloud requires a GNU or Clang C++ compiler")
endif()
target_compile_options(nickelcloud PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:-O2;-pthread;${NICKELCLOUD_WARNINGS};${NICKELCLOUD_CXX_WARNINGS}>"
)
if(UNIX)
    target_link_options(nickelcloud PRIVATE -pthread -Wl,--as-needed -Wl,--no-undefined)
endif()
target_link_libraries(nickelcloud PRIVATE
    Qt5::Core
    Qt5::Gui
    ${CMAKE_DL_LIBS}
)

set(RCLONE_VERSION "1.75.1" CACHE STRING "rclone version to package")

function(add_nickelcloud_download output script)
    set(output_path "${CMAKE_CURRENT_BINARY_DIR}/${output}")
    add_custom_command(
        OUTPUT "${output_path}"
        COMMAND "${CMAKE_COMMAND}" "-DOUTPUT=${output_path}" ${ARGN}
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${script}"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${script}"
        VERBATIM
    )
    set(NICKELCLOUD_DOWNLOADS ${NICKELCLOUD_DOWNLOADS} "${output_path}" PARENT_SCOPE)
endfunction()

add_nickelcloud_download(rclone-armv7 DownloadRclone.cmake "-DRCLONE_VERSION=${RCLONE_VERSION}")
add_nickelcloud_download(cacert.pem DownloadCaBundle.cmake)
add_custom_target(nickelcloud_assets DEPENDS ${NICKELCLOUD_DOWNLOADS})

add_custom_target(koboroot
    COMMAND "${CMAKE_COMMAND}"
        "-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
        "-DBINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/PackageKoboRoot.cmake"
    DEPENDS nickelcloud nickelcloud_assets
    VERBATIM
)

if(BUILD_TESTING)
    include(CTest)
    find_package(Qt5 REQUIRED COMPONENTS Test)

    file(GLOB NICKELCLOUD_TEST_SOURCES CONFIGURE_DEPENDS
        "tests/*.cc"
        "tests/*.h"
    )
    add_executable(nickelcloud_tests
        ${NICKELCLOUD_TEST_SOURCES}
        src/Constants.cc
        src/Log.cc
        src/UserConfig.cc
    )
    target_include_directories(nickelcloud_tests PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/NickelHook"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
    )
    target_compile_options(nickelcloud_tests PRIVATE -Wno-deprecated-declarations)
    target_link_libraries(nickelcloud_tests PRIVATE Qt5::Core Qt5::Test)
    add_test(NAME NickelCloudConfig COMMAND nickelcloud_tests)
endif()
