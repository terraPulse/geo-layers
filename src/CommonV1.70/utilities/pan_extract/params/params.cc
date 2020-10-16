// params.cc

#include "params.h"
#include <image/image.h>
#include <iostream>
#include <fstream>

CommonTilton::Image multispectralImage;
CommonTilton::Image panchromaticImage;

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
    multispectral_image_flag = false;
    panchromatic_image_flag = false;
    extracted_panchromatic_image_flag = false;
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
      if (line.find("-multispectral_image") != string::npos)
      {
        multispectral_image_file = process_line(line,false);
        multispectral_image_flag =  true;
      }
      if (line.find("-panchromatic_image") != string::npos)
      {
        panchromatic_image_file = process_line(line,false);
        panchromatic_image_flag = true;
      }
      if (line.find("-extracted_panchromatic_image") != string::npos)
      {
        extracted_panchromatic_image_file = process_line(line,false);
        extracted_panchromatic_image_flag = true;
      }
    }
    param_fs.close();

  // Exit with false status if a required parameter is not set, or an improper parameter
  // setting is detected.
    if (multispectral_image_flag == false)
    {
      cout << "ERROR: -multispectral_image (Input multispectral image data file name) is required" << endl;
      return false;
    }
    if (panchromatic_image_flag == false)
    {
      cout << "ERROR: -panchromatic_image (Input panchromatic image data file name of image to be extracted from) is required" << endl;
      return false;
    }
    if (extracted_panchromatic_image_flag == false)
    {
      cout << "ERROR: -extracted_panchromatic_image (OUTPUT extracted panchromatic image data file name) is required" << endl;
      return false;
    }

    if (!multispectralImage.open(multispectral_image_file))
    {
      cout << "ERROR:  Could not open -multispectral_image: " << multispectral_image_file << endl;
      return false;
    }
    if (!panchromaticImage.open(panchromatic_image_file))
    {
      cout << "ERROR:  Could not open -panchromatic_image: " << panchromatic_image_file << endl;
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
    cout << "Formatted input multispectral input image data file = " << multispectral_image_file << endl;
    cout << "Formatted input panchromatic image data file = " << panchromatic_image_file << endl;
    cout << "Output extracted panchromatic image data file = " << extracted_panchromatic_image_file << endl;

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

