cmake_minimum_required(VERSION 3.16)
project(stayink_hide_guard C)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(STAYINK_BUILD_LINUX_GUARD_DEFAULT ON)
else()
    set(STAYINK_BUILD_LINUX_GUARD_DEFAULT OFF)
endif()
option(STAYINK_BUILD_LINUX_GUARD "Build the Linux/Xlib guard" ${STAYINK_BUILD_LINUX_GUARD_DEFAULT})
option(STAYINK_ENABLE_EXPERIMENTAL_WRITES "Build a separately named write-capable guard" OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(hide_guard_core STATIC
    src/hide_guard_core.c
)
target_include_directories(hide_guard_core PUBLIC include)
target_compile_options(hide_guard_core PRIVATE -Wall -Wextra -Werror -pedantic)

enable_testing()
add_executable(hide_guard_core_test tests/hide_guard_core_test.c)
target_link_libraries(hide_guard_core_test PRIVATE hide_guard_core)
target_compile_options(hide_guard_core_test PRIVATE -Wall -Wextra -Werror -pedantic)
add_test(NAME hide_guard_core_test COMMAND hide_guard_core_test)

if(STAYINK_BUILD_LINUX_GUARD)
    if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
        message(FATAL_ERROR "The guard uses Linux timerfd and may only be built for Linux")
    endif()

    find_package(X11 REQUIRED)

    add_executable(stayink-hide-guard src/hide_guard_linux.c)
    target_link_libraries(stayink-hide-guard PRIVATE hide_guard_core X11::X11)
    target_compile_definitions(stayink-hide-guard PRIVATE STAYINK_ENABLE_WRITES=0)
    target_compile_options(stayink-hide-guard PRIVATE -Wall -Wextra -Werror -pedantic)

    if(STAYINK_ENABLE_EXPERIMENTAL_WRITES)
        add_executable(stayink-hide-guard-write-enabled src/hide_guard_linux.c)
        target_link_libraries(stayink-hide-guard-write-enabled PRIVATE hide_guard_core X11::X11)
        target_compile_definitions(stayink-hide-guard-write-enabled PRIVATE STAYINK_ENABLE_WRITES=1)
        target_compile_options(stayink-hide-guard-write-enabled PRIVATE -Wall -Wextra -Werror -pedantic)
    endif()
endif()
