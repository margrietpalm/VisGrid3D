cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

PROJECT(VisGrid3D)

set(CMAKE_CXX_STANDARD 11)
set(SOURCE_FILES src/VisGrid3D.cpp src/visualizer.cpp src/datareader.cpp src/colormap.h src/movie.h)
find_package(VTK REQUIRED)
find_package(ImageMagick REQUIRED)
include(${VTK_USE_FILE})

add_executable(VisGrid3D ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(VisGrid3D)

if(VTK_LIBRARIES)
    target_link_libraries(VisGrid3D ${VTK_LIBRARIES})
else()
    target_link_libraries(VisGrid3D vtkHybrid vtkWidgets)
endif()

#install(TARGETS VisGrid3D RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX} DESTINATION bin)
