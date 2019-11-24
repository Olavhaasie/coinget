cmake_minimum_required(VERSION 3.1...3.15)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(coinget VERSION 2.1.0
                DESCRIPTION "the fastest cli computer price checker for cryptocurrencies"
                LANGUAGES C)

configure_file (
    "${PROJECT_SOURCE_DIR}/src/version.h.in"
    "${PROJECT_BINARY_DIR}/version.h"
    @ONLY
)

add_executable(coinget src/coin.c src/main.c src/util.c)
target_include_directories(coinget PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src extern/jsmn)
target_link_libraries(coinget PRIVATE curl)
target_compile_options(coinget PRIVATE -Wall -Wextra -Wpedantic -Werror)
set_target_properties(coinget PROPERTIES C_STANDARD 99)

install(TARGETS coinget RUNTIME DESTINATION bin)

install(FILES doc/coinget.1 TYPE MAN)
