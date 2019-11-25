cmake_minimum_required(VERSION 3.1)
project(NodePhysics VERSION 1.0 LANGUAGES CXX)

set(${PROJECT_NAME}_MAJOR_VERSION 1)
set(${PROJECT_NAME}_MINOR_VERSION 0)
set(${PROJECT_NAME}_VERSION ${QMLUI_MAJOR_VERSION}.${QMLUI_MINOR_VERSION})

set (CMAKE_CXX_STANDARD 17)

find_package(SofaFramework REQUIRED)

set(HEADER_FILES
        src/NodePhysics/config.h
        src/NodePhysics/initNodePhysics.h
        src/NodePhysics/ObjectLink.h
    )
    
set(SOURCE_FILES
        src/NodePhysics/initNodePhysics.cpp
    )
    
set(EXTRA_FILES
        README.md
    )

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES} ${EXTRA_FILES})
target_link_libraries(${PROJECT_NAME} PUBLIC SofaHelper SofaBaseLinearSolver)
target_include_directories(${PROJECT_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>")
target_include_directories(${PROJECT_NAME} PUBLIC "$<INSTALL_INTERFACE:include>")

sofa_generate_package(
    NAME ${PROJECT_NAME}
    VERSION ${PROJECT_VERSION}
    TARGETS ${PROJECT_NAME}
    INCLUDE_SOURCE_DIR "src/NodePhysics"
    INCLUDE_INSTALL_DIR "NodePhysics"
    RELOCATABLE "plugins"
    )
  
set (NodePhysics_BUILD_TEST "ON")
if (NodePhysics_BUILD_TEST)
    add_subdirectory(tests)
endif (NodePhysics_BUILD_TEST)
