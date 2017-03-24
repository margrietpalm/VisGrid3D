#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include "cxxopts.hpp"
#include <fstream>
#include <boost/program_options.hpp>
#include "visualizer.h"
#include "colormap.h"
#include <boost/filesystem.hpp>

// TODO: Add support for generating movies


std::vector<std::string> SplitString(std::string s) {
  unsigned long idx_prev = 0;
  unsigned long idx = s.find(",", idx_prev);
  std::vector<std::string> v;
  while (idx != std::string::npos) {
    v.push_back(s.substr(idx_prev, idx - idx_prev));
    idx_prev = idx + 1;
    idx = (int) s.find(",", idx_prev);
  }
  v.push_back(s.substr(idx_prev, s.size() - idx_prev));
  return v;
}

cxxopts::Options GetPars(int argc, char *argv[]) {
  cxxopts::Options options(argv[0], " - example command line options");
  options.add_options()
      ("h,help", "Print help")
      ("i,simdir", "Folder containing vtk files", cxxopts::value<std::string>())
      ("t,types", "Comma-separated list of cell types to visualize", cxxopts::value<std::string>())
      ("f,fields",
       "Comma-separated list of fields (stored in the vtk) used to color the cells (use none to skip a cell type)",
       cxxopts::value<std::string>())
      ("c,colors", "Comma-separated list of colors associated to the cell types", cxxopts::value<std::string>())
      ("a,alpha", "Comma-separated list of alpha-values associated to the cell types", cxxopts::value<std::string>())
      ("static", "Comma-separated list of static cell types", cxxopts::value<std::string>())
      ("steps", "Comma-separated list of time steps to visualize", cxxopts::value<std::string>())
      ("W,width", "visualization width", cxxopts::value<int>()->default_value("800"))
      ("H,height", "visualization height", cxxopts::value<int>()->default_value("800"))
      ("bgcolor", "background color", cxxopts::value<std::string>()->default_value("black"))
      ("bboxcolor", "bounding box color", cxxopts::value<std::string>()->default_value("white"))
      ("campos", "camera position", cxxopts::value<std::string>())
      ("camfocus", "camera focus", cxxopts::value<std::string>())
      ("campitch", "camera pitch", cxxopts::value<double>())
      ("fps", "frame rate", cxxopts::value<int>())
      ("o,outdir", "Folder to write images to", cxxopts::value<std::string>())
      ("s,save", "Save images", cxxopts::value<bool>())
      ("prefix", "Prefix for image names", cxxopts::value<std::string>())
      ("m,colormap","File with colormap to be used with the fields", cxxopts::value<std::string>())
      ("fmax","Comma-seperated list with max value for each field", cxxopts::value<std::string>())
      ("fmin","Comma-seperated list with min value for each field", cxxopts::value<std::string>())
      ("showcolors", "show available colors", cxxopts::value<bool>())
      ("l,loop","Loop visualization", cxxopts::value<bool>())
      ("q,quiet","Hide visualization windows", cxxopts::value<bool>())
      ;


  options.parse(argc, argv);
  if (options.count("help")) {
    std::cout << options.help({""}) << std::endl;
    exit(0);
  }
  std::string types = options["types"].as<std::string>();
  return options;
}

void SetOutputDirectory(std::string outdir){
  boost::filesystem::path p (outdir);
  // clean up old simulation files
  if (!(boost::filesystem::is_directory(p)))
    boost::filesystem::create_directories(p);
}

// check if path ends with / and add if missing
std::string FixPath(std::string path){
  char ch = path.back();
  if (ch!='/')
    path = path+"/";
  return path;
}


