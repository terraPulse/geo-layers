// params.cc

#include "params.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>

CommonTilton::Image inputImage;
CommonTilton::Image maskImage;

namespace CommonTilton
{

 // Constructor
  Params::Params(const string& value)
  {
    mask_flag = false;
#ifndef GDAL
    mask_value = MASK_VALUE;
#endif

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
    bool input_image_flag = false;
#ifdef GDAL
    double min_value, max_value;
#else
    bool ncols_flag = false;
    bool nrows_flag = false;
    bool nbands_flag = false;
    bool dtype_flag = false;
#endif
    bool band_diff_image_flag = false;
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
        input_image_flag =  true;
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
      if (line.find("-mask_value") != string::npos)
      {
        sub_string = process_line(line,false);
        mask_value = atoi(sub_string.c_str());
      }
#endif
      if ((line.find("-mask") != string::npos) && (line.find("-mask_value") == string::npos))
      {
        mask_file = process_line(line,false);
        mask_flag = true;
      }
      if (line.find("-band_diff_image") != string::npos)
      {
        band_diff_image_file = process_line(line,false);
        band_diff_image_flag = true;
      }
    }
    param_fs.close();

  // Exit with false status if a required parameter is not set, or an improper parameter
  // setting is detected.
    if (input_image_flag == false)
    {
      cout << "ERROR: -input_image (Input image file name) is required" << endl;
      return false;
    }
#ifdef GDAL
    if (inputImage.open(input_image_file))
    {
      ncols = inputImage.get_ncols();
      nrows = inputImage.get_nrows();
      nbands = inputImage.get_nbands();
      dtype = inputImage.get_dtype();
    }
    else
    {
      cout << "ERROR:  Input image " << input_image_file << " is of unknown format." << endl;
      return false;
    }
#else
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
    if (!inputImage.open(input_image_file,ncols,nrows,nbands,dtype))
    {
      cout << "ERROR: Could not open -input_image: " << input_image_file << endl;
      return false;
    }
#endif
    if (band_diff_image_flag == false)
    {
      cout << "ERROR: -band_diff_image (Output band differences image file name) is required" << endl;
      return false;
    }
    if (mask_flag == false)
    {
      cout << "ERROR: -mask_image (mask image file name) is required" << endl;
      return false;
    }
#ifdef GDAL
    if (maskImage.open(mask_file))
    {
      if ((ncols != maskImage.get_ncols()) || (nrows != maskImage.get_nrows()))
      {
        cout << "ERROR: " << mask_file << " is not a valid input mask" << endl;
        cout << "Input mask must be have the same number of columns and rows as the input image" << endl;
        return false;
      }
      min_value = 0;
      max_value = 255;
      if (min_value > maskImage.getMinimum(0))
        min_value = maskImage.getMinimum(0);
      if (max_value < maskImage.getMaximum(0))
        max_value = maskImage.getMaximum(0);
      if ((min_value < 0.0) || (max_value > 255))
      {
        cout << "ERROR: Invalid range for input mask image. Must be in range 0 to 255." << endl;
        return false;
      }
      if (maskImage.no_data_value_valid(0))
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
      else
        mask_value = MASK_VALUE;
    }
    else
    {
      cout << "ERRIR:  Input mask file " << mask_file << " is of unknown format." << endl;
      return false;
    }
#else
    if (!maskImage.open(mask_file, ncols, nrows, 1, UInt8))
    {
      cout << "ERROR: Could not open -mask_image: " << mask_file << endl;
      return false;
    }
#endif

    return true;
  }

 // Print parameters
  void Params::print()
  {
   // Print version
    cout << "This is " << version << endl << endl;

   // Print input parameters
    cout << "Input image file = " << input_image_file << endl;
    cout << "Output band differences image file = " << band_diff_image_file << endl;

    return;
  }

 // Set temporary file names
  void Params::set_temp_files()
  {
   // Create prefix for temporary files
    static char time_buffer[26];
    time_t now;
    const  struct tm *tm_ptr;
    now = time(NULL);
    tm_ptr = localtime(&now);
    strftime(time_buffer,26,"%a%b%d%H%M%S%Y",tm_ptr);

    string temp_file_name = time_buffer;
    string prefix = "NB_DIFF" + temp_file_name;

  // Set temporary file names
    temp_NIR_image_file = prefix + "NIR_image";
    temp_red_image_file = prefix + "red_image";

    return;
  }

 // Remove temporary files
  void Params::remove_temp_files()
  {
    remove(temp_red_image_file.c_str());
    temp_red_image_file += ".hdr";
    remove(temp_red_image_file.c_str());

    remove(temp_NIR_image_file.c_str());
    temp_NIR_image_file += ".hdr";
    remove(temp_NIR_image_file.c_str());

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

