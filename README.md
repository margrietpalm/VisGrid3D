# Visualizing 3D grid data

## Todo list

- Add support for generating movies
- Add support for drawing surfaces instead of voxels
- Add support for drawing colored boundaries

## Installation

1. Install dependencies 
    - [VTK](http://www.vtk.org/)
    - [CMake 3.1 or higher](https://cmake.org/)
2. Install using cmake:
```
mkdir build
cd build
cmake ../
make
```


## Generating VTK files in Morpheus
Morpheus can generate VTK files with the *VtkPlotter* analysis plugin. Set up this plugin to write ascii files and
 to store the cell id and cell type:

```
<VtkPlotter mode="ascii" time-step="10">
	<Channel symbol-ref="cell.id"/>
	<Channel symbol-ref="cell.type"/>
</VtkPlotter>
```

Note that VTK files can become quite large. So keep the number of output files low during testing, or remove them.


## Usage

## Acknowledgements
- We thank the developers of the [cxxopts](https://github.com/jarro2783/cxxopts) library which we used for parsing command line arguments.
- We thank the developers of [matplotlib](http://matplotlib.org/) from which we extracted the colormap stored in colors.csv.