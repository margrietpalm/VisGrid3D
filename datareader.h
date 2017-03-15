//
// Created by mpalm on 15/03/17.
//

#ifndef VISGRID3D_READER_H
#define VISGRID3D_READER_H

#include <map>
#include <string>
#include <vtkSmartPointer.h>
#include <vtkStructuredPointsReader.h>
#include <vtkDataArray.h>

struct stepdata{
  vtkSmartPointer<vtkStructuredPoints> sp;
  vtkSmartPointer<vtkDataArray> sigma;
  vtkSmartPointer<vtkDataArray> tau;
};

class DataReader {
 public:
  DataReader();
  DataReader(std::string _basename, std::string _datapath);
  stepdata GetDataForStep(int step);
  stepdata ReadData(int step);
  std::map<int, stepdata> data;

  std::string basename;
  std::string datapath;

 private:
  vtkSmartPointer<vtkDataArray> GetArrayFromFile(std::string fn);
  vtkSmartPointer<vtkStructuredPointsReader> reader;
  std::string GetFileNameForStep(int step);

};

#endif //VISGRID3D_READER_H
