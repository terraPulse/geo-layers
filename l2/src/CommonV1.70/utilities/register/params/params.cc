// params.cc

#include "params.h"
#include <image/image.h>
#include <iostream>
#include <fstream>

CommonTilton::Image baseImage;
CommonTilton::Image registerImage;

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
    if (!param_fs.is_open())
    {
      cout << "Cannot open input file " << param_file << endl;
      return false;
    }

  // Read parameters from parameter file until End-of-File reached
    base_image_flag = false;
    register_image_flag = false;
    OUT_registered_image_flag = false;
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
      if (line.find("-base_image") != string::npos)
      {
        base_image_file = process_line(line,false);
        base_image_flag = true;
      }
      if (line.find("-register_image") != string::npos)
      {
        register_image_file = process_line(line,false);
        register_image_flag = true;
      }
      if (line.find("-OUT_registered_image") != string::npos)
      {
        OUT_registered_image_file = process_line(line,false);
        OUT_registered_image_flag = true;
      }
    }
    param_fs.close();

  // Exit with false status if a required parameter is not set, or an improper parameter
  // setting is detected.
    if (base_image_flag == false)
    {
      cout << "ERROR: -base_image (Input base image data file name) is required" << endl;
      return false;
    }
    if (register_image_flag == false)
    {
      cout << "ERROR: -register_image (Input mage data file name of image to be registered) is required" << endl;
      return false;
    }
    if (OUT_registered_image_flag == false)
    {
      cout << "ERROR: -OUT_registered_image (OUTPUT registered image data file name) is required" << endl;
      return false;
    }

    if (!baseImage.open(base_image_file))
    {
      cout << "ERROR:  Could not open input base image " << base_image_file << endl;
      return false;
    }
    if (!registerImage.open(register_image_file))
    {
      cout << "ERROR:  Could not open input register image " << register_image_file << endl;
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
    cout << "Formatted base input image data file = " << base_image_file << endl;
    cout << "Formatted input image data file of image to be registered = " << register_image_file << endl;
    cout << "Output image data file of registered image = " << OUT_registered_image_file << endl;

    return;
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

