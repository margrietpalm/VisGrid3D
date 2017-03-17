//
// Created by mpalm on 17/03/17.
//

#ifndef VISGRID3D_COLORMAP_H
#define VISGRID3D_COLORMAP_H

#include <string>
#include <map>
#include <cmath>
#include "colortable.h"

class ColorMap{

public:
  ColorMap(){
    gmin = 0;
    gmax = -1;
    std::string thisfile = __FILE__;
    std::string fn = thisfile.substr(0, thisfile.rfind("/") + 1) + "colormap.csv";
    ReadColorMap(fn);
  };

  ColorMap(std::string fn){
    gmin = 0;
    gmax = -1;
    ReadColorMap(fn);
  };

  void ReadColorMap(std::string fn){
    std::cout << "Read colormap from " << fn << std::endl;
    std::ifstream file;
    file.open(fn);
    std::string line;
    double r, g, b;
    int val;
    while (getline(file, line)) {
      ParseLine(line);
    }
  }

  void ParseLine(std::string line) {
    color c;
    unsigned long stop_val = line.find("\t");
    unsigned long stop_r = line.find("\t", stop_val + 1);
    unsigned long stop_g = line.find("\t", stop_r + 1);
    unsigned long stop_b = line.size();
    int val = std::stoi(line.substr(0, stop_val));
    c.r = std::stod(line.substr(stop_val + 1, stop_r - stop_val - 1).c_str());
    c.g = std::stod(line.substr(stop_r + 1, stop_g - stop_r - 1).c_str());
    c.b = std::stod(line.substr(stop_g + 1, stop_b - stop_g - 1).c_str());
    colormap[val] = c;
  };

  color GetColor(int val, int vmin, int vmax){
    if (gmax == -1)
      return colormap[(int)round(255*(val-vmin)/(vmax-vmin))];
    else
      return colormap[(int)round(255*(val-gmin)/(gmax-gmin))];

  }

  int gmin, gmax;

 private:
  std::map<int,color> colormap;

};

#endif //VISGRID3D_COLORMAP_H
