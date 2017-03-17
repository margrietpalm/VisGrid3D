//
// Created by mpalm on 15/03/17.
//

#ifndef VISGRID3D_VISUALIZER_H
#define VISGRID3D_VISUALIZER_H

#include <vector>
#include <utility>
#include <vtkActor.h>
#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkUnsignedCharArray.h>

#include "datareader.h"
#include "colortable.h"
#include "colormap.h"


class Visualizer {
 public:
  Visualizer(){};
  Visualizer(DataReader *_reader);
  void InitRenderer();
  void ModifyCamera();
  void Animate(std::vector<int> taulist, std::vector<int> steps, std::vector<int> static_tau,
                 std::vector<color> colors, std::vector<double> opacity, bool save,
                 std::vector<std::string> color_by, std::vector<ColorMap *> cms);
  std::vector<vtkSmartPointer<vtkActor> > VisualizeStep(int step,
                                                          std::vector<int> taulist,
                                                          bool show,
                                                          std::vector<color> tau_colors,
                                                          std::vector<double> tau_opacity,
                                                          bool save,
                                                          std::vector<std::string> color_by,
                                                          std::vector<ColorMap *> cms);
  color bgcolor, bbcolor;
  std::vector<int> winsize;
  double fps;
  double camposition[3];
  double camfocus[3];
  double campitch;
  int numlen;
  std::string prefix;
  std::string impath;

 private:
  vtkSmartPointer<vtkActor>
  GetActorForType(stepdata data, int tau, color c, double opacity, std::string color_by, ColorMap *cm);
  vtkSmartPointer<vtkActor> GetActorForBox(stepdata data);
  vtkSmartPointer<vtkPoints> GetPointsForTau(stepdata data, int tau);
  std::pair<vtkSmartPointer<vtkPoints>, vtkSmartPointer<vtkUnsignedCharArray>>
  GetPointsAndColorsForTau(stepdata data, int tau, std::string color_by, ColorMap *cm);

  std::string GetImNameForStep(int step);

  vtkSmartPointer<vtkRenderer> renderer;
  vtkSmartPointer<vtkRenderWindow> renderWindow;
  vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor;
  DataReader *reader;
};

#endif //VISGRID3D_VISUALIZER_H
