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

    latitude_flag = false;
    longitude_flag = false;
    UTM_X_flag = false;
    UTM_Y_flag = false;
    lat_long_flag = false;

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
    bool WGS_84_UTM_Zone_flag = false;
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
      if (line.find("-latitude") != string::npos)
      {
        sub_string = process_line(line,false);
        latitude = atof(sub_string.c_str());
        latitude_flag = true;
      }
      if (line.find("-longitude") != string::npos)
      {
        sub_string = process_line(line,false);
        longitude = atof(sub_string.c_str());
        longitude_flag = true;
      }
      if (line.find("-UTM_X") != string::npos)
      {
        sub_string = process_line(line,false);
        UTM_X = atof(sub_string.c_str());
        UTM_X_flag = true;
      }
      if (line.find("-UTM_Y") != string::npos)
      {
        sub_string = process_line(line,false);
        UTM_Y = atof(sub_string.c_str());
        UTM_Y_flag = true;
      }
      if (line.find("-WGS_84_UTM_Zone") != string::npos)
      {
        sub_string = process_line(line,false);
        WGS_84_UTM_Zone = atoi(sub_string.c_str());
        WGS_84_UTM_Zone_flag = true;
      }
    }
    param_fs.close();
    if (latitude_flag && longitude_flag)
      lat_long_flag = true;

// Exit with false status if a required parameter is not set, or an improper parameter
// setting is detected.
    if ((!latitude_flag) && (!longitude_flag))
    {
      if ((!UTM_X_flag) || (!UTM_Y_flag))
      {
        cout << "ERROR: Both -UTM_X and -UTM_Y must be provided if -latitude and -longitude are not provided" << endl;
        return false;
      }
    }
    if ((!UTM_X_flag) && (!UTM_Y_flag))
    {
      if ((!latitude_flag) || (!longitude_flag))
      {
        cout << "ERROR: Both -latitude and -longitude must be provided if -UTM_X and -UTM_Y are not provided" << endl;
        return false;
      }
    }
    if (!WGS_84_UTM_Zone_flag)
    {
      cout << "ERROR: -WGS_84_UTM_Zone (WGS 84 / UTM Zone Number) is required" << endl;
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
      if (latitude_flag)
        cout << "Input latitude = " << latitude << endl;
      if (longitude_flag)
        cout << "Input longitude = " << longitude << endl;
      if (UTM_X_flag)
        cout << "Input UTM_X = " << UTM_X << endl;
      if (UTM_Y_flag)
        cout << "Input UTM_Y = " << UTM_Y << endl;
      cout << "WGS 84 / UTM Zone Number = " << WGS_84_UTM_Zone << endl;

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
