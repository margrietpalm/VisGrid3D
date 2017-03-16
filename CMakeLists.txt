cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

PROJECT(VisGrid3D)

set(CMAKE_CXX_STANDARD 11)
set(SOURCE_FILES VisGrid3D.cpp visualizer.cpp datareader.cpp)
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(VisGrid3D ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(VisGrid3D)



if(VTK_LIBRARIES)
    target_link_libraries(VisGrid3D ${VTK_LIBRARIES})
else()
    target_link_libraries(VisGrid3D vtkHybrid vtkWidgets)
endif()
