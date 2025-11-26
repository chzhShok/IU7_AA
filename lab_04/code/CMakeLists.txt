cmake_minimum_required(VERSION 3.16)
project(lab04_graph_sp CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-O3 -Wall -Wextra -Wpedantic)

include_directories(${CMAKE_SOURCE_DIR}/include)

# Основная программа
add_executable(lab04
        src/Main.cpp
        src/Graph.cpp
        src/DijkstraSeq.cpp
        src/DijkstraPar.cpp
        include/Config.h
        src/JsonResultBuilder.cpp
        src/ArgsParser.cpp
        src/Experiments.cpp
        include/Experiments.h
)

# Программа для экспериментов
add_executable(lab04_experiments
        src/Main.cpp
        src/Experiments.cpp
        src/Graph.cpp
        src/DijkstraSeq.cpp
        src/DijkstraPar.cpp
        src/JsonResultBuilder.cpp
        src/ArgsParser.cpp
        include/Experiments.h
)

# Tests
enable_testing()
add_executable(lab04_tests
        tests/test_main.cpp
        src/Graph.cpp
        src/DijkstraSeq.cpp
        src/DijkstraPar.cpp
        include/Config.h
        src/JsonResultBuilder.cpp
        src/ArgsParser.cpp
        include/Experiments.h
)

add_test(NAME unit COMMAND lab04_tests)

# On macOS, link pthread explicitly for std::thread if needed
if(APPLE)
    target_link_libraries(lab04 PRIVATE pthread)
    target_link_libraries(lab04_experiments PRIVATE pthread)
    target_link_libraries(lab04_tests PRIVATE pthread)
endif()
