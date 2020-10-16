// params.cc

#include "params.h"
#include <image/image.h>
#include <shape/shape.h>
#include <iostream>
#include <fstream>

// Externals
extern CommonTilton::Image maskImage;
extern CommonTilton::Shape shapeFile;

namespace CommonTilton
{

 // Constructor
  Params::Params(const string& value)
  {
    version = value;

    return;
  }

 // Destructor...
  Params::~Params() 
  {
    return;
  }

 // Print version
  void Params::print_version( )
  {
    cout << endl << version << endl << endl;

    return;
  }

 // Read parameters
  bool Params::read(const char *param_file)
  {
//    cout << endl << "Parameters to be read from file " << param_file << endl << endl;
    ifstream param_fs(param_file);
    if (!param_fs)
    {
      cout << "Cannot open input file " << param_file << endl;
      return false;
    }

  // Read user defined parameters from parameter file until End-of-File reached
    bool mask_image_flag = false;
    bool shape_file_flag = false;
    bool output_image_flag = false;
    int sub_pos;
    string line, sub_string;
    while (!param_fs.eof())
    {
      getline(param_fs,line);
      sub_pos = line.find("-");
      while ((!param_fs.eof()) && (sub_pos != 0))
      {
        getline(param_fs,line);
        sub_pos = line.find("-");
      }
      if (line.find("-shape_file") != string::npos)
      {
        shape_file = process_line(line,false);
        shape_file_flag = true;
      }
      if (line.find("-mask_image") != string::npos)
      {
        mask_image_file = process_line(line,false);
        mask_image_flag = true;
      }
      if (line.find("-output_image") != string::npos)
      {
        output_image_file = process_line(line,false);
        output_image_flag = true;
      }
    }
    param_fs.close();

  // Exit with false status if a required parameter is not set.
    if (shape_file_flag == false)
    {
      cout << "ERROR: -shape_file (Input shape file name) is required" << endl;
      return false;
    }
    if (mask_image_flag == false)
    {
      cout << "ERROR: -mask_image (Input mask image file name) is required" << endl;
      return false;
    }
    if (output_image_flag == false)
    {
      cout << "ERROR: -output_image (output image file name) is required" << endl;
      return false;
    }

    shapeFile.open(shape_file);
    if (!shapeFile.valid())
    {
      cout << "ERROR: " << shape_file << " is not a valid Input Shape File" << endl;
      return false;
    }
    maskImage.open(mask_image_file);
    if (!maskImage.info_valid())
    {
      cout << "ERROR: " << mask_image_file << " is not a valid mask image" << endl;
      return false;
    }  

    return true;
 }

 // Print parameters
 void Params::print()
 {
  // Print version
   cout << "This is " << version << endl << endl;

  // Print input parameters
   cout << "Input shape file = " << shape_file << endl;
   cout << "Input mask image file = " << mask_image_file << endl;
   cout << "Output image file = " << output_image_file << endl;

   return;
 }

  string process_line(const string& line, const bool& list_flag)
  {
    int sub_pos, sub_pos_space, sub_pos_tab;
    string sub_string;

    sub_pos_space = line.find_first_of(" ");
    sub_pos_tab = line.find_first_of("\t");
    sub_pos = sub_pos_space;
    if (sub_pos == (int) string::npos)
      sub_pos = sub_pos_tab;
    if (sub_pos == (int) string::npos)
      return " ";

    sub_string = line.substr(sub_pos);
    while ((sub_string.substr(0,1) == "\t") || (sub_string.substr(0,1) == " "))
      sub_string = line.substr(++sub_pos);

    if (list_flag)
      return sub_string;

    if (sub_string.substr(0,1) == "\"")
    {
      sub_string = line.substr(++sub_pos);
      sub_pos = sub_string.find_first_of("\"");
      if (sub_pos !=  (int) string::npos)
        sub_string = sub_string.substr(0,sub_pos);
    }
    else
    {
#ifndef WINDOWS
      sub_pos = sub_string.find_first_of(" ");
      if (sub_pos !=  (int) string::npos)
        sub_string = sub_string.substr(0,sub_pos);
      sub_pos = sub_string.find_first_of("\t");
      if (sub_pos !=  (int) string::npos)
        sub_string = sub_string.substr(0,sub_pos);
      sub_pos = sub_string.find_first_of("\r");
      if (sub_pos !=  (int) string::npos)
        sub_string = sub_string.substr(0,sub_pos);
#endif
    }

    return sub_string;
  }

} // namespace CommonTilton

