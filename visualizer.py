#!/usr/bin/env python
"""Visualizes data on a cubic lattice

Built specifically to visualize the VTK files created by Morpheus
"""

import os
import glob
import sys
import argparse

import numpy as np
import vtk
from vtk.util import numpy_support as VN
from matplotlib import colors

found_im2movie = True
try:
    from im2movie import makeMovie
except ImportError:
    found_im2movie = False

__author__ = "Margriet Palm"
__copyright__ = "Copyright 2016"
__credits__ = "Margriet Palm"
__license__ = "MIT"
__version__ = "0.1"
__maintainer__ = "Margriet Palm"


# the vtkTimerCallback takes care of updating the visualzation
class vtkTimerCallback():
    def __init__(self, update_func, tmax=1, save=False):
        self.timer_count = 0
        self.update = update_func
        self.tmax = tmax
        self.update_actors = None
        self.save = save

    def execute(self, obj, event):
        iren = obj
        win = iren.GetRenderWindow()
        ren = win.GetRenderers().GetFirstRenderer()
        # remove all actors that will be updated
        for actor in self.update_actors:
            ren.RemoveActor(actor)
        # set t to correct value
        t = self.timer_count
        if self.timer_count >= self.tmax:
            t = self.timer_count % self.tmax
            self.save = False
        # get new actors
        actors = self.update(t, self.save)
        self.update_actors = actors
        self.timer_count += 1


