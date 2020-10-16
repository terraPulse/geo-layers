// params.cc

#include "params.h"
#include <iostream>

CommonTilton::Image inputImage1;
CommonTilton::Image inputImage2;

namespace CommonTilton
{

 // Constructor
  Params::Params(const string& value)
  {
    version = value;

    input_image2_flag = false;
    multiply_value_flag = false;
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
    bool input_image1_flag = false;
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
      if (line.find("-input_image1") != string::npos)
      {
        input_image1_file = process_line(line,false);
        input_image1_flag = true;
      }
      if (line.find("-input_image2") != string::npos)
      {
        input_image2_file = process_line(line,false);
        input_image2_flag = true;
      }
      if (line.find("-multiply_value") != string::npos)
      {
        sub_string = process_line(line,false);
        multiply_value = atof(sub_string.c_str());
        multiply_value_flag = true;
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
    if (!input_image1_flag)
    {
      cout << "ERROR: -input_image1 (First input image file) is required" << endl;
      return false;
    }
    if (input_image2_flag)
    {
       if (multiply_value_flag)
         cout << "WARNING: -multiply_value is ignored because -input_image2 is provided" << endl;
    }
    else
    {
      if (!multiply_value_flag)
      {
        cout << "ERROR: Either -input_image2 (Second input image file) or" << endl;
        cout << "-multiply_value (value to add to input_image1) is required" << endl;
        return false;
      }
    }
    if (!output_image_flag)
    {
      cout << "ERROR: -output_image (Output image file) is required" << endl;
      return false;
    }

    if (!inputImage1.open(input_image1_file))
    {
      cout << "ERROR: Could not open -input_image1: " << input_image1_file << endl;
      return false;
    }
    if (input_image2_flag)
    {
      if (!inputImage2.open(input_image2_file))
      {
        cout << "ERROR: Could not open -input_image2: " << input_image2_file << endl;
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
      cout << "First input image file name: " << input_image1_file << endl;
      if (input_image2_flag)
        cout << "Second input image file name: " << input_image2_file << endl;
      else if (multiply_value_flag)
        cout << "Multiply value = " << multiply_value << endl;

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
