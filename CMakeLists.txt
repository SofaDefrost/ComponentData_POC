cmake_minimum_required(VERSION 3.1)
project(NodePhysics VERSION 1.0 LANGUAGES CXX)

find_package(SofaFramework REQUIRED)

set(HEADER_FILES
        src/NodePhysics/MechanicalObject.h        
        src/NodePhysics/MechanicalObject.inl        
    )
    
set(SOURCE_FILES
        src/NodePhysics/MechanicalObject.cpp
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
