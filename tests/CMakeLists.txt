cmake_minimum_required(VERSION 3.1)
project(NodePhysics_test VERSION 1.0)

find_package(SofaFramework REQUIRED)
find_package(SofaCommon REQUIRED)
find_package(SofaTest REQUIRED)
find_package(SofaGTestMain REQUIRED)
find_package(SofaSimulation REQUIRED)
find_package(NodePhysics REQUIRED)

set(SOURCE_FILES
    ObjectLinkTest.cpp
    )

add_executable(NodePhysics_test ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC "${NodePhysics_INCLUDE_DIRS}")
target_link_libraries(${PROJECT_NAME} SofaTest SofaGTestMain SofaCore SofaSimulationGraph NodePhysics)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

