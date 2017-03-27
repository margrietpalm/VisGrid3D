//
// Created by mpalm on 15/03/17.
//

// TODO: Add support for drawing surfaces instead of voxels
// TODO: Add support for drawing colored boundaries

#include "visualizer.h"
#include <sstream>      // std::stringstream
#include <algorithm>

#include <vtkStructuredPoints.h>
#include <vtkDataSetMapper.h>
#include <vtkPolyData.h>
#include <vtkCubeSource.h>
#include <vtkGlyph3D.h>
#include <vtkPolyDataMapper.h>
#include <vtkCommand.h>
#include <vtkProperty.h>
#include <vtkRendererCollection.h>
#include <vtkCamera.h>
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>
#include <vtkPointData.h>
#include <vtkLookupTable.h>
#include <vtkCellArray.h>
#include <vtkPolygon.h>
#include <vtkPoints.h>

// For compatibility with new VTK generic data arrays
#ifdef vtkGenericDataArray_h
#define InsertNextTupleValue InsertNextTypedTuple
#endif


class vtkTimerCallback: public vtkCommand {
 private:
  int TimerCount;

 public:
  Visualizer *v;
  std::vector<int> taulist;
  std::vector<double> opacity;
  std::vector<color> colors;
  std::vector<std::string> color_by;
  std::vector<vtkSmartPointer<vtkActor> > update_actors;
  std::map<int, std::vector<vtkSmartPointer<vtkActor> > > used_actors;
  std::vector<int> steps;
  std::vector<ColorMap *> cms;
  bool loop;
  int tmax;
  bool save;

  static vtkTimerCallback *New() {
    vtkTimerCallback *cb = new vtkTimerCallback;
    cb->TimerCount = 0;
    return cb;
  }

  virtual void Execute(vtkObject *caller, unsigned long eventId, void *vtkNotUsed(callData)) {
    vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::SafeDownCast(caller);
    if (this->TimerCount == tmax) {
      if (this->loop)
        this->TimerCount = 0;
      else {
        iren->DestroyTimer();
        iren->GetRenderWindow()->Finalize();

        // Stop the interactor
        iren->TerminateApp();
        std::cout << "Closing window..." << std::endl;
      }
    }
    vtkRenderWindow *win = iren->GetRenderWindow();
    vtkRenderer *ren = win->GetRenderers()->GetFirstRenderer();
    std::map<std::string, color> planes;
    for (auto actor : update_actors) { ren->RemoveActor(actor); }
    if (used_actors.find(TimerCount) == used_actors.end()) {
      update_actors = v->VisualizeStep(steps[TimerCount],
                                       taulist,
                                       false,
                                       colors,
                                       opacity,
                                       save,
                                       color_by,
                                       cms, planes);
      if (loop)
        used_actors[steps[TimerCount]] = update_actors;
    } else {
      std::stringstream title;
      title << "step " << steps[TimerCount];
      win->SetWindowName(title.str().c_str());
      for (auto actor : used_actors[steps[TimerCount]]) {
        ren->AddActor(actor);
      }
      win->Render();
      update_actors = used_actors[TimerCount];
    }
    ++this->TimerCount;
  }

};

Visualizer::Visualizer(DataReader *_reader) {
  reader = _reader;
  bgcolor = {0, 0, 0};
  bbcolor = {1, 1, 1};
  winsize = {800, 800};
  fps = 1;
  campitch = 0;
  numlen = 6;
  prefix = "im";
  impath = "./";
}

void Visualizer::InitRenderer(bool onscreen) {
  renderer = vtkSmartPointer<vtkRenderer>::New();
  renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow->AddRenderer(renderer);
  renderer->SetBackground(bgcolor.r, bgcolor.g, bgcolor.b);
  renderWindow->SetSize(winsize[0], winsize[1]);
  vtkSmartPointer<vtkCamera> cam = renderer->GetActiveCamera();
  if (onscreen) {
    renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderWindowInteractor->SetRenderWindow(renderWindow);
  }
  else{
    renderWindow->SetOffScreenRendering( 1 );
  }
}

