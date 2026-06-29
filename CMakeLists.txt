cmake_minimum_required(VERSION 3.10)
project(pacman_tests)

# Find required packages
find_package(SDL2 REQUIRED)
find_package(cmocka REQUIRED)

# Include directories
include_directories(
    ${SDL2_INCLUDE_DIRS}
    ${CMOCKA_INCLUDE_DIR}
    src/
    tests/
)

# Set compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -DPACMAN_TESTS -Wno-unused-parameter")

# Test files - use MOCKS instead of real implementations
set(TEST_FILES
    tests/framework_mock.c    # Use mock framework
    tests/test.c              # Your test cases
    tests/firstLevel_stub.c   # Use stub for firstLevel
)

# DO NOT include src/framework.c or src/firstLevel.c
# Only include other necessary source files
set(SRC_FILES
    # Only include source files that are NOT mocked
    # Include main.c so the test target can link the real game logic (gameLoop)
    src/main.c
    # src/framework.c # Don't include - using mock instead
    # src/firstLevel.c # Don't include - using stub instead
)

# Add executable for tests
add_executable(pacman_tests
    ${TEST_FILES}
    ${SRC_FILES}
)

# Link libraries
target_link_libraries(pacman_tests
    ${SDL2_LIBRARIES}
    ${CMOCKA_LIBRARY}
    m
)