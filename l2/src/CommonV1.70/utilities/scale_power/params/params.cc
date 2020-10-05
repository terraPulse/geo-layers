// params.cc

#include "params.h"
#include <iostream>
#include <cstdlib>

CommonTilton::Image inputImage;
CommonTilton::Image maskImage;

namespace CommonTilton
{

 // Constructor
  Params::Params(const string& value)
  {
    version = value;

    mask_flag = false;
    mask_value = 0;
    power_value = 0.0;
    copy_color_table_flag = false;

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
    bool mask_value_flag = false;
    bool power_value_flag = false;
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
      if (line.find("-nbands") != string::npos)
      {
        sub_string = process_line(line,false);
        nbands = atoi(sub_string.c_str());
        nbands_flag = true;
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
      if ((line.find("-mask") != string::npos) && (line.find("-mask_value") == string::npos))
      {
        mask_file = process_line(line,false);
        mask_flag = true;
      }
      if (line.find("-mask_value") != string::npos)
      {
        sub_string = process_line(line,false);
        mask_value = atoi(sub_string.c_str());
        mask_value_flag = true;
      }
      if (line.find("-power") != string::npos)
      {
        sub_string = process_line(line,false);
        power_value = atof(sub_string.c_str());
        power_value_flag = true;
      }
      if (line.find("-output_image") != string::npos)
      {
        output_image_file = process_line(line,false);
        output_image_flag = true;
      }
      if (line.find("-copy_color_table") != string::npos)
      {
        sub_string = process_line(line,false);
        sub_pos = atoi(sub_string.c_str());
        if (sub_pos == 1)
          copy_color_table_flag = true;
        else
          copy_color_table_flag = false;
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
#ifndef GDAL
    if (!ncols_flag)
    {
      cout << "ERROR: -ncols (Number of columns in input image data) is required" << endl;
      return false;
    }
    if (!nrows_flag)
    {
      cout << "ERROR: -nrows (Number of rows in input image data) is required" << endl;
      return false;
    }
    if (!nbands_flag)
    {
      cout << "ERROR: -nbands (Number of spectral bands in input image data) is required" << endl;
      return false;
    }
    if (!dtype_flag)
    {
      cout << "ERROR: -dtype (Data type of input image data) is required" << endl;
      return false;
    }
#endif
    if (!power_value_flag)
    {
      power_value = 2.0;
      return false;
    }
    if (!output_image_flag)
    {
      cout << "ERROR: -output_image (Output image file) is required" << endl;
      return false;
    }
#ifdef GDAL
    if (!inputImage.open(input_image_file))
#else
    if (!inputImage.open(input_image_file,ncols,nrows,nbands,dtype))
#endif
    {
      cout << "ERROR: Could not open -input_image: " << input_image_file << endl;
      return false;
    }
    if (mask_flag)
    {
#ifdef GDAL
      if(!maskImage.open(mask_file))
      {
        int min_value = 0;
        int max_value = 255;
        if (min_value > maskImage.getMinimum(0))
          min_value = maskImage.getMinimum(0);
        if (max_value < maskImage.getMaximum(0))
          max_value = maskImage.getMaximum(0);
        if ((min_value < 0.0) || (max_value > 255))
        {
          cout << "ERROR: Invalid range for input mask image. Must be in range 0 to 255." << endl;
          mask_flag = false;
          return false;
        }
        if ((!mask_value_flag) && (maskImage.no_data_value_valid(0)))
        {
          min_value = maskImage.get_no_data_value(0);
          if ((min_value < 256.0) && (min_value >= 0.0))
            mask_value = (unsigned char) min_value;
          else
          {
            cout << "ERROR: Mask value, " << mask_value << ", is out of 8-bit range" << endl;
            mask_flag = false;
            return false;
          }
        }
#else
      if (!maskImage.open(mask_file,ncols,nrows,nbands,UInt8))
      {
#endif
        cout << "ERROR: Could not open -mask: " << mask_file << endl;
        return false;
      }
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
#ifndef GDAL
      cout << "Input image number of columns = " <<  ncols << endl;
      cout << "Input image number of rows = " <<  nrows << endl;
      cout << "Input image number of bands = " <<  nbands << endl;
      switch (dtype)
      {
        case UInt8:   cout << "Input image data type is UNSIGNED 8-bit (UInt8)" << endl;
                      break;
        case UInt16:  cout << "Input image data type is UNSIGNED 16-bit (UInt16)" << endl;
                      break;
        case Float32: cout << "Input image data type is 32-bit FLOAT (Float32)" << endl;
                      break;
        default:      cout << "WARNING: Input image data type in invalid (Unknown)" << endl;
                      break;
      }
#endif
      cout << "Power value = " << power_value << endl;

      cout << "Output image file name: " << output_image_file << endl;
      if (copy_color_table_flag)
        cout << "Copying of color table from first input image to output image requested." << endl;

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