class Visualizer3D():
    """ Create visualizer object


    :param simdir: path to folder containing vtk files
    :param steps: steps to visualize
    :param winsize: window size
    :param bg: background color
    :param bbox_color: bounding box wire frame color
    :param cam_props: dictionary with camera settings
    :param onthefly: read data on the fly instead of all at once
    """

    def __init__(self, simdir, steps=None, winsize=(800, 800), bg=(0, 0, 0), bbox_color=(1, 1, 1),
                 cam_props=None, onthefly=False, storeafterread=True):
        self.bbox_color = bbox_color
        self.cam_props = cam_props
        self.storeafterread = storeafterread
        # read data
        get_num = lambda fn: int(fn.split('_')[-1].replace('.vtk', ''))
        if steps is not None:
            self.files = {get_num(f) : f for f in glob.glob('{}/plot_*.vtk'.format(simdir)) if get_num(f) in steps}
        else:
            self.files = {get_num(f) : f for f in glob.glob('{}/plot_*.vtk'.format(simdir))}
        if not onthefly:
            self.data = {n : self._load_data(f) for n,f in self.files.iteritems()}
        else:
            self.data = {self.files.keys()[0] : self._load_data(self.files[self.files.keys()[0]])}
        # setup renderer
        self._set_renderer(winsize, bg)

    def _get_step(self,step):
        """ Retrieve vtk data for a specific step """
        if step in self.data:
            return self.data[step]
        else:
            if self.storeafterread:
                self.data[step] = self._load_data(self.files[step])
                return self.data[step]
            else:
                return self._load_data(self.files[step])

    def _set_renderer(self, winsize, bg):
        """ Set up vtk renderer """
        self.renderer = vtk.vtkRenderer()
        self.renderer.SetBackground(bg[0], bg[1], bg[2])
        self.renderWindow = vtk.vtkRenderWindow()
        self.renderWindow.AddRenderer(self.renderer);
        self.renderWindowInteractor = vtk.vtkRenderWindowInteractor()
        self.renderWindowInteractor.SetRenderWindow(self.renderWindow)
        self.renderWindow.SetSize(winsize[0], winsize[1])

    def get_actors(self, step, tau_list, tau_colors=None, tau_alpha=None, bbox=True):
        """
        Create actors for a list of cell types and add them to the renderer

        :param step: step to visualize
        :param tau_list: list of cell types
        :param tau_colors: list with color per cell type
        :param tau_alpha: list with opacity per cell type
        :param bbox: show bounding box

        :returns: list of actors with first the actors for tau_list followed by the bounding box (if applicable)
        """
        # set default colors and opacity when they are not specified
        if tau_colors is None:
            tau_colors = [(0.5, 0.5, 0.5) for tau in tau_list]
        if tau_alpha is None:
            tau_alpha = [1 for tau in tau_list]
        # get actors
        stepdata = self._get_step(step)
        if stepdata is None:
            return []
        else:
            actors = [self._get_actor_for_tau(stepdata, tau, tau_colors[i], tau_alpha[i]) for i, tau in enumerate(tau_list)]
            # get bounding box wire frame
            if bbox:
                actors.append(self._get_box_actor())
            # add actors to the renderer
            for actor in actors:
                self.renderer.AddActor(actor)
            return actors

    def _modify_cam(self):
        """ Modify the camera settings for the renderer.

        Available options:
            - position
            - focal point
            - pitch

        If position and focal point are not given, they will be taken
        from the camera in the renderer.

        :param renderer: vtk renderer
        :param cam_props: dictionary with options (see above) as keys and settings as values
        """
        old_cam = self.renderer.GetActiveCamera();
        cam = vtk.vtkCamera()
        if 'position' in self.cam_props:
            cam.SetPosition(self.cam_props['position'])
        else:
            cam.SetPosition(old_cam.GetPosition())
        if 'focal_point' in self.cam_props:
            cam.SetFocalPoint(self.cam_props['focal point'])
        else:
            cam.SetFocalPoint(old_cam.GetFocalPoint())
        if 'pitch' in self.cam_props:
            cam.Pitch(self.cam_props['pitch'])
        self.renderer.SetActiveCamera(cam)

    def _get_box_actor(self):
        """ Create and return actor for wire frame box of the simulation domain """
        (w, h, d) = self.data[self.data.keys()[0]].GetDimensions()
        imageData = vtk.vtkImageData()
        imageData.SetDimensions(2, 2, 2)
        imageData.SetSpacing(w, h, d)
        imageData.SetOrigin(0, 0, 0)
        mapper = vtk.vtkDataSetMapper()
        if vtk.VTK_MAJOR_VERSION <= 5:
            mapper.SetInput(imageData)
        else:
            mapper.SetInputData(imageData)
        actor = vtk.vtkActor()
        actor.SetMapper(mapper)
        actor.GetProperty().SetColor(self.bbox_color[0], self.bbox_color[1], self.bbox_color[2])
        actor.GetProperty().SetRepresentationToWireframe()
        return actor

    def _get_actor_for_tau(self, stepdata, show_tau, color=(0.5, 0.5, 0.5), opacity=1):
        """ Create actor for a cell type """
        if isinstance(color, basestring):
            # convert color to rgb string
            if color in colors.cnames:
                color = get_color(color)
            else:
                color = (0.5, 0.5, 0.5)
        dim = stepdata.GetDimensions()
        sigma = VN.vtk_to_numpy(stepdata.GetPointData().GetArray('cell.id'))
        sigma = sigma.reshape(dim, order='F')
        tau = VN.vtk_to_numpy(stepdata.GetPointData().GetArray('cell.type'))
        tau = tau.reshape(dim, order='F')
        show_idx = np.unique(sigma[tau == show_tau])
        points = vtk.vtkPoints()
        for s in show_idx:
            if s not in sigma:
                continue
            pix = np.column_stack(np.where(sigma == s))
            for p in pix:
                points.InsertNextPoint(p[0] - .5, p[1] - .5, p[2] - .5)
        polydata = vtk.vtkPolyData()
        polydata.SetPoints(points)
        sources = vtk.vtkCubeSource()
        sources.Update()
        glyph = vtk.vtkGlyph3D()
        if vtk.VTK_MAJOR_VERSION <= 5:
            glyph.SetInput(polydata)
        else:
            glyph.SetInputData(polydata)
        glyph.SetSourceConnection(sources.GetOutputPort())
        glyph.ScalingOff()
        glyph.Update()
        mapper = vtk.vtkPolyDataMapper()
        mapper.SetInputConnection(glyph.GetOutputPort())
        actor = vtk.vtkActor()
        actor.GetProperty().SetOpacity(opacity)
        actor.GetProperty().SetColor(color[0], color[1], color[2])
        actor.SetMapper(mapper)
        return actor

    def _load_data(self, fn):
        """ Load vtk files """
        reader = vtk.vtkStructuredPointsReader()
        reader.SetFileName(fn)
        reader.ReadAllScalarsOn()
        reader.Update()
        data = reader.GetOutput()
        if data.GetPointData().HasArray('cell.id') != 1:
            print "'cell.id' array missing from {} -> skip file".format(fn)
            return None
        if data.GetPointData().HasArray('cell.type') != 1:
            print "'cell.id' array missing from {} -> skip file".format(fn)
            return None
        return reader.GetOutput()

    def visualize(self, step, tau_list, show=False, save=False, impath=None, imprefix=None, bbox=True,
                  tau_alpha=None, tau_colors=None):
        """
        Visualize a given step.

        :param step: step to visualize
        :param tau_list: list of cell types
        :param show: initialize and start the render window after adding the actors to the renderer, should not be used for animations
        :param save: save view to png
        :param impath: path to store image
        :param imprefix: image prefix
        :param bbox: show bounding box
        :param tau_alpha: list with opacity per cell type
        :param tau_colors: list with color per cell type
        """
        self.renderWindow.SetWindowName('step ' + str(int(step)))
        actors = self.get_actors(step, tau_list, tau_colors, tau_alpha, bbox=bbox)
        self.renderWindow.Render()
        if self.cam_props is not None:
            self._modify_cam()
        if show:
            self.renderWindowInteractor.Initialize()
            self.renderWindowInteractor.Start()
        if save:
            w2i = vtk.vtkWindowToImageFilter()
            w2i.SetInput(self.renderWindow)
            w2i.Update()
            writer = vtk.vtkPNGWriter()
            writer.SetInputConnection(w2i.GetOutputPort())
            if imprefix is not None and imprefix.endswith('_'):
                imprefix = imprefix + '_'
            if imprefix is None:
                imprefix = ''
            if impath is None:
                impath = '.'
            writer.SetFileName('{}/{}{:03d}.png'.format(impath, imprefix, step))
            print 'save image {}/{}{:03d}.png'.format(impath, imprefix, step)
            writer.Write()
        return actors

    def animate(self, tau, tau_colors=None, tau_alpha=None, steps=None, save=False, impath=None, imprefix=None,
                fps=5, static_tau=None):
        """
        Animate simulation results

        :param tau: list of cell types
        :param tau_colors: list with color per cell type
        :param tau_alpha: list with opacity per cell type
        :param steps: steps (all steps are shown when not specified)
        :param save: save view to png
        :param impath: path to store image
        :param imprefix: image prefix
        :param fps: frames per second
        :param static_tau: static cell types that should not be updated during the animation
        """
        if (tau_colors is None) or (len(tau_colors) is not len(tau)):
            tau_colors = [(.5, .5, .5) for t in tau]
        if (tau_alpha is None) or (len(tau_alpha) is not len(tau)):
            tau_alpha = [1 for t in tau]
        if steps is None:
            steps = self.files.keys()
        steps.sort()
        self.renderWindowInteractor.Initialize()
        actors = self.visualize(steps[0], tau, show=False, save=False, bbox=True, tau_alpha=tau_alpha,
                                tau_colors=tau_colors)
        if static_tau is None:
            static_tau = []
        update_tau = [t for t in tau if t not in static_tau]
        update_colors = [tau_colors[i] for i, t in enumerate(tau) if t in update_tau]
        update_alpha = [tau_alpha[i] for i, t in enumerate(tau) if t in update_tau]
        update_func = lambda t, s: self.visualize(steps[t], update_tau, show=False, save=s, bbox=False,
                                               tau_alpha=update_alpha, tau_colors=update_colors, imprefix=imprefix)
        cb = vtkTimerCallback(update_func, len(steps), save)
        if len(actors) > 0:
            cb.update_actors = [actors[tau.index(t)] for t in tau if t not in static_tau]
        else:
            cb.update_actors = []
        self.renderWindowInteractor.AddObserver('TimerEvent', cb.execute)
        timerId = self.renderWindowInteractor.CreateRepeatingTimer(int(1000 / float(fps)))
        cb.timerId = timerId
        # start the interaction and timer
        self.renderWindowInteractor.Start()


