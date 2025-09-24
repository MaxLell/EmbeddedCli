cmake_minimum_required(VERSION 3.10)
project(EmbeddedCLI C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Collect sources: main.c and all .c files in Cli/
# Collect all .c sources in Cli/
file(GLOB CLI_SOURCES "${CMAKE_SOURCE_DIR}/Cli/*.c")

add_executable(firmware-cli ${CMAKE_SOURCE_DIR}/main.c ${CLI_SOURCES})

# Include directories
target_include_directories(firmware-cli PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/Cli)

# Compiler warnings (optional)
if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(firmware-cli PRIVATE -Wall -Wextra -Wpedantic -g)
endif()
