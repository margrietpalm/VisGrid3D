#include <string>
#include <iostream>
#include <vector>

#include <boost/program_options.hpp>

#include "visualizer.h"
#include "datareader.h"

using namespace boost::program_options;
namespace po = boost::program_options;

po::variables_map getPars(int argc, char **argv){
  // Declare the supported options.
  int var2;
  po::options_description desc("Allowed options");
  desc.add_options()
      ("help,h", "print help message")
//      ("input-file,i",po::value<std::string>(), "data file")
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

//  if (!(vm.count("input-file"))){
//    cout << "missing input-file\n";
//    exit(1);
//  }

  return vm;
};




int main(int argc, char **argv)
{
//  po::variables_map pars = getPars(argc,argv);
//  std::string datapath = "/home/mpalm/morpheus/3D_Migration_611/";
  std::string datapath = "/home/mpalm/morpheus/Example-Protrusion_616/";

  std::string basename = "plot";
  std::string fn = "/home/mpalm/morpheus/Example-Protrusion_616/plot_000010.vtk";
//  std::string fn = "/home/mpalm/morpheus/3D_Migration_611/plot_000001.vtk";

  DataReader * dr = new DataReader(basename,datapath);
  stepdata data = dr->GetDataForStep(1);
  Visualizer * v = new Visualizer(dr);
  v->InitRenderer();
  double p [3] = {400,400,400};
  v->camposition = p;
  v->ModifyCamera();
  std::vector<int> taulist = {1};
  std::vector<int> steps = {0,1,2,3,4};
  std::vector<int> static_tau = {};
  v->Animate(taulist,steps,static_tau);
//  v->VisualizeStep(1,taulist);


  return EXIT_SUCCESS;
}