def get_color(name):
    """ Get color for matplotlib color name """
    cc = colors.ColorConverter()
    if name in colors.cnames:
        return cc.to_rgb(name)
    else:
        return cc.to_rgb("grey")


def parse_args():
    parser = argparse.ArgumentParser()
    # parser.description("Animate 3D Morpheus simulations")
    parser.add_argument("-i", "--simdir", type=str, default="./", help="Simulation folder", required=True)
    parser.add_argument("-w", "--winsize", type=int, nargs=2, help="window size", default=(800, 800))
    parser.add_argument("-t", "--celltypes", type=int, nargs="*", help="cell types to animate", required=True)
    parser.add_argument("-c", "--colors", type=str, nargs="*", help="colors or the cell types")
    parser.add_argument("-a", "--alpha", type=float, nargs="*", help="opacity of the cell types")
    parser.add_argument("--static", type=int, nargs="*",
                        help="static cell types (will NOT be updated during animation)")
    parser.add_argument("--bboxcolor", type=float, nargs=3, default=(1, 1, 1), help="bounding box color")
    parser.add_argument("--bgcolor", type=float, nargs=3, default=(0, 0, 0), help="background color")
    parser.add_argument("--camposition", type=float, nargs=3, default=(-200, 200, 200), help="camera position")
    parser.add_argument("--camfocus", type=float, nargs=3, default=(100, 100, 50), help="camera focal point")
    parser.add_argument("--steps", type=int, nargs="*", help="steps to animate, all steps will be shown if this "
                                                             "is not specified")
    parser.add_argument("-f", "--fps", type=float, default=5, help="frames per second")
    parser.add_argument("-o", "--outdir", type=str, help="output directory")
    parser.add_argument("-p", "--imprefix", type=str, help="image prefix")
    parser.add_argument("-s", "--saveim", action="store_true", help="save images")
    parser.add_argument("-m", "--movie", action="store_true", help="make movie after closing the visualization window")
    parser.add_argument("--readall", action="store_true", help="read all data at once before the visualization starts")
    parser.add_argument("--savemem", action="store_true", help="reread vtk file every time it is used instead of "
                                                               "keeping it in memory")
    parser.add_argument("--win", action="store_true", help="make movie windows compatible")
    parser.add_argument("--mp4", action="store_true", help="make mp4 movie")
    return parser.parse_args()


