// initialParams.cc

#include "initialParams.h"
#include <params/params.h>
#include <iostream>

extern HSEGTilton::Params params;
extern HSEGTilton::oParams oparams;
#ifdef GLSIMP
extern CommonTilton::Image inputImage;
#endif

namespace HSEGTilton
{

 // Constructor
  InitialParams::InitialParams()
  {
    rgb_image_stretch = 2;
    range[0] = 0.1;
    range[1] = 0.9;
    examples_out_file = "examples_out.txt";
    label_out_file = "label_out";
    ascii_out_file = "ascii_out.txt";

   // Flags for parameters under user control.
    oparam_flag = false;
    red_display_flag = false;
    green_display_flag = false;
    blue_display_flag = false;
    view_dimension_flag = false;
    view_element_flag = false;
    examples_in_flag = false;
    panchromatic_image_flag = false;
    reference_image_flag = false;

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
#ifdef GLSIMP
    int    band;
    double min_value;
#endif
    unsigned int index, sub_pos;
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
#ifdef GLSIMP
      if (line.find("-input_image") != string::npos)
      {
        params.input_image_file = process_line(line,false);
        params.input_image_flag = inputImage.open(params.input_image_file);
        if (params.input_image_flag)
        {
          params.ncols = inputImage.get_ncols();
          params.ncols_flag = true;
          params.nrows = inputImage.get_nrows();
          params.nrows_flag = true;
          params.nbands = inputImage.get_nbands();
          params.nbands_flag = true;
          params.dtype = inputImage.get_dtype();
          params.dtype_flag = true;
          params.data_type = inputImage.get_data_type();
          switch (params.data_type)
          {
              case GDT_Byte:    break;
              case GDT_UInt16:  break;
              case GDT_Int16:   min_value = 0;
                                for (band = 0; band < params.nbands; band++)
                                  if (min_value > inputImage.getMinimum(band))
                                    min_value = inputImage.getMinimum(band);
                                if (min_value < 0.0)
                                {
                                  cout << "WARNING: For the input image data file " << params.input_image_file << "," << endl;
                                  cout << "short integer data will be converted to 32-bit float data." << endl;
                                  params.dtype = Float32;
                                }
                                else
                                {
                                  cout << "WARNING: For the input image data file " << params.input_image_file << "," << endl;
                                  cout << "short integer data will be converted to unsigned short integer data." << endl;
                                  params.dtype = UInt16;
                                }
                                break;
              case GDT_UInt32:  cout << "NOTE: For the input image data file " << params.input_image_file << "," << endl;
                                cout << "32-bit unsigned integer data will be converted to 32-bit float data." << endl;
                                params.dtype = Float32;
                                break;
              case GDT_Int32:   cout << "NOTE: For the input image data file " << params.input_image_file << "," << endl;
                                cout << "32-bit integer data will be converted to 32-bit float data." << endl;
                                params.dtype = Float32;
                                break;
              case GDT_Float32: break;
              case GDT_Float64: cout << "WARNING: For the input image data file " << params.input_image_file << "," << endl;
                                cout << "64-bit double data will be converted to 32-bit float data." << endl;
                                cout << "Out of ranges value will not be read properly." << endl;
                                params.dtype = Float32;
                                break;
              default:          cout << "Unknown or unsupported image data type for input image data file ";
                                cout << params.input_image_file << endl;
                                return false;
          } 
        }
        else
        {
          cout << "WARNING:  Input image file " << params.input_image_file << " is of unknown format." << endl;
        }
      }
#endif
      if (line.find("-red_display_band") != string::npos)
      {
        sub_string = process_line(line,false);
        red_display_band = atoi(sub_string.c_str());
        red_display_flag = true;
      }
      if (line.find("-green_display_band") != string::npos)
      {
        sub_string = process_line(line,false);
        green_display_band = atoi(sub_string.c_str());
        green_display_flag = true;
      }
      if (line.find("-blue_display_band") != string::npos)
      {
        sub_string = process_line(line,false);
        blue_display_band = atoi(sub_string.c_str());
        blue_display_flag = true;
      }
      if (line.find("-RBG_image_stretch") != string::npos)
      {
        sub_string = process_line(line,false);
        rgb_image_stretch = atoi(sub_string.c_str());
      }
      if (line.find("-range") != string::npos)
      {
        line = process_line(line,true);
        index = 0;
        while (line.size() > 0)
        {
          if (index > 1)
            break;
          sub_pos = line.find_first_of(",");
          if (sub_pos != (unsigned int) string::npos)
          {
            sub_string = line.substr(0,sub_pos);
            line = line.substr(sub_pos+1);
          }
          else
          {
            sub_string = line;
            line = "";
          }
          range[index] = atof(sub_string.c_str());
          index++;
        }
      }
      if (line.find("-examples_out") != string::npos)
      {
        examples_out_file = process_line(line,false);
      }
      if (line.find("-label_out") != string::npos)
      {
        label_out_file = process_line(line,false);
      }
      if (line.find("-ascii_out") != string::npos)
      {
        ascii_out_file = process_line(line,false);
      }
      if (line.find("-view_dimension") != string::npos)
      {
        view_dimension_flag = true;
        sub_string = process_line(line,false);
        if (sub_string == "column")
          view_dimension = COLUMN;
        else if (sub_string == "row")
          view_dimension = ROW;
        else if (sub_string == "slice")
          view_dimension = SLICE;
        else
          view_dimension_flag = false;
      }
      if (line.find("-view_element") != string::npos)
      {
        sub_string = process_line(line,false);
        view_element = atoi(sub_string.c_str());
        view_element_flag = true;
      }
      if (line.find("-examples_in") != string::npos)
      {
        examples_in_file = process_line(line,false);
        examples_in_flag = true;
      }
      if (line.find("-panchromatic_image") != string::npos)
      {
        panchromatic_image_file = process_line(line,false);
        panchromatic_image_flag = true;
      }
      if (line.find("-reference_image") != string::npos)
      {
        reference_image_file = process_line(line,false);
        reference_image_flag = true;
      }
    }
    param_fs.close();

