cmake_minimum_required(VERSION 3.14)
project(ComponentData_POC)

set(${PROJECT_NAME}_MAJOR_VERSION 1)
set(${PROJECT_NAME}_MINOR_VERSION 0)
set(${PROJECT_NAME}_VERSION ${QMLUI_MAJOR_VERSION}.${QMLUI_MINOR_VERSION})

set (CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES
  # All required files to modify to implement a Link-less version of the components used by the packaged scene ConstantForceField.py 
    src/${PROJECT_NAME}/DataComponent.cpp
  
    src/${PROJECT_NAME}/MyUniformMass.cpp
    src/${PROJECT_NAME}/MyMass.cpp
    src/${PROJECT_NAME}/MyConstantForceField.cpp
    src/${PROJECT_NAME}/MyForceField.cpp

    src/${PROJECT_NAME}/MyMechanicalObject.cpp

    src/${PROJECT_NAME}/MyRigidMapping.cpp
    src/${PROJECT_NAME}/MyMapping.cpp

    src/${PROJECT_NAME}/initComponentData_POC.cpp
)

set(HEADER_FILES
    src/${PROJECT_NAME}/DataComponent.h
  
    src/${PROJECT_NAME}/MyUniformMass.h
    src/${PROJECT_NAME}/MyMass.h
    src/${PROJECT_NAME}/MyConstantForceField.h
    src/${PROJECT_NAME}/MyForceField.h

    src/${PROJECT_NAME}/MyMechanicalObject.h

    src/${PROJECT_NAME}/MyRigidMapping.h
    src/${PROJECT_NAME}/MyMapping.h

    src/${PROJECT_NAME}/initComponentData_POC.h
)

find_package(SofaFramework REQUIRED)
find_package(SofaBase REQUIRED)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES} ${QML_TEMPLATES})

target_link_libraries(${PROJECT_NAME} PUBLIC SofaCore)

target_include_directories(${PROJECT_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>")
target_include_directories(${PROJECT_NAME} PUBLIC "$<INSTALL_INTERFACE:include/${PROJECT_NAME}>")

target_compile_definitions(${PROJECT_NAME} PRIVATE "-DSOFA_BUILD_COMPONENTDATA_POC")

## Install rules for the library; CMake package configurations files
sofa_generate_package(NAME ${PROJECT_NAME} VERSION ${${PROJECT_NAME}_VERSION} TARGETS ${PROJECT_NAME} INCLUDE_INSTALL_DIR ${PROJECT_NAME} INCLUDE_SOURCE_DIR src/${PROJECT_NAME})
