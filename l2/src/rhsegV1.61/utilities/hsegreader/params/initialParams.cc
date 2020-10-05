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
    region_class_nghbrs_list_flag = false;
    region_object_nghbrs_list_flag = false;
    debug = 1;
    log_file = "hsegreader.log";
 
  }

 // Destructor...
  InitialParams::~InitialParams() 
  {
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
    bool log_flag = false;
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
      if (line.find("-region_class_nghbrs_list") != string::npos)
      {
        sub_string = process_line(line,false);
        sub_pos = atoi(sub_string.c_str());
        if (sub_pos == 1)
          region_class_nghbrs_list_flag = true;
        else
          region_class_nghbrs_list_flag = false;
      }
      if (line.find("-region_object_nghbrs_list") != string::npos)
      {
        sub_string = process_line(line,false);
        sub_pos = atoi(sub_string.c_str());
        if (sub_pos == 1)
          region_object_nghbrs_list_flag = true;
        else
          region_object_nghbrs_list_flag = false;
      }
      if (line.find("-debug") != string::npos)
      {
        sub_string = process_line(line,false);
        debug = atoi(sub_string.c_str());
      }
      if (line.find("-log") != string::npos)
      {
        log_file = process_line(line,false);
        log_flag = true;
      }
    }
    param_fs.close();

    if (!oparam_flag)
    {
      cout << "ERROR: -oparam (RHSeg output parameter file) is required" << endl;
      return false;
    }
    if ((debug > 0) && (log_flag == false))
    {
      if (!params.gtkmm_flag)
        cout << "NOTE: Output log file defaulted to " << log_file << endl;
      log_flag = true;
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
    short unsigned int band;

    if (debug > 0)
    {
    // Print version
      params.log_fs << "This is " << params.version << endl << endl;
    // Print input parameters
      params.log_fs << "RHSeg output parameter file:" << oparam_file << endl;
      if (region_class_nghbrs_list_flag)
        params.log_fs << "List of the region classes neighboring each region object included" << endl;
      if (region_object_nghbrs_list_flag)
        params.log_fs << "List of the region objects neighboring each region object included" << endl;
      params.log_fs << endl << "Debug option = " <<  debug << endl;
      if (debug > 0)
        params.log_fs << "Output log file name: " << log_file << endl;

    // Print RHSeg output parameters
      params.log_fs << endl << "Parameters input from the RHSeg output parameter file:" << endl;
      params.log_fs << endl << "Parameters that specify and describe the input data:" << endl;
      params.log_fs << "Input image file name: " << params.input_image_file << endl;
      params.log_fs << "Input image number of columns = " <<  params.ncols << endl;
      params.log_fs << "Input image number of rows = " <<  params.nrows << endl;
#ifdef THREEDIM
      params.log_fs << "Input image number of slices = " <<  params.nslices << endl;
#endif
      params.log_fs << "Input image number of bands = " <<  params.nbands << endl;
      switch (params.dtype)
      {
        case UInt8:   params.log_fs << "Input image data type is UNSIGNED 8-bit (UInt8)" << endl;
                      break;
        case UInt16:  params.log_fs << "Input image data type is UNSIGNED 16-bit (UInt16)" << endl;
                      break;
        case Float32: params.log_fs << "Input image data type is 32-bit FLOAT (Float32)" << endl;
                      break;
        default:      params.log_fs << "Input image data type in invalid (Unknown)" << endl;
                      break;
      }

      params.log_fs << endl << "Parameters that specify and describe the RHSeg output:" << endl;
      params.log_fs << endl << "Class labels map file name: " << params.class_labels_map_file << endl;
      if (params.boundary_map_flag)
        params.log_fs << "Optional Hierarchical boundary map file name: " << params.boundary_map_file << endl;
      params.log_fs << "Region classes file name: " << params.region_classes_file << endl;
      if (params.object_labels_map_flag)
        params.log_fs << "Object labels map output file name: " << params.object_labels_map_file << endl;
      if (params.region_objects_flag)
        params.log_fs << "Region objects output file name: " << params.region_objects_file << endl;

      params.log_fs << endl << "Optional parameters that select the contents of the output" << endl;
      params.log_fs << "region object files (above):" << endl;
      if (params.region_sum_flag)
      {
        params.log_fs << "Region sum information included." << endl;
        if (params.region_sumsq_flag)
          params.log_fs << "Region sumsq information included." << endl;
        if (params.region_sumxlogx_flag)
          params.log_fs << "Region sumxlogx information included." << endl;
      }
      if (params.region_std_dev_flag)
        params.log_fs << "Region standard deviation information included" << endl;
      if (params.region_boundary_npix_flag)
        params.log_fs << "Region region boundary number of pixels information included" << endl;
      if (params.region_threshold_flag)
        params.log_fs << "Region most recent merge threshold information included" << endl;
      if (params.region_nghbrs_list_flag)
        params.log_fs << "List of the region classes neighboring each region class included" << endl;
      if (params.region_nb_objects_flag)
        params.log_fs << "Number of region objects in each region class included" << endl;
      if (params.region_objects_list_flag)
        params.log_fs << "List of region objects contained in each region class included" << endl;
 
      params.log_fs << endl << "Optional input parameters that specify optional input files and parameter values:" << endl;
      params.log_fs << "Input scale factors: ";
      for (band = 0; band < params.nbands; band++)
        params.log_fs << params.scale[band] << " ";
      params.log_fs << endl;
      params.log_fs << "Input offset factors: ";
      for (band = 0; band < params.nbands; band++)
        params.log_fs << params.offset[band] << " ";
      params.log_fs << endl;
      params.log_fs << "Normalization scale factors: ";
      for (band = 0; band < params.nbands; band++)
        params.log_fs << oparams.scale[band] << " ";
      params.log_fs << endl;
      params.log_fs << "Normalization offset factors: ";
      for (band = 0; band < params.nbands; band++)
        params.log_fs << oparams.offset[band] << " ";
      params.log_fs << endl;

      params.log_fs << endl << "Defaultable required input parameters, along with other associated parameters:" << endl;
      params.log_fs <<  "Utilize Spectral Clustering?  ";
      if (params.spclust_wght_flag)
      {
        params.log_fs <<  "Yes, with " << params.spclust_wght << " weighting factor (relative to region growing)." << endl;
      }
      else
        params.log_fs <<  "No." << endl;
      params.log_fs <<  "Dissimilarity Criterion:  ";
      switch(params.dissim_crit)
      {  
        case 1:  params.log_fs <<  "1-Norm" << endl;
     	         break;
        case 2:  params.log_fs <<  "2-Norm" << endl;
  	         break;
        case 3:  params.log_fs <<  "Infinity Norm" << endl;
  	         break;
        case 4:  params.log_fs <<  "Spectral Angle Mapper" << endl;
  	         break;
        case 5:  params.log_fs <<  "Spectral Information Divergence" << endl;
  	         break;
        case 6:  params.log_fs <<  "Square Root of Band Sum Mean Squared Error" << endl;
  	         break;
        case 7:  params.log_fs <<  "Square Root of Band Maximum Mean Squared Error" << endl;
  	         break;
        case 8:  params.log_fs <<  "Normalized Vector Distance" << endl;
  	         break;
        case 9:  params.log_fs <<  "Entropy" << endl;
  	         break;
        case 10: params.log_fs <<  "SAR Speckle Noise Criterion" << endl;
  	         break;
        default: params.log_fs <<  "INVALID" << endl;
  	         break;
      }
      params.log_fs << "Number of Nearest Neighbors = " <<  params.maxnbdir << endl;
      if (params.std_dev_image_flag)
        params.log_fs << "Weight for standard deviation spatial feature = " << params.std_dev_wght << endl;
      params.log_fs << "Normalize Input Data?  ";
      switch(params.normind)
      {  
        case 1: params.log_fs <<  "No." << endl << "Do not normalize input data." << endl;
                break;
        case 2: params.log_fs <<  "Yes." << endl << "Normalize input data across all bands." << endl;
  	        break;
        case 3: params.log_fs <<  "Yes." << endl << "Normalize each band of input data separately." << endl;
  	        break;
      }
      params.log_fs << "Number of hierarchical segmentation levels = " << oparams.nb_levels << endl;
      params.log_fs << "Number of region classes at hierarchical segmentation level 0: " << oparams.level0_nb_classes << endl;
      if (params.region_nb_objects_flag)
        params.log_fs << "Number of region objects at hierarchical segmentation level 0: " << oparams.level0_nb_objects << endl;
      for (band = 0; band < oparams.nb_levels; band++)
        params.log_fs << oparams.int_buffer_size[band] << " ";
      params.log_fs << endl;
      if (params.gdissim_flag)
      {
        params.log_fs << "Global dissimilarity at each hiearchical level: ";
        for (band = 0; band < oparams.nb_levels; band++)
          params.log_fs << oparams.gdissim[band] << " ";
        params.log_fs << endl;
     }
      params.log_fs << "Maximum merge threshold at each hiearchical level: ";
      for (band = 0; band < oparams.nb_levels; band++)
        params.log_fs << oparams.max_threshold[band] << " ";
      params.log_fs << endl;
    }
    else
    {
    // Print version
      cout << "This is " << params.version << endl << endl;
    // Print input parameters
      cout << "RHSeg output parameter file:" << oparam_file << endl;
      if (region_class_nghbrs_list_flag)
        cout << "List of the region classes neighboring each region object included" << endl;
      if (region_object_nghbrs_list_flag)
        cout << "List of the region objects neighboring each region object included" << endl;
      cout << endl << "Debug option = " <<  debug << endl;
      if (debug > 0)
        cout << "Output log file name: " << log_file << endl;

    // Print RHSeg output parameters
      cout << endl << "Parameters input from the RHSeg output parameter file:" << endl;
      cout << endl << "Parameters that specify and describe the input data:" << endl;
      cout << "Input image file name: " << params.input_image_file << endl;
      cout << "Input image number of columns = " <<  params.ncols << endl;
      cout << "Input image number of rows = " <<  params.nrows << endl;
#ifdef THREEDIM
      cout << "Input image number of slices = " <<  params.nslices << endl;
#endif
      cout << "Input image number of bands = " <<  params.nbands << endl;
      switch (params.dtype)
      {
        case UInt8:   cout << "Input image data type is UNSIGNED 8-bit (UInt8)" << endl;
                      break;
        case UInt16:  cout << "Input image data type is UNSIGNED 16-bit (UInt16)" << endl;
                      break;
        case Float32: cout << "Input image data type is 32-bit FLOAT (Float32)" << endl;
                      break;
        default:      cout << "Input image data type in invalid (Unknown)" << endl;
                      break;
      }

      cout << endl << "Parameters that specify and describe the RHSeg output:" << endl;
      cout << endl << "Class labels map file name: " << params.class_labels_map_file << endl;
      if (params.boundary_map_flag)
        cout << "Optional Hierarchical boundary map file name: " << params.boundary_map_file << endl;
      cout << "Region classes file name: " << params.region_classes_file << endl;
      if (params.object_labels_map_flag)
        cout << "Object labels map output file name: " << params.object_labels_map_file << endl;
      if (params.region_objects_flag)
        cout << "Region objects output file name: " << params.region_objects_file << endl;

      cout << endl << "Optional parameters that select the contents of the output" << endl;
      cout << "region object files (above):" << endl;
      if (params.region_sum_flag)
      {
        cout << "Region sum information included." << endl;
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
        cout << "List of the region classes neighboring each region class included" << endl;
      if (params.region_nb_objects_flag)
        cout << "Number of region objects in each region class included" << endl;
      if (params.region_objects_list_flag)
        cout << "List of region objects contained in each region class included" << endl;
 
      cout << endl << "Optional input parameters that specify optional input files and parameter values:" << endl;
      cout << "Input scale factors: ";
      for (band = 0; band < params.nbands; band++)
        cout << params.scale[band] << " ";
      cout << endl;
      cout << "Input offset factors: ";
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

      cout << endl << "Defaultable required input parameters, along with other associated parameters:" << endl;
      cout <<  "Utilize Spectral Clustering?  ";
      if (params.spclust_wght_flag)
      {
        cout <<  "Yes, with " << params.spclust_wght << " weighting factor (relative to region growing)." << endl;
      }
      else
        cout <<  "No." << endl;
      cout <<  "Dissimilarity Criterion:  ";
      switch(params.dissim_crit)
      {  
        case 1:  cout <<  "1-Norm" << endl;
     	         break;
        case 2:  cout <<  "2-Norm" << endl;
  	         break;
        case 3:  cout <<  "Infinity Norm" << endl;
  	         break;
        case 4:  cout <<  "Spectral Angle Mapper" << endl;
  	         break;
        case 5:  cout <<  "Spectral Information Divergence" << endl;
  	         break;
        case 6:  cout <<  "Square Root of Band Sum Mean Squared Error" << endl;
  	         break;
        case 7:  cout <<  "Square Root of Band Maximum Mean Squared Error" << endl;
  	         break;
        case 8:  cout <<  "Normalized Vector Distance" << endl;
  	         break;
        case 9:  cout <<  "Entropy" << endl;
  	         break;
        case 10: cout <<  "SAR Speckle Noise Criterion" << endl;
  	         break;
        default: cout <<  "INVALID" << endl;
  	         break;
      }
      cout << "Number of Nearest Neighbors = " <<  params.maxnbdir << endl;
      if (params.std_dev_image_flag)
        cout << "Weight for standard deviation spatial feature = " << params.std_dev_wght << endl;
      cout << "Normalize Input Data?  ";
      switch(params.normind)
      {  
        case 1: cout <<  "No." << endl << "Do not normalize input data." << endl;
                break;
        case 2: cout <<  "Yes." << endl << "Normalize input data across all bands." << endl;
  	        break;
        case 3: cout <<  "Yes." << endl << "Normalize each band of input data separately." << endl;
  	        break;
      }
      cout << "Number of hierarchical segmentation levels = " << oparams.nb_levels << endl;
      cout << "Number of region classes at hierarchical segmentation level 0: " << oparams.level0_nb_classes << endl;
      if (params.region_nb_objects_flag)
        cout << "Number of region objects at hierarchical segmentation level 0: " << oparams.level0_nb_objects << endl;
      for (band = 0; band < oparams.nb_levels; band++)
        cout << oparams.int_buffer_size[band] << " ";
      cout << endl;
      if (params.gdissim_flag)
      {
        cout << "Global dissimilarity at each hiearchical level: ";
        for (band = 0; band < oparams.nb_levels; band++)
          cout << oparams.gdissim[band] << " ";
        cout << endl;
     }
      cout << "Maximum merge threshold at each hiearchical level: ";
      for (band = 0; band < oparams.nb_levels; band++)
        cout << oparams.max_threshold[band] << " ";
      cout << endl;
    }
  }
} // namespace HSEGTilton