    if (!oparam_flag)
    {
      cout << "ERROR: -oparam (RHSeg output parameter file) is required" << endl;
      return false;
    }
    if ((!red_display_flag) || (!green_display_flag) || (!blue_display_flag))
    {
      if (!params.gtkmm_flag)
      {
        cout << "ERROR: -red_display_band, -green_display_band, and -blue_display_band";
        cout << " must each be specified" << endl;
      }
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

    if (!view_dimension_flag)
    {
      view_dimension = SLICE;
      view_dimension_flag = true;
    }
    if (!view_element_flag)
    {
      view_element = 0;
      view_element_flag = true;
    }

   // Set internal program paramaters and return
    return set_internal_parameters();
 }

 // Read parameters
  bool InitialParams::read_oparam()
  {
   // Read parameters from RHSeg output parameter file until End-of-File reached
    ifstream oparam_fs(oparam_file.c_str());
    if (!oparam_fs)
    {
      cout << "Cannot open RHSeg output parameter file " << oparam_file << endl;
      return false;
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

    ifstream param_fs(oparam_file.c_str());
    if (!param_fs.is_open())
    {
      cout << "Cannot (re)open RHSeg output parameter file " << oparam_file << endl;
      return false;
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

 // Determine number of dimensions
    params.nb_dimensions = 0;
#ifdef THREEDIM
    if ((params.ncols > 1) && (params.nrows == 1) && (params.nslices == 1))
      params.nb_dimensions = 1;
    if ((params.ncols > 1) && (params.nrows > 1) && (params.nslices == 1))
      params.nb_dimensions = 2;
    if ((params.ncols > 1) && (params.nrows > 1) && (params.nslices > 1))
      params.nb_dimensions = 3;
#else
    if ((params.ncols > 1) && (params.nrows == 1))
      params.nb_dimensions = 1;
    if ((params.ncols > 1) && (params.nrows > 1))
      params.nb_dimensions = 2;
#endif
    if (params.nb_dimensions == 0)
    {
      cout << "Unexpected dimensions: For 1-D, must have nrows = nslices = 1; for 2-D must have nslices = 1" << endl;
      return false;
    }

    return true;
 }

 // Set internal parameters
  bool InitialParams::set_internal_parameters()
  {
    ncols_subset = params.ncols;
    nrows_subset = params.nrows;
#ifdef THREEDIM
    nslices_subset = params.nslices;
#endif
    ncols_offset = 0;
    nrows_offset = 0;
#ifdef THREEDIM
    nslices_offset = 0;
    switch (view_dimension)
    {
      case COLUMN: ncols_subset = 1;
                   ncols_offset = view_element;
                   break;
      case    ROW: nrows_subset = 1;
                   nrows_offset = view_element;
                   break;
      case  SLICE: nslices_subset = 1;
                   nslices_offset = view_element;
                   break;
    }
#endif

    const char *temp_dir;
    temp_dir = getenv("TMP");
    if (temp_dir == NULL)
      temp_dir = getenv("TEMP");
    if (temp_dir == NULL)
      temp_dir = getenv("TMPDIR");
    if (temp_dir == NULL)
    {
      temp_dir = (char *) malloc(5*sizeof(char));
      string tmp = "/tmp";
      temp_dir = tmp.c_str();
    }
    string temp_directory = temp_dir;

    static char time_buffer[TIME_SIZE];
    time_t now;
    const  struct tm *tm_ptr;
    now = time(NULL);
    tm_ptr = localtime(&now);
/*
    int length;
    length = strftime(time_buffer,TIME_SIZE,"%a%b%d%H%M%S%Y",tm_ptr);
*/
    strftime(time_buffer,TIME_SIZE,"%a%b%d%H%M%S%Y",tm_ptr);

    string temp_file_name = time_buffer;
#ifdef WINDOWS
    string slash = "\\";
#else
    string slash = "/";
#endif
    label_data_file = temp_directory + slash + "HSEG_LEARN_LABEL_DATA_" + temp_file_name + ".tif";
    fstream io_file_fs;
    io_file_fs.open(label_data_file.c_str( ), ios_base::out );
    if (!io_file_fs)
    {
      cout << "ERROR(initialParams.read):  Cannot open a file in the temporary directory " << temp_directory << endl;
      return false;
    }
    io_file_fs.close( );

    saved_label_data_file = temp_directory + slash + "HSEG_LEARN_SAVED_LABEL_DATA_" + temp_file_name + ".tif";
#ifdef THREEDIM
    view_input_image_file = temp_directory + slash + "HSEG_LEARN_VIEW_INPUT_IMAGE_" + temp_file_name + ".tif";
    view_mask_image_file = temp_directory + slash + "HSEG_LEARN_VIEW_MASK_IMAGE_" + temp_file_name + ".tif";
#endif
    seg_level_classes_map_file = temp_directory + slash + "HSEG_LEARN_SEG_LEVEL_CLASSES_MAP_" + temp_file_name + ".tif";
    region_map_file = temp_directory + slash + "HSEG_LEARN_REGION_MAP_" + temp_file_name + ".tif";
    saved_region_map_file = temp_directory + slash + "HSEG_LEARN_SAVED_REGION_MAP_" + temp_file_name + ".tif";
    pan_subset_image_file = temp_directory + slash + "HSEG_LEARN_PAN_SUBSET_" + temp_file_name + ".tif";

    return true;
  } 

 // Print parameters
 void InitialParams::print()
 {
   int band;

  // Print version
   cout << "This is " << params.version << endl << endl;
  // Print input parameters
   cout << "RHSeg output parameter file: " << oparam_file << endl;
   cout << "Spectral band to be displayed as red = " << red_display_band << endl;
   cout << "Spectral band to be displayed as green = " << green_display_band << endl;
   cout << "Spectral band to be displayed as blue = " << blue_display_band << endl;
   cout << "RBG Image stretch option: ";
   switch (rgb_image_stretch)
   {
     case 1:  cout << "Linear Stretch with Percent Clipping" << endl;
              cout << range[0] << " to " << range[1] << endl;
              break;
     case 2:  cout << "Histogram Equalization" << endl;
              break;
     case 3:  cout << "Linear Stretch to Percentile Range: ";
              cout << range[0] << " to " << range[1] << endl;
              break;
     default: cout << "Not specified" << endl; // Should never happen!
   }
   cout << "Output ASCII examples list file: " << examples_out_file << endl;
   cout << "Output label map file: " << label_out_file << endl;
   cout << "Output ASCII class label names list file: " << ascii_out_file << endl;
   cout << "Dimension from which a selected element (below)" << endl;
   cout << "is displayed in the 2-D display = " << view_dimension << endl;
   cout << "Selected element for 2-D display = " << view_element << endl;
   if (examples_in_flag)
     cout << "Input ASCII examples list file: " << examples_in_file << endl;
   if (panchromatic_image_flag)
     cout << "Input panchromatic image: " << panchromatic_image_file << endl;
   if (reference_image_flag)
     cout << "Input reference image: " << reference_image_file << endl;

  // Print RHSeg output parameters
   cout << endl << "Parameters input from the RHSeg output parameter file:" << endl;
   cout << endl << "Parameters that specify and describe the RHSeg input data:" << endl;
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
   cout << "Region classes list file name: " << params.region_classes_file << endl;
   if (params.object_labels_map_flag)
     cout << "Object labels map output file name: " << params.object_labels_map_file << endl;
   if (params.region_objects_flag)
     cout << "Region objects output file name: " << params.region_objects_file << endl;

   cout << endl << "Optional parameters that select the contents of the output" << endl;
   cout << "region classes and optional region objects files (above):" << endl;
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
     cout << "Region labels of the neighboring region classess included" << endl;
   if (params.region_nb_objects_flag)
     cout << "Number of region objects contained in regions class included" << endl;
   if (params.region_objects_list_flag)
     cout << "Region labels of region objects contained in region class included" << endl;

   cout << endl << "Optional input parameters that specify optional input files and parameter values:" << endl;
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
   if (params.mask_flag)
   {
     cout << "Input bad data mask file name: " << params.mask_file << endl;
     cout << "Bad data mask flag value = " << params.mask_value << endl;
   }

   cout << endl << "Defaultable required input parameters, along with other associated parameters:" << endl;
   cout <<  "Utilize Spectral Clustering?  " << endl;
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
    case 1:    cout <<  "No." << endl << "Do not normalize input data." << endl;
                break;
    case 2:    cout <<  "Yes." << endl << "Normalize input data across all bands." << endl;
  	        break;
    case 3:    cout <<  "Yes." << endl << "Normalize each band of input data separately." << endl;
  	        break;
   }
   cout << "Number of hierarchical segmentation levels = " << oparams.nb_levels << endl;
   cout << "Number of region classes at hierarchical segmentation level 0: " << oparams.level0_nb_classes << endl;
   if (params.region_nb_objects_flag)
     cout << "Number of region objects at hierarchical segmentation level 0: " << oparams.level0_nb_objects << endl;
   cout << "Input buffer size for region object input at each hiearchical level: ";
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
   cout << "Number of Dimensions = " << params.nb_dimensions << endl;
   return;
 }

} // namespace HSEGTilton
