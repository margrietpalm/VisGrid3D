cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
project(VisGrid3D)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        src/colortable.h
        src/cxxopts.hpp
        src/datareader.cpp
        src/datareader.h
        src/VisGrid3D.cpp
        src/visualizer.cpp
        src/visualizer.h
        src/colormap.h)

find_package(VTK REQUIRED)
find_package(ImageMagick REQUIRED)
include(${VTK_USE_FILE})
message(${VTK_LIBRARIES})


add_executable(VisGrid3D ${SOURCE_FILES})

if(VTK_LIBRARIES)
    target_link_libraries(VisGrid3D ${VTK_LIBRARIES})
else()
    target_link_libraries(VisGrid3D vtkHybrid vtkWidgets)
endif()
