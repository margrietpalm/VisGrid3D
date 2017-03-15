//
// Created by mpalm on 15/03/17.
//

#include "visualizer.h"
#include <sstream>      // std::stringstream
#include <algorithm>

#include <vtkStructuredPoints.h>
#include <vtkDataArray.h>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCubeSource.h>
#include <vtkGlyph3D.h>
#include <vtkPolyDataMapper.h>
#include <vtkCommand.h>
#include <vtkRendererCollection.h>


class vtkTimerCallback : public vtkCommand
{
 private:
  int TimerCount;

 public:
  Visualizer * v;
  std::vector<int> taulist;
  std::map<int, vtkSmartPointer<vtkActor> > update_actors;
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
    for (auto item : update_actors)
      ren->RemoveActor(item.second);
    update_actors = v->VisualizeStep(TimerCount,taulist);
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



std::map<int, vtkSmartPointer <vtkActor> > Visualizer::VisualizeStep(int step, std::vector<int> taulist) {
  return VisualizeStep(step, taulist, false);
}

std::map<int, vtkSmartPointer <vtkActor> > Visualizer::VisualizeStep(int step,std::vector<int> taulist,
                                                                     bool show){
  std::stringstream title;
  title << "step " << step;
  std::cout << title.str() << std::endl;
  renderWindow->SetWindowName(title.str().c_str());

  stepdata data = reader->GetDataForStep(step);
  std::map<int, vtkSmartPointer <vtkActor> > actors;
  for (auto tau : taulist) {
    actors[tau] = GetActorForType(data, tau);
    renderer->AddActor(actors[tau]);
  }
  renderWindow->Render();
  if (show) {
    renderWindowInteractor->Initialize();
    renderWindowInteractor->Start();
  }
  return actors;
}

void Visualizer::Animate(std::vector<int> taulist,std::vector<int> steps, std::vector<int> static_tau){
  renderWindowInteractor->Initialize();
  std::map<int, vtkSmartPointer <vtkActor> > actors = VisualizeStep(steps[0], taulist);
  std::vector<int> update_tau;
  vtkSmartPointer<vtkTimerCallback> cb =
      vtkSmartPointer<vtkTimerCallback>::New();
  cb->tmax = steps[steps.size()-1];
  cb->v = this;
  if (static_tau.size() == 0){
    cb->taulist = taulist;
    cb->update_actors = actors;
  }
  else{
    for (auto tau : taulist){
      if (std::find(static_tau.begin(), static_tau.end(), tau) == static_tau.end()){
        cb->taulist.push_back(tau);
        cb->update_actors[tau] = actors[tau];
      }
    }
  }
  renderWindowInteractor->AddObserver(vtkCommand::TimerEvent,cb);
  int timerId = renderWindowInteractor->CreateRepeatingTimer(1000);
  renderWindowInteractor->Start();
//  timerId = renderWindowInteractor->CreateRepeatingTimer(int(1000 / float(fps)))
}


