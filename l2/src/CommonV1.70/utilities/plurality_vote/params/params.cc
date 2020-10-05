// params.cc

#include "params.h"
#include <iostream>
#include <cstdlib>

CommonTilton::Image pixelClassImage;
CommonTilton::Image regionSegImage;

namespace CommonTilton
{

 // Constructor
  Params::Params(const string& value)
  {
    version = value;
#ifdef GDAL
    color_table_flag = false;
#endif

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
    bool pixel_class_flag = false;
#ifndef GDAL
    bool ncols_flag = false;
    bool nrows_flag = false;
    bool pixel_dtype_flag = false;
#endif
    bool region_seg_flag = false;
#ifndef GDAL
    bool region_dtype_flag = false;
#endif
    bool region_class_flag = false;
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
      if (line.find("-pixel_classification") != string::npos)
      {
        pixel_class_file = process_line(line,false);
        pixel_class_flag = true;
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
      if (line.find("-pixel_dtype") != string::npos)
      {
        sub_string = process_line(line,false);
        if (sub_string == "UInt8")
        {
          pixel_dtype = UInt8;
          pixel_dtype_flag = true;
        }
        else if (sub_string == "UInt16")
        {
          pixel_dtype = UInt16;
          pixel_dtype_flag = true;
        }
        else if (sub_string == "Float32")
        {
          pixel_dtype = Float32;
          pixel_dtype_flag = true;
        }
        else
        { 
          pixel_dtype = Unknown;
          pixel_dtype_flag = false;
        }
      }
#endif
      if (line.find("-region_segmentation") != string::npos)
      {
        region_seg_file = process_line(line,false);
        region_seg_flag = true;
      }
#ifdef GDAL
      if (line.find("-color_table") != string::npos)
      {
        color_table_file = process_line(line,false);
        color_table_flag = true;
      }
#else
      if (line.find("-region_dtype") != string::npos)
      {
        sub_string = process_line(line,false);
        if (sub_string == "UInt8")
        {
          region_dtype = UInt8;
          region_dtype_flag = true;
        }
        else if (sub_string == "UInt16")
        {
          region_dtype = UInt16;
          region_dtype_flag = true;
        }
        else if (sub_string == "Float32")
        {
          region_dtype = Float32;
          region_dtype_flag = true;
        }
        else
        { 
          region_dtype = Unknown;
          region_dtype_flag = false;
        }
      }
#endif
      if (line.find("-region_classification") != string::npos)
      {
        region_class_file = process_line(line,false);
        region_class_flag = true;
      }
    }

    param_fs.close();

// Exit with false status if a required parameter is not set.
    if (!pixel_class_flag)
    {
      cout << "ERROR: -pixel_classification (Input pixel-based classification) is required" << endl;
      return false;
    }
#ifndef GDAL
    if (!ncols_flag)
    {
      cout << "ERROR: -ncols (Number of columns in input pixel-based classification) is required" << endl;
      return false;
    }
    if (!nrows_flag)
    {
      cout << "ERROR: -nrows (Number of rows in input pixel-based classification) is required" << endl;
      return false;
    }
    if (!pixel_dtype_flag)
    {
      cout << "ERROR: -pixel_dtype (Data type of input pixel-based classification) is required" << endl;
      return false;
    }
#endif
    if (!region_seg_flag)
    {
      cout << "ERROR: -region_segmentation (Input region segmentation) is required" << endl;
      return false;
    }
#ifndef GDAL
    if (!region_dtype_flag)
    {
      cout << "ERROR: -region_dtype (Data type of input region segmentation) is required" << endl;
      return false;
    }
#endif
    if (!region_class_flag)
    {
      cout << "ERROR: -region_classification (Output region-based classification) is required" << endl;
      return false;
    }
 
#ifdef GDAL
    if (!pixelClassImage.open(pixel_class_file))
#else
    if (!pixelClassImage.open(pixel_class_file, ncols, nrows, 1, pixel_dtype))
#endif
    {
      cout << "ERROR:  Could not open pixel-based classification " << pixel_class_file << endl;
      return false;
    }
#ifdef GDAL
    if (!regionSegImage.open(region_seg_file))
#else
    if (!regionSegImage.open(region_seg_file, ncols, nrows, 1, region_dtype))
#endif
    {
      cout << "ERROR:  Could not open region segmentation image file " << region_seg_file << endl;
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
      cout << "Input pixel-based classification image file name: " << pixel_class_file << endl;
#ifndef GDAL
      cout << "Input pixel-based classification number of columns = " <<  ncols << endl;
      cout << "Input pixel-based classification number of rows = " <<  nrows << endl;
      switch (pixel_dtype)
      {
        case UInt8:   cout << "Input pixel-based classification data type is UNSIGNED 8-bit (UInt8)" << endl;
                      break;
        case UInt16:  cout << "Input pixel-based classification data type is UNSIGNED 16-bit (UInt16)" << endl;
                      break;
        case Float32: cout << "Input pixel-based classification data type is 32-bit FLOAT (Float32)" << endl;
                      break;
        default:      cout << "Input pixel-based classification data type in invalid (Unknown)" << endl;
                      break;
      }
#endif
      cout << "Input region segmentation image file image: " << region_seg_file << endl;
#ifdef GDAL
      if (color_table_flag)
        cout << "Color table file name: " << color_table_file << endl;
#else
      switch (region_dtype)
      {
        case UInt8:   cout << "Input region segmentation data type is UNSIGNED 8-bit (UInt8)" << endl;
                      break;
        case UInt16:  cout << "Input region segmentation data type is UNSIGNED 16-bit (UInt16)" << endl;
                      break;
        case Float32: cout << "Input region segmentation data type is 32-bit FLOAT (Float32)" << endl;
                      break;
        default:      cout << "Input region segmentation data type in invalid (Unknown)" << endl;
                      break;
      }
#endif
      cout << "Output region-based classification image file name: " << region_class_file << endl;
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
