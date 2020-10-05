// params.cc

#include "params.h"
#include <iostream>
#include <ctime>

namespace HSEGTilton
{

 // Constructor
  Params::Params(const string& value)
  {
    version = value;
    current_folder = "";

   // Flags for parameters under user control.
    input_image_flag = false;
    ncols_flag = false;
    nrows_flag = false;
#ifdef THREEDIM
    nslices_flag = false;
#endif // THREEDIM
    nbands_flag = false;
    data_type_flag = false;
    mask_flag = false;
    log_flag = false;
    spclust_wght_flag = false;
    boundary_map_flag = false;
    object_labels_map_flag = false;
    region_objects_flag = false;
    object_conn_type1_flag = false;
    region_sum_flag = false;
    region_std_dev_flag = false;
    region_boundary_npix_flag = false;
    region_threshold_flag = false;
    region_nghbrs_list_flag = true;
    region_nb_objects_flag = false;
    region_objects_list_flag = false;
    gdissim_flag = false;
   // These parameters have calculated defaults that can be overridden by the user

   // Flags for internal program parameters (NOT under user control).
    region_sumsq_flag = false;
    region_sumxlogx_flag = false;

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

#if (defined(WINDOWS) || defined(CYGWIN))
    cout << endl << "Copyright (C) 2006 United States Government as represented by the" << endl;
#else
    cout << endl << "Copyright \u00a9 2006 United States Government as represented by the" << endl;
#endif
    cout << "Administrator of the National Aeronautics and Space Administration." << endl;
    cout << "No copyright is claimed in the United States under Title 17, U.S. Code." << endl;
    cout << "All Other Rights Reserved." << endl << endl;
  }

 // Read initial parameters
  bool Params::read_init(const char *param_file)
  {
//    cout << "Reading initial parameters from " << param_file << endl;
    bool status=false;

    ifstream param_fs(param_file);
    if (!param_fs.is_open())
    {
      cout << "Cannot open input file " << param_file << endl;
      return status;
    }

  // Read initial parameters from parameter file until End-of-File reached
    mask_value = MASK_VALUE;
    int sub_pos;
    string line, sub_string;
    data_type = GDT_Unknown;
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
#ifdef THREEDIM
      if (line.find("-nslices") != string::npos)
      {
        sub_string = process_line(line,false);
        nslices = atoi(sub_string.c_str());
        nslices_flag = true;
      }
#endif // THREEDIM
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
          data_type = GDT_Byte;
          data_type_flag = true;
        }
        else if (sub_string == "UInt16")
        {
          data_type = GDT_UInt16;
          data_type_flag = true;
        }
        else if (sub_string == "Float32")