int main(int argc, char *argv[]) {
  std::vector<int> steps;
  std::vector<double> alpha;
  std::vector<color> colors;

  cxxopts::Options opt = GetPars(argc, argv);

  // Set up color map
  ColorTable *ct = new ColorTable();
  if (opt.count("showcolors")) {
    ct->PrintAvailableColors();
    exit(0);
  }

  // Set up data reader
  std::string datapath;
  if (opt.count("simdir")) { datapath = FixPath(opt["simdir"].as<std::string>()); }
  else { datapath = "./"; }
  std::vector<std::string> color_by;
  std::vector<std::string> extra_fields;
  if (opt.count("fields")) {
    color_by = SplitString(opt["fields"].as<std::string>());
    extra_fields = color_by;
    std::vector<std::string>::iterator it = std::unique(extra_fields.begin(), extra_fields.end());
    extra_fields.resize(std::distance(extra_fields.begin(), it));
  }
  DataReader *dr = new DataReader("plot", datapath, extra_fields);
  // select step to visualize
  if (opt.count("steps")) {
    for (auto s : SplitString(opt["steps"].as<std::string>()))
      steps.push_back(stoi(s));
  } else {
    steps = dr->FindSteps();
    std::cout << "Steps not specified - Visualize for all " << steps.size() << " vtk files" << std::endl;
  }

  // Select types to plot and update
  std::vector<int> types;
  std::vector<int> stattypes;
  if (opt.count("types"))
    for (auto s : SplitString(opt["types"].as<std::string>())) { types.push_back(stoi(s)); }
  else {
    std::cout << "Please specify cell types to plot" << std::endl;
    exit(0);
  }
  if (opt.count("static"))
    for (auto s : SplitString(opt["static"].as<std::string>())) { stattypes.push_back(stoi(s)); }

  // set colors and opacity for cells
  if (opt.count("colors")) {
    for (auto s : SplitString(opt["colors"].as<std::string>())) {
      colors.push_back(ct->GetRGBDouble(s));
    }
    if (colors.size() != types.size()) {
      std::cout << "!!! Number of specified colors did not match number of types - default to grey" << std::endl;
      colors.clear();
      for (auto t : types) {
        colors.push_back(ct->GetRGBDouble("grey"));
      }
    }
  }
  if (opt.count("colors") == 0) {
    std::cout << "Colors not specified - default to grey cells" << std::endl;
    for (auto t : types) { colors.push_back(ct->GetRGBDouble("grey")); }
  }
  if (opt.count("alpha")) {
    for (auto s : SplitString(opt["alpha"].as<std::string>()))
      alpha.push_back(stod(s));
    if (colors.size() != types.size()) {
      std::cout << "!!! Number of specified alpha values did not match number of types - default to 1" << std::endl;
      alpha.clear();
      for (auto t : types) { alpha.push_back(1); }
    }
  }
  if (opt.count("alpha") == 0) {
    std::cout << "Alpha not specified - default to 1" << std::endl;
    for (auto t : types) { alpha.push_back(1); }
  }
  if (color_by.size() != types.size()) {
    color_by.clear();
    for (auto t : types) { color_by.push_back("none"); }
  }
  ColorMap * cm;
  if (opt.count("colormap") == 0)
    cm = new ColorMap();
  else
    cm = new ColorMap(opt["colormap"].as<std::string>());
  std::vector<ColorMap *> cms(types.size(),cm);
  if ((opt.count("fmin") != 0) & (opt.count("fmax") != 0)){
    std::vector<std::string> fmin = SplitString(opt["fmin"].as<std::string>());
    std::vector<std::string> fmax = SplitString(opt["fmax"].as<std::string>());
    if ((fmin.size() == cms.size()) & (fmax.size() == cms.size())) {
      for (int i = 0; i < cms.size(); i++) {
        cms[i]->gmin = stod(fmin[i]);
        cms[i]->gmax = stod(fmax[i]);
      }
    }
  }

    // initialize visualization
  Visualizer *vis = new Visualizer(dr);
  if (opt.count("width")) { vis->winsize[0] = opt["width"].as<int>(); }
  if (opt.count("height")) { vis->winsize[1] = opt["height"].as<int>(); }
  if (opt.count("bgcolor")) {
    std::vector<std::string> v = SplitString(opt["bgcolor"].as<std::string>());
    if (v.size() == 3)
      vis->bgcolor = {stod(v[0]), stod(v[1]), stod(v[2])};
    else if (v.size() == 1)
      vis->bgcolor = ct->GetRGBDouble(v[0]);
  }
  if (opt.count("bboxcolor")) {
    std::vector<std::string> v = SplitString(opt["bboxcolor"].as<std::string>());
    if (v.size() == 3)
      vis->bbcolor = {stod(v[0]), stod(v[1]), stod(v[2])};
    else if (v.size() == 1)
      vis->bbcolor = ct->GetRGBDouble(v[0]);
  }
  if (opt.count("fps")) { vis->fps = opt["fps"].as<double>(); }
  bool onscreen = true;
  if (opt.count("quiet")){ onscreen = false;}
  vis->InitRenderer(onscreen);

//   set up camera
  bool modcam = false;
  if (opt.count("campos")) {
    modcam = true;
    std::vector<std::string> v = SplitString(opt["campos"].as<std::string>());
    if (v.size() == 3) {
      for (int i = 0; i < 3; i++) {
        vis->camposition[i] = stoi(v[i]);
      }
    }
  }
  if (opt.count("camfocus")) {
    modcam = true;
    std::vector<std::string> v = SplitString(opt["camfocus"].as<std::string>());
    if (v.size() == 3) {
      for (int i = 0; i < 3; i++) {
        vis->camfocus[i] = stoi(v[i]);
      }
    }
  }
  if (opt.count("campitch")) {
    modcam = true;
    vis->campitch = opt["campitch"].as<double>();
  }
  if (modcam) { vis->ModifyCamera(); }

  // set saving options
  bool save = false;
  if (opt.count("save") | opt.count("outdir")) { save = true; }
  if (opt.count("outdir")) {
    std::string outdir = FixPath(opt["outdir"].as<std::string>());
    SetOutputDirectory(outdir);
    vis->impath = outdir;
  }
  if (opt.count("prefix")) { vis->prefix = opt["prefix"].as<std::string>(); }
  if (save) { vis->numlen = (int)std::to_string(steps[steps.size() - 1]).size(); }

  bool loop = false;
  if (opt.count("loop")) {loop = true;}

    // run animation
  if (steps.size() > 1) {
    if (onscreen)
      vis->AnimateOnScreen(types, steps, stattypes, colors, alpha, save, color_by, cms, loop);
    else {
      std::cout << "animate " << steps.size() << " steps\n";
      vis->AnimateOffScreen(types, steps, stattypes, colors, alpha, color_by, cms);
    }
  }
  else
    vis->VisualizeStep(steps[0], types, onscreen, colors, alpha, save, color_by, cms);


  return EXIT_SUCCESS;
}