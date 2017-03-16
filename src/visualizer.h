//
// Created by mpalm on 15/03/17.
//

#ifndef VISGRID3D_VISUALIZER_H
#define VISGRID3D_VISUALIZER_H

#include <vector>

#include <vtkActor.h>
#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

#include "datareader.h"
#include "colormap.h"



class Visualizer {
 public:
  Visualizer();
  Visualizer(DataReader * _reader);
  void InitRenderer();
  void ModifyCamera();
  void Animate(std::vector<int> taulist,std::vector<int> steps, std::vector<int> static_tau,
               std::vector<color> colors, std::vector<double> opacity);
  std::vector<vtkSmartPointer <vtkActor> > VisualizeStep(int step, std::vector<int> taulist, bool show);
  std::vector<vtkSmartPointer <vtkActor> > VisualizeStep(int step, std::vector<int> taulist);
  std::vector<vtkSmartPointer <vtkActor> > VisualizeStep(int step,std::vector<int> taulist,
                                                         bool show,std::vector<color> tau_colors,
                                                         std::vector<double> tau_opacity);
  color bgcolor, bbcolor;
  std::vector<int> winsize;
  double fps;
  double camposition [3];
  double camfocus [3];
//  double * camposition;
//  double * camfocus;
  double campitch;

 private:
  vtkSmartPointer<vtkActor> GetActorForTau(){};
  vtkSmartPointer<vtkActor> GetActorForBBox(){};
  vtkSmartPointer<vtkActor> GetActorForBnd(){};
  vtkSmartPointer<vtkActor> GetActorForType(stepdata data, int tau, color c, double opacity);
  vtkSmartPointer<vtkActor> GetActorForType(stepdata data, int tau);
  vtkSmartPointer<vtkActor> GetActerForBBox(stepdata data);
  vtkSmartPointer<vtkPoints> GetPointsForTau(stepdata data, int tau);

  vtkSmartPointer<vtkRenderer> renderer;
  vtkSmartPointer<vtkRenderWindow> renderWindow;
  vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor;
  DataReader * reader;
};

#endif //VISGRID3D_VISUALIZER_H
