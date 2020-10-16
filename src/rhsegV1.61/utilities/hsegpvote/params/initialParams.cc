// initialParams.cc

#include "initialParams.h"
#include <params/params.h>
#include <image/image.h>
#include <iostream>
#include <fstream>

using namespace CommonTilton;

// Globals
Image pixelClassImage;
Image trainingLabelImage;

// Externals
extern HSEGTilton::Params params;
extern HSEGTilton::oParams oparams;

namespace HSEGTilton
{

 // Constructor
  InitialParams::InitialParams()
  {

   // Flags for parameters under user control.
    region_class_flag = false;
    color_table_flag = false;

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
    bool pixel_class_flag = false;
    bool oparam_flag = false;
    bool training_label_flag = false;
    bool opt_class_flag = false;
    bool log_flag = false;
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
      if (line.find("-pixel_classification") != string::npos)
      {
        pixel_class_file = process_line(line,false);
        pixel_class_flag = pixelClassImage.open(pixel_class_file);
        if (!pixel_class_flag)
        {
          cout << "ERROR:  Input pixel-based classification " << pixel_class_file << " is of unknown image format." << endl;
          return false;
        }
      }
      if (line.find("-oparam") != string::npos)
      {
        oparam_file = process_line(line,false);
        oparam_flag = true;
      }
      if (line.find("-training_labeling") != string::npos)
      {
        training_label_file = process_line(line,false);
        training_label_flag = trainingLabelImage.open(training_label_file);
        if (!training_label_flag)
        {
          cout << "WARNING:  Input training label data file " << training_label_file << " is of unknown format." << endl;
          return false;
        }
      }
      if (line.find("-region_class_flag") != string::npos)
      {
        sub_string = process_line(line,false);
        sub_pos = atoi(sub_string.c_str());
        if (sub_pos == 1)
          region_class_flag = true;
        else
          region_class_flag = false;
      }
      if (line.find("-color_table") != string::npos)
      {
        color_table_file = process_line(line,false);
        color_table_flag = true;
      }
      if (line.find("-opt_classification") != string::npos)
      {
        opt_class_file = process_line(line,false);
        opt_class_flag = true;
      }
      if (line.find("-log_file") != string::npos)
      {
        log_file = process_line(line,false);
        log_flag = true;
      }
    }
    param_fs.close();

// Exit with false status if a required parameter is not set.
    if (!pixel_class_flag)
    {
      cout << "ERROR: -pixel_classification (Input pixel-based classification) is required" << endl;
      return false;
    }
    if (!oparam_flag)
    {
      cout << "ERROR: -oparam (RHSeg output parameter file) is required" << endl;
      return false;
    }
    if (!training_label_flag)
    {
      cout << "ERROR: -training_labeling (Input training label data) is required" << endl;
      return false;
    }
    if (!log_flag)
    {
      cout << "ERROR: -log_file (Output log file) is required" << endl;
      return false;
    }
    if (!opt_class_flag)
    {
      cout << "ERROR: -opt_classification_file (Output class optimized classification file) is required" << endl;
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

   // Override user specification of region_class_flag for maxnbdir > 4nn (NB_LIMIT).
    if (params.maxnbdir > NB_LIMIT)
      region_class_flag = false;

    set_temp_files();

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
      }
    }

    param_fs.close();

    return true;
  }

 // Set temporary file names
  void InitialParams::set_temp_files()
  {
   // Create prefix for temporary files
    static char time_buffer[26];
    time_t now;
    const  struct tm *tm_ptr;
    now = time(NULL);
    tm_ptr = localtime(&now);
/*
    unsigned int length;
    length = strftime(time_buffer,26,"%a%b%d%H%M%S%Y",tm_ptr);
*/
    strftime(time_buffer,26,"%a%b%d%H%M%S%Y",tm_ptr);

    string temp_file_name = time_buffer;
    prefix = "PVRHSEG" + temp_file_name;

  // Set temporary file names
    temp_seg_level_label_file = prefix + "seg_level_label.tif";
    temp_object_label_file = prefix + "object_label.tif";
    temp_region_class_file = prefix + "region_class.tif";
    temp_mask_file = prefix + "mask.tif";

    return;
  }

  // Print parameters
  void InitialParams::print()
  {
   // Print input parameters
    cout << "Input pixel-based classification image file name: " << pixel_class_file << endl;
    cout << "RHSeg output parameter file: " << oparam_file << endl;
    cout << "Training label image file name: " << training_label_file << endl;
    if (color_table_flag)
      cout << "Color table file name: " << color_table_file << endl;
    cout << "Output class optimized classification file name: " << opt_class_file << endl;
    cout << "Output log file name: " << log_file << endl;

    return;
  }

 // Print param and oparam values
  void InitialParams::print_oparam()
  {
      int band, index;

  // Print input parameters
      cout << endl << "Required input parameters that specify and describe the input data:" << endl;
      if (params.gdal_input_flag)
        cout << "gdal_input flag is true" << endl;
      else
        cout << "gdal_input flag is flase" << endl;
      cout << "Input image file name: " << params.input_image_file << endl;
      cout << "Input image number of columns = " <<  params.ncols << endl;
      cout << "Input image number of rows = " <<  params.nrows << endl;
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
#ifdef MSE_SQRT
       case 6:  cout <<  "Square Root of Band Sum Mean Squared Error" << endl;
  	        break;
       case 7:  cout <<  "Square Root of Band Maximum Mean Squared Error" << endl;
  	        break;
#else
       case 6:  cout <<  "Band Sum Mean Squared Error" << endl;
  	        break;
       case 7:  cout <<  "Band Maximum Mean Squared Error" << endl;
  	        break;
#endif
       case 8:  cout <<  "Normalized Vector Distance" << endl;
  	        break;
       case 9:  cout <<  "Entropy" << endl;
  	        break;
       case 10: cout <<  "SAR Speckle Noise Criterion" << endl;
  	        break;
//       case 11: cout <<  "Feature Range" << endl;
//  	        break;
       default: cout <<  "WARNING: Invalid value for dissim_crit parameter" << endl;
  	        break;
      }
      cout << "Number of Nearest Neighbors = " <<  params.maxnbdir << endl;

      cout << endl << "Parameters with recommended default values:" << endl;
      if (params.std_dev_image_flag)
        cout << "Weight for standard deviation spatial feature = " << params.std_dev_wght << endl;
      cout << "Normalize Input Data?  ";
      switch(params.normind)
      {
        case 1:    cout <<  "No." << endl << "Do not normalize input data." << endl;
                   break;
        case 2:    cout <<  "Yes." << endl << "Normalize input data across all bands." << endl;
  	           break;
        case 3:    cout <<  "Yes." << endl << "Normalize each band of input data separately." << endl;
  	           break;
        default:   cout <<  "WARNING: INVALID value for normind parameter." << endl;
                   break;
      }
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

 // Remove temporary files
  void InitialParams::remove_temp_files()
  {
    remove(temp_seg_level_label_file.c_str());
    remove(temp_region_class_file.c_str());
    if ((params.maxnbdir > NB_LIMIT) || 
        ((!params.object_labels_map_flag) && (!region_class_flag)))
      remove(temp_object_label_file.c_str());
    remove(temp_mask_file.c_str());

    return;
  }

} // namespace HSEGTilton
