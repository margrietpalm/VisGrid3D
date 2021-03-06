# Visualizing 3D grid data

* [Installation](#installation)
  * [Generic instructions](#generic-instructions)
  * [Ubuntu 16.04](#ubuntu-1604)
  * [Docker](#docker)
* [Generating VTK files in Morpheus](#generating-vtk-files-in-morpheus)
* [Usage](#usage)
  * [Examples](#examples)


## Installation

### Generic instructions

1. Install dependencies
    - [boost](http://www.boost.org/)
    - [VTK](http://www.vtk.org/)
    - [CMake 3.1 or higher](https://cmake.org/)
2. Install using cmake:
```
mkdir build
cd build
cmake ../
make
```

Note, on OSX you may need to use clang++ instead of gcc (see [issue #3](https://github.com/margrietpalm/VisGrid3D/issues/3)).


### Ubuntu 16.04
1. Install dependencies: `sudo apt-get install cmake git libvtk6-dev libboost-filesystem1.58-dev libboost-iostreams1.58-dev libproj-dev`
2. Clone repository: `git clone https://github.com/margrietpalm/VisGrid3D.git`
3. Make build directory in the source folder, i.e.: `cd VisGrid3D && mkdir build`
4. From the build directory, configure build with cmake, i.e.: `cd build && cmake ../`
5. Build VisGrid3d: `build`


### Docker
After cloning VisGrid3D, the Docker container can be build with the local dockerfile: ```docker build -t visgrid3d . ```. And then VisGrid3D can be used from inside the containter:
```
xhost +local:root
docker run -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix:rw -v $PWD:/data/ visgrid3d ...
```
Besides running the VisGrid3D in the Docker container, this does the following:
* Mount a local drive in the container to read from and write to the local filesystem.
* Share the local X11 socket with the container so the VTK window works (only tested on Linux).

Alternatively, the image from DockerHub may be used:
```
xhost +local:root
docker run -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix:rw -v $PWD:/data/ margrietpalm/visgrid3d ...
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

- Repeat above off screen and save results:

```VisGrid3D -i morpheus/3d_migration_138/ -t 0,2 -s -q```

- Colors and opacity can be defined per cell type:

```VisGrid3D -i morpheus/3d_migration_138/ -t 0,2 -c red,grey -a 1,0.1```

- Static (frozen) cell types can be defined such that they are only rendered once:

```VisGrid3D -i morpheus/3d_migration_138/ -t 0,2 -c red,grey -a 1,0.1 --static 2```

- Save snapshots:

```VisGrid3D -i morpheus/3d_migration_138/ -t 0,2 -c red,grey -a 1,0.1 --static 2 -s```

- Map a field, named 'act' on the cell (please note that I changed the order of amoeba and medium in Protrusion_3D example):

```VisGrid3D -i ~/morpheus/Example-Protrusion_616/ -t 1 --campos 500,500,400 --camfocus 100,100,100 --steps 5 -f act --fmin 0 --fmax 150```



### Help

```
VisGrid3D -h
 - example command line options
Usage:
  ./VisGrid3D [OPTION...]

  -h, --help            Print help
  -i, --simdir arg      Folder containing vtk files
  -t, --types arg       Comma-separated list of cell types to visualize
  -f, --fields arg      Comma-separated list of fields (stored in the vtk)
                        used to color the cells (use none to skip a cell type)
  -c, --colors arg      Comma-separated list of colors associated to the cell
                        types
  -a, --alpha arg       Comma-separated list of alpha-values associated to
                        the cell types
      --static arg      Comma-separated list of static cell types
      --steps arg       Comma-separated list of time steps to visualize
  -W, --width arg       visualization width (default: 800)
  -H, --height arg      visualization height (default: 800)
      --bgcolor arg     background color (default: black)
      --bboxcolor arg   bounding box color (default: white)
      --campos arg      camera position
      --camfocus arg    camera focus
      --campitch arg    camera pitch
      --camroll arg     camera roll
      --camazimuth arg  camera aximuth
      --fps arg         frame rate
  -o, --outdir arg      Folder to write images to
  -s, --save            Save images
      --prefix arg      Prefix for image names
  -m, --colormap arg    File with colormap to be used with the fields
      --fmax arg        Comma-seperated list with max value for each field
      --fmin arg        Comma-seperated list with min value for each field
      --xmin arg        color boundary at xmin
      --xmax arg        color boundary at xmax
      --ymin arg        color boundary at ymin
      --ymax arg        color boundary at ymax
      --zmin arg        color boundary at zmin
      --zmax arg        color boundary at zmax
      --showcolors      show available colors
  -l, --loop            Loop visualization
  -q, --quiet           Hide visualization windows
      --clean           Remove existing content of outdir
  -z, --gzip            Use gzipped vtk files

```


## Acknowledgements
- We thank the developers of the [cxxopts](https://github.com/jarro2783/cxxopts) library which we used for parsing command line arguments.
- We thank the developers of [matplotlib](http://matplotlib.org/) from which we extracted the colortable that maps color names to rgb values (colors.csv).
