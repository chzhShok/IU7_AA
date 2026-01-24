cmake_minimum_required(VERSION 3.16)
project(lab05_pipeline CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-O3 -Wall -Wextra -Wpedantic)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(lab05_pipeline
        src/PipelineMain.cpp
        src/Graph.cpp
        src/DijkstraSeq.cpp
        src/DijkstraPar.cpp
)

add_executable(lab05_sequential
        src/Main.cpp
        src/Graph.cpp
        src/DijkstraSeq.cpp
        src/DijkstraPar.cpp
)

if(APPLE)
    target_link_libraries(lab05_pipeline PRIVATE pthread)
    target_link_libraries(lab05_sequential PRIVATE pthread)
endif()
