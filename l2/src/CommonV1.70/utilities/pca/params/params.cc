// params.cc

#include "params.h"
#include <iostream>
#include <cstdlib>

 // Constructor
  Params::Params(const string& value)
  {
    version = value;

    number_flag = false;
    variance = 0.95;

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

  // Read initial parameters from parameter file until End-of-File reached
    bool input_image_flag = false;
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
      if (line.find("-input_image") != string::npos)
      {
        input_image_file = process_line(line,false);
        input_image_flag = true;
      }
      if (line.find("-number_components") != string::npos)
      {
        sub_string = process_line(line,false);
        number_components = atoi(sub_string.c_str());
        number_flag = true;
      }
      if (line.find("-variance") != string::npos)
      {
        sub_string = process_line(line,false);
        variance = atof(sub_string.c_str());
        variance /= 100.0;
      }
      if (line.find("-output_image") != string::npos)
      {
        output_image_file = process_line(line,false);
        output_image_flag = true;
      }
    }
    param_fs.close();

// Exit with false status if a required parameter is not set, or an improper parameter
// setting is detected.
    if (!input_image_flag)
    {
      cout << "ERROR: -input_image (Input image file) is required" << endl;
      return false;
    }
    if (!output_image_flag)
    {
      cout << "ERROR: -output_image (Output image file name) is required" << endl;
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
      cout << "Input image file name: " << input_image_file << endl;
      if (number_flag)
        cout << "Number of components retained = " << number_components << endl;
      else
        cout << "Retained percent variance = " << (variance*100) << endl;
      cout << "Output image file name = " << output_image_file << endl;

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