def main():
    args = parse_args()
    # check if there is something to animate
    if not os.path.isdir(args.simdir):
        sys.exit("Could not find {}".format(args.simdir))
    elif len(glob.glob("{}/*.vtk".format(args.simdir))) == 0:
        sys.exit("No vtk files found in {}".format(args.simdir))

    # set colors and opacity
    if not args.colors:
        print "Cell color not specified - default to grey"
        args.colors = [get_color("grey") for t in args.celltypes]
    elif len(args.colors) == 1:
        args.colors = [get_color(args.colors) for t in args.celltypes]
    elif len(args.colors) < len(args.celltypes):
        print "Number of colors does not match number of cell types - default to grey"
        args.colors = [get_color("grey") for t in args.celltypes]
    else:
        args.colors = [get_color(c) for c in args.colors]
    if not args.alpha:
        print "Alpha values not specified - default to opaque objects"
        args.alpha = [1 for t in args.celltypes]
    elif len(args.alpha) == 1:
        args.alpha = [args.alpha for t in args.celltypes]
    elif len(args.alpha) < len(args.celltypes):
        print "Number of alpha values does not match number of cell types - default to opaque objects"
        args.alpha = [1 for t in args.celltypes]

    # set saving options
    if args.imprefix or args.outdir or args.movie:
        args.saveim = True
    if args.saveim:
        if not args.outdir:
            args.outdir = args.simdir
        if not os.path.isdir(args.outdir):
            print "Create output directory {}".format(args.outdir)
            os.makedirs(args.outdir)
        if not args.imprefix:
            args.imprefix = "frame"

    # set camera
    cam_props = {'position': args.camposition, 'focal point': args.camfocus}
    # create visualizer
    v = Visualizer3D(args.simdir, winsize=args.winsize, bg=args.bgcolor, bbox_color=args.bboxcolor,
                     cam_props=cam_props, onthefly=(not args.readall), storeafterread=(not args.savemem))
    # start animation
    v.animate(args.celltypes, tau_colors=args.colors, tau_alpha=args.alpha, steps=args.steps,
              save=args.saveim, impath=args.outdir, imprefix=args.imprefix, fps=args.fps, static_tau=args.static)
    # create and store movie
    if args.movie and found_im2movie:
        makeMovie(args.imprefix, 'png', args.imprefix, args.outdir, args.outdir, args.fps,
                  win=args.win, tomp4=args.mp4)
    elif not found_im2movie:
        print "WARNING: Movie generation is turned of because im2movie was not found"


if __name__ == "__main__":
    main()
