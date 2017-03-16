#include <string>
#include <iostream>
#include <vector>
#include "cxxopts.hpp"
#include <fstream>
#include <boost/program_options.hpp>
#include <stdlib.h>
#include "visualizer.h"
#include "datareader.h"
#include "colormap.h"

std::vector<std::string> SplitString(std::string s){
  int idx_prev = 0;
  int idx = s.find(",",idx_prev);
  std::vector<std::string> v;
  while (idx!=std::string::npos){
    v.push_back(s.substr(idx_prev,idx-idx_prev));
    idx_prev = idx+1;
    idx = (int)s.find(",",idx_prev);
  }
  v.push_back(s.substr(idx_prev,s.size()-idx_prev));
  return v;
}

cxxopts::Options GetPars(int argc, char *argv[]){
  cxxopts::Options options(argv[0], " - example command line options");
  options.add_options()
      ("h,help", "Print help")
      ("i,simdir", "Folder containing vtk files", cxxopts::value<std::string>())
      ("t,types", "Comma-separated list of cell types to visualize", cxxopts::value<std::string>())
      ("c,colors", "Comma-separated list of colors associated to the cell types", cxxopts::value<std::string>())
      ("a,alpha", "Comma-separated list of alpha-values associated to the cell types", cxxopts::value<std::string>())
      ("static", "Comma-separated list of static cell types", cxxopts::value<std::string>())
      ("steps", "Comma-separated list of time steps to visualize", cxxopts::value<std::string>())
      ("W,width","visualization width", cxxopts::value<int>()->default_value("800"))
      ("H,height","visualization height", cxxopts::value<int>()->default_value("800"))
      ("bgcolor","background color", cxxopts::value<std::string>()->default_value("black"))
      ("bboxcolor","bounding box color", cxxopts::value<std::string>()->default_value("white"))
      ("f,fps","frame rate", cxxopts::value<int>())
      ("showcolors","show available colors", cxxopts::value<bool>());


  options.parse(argc, argv);
  if (options.count("help"))
  {
    std::cout << options.help({""}) << std::endl;
    exit(0);
  }
  std::string types = options["types"].as<std::string>();
  std::cout << types << std::endl;
  return options;
}



int main(int argc, char *argv[])
{

  std::vector<int> steps;
  std::vector<double> alpha;
  std::vector<color> colors;

  cxxopts::Options opt = GetPars(argc, argv);

  // Set up color map
  ColorMap * cm = new ColorMap();
  if (opt.count("showcolors")){
    cm->PrintAvailableColors();
    exit(0);
  }

  // Set up data reader
  std::string datapath;
  if (opt.count("simdir")){ datapath = opt["simdir"].as<std::string>(); }
  else { datapath = "./"; }
  DataReader * dr = new DataReader("plot",datapath);
  // select step to visualize
  if (opt.count("steps")){
    for (auto s : SplitString(opt["steps"].as<std::string>()))
      steps.push_back(stod(s));
  }
  else{
    steps = dr->FindSteps();
    std::cout << "Steps not specified - Visualize for all " << steps.size() << " vtk files" << std::endl;
  }

  // Select types to plot and update
  std::vector<int> types;
  std::vector<int> stattypes;
  if (opt.count("types"))
    for (auto s : SplitString(opt["types"].as<std::string>())){ types.push_back(stoi(s)); }
  else  {
    std::cout << "Please specify cell types to plot" << std::endl;
    exit(0);
  }
  if (opt.count("static"))
    for (auto s : SplitString(opt["static"].as<std::string>())){ stattypes.push_back(stoi(s)); }

  // set colors and opacity for cells
  if (opt.count("colors")){
    for (auto s : SplitString(opt["colors"].as<std::string>())) {
      colors.push_back(cm->GetRGBDouble(s));
    }
    if (colors.size() != types.size()) {
      std::cout << "!!! Number of specified colors did not match number of types - default to grey" << std::endl;
      colors.clear();
      for (auto t : types) {
        colors.push_back(cm->GetRGBDouble("grey"));
      }
    }
  }
  if (opt.count("colors")==0) {
    std::cout << "Colors not specified - default to grey cells" << std::endl;
    for (auto t : types){colors.push_back(cm->GetRGBDouble("grey"));}
  }
  if (opt.count("alpha")){
    for (auto s : SplitString(opt["alpha"].as<std::string>()))
      alpha.push_back(stod(s));
    if (colors.size() != types.size()){
      std::cout << "!!! Number of specified alpha values did not match number of types - default to 1" << std::endl;
      alpha.clear();
      for (auto t : types){alpha.push_back(1);}
    }
  }
  if (opt.count("alpha")==0) {
    std::cout << "Alpha not specified - default to 1" << std::endl;
    for (auto t : types){alpha.push_back(1);}
  }

  // various visualization settings
  int winwidth = 800;
  if (opt.count("width")){winwidth=opt["width"].as<int>();}
  int winheight = 800;
  if (opt.count("height")){winheight=opt["height"].as<int>();}
  double fps = 1;
  if (opt.count("fps")){fps=opt["fps"].as<double>();}
  color bgcolor = {0,0,0};
  if (opt.count("bgcolor")){
    std::vector<std::string> v = SplitString(opt["bgcolor"].as<std::string>());
    if (v.size() == 3)
      bgcolor = {stod(v[0]),stod(v[1]),stod(v[2])};
  }
  color bboxcolor = {1,1,1};
  if (opt.count("bboxcolor")){
    std::vector<std::string> v = SplitString(opt["bboxcolor"].as<std::string>());
    if (v.size() == 3)
      bboxcolor = {stod(v[0]),stod(v[1]),stod(v[2])};
  }

  Visualizer * v = new Visualizer(dr);
  v->winsize[0] = winwidth;
  v->winsize[1] = winheight;
  v->bbcolor = bboxcolor;
  v->bgcolor = bgcolor;
  v->InitRenderer();
  v->fps = fps;
  double p [3] = {200,200,600};
  v->camposition = p;
  double f [3] = {100,100,0};
  v->camfocus = f;
  v->ModifyCamera();
  steps = {0,1,2};
  v->Animate(types,steps,stattypes,colors,alpha);




  return EXIT_SUCCESS;
}