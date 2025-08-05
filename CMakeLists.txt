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

file(GLOB_RECURSE HEADERS include/*.h)
file(GLOB_RECURSE SOURCES src/*.cpp)

add_library(
    TradingEngineLib
    ${HEADERS}
    ${SOURCES}
)

target_include_directories(
    TradingEngineLib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(
    TradingEngineLib
    PUBLIC
        benchmark::benchmark    
)

# generated tests
function(create_sanitized_targets base_name sources)
    # regular target
    add_executable(${base_name} ${sources})
    target_link_libraries(${base_name} TradingEngineLib)

    # compiler optimisations
    target_compile_options(${base_name} PRIVATE 
        -O3 -march=native -mtune=native -DNDEBUG 
        -flto -funroll-loops -fvectorize -fslp-vectorize 
        -finline-functions
    )
    target_link_options(${base_name} PRIVATE -flto -fuse-ld=lld)
    
    # ASan target
    add_executable(${base_name}_asan ${sources})
    target_link_libraries(${base_name}_asan TradingEngineLib)
    target_compile_options(${base_name}_asan PRIVATE -fsanitize=address -fno-omit-frame-pointer -g)
    target_link_options(${base_name}_asan PRIVATE -fsanitize=address)
    
    # TSan target
    add_executable(${base_name}_tsan ${sources})
    target_link_libraries(${base_name}_tsan TradingEngineLib)
    target_compile_options(${base_name}_tsan PRIVATE -fsanitize=thread -fno-omit-frame-pointer -g)
    target_link_options(${base_name}_tsan PRIVATE -fsanitize=thread)
endfunction()

# create targets
create_sanitized_targets(ringbench "tests/ringbench.cpp")