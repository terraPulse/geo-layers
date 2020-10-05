// params.cc

#include "params.h"
#include <iostream>
#include <cstdlib>

CommonTilton::Image testImage;
CommonTilton::Image classifiedImage;

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

  // Read initial parameters from parameter file until End-of-File reached
    bool test_image_flag = false;
    bool classified_image_flag = false;
#ifndef GDAL
    bool ncols_flag = false;
    bool nrows_flag = false;
    bool dtype_flag = false;
#endif
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
      if (line.find("-test_image") != string::npos)
      {
        test_image_file = process_line(line,false);
        test_image_flag = true;
      }
      if (line.find("-classified_image") != string::npos)
      {
        classified_image_file = process_line(line,false);
        classified_image_flag = true;
      }
#ifndef GDAL
      if (line.find("-ncols") != string::npos)
      {
        sub_string = process_line(line,false);
        ncols = atoi(sub_string.c_str());
        ncols_flag = true;
      }
      if (line.find("-nrows") != string::npos)
      {
        sub_string = process_line(line,false);
        nrows = atoi(sub_string.c_str());
        nrows_flag = true;
      }
      if (line.find("-dtype") != string::npos)
      {
        sub_string = process_line(line,false);
        if (sub_string == "UInt8")
        {
          dtype = UInt8;
          dtype_flag = true;
        }
        else if (sub_string == "UInt16")
        {
          dtype = UInt16;
          dtype_flag = true;
        }
        else if (sub_string == "Float32")
        {
          dtype = Float32;
          dtype_flag = true;
        }
        else
        { 
          dtype = Unknown;
          dtype_flag = false;
        }
      }
#endif
    }

    param_fs.close();

// Exit with false status if a required parameter is not set, or an improper parameter
// setting is detected.
    if (!test_image_flag)
    {
      cout << "ERROR: -test_image (Input test image file) is required" << endl;
      return false;
    }
    if (!classified_image_flag)
    {
      cout << "ERROR: -classified_image (Input classified image file) is required" << endl;
      return false;
    }
#ifndef GDAL
    if (!ncols_flag)
    {
      cout << "ERROR: -ncols (Number of columns in input images) is required" << endl;
      return false;
    }
    if (!nrows_flag)
    {
      cout << "ERROR: -nrows (Number of rows in input images) is required" << endl;
      return false;
    }
    if (!dtype_flag)
    {
      cout << "ERROR: -dtype (Data type of input images) is required" << endl;
      return false;
    }
#endif
#ifdef GDAL
    if (!testImage.open(test_image_file))
#else
    if (!testImage.open(test_image_file, ncols, nrows, 1, dtype))
#endif
    {
      cout << "ERROR:  Could not open -test_image: " << test_image_file << endl;
      return false;
    }
#ifdef GDAL
    if (!classifiedImage.open(classified_image_file))
#else
    if (!classifiedImage.open(classified_image_file, ncols, nrows, 1, dtype))
#endif
    {
      cout << "ERROR:  Could not open -classified_image: " << classified_image_file << endl;
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
      cout << "Input test image file name: " << test_image_file << endl;
      cout << "Input classified file image: " << classified_image_file << endl;
#ifndef GDAL
      cout << "Input image number of columns = " <<  ncols << endl;
      cout << "Input image number of rows = " <<  nrows << endl;
      switch (dtype)
      {
        case UInt8:   cout << "Input image data type is UNSIGNED 8-bit (UInt8)" << endl;
                      break;
        case UInt16:  cout << "Input image data type is UNSIGNED 16-bit (UInt16)" << endl;
                      break;
        case Float32: cout << "Input image data type is 32-bit FLOAT (Float32)" << endl;
                      break;
        default:      cout << "Input image data type in invalid (Unknown)" << endl;
                      break;
      }
#endif
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
