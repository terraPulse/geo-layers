// initialParams.cc

#include "initialParams.h"
#include <params/params.h>
#include <iostream>
#include <cstdlib>

extern HSEGTilton::Params params;
extern HSEGTilton::oParams oparams;

namespace HSEGTilton
{

 // Constructor
  InitialParams::InitialParams()
  {
    oparam_flag = false;
    class_labels_map_ext_flag = false;
    class_npix_map_ext_flag = false;
    class_mean_map_ext_flag = false;
    class_std_dev_map_ext_flag = false;
    class_bpratio_map_ext_flag = false;
#ifdef SHAPEFILE
    class_shapefile_ext_flag = false;
#endif
    object_labels_map_ext_flag = false;
    object_npix_map_ext_flag = false;
    object_mean_map_ext_flag = false;
    object_std_dev_map_ext_flag = false;
    object_bpratio_map_ext_flag = false;
#ifdef SHAPEFILE
    object_shapefile_ext_flag = false;
#endif

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
    bool hseg_level_flag = false;
    unsigned int sub_pos;
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
      if (line.find("-hseg_level") != string::npos)
      {
        sub_string = process_line(line,false);
        hseg_level = atoi(sub_string.c_str());
        hseg_level_flag = true;
      }
      if (line.find("-class_labels_map_ext") != string::npos)
      {
        class_labels_map_ext_file = process_line(line,false);
        class_labels_map_ext_flag = true;
      }
      if (line.find("-class_npix_map_ext") != string::npos)
      {
        class_npix_map_ext_file = process_line(line,false);
        class_npix_map_ext_flag = true;
      }
      if (line.find("-class_mean_map_ext") != string::npos)
      {
        class_mean_map_ext_file = process_line(line,false);
        class_mean_map_ext_flag = true;
      }
      if (line.find("-class_std_dev_map_ext") != string::npos)
      {
        class_std_dev_map_ext_file = process_line(line,false);
        class_std_dev_map_ext_flag = true;
      }
      if (line.find("-class_bpratio_map_ext") != string::npos)
      {
        class_bpratio_map_ext_file = process_line(line,false);
        class_bpratio_map_ext_flag = true;
      }
#ifdef SHAPEFILE
      if (line.find("-class_shapefile_ext") != string::npos)
      {
        class_shapefile_ext_file = process_line(line,false);
        class_shapefile_ext_flag = true;
      }
#endif
      if (line.find("-object_labels_map_ext") != string::npos)
      {
        object_labels_map_ext_file = process_line(line,false);
        object_labels_map_ext_flag = true;
      }
      if (line.find("-object_npix_map_ext") != string::npos)
      {
        object_npix_map_ext_file = process_line(line,false);
        object_npix_map_ext_flag = true;
      }
      if (line.find("-object_mean_map_ext") != string::npos)
      {
        object_mean_map_ext_file = process_line(line,false);
        object_mean_map_ext_flag = true;
      }
      if (line.find("-object_std_dev_map_ext") != string::npos)
      {
        object_std_dev_map_ext_file = process_line(line,false);
        object_std_dev_map_ext_flag = true;
      }
      if (line.find("-object_bpratio_map_ext") != string::npos)
      {
        object_bpratio_map_ext_file = process_line(line,false);
        object_bpratio_map_ext_flag = true;
      }
#ifdef SHAPEFILE
      if (line.find("-object_shapefile_ext") != string::npos)
      {
        object_shapefile_ext_file = process_line(line,false);
        object_shapefile_ext_flag = true;
      }
#endif
    }
    param_fs.close();