void Visualizer::ModifyCamera() {
  vtkSmartPointer<vtkCamera> cam = vtkSmartPointer<vtkCamera>::New();
  cam->SetPosition(camposition);
  cam->SetFocalPoint(camfocus);
  cam->Pitch(campitch);
  renderer->SetActiveCamera(cam);
}

vtkSmartPointer<vtkActor> Visualizer::GetPlane(std::vector<std::vector<int>> corners, color planecolor){
  vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
  if (corners.size() != 4){
    return actor;
  }
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  for (auto pt : corners){
    if (pt.size() != 3){ return actor;}
    points->InsertNextPoint(pt[0],pt[1],pt[2]);
  }
  vtkSmartPointer<vtkPolygon> polygon = vtkSmartPointer<vtkPolygon>::New();
  polygon->GetPointIds()->SetNumberOfIds(4);
  polygon->GetPointIds()->SetId(0, 0);
  polygon->GetPointIds()->SetId(1, 1);
  polygon->GetPointIds()->SetId(2, 2);
  polygon->GetPointIds()->SetId(3, 3);

  vtkSmartPointer<vtkCellArray> polygons = vtkSmartPointer<vtkCellArray>::New();
  polygons->InsertNextCell(polygon);

  vtkSmartPointer<vtkPolyData> polygonPolyData = vtkSmartPointer<vtkPolyData>::New();
  polygonPolyData->SetPoints(points);
  polygonPolyData->SetPolys(polygons);

  vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New();
  #if VTK_MAJOR_VERSION <= 5
    mapper->SetInput(polygonPolyData);
  #else
    mapper->SetInputData(polygonPolyData);
  #endif

  actor->SetMapper(mapper);
  actor->GetProperty()->SetColor(planecolor.r,planecolor.g,planecolor.g);
  return actor;
}

std::vector< vtkSmartPointer<vtkActor> > Visualizer::GetBoundaryPlanes(stepdata data, std::map<std::string,color> planes){
  std::vector< vtkSmartPointer<vtkActor> > actors;
  int *dim = data.sp->GetDimensions();
  int w = dim[0];
  int h = dim[1];
  int d = dim[2];

  if (planes.find("xmin") != planes.end())
    actors.push_back(GetPlane({{0,0,0},{0,h,0},{0,h,d},{0,0,d}},planes["xmin"]));
  if (planes.find("xmax") != planes.end())
    actors.push_back(GetPlane({{w,0,0},{w,h,0},{w,h,d},{w,0,d}},planes["xmax"]));
  if (planes.find("ymin") != planes.end())
    actors.push_back(GetPlane({{0,0,0},{w,0,0},{w,0,d},{0,0,d}},planes["ymin"]));
  if (planes.find("ymax") != planes.end())
    actors.push_back(GetPlane({{0,h,0},{w,h,0},{w,h,d},{0,h,d}},planes["ymin"]));
  if (planes.find("zmin") != planes.end())
    actors.push_back(GetPlane({{0,0,0},{w,0,0},{w,h,0},{0,h,0}},planes["zmin"]));
  if (planes.find("zmax") != planes.end())
    actors.push_back(GetPlane({{0,0,d},{w,0,d},{w,h,d},{0,h,d}},planes["zmin"]));
  return actors;
}


vtkSmartPointer<vtkActor> Visualizer::GetActorForBox(stepdata data) {
  int *dim = data.sp->GetDimensions();
  vtkSmartPointer<vtkImageData> boxdata = vtkSmartPointer<vtkImageData>::New();
  boxdata->SetDimensions(2, 2, 2);
  boxdata->SetSpacing(dim[0], dim[1], dim[2]);
  boxdata->SetOrigin(0, 0, 0);
  vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New();
#if VTK_MAJOR_VERSION <= 5
  mapper->SetInput(boxdata);
#else
  mapper->SetInputData(boxdata);
#endif
  vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
  actor->GetProperty()->SetColor(bbcolor.r, bbcolor.g, bbcolor.b);
  actor->GetProperty()->SetRepresentationToWireframe();
  actor->SetMapper(mapper);
  return actor;
}

vtkSmartPointer<vtkPoints> Visualizer::GetPointsForTau(stepdata data, int tau) {
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  for (int i = 0; i < (int) data.sp->GetNumberOfPoints(); i++) {
    double p[3];
    data.sp->GetPoint(i, p);
    if (data.tau->GetComponent(i, 0) == tau) { points->InsertNextPoint(p); }
  }
  return points;
}

std::pair<vtkSmartPointer<vtkPoints>, vtkSmartPointer<vtkUnsignedCharArray>>
Visualizer::GetPointsAndColorsForTau(stepdata data, int tau, std::string color_by, ColorMap *cm) {
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkDataArray> v = data.extra_fields[color_by];
//  double color[3];

  color c;
  // Set up character array that holds the colors for each voxel
  vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
  colors->SetName("colors");
  colors->SetNumberOfComponents(3);

  // Create lookup table for colors
  vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
  double *range = v->GetRange();
//  lut->SetTableRange(range[0], range[1]);
//  lut->Build();

  // get points and set colors
  for (int i = 0; i < (int) data.sp->GetNumberOfPoints(); i++) {
    double p[3];
    data.sp->GetPoint(i, p);
    if (data.tau->GetComponent(i, 0) == tau) {
      points->InsertNextPoint(p);
      c = cm->GetColor(v->GetComponent(i, 0),range[0],range[1]);
//      lut->GetColor(v->GetComponent(i, 0), color);
      unsigned char ccolor[3] = {static_cast<unsigned char>(255.0*c.r),
                   static_cast<unsigned char>(255.0*c.g),
                   static_cast<unsigned char>(255.0*c.b)};
      colors->InsertNextTupleValue(ccolor);
    }
  }
  return {points, colors};
}


vtkSmartPointer<vtkActor>
Visualizer::GetActorForType(stepdata data, int tau, color c, double opacity, std::string color_by, ColorMap *cm) {
  vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkPoints> points;
  vtkSmartPointer<vtkUnsignedCharArray> colors;
  if (color_by.compare("none") == 0) {
    points = GetPointsForTau(data, tau);
    polydata->SetPoints(points);
  } else {
    std::pair<vtkSmartPointer<vtkPoints>, vtkSmartPointer<vtkUnsignedCharArray>>
        p = GetPointsAndColorsForTau(data, tau, color_by, cm);
    points = p.first;
    colors = p.second;
    polydata->SetPoints(points);
    polydata->GetPointData()->SetScalars(colors);
  }
//  vtkSmartPointer<vtkPolyData> glyph = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkCubeSource> cubeSource = vtkSmartPointer<vtkCubeSource>::New();
  vtkSmartPointer<vtkGlyph3D> glyph3D = vtkSmartPointer<vtkGlyph3D>::New();
  glyph3D->SetColorModeToColorByScalar();
  glyph3D->SetSourceConnection(cubeSource->GetOutputPort());
#if VTK_MAJOR_VERSION <= 5
  glyph3D->SetInput(polydata);
#else
  glyph3D->SetInputData(polydata);
#endif
  glyph3D->ScalingOff();
  glyph3D->Update();
  vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputConnection(glyph3D->GetOutputPort());
  vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(mapper);
  if (color_by.compare("none") == 0) {
    actor->GetProperty()->SetOpacity(opacity);
    actor->GetProperty()->SetColor(c.r, c.g, c.b);
  }
  return actor;
}

std::string Visualizer::GetImNameForStep(int step) {
  std::stringstream num;
  num << std::setfill('0') << std::setw(numlen);
  num << step;
  return impath + prefix + "_" + num.str() + ".png";
}


