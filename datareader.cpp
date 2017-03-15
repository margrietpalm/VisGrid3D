//
// Created by mpalm on 15/03/17.
//

#include <iostream>     // std::cout, std::endl
#include <iomanip>      // std::setfill, std::setw
#include <sstream>      // std::stringstream

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
  stepdata sd;
  sd.sp = reader->GetOutput();
  sd.sigma = GetArrayFromFile("cell.id");
  sd.tau = GetArrayFromFile("cell.type");
//  data[step] = sd;
  return sd;
}



vtkSmartPointer<vtkDataArray> DataReader::GetArrayFromFile(std::string name){
  reader->SetScalarsName(name.c_str());
  reader->Update();  //I think this actually makes the reader do something
  vtkSmartPointer<vtkStructuredPoints> sp = reader->GetOutput();
  vtkSmartPointer<vtkPointData> pd = sp->GetPointData();
  pd->Update();
  return pd->GetScalars(name.c_str());
};