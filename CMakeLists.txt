cmake_minimum_required(VERSION 3.28)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(
    CMAKE_CXX_FLAGS 
    "${CMAKE_CXX_FLAGS} -stdlib=libstdc++"
)
set(
    CMAKE_EXE_LINKER_FLAGS 
    "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libstdc++"
)

project(TradingEngine LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(
    gtest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)

FetchContent_Declare(
    gbench
    GIT_REPOSITORY https://github.com/google/benchmark
    GIT_TAG v1.9.4
)

FetchContent_MakeAvailable(
    gtest
    gbench
)
# generated tests
function(create_sanitized_targets base_name sources)
    # Regular target
    add_executable(${base_name} ${sources})
    target_include_directories(${base_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    
    # ASan target
    add_executable(${base_name}_asan ${sources})
    target_include_directories(${base_name}_asan PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_compile_options(${base_name}_asan PRIVATE -fsanitize=address -fno-omit-frame-pointer -g)
    target_link_options(${base_name}_asan PRIVATE -fsanitize=address)
    
    # TSan target
    add_executable(${base_name}_tsan ${sources})
    target_include_directories(${base_name}_tsan PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_compile_options(${base_name}_tsan PRIVATE -fsanitize=thread -fno-omit-frame-pointer -g)
    target_link_options(${base_name}_tsan PRIVATE -fsanitize=thread)
endfunction()

# Create all your targets
create_sanitized_targets(server "src/main.cpp")
create_sanitized_targets(ringtest "tests/ringtest.cpp")
create_sanitized_targets(ringbench "tests/ringbench.cpp")

# Link libraries for test targets (all variants)
foreach(variant "" "_asan" "_tsan")
    target_link_libraries(ringtest${variant} gtest_main gtest)
    target_link_libraries(ringbench${variant} benchmark::benchmark)
endforeach()