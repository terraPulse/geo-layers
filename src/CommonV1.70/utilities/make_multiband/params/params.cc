// params.cc

#include "params.h"
#include <iostream>
#include <cstdlib>

CommonTilton::Image inputImage;

namespace CommonTilton
{

 // Constructor
  Params::Params(const string& value)
  {
    version = value;

    nbands = 0;

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
    bool input_base_flag = false;
    bool suffix_list_flag = false;
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
      if (line.find("-input_base") != string::npos)
      {
        input_base = process_line(line,false);
        input_base_flag = true;
      }
      if (line.find("-suffix_list") != string::npos)
      {
        line = process_line(line,true);
        while (line.size() > 0)
        {
          sub_pos = line.find_first_of(",");
          if (sub_pos != (int) string::npos)
          {
            sub_string = line.substr(0,sub_pos);
            line = line.substr(sub_pos+1);
          }
          else
          {
            sub_string = line;
            line = "";
          }
          suffix_list.push_back(sub_string.c_str());
        }
        nbands = suffix_list.size();
        suffix_list_flag = true;
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
    if (!input_base_flag)
    {
      cout << "ERROR: -input_base (Base of multiple input images) is required" << endl;
      return false;
    }
    if (!suffix_list_flag)
    {
      cout << "ERROR: -suffix_list (List of suffixes of the multiband images) is required" << endl;
      return false;
    }
    if (!output_image_flag)
    {
      cout << "ERROR: -output_image (Output image file name) is required" << endl;
      return false;
    }
    string input_image_file = input_base + suffix_list[0];
    if (!inputImage.open(input_image_file))
    {
      cout << "ERROR: Could not open first input_image: " << input_image_file << endl;
      return false;
    }

    return true;
  }

 // Print parameters
  void Params::print()
  {
     int band;
     string input_image_file;
  // Print version
     cout << "This is " << version << endl << endl;

  // Print input parameters
     cout << "Input image file names: " << endl;
     for (band = 0; band < nbands; band++)
     {
       input_image_file = input_base + suffix_list[band];
       cout << input_image_file << endl;
     }
     cout << endl << "Output image file name: " << output_image_file << endl;

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
