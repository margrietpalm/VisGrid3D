#include <string>
#include <iostream>

#include <vtkCubeSource.h>
#include <vtkGlyph3D.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyDataMapper.h>
#include <vtkSmartPointer.h>
#include <vtkStructuredPoints.h>
#include <vtkStructuredPointsReader.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

#include <boost/program_options.hpp>

using namespace boost::program_options;
namespace po = boost::program_options;

po::variables_map getPars(int argc, char **argv){
  // Declare the supported options.
  int var2;
  po::options_description desc("Allowed options");
  desc.add_options()
      ("help,h", "print help message")
      ("input-file,i",po::value<std::string>(), "data file")
//      ("simid,s",po::value<string>(), "simulation identifier, if omitted this is extracted form the filename of the input-file.")
//      ("save,S", "save images (only necessary when imagepath is not set)")
//      ("imagepath,o", po::value<string>()->default_value("images/"), "target directory for images")
//      ("stepsize,z", po::value<int>()->default_value(1), "step size of data")
//      ("fps,f", po::value<double>()->default_value(10.), "frames per second")
//      ("camera_distance", po::value<double>()->default_value(20.), "distance between camera and center (0,0,0)")
//      ("camera_rotation", po::value<double>()->default_value(0.), "rotation speed of the camera")
//      ("camera_angle", po::value<double>()->default_value(0.), "angle of the camera")
//      ("width", po::value<int>()->default_value(800), "width of the animation window")
//      ("height", po::value<int>()->default_value(800), "height of the animation window")
//      ("color_by", po::value<string>(), "property used for coloring")
//      ("ctb", po::value<string>(), "file with colortable")
//      ("bgcolor", po::value< vector<int> >()->multitoken(), "background color")
      ;

  po::variables_map vm;
  // parse regular options
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    cout << desc << endl;
    exit(0);
  }

  if (!(vm.count("input-file"))){
    cout << "missing input-file\n";
    exit(1);
  }

  return vm;
};


int main(int argc, char **argv)
{
  for (int i = 0; i < argc; i++)
    std::cout << argv[i] << " ";
  std::cout << std::endl;
  po::variables_map pars = getPars(argc,argv);

  std::string fn = "/home/mpalm/morpheus/Example-Protrusion_616/plot_000010.vtk";
//  std::string fn = "/home/mpalm/morpheus/3D_Migration_611/plot_000001.vtk";
  vtkSmartPointer<vtkStructuredPointsReader> reader =
      vtkSmartPointer<vtkStructuredPointsReader>::New();
  reader->SetFileName(fn.c_str());

  //First you have to create a reader object:
  vtkIdType numScal=reader->GetNumberOfScalarsInFile();
  std::cout << "number of scalars in file: " << numScal << std::endl;
//Show which scalars we have
  int i;
  for(i=0;i< numScal;i++){
    std::cout << "Scalar "<< i <<": " << reader->GetScalarsNameInFile(i)<< std::endl;
  }
  vtkSmartPointer<vtkStructuredPoints> sp;
  vtkSmartPointer<vtkPointData> pd;
  vtkSmartPointer<vtkDataArray> sigma;
  reader->SetScalarsName(reader->GetScalarsNameInFile(0));
  reader->Update();  //I think this actually makes the reader do something
  sp = reader->GetOutput();
  pd = sp->GetPointData();
  pd->Update();
  sigma = pd->GetScalars(reader->GetScalarsNameInFile(0));
  vtkIdType numPoints = sp->GetNumberOfPoints();

  vtkSmartPointer<vtkDataArray> tau;
  reader->SetScalarsName(reader->GetScalarsNameInFile(1));
  reader->Update();  //I think this actually makes the reader do something
  sp = reader->GetOutput();
  pd = sp->GetPointData();
  pd->Update();
  tau = pd->GetScalars(reader->GetScalarsNameInFile(2));

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  for(i=0;i< numPoints;i++){
    double p[3];
    sp->GetPoint(i,p);
    if (sigma->GetComponent(i,0) > 0) {
//      std::cout << "(" << p[0] << "," << p[1] << "," << p[2] << ") " << ": ";
//      std::cout << sigma->GetComponent(i,0) << " "  << tau->GetComponent(i,0) << std::endl;
      points->InsertNextPoint(p);
    }

  }

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

  vtkSmartPointer<vtkActor> actor =
      vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(mapper);

  vtkSmartPointer<vtkRenderer> renderer =
      vtkSmartPointer<vtkRenderer>::New();
  vtkSmartPointer<vtkRenderWindow> renderWindow =
      vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow->AddRenderer(renderer);
  vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
      vtkSmartPointer<vtkRenderWindowInteractor>::New();
  renderWindowInteractor->SetRenderWindow(renderWindow);

  renderer->AddActor(actor);
//  renderer->SetBackground(.3, .6, .3); // Background color green
//
  renderWindow->Render();
  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}