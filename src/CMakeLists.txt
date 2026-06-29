cmake_minimum_required(VERSION 3.16)
project(pacman C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Enable warnings
add_compile_options(-Wall -Wextra -Wpedantic)

# Find SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Sources
set(SOURCES
    main.c
    framework.c
    firstLevel.c
)

add_executable(pacman ${SOURCES})

# Link SDL2 properly
target_link_libraries(pacman ${SDL2_LIBRARIES})
