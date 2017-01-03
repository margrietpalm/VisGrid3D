# Visualizing 3D grid data

## Installation

### Dependencies
- [Python 2.7](https://www.python.org/download/releases/2.7/)
- [Numpy](http://www.numpy.org/)
- [Matplotlib](http://matplotlib.org/)
- [VTK with Python bindings](http://www.vtk.org/)
- [im2movie](https://github.com/margrietpalm/im2movie) installed as library in the python path (only needed for creating movies)

### Set up as command line program
1. Make script executable:
    ```chmod +x visualizer.py```
2. Create a link to `visualizer.py` in a folder in your `PATH`. For example, when you have a `bin` folder in your home
 that is in your `PATH`: `ln -s /path/to/VisGrid3D/visualizer.py /home/USERNAME/bin/visgrid3D`
3. Now you can run the script with `visgrid3D`

### Python library
Place `VisGrid3D` folder in the python path and have a look at the documentation.


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
to animate (`-t`). All other command line arguments are optimal and use defaults if not specify.

### Examples

- Plot cell types 0 and 2 for results in `morphesu/3d_migration_138/`:

```visgrid3D -i morpheus/3d_migration_138/ -t 0 2```

- Colors and opacity can be defined per cell type:

```visgrid3D -i morpheus/3d_migration_138/ -t 0 2 -c red grey -a 1 0.1```

- Static (frozen) cell types can be defined such that they are only rendered once:

```visgrid3D -i morpheus/3d_migration_138/ -t 0 2 -c red grey -a 1 0.1 --static 2```

- Save snapshots and make movie from those snapshots when the visualization windows is closed

```visgrid3D -i morpheus/3d_migration_138/ -t 0 2 -c red grey -a 1 0.1 --static 2 -s -m```


### Help

```
visgrid3D -h
usage: animate_morpheus_3D [-h] -i SIMDIR [-w WINSIZE WINSIZE] -t
                           [CELLTYPES [CELLTYPES ...]]
                           [-c [COLORS [COLORS ...]]] [-a [ALPHA [ALPHA ...]]]
                           [--static [STATIC [STATIC ...]]]
                           [--bboxcolor BBOXCOLOR BBOXCOLOR BBOXCOLOR]
                           [--bgcolor BGCOLOR BGCOLOR BGCOLOR]
                           [--camposition CAMPOSITION CAMPOSITION CAMPOSITION]
                           [--camfocus CAMFOCUS CAMFOCUS CAMFOCUS]
                           [--steps [STEPS [STEPS ...]]] [-f FPS] [-o OUTDIR]
                           [-p IMPREFIX] [-s] [-m] [--win] [--mp4]

optional arguments:
  -h, --help            show this help message and exit
  -i SIMDIR, --simdir SIMDIR
                        Simulation folder
  -w WINSIZE WINSIZE, --winsize WINSIZE WINSIZE
                        window size
  -t [CELLTYPES [CELLTYPES ...]], --celltypes [CELLTYPES [CELLTYPES ...]]
                        cell types to animate
  -c [COLORS [COLORS ...]], --colors [COLORS [COLORS ...]]
                        colors or the cell types
  -a [ALPHA [ALPHA ...]], --alpha [ALPHA [ALPHA ...]]
                        opacity of the cell types
  --static [STATIC [STATIC ...]]
                        static cell types (will NOT be updated during
                        animation)
  --bboxcolor BBOXCOLOR BBOXCOLOR BBOXCOLOR
                        bounding box color
  --bgcolor BGCOLOR BGCOLOR BGCOLOR
                        background color
  --camposition CAMPOSITION CAMPOSITION CAMPOSITION
                        camera position
  --camfocus CAMFOCUS CAMFOCUS CAMFOCUS
                        camera focal point
  --steps [STEPS [STEPS ...]]
                        steps to animate, all steps will be shown if this is
                        not specified
  -f FPS, --fps FPS     frames per second
  -o OUTDIR, --outdir OUTDIR
                        output directory
  -p IMPREFIX, --imprefix IMPREFIX
                        image prefix
  -s, --saveim          save images
  -m, --movie           make movie
  --readall             read all data at once before the visualization starts
  --savemem             reread vtk file every time it is used instead of
                        keeping it in memory  
  --win                 make movie windows compatible
  --mp4                 make mp4 movie
```
