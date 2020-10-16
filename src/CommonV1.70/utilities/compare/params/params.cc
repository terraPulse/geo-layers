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

    compare_type = Equal;
    input_image2_flag = false;
    compare_value_flag = false;
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
    bool output_value_flag = false;
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
      if (line.find("-compare_type") != string::npos)
      {
        sub_string = process_line(line,false);
        if (sub_string == "Equal")
          compare_type = Equal;
        else if (sub_string == "LessThan")
          compare_type = LessThan;
        else if (sub_string == "MoreThan")
          compare_type = MoreThan;
        else
        {
          cout << "ERROR: Invalid comparison type. Must be \"Equal,\" \"LessThan,\" or \"MoreThan\" (no quotes)" << endl;
          return false;
        }
      }
      if (line.find("-input_image2") != string::npos)
      {
        input_image2_file = process_line(line,false);
        input_image2_flag = true;
      }
      if (line.find("-compare_value") != string::npos)
      {
        sub_string = process_line(line,false);
        compare_value = atof(sub_string.c_str());
        compare_value_flag = true;
      }
      if (line.find("-output_image") != string::npos)
      {
        output_image_file = process_line(line,false);
        output_image_flag = true;
      }
      if (line.find("-output_value") != string::npos)
      {
        sub_string = process_line(line,false);
        output_value = atoi(sub_string.c_str());
        output_value_flag = true;
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
    if ((!input_image2_flag) && (!compare_value_flag))
    {
      cout << "ERROR: Either -input_image2 (Second input image file) or" << endl;
      cout << "-compare_value (value to compare input_image1 to) is required" << endl;
      return false;
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
    if (output_value_flag)
    {
      if (output_value > 255)
      {
        cout << "WARNING: Output value " << output_value << " invalid, reset to 255." << endl;
        output_value = 255;
      }
      else if (output_value < 1)
      {
        cout << "WARNING: Output value " << output_value << " invalid, reset to 1." << endl;
        output_value = 1;
      }
    }
    else
      output_value = 1;

    return true;
  }

 // Print parameters
  void Params::print()
  {
  // Print version
      cout << "This is " << version << endl << endl;

  // Print input parameters
      cout << "First input image file name: " << input_image1_file << endl;
      switch(compare_type)
      {
        case Equal:    cout << "Comparison Type: Equal" << endl;
                       break;
        case LessThan: cout << "Comparison Type: LessThan" << endl;
                       break;
        case MoreThan: cout << "Comparison Type: MoreThan" << endl;
                       break;
        default:       cout << "(Unknown Comparison Type)" << endl;
                       break;
      }
      if (input_image2_flag)
        cout << "Second input image file name: " << input_image2_file << endl;
      else if (compare_value_flag)
        cout << "Compare value = " << compare_value << endl;

      cout << "Output image file name: " << output_image_file << endl;
      cout << "Output value for true locations: " << output_value << endl;
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
