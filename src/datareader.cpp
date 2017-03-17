//
// Created by mpalm on 15/03/17.
//

#include <iostream>     // std::cout, std::endl
#include <iomanip>      // std::setfill, std::setw
#include <sstream>      // std::stringstream
#include <glob.h>
#include <algorithm>

#include "datareader.h"
#include <vtkStructuredPoints.h>
#include <vtkPointData.h>


DataReader::DataReader(){
  basename = "plot";
  datapath = "./";
}

DataReader::DataReader(std::string _basename, std::string _datapath){
  basename = _basename;
  datapath = _datapath;
}

DataReader::DataReader(std::string _basename, std::string _datapath, std::vector<std::string> _extra_fields){
  basename = _basename;
  datapath = _datapath;
  extra_fields = _extra_fields;
}

std::vector<int> DataReader::FindSteps(){
  glob_t globbuf;
  int err = glob((datapath+basename+"_*"+".vtk").c_str(), 0, NULL, &globbuf);
  std::vector<int> steps;
  if(err == 0)
  {
    for (size_t i = 0; i < globbuf.gl_pathc; i++) {
      std::string s = globbuf.gl_pathv[i];
      steps.push_back(stoi(s.substr(s.rfind('_')+1,s.rfind('.')-s.rfind('_')-1)));
    }
    globfree(&globbuf);
  }
  std::sort(steps.begin(), steps.end());
  return steps;
}

std::string DataReader::GetFileNameForStep(int step){
  std::stringstream num;
  num << std::setfill('0') << std::setw(6);
  num << step;
  return datapath+basename+"_"+num.str()+".vtk";
}

stepdata DataReader::GetDataForStep(int step){
//  if (data.find(step) == data.end()){ ReadData(step); }
  return ReadData(step);
}




stepdata DataReader::ReadData(int step){
  reader = vtkSmartPointer<vtkStructuredPointsReader>::New();
  std::string fn = GetFileNameForStep(step);
  reader->SetFileName(fn.c_str());
  for (int i = 0; i < reader->GetNumberOfScalarsInFile(); i++)
    fields.push_back(reader->GetScalarsNameInFile(i));
  stepdata sd;
  sd.sp = reader->GetOutput();
  sd.sigma = GetArrayFromFile("cell.id");
  sd.tau = GetArrayFromFile("cell.type");
  for (auto f : extra_fields){
    if (f.compare("none") != 0)
      sd.extra_fields[f] = GetArrayFromFile(f);
  }
//  data[step] = sd;
  return sd;
}



vtkSmartPointer<vtkDataArray> DataReader::GetArrayFromFile(std::string name){
  if (std::find(fields.begin(),fields.end(),name)!=fields.end()) {
    reader->SetScalarsName(name.c_str());
    reader->Update();  //I think this actually makes the reader do something
    vtkSmartPointer <vtkStructuredPoints> sp = reader->GetOutput();
    vtkSmartPointer <vtkPointData> pd = sp->GetPointData();
    pd->Update();
    return pd->GetScalars(name.c_str());
  }
  else{
    std::cout << "Could not find array " << name << " in " << reader->GetFileName() << std::endl;
    exit(0);
  }
};

