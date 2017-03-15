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


class vtkTimerCallback : public vtkCommand
{
 private:
  int TimerCount;

 public:
  Visualizer * v;
  std::vector<int> taulist;
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
      update_actors = v->VisualizeStep(TimerCount, taulist);
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
}

void Visualizer::InitRenderer(){
  renderer = vtkSmartPointer<vtkRenderer>::New();
  renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow->AddRenderer(renderer);
  renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
  renderWindowInteractor->SetRenderWindow(renderWindow);
  renderer->SetBackground(bgcolor.r,bgcolor.g,bgcolor.b); // Background color green
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
  actor->GetProperty()->SetColor(1,1,1);
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

vtkSmartPointer<vtkActor> Visualizer::GetActorForType(stepdata data, int tau){
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
  return actor;
}



std::vector<vtkSmartPointer <vtkActor> > Visualizer::VisualizeStep(int step, std::vector<int> taulist) {
  return VisualizeStep(step, taulist, false);
}

std::vector<vtkSmartPointer <vtkActor> > Visualizer::VisualizeStep(int step,std::vector<int> taulist,
                                                                     bool show){
  std::stringstream title;
  title << "step " << step;
  renderWindow->SetWindowName(title.str().c_str());
  stepdata data = reader->GetDataForStep(step);
  std::vector<vtkSmartPointer <vtkActor> > actors;
  for (auto tau : taulist) {
    vtkSmartPointer <vtkActor> actor = GetActorForType(data, tau);
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

void Visualizer::Animate(std::vector<int> taulist,std::vector<int> steps, std::vector<int> static_tau){
  renderWindowInteractor->Initialize();
  VisualizeStep(steps[0], static_tau);
  std::vector<int> update_tau;
  vtkSmartPointer<vtkTimerCallback> cb = vtkSmartPointer<vtkTimerCallback>::New();
  cb->tmax = steps[steps.size()-1];
  cb->v = this;
  for (auto tau : taulist) {
    if (std::find(static_tau.begin(), static_tau.end(), tau) == static_tau.end())
      cb->taulist.push_back(tau);
  }
  renderWindowInteractor->AddObserver(vtkCommand::TimerEvent,cb);
  int timerId = renderWindowInteractor->CreateRepeatingTimer(100);
  renderWindowInteractor->Start();
}


