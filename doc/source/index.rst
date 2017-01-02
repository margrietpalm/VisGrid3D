.. VisGrid3D documentation master file, created by
   sphinx-quickstart on Mon Jan  2 13:38:22 2017.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to VisGrid3D's documentation!
=====================================

Example
==============

.. code-block:: python

   from VisGrid3D import *

   simdir = 'morpheus/simulation_name_10/'
   # only show pixels for cells of type 0 and 2
   celltypes = [0,2]
   # color for each type
   colors = [(1,0,0),(0,0,1)]
   # alpha (opacity) for each type
   alpha = [1,0.1]
   # background color
   bgcolor = (0,0,0)
   # color of bounding box wire frame
   bboxcolor = (1,1,1)

   # create visualizer
   v = Visualizer3D(simdir, bg=bgcolor, bbox_color=bboxcolor)

   # start animation
   v.animate(celltypes, tau_colors=colors, tau_alpha=alpha, fps=5, static_tau=[2])



Visualizer
==============

.. autoclass:: VisGrid3D.Visualizer3D
   :members:
   :undoc-members:



