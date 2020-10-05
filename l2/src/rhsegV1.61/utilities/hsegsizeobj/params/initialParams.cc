// initialParams.cc

#include "initialParams.h"
#include <params/params.h>
#include <iostream>
#include <fstream>

// Globals

// Externals
extern HSEGTilton::Params params;
extern HSEGTilton::oParams oparams;

namespace HSEGTilton
{

 // Constructor
  InitialParams::InitialParams()
  {

   // Flags for parameters under user control.

    return;
  }

 // Destructor...
  InitialParams::~InitialParams()
  {
    return;
  }

 // Read parameters
  bool InitialParams::read(const char *param_file)
  {
//    cout << endl << "Parameters to be read from file " << param_file << endl << endl;
    ifstream param_fs(param_file);
    if (!param_fs)
    {
      cout << "Cannot open input file " << param_file << endl;
      return false;
    }

  // Read user defined parameters from parameter file until End-of-File reached
    bool oparam_flag = false;
    sizenpix_flag = false;
    sizeobj_flag = false;
    bool sizethres_flag = false;
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
      if (line.find("-oparam") != string::npos)
      {
        oparam_file = process_line(line,false);
        oparam_flag = true;
      }
      if (line.find("-sizenpix") != string::npos)
      {
        sizenpix_file = process_line(line,false);
        sizenpix_flag = true;
      }
      if (line.find("-sizeobj") != string::npos)
      {
        sizeobj_file = process_line(line,false);
        sizeobj_flag = true;
      }
      if (line.find("-sizethres") != string::npos)
      {
        sizethres_flag = true;
        sub_string = process_line(line,false);
        int comma_pos;
        levels = 0;
        while(sub_string.find(",") != std::string::npos) {
          comma_pos = sub_string.find(",");
          sizethres[levels] = atoi(sub_string.substr(0,comma_pos).c_str());
          levels += 1;
          sub_string = sub_string.substr(comma_pos+1);
        }
        sizethres[levels] = atoi(sub_string.c_str());
        levels += 1;
      }
    }
    param_fs.close();

// Exit with false status if a required parameter is not set.
    if (!oparam_flag)
    {
      cout << "ERROR: -oparam (RHSeg output parameter file) is required" << endl;
      return false;
    }
    if (!sizeobj_flag)
    {
      cout << "ERROR: -sizeobj (Output data file) is required" << endl;
      return false;
    }
    if (!sizethres_flag)
    {
      cout << "ERROR: -sizethres (A list of size thresholds) is required" << endl;
      return false;
    }

   // Read initial parameters from RHSeg output parameter file
    if (!params.read_init(oparam_file.c_str()))
      return false;

   // Read additional parameters from RHSeg output parameter file
    if (!params.read(oparam_file.c_str()))
      return false;

   // Read "output" parameters from RHSeg output parameter file
    if (!read_oparam())
      return false;

