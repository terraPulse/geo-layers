// params.cc

#include "params.h"
#include <iostream>

CommonTilton::Image classSegImage;

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
//    cout << "Reading parameters from " << param_file << endl;
    ifstream param_fs(param_file);
    if (!param_fs.is_open())
    {
      cout << "Cannot open input file " << param_file << endl;
      return false;
    }

  // Read parameters from parameter file until End-of-File reached
    bool class_segmentation_flag = false;
    bool object_segmentation_flag = false;
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
      if (line.find("-class_segmentation") != string::npos)
      {
        class_segmentation_file = process_line(line,false);
        class_segmentation_flag = true;
      }
      if (line.find("-object_segmentation") != string::npos)
      {
        object_segmentation_file = process_line(line,false);
        object_segmentation_flag = true;
      }
    }

    param_fs.close();

// Exit with false status if a required parameter is not set, or an improper parameter
// setting is detected.
    if (!class_segmentation_flag)
    {
      cout << "ERROR: -class_segmentation (Input region class segmentation) is required" << endl;
      return false;
    }
    if (!object_segmentation_flag)
    {
      cout << "ERROR: -object_segmentation (Output connected region object) is required" << endl;
      return false;
    }
    if (!classSegImage.open(class_segmentation_file))
    {
      cout << "WARNING:  Input region class segmentation image file " << class_segmentation_file << " is of unknown format." << endl;
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
      cout << "Input region class segmentation image file image: " << class_segmentation_file << endl;
      cout << "Output connected region object segmentation image file name: " << object_segmentation_file << endl;
  }

  string process_line(const string& line, const bool& list_flag)
  {
    int sub_pos, sub_pos_space, sub_pos_tab;
    string sub_string;

    sub_pos_space = line.find_first_of(" ");
    sub_pos_tab = line.find_first_of("\t");
    sub_pos = sub_pos_space;
    if (sub_pos ==  (int) string::npos)
      sub_pos = sub_pos_tab;
    if (sub_pos ==  (int) string::npos)
      return " ";

    sub_string = line.substr(sub_pos);
    while ((sub_string.substr(0,1) == "\t") || (sub_string.substr(0,1) == " "))
      sub_string = line.substr(++sub_pos);
#ifndef WINDOWS
    if (list_flag)
      return sub_string;

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
    return sub_string;
  }

} // namespace CommonTilton
