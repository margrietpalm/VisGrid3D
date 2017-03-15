cmake_minimum_required(VERSION 2.8)

PROJECT(VisGrid3D)

set(CMAKE_CXX_STANDARD 11)
set(SOURCE_FILES VisGrid3D.cpp)
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

FIND_PACKAGE(Boost COMPONENTS program_options REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

add_executable(VisGrid3D ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(VisGrid3D ${Boost_LIBRARIES})



if(VTK_LIBRARIES)
    target_link_libraries(VisGrid3D ${VTK_LIBRARIES})
else()
    target_link_libraries(VisGrid3D vtkHybrid vtkWidgets)
endif()
