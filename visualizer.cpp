//
// Created by mpalm on 15/03/17.
//

#include "visualizer.h"
#include <sstream>      // std::stringstream
#include <algorithm>
#include <vector>

#include <vtkStructuredPoints.h>
#include <vtkDataSetMapper.h>
#include <vtkDataArray.h>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCubeSource.h>
#include <vtkGlyph3D.h>
#include <vtkPolyDataMapper.h>
#include <vtkCommand.h>
#include <vtkProperty.h>
#include <vtkRendererCollection.h>
#include <vtkCamera.h>


class vtkTimerCallback : public vtkCommand
{
 private:
  int TimerCount;

 public:
  Visualizer * v;
  std::vector<int> taulist;
  std::vector<double> opacity;
  std::vector<color> colors;
  std::vector<vtkSmartPointer<vtkActor> > update_actors;
  std::map<int, std::vector < vtkSmartPointer<vtkActor> > > used_actors;
  int tmax;

  static vtkTimerCallback *New()
  {
    vtkTimerCallback *cb = new vtkTimerCallback;
    cb->TimerCount = 0;
    return cb;
  }

  virtual void Execute(vtkObject *caller, unsigned long eventId, void *vtkNotUsed(callData)){
    if (this->TimerCount == tmax){this->TimerCount = 0;}

    vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::SafeDownCast(caller);
    vtkRenderWindow *win = iren->GetRenderWindow();
    vtkRenderer * ren = win->GetRenderers()->GetFirstRenderer();
    for (auto actor : update_actors){ ren->RemoveActor(actor);}
    if (used_actors.find(TimerCount) == used_actors.end()) {
      update_actors = v->VisualizeStep(TimerCount, taulist, false, colors, opacity);
      used_actors[TimerCount] = update_actors;
    }
    else{
      std::stringstream title;
      title << "step " << TimerCount;
      win->SetWindowName(title.str().c_str());
      for (auto actor : used_actors[TimerCount]){
        ren->AddActor(actor);
      }
      win->Render();
      update_actors = used_actors[TimerCount];
    }
    ++this->TimerCount;
  }


};

Visualizer::Visualizer(DataReader * _reader){
  reader = _reader;
  bgcolor = {0,0,0};
  bbcolor = {1,1,1};
  winsize = {800,800};
  fps = 1;
  camposition = NULL;
  camfocus = NULL;
  campitch = 0;
}

void Visualizer::InitRenderer(){
  renderer = vtkSmartPointer<vtkRenderer>::New();
  renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow->AddRenderer(renderer);
  renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
  renderWindowInteractor->SetRenderWindow(renderWindow);
  renderWindow->SetSize(winsize[0], winsize[1]);
  renderer->SetBackground(bgcolor.r,bgcolor.g,bgcolor.b);
  vtkSmartPointer<vtkCamera> cam = renderer->GetActiveCamera();
  if (!camposition){ camposition = cam->GetPosition();}
  if (!camfocus){ camfocus = cam->GetFocalPoint();}
}

void Visualizer::ModifyCamera(){
  vtkSmartPointer<vtkCamera> cam = vtkSmartPointer<vtkCamera>::New();
  cam->SetPosition(camposition);
  cam->SetFocalPoint(camfocus);
  cam->Pitch(campitch);
  renderer->SetActiveCamera(cam);
}

vtkSmartPointer<vtkActor> Visualizer::GetActerForBBox(stepdata data){
  int * dim = data.sp->GetDimensions();
  vtkSmartPointer<vtkImageData> boxdata = vtkSmartPointer<vtkImageData>::New();
  boxdata->SetDimensions(2, 2, 2);
  boxdata->SetSpacing(dim[0], dim[1], dim[2]);
  boxdata->SetOrigin(0, 0, 0);
  vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New();
  mapper->SetInputData(boxdata);
  vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
  actor->GetProperty()->SetColor(bbcolor.r,bbcolor.g,bbcolor.b);
  actor->GetProperty()->SetRepresentationToWireframe();
  actor->SetMapper(mapper);
  return actor;
}

