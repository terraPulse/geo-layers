// params.cc

#include "params.h"
#include <image/image.h>
#include <shape/shape.h>
#include <iostream>
#include <fstream>

// Externals
extern CommonTilton::Image inputImage;

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
    Shape shapeFile;

//    cout << endl << "Parameters to be read from file " << param_file << endl << endl;
    ifstream param_fs(param_file);
    if (!param_fs)
    {
      cout << "Cannot open input file " << param_file << endl;
      return false;
    }

  // Read user defined parameters from parameter file until End-of-File reached
    bool input_image_flag = false;
    bool shape_flag = false;
    bool rgb_flag = false;
    bool output_image_flag = false;
    int sub_pos, band;
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
      if (line.find("-shape_file") != string::npos)
      {
        shape_file = process_line(line,false);
        shape_flag = true;
      }
      if (line.find("-RGB") != string::npos)
      {
        line = process_line(line,true);
        band = 0;
        while (line.size() > 0)
        {
          if (band >= 3)
            break;
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
          if (band==0)
            red = atoi(sub_string.c_str());
          if (band==1)
            green = atoi(sub_string.c_str());
          if (band==2)
            blue = atoi(sub_string.c_str());
          band++;
        }
        rgb_flag = true;
      }
      if (line.find("-output_image") != string::npos)
      {
        output_image_file = process_line(line,false);
        output_image_flag = true;
      }
    }
    param_fs.close();

  // Exit with false status if a required parameter is not set.
    if (input_image_flag == false)
    {
      cout << "ERROR: -input_image (Input image file name) is required" << endl;
      return false;
    }
    if (shape_flag == false)
    {
      cout << "ERROR: -shape_file (Input shape file name) is required" << endl;
      return false;
    }
    if (rgb_flag == false)
    {
      cout << "ERROR: -RGB (red, green and blue values) for the input shape file is required" << endl;
      return false;
    }
    if (output_image_flag == false)
    {
      cout << "ERROR: -output_image (output image file name) is required" << endl;
      return false;
    }

    inputImage.open(input_image_file);
    if (!inputImage.info_valid())
    {
      cout << "ERROR: " << input_image_file << " is not a valid input image" << endl;
      return false;
    }  
    shapeFile.open(shape_file);
    if (shapeFile.valid())
    {
      shapeFile.close();
    }
    else
    {
      cout << "ERROR: " << shape_file << " is not a valid Input Shape File" << endl;
      return false;
    }

    return true;
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
    string prefix = "SO" + temp_file_name;

  // Set temporary file names
    temp_output_image_file = prefix + "output_image";

    return;
  }

 // Print parameters
 void Params::print()
 {
  // Print version
   cout << "This is " << version << endl << endl;

  // Print input parameters
   cout << "Input image file = " << input_image_file << endl;
   cout << "Input shape file = " << shape_file << endl;
   cout << "Red, green, blue values of overlay display: " << red << ", " << green << ", " << blue << endl;
   cout << "Output image file = " << output_image_file << endl;

   return;
 }

 // Remove temporary files
  void Params::remove_temp_files()
  {

    remove(temp_output_image_file.c_str());
    temp_output_image_file += ".hdr";
    remove(temp_output_image_file.c_str());

    return;
  }

  string process_line(const string& line, const bool& list_flag)
  {
    int sub_pos, sub_pos_space, sub_pos_tab;
    string sub_string;

    sub_pos_space = line.find_first_of(" ");
    sub_pos_tab = line.find_first_of("\t");
    sub_pos = sub_pos_space;
    if (sub_pos == (int) string::npos)
      sub_pos = sub_pos_tab;
    if (sub_pos == (int) string::npos)
      return " ";

    sub_string = line.substr(sub_pos);
    while ((sub_string.substr(0,1) == "\t") || (sub_string.substr(0,1) == " "))
      sub_string = line.substr(++sub_pos);

    if (list_flag)
      return sub_string;

    if (sub_string.substr(0,1) == "\"")
    {
      sub_string = line.substr(++sub_pos);
      sub_pos = sub_string.find_first_of("\"");
      if (sub_pos !=  (int) string::npos)
        sub_string = sub_string.substr(0,sub_pos);
    }
    else
    {
#ifndef WINDOWS
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
    }

    return sub_string;
  }

} // namespace CommonTilton