    if (!oparam_flag)
    {
      cout << "ERROR: -oparam (RHSeg output parameter file) is required" << endl;
      return false;
    }
    if (!hseg_level_flag)
    {
      cout << "ERROR: -hseg_level (Hierarchical segmentation level) is required" << endl;
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

    if (hseg_level >= oparams.nb_levels)
    {
      cout << "ERROR: -hseg_level at " << hseg_level << " is out of range" << endl;
      cout << "       hseg_level must be >= 0 and < " << oparams.nb_levels << endl;
      return false;
    }

    if ((!params.region_sum_flag) && class_mean_map_ext_flag)
    {
      cout << "WARNING: output class mean map cannot be produced - no region class sum information available!" << endl;
      class_mean_map_ext_flag = false;
    }

    if ((!params.region_std_dev_flag) && class_std_dev_map_ext_flag)
    {
      cout << "WARNING: output class standard deviation map cannot be produced - no region class standard deviation information available!" << endl;
      class_std_dev_map_ext_flag = false;
    }

    if ((!params.region_boundary_npix_flag) && class_bpratio_map_ext_flag)
    {
      cout << "WARNING: output class boundary pixel ratio map cannot be produced - no region class boundary pixel information available!" << endl;
      class_bpratio_map_ext_flag = false;
    }
    if (!params.object_labels_map_flag)
    {
      if ((object_labels_map_ext_flag) || (object_npix_map_ext_flag) || (object_mean_map_ext_flag) ||
#ifdef SHAPEFILE
          (object_shapefile_ext_flag) ||
#endif
          (object_std_dev_map_ext_flag) || (object_bpratio_map_ext_flag))
        cout << "WARNING: output object feature value maps ignored - no object feature information available!" << endl;
      object_labels_map_ext_flag = false;
      object_npix_map_ext_flag = false;
      object_mean_map_ext_flag = false;
      object_std_dev_map_ext_flag = false;
      object_bpratio_map_ext_flag = false;
#ifdef SHAPEFILE
      object_shapefile_ext_flag = false;
#endif
    }

    if ((!params.region_sum_flag) && object_mean_map_ext_flag)
    {
      cout << "WARNING: output object mean map cannot be produced - no region object sum information available!" << endl;
      object_mean_map_ext_flag = false;
    }

    if ((!params.region_std_dev_flag) && object_std_dev_map_ext_flag)
    {
      cout << "WARNING: output object standard deviation map cannot be produced - no region object standard deviation information available!" << endl;
      object_std_dev_map_ext_flag = false;
    }

    if ((!params.region_boundary_npix_flag) && object_bpratio_map_ext_flag)
    {
      cout << "WARNING: output object boundary pixel ratio map cannot be produced - no region object boundary pixel information available!" << endl;
      object_bpratio_map_ext_flag = false;
    }
    if ((!class_labels_map_ext_flag) && (!class_npix_map_ext_flag) && (!class_mean_map_ext_flag) &&
        (!class_std_dev_map_ext_flag) && (!class_bpratio_map_ext_flag) &&
        (!object_labels_map_ext_flag) && (!object_npix_map_ext_flag) && (!object_mean_map_ext_flag) &&
#ifdef SHAPEFILE
        (!class_shapefile_ext_flag) && (!object_shapefile_ext_flag) &&
#endif
        (!object_std_dev_map_ext_flag) && (!object_bpratio_map_ext_flag))
    {
      cout << "ERROR: You must specify at least one output file" << endl;
      return false;
    }

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
        params.gdissim_flag = true;
      }
    }

    param_fs.close();

    return true;
  }

 // Print parameters
  void InitialParams::print()
  {

    // Print version
      cout << "This is " << params.version << endl << endl;
    // Print input parameters
      cout << "RHSeg output parameter file:" << oparam_file << endl;
      cout << "Hierarchical segmentation level for extracting selected feature values:" << hseg_level << endl;
      if (class_labels_map_ext_flag)
        cout << "File name of extracted class labels map:" << class_labels_map_ext_file << endl;
      if (class_npix_map_ext_flag)
        cout << "File name of extracted class number of pixels map:" << class_npix_map_ext_file << endl;
      if (class_mean_map_ext_flag)
        cout << "File name of extracted class mean feature map:" << class_mean_map_ext_file << endl;
      if (class_std_dev_map_ext_flag)
        cout << "File name of extracted class std. dev. feature map:" << class_std_dev_map_ext_file << endl;
      if (class_bpratio_map_ext_flag)
        cout << "File name of extracted class boundary pixel ratio feature map:" << class_bpratio_map_ext_file << endl;
#ifdef SHAPEFILE
      if (class_shapefile_ext_flag)
        cout << "Base file name of extracted class shapefile:" << class_shapefile_ext_file << endl;
#endif
      if (object_labels_map_ext_flag)
        cout << "File name of extracted object labels map:" << object_labels_map_ext_file << endl;
      if (object_npix_map_ext_flag)
        cout << "File name of extracted object number of pixels map:" << object_npix_map_ext_file << endl;
      if (object_mean_map_ext_flag)
        cout << "File name of extracted object mean feature map:" << object_mean_map_ext_file << endl;
      if (object_std_dev_map_ext_flag)
        cout << "File name of extracted object std. dev. feature map:" << object_std_dev_map_ext_file << endl;
      if (object_bpratio_map_ext_flag)
        cout << "File name of extracted object boundary pixel ratio feature map:" << object_bpratio_map_ext_file << endl;
#ifdef SHAPEFILE
      if (object_shapefile_ext_flag)
        cout << "Base file name of extracted object shapefile:" << object_shapefile_ext_file << endl;
#endif
  }
} // namespace HSEGTilton