std::vector<vtkSmartPointer<vtkActor> > Visualizer::VisualizeStep(int step,
                                                                  std::vector<int> taulist,
                                                                  bool show,
                                                                  std::vector<color> tau_colors,
                                                                  std::vector<double> tau_opacity,
                                                                  bool save,
                                                                  std::vector<std::string> color_by,
                                                                  std::vector<ColorMap *> cms,
                                                                  std::map<std::string,color> planes) {
  stepdata data = reader->GetDataForStep(step);
  std::vector<vtkSmartPointer<vtkActor> > actors;
  for (int i = 0; i < taulist.size(); i++) {
    vtkSmartPointer<vtkActor> actor =
        GetActorForType(data, taulist[i], tau_colors[i], tau_opacity[i], color_by[i], cms[i]);
    renderer->AddActor(actor);
    actors.push_back(actor);
  }
  renderer->AddActor(GetActorForBox(data));
  if (planes.size() > 0){
    std::vector<vtkSmartPointer<vtkActor> > bnd_actors  = GetBoundaryPlanes(data, planes);
    for (auto plane : bnd_actors){
      renderer->AddActor(plane);
      actors.push_back(plane);
    }
  }

//  renderWindow->Render();
  if (show) {
    std::stringstream title;
    title << "step " << step;
    renderWindow->SetWindowName(title.str().c_str());
    renderWindowInteractor->Initialize();
    renderWindowInteractor->Start();
  }
  if (save) {
    vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
    windowToImageFilter->SetInput(renderWindow);
    windowToImageFilter->Update();
    vtkSmartPointer<vtkPNGWriter> writer =
        vtkSmartPointer<vtkPNGWriter>::New();
    writer->SetFileName(GetImNameForStep(step).c_str());
    writer->SetInputConnection(windowToImageFilter->GetOutputPort());
    writer->Write();
    if (!show)
      std::cout << "Create new image: " << GetImNameForStep(step).c_str() << std::endl;
  }
  return actors;
}


void Visualizer::AnimateOffScreen(std::vector<int> taulist,
                         std::vector<int> steps,
                         std::vector<int> static_tau,
                         std::vector<color> colors,
                         std::vector<double> opacity,
                         std::vector<std::string> color_by,
                         std::vector<ColorMap *> cms, std::map<std::string,color> planes) {
  std::cout << "Running visualization off screen!\n";
  if (static_tau.size() > 0)
    VisualizeStep(steps[0],static_tau, false, colors, opacity,false, color_by, cms, planes);
  std::vector<vtkSmartPointer<vtkActor> > update_actors;
  for (auto step : steps){
    for (auto actor : update_actors) { renderer->RemoveActor(actor); }
    update_actors = VisualizeStep(step,taulist, false, colors, opacity, true, color_by, cms, planes);
  }
}

void Visualizer::AnimateOnScreen(std::vector<int> taulist,
                         std::vector<int> steps,
                         std::vector<int> static_tau,
                         std::vector<color> colors,
                         std::vector<double> opacity,
                         bool save,
                         std::vector<std::string> color_by,
                         std::vector<ColorMap *> cms,bool loop, std::map<std::string,color> planes) {
  renderWindowInteractor->Initialize();
  VisualizeStep(steps[0],
                static_tau,
                false,
                colors,
                opacity,
                save,
                color_by,
                cms, planes);
  std::vector<int> update_tau;
  vtkSmartPointer<vtkTimerCallback> cb = vtkSmartPointer<vtkTimerCallback>::New();
  cb->tmax = (int) steps.size();
  cb->v = this;
  cb->save = save;
  cb->steps = steps;
  cb->cms = cms;
  cb->loop = loop;
  for (int i = 0; i < taulist.size(); i++) {
    if (std::find(static_tau.begin(), static_tau.end(), taulist[i]) == static_tau.end()) {
      cb->taulist.push_back(taulist[i]);
      cb->colors.push_back(colors[i]);
      cb->opacity.push_back(opacity[i]);
      cb->color_by.push_back(color_by[i]);
    }
  }
  renderWindowInteractor->AddObserver(vtkCommand::TimerEvent, cb);
  int timerId = renderWindowInteractor->CreateRepeatingTimer((unsigned int) (1000 / fps));

  renderWindowInteractor->Start();
}


