//
// Created by mpalm on 16/03/17.
//

#ifndef VISGRID3D_COLORMAP_H
#define VISGRID3D_COLORMAP_H

#include <string>
#include <fstream>

struct color{ double r,g,b; };

class ColorMap {
 public:

  ColorMap(){
    std::string thisfile = __FILE__;
    std::string fn_colormap = thisfile.substr(0,thisfile.rfind("/")+1)+"colors.csv";
    std::cout << fn_colormap << std::endl;
    std::ifstream file;
    file.open("../colors.csv");
    std::string line, name;
    double r,g,b;
    while ( getline(file,line)) {
      if (line.find("#")!=std::string::npos){ continue; }
      ParseLine(line);
    }
  };

  color GetRGBDouble(std::string colorname){
    if (colormap.find(colorname) == colormap.end())
      return {0.5,0.5,0.5};
    else
      return colormap[colorname];
  };

  void PrintAvailableColors(){
    std::cout << "There are " << colormap.size() << " colors available: ";
    for (auto c : colormap)
      std::cout << c.first << " ";
    std::cout << std::endl;
  }

 private:

  void ParseLine(std::string line){
      color c;
      int stop_name = line.find(",");
      int stop_r = line.find(",",stop_name+1);
      int stop_g = line.find(",",stop_r+1);
      int stop_b = line.size();
      std::string name = line.substr(0,stop_name);
      c.r = atof(line.substr(stop_name+1,stop_r-stop_name-1).c_str());
      c.g = atof(line.substr(stop_r+1,stop_g-stop_r-1).c_str());
      c.b = atof(line.substr(stop_g+1,stop_b-stop_g-1).c_str());
      colormap[name] = c;
    };

  std::map<std::string, color> colormap;


};

#endif //VISGRID3D_COLORMAP_H
