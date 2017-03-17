# Visualizing 3D grid data

The *old* Python based VisGrid3D can be found [here](https://github.com/margrietpalm/VisGrid3D-python)

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

To animate the results you must at least specify the folder containing the vtk files (`-i`) and the cell types
to animate (`-t`). All other command line arguments are optimal and use defaults if not specified. When an argument
has multiple values, they must be presented in a comma-separated list.

### Examples


- Plot cell types 0 and 2 for results in `morpheus/3d_migration_138/`:

```VisGrid3D -i morpheus/3d_migration_138/ -t 0,2```

- Colors and opacity can be defined per cell type:

```VisGrid3D -i morpheus/3d_migration_138/ -t 0,2 -c red,grey -a 1,0.1```

- Static (frozen) cell types can be defined such that they are only rendered once:

```VisGrid3D -i morpheus/3d_migration_138/ -t 0,2 -c red,grey -a 1,0.1 --static 2```

- Save snapshots:

```visgrid3D -i morpheus/3d_migration_138/ -t 0,2 -c red,grey -a 1,0.1 --static 2 -s```



### Help

```
VisGrid3D -h
 - example command line options
Usage:
  ./VisGrid3D [OPTION...]

  -h, --help           Print help
  -i, --simdir arg     Folder containing vtk files
  -t, --types arg      Comma-separated list of cell types to visualize
  -f, --fields arg     Comma-separated list of fields (stored in the vtk)
                       used to color the cells (use none to skip a cell type)
  -c, --colors arg     Comma-separated list of colors associated to the cell
                       types
  -a, --alpha arg      Comma-separated list of alpha-values associated to the
                       cell types
      --static arg     Comma-separated list of static cell types
      --steps arg      Comma-separated list of time steps to visualize
  -W, --width arg      visualization width (default: 800)
  -H, --height arg     visualization height (default: 800)
      --bgcolor arg    background color (default: black)
      --bboxcolor arg  bounding box color (default: white)
      --campos arg     camera position
      --camfocus arg   camera focus
      --campitch arg   camera pitch
      --fps arg        frame rate
  -o, --outdir arg     Folder to write images to
  -s, --save           Save images
      --prefix arg     Prefix for image names
  -m, --colormap arg   File with colormap to be used with the fields
      --frange arg     min and max value of the field
      --fmax arg       Comma-seperated list with max value for each field
      --fmin arg       Comma-seperated list with min value for each field
      --showcolors     show available colors

## Acknowledgements
- We thank the developers of the [cxxopts](https://github.com/jarro2783/cxxopts) library which we used for parsing command line arguments.
- We thank the developers of [matplotlib](http://matplotlib.org/) from which we extracted the colortable that maps color names to rgb values (colors.csv).
