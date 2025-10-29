cmake_minimum_required(VERSION 3.10)
project(EmbeddedCLI C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Enable testing
enable_testing()

# Collect sources: main.c and all .c files in Cli/
# Collect all .c sources in Cli/
file(GLOB CLI_SOURCES "${CMAKE_SOURCE_DIR}/src/*.c")

# Collect custom_assert sources
file(GLOB CUSTOM_ASSERT_SOURCES "${CMAKE_SOURCE_DIR}/utils/embedded_utils/utils/*.c")

add_executable(firmware-cli ${CMAKE_SOURCE_DIR}/example/host.c ${CLI_SOURCES} ${CUSTOM_ASSERT_SOURCES})

# Include directories
target_include_directories(firmware-cli PRIVATE 
    ${CMAKE_SOURCE_DIR} 
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/utils/embedded_utils/utils
)

# Compiler warnings (optional)
if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(firmware-cli PRIVATE -Wall -Wextra -Wpedantic -g3)
endif()

# Add Ceedling integration
find_program(CEEDLING_EXECUTABLE ceedling)
if(CEEDLING_EXECUTABLE)
    # Add custom target for running Ceedling tests
    add_custom_target(ceedling_tests
        COMMAND ${CEEDLING_EXECUTABLE} test:all
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running Ceedling unit tests"
        USES_TERMINAL
    )
    
    # Add Ceedling as a CTest
    add_test(
        NAME ceedling_unit_tests
        COMMAND ${CEEDLING_EXECUTABLE} test:all
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    
    # Optional: Add clean target for Ceedling
    add_custom_target(ceedling_clean
        COMMAND ${CEEDLING_EXECUTABLE} clean
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Cleaning Ceedling build artifacts"
    )
    
    message(STATUS "Ceedling found: ${CEEDLING_EXECUTABLE}")
else()
    message(WARNING "Ceedling not found. Unit tests will not be available.")
endif()

# Optional: Create a combined target that builds and tests
add_custom_target(build_and_test
    DEPENDS firmware-cli ceedling_tests
    COMMENT "Building project and running all tests"
)
