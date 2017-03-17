//
// Created by mpalm on 15/03/17.
//

#ifndef VISGRID3D_READER_H
#define VISGRID3D_READER_H

#include <map>
#include <string>
#include <vector>
#include <vtkSmartPointer.h>
#include <vtkStructuredPointsReader.h>
#include <vtkDataArray.h>


struct stepdata {
  vtkSmartPointer<vtkStructuredPoints> sp;
  vtkSmartPointer<vtkDataArray> sigma;
  vtkSmartPointer<vtkDataArray> tau;
  std::map<std::string, vtkSmartPointer<vtkDataArray> > extra_fields;
};

class DataReader {
 public:
  DataReader();
  DataReader(std::string _basename, std::string _datapath);
  DataReader(std::string _basename, std::string _datapath, std::vector<std::string> _extra_fields);
  std::vector<int> FindSteps();
  stepdata GetDataForStep(int step);
  stepdata ReadData(int step);

 private:
  vtkSmartPointer<vtkDataArray> GetArrayFromFile(std::string name);
  vtkSmartPointer<vtkStructuredPointsReader> reader;
  std::string GetFileNameForStep(int step);
  std::vector<std::string> fields;
  std::vector<std::string> extra_fields;
  std::string basename;
  std::string datapath;

};

#endif //VISGRID3D_READER_H