        {
          data_type = GDT_Float32;
          data_type_flag = true;
        }
        else
        { 
          data_type = GDT_Unknown;
          data_type_flag = false;
        }
      }
      if ((line.find("-mask") != string::npos) && (line.find("-mask_value") == string::npos))
      {
        mask_file = process_line(line,false);
        mask_flag = true;
      }
      if (line.find("-mask_value") != string::npos)
      {
        sub_string = process_line(line,false);
        mask_value = atoi(sub_string.c_str());
      }
      if (line.find("-spclust_wght") != string::npos)
      {
        sub_string = process_line(line,false);
        spclust_wght = (float) atof(sub_string.c_str());
        spclust_wght_flag = ((spclust_wght >= 0.0) && (spclust_wght <= 1.0));
      }
      if (line.find("-log") != string::npos)
      {
        log_file = process_line(line,false);
        log_flag = true;
      }
    }

    param_fs.close();

 // Set default parameter values, based on initial parameter values
    status = set_defaults();

    return status;
  }

  // Set defaults
  bool Params::set_defaults()
  {
    int band;

    scale.resize(nbands);
    for (band = 0; band < nbands; band++)
      scale[band] = RHSEG_SCALE;
    offset.resize(nbands);
    for (band = 0; band < nbands; band++)
      offset[band] = RHSEG_OFFSET;

    if (spclust_wght_flag)
    {
      object_labels_map_flag = REGION_OBJECTS_FLAG;
      region_objects_flag = REGION_OBJECTS_FLAG;
    }
    else
    {
      object_labels_map_flag = false;
      region_objects_flag = false;
    }

    region_sum_flag = false;
    if (nbands < REGION_SUM_FLAG_MAX)
      region_sum_flag = true;
    region_std_dev_flag = REGION_STD_DEV_FLAG;
    region_boundary_npix_flag = REGION_BOUNDARY_NPIX_FLAG;
    region_threshold_flag = REGION_THRESHOLD_FLAG;
    region_nghbrs_list_flag = REGION_NGHBRS_LIST_FLAG;
    
    gdissim_flag = GDISSIM_FLAG;

  // Program defaults usually utilized
    debug = DEBUG_DEFAULT_VALUE;

    bool status = true;

    return status;
  }

 // Read remaining parameters
  bool Params::read(const char *param_file)
  {
//    cout << "Reading remaining parameters from " << param_file << endl;
    ifstream param_fs(param_file);
    if (!param_fs)
    {
      cout << "Cannot open input file " << param_file << endl;
      return false;
    }

  // Read the rest of the parameters from parameter file until End-of-File reached
    int band;
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
      if (line.find("-scale") != string::npos)
      {
        line = process_line(line,true);
        band = 0;
        while (line.size() > 0)
        {
          if (band > nbands)
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
          scale[band] = atof(sub_string.c_str());
          band++;
        }
      }
      if (line.find("-offset") != string::npos)
      {
        line = process_line(line,true);
        band = 0;
        while (line.size() > 0)
        {
          if (band > nbands)
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
          offset[band] = atof(sub_string.c_str());
          band++;
        }
      }
      if (line.find("-class_labels_map") != string::npos)
      {
        sub_string = process_line(line,false);
        class_labels_map_file = sub_string;
      }
      if (line.find("-boundary_map") != string::npos)
      {
        sub_string = process_line(line,false);
        boundary_map_file = sub_string;
        boundary_map_flag = true;
      }
      if (line.find("-region_classes") != string::npos)
      {
        region_classes_file = process_line(line,false);
      }
      if (line.find("-object_labels_map") != string::npos)
      {
        sub_string = process_line(line,false);
        object_labels_map_file = sub_string;
        object_labels_map_flag = true;
      }
      if (line.find("-region_objects") != string::npos)
      {
        if (line.find("-region_objects_list") != string::npos)
        {
          sub_string = process_line(line,false);
          sub_pos = atoi(sub_string.c_str());
          if (sub_pos == 1)
            region_objects_list_flag = true;
          else
            region_objects_list_flag = false;
        }
        else
        {
          region_objects_file = process_line(line,false);
          region_objects_flag = true;
        }
      }
      if (line.find("-object_conn_type1") != string::npos)
      {
        sub_string = process_line(line,false);
        sub_pos = atoi(sub_string.c_str());
        if (sub_pos == 1)
          object_conn_type1_flag = true;
        else
          object_conn_type1_flag = false;
      }
      if (line.find("-oparam") != string::npos)
      {
        oparam_file = process_line(line,false);
      }
      if (line.find("-region_sum") != string::npos)
      {
        sub_string = process_line(line,false);
        sub_pos = atoi(sub_string.c_str());
        if (sub_pos == 1)
          region_sum_flag = true;
        else
          region_sum_flag = false;
      }
      if (line.find("-region_std_dev") != string::npos)
      {
        sub_string = process_line(line,false);
        sub_pos = atoi(sub_string.c_str());
        if (sub_pos == 1)
          region_std_dev_flag = true;
        else
          region_std_dev_flag = false;
      }
      if (line.find("-region_boundary_npix") != string::npos)
      {
        sub_string = process_line(line,false);
        sub_pos = atoi(sub_string.c_str());
        if (sub_pos == 1)
          region_boundary_npix_flag = true;
        else
          region_boundary_npix_flag = false;
      }
      if (line.find("-region_threshold") != string::npos)
      {
        sub_string = process_line(line,false);
        sub_pos = atoi(sub_string.c_str());
        if (sub_pos == 1)
          region_threshold_flag = true;
        else
          region_threshold_flag = false;
      }
      if (line.find("-region_nb_objects") != string::npos)
      {
        sub_string = process_line(line,false);
        sub_pos = atoi(sub_string.c_str());
        if (sub_pos == 1)
          region_nb_objects_flag = true;
        else
          region_nb_objects_flag = false;
      }
      if (line.find("-conn_type") != string::npos)
      {
        sub_string = process_line(line,false);
        conn_type = atoi(sub_string.c_str());
      }
      if (line.find("-gdissim") != string::npos)
      {
        sub_string = process_line(line,false);
        sub_pos = atoi(sub_string.c_str());
        if (sub_pos == 1)
          gdissim_flag = true;
        else
          gdissim_flag = false;
      }
      if (line.find("-debug") != string::npos)
      {
        sub_string = process_line(line,false);
        debug = atoi(sub_string.c_str());
      }
    }

    param_fs.close();

    bool status = true;

    return status;
  }

 // oParams Constructor
  oParams::oParams()
  {
    return;
  }

 // oParams Destructor...
  oParams::~oParams() 
  {
    return;
  }

 // Print parameters
  void Params::print()
  {
    int band;

    if (debug > 0)
    {
  // Print version
      log_fs << "This is " << version << endl << endl;

  // Print input parameters
      log_fs << endl << "Required input parameters that specify and describe the input data:" << endl;
      log_fs << "Input image file name: " << input_image_file << endl;
      log_fs << "Input image number of columns = " <<  ncols << endl;
      log_fs << "Input image number of rows = " <<  nrows << endl;
#ifdef THREEDIM
      log_fs << "Input image number of slices = " <<  nslices << endl;
#endif
      log_fs << "Input image number of bands = " <<  nbands << endl;
      switch (data_type)
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
        default:          log_fs << "Input image data type in invalid (Unknown)" << endl;
                          break;
      }

      log_fs << endl << "Optional input parameters that specify optional input files and parameter values:" << endl;
      if (nbands < 10)
      {
        log_fs << "Input data scale factors: ";
        for (band = 0; band < nbands; band++)
          log_fs << scale[band] << " ";
        log_fs << endl;
        log_fs << "Input data offset factors: ";
        for (band = 0; band < nbands; band++)
          log_fs << offset[band] << " ";
        log_fs << endl;
      }
      if (mask_flag)
      {
        log_fs << "Input bad data mask file name: " << mask_file << endl;
        log_fs << "Bad data mask flag value = " << mask_value << endl;
      }

      log_fs << endl << "Input parameters that specify output files:" << endl;
      log_fs << "Output parameter file name: " << oparam_file << endl;
      log_fs << "Output class labels map file name: " << class_labels_map_file << endl;
      if (boundary_map_flag)
        log_fs << "Optional output hierarchical boundary map file name: " << boundary_map_file << endl;
      log_fs << "Output region classes file name: " << region_classes_file << endl;
      log_fs << endl << "Optional parameters that select the contents of the output" << endl;
      log_fs << "region object files (above):" << endl;
      if (region_sum_flag)
        log_fs << "Region sum (and, if available, sumsq and sumxlogx) information included." << endl;
      if (region_std_dev_flag)
        log_fs << "Region standard deviation information included" << endl;
      if (region_boundary_npix_flag)
        log_fs << "Region region boundary number of pixels information included" << endl;
      if (region_threshold_flag)
        log_fs << "Region most recent merge threshold information included" << endl;
      if (region_nghbrs_list_flag)
        log_fs << "List of region clases neighboring each region class included" << endl;
      if (region_nb_objects_flag)
        log_fs << "Number of region objects contained in each region class included" << endl;
      if (region_objects_list_flag)
        log_fs << "List of region objects contained in each region class included" << endl;

      log_fs << endl << "Defaultable required input parameters, along with other associated parameters:" << endl;
      log_fs <<  "Utilize Spectral Clustering?  ";
      if (spclust_wght_flag)
      {
       log_fs <<  "Yes, with " << spclust_wght << " weighting factor (relative to region growing)." << endl;
      }
      else
        log_fs <<  "No." << endl;
      log_fs << "Number of Nearest Neighbors = " <<  maxnbdir << endl;

      log_fs << endl << "Parameters with recommended default values:" << endl;
      if (gdissim_flag)
        log_fs << "Global dissimilarity values calculated for each hierarchical level output" << endl;
      if (region_objects_list_flag)
      {
        log_fs << endl << "Optional parameters:" << endl;
        if (object_labels_map_flag)
          log_fs << "Output object labels map file name: " << object_labels_map_file << endl;
        if (region_objects_flag)
          log_fs << "Output region objects file name: " << region_objects_file << endl;
        if (object_conn_type1_flag)
          log_fs << "Minimum Nearest Neighbors used for Region Objects " << endl;
      }
      log_fs << endl << "Debug option = " <<  debug << endl;
      if (log_flag)
        log_fs << "Output log file name: " << log_file << endl;
      if (region_sum_flag)
      {
        if (region_sumsq_flag)
          log_fs << "Region sumsq information included." << endl;
        if (region_sumxlogx_flag)
          log_fs << "Region sumxlogx information included." << endl;
      }
    }
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

} // namespace HSEGTilton
