// params.cc

#include "params.h"
#include <image/image.h>
#include <shape/shape.h>
#include <iostream>
#include <fstream>

// Externals
extern CommonTilton::Image maskImage;
extern CommonTilton::Shape shapeFile;

namespace CommonTilton
{

 // Constructor
  Params::Params(const string& value)
  {
    field_names_flag = false;
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
    if (!param_fs)
    {
      cout << "Cannot open input parameter file " << param_file << endl;
      return false;
    }

  // Read user defined parameters from parameter file until End-of-File reached
    bool mask_flag = false;
    bool shape_flag = false;
    bool ascii_shape_flag = false;
    int index, sub_pos;
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
      if (line.find("-mask_image") != string::npos)
      {
        mask_file = process_line(line,false);
        mask_flag = true;
      }
      if (line.find("-shape_file_in") != string::npos)
      {
        shape_file = process_line(line,false);
        shapeFile.open(shape_file);
        if (!shapeFile.valid())
        {
          cout << "ERROR: " << shape_file << " is not a valid Input Shape File" << endl;
          return false;
        }
        if ((shapeFile.get_shapeType() != SHPT_POINT) &&
            (shapeFile.get_shapeType() != SHPT_POLYGON) &&
            (shapeFile.get_shapeType() != SHPT_POLYGONZ))
        {
cout << "get_shapeType = " << shapeFile.get_shapeType() << endl;
          cout << "ERROR: Shape type must be SHPT_POINT, SHPT_POLYGON, or SHPT_POLYGONZ" << endl;
          return false;
        }
        shape_flag = true;
      }
      if (line.find("-field_names") != string::npos)
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
          field_names.push_back(sub_string);
        }
        field_names_flag = true;
      }
      if (line.find("-ascii_shape_out") != string::npos)
      {
        ascii_shape_file = process_line(line,false);
        ascii_shape_flag = true;
      }
    }
    param_fs.close();

  // Exit with false status if a required parameter is not set.
    if (mask_flag == false)
    {
      cout << "ERROR: -mask_image (Input mask image file name) is required" << endl;
      return false;
    }
    if (shape_flag == false)
    {
      cout << "ERROR: -shape_file_in (Input shape file name) is required" << endl;
      return false;
    }
    if (ascii_shape_flag == false)
    {
      cout << "ERROR: -ascii_shape_out (Output ASCII format shapefile) is required" << endl;
      return false;
    }
    if (field_names_flag)
    {
      sub_pos = field_names.size();
      for (index = 0; index < sub_pos; index++)
        if (shapeFile.get_fieldIndex(field_names[index].c_str()) == -1)
        {
          cout << "ERROR: Field Name " << field_names[index].c_str() << " not found in DBF file" << endl;
          return false;
        }
    }
    
    maskImage.open(mask_file);
    if (!maskImage.info_valid())
    {
      cout << "ERROR: " << mask_file << " is not a valid input mask image" << endl;
      return false;
    }  

    return true;
 }

 // Print parameters
 void Params::print()
 {
   int index, size;
  // Print version
   cout << "This is " << version << endl << endl;

  // Print input parameters
   cout << "Input mask image file = " << mask_file << endl;
   cout << "Input shape file = " << shape_file << endl;
   if (field_names_flag)
   {
     cout << "Filed names used in translation: " << endl;
     size = field_names.size();
     for (index = 0; index < size; index++)
     {
       cout << "    " << field_names[index] << endl;
     }
   }
   cout << "Output ASCII format shape file = " << ascii_shape_file << endl;

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