    return true;
  }

 // Read parameters
  bool InitialParams::read_oparam()
  {
//    cout << endl << "Output parameters to be read from file " << oparam_file << endl;
    bool status=false;

    ifstream oparam_fs;
    oparam_fs.open(oparam_file.c_str());
    if (!oparam_fs.is_open())
    {
      cout << "Cannot open RHSeg output parameter file " << oparam_file << endl;
      return status;
    }

    params.scale.resize(params.nbands);
    params.offset.resize(params.nbands);
    oparams.scale.resize(params.nbands);
    oparams.offset.resize(params.nbands);

    int band;
    int sub_pos;
    string line, sub_string;
    while (!oparam_fs.eof())
    {
      getline(oparam_fs,line);
      sub_pos = line.find("-");
      while ((!oparam_fs.eof()) && (sub_pos != 0))
      {
        getline(oparam_fs,line);
        sub_pos = line.find("-");
      }
      if (line.find("-input_scale") != string::npos)
      {
        line = process_line(line,true);
        band = 0;
        while (line.size() > 0)
        {
          if (band > params.nbands)
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
          params.scale[band] = atof(sub_string.c_str());
          band++;
        }
      }
      if (line.find("-input_offset") != string::npos)
      {
        line = process_line(line,true);
        band = 0;
        while (line.size() > 0)
        {
          if (band > params.nbands)
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
          params.offset[band] = atof(sub_string.c_str());
          band++;
        }
      }
      if (line.find("-norm_scale") != string::npos)
      {
        line = process_line(line,true);
        band = 0;
        while (line.size() > 0)
        {
          if (band > params.nbands)
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
          oparams.scale[band] = atof(sub_string.c_str());
          band++;
        }
      }
      if (line.find("-norm_offset") != string::npos)
      {
        line = process_line(line,true);
        band = 0;
        while (line.size() > 0)
        {
          if (band > params.nbands)
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
          oparams.offset[band] = atof(sub_string.c_str());
          band++;
        }
      }
      if (line.find("-maxnbdir") != string::npos)
      {
        sub_string = process_line(line,false);
        params.maxnbdir = atoi(sub_string.c_str());
      }
      if (line.find("-region_sumsq") != string::npos)
      {
        sub_string = process_line(line,false);
        sub_pos = atoi(sub_string.c_str());
        if (sub_pos == 1)
          params.region_sumsq_flag = true;
        else
          params.region_sumsq_flag = false;
      }
      if (line.find("-region_sumxlogx") != string::npos)
      {
        sub_string = process_line(line,false);
        sub_pos = atoi(sub_string.c_str());
        if (sub_pos == 1)
          params.region_sumxlogx_flag = true;
        else
          params.region_sumxlogx_flag = false;
      }
      if (line.find("-nb_levels") != string::npos)
      {
        sub_string = process_line(line,false);
        oparams.nb_levels = atoi(sub_string.c_str());
      }
      if (line.find("-level0_nb_classes") != string::npos)
      {
        sub_string = process_line(line,false);
        oparams.level0_nb_classes = atoi(sub_string.c_str());
      }
      if (line.find("-level0_nb_objects") != string::npos)
      {
        sub_string = process_line(line,false);
        oparams.level0_nb_objects = atoi(sub_string.c_str());
      }
    }

    oparam_fs.close();

    ifstream param_fs;
    param_fs.open(oparam_file.c_str());
    if (!param_fs.is_open())
    {
      cout << "Cannot (re)open RHSeg output parameter file " << oparam_file << endl;
      return status;
    }

    while (!param_fs.eof())
    {
      getline(param_fs,line);
      sub_pos = line.find("-");
      while ((!param_fs.eof()) && (sub_pos != 0))
      {
        getline(param_fs,line);
        sub_pos = line.find("-");
      }
      if (line.find("-int_buffer_size") != string::npos)
      {
        oparams.int_buffer_size.resize(oparams.nb_levels);
        line = process_line(line,true);
        band = 0;
        while (line.size() > 0)
        {
          if (band > oparams.nb_levels)
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
          oparams.int_buffer_size[band] = atoi(sub_string.c_str());
          band++;
        }
      }
      if (line.find("-max_threshold") != string::npos)
      {
        oparams.max_threshold.resize(oparams.nb_levels);
        line = process_line(line,true);
        band = 0;
        while (line.size() > 0)
        {
          if (band > oparams.nb_levels)
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
          oparams.max_threshold[band] = atof(sub_string.c_str());
          band++;
        }
      }
      if (line.find("-gdissim") != string::npos)
      {
        oparams.gdissim.resize(oparams.nb_levels);
        line = process_line(line,true);
        band = 0;
        while (line.size() > 0)
        {
          if (band > oparams.nb_levels)
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
          oparams.gdissim[band] = atof(sub_string.c_str());
          band++;
        }
      }
    }

    param_fs.close();

    return true;
  }

  // Print parameters
  void InitialParams::print()
  {
   // Print input parameters
    unsigned int i;
    cout << "RHSeg output parameter file: " << oparam_file << endl;
    cout << "Output number of pixels file name: " << sizenpix_file << endl;
    cout << "Output object label file name: " << sizeobj_file << endl;
    cout << "Size thresholds: ";
    for (i=0;i<levels-1;i++) cout << sizethres[i] << ", ";
    cout << sizethres[levels-1] << endl;
    return;
  }

 // Print param and oparam values
  void InitialParams::print_oparam()
  {
      int band, index;

  // Print input parameters
      cout << endl << "Required input parameters that specify and describe the input data:" << endl;
      cout << "Input image file name: " << params.input_image_file << endl;
      cout << "Input image number of columns = " <<  params.ncols << endl;
      cout << "Input image number of rows = " <<  params.nrows << endl;
      cout << "Input image number of bands = " <<  params.nbands << endl;
      switch (params.data_type)
      {
        case GDT_Byte:    cout << "Input image data type is UNSIGNED 8-bit (GDT_Byte)" << endl;
                          break;
        case GDT_UInt16:  cout << "Input image data type is UNSIGNED 16-bit (GDT_UInt16)" << endl;
                          break;
        case GDT_Int16:   cout << "Input image data type is SIGNED 16-bit (GDT_Int16)" << endl;
                          break;
        case GDT_UInt32:  cout << "Input image data type is UNSIGNED 32-bit (GDT_UInt32)" << endl;
                          break;
        case GDT_Int32:   cout << "Input image data type is SIGNED 32-bit (GDT_Int32)" << endl;
                          break;
        case GDT_Float32: cout << "Input image data type is 32-bit FLOAT (GDT_Float32)" << endl;
                          break;
        case GDT_Float64: cout << "Input image data type is 64-bit DOUBLE FLOAT (GDT_Float64)" << endl;
                          break;
        default:          cout << "Input image data type in invalid (Unknown)" << endl;
                          break;
      }

      cout << endl << "Optional input parameters that specify optional input files and parameter values:" << endl;
      if (params.nbands < 10)
      {
        cout << "Input data scale factors: ";
        for (band = 0; band < params.nbands; band++)
          cout << params.scale[band] << " ";
        cout << endl;
        cout << "Input data offset factors: ";
        for (band = 0; band < params.nbands; band++)
          cout << params.offset[band] << " ";
        cout << endl;
        cout << "Normalization scale factors: ";
        for (band = 0; band < params.nbands; band++)
          cout << oparams.scale[band] << " ";
        cout << endl;
        cout << "Normalization offset factors: ";
        for (band = 0; band < params.nbands; band++)
          cout << oparams.offset[band] << " ";
        cout << endl;
      }
      if (params.mask_flag)
      {
        cout << "Input bad data mask file name: " << params.mask_file << endl;
        cout << "Bad data mask flag value = " << params.mask_value << endl;
      }

      cout << endl << "Input parameters that specify output files:" << endl;
      cout << "Output class labels map file name: " << params.class_labels_map_file << endl;
      if (params.boundary_map_flag)
        cout << "Optional output hierarchical boundary map file name: " << params.boundary_map_file << endl;
      cout << "Output region classes file name: " << params.region_classes_file << endl;
      if (params.region_objects_list_flag)
      {
        cout << endl << "Optional output files:" << endl;
        if (params.object_labels_map_flag)
          cout << "Output object labels map file name: " << params.object_labels_map_file << endl;
        if (params.region_objects_flag)
          cout << "Output region objects file name: " << params.region_objects_file << endl;
      }
      cout << endl << "Optional parameters that select the contents of the output" << endl;
      cout << "region class (and object) files (above):" << endl;
      if (params.region_sum_flag)
      {
        cout << "Region sum (and, if available, sumsq and sumxlogx) information included." << endl;
        if (params.region_sumsq_flag)
          cout << "Region sumsq information included." << endl;
        if (params.region_sumxlogx_flag)
          cout << "Region sumxlogx information included." << endl;
      }
      if (params.region_std_dev_flag)
        cout << "Region standard deviation information included" << endl;
      if (params.region_boundary_npix_flag)
        cout << "Region region boundary number of pixels information included" << endl;
      if (params.region_threshold_flag)
        cout << "Region most recent merge threshold information included" << endl;
      if (params.region_nghbrs_list_flag)
        cout << "List of region clases neighboring each region class included" << endl;
      if (params.region_nb_objects_flag)
        cout << "Number of region objects contained in each region class included" << endl;
      if (params.region_objects_list_flag)
        cout << "List of region objects contained in each region class included" << endl;

      cout << endl << "Defaultable required input parameters, along with other associated parameters:" << endl;
      cout << "Number of Nearest Neighbors = " <<  params.maxnbdir << endl;

      cout << endl << "Parameters with recommended default values:" << endl;
      cout << "Number of output hierarchical levels = " << oparams.nb_levels << endl;
      cout << "Number of region classes at hierarchical level 0 = " << oparams.level0_nb_classes << endl;
      if (params.region_nb_objects_flag)
        cout << "Number of region objects at hierarchical level 0 = " << oparams.level0_nb_objects << endl;
      cout << "int_buffer_size: ";
      for (index = 0; index < (oparams.nb_levels-1); index++)
        cout << oparams.int_buffer_size[index] << ", ";
      cout << oparams.int_buffer_size[oparams.nb_levels-1] << endl;
      if (params.gdissim_flag)
      {
        cout << "gdissim: ";
        for (index = 0; index < (oparams.nb_levels-1); index++)
          cout << oparams.gdissim[index] << ", ";
        cout << oparams.gdissim[oparams.nb_levels-1] << endl;
      }
      cout << "max_threshold: ";
      for (index = 0; index < (oparams.nb_levels-1); index++)
        cout << oparams.max_threshold[index] << ", ";
      cout << oparams.max_threshold[oparams.nb_levels-1] << endl;


      return;
  }

} // namespace HSEGTilton
