// params.cc

#include "params.h"
#include <iostream>
#include <cstdlib>

namespace CommonTilton
{

 // Constructor
  Params::Params(const string& value)
  {
    version = value;

    nbands = 0;
    output_format = "GTiff";

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
    bool input_HDF_flag = false;
    bool subdatasets_flag = false;
    bool output_image_flag = false;
    bool output_format_flag = false;
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
      if (line.find("-input_HDF") != string::npos)
      {
        input_HDF = process_line(line,false);
        input_HDF_flag = true;
      }
      if (line.find("-subdatasets") != string::npos)
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
          subdatasets.push_back(sub_string.c_str());
        }
        nbands = subdatasets.size();
        subdatasets_flag = true;
      }
      if (line.find("-output_image") != string::npos)
      {
        output_image = process_line(line,false);
        output_image_flag = true;
      }
      if (line.find("-output_format") != string::npos)
      {
        output_format = process_line(line,false);
        output_format_flag = true;
      }
    }
    param_fs.close();

// Exit with false status if a required parameter is not set, or an improper parameter
// setting is detected.
    if (!input_HDF_flag)
    {
      cout << "ERROR: -input_HDF (Input HDF file name) is required" << endl;
      return false;
    }
    if (!subdatasets_flag)
    {
      cout << "ERROR: -subdatasets (List of subdatasets to be extracted) is required" << endl;
      return false;
    }
    if (!output_image_flag)
    {
      cout << "ERROR: -output_image (Output image file name) is required" << endl;
      return false;
    }
    if (!output_format_flag)
    {
      output_format = "GTiff";
    }

    return true;
  }

 // Print parameters
  void Params::print()
  {
      int band;

  // Print version
      cout << "This is " << version << endl << endl;

  // Print input parameters
      cout << "Input HDF file name = " << input_HDF << endl;
      cout << "Subdatasets to be extracted:" << endl;
      for (band = 0; band < nbands; band++)
        cout << "  " << subdatasets[band] << endl;
      cout << "Output image file name = " << output_image << endl;
      cout << "Output image format = " << output_format << endl << endl;

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