vtkSmartPointer<vtkPoints> Visualizer::GetPointsForTau(stepdata data, int tau){
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  for(int i=0;i< (int)data.sp->GetNumberOfPoints(); i++){
    double p[3];
    data.sp->GetPoint(i,p);
    if (data.tau->GetComponent(i,0) == tau) { points->InsertNextPoint(p); }
  }
  return points;
}

vtkSmartPointer<vtkActor> Visualizer::GetActorForType(stepdata data, int tau) {
  return GetActorForType(data,tau,{0.5,0.5,0.5},1);
}


vtkSmartPointer<vtkActor> Visualizer::GetActorForType(stepdata data, int tau, color c, double opacity){
  vtkSmartPointer<vtkPoints> points = GetPointsForTau(data,tau);
  vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
  polydata->SetPoints(points);
  vtkSmartPointer<vtkPolyData> glyph = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkCubeSource> cubeSource = vtkSmartPointer<vtkCubeSource>::New();
  vtkSmartPointer<vtkGlyph3D> glyph3D = vtkSmartPointer<vtkGlyph3D>::New();
  glyph3D->SetSourceConnection(cubeSource->GetOutputPort());
  glyph3D->SetColorModeToColorByScalar();
  glyph3D->SetInputData(polydata);
  glyph3D->Update();
  vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputConnection(glyph3D->GetOutputPort());
  vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(mapper);
  actor->GetProperty()->SetOpacity(opacity);
  actor->GetProperty()->SetColor(c.r,c.g,c.b);
  return actor;
}



std::vector<vtkSmartPointer <vtkActor> > Visualizer::VisualizeStep(int step, std::vector<int> taulist) {
  std::vector<double> tau_opacity(taulist.size(),1);
  std::vector<color> tau_colors(taulist.size(),{.5,.5,.5});
  return VisualizeStep(step,taulist,false,tau_colors,tau_opacity);
}

std::vector<vtkSmartPointer <vtkActor> > Visualizer::VisualizeStep(int step,std::vector<int> taulist,
                                                                   bool show){
  std::vector<double> tau_opacity(taulist.size(),1);
  std::vector<color> tau_colors(taulist.size(),{.5,.5,.5});
  return VisualizeStep(step,taulist,show,tau_colors,tau_opacity);
}


std::vector<vtkSmartPointer <vtkActor> > Visualizer::VisualizeStep(int step,std::vector<int> taulist,
                                                                     bool show,std::vector<color> tau_colors,
                                                                   std::vector<double> tau_opacity){
  std::stringstream title;
  title << "step " << step;
  renderWindow->SetWindowName(title.str().c_str());
  stepdata data = reader->GetDataForStep(step);
  std::vector<vtkSmartPointer <vtkActor> > actors;
  for (int i = 0; i < taulist.size(); i++){
    vtkSmartPointer <vtkActor> actor = GetActorForType(data, taulist[i], tau_colors[i], tau_opacity[i]);
    renderer->AddActor(actor);
    actors.push_back(actor);
  }
  renderer->AddActor(GetActerForBBox(data));

  renderWindow->Render();
  if (show) {
    renderWindowInteractor->Initialize();
    renderWindowInteractor->Start();
  }
  return actors;
}

void Visualizer::Animate(std::vector<int> taulist,std::vector<int> steps, std::vector<int> static_tau,
                         std::vector<color> colors, std::vector<double> opacity){
  renderWindowInteractor->Initialize();
  VisualizeStep(steps[0], static_tau, false, colors, opacity);
  std::vector<int> update_tau;
  vtkSmartPointer<vtkTimerCallback> cb = vtkSmartPointer<vtkTimerCallback>::New();
  cb->tmax = steps[steps.size()-1];
  cb->v = this;
//  for (auto tau : taulist) {
  for (int i = 0; i < taulist.size(); i++){
    if (std::find(static_tau.begin(), static_tau.end(), taulist[i]) == static_tau.end()) {
      cb->taulist.push_back(taulist[i]);
      cb->colors.push_back(colors[i]);
      cb->opacity.push_back(opacity[i]);
    }
  }
  renderWindowInteractor->AddObserver(vtkCommand::TimerEvent,cb);
  int timerId = renderWindowInteractor->CreateRepeatingTimer((int)(1000 / fps));

  renderWindowInteractor->Start();
}


