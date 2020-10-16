// params.cc

#include "params.h"
#include <iostream>
#include <ctime>

#ifdef GDAL
Image inputImage;
Image maskImage;
Image stdDevInImage;
Image stdDevMaskImage;
Image edgeInImage;
Image edgeMaskImage;
Image regionMapInImage;
#endif

namespace HSEGTilton
{

 // Constructor
  Params::Params(const string& value)
  {
    program_mode = 3;
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
    dtype_flag = false;
    mask_flag = false;
    std_dev_image_flag = false;
    std_dev_mask_flag = false;
    edge_image_flag = false;
    edge_mask_flag = false;
    region_map_in_flag = false;
    complete_labeling_flag = false;
    spclust_wght_flag = false;
    gtkmm_flag = false;
    boundary_map_flag = false;
    object_labels_map_flag = false;
    region_objects_flag = false;
    object_conn_type1_flag = false;
    region_sum_flag = false;
    region_std_dev_flag = false;
    region_boundary_npix_flag = false;
    region_threshold_flag = false;
//    region_nghbrs_list_flag = false; // Always assumed true!
    region_nghbrs_list_flag = true;
    region_nb_objects_flag = false;
    region_objects_list_flag = false;
    chk_nregions_flag = false;
    hseg_out_nregions_flag = false;
    hseg_out_thresholds_flag = false;
    gdissim_flag = false;
    merge_accel_flag = false;
   // These parameters have calculated defaults that can be overridden by the user
    rnb_levels_flag = false;
#ifndef PARALLEL
    ionb_levels_flag = false;
#endif
    min_nregions_flag = false;

   // Flags for internal program parameters (NOT under user control).
    region_sumsq_flag = false;
    region_sumxlogx_flag = false;
    padded_flag = false;
    col_offset_flag = false;
    row_offset_flag = false;
#ifdef THREEDIM
    slice_offset_flag = false;
#endif

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

    dissim_crit = DISSIM_CRIT;

    ifstream param_fs(param_file);
    if (!param_fs.is_open())
    {
      cout << "Cannot open input file " << param_file << endl;
      return status;
    }

  // Read initial parameters from parameter file until End-of-File reached
#ifdef GDAL
    int    band;
    double min_value, max_value;
#endif
    mask_value = MASK_VALUE;
    bool log_flag = false;
    int sub_pos;
    string line, sub_string;
#ifdef GDAL
    gdal_input_flag = true;
#else
    gdal_input_flag = false;
#endif
    dtype = Unknown;
#ifdef GDAL
    data_type = GDT_Unknown;
#endif
    while (!param_fs.eof())
    {
      getline(param_fs,line);
      sub_pos = line.find("-");
      while ((!param_fs.eof()) && (sub_pos != 0))
      {
        getline(param_fs,line);
        sub_pos = line.find("-");
      }
      if (line.find("-program_mode") != string::npos)
      {
        sub_string = process_line(line,false);
        if (sub_string == "HSWO")
          program_mode = 1;
        else if (sub_string == "HSEG")
          program_mode = 2;
        else if (sub_string == "RHSEG")
          program_mode = 3;
        else
        {
          cout << "ERROR: Invalid program mode. Must be \"HSWO,\" \"HSEG,\" or \"RHSEG\" (no quotes)" << endl;
          program_mode = 0;
        }
      }
      if (line.find("-gdal_input") != string::npos)
      {
        sub_string = process_line(line,false);
        sub_pos = atoi(sub_string.c_str());
        if (sub_pos == 1)
          gdal_input_flag = true;
        else
          gdal_input_flag = false;
      }
      if (line.find("-input_image") != string::npos)
      {
        input_image_file = process_line(line,false);
#ifdef GDAL
        if (gdal_input_flag)
        {
          input_image_flag = inputImage.open(input_image_file);
          if (input_image_flag)
          {
            ncols = inputImage.get_ncols();
            ncols_flag = true;
            nrows = inputImage.get_nrows();
            nrows_flag = true;
            nbands = inputImage.get_nbands();
            nbands_flag = true;
            dtype = inputImage.get_dtype();
            dtype_flag = true;
            data_type = inputImage.get_data_type();
            switch (data_type)
            {
              case GDT_Byte:    break;
              case GDT_UInt16:  break;
              case GDT_Int16:   min_value = 0;
                                for (band = 0; band < nbands; band++)
                                  if (min_value > inputImage.getMinimum(band))
                                    min_value = inputImage.getMinimum(band);
                                if (min_value < 0.0)
                                {
                                  cout << "WARNING: For the input image data file " << input_image_file << "," << endl;
                                  cout << "short integer data will be converted to 32-bit float data." << endl;
                                  dtype = Float32;
                                }
                                else
                                {
                                  cout << "WARNING: For the input image data file " << input_image_file << "," << endl;
                                  cout << "short integer data will be converted to unsigned short integer data." << endl;
                                  dtype = UInt16;
                                }
                                break;
              case GDT_UInt32:  cout << "NOTE: For the input image data file " << input_image_file << "," << endl;
                                cout << "32-bit unsigned integer data will be converted to 32-bit float data." << endl;
                                dtype = Float32;
                                break;
              case GDT_Int32:   cout << "NOTE: For the input image data file " << input_image_file << "," << endl;
                                cout << "32-bit integer data will be converted to 32-bit float data." << endl;
                                dtype = Float32;
                                break;
              case GDT_Float32: break;
              case GDT_Float64: cout << "WARNING: For the input image data file " << input_image_file << "," << endl;
                                cout << "64-bit double data will be converted to 32-bit float data." << endl;
                                cout << "Out of ranges value will not be read properly." << endl;
                                dtype = Float32;
                                break;
              default:          cout << "Unknown or unsupported image data type for input image data file ";
                                cout << input_image_file << endl;
                                return false;
            } 
          }
          else
          {
            cout << "WARNING:  Input image file " << input_image_file << " is of unknown format. Assumed to be a raw binary input file." << endl;
            input_image_flag = true;
            gdal_input_flag = false;
          }
        } // if (gdal_input_flag)
        else
        {
#endif //
          input_image_flag = true;
#ifdef GDAL
        }
#endif
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
          dtype = UInt8;
          dtype_flag = true;
        }
        else if (sub_string == "UInt16")
        {
          dtype = UInt16;
          dtype_flag = true;
        }
        else if (sub_string == "Float32")
        {
          dtype = Float32;
          dtype_flag = true;
        }
        else
        { 
          dtype = Unknown;
          dtype_flag = false;
        }
      }
      if ((line.find("-mask") != string::npos) && (line.find("-mask_value") == string::npos))
      {
        mask_file = process_line(line,false);
#ifdef GDAL
        if (gdal_input_flag)
        {
          mask_flag = maskImage.open(mask_file);
          if (mask_flag)
          {
            min_value = 0;
            max_value = 255;
            if (min_value > maskImage.getMinimum(0))
              min_value = maskImage.getMinimum(0);
            if (max_value < maskImage.getMaximum(0))
              max_value = maskImage.getMaximum(0);
            if ((min_value < 0.0) || (max_value > 255))
            {
              cout << "ERROR: Invalid range for input mask image. Must be in range 0 to 255." << endl;
              mask_flag = false;
              return status;
            }
            if (maskImage.no_data_value_valid(0))
            {
              min_value = maskImage.get_no_data_value(0);
              if ((min_value < 256.0) && (min_value >= 0.0))
                mask_value = (unsigned char) min_value;
              else
              {
                cout << "ERROR: Mask value, " << mask_value << ", is out of 8-bit range" << endl;
                mask_flag = false;
                return status;
              }
            }
            else
              mask_value = MASK_VALUE;
          }
          else
          {
            cout << "WARNING:  Input mask file " << mask_file << " is of unknown format. Assumed to be a raw binary input file." << endl;
            mask_flag = true;
          }
        } // if (gdal_input_flag)
        else
        {
#endif //
          mask_flag = true;
#ifdef GDAL
        }
#endif
      }
      if (line.find("-mask_value") != string::npos)
      {
        sub_string = process_line(line,false);
        mask_value = atoi(sub_string.c_str());
      }
      if (line.find("-std_dev_image") != string::npos)
      {
        std_dev_image_file = process_line(line,false);
#ifdef GDAL
        if (gdal_input_flag)
        {
          std_dev_image_flag = stdDevInImage.open(std_dev_image_file);
          if (std_dev_image_flag)
          {
            switch (stdDevInImage.get_data_type())
            {
              case GDT_Byte:    break;
              case GDT_UInt16:  break;
              case GDT_Int16:   break;
              case GDT_UInt32:  break;
              case GDT_Int32:   break;
              case GDT_Float32: break;
              case GDT_Float64: cout << "WARNING: For the input standard devation image file " << std_dev_image_file << "," << endl;
                                cout << "64-bit double data will be converted to 32-bit float data." << endl;
                                cout << "Out of ranges value will not be read properly." << endl;
                                break;
              default:          cout << "Unknown or unsupported image data type for input standard deviation image file ";
                                cout << std_dev_image_file << endl;
                                return false;
            }
          }
          else
          {
            cout << "WARNING:  Input standard deviation image file " << std_dev_image_file << " is of unknown format. Assumed to be a raw binary input file." << endl;
            std_dev_image_flag = true;
          }
        } // if (gdal_input_flag)
        else
        {
#endif //
          std_dev_image_flag = true;
#ifdef GDAL
        }
#endif
      }
      if (line.find("-std_dev_mask") != string::npos)
      {
        std_dev_mask_file = process_line(line,false);
#ifdef GDAL
        if (gdal_input_flag)
        {
          std_dev_mask_flag = stdDevMaskImage.open(std_dev_mask_file);
          if (!std_dev_mask_flag)
          {
            cout << "WARNING:  Input standard deviation mask file " << std_dev_mask_file << " is of unknown format. Assumed to be a raw binary input file." << endl;
            std_dev_mask_flag = true;
          }
        } // if (gdal_input_flag)
        else
        {
#endif //
          std_dev_mask_flag = true;
#ifdef GDAL
        }
#endif
      }
      if (line.find("-edge_image") != string::npos)
      {
        edge_image_file = process_line(line,false);
#ifdef GDAL
        if (gdal_input_flag)
        {
          edge_image_flag = edgeInImage.open(edge_image_file);
          if (edge_image_flag)
          {
            switch (edgeInImage.get_data_type())
            {
              case GDT_Byte:    break;
              case GDT_UInt16:  break;
              case GDT_Int16:   break;
              case GDT_UInt32:  break;
              case GDT_Int32:   break;
              case GDT_Float32: break;
              case GDT_Float64: cout << "WARNING: For the input edge image file " << edge_image_file << "," << endl;
                                cout << "64-bit double data will be converted to 32-bit float data." << endl;
                                cout << "Out of ranges value will not be read properly." << endl;
                                break;
              default:          cout << "Unknown or unsupported image data type for input edge image file ";
                                cout << edge_image_file << endl;
                                return false;
            }
          }
          else
          {
            cout << "WARNING:  Input edge image file " << edge_image_file << " is of unknown format. Assumed to be a raw binary input file." << endl;
            edge_image_flag = true;
          }
        } // if (gdal_input_flag)
        else
        {
#endif //
          edge_image_flag = true;
#ifdef GDAL
        }
#endif
      }
      if (line.find("-edge_mask") != string::npos)
      {
        edge_mask_file = process_line(line,false);
#ifdef GDAL
        if (gdal_input_flag)
        {
          edge_mask_flag = edgeMaskImage.open(edge_mask_file);
          if (!edge_mask_flag)
          {
            cout << "WARNING:  Input edge mask file " << edge_mask_file << " is of unknown format. Assumed to be a raw binary input file." << endl;
            edge_mask_flag = true;
          }
        } // if (gdal_input_flag)
        else
        {
#endif //
          edge_mask_flag = true;
#ifdef GDAL
        }
#endif
      }
      if (line.find("-region_map_in") != string::npos)
      {
        region_map_in_file = process_line(line,false);
#ifdef GDAL
        if (gdal_input_flag)
        {
          region_map_in_flag = regionMapInImage.open(region_map_in_file);
          if (region_map_in_flag)
          {
            switch (regionMapInImage.get_data_type())
            {
              case GDT_Byte:    break;
              case GDT_UInt16:  break;
              case GDT_Int16:   min_value = 0;
                                for (band = 0; band < nbands; band++)
                                  if (min_value > regionMapInImage.getMinimum(band))
                                    min_value = regionMapInImage.getMinimum(band);
                                if (min_value < 0.0)
                                {
                                  cout << "WARNING: For the input region map data file " << region_map_in_file << "," << endl;
                                  cout << "short integer data will be converted to 32-bit unsigned integer data." << endl;
                                  cout << "Negative values will not be read properly." << endl;
                                }
                                break;
              case GDT_UInt32:  break;
              case GDT_Int32:   min_value = 0;
                                for (band = 0; band < nbands; band++)
                                  if (min_value > regionMapInImage.getMinimum(band))
                                    min_value = regionMapInImage.getMinimum(band);
                                if (min_value < 0.0)
                                {
                                  cout << "WARNING: For the input region map data file " << region_map_in_file << "," << endl;
                                  cout << "32-bit integer data will be converted to 32-bit unsigned integer data." << endl;
                                  cout << "Negative values will not be read properly." << endl;
                                }
                                break;
              case GDT_Float32: cout << "WARNING: For the input region map data file " << region_map_in_file << "," << endl;
                                cout << "32-bit float data will be converted to 32-bit unsigned integer data." << endl;
                                cout << "Out of ranges value will not be read properly." << endl;
                                break;
              case GDT_Float64: cout << "WARNING: For the input region map data file " << region_map_in_file << "," << endl;
                                cout << "64-bit double data will be converted to 32-bit unsigned integer data." << endl;
                                cout << "Out of ranges value will not be read properly." << endl;
                                break;
              default:          cout << "Unknown or unsupported image data type for input region map data file ";
                                cout << region_map_in_file << endl;
                                return false;
            }
          }
          else
          {
            cout << "WARNING:  Input region map file " << region_map_in_file << " is of unknown format. Assumed to be a raw binary input file." << endl;
            region_map_in_flag = true;
          }
        } // if (gdal_input_flag)
        else
        {
#endif //
          region_map_in_flag = true;
#ifdef GDAL
        }
#endif
      }
      if (line.find("-spclust_wght") != string::npos)
      {
        sub_string = process_line(line,false);
        spclust_wght = (float) atof(sub_string.c_str());
        spclust_wght_flag = ((spclust_wght >= 0.0) && (spclust_wght <= 1.0));
      }
      if (line.find("-dissim_crit") != string::npos)
      {
        sub_string = process_line(line,false);
        dissim_crit = atoi(sub_string.c_str());
        if ((dissim_crit < 1) || (dissim_crit > 11))
        {
          cout << "WARNING: Dissimilarity criterion " << dissim_crit << " is invalid. Reset to default " << DISSIM_CRIT << endl;
          dissim_crit = DISSIM_CRIT;
        }
      }
      if (line.find("-log") != string::npos)
      {
        log_file = process_line(line,false);
        log_flag = true;
      }
    }

    param_fs.close();

// Exit with false status if a required parameter is not set, or an improper parameter
// setting is detected.
    switch(program_mode)
    {
       case 1:  rnb_levels = 1;
                rnb_levels_flag = true;
                spclust_wght = 0.0;
                spclust_wght_flag = false;
                break;
       case 2:  rnb_levels = 1;
                rnb_levels_flag = true;
                break;
       case 3:  break;
       default: break;
    }
    if (!input_image_flag)
    {
      cout << "ERROR: -input_image (Input image data file) is required" << endl;
      return status;
    }
    if (!ncols_flag)
    {
      cout << "ERROR: -ncols (Number of columns in input image data) is required" << endl;
      return status;
    }
    if (!nrows_flag)
    {
      cout << "ERROR: -nrows (Number of rows in input image data) is required" << endl;
      return status;
    }
#ifdef THREEDIM
    if (!nslices_flag)
    {
      cout << "ERROR: -nslices (Number of slices in input image data) is required" << endl;
      return status;
    }
#endif // THREEDIM
    if (!nbands_flag)
    {
      cout << "ERROR: -nbands (Number of spectral bands in input image data) is required" << endl;
      return status;
    }
    if (!dtype_flag)
    {
      cout << "ERROR: -dtype (Data type of input image data) is required" << endl;
      return status;
    }
#ifdef GDAL
    else if (data_type == GDT_Unknown)
    {
      switch(dtype)
      {
        case UInt8:       data_type = GDT_Byte;
                          break;
        case UInt16:      data_type = GDT_UInt16;
                          break;
        case UInt32:      data_type = GDT_UInt32;
                          break;
        case Float32:     data_type = GDT_Float32;
                          break;
        default:          break;
      }
    }
#endif
#ifdef RHSEG_RUN
    if ((program_mode == 3) && (!edge_image_flag))
    {
      cout << "ERROR: -edge_image (Input edge image) is required for RHSEG mode." << endl;
      return status;
    }
#endif
    if ((program_mode != 1) && (!spclust_wght_flag))
    {
      cout << "ERROR: -spclust_wght (Spectral Clustering Weight) is required. The value must be in the range of 0.0 to 1.0 (inclusive)" << endl;
      return status;
    }
    spclust_wght_flag = (spclust_wght != 0.0);
    if (!log_flag)
    {
      cout << "ERROR: -log (Output log file) is required" << endl;
      return status; 
    }
#ifdef GDAL
    if (gdal_input_flag)
    {
      if (mask_flag && maskImage.data_valid())
      {
        if ((ncols != maskImage.get_ncols()) || (nrows != maskImage.get_nrows()))
        {
          cout << "ERROR: " << mask_file << " is not a valid input mask" << endl;
          cout << "Input mask must be have the same number of columns and rows as the input image" << endl;
          return status;
        }
      }
      if (std_dev_image_flag && stdDevInImage.data_valid())
      {
        if ((ncols != stdDevInImage.get_ncols()) || (nrows != stdDevInImage.get_nrows()))
        {
          cout << "ERROR: " << std_dev_image_file << " is not a valid input standard deviation image" << endl;
          cout << "Input standard deviation image must be have the same number of columns and rows as the input image" << endl;
          return status;
        } 
      }
      if (std_dev_mask_flag && stdDevMaskImage.data_valid())
      {
        if ((ncols != stdDevMaskImage.get_ncols()) || (nrows != stdDevMaskImage.get_nrows()))
        {
          cout << "ERROR: " << std_dev_mask_file << " is not a valid input standard deviation mask" << endl;
          cout << "Input standard deviation mask must be have the same number of columns and rows as the input image" << endl;
          return status;
        } 
      }
      if (edge_image_flag && edgeInImage.data_valid())
      {
        if ((ncols != edgeInImage.get_ncols()) || (nrows != edgeInImage.get_nrows()))
        {
          cout << "ERROR: " << edge_image_file << " is not a valid input edge image" << endl;
          cout << "Input edge image must be have the same number of columns and rows as the input image" << endl;
          return status;
        } 
      }
      if (edge_mask_flag && edgeMaskImage.data_valid())
      {
        if ((ncols != edgeMaskImage.get_ncols()) || (nrows != edgeMaskImage.get_nrows()))
        {
          cout << "ERROR: " << edge_mask_file << " is not a valid input edge mask" << endl;
          cout << "Input edge mask must be have the same number of columns and rows as the input image" << endl;
          return status;
        } 
      }
      if (region_map_in_flag && regionMapInImage.data_valid())
      {
        if ((ncols != regionMapInImage.get_ncols()) || (nrows != regionMapInImage.get_nrows()))
        {
          cout << "ERROR: " << region_map_in_file << " is not a valid input region map" << endl;
          cout << "Input region map must be have the same number of columns and rows as the input image" << endl;
          return status;
        } 
      }
    }
    else
    {
#ifdef THREEDIM
      inputImage.open(input_image_file, ncols, nrows, nslices, nbands, dtype);
      if (mask_flag)
        maskImage.open(mask_file, ncols, nrows, nslices, 1, UInt8);
      if (std_dev_image_flag)
        stdDevInImage.open(std_dev_image_file, ncols, nrows, nslices, 1, Float32);
      if (std_dev_mask_flag)
        stdDevMaskImage.open(std_dev_mask_file, ncols, nrows, nslices, 1, UInt8);
      if (edge_image_flag)
        edgeInImage.open(edge_image_file, ncols, nrows, nslices, 1, Float32);
      if (edge_mask_flag)
        edgeMaskImage.open(edge_mask_file, ncols, nrows, nslices, 1, UInt8);
      if (region_map_in_flag)
        regionMapInImage.open(region_map_in_file, ncols, nrows, nslices, 1, UInt32);
#else
      inputImage.open(input_image_file, ncols, nrows, nbands, dtype);
      if (mask_flag)
        maskImage.open(mask_file, ncols, nrows, 1, UInt8);
      if (std_dev_image_flag)
        stdDevInImage.open(std_dev_image_file, ncols, nrows, 1, Float32);
      if (std_dev_mask_flag)
        stdDevMaskImage.open(std_dev_mask_file, ncols, nrows, 1, UInt8);
      if (edge_image_flag)
        edgeInImage.open(edge_image_file, ncols, nrows, 1, Float32);
      if (edge_mask_flag)
        edgeMaskImage.open(edge_mask_file, ncols, nrows, 1, UInt8);
      if (region_map_in_flag)
        regionMapInImage.open(region_map_in_file, ncols, nrows, 1, UInt32);
#endif
    }
#endif

#ifdef GDAL
    if (gdal_input_flag)
    {
      output_driver_description = inputImage.get_driver_description();
#ifdef RHSEG_RUN
      if ((output_driver_description == "BMP") || (output_driver_description == "PNG") || (output_driver_description == "PNG") ||
          (output_driver_description == "JPEG") || (output_driver_description == "XPM") || (output_driver_description == "GIF"))
      {
        cout << "Since the input image format, " << output_driver_description;
        cout << ", does not support 16 and/or 32 bit data," << endl;
        output_driver_description = OUTPUT_DRIVER_DESCRIPTION;
        cout << "output image files will be created in " << output_driver_description << " format" << endl;
      }
#endif
    }
#endif
 
    int string_first_pos, string_last_pos;
#ifdef WINDOWS
    string_first_pos = input_image_file.find_last_of("\\");
#else
    string_first_pos = input_image_file.find_last_of("/");
#endif
    if (string_first_pos == (int) string::npos)
      string_first_pos = 0;
    else
      string_first_pos++;
    string_last_pos = input_image_file.find_last_of(".");
#ifdef GDAL
    if (output_driver_description == "ENVI")
    {
      suffix = "";
    }
    else if (string_last_pos == (int) string::npos)
#else
    if (string_last_pos == (int) string::npos)
#endif
    {
      suffix = "";
    }
    else
    {
      suffix = input_image_file.substr(string_last_pos,string::npos);
    }
    prefix = input_image_file.substr(string_first_pos,(string_last_pos-string_first_pos));

    sub_pos = prefix.find(".");
    while (sub_pos != (int) string::npos)
    {
      prefix[sub_pos] = '_';
      sub_pos = prefix.find(".");
    }

  // Determine number of dimensions
    nb_dimensions = 0;
#ifdef THREEDIM
    if ((ncols > 1) && (nrows == 1) && (nslices == 1))
      nb_dimensions = 1;
    if ((ncols > 1) && (nrows > 1) && (nslices == 1))
      nb_dimensions = 2;
    if ((ncols > 1) && (nrows > 1) && (nslices > 1))
      nb_dimensions = 3;
    if (nb_dimensions == 0)
    {
      cout << "Unexpected dimensions: For 1-D, must have nrows = nslices = 1 and ncols > 1;" << endl;
      cout << "for 2-D must have nslices = 1, ncols > 1 and nrows > 1" << endl;
      return status;
    }
#else
    if ((ncols > 1) && (nrows == 1))
      nb_dimensions = 1;
    if ((ncols > 1) && (nrows > 1))
      nb_dimensions = 2;
    if (nb_dimensions == 0)
    {
      cout << "Unexpected dimensions: For 1-D, must have nrows = 1 and ncols > 1" << endl;
      return status;
    }
#endif

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

    class_labels_map_file = current_folder + prefix + CLASS_LABELS_MAP_FILE;
    boundary_map_file = current_folder + prefix + BOUNDARY_MAP;
    boundary_map_flag = BOUNDARY_MAP_FLAG;
    region_classes_file = current_folder + prefix + REGION_CLASSES;
    object_labels_map_file = current_folder + prefix + OBJECT_LABELS_MAP;
    region_objects_file = current_folder + prefix + REGION_OBJECTS;
    if (suffix.length() > 0)
    {
      class_labels_map_file += suffix;
      boundary_map_file += suffix;
      object_labels_map_file += suffix;
    }
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

#if (defined(CONSOLIDATED) || defined(RHSEG_RUN))
    oparam_file = current_folder + prefix + ".oparam";
#else
#ifdef RHSEG_SETUP
  oparam_file = current_folder + "rhseg_run.params";
#else
  oparam_file = "";
#endif
#endif

    region_sum_flag = false;
    if (nbands < REGION_SUM_FLAG_MAX)
      region_sum_flag = true;
    region_std_dev_flag = REGION_STD_DEV_FLAG;
    region_boundary_npix_flag = REGION_BOUNDARY_NPIX_FLAG;
    region_threshold_flag = REGION_THRESHOLD_FLAG;
    region_nghbrs_list_flag = REGION_NGHBRS_LIST_FLAG;
    
 // Set default neighborhood connectivity (depends in number of dimensions!)
    switch (nb_dimensions)
    {
      case 1:  conn_type = 1;
               break;
      case 2:  conn_type = 2;
               break;
      case 3:  conn_type = 3;
               break;
      default: cout << "ERROR: Invalid value for nb_dimensions (=" << nb_dimensions << ")" << endl;
               return false;
    }

    hseg_out_nregions_flag = HSEG_OUT_NREGIONS_FLAG;
    hseg_out_thresholds_flag = HSEG_OUT_THRESHOLDS_FLAG;
    conv_nregions = CONV_NREGIONS;
    gdissim_flag = GDISSIM_FLAG;
    merge_accel_flag = MERGE_ACCEL_FLAG;

  // Program defaults usually utilized
    debug = DEBUG_DEFAULT_VALUE;
    normind = NORMIND;
    init_threshold = INIT_THRESHOLD;
    random_init_seed_flag = false;
    initial_merge_flag = false;
    sort_flag = SORT_FLAG;
    std_dev_wght = STD_DEV_WGHT;
    if (edge_image_flag)
      edge_threshold = EDGE_THRESHOLD;
    else
      edge_threshold = 0.0;
    edge_power = 1.0;
    if (edge_image_flag)
      edge_wght = EDGE_WGHT;
    else
      edge_wght = 0.0;
    if (program_mode == 3)
      seam_edge_threshold = SEAM_EDGE_THRESHOLD;
    else
      seam_edge_threshold = 0.0;
    spclust_min = 0;
    if (spclust_wght > 0.0)
      spclust_min = SPCLUST_MIN;
    spclust_max = 0;
    if (spclust_wght > 0.0)
      spclust_max = SPCLUST_MAX;
    edge_dissim_option = EDGE_DISSIM_OPTION;
    if (program_mode == 1)
      edge_dissim_option = 1;

// Set rnb_levels, ionb_levels and min_nregions in calc_defaults
    bool status = true;
#ifdef GTKMM
    if (gtkmm_flag)
      status = calc_defaults(); // Only needed at this point in GUI version
#endif

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
    bool region_nb_objects_specified = false;
    bool region_objects_list_specified = false;
    int band;
    int sub_pos;
#ifdef GDAL
    int dot_pos, slash_pos;
#endif
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
#ifdef GDAL
        if (output_driver_description == "ENVI")
        {
          dot_pos = sub_string.find_last_of(".");
          if (dot_pos != (int) string::npos)
          {
#ifdef WINDOWS
            slash_pos = sub_string.find_last_of("\\");
#else
            slash_pos = sub_string.find_last_of("/");
#endif
            if (slash_pos != (int) string::npos)
            {
              if (slash_pos < dot_pos)
              {
                class_labels_map_file = sub_string.substr(0,sub_pos);
                dot_pos = class_labels_map_file.find(".");
                while ((dot_pos != (int) string::npos) && (slash_pos < dot_pos))
                {
                  class_labels_map_file[sub_pos] = '_';
                  dot_pos = class_labels_map_file.find(".");
                }
              }
              else
                class_labels_map_file = sub_string;
            }
            else
            {
              class_labels_map_file = sub_string.substr(0,sub_pos);
              dot_pos = class_labels_map_file.find(".");
              while ((dot_pos != (int) string::npos) && (slash_pos < dot_pos))
              {
                class_labels_map_file[sub_pos] = '_';
                dot_pos = class_labels_map_file.find(".");
              }
            }
            if (sub_string != class_labels_map_file)
              cout << "class_labels_map file name modified to accomodate ENVI format: " << class_labels_map_file << endl;
          }
          else
            class_labels_map_file = sub_string;
        }
        else
#endif
          class_labels_map_file = sub_string;
      }
      if (line.find("-boundary_map") != string::npos)
      {
        sub_string = process_line(line,false);
#ifdef GDAL
        if (output_driver_description == "ENVI")
        {
          dot_pos = sub_string.find_last_of(".");
          if (dot_pos != (int) string::npos)
          {
#ifdef WINDOWS
            slash_pos = sub_string.find_last_of("\\");
#else
            slash_pos = sub_string.find_last_of("/");
#endif
            if (slash_pos != (int) string::npos)
            {
              if (slash_pos < dot_pos)
              {
                boundary_map_file = sub_string.substr(0,sub_pos);
                dot_pos = boundary_map_file.find(".");
                while ((dot_pos != (int) string::npos) && (slash_pos < dot_pos))
                {
                  boundary_map_file[sub_pos] = '_';
                  dot_pos = boundary_map_file.find(".");
                }
              }
              else
                boundary_map_file = sub_string;
            }
            else
            {
              boundary_map_file = sub_string.substr(0,sub_pos);
              dot_pos = boundary_map_file.find(".");
              while ((dot_pos != (int) string::npos) && (slash_pos < dot_pos))
              {
                boundary_map_file[sub_pos] = '_';
                dot_pos = boundary_map_file.find(".");
              }
            }
            if (sub_string != boundary_map_file)
              cout << "boundary_map file name modified to accomodate ENVI format: " << boundary_map_file << endl;
          }
          else
            boundary_map_file = sub_string;
        }
        else
#endif
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
#ifdef GDAL
        if (output_driver_description == "ENVI")
        {
          dot_pos = sub_string.find_last_of(".");
          if (dot_pos != (int) string::npos)
          {
#ifdef WINDOWS
            slash_pos = sub_string.find_last_of("\\");
#else
            slash_pos = sub_string.find_last_of("/");
#endif
            if (slash_pos != (int) string::npos)
            {
              if (slash_pos < dot_pos)
              {
                object_labels_map_file = sub_string.substr(0,sub_pos);
                dot_pos = object_labels_map_file.find(".");
                while ((dot_pos != (int) string::npos) && (slash_pos < dot_pos))
                {
                  object_labels_map_file[sub_pos] = '_';
                  dot_pos = object_labels_map_file.find(".");
                }
              }
              else
                object_labels_map_file = sub_string;
            }
            else
            {
              object_labels_map_file = sub_string.substr(0,sub_pos);
              dot_pos = object_labels_map_file.find(".");
              while ((dot_pos != (int) string::npos) && (slash_pos < dot_pos))
              {
                object_labels_map_file[sub_pos] = '_';
                dot_pos = object_labels_map_file.find(".");
              }
            }
            if (sub_string != object_labels_map_file)
              cout << "object_labels_map file name modified to accomodate ENVI format: " << object_labels_map_file << endl;
          }
          else
            object_labels_map_file = sub_string;
        }
        else
#endif
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
          region_objects_list_specified = true;
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
/*
      if (line.find("-region_nghbrs_list") != string::npos)
      {
        sub_string = process_line(line,false);
        sub_pos = atoi(sub_string.c_str());
        if (sub_pos == 1)
          region_nghbrs_list_flag = true;
        else
          region_nghbrs_list_flag = false;
      } 
*/
      if (line.find("-region_nb_objects") != string::npos)
      {
        sub_string = process_line(line,false);
        sub_pos = atoi(sub_string.c_str());
        if (sub_pos == 1)
          region_nb_objects_flag = true;
        else
          region_nb_objects_flag = false;
        region_nb_objects_specified = true;
      }
      if (line.find("-std_dev_wght") != string::npos)
      {
        sub_string = process_line(line,false);
        std_dev_wght = atof(sub_string.c_str());
      }
      if (line.find("-edge_power") != string::npos)
      {
        sub_string = process_line(line,false);
        edge_power = atof(sub_string.c_str());
      }
      if (line.find("-edge_threshold") != string::npos)
      {
        sub_string = process_line(line,false);
        edge_threshold = atof(sub_string.c_str());
      }
      if (line.find("-edge_wght") != string::npos)
      {
        sub_string = process_line(line,false);
        edge_wght = atof(sub_string.c_str());
      }
      if (line.find("-seam_edge_threshold") != string::npos)
      {
        sub_string = process_line(line,false);
        seam_edge_threshold = atof(sub_string.c_str());
      }
      if (line.find("-conn_type") != string::npos)
      {
        sub_string = process_line(line,false);
        conn_type = atoi(sub_string.c_str());
        switch (nb_dimensions)
        {
          case 1:  if ((conn_type < 1) || (conn_type > 4))
                   {
                     cout << "WARNING: Connectivity type " << conn_type << " is invalid. Reset to default 1" << endl;
                     conn_type = 1;
                   }
                   break;
          case 2:  if ((conn_type < 1) || (conn_type > 5))
                   {
                     cout << "WARNING: Connectibity type " << conn_type << " is invalid. Reset to default 2" << endl;
                     conn_type = 2;
                   }
                   break;
          case 3:  if ((conn_type < 1) || (conn_type > 4))
                   {
                     cout << "WARNING: Connectibity type " << conn_type << " is invalid. Reset to default 3" << endl;
                     conn_type = 3;
                   }
                   break;
          default: cout << "WARNING: Invalid value for nb_dimensions (=" << nb_dimensions << ")" << endl;
                   break;
        }
      }
      if (line.find("-chk_nregions") != string::npos)
      {
        sub_string = process_line(line,false);
        chk_nregions = atoi(sub_string.c_str());
        chk_nregions_flag = true;
        hseg_out_nregions_flag = false;
        hseg_out_thresholds_flag = false;
      }
      if (line.find("-hseg_out_nregions") != string::npos)
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
          hseg_out_nregions.push_back(atoi(sub_string.c_str()));
        }
        nb_hseg_out_nregions = hseg_out_nregions.size();
        chk_nregions_flag = false;
        hseg_out_nregions_flag = true;
        hseg_out_thresholds_flag = false;
      }
      if (line.find("-hseg_out_thresholds") != string::npos)
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
          hseg_out_thresholds.push_back(atof(sub_string.c_str()));
        }
        nb_hseg_out_thresholds = hseg_out_thresholds.size();
        chk_nregions_flag = false;
        hseg_out_nregions_flag = false;
        hseg_out_thresholds_flag = true;
      }
      if (line.find("-conv_nregions") != string::npos)
      {
        sub_string = process_line(line,false);
        conv_nregions = atoi(sub_string.c_str());
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
      if (line.find("-normind") != string::npos)
      {
        sub_string = process_line(line,false);
        normind = atoi(sub_string.c_str());
        if ((normind < 1) || (normind > 3))
        {
          cout << "WARNING: Image normalization type " << normind << " is invalid. Reset to default " << NORMIND << endl;
          normind = NORMIND;
        }
      }
      if (line.find("-init_threshold") != string::npos)
      {
        sub_string = process_line(line,false);
        init_threshold = atof(sub_string.c_str());
      }
      if (line.find("-initial_merge_npix") != string::npos)
      {
        sub_string = process_line(line,false);
        if (atoi(sub_string.c_str()) > 0)
        {
          initial_merge_npix = atoi(sub_string.c_str());
          initial_merge_flag = true;
        }
      }
      if (line.find("-random_init_seed") != string::npos)
      {
        sub_string = process_line(line,false);
        sub_pos = atoi(sub_string.c_str());
        if (sub_pos == 1)
          random_init_seed_flag = true;
        else
          random_init_seed_flag = false;
      }
      if (line.find("-sort") != string::npos)
      {
        sub_string = process_line(line,false);
        sub_pos = atoi(sub_string.c_str());
        if (sub_pos == 1)
          sort_flag = true;
        else
          sort_flag = false;
      }
      if (line.find("-edge_dissim_option") != string::npos)
      {
        sub_string = process_line(line,false);
        edge_dissim_option = atoi(sub_string.c_str());
        if ((edge_dissim_option < 1) || (edge_dissim_option > 2))
        {
          cout << "WARNING: Edge dissimilarity option " << edge_dissim_option << " is invalid. Reset to default " << EDGE_DISSIM_OPTION << endl;
          edge_dissim_option = EDGE_DISSIM_OPTION;
        }
        if ((edge_dissim_option == 2) && ((program_mode == 1) || (spclust_wght == 0.0)))
        {
          cout << "\"Merge Suppression Option\" not allowed for HSWO program mode" << endl;
          edge_dissim_option = 1;
        }
      }
      if (line.find("-rnb_levels") != string::npos)
      {
        sub_string = process_line(line,false);
        rnb_levels = atoi(sub_string.c_str());
        if ((rnb_levels_flag) && (rnb_levels != 1) && (program_mode != 3))
        {
          cout << "WARNING: -rnb_levels cannot be reset in the specified program mode." << endl;
        }
        else
        {
          rnb_levels_flag = true;
        }
      }
#ifndef PARALLEL
      if (line.find("-ionb_levels") != string::npos)
      {
        sub_string = process_line(line,false);
        ionb_levels = atoi(sub_string.c_str());
        ionb_levels_flag = true;
      }
#endif
      if (line.find("-min_nregions") != string::npos)
      {
        sub_string = process_line(line,false);
        min_nregions = atoi(sub_string.c_str());
        min_nregions_flag = true;
      }
      if (line.find("-spclust_min") != string::npos)
      {
        sub_string = process_line(line,false);
        spclust_min = (unsigned int) atoi(sub_string.c_str());
      }
      if (line.find("-spclust_max") != string::npos)
      {
        sub_string = process_line(line,false);
        spclust_max = (unsigned int) atoi(sub_string.c_str());
      }
      if (line.find("-merge_acceleration") != string::npos)
      {
        sub_string = process_line(line,false);
        sub_pos = atoi(sub_string.c_str());
        if (sub_pos == 1)
          merge_accel_flag = true;
        else
          merge_accel_flag = false;
      }
    }

    param_fs.close();

    if ((std_dev_image_flag) && ((dissim_crit == 5) || (dissim_crit == 9)))
    {
      if (dissim_crit == 5)
        cout << "WARNING:  For the \"Spectral Information Divergence\" ";
      if (dissim_crit == 9)
        cout << "WARNING:  For the \"Entropy\" ";
      cout << "dissimilarity criterion, cannot use standard deviation feature" << endl;
      std_dev_image_flag = false;
    }
#ifdef RHSEG_RUN
    if (!edge_image_flag)
    {
      if (edge_threshold > 0.0)
      {
        cout << "WARNING: Since an edge_image was not provided, edge_threshold has been reset to 0.0" << endl;
        edge_threshold = 0.0;
      }
      if (edge_wght > 0.0)
      {
        cout << "WARNING: Since an edge_image was not provided, edge_wght has been reset to 0.0" << endl;
        edge_wght = 0.0;
      }
    }
#endif
    if ((object_conn_type1_flag) && (conn_type != 1))
    {
      region_objects_flag = true;
      object_labels_map_flag = true;
    }
    else if ((program_mode == 1) || (spclust_wght == 0.0))
    {
      region_objects_flag = false;
      object_labels_map_flag = false;
    }

    if (!region_nb_objects_specified)
      region_nb_objects_flag = ((spclust_wght_flag) || (object_conn_type1_flag)) && 
                                (object_labels_map_flag) && (region_objects_flag);
    if (!region_objects_list_specified)
      region_objects_list_flag = ((spclust_wght_flag) || (object_conn_type1_flag)) && 
                                 (object_labels_map_flag) && (region_objects_flag);
    if (region_objects_list_flag)
      region_nb_objects_flag = true;
    if (((!spclust_wght_flag) && (!object_conn_type1_flag)) || (!object_labels_map_flag) || (!region_objects_flag))
    {
      region_nb_objects_flag = false;
      region_objects_list_flag = false;
    }

    if ((region_objects_list_flag) || (region_nb_objects_flag))
    {
      if ((!object_labels_map_flag) || (!region_objects_flag))
      {
        cout << "WARNING: Must specify both output files (-object_labels_map and -region_objects)";
        cout << "when requesting region object information." << endl;
        cout << "-region_objects_list and region_nb_objects reset to false (0)" << endl;
        region_objects_list_flag = false;
        region_nb_objects_flag = false;
      }
    }
    if ((object_labels_map_flag) || (region_objects_flag))
    {
      if ((!object_labels_map_flag) || (!region_objects_flag))
      {
        cout << "WARNING: -object_labels_map and -region_objects must BOTH be specified";
        cout << "Any specified values for -object_labels_map and -region_objects ignored." << endl;
        object_labels_map_flag = false;
        region_objects_flag = false;
      }
    }

  // Determine if region sumsq information is required
  // Must be determined here because gdissim_flag is not transmitted to parallel tasks!
    set_region_sumsq_flag();

    bool status;

 // Set the maxnbdir parameter (equals number of neighbors considered)
 // Must be set here because conn_type is not transmitted to parallel tasks!
    status = set_maxnbdir();

    if (status)
      status = calc_defaults();

    return status;
  }

  void Params::set_region_sumsq_flag()
  {
    region_sumsq_flag = false;
    if ((region_std_dev_flag) || (std_dev_image_flag)  || 
        ((gdissim_flag) &&
         ((dissim_crit == 6) || (dissim_crit == 7))))
      region_sumsq_flag = true;

    return;
  }

  bool Params::set_maxnbdir()
  {
    switch (nb_dimensions)
    {
       case 1: switch (conn_type)
               {
                 case 1:   maxnbdir = 2;
                           break;
                 case 2:   maxnbdir = 4;
                           break;
                 case 3:   maxnbdir = 6;
                           break;
                 case 4:   maxnbdir = 8;
                           break;
                 default:  maxnbdir = MAXNBDIR + 1;
                           break;
               }
               break;
       case 2: switch (conn_type)
               {
                 case 1:   maxnbdir = 4;
                           break;
                 case 2:   maxnbdir = 8;
                           break;
                 case 3:   maxnbdir = 12;
                           break;
                 case 4:   maxnbdir = 20;
                           break;
                 case 5:   maxnbdir = 24;
                           break;
                 default:  maxnbdir = MAXNBDIR + 1;
                           break;
               }
               break;
       case 3: switch (conn_type)
               {
                 case 1:   maxnbdir = 6;
                           break;
                 case 2:   maxnbdir = 18;
                           break;
                 case 3:   maxnbdir = 26;
                           break;
                 default:  maxnbdir = MAXNBDIR + 1;
                           break;
               }
               break;
      default: cout << "ERROR: Invalid value for nb_dimensions (=" << nb_dimensions << ")" << endl;
               return false;
    }
  // Initially, object_maxnbdir should be equal to maxnbdir, irrespective of the value of object_conn_type1_flag.
    object_maxnbdir = maxnbdir;
    if (maxnbdir > MAXNBDIR)
    {
      if (debug > 0)
        log_fs << "Too many nearest neighbors selected (" << maxnbdir << ")" << endl;
      else
        cout << "Too many nearest neighbors selected (" << maxnbdir << ")" << endl;
      return false;
    }

    return true;
  }

  bool Params::set_object_maxnbdir()
  {
    if (object_conn_type1_flag)
    {
      switch (nb_dimensions)
      {
         case 1: object_maxnbdir = 2;
                 break;
         case 2: object_maxnbdir = 4;
                 break;
         case 3: object_maxnbdir = 6;
                 break;
        default: cout << "ERROR: Invalid value for nb_dimensions (=" << nb_dimensions << ")" << endl;
                 return false;
      }
    }
    else
      object_maxnbdir = maxnbdir;

    return true;
  }

  // Calculate dependent defaults, based on current parameter settings
  bool Params::calc_defaults()
  {
  // Determine if sumxlogx required
    region_sumxlogx_flag = false;
    if (dissim_crit == 9)
       region_sumxlogx_flag = true;

   // The processing window seam size depends in number of dimensions.
   // The seam size specifies the amount of processing seam overlap that is utilized.
    switch (nb_dimensions)
    {
      case 1:  seam_size = 8;
               break;
      case 2:  seam_size = 4;
               break;
      case 3:  seam_size = 2;
               break;
      default: cout << "ERROR: Invalid value for nb_dimensions (=" << nb_dimensions << ")" << endl;
               return false;
    }

  // Find prime_dimension
    prime_dimension = 0;
#ifdef THREEDIM
    if ((ncols >= nrows) && (ncols >= nslices))
      prime_dimension = 1;
    else if ((nrows > ncols) && (nrows >= nslices))
      prime_dimension = 2;
    else
      prime_dimension = 3;
#else
    if (ncols >= nrows)
      prime_dimension = 1;
    else
      prime_dimension = 2;
#endif

    int level;

    if (rnb_levels_flag)
    {
     // Set recursive sections processing size
      rnb_ncols = ncols;
      rnb_nrows = nrows;
#ifdef THREEDIM
      rnb_nslices = nslices;
#endif
      level = 0;
      for (level = 1; level < rnb_levels; ++level)
      {
        switch (prime_dimension)
        {
          case 1:  rnb_ncols = (rnb_ncols + 1)/2;
                   if (rnb_nrows > rnb_ncols)
                     rnb_nrows = (rnb_nrows + 1)/2;
#ifdef THREEDIM
                   if (rnb_nslices > rnb_ncols)
                     rnb_nslices = (rnb_nslices + 1)/2;
#endif
                   break;
          case 2:  rnb_nrows = (rnb_nrows + 1)/2;
                   if (rnb_ncols > rnb_nrows)
                     rnb_ncols = (rnb_ncols + 1)/2;
#ifdef THREEDIM
                   if (rnb_nslices > rnb_nrows)
                     rnb_nslices = (rnb_nslices + 1)/2;
#endif
                   break;
#ifdef THREEDIM
          case 3:  rnb_nslices = (rnb_nslices + 1)/2;
                   if (rnb_ncols > rnb_nslices)
                     rnb_ncols = (rnb_ncols + 1)/2;
                   if (rnb_nrows > rnb_nslices)
                     rnb_nrows = (rnb_nrows + 1)/2;
                   break;
#endif
          default: break;
        }
        if (min_nregions_flag &&
#ifdef THREEDIM
            ((rnb_ncols*rnb_nrows*rnb_nslices) <= (int) min_nregions))
#else
            ((rnb_ncols*rnb_nrows) <= (int) min_nregions))
#endif
          break;
        if (chk_nregions_flag && (!min_nregions_flag) &&
#ifdef THREEDIM
            ((rnb_ncols*rnb_nrows*rnb_nslices) < (int) chk_nregions))
#else
            ((rnb_ncols*rnb_nrows) < (int) chk_nregions))
#endif
          break;
      } // for (level = 1; level < rnb_levels; ++level)
      if (level < rnb_levels)
      {
        rnb_levels = level;
        cout << "WARNING:  rnb_levels readjusted to " << rnb_levels << endl;
        rnb_ncols = ncols;
        rnb_nrows = nrows;
#ifdef THREEDIM
        rnb_nslices = nslices;
#endif
        level = 0;
        for (level = 1; level < rnb_levels; ++level)
        {
          switch (prime_dimension)
          {
            case 1:  rnb_ncols = (rnb_ncols + 1)/2;
                     if (rnb_nrows > rnb_ncols)
                       rnb_nrows = (rnb_nrows + 1)/2;
#ifdef THREEDIM
                     if (rnb_nslices > rnb_ncols)
                       rnb_nslices = (rnb_nslices + 1)/2;
#endif
                     break;
            case 2:  rnb_nrows = (rnb_nrows + 1)/2;
                     if (rnb_ncols > rnb_nrows)
                       rnb_ncols = (rnb_ncols + 1)/2;
#ifdef THREEDIM
                     if (rnb_nslices > rnb_nrows)
                       rnb_nslices = (rnb_nslices + 1)/2;
#endif
                     break;
#ifdef THREEDIM
            case 3:  rnb_nslices = (rnb_nslices + 1)/2;
                     if (rnb_ncols > rnb_nslices)
                       rnb_ncols = (rnb_ncols + 1)/2;
                     if (rnb_nrows > rnb_nslices)
                       rnb_nrows = (rnb_nrows + 1)/2;
                     break;
#endif
            default: break;
          }
        } // for (level = 1; level < rnb_levels; ++level)
      } // if (level < rnb_levels)
    } // if (rnb_levels_flag)
    else
    {
     // Set default recursive number of levels
      rnb_ncols = ncols;
      rnb_nrows = nrows;
#ifdef THREEDIM
      rnb_nslices = nslices;
#endif
      level = 0;
     // Restrict maximum number of initial regions to the number of data pixels at the deepest level of recutsion
#ifdef THREEDIM
      while ((rnb_ncols*rnb_nrows*rnb_nslices) > MAX_NREGIONS)
#else
      while ((rnb_ncols*rnb_nrows) > MAX_NREGIONS)
#endif
      {
        switch (prime_dimension)
        {
          case 1:  rnb_ncols = (rnb_ncols + 1)/2;
                   if (rnb_nrows > rnb_ncols)
                     rnb_nrows = (rnb_nrows + 1)/2;
#ifdef THREEDIM
                   if (rnb_nslices > rnb_ncols)
                     rnb_nslices = (rnb_nslices + 1)/2;
#endif
                   break;
          case 2:  rnb_nrows = (rnb_nrows + 1)/2;
                   if (rnb_ncols > rnb_nrows)
                     rnb_ncols = (rnb_ncols + 1)/2;
#ifdef THREEDIM
                   if (rnb_nslices > rnb_nrows)
                     rnb_nslices = (rnb_nslices + 1)/2;
#endif
                   break;
#ifdef THREEDIM
          case 3:  rnb_nslices = (rnb_nslices + 1)/2;
                   if (rnb_ncols > rnb_nslices)
                     rnb_ncols = (rnb_ncols + 1)/2;
                   if (rnb_nrows > rnb_nslices)
                     rnb_nrows = (rnb_nrows + 1)/2;
                   break;
#endif
          default: break;
        }
        ++level;
      }
      rnb_levels = level + 1;
      rnb_levels_flag = true;
    } // else (if (!rnb_levels_flag))

   // Make sure the processing window at the deepest level of recursion isn't so large
   // that the maximum region label would exceed the range of an unsigned integer!
#ifdef THREEDIM
    if ((((unsigned int) (rnb_ncols*rnb_nrows*rnb_nslices)) > UINT_MAX) &&
#else
    if ((((unsigned int) (rnb_ncols*rnb_nrows)) > UINT_MAX) &&
#endif
        (!region_map_in_flag))
    {
      cout << "WARNING:  Processing section is too large." << endl;
      cout << "Maximum number of pixels allowed per section is " << UINT_MAX << "." << endl;

      return false;
    }

#ifdef PARALLEL
   // Sanity check (must have onb_levels <= inb_levels <= rnb_levels)
    if ((inb_levels > rnb_levels))
    {
      cout << "ERROR: Cannot have inb_levels > rnb_levels.  Program aborting." << endl;
      return false;
    }
    if ((onb_levels > rnb_levels))
    {
      cout << "ERROR: Cannot have onb_levels > rnb_levels.  Program aborting." << endl;
      return false;
    }
#else
  // Set default data I/O recursive levels (if necessary)
    if (!ionb_levels_flag)
    {
      ionb_levels = 1;
#ifdef THREEDIM
      if ((ncols*nrows*nslices) > (MAX_NPIXELS*36))
#else
      if ((ncols*nrows) > (MAX_NPIXELS*36))
#endif
      {
        ionb_ncols = rnb_ncols;
        ionb_nrows = rnb_nrows;
#ifdef THREEDIM
        ionb_nslices = rnb_nslices;
#endif
        level = rnb_levels-1;
    // Restrict maximum number pixels at data I/O level of recursion to MAX_NPIXELS
#ifdef THREEDIM
        while ((ionb_ncols*ionb_nrows*ionb_nslices) <= MAX_NPIXELS)
#else 
        while ((ionb_ncols*ionb_nrows) <= MAX_NPIXELS)
#endif
        {
          if (level == 0)
            break;
          level--;
          if (ionb_ncols < ncols)
            ionb_ncols *= 2;
          if (ionb_nrows < nrows)
            ionb_nrows *= 2;
#ifdef THREEDIM
          if (ionb_nslices < nslices)
            ionb_nslices *= 2;
#endif
        }
        ionb_levels = level+1;
        if (ionb_levels > rnb_levels)
          ionb_levels = rnb_levels;
        ionb_levels_flag = true;
      }
    } // if (!ionb_levels_flag)
#endif // PARALLEL

   // Determine the recursion mask flags and data padding
   // recur mask flag designations: 0 => undefined, 1 => col only, 2 => row only, 3=> col & row,
   //                               4=> slice only, 5=> col & slice, 6 => row & slice, 7 => col, row & slice
    recur_mask_flags.resize(rnb_levels);
    for (level = 0; level < rnb_levels; level++)
      recur_mask_flags[level] = 0;
    level = rnb_levels - 1;
    recur_mask_flags[level] += 1;
    if (nb_dimensions > 1)
      recur_mask_flags[level] += 2;
#ifdef THREEDIM
    if (nb_dimensions > 2)
      recur_mask_flags[level] += 4;
#endif
  
    padded_ncols = rnb_ncols;
    padded_nrows = rnb_nrows;
#ifdef THREEDIM
    padded_nslices = rnb_nslices;
#endif
    for (level = (rnb_levels-2); level >= 0; level--)
    {
      if (padded_ncols < ncols)
      {
        padded_ncols *= 2;
        recur_mask_flags[level] += 1;
      }
      if (nb_dimensions > 1)
      {
        if (padded_nrows < nrows)
        {
          padded_nrows *= 2;
          recur_mask_flags[level] += 2;
        }
#ifdef THREEDIM
        if (nb_dimensions > 2)
        {
          if (padded_nslices < nslices)
          {
            padded_nslices *= 2;
            recur_mask_flags[level] += 4;
          }
        }
#endif
      }
    }
    padded_flag = false;
#ifdef THREEDIM
    if ((ncols != padded_ncols) ||
        (nrows != padded_nrows) ||
        (nslices != padded_nslices))
#else
    if ((ncols != padded_ncols) ||
        (nrows != padded_nrows))
#endif
      padded_flag = true;

#ifdef RHSEG_RUN
   // Set default for min_regions (if not previously specified)
    if ((rnb_levels > 1) && (!min_nregions_flag))
    {
      bool col_flag, row_flag;
#ifdef THREEDIM
      bool slice_flag;
#endif
      int nb_strides;
#ifdef THREEDIM
      nb_strides = set_recur_flags(recur_mask_flags[rnb_levels-2],col_flag,row_flag,slice_flag);
      min_nregions = (rnb_ncols*rnb_nrows*rnb_nslices)/nb_strides;
#else
      nb_strides = set_recur_flags(recur_mask_flags[rnb_levels-2],col_flag,row_flag);
      min_nregions = (rnb_ncols*rnb_nrows)/nb_strides;
#endif
      min_nregions_flag = true;
    }
#endif

    return true;
  }

#ifdef RHSEG_RUN
/*-----------------------------------------------------------
|
|  Routine Name: calc
|
|       Purpose: Calculates certain HSEG program parameters.
|
|         Input: 
|
|       Returns: bool
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt,MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: November 4, 2003.
| Modifications: May 9, 2005: Create VisiQuest version with global params
|                May 27, 2005: Added temp_file_name generation
|                May 12, 2008: Rewritten as a member function of the Params object class
|
------------------------------------------------------------*/
  bool Params::calc()
  {
    bool col_flag, row_flag;
#ifdef THREEDIM
    bool slice_flag;
#endif
    int level, stride;
#ifdef PARALLEL
    int nb_strides;
   // Make sure the number of tasks required by inb_levels of recursion
   // matches number of parallel processes.
    nb_tasks = 1;
    for (level = 0; level < (inb_levels-1); ++level)
    {
#ifdef THREEDIM
      nb_strides = set_recur_flags(recur_mask_flags[level],col_flag,row_flag,slice_flag);
#else
      nb_strides = set_recur_flags(recur_mask_flags[level],col_flag,row_flag);
#endif
      nb_tasks *= nb_strides;
    }
    if (nb_tasks != numprocs)
    {
      if (debug > 0)
      {
        log_fs << "Calculated number of tasks (" << nb_tasks;
        log_fs << ") does not match number of MPI processes (";
        log_fs << numprocs << ")." << endl;
      }
      else
      {
        cout << "Calculated number of tasks (" << nb_tasks;
        cout << ") does not match number of MPI processes (";
        cout << numprocs << ")." << endl;
      }
      return false;
    }

   // In the parallel case, a particular parallel instance of the program could be called initially
   // from any recursive level from 0 up to inb_levels-1.
   // Find the recursive level at which this instance of the program is called from.
   // Also find the number of parallel tasks called by this instance of the program
   // (nb_tasks = 1 means this instance only calls itself).
    recur_level = -1;
    stride = nb_tasks;
    for (level = 0; level < inb_levels; ++level)
    {
     // Lowest recursive level at which a task is active is where the number of tasks covered by a 
     // processing node at a particular recursive level divides evenly into the task ID
      if (myid == (myid/stride)*stride)
        recur_level = level;
      if (recur_level > -1)
        break;
#ifdef THREEDIM
      nb_strides = set_recur_flags(recur_mask_flags[level],col_flag,row_flag,slice_flag);
#else
      nb_strides = set_recur_flags(recur_mask_flags[level],col_flag,row_flag);
#endif
      stride /= nb_strides;
    }
    set_stride_sections(recur_level,stride,nb_tasks);
    nb_sections = nb_tasks;

    int task;
    if (debug > 1)
    {
      log_fs << "This is task " << myid << "." << endl;
      log_fs << "The lowest recursive level at which this task is active is level " << recur_level << endl;
      if (nb_sections > 1)
      {
        log_fs << "This task covers " << nb_sections << " data sections, starting at section ";
        log_fs << myid << "." << endl;
      // Print out child task IDs
        log_fs << "It calls tasks:";
        for (task = myid + stride;
             task < myid + nb_tasks;
             task += stride)
          log_fs << " " << task;
        log_fs << endl;
        log_fs << "(nb_strides = " << nb_strides << ")" << endl;
      }
    // No child tasks for nb_sections == 1
      else
        log_fs << "This task covers data section " << myid << "." << endl;
    }
#else
   // In the serial case, the program is always at recursive level 0 at this point.
    recur_level = 0;
    set_stride_sections(recur_level,stride,nb_sections);
//    nb_strides = nb_sections/stride;
#endif

#ifdef PARALLEL
   // Set processing dimensions for onb_levels recursive level
    onb_ncols = rnb_ncols;
    onb_nrows = rnb_nrows;
#ifdef THREEDIM
    onb_nslices = rnb_nslices;
#endif
    for (level = (rnb_levels-2); level >= (onb_levels-1); level--)
    {
#ifdef THREEDIM
      set_recur_flags(recur_mask_flags[level],col_flag,row_flag,slice_flag);
#else
      set_recur_flags(recur_mask_flags[level],col_flag,row_flag);
#endif
      if (col_flag)
        onb_ncols *= 2;
      if (row_flag)
        onb_nrows *= 2;
#ifdef THREEDIM
      if (slice_flag)
        onb_nslices *= 2;
#endif
    }

   // Set processing dimensions for inb_levels recursive level
    inb_ncols = rnb_ncols;
    inb_nrows = rnb_nrows;
#ifdef THREEDIM
    inb_nslices = rnb_nslices;
#endif
    for (level = (rnb_levels-2); level >= (inb_levels-1); level--)
    {
#ifdef THREEDIM
      set_recur_flags(recur_mask_flags[level],col_flag,row_flag,slice_flag);
#else
      set_recur_flags(recur_mask_flags[level],col_flag,row_flag);
#endif
      if (col_flag)
        inb_ncols *= 2;
      if (row_flag)
        inb_nrows *= 2;
#ifdef THREEDIM
      if (slice_flag)
        inb_nslices *= 2;
#endif
    }

    if (recur_level < onb_levels)
    {
      pixel_ncols = onb_ncols;
      pixel_nrows = onb_nrows;
#ifdef THREEDIM
      pixel_nslices = onb_nslices;
#endif
    }
    else
    {
      pixel_ncols = rnb_ncols;
      pixel_nrows = rnb_nrows;
#ifdef THREEDIM
      pixel_nslices = rnb_nslices;
#endif
      for (level = (rnb_levels-2); level >= recur_level; level--)
      {
#ifdef THREEDIM
        set_recur_flags(recur_mask_flags[level],col_flag,row_flag,slice_flag);
#else
        set_recur_flags(recur_mask_flags[level],col_flag,row_flag);
#endif
        if (col_flag)
          pixel_ncols *= 2;
        if (row_flag)
          pixel_nrows *= 2;
#ifdef THREEDIM
        if (slice_flag)
          pixel_nslices *= 2;
#endif
      }
    }
#else // !PARALLEL
   // For programming convenience...
    inb_levels = ionb_levels;
    onb_levels = ionb_levels;
   // Set processing dimensions for ionb_levels recursive level
    ionb_ncols = rnb_ncols;
    ionb_nrows = rnb_nrows;
#ifdef THREEDIM
    ionb_nslices = rnb_nslices;
#endif
    for (level = (rnb_levels-2); level >= (ionb_levels-1); level--)
    {
#ifdef THREEDIM
      set_recur_flags(recur_mask_flags[level],col_flag,row_flag,slice_flag);
#else
      set_recur_flags(recur_mask_flags[level],col_flag,row_flag);
#endif
      if (col_flag)
        ionb_ncols *= 2;
      if (row_flag)
        ionb_nrows *= 2;
#ifdef THREEDIM
      if (slice_flag)
        ionb_nslices *= 2;
#endif
    }

   // For programming convenience...
    pixel_ncols = ionb_ncols;
    pixel_nrows = ionb_nrows;
#ifdef THREEDIM
    pixel_nslices = ionb_nslices;
#endif
    inb_ncols = ionb_ncols;
    inb_nrows = ionb_nrows;
#ifdef THREEDIM
    inb_nslices = ionb_nslices;
#endif
    onb_ncols = ionb_ncols;
    onb_nrows = ionb_nrows;
#ifdef THREEDIM
    onb_nslices = ionb_nslices;
#endif

    if (nb_sections > 1)
    {
     // Set temporary file name (including path)
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
      temp_file_name = temp_directory + "\\RHSEG_" + temp_file_name;
#else
      temp_file_name = temp_directory + "/RHSEG_" + temp_file_name;
#endif
      temp_file_name = temp_file_name + ".test_open";
      fstream io_file_fs;
      io_file_fs.open(temp_file_name.c_str( ), ios_base::out );
      if (!io_file_fs)
      {
        if (debug > 0)
          log_fs << "ERROR:  Failed to open temporary file: " << temp_file_name << endl;
        else
          cout << "ERROR:  Failed to open temporary file: " << temp_file_name << endl;
        return false;
      }
      else if (debug > 2)
        log_fs << "Successfully opened temporary file: " << temp_file_name << endl;
      io_file_fs.close( );
    }
#endif

   // Calculate row, column and slice offsets for the data processed by each recursive task (parallel)
   // or I/O processing section (serial)
    int recur_ncols = rnb_ncols;
    int recur_nrows = rnb_nrows;
#ifdef THREEDIM
    int recur_nslices = rnb_nslices;
#endif
    for (level = (rnb_levels-2); level >= recur_level; level--)
    {
#ifdef THREEDIM
      set_recur_flags(recur_mask_flags[level],col_flag,row_flag,slice_flag);
#else
      set_recur_flags(recur_mask_flags[level],col_flag,row_flag);
#endif
      if (col_flag)
        recur_ncols *= 2;
      if (row_flag)
        recur_nrows *= 2;
#ifdef THREEDIM
      if (slice_flag)
        recur_nslices *= 2;
#endif
    }

    int init_col_offset = 0;
    int init_row_offset = 0;
    col_offset.resize(nb_sections);
    row_offset.resize(nb_sections);
#ifdef THREEDIM
    int init_slice_offset = 0;
    slice_offset.resize(nb_sections);
#endif

    col_offset_flag = 1;  // col_offset_flag is always "1".
    row_offset_flag = 0;
    if (nb_dimensions > 1)
      row_offset_flag = 1;
#ifdef THREEDIM
    slice_offset_flag = 0;
    if (nb_dimensions > 2)
      slice_offset_flag = 1;
#endif
#ifdef PARALLEL
#ifdef THREEDIM
    set_offsets(recur_level,nb_sections,myid,
                init_col_offset,init_row_offset,init_slice_offset,
                recur_ncols,recur_nrows,recur_nslices);
#else
    set_offsets(recur_level,nb_sections,myid,
                init_col_offset,init_row_offset,
                recur_ncols,recur_nrows);
#endif
#else
#ifdef THREEDIM
    set_offsets(0,nb_sections,0,
                init_col_offset,init_row_offset,init_slice_offset,
                recur_ncols,recur_nrows,recur_nslices);
#else
    set_offsets(0,nb_sections,0,
                init_col_offset,init_row_offset,
                recur_ncols,recur_nrows);
#endif
#endif
    int section, section_offset = 0;
#ifdef PARALLEL
    section_offset = myid;
#endif
    if (debug > 2)
      for (section = section_offset; section < (nb_sections + section_offset); ++section)
      {
        log_fs << "For data section " << section << ", col_offset[" << (section - section_offset) << "] = ";
        log_fs << col_offset[section - section_offset];
        if (row_offset_flag > 0)
          log_fs << ", row_offset[" << (section - section_offset) << "] = " << row_offset[section - section_offset];
        else
          log_fs << endl;
#ifdef THREEDIM
        if (slice_offset_flag > 0)
          log_fs << " and slice_offset[" << (section - section_offset) << "] = " << slice_offset[section - section_offset] << endl;
        else
#endif
          log_fs << endl;
      }

    return true;
  }
#endif // RHSEG_RUN

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

#ifdef RHSEG_RUN
  void Params::write_oparam(const oParams& oparams)
  {
    short unsigned int index;
    fstream oparam_fs;
 
    oparam_fs.open(oparam_file.c_str( ),ios_base::out);
    switch(program_mode)
    {
       case 1:  oparam_fs << "-program_mode HSWO" << endl;
                break;
       case 2:  oparam_fs << "-program_mode HSEG" << endl;
                break;
       case 3:  oparam_fs << "-program_mode RHSEG" << endl;
                break;
       default: oparam_fs << "-program_mode Unknown" << endl;
                break;
    }
    if (!gdal_input_flag)
      oparam_fs << "-gdal_input 0" << endl;
    oparam_fs << "-input_image " << input_image_file << endl;
    oparam_fs << "-ncols " << ncols << endl;
    oparam_fs << "-nrows " << nrows << endl;
#ifdef THREEDIM
    oparam_fs << "-nslices " << nslices << endl;
#endif
    oparam_fs << "-nbands " << nbands << endl;
    switch (dtype)
    {
      case UInt8:   oparam_fs << "-dtype UInt8" << endl;
                    break;
      case UInt16:  oparam_fs << "-dtype UInt16" << endl;
                    break;
      case Float32: oparam_fs << "-dtype Float32" << endl;
                    break;
      default:      oparam_fs << "-dtype Unknown" << endl;
                    break;
    }
    if (mask_flag)
    {
      oparam_fs << "-mask " << mask_file << endl;
      oparam_fs << "-mask_value " << mask_value << endl;
    }
    oparam_fs << "-spclust_wght " << spclust_wght << endl;
    oparam_fs << "-dissim_crit " << dissim_crit << endl;
    oparam_fs << "-log " << log_file << endl;
    oparam_fs << "-input_scale ";
    for (index = 0; index < (nbands-1); index++)
      oparam_fs << scale[index] << ", ";
    oparam_fs << scale[nbands-1] << endl;
    oparam_fs << "-input_offset ";
    for (index = 0; index < (nbands-1); index++)
      oparam_fs << offset[index] << ", ";
    oparam_fs << offset[nbands-1] << endl;
    oparam_fs << "-norm_scale ";
    for (index = 0; index < (nbands-1); index++)
      oparam_fs << oparams.scale[index] << ", ";
    oparam_fs << oparams.scale[nbands-1] << endl;
    oparam_fs << "-norm_offset ";
    for (index = 0; index < (nbands-1); index++)
      oparam_fs << oparams.offset[index] << ", ";
    oparam_fs << oparams.offset[nbands-1] << endl;
    oparam_fs << "-class_labels_map " << class_labels_map_file << endl;
    if (boundary_map_flag)
      oparam_fs << "-boundary_map " << boundary_map_file << endl;
    oparam_fs << "-region_classes " << region_classes_file << endl;
    if (object_labels_map_flag)
      oparam_fs << "-object_labels_map " << object_labels_map_file << endl;
    if (region_objects_flag)
      oparam_fs << "-region_objects " << region_objects_file << endl;
    if (std_dev_image_flag)
      oparam_fs << "-std_dev_wght " << std_dev_wght << endl;
    if (edge_image_flag)
    {
      oparam_fs << "-edge_threshold " << edge_threshold << endl;
      oparam_fs << "-edge_power " << edge_power << endl;
      oparam_fs << "-edge_wght " << edge_wght << endl;
    }
    if (program_mode == 3)
      oparam_fs << "-seam_edge_threshold " << seam_edge_threshold << endl;
    if (object_conn_type1_flag)
      oparam_fs << "-object_conn_type1 1" << endl;
    if (region_sum_flag)
    {
      oparam_fs << "-region_sum 1" << endl;
      if (region_sumsq_flag)
        oparam_fs << "-region_sumsq 1" << endl;
      if (region_sumxlogx_flag)
        oparam_fs << "-region_sumxlogx 1" << endl;
    }
    else
      oparam_fs << "-region_sum 0" << endl;
    if (region_std_dev_flag)
      oparam_fs << "-region_std_dev 1" << endl;
    if (region_boundary_npix_flag)
      oparam_fs << "-region_boundary_npix 1" << endl;
    if (region_threshold_flag)
      oparam_fs << "-region_threshold 1" << endl;
    if (region_nghbrs_list_flag)
      oparam_fs << "-region_nghbrs_list 1" << endl;
    if (region_nb_objects_flag)
      oparam_fs << "-region_nb_objects 1" << endl;
    if (region_objects_list_flag)
      oparam_fs << "-region_objects_list 1" << endl;
    oparam_fs << "-maxnbdir " << maxnbdir << endl;
    oparam_fs << "-normind " << normind << endl;
    oparam_fs << "-nb_levels " << oparams.nb_levels << endl;
    oparam_fs << "-level0_nb_classes " << oparams.level0_nb_classes << endl;
    if (region_nb_objects_flag)
      oparam_fs << "-level0_nb_objects " << oparams.level0_nb_objects << endl;
    oparam_fs << "-int_buffer_size ";
    for (index = 0; index < (oparams.nb_levels-1); index++)
      oparam_fs << oparams.int_buffer_size[index] << ", ";
    oparam_fs << oparams.int_buffer_size[oparams.nb_levels-1] << endl;
    if (gdissim_flag)
    {
      oparam_fs << "-gdissim ";
      for (index = 0; index < (oparams.nb_levels-1); index++)
        oparam_fs << oparams.gdissim[index] << ", ";
      oparam_fs << oparams.gdissim[oparams.nb_levels-1] << endl;
    }
    oparam_fs << "-max_threshold ";
    for (index = 0; index < (oparams.nb_levels-1); index++)
      oparam_fs << oparams.max_threshold[index] << ", ";
    oparam_fs << oparams.max_threshold[oparams.nb_levels-1] << endl;

    oparam_fs.close( );

    return;
  }
#else // RHSEG_RUN
  void Params::write_oparam()
  {
    bool write_flag;
    int index;
    fstream oparam_fs;

    oparam_fs.open(oparam_file.c_str( ),ios_base::out);
    switch(program_mode)
    {
       case 1:  oparam_fs << "-program_mode HSWO" << endl;
                break;
       case 2:  oparam_fs << "-program_mode HSEG" << endl;
                break;
       case 3:  oparam_fs << "-program_mode RHSEG" << endl;
                break;
       default: oparam_fs << "-program_mode Unknown" << endl;
                break;
    }
    oparam_fs << "-input_image " << input_image_file << endl;
    oparam_fs << "-ncols " << ncols << endl;
    oparam_fs << "-nrows " << nrows << endl;
#ifdef THREEDIM
    oparam_fs << "-nslices " << nslices << endl;
#endif
    oparam_fs << "-nbands " << nbands << endl;
    switch (dtype)
    {
      case UInt8:   oparam_fs << "-dtype UInt8" << endl;
                    break;
      case UInt16:  oparam_fs << "-dtype UInt16" << endl;
                    break;
      case Float32: oparam_fs << "-dtype Float32" << endl;
                    break;
      default:      oparam_fs << "-dtype Unknown" << endl;
                    break;
    }
    if (mask_flag)
    {
      oparam_fs << "-mask " << mask_file << endl;
      oparam_fs << "-mask_value " << mask_value << endl;
    }
    if (std_dev_image_flag)
      oparam_fs << "-std_dev_image " << std_dev_image_file << endl;
    if (std_dev_mask_flag)
      oparam_fs << "-std_dev_mask " << std_dev_mask_file << endl;
    if (edge_image_flag)
      oparam_fs << "-edge_image " << edge_image_file << endl;
    if (edge_mask_flag)
      oparam_fs << "-edge_mask " << edge_mask_file << endl;
    if (region_map_in_flag)
      oparam_fs << "-region_map_in " << region_map_in_file << endl;
    oparam_fs << "-spclust_wght " << spclust_wght << endl;
    oparam_fs << "-log " << log_file << endl;
    write_flag = false;
    for (index = 0; index < nbands; index++)
      if (scale[index] != RHSEG_SCALE)
        write_flag = true;
    if (write_flag)
    {
      oparam_fs << "-scale ";
      for (index = 0; index < (nbands-1); index++)
        oparam_fs << scale[index] << ", ";
      oparam_fs << scale[index] << endl;
    }
    write_flag = false;
    for (index = 0; index < nbands; index++)
      if (offset[index] != RHSEG_OFFSET)
        write_flag = true;
    if (write_flag)
    { 
      oparam_fs << "-offset ";
      for (index = 0; index < (nbands-1); index++)
        oparam_fs << offset[index] << ", ";
      oparam_fs << offset[index] << endl;
    }
    oparam_fs << "-class_labels_map " << class_labels_map_file << endl;
    if (boundary_map_flag)
      oparam_fs << "-boundary_map " << boundary_map_file << endl;
    oparam_fs << "-region_classes " << region_classes_file << endl;
    if (object_labels_map_flag)
      oparam_fs << "-object_labels_map " << object_labels_map_file << endl;
    if (region_objects_flag)
      oparam_fs << "-region_objects " << region_objects_file << endl;
    if (object_conn_type1_flag)
      oparam_fs << "-object_conn_type1 1" << endl;
    if (region_sum_flag)
    {
      oparam_fs << "-region_sum 1" << endl;
      if (region_sumsq_flag)
        oparam_fs << "-region_sumsq 1" << endl;
      if (region_sumxlogx_flag)
        oparam_fs << "-region_sumxlogx 1" << endl;
    }
    else
      oparam_fs << "-region_sum 0" << endl;
    if (region_std_dev_flag)
      oparam_fs << "-region_std_dev 1" << endl;
    if (region_boundary_npix_flag)
      oparam_fs << "-region_boundary_npix 1" << endl;
    if (region_threshold_flag)
      oparam_fs << "-region_threshold 1" << endl;
    if (region_nb_objects_flag != ((spclust_wght_flag) && (object_labels_map_flag) && (region_objects_flag)))
    {
      if (region_nb_objects_flag)
        oparam_fs << "-region_nb_objects 1" << endl;
      else
        oparam_fs << "-region_nb_objects 0" << endl;
    }
    if (region_objects_list_flag != ((spclust_wght_flag) && (object_labels_map_flag) && (region_objects_flag)))
    {
      if (region_objects_list_flag)
        oparam_fs << "-region_objects_list 1" << endl;
      else
        oparam_fs << "-region_objects_list 0" << endl;
    }
    if (dissim_crit != DISSIM_CRIT)
      oparam_fs << "-dissim_crit " << dissim_crit << endl;
    if (std_dev_image_flag)
      oparam_fs << "-std_dev_wght " << std_dev_wght << endl;
    if (edge_image_flag)
    {
      oparam_fs << "-edge_threshold " << edge_threshold << endl;
      oparam_fs << "-edge_power " << edge_power << endl;
      oparam_fs << "-edge_wght " << edge_wght << endl;
    }
    if (program_mode == 3)
      oparam_fs << "-seam_edge_threshold " << seam_edge_threshold << endl;
    switch (nb_dimensions)
    {
      case 1:  if (conn_type != 1)
                 oparam_fs << "-conn_type " << conn_type << endl;
               break;
      case 2:  if (conn_type != 2)
                 oparam_fs << "-conn_type " << conn_type << endl;
               break;
      case 3:  if (conn_type != 3)
                 oparam_fs << "-conn_type " << conn_type << endl;
               break;
      default: cout << "ERROR: Invalid value for nb_dimensions (=" << nb_dimensions << ")" << endl;
               return;
    }
    if (chk_nregions_flag)
      oparam_fs << "-chk_nregions " << chk_nregions << endl;
    if (hseg_out_nregions_flag)
    {
      oparam_fs << "-hseg_out_nregions ";
      for (index = 0; index < (nb_hseg_out_nregions-1); index++)
        oparam_fs << hseg_out_nregions[index] << ", ";
      oparam_fs << hseg_out_nregions[index] << endl;
    }
    if (hseg_out_thresholds_flag)
    {
      oparam_fs << "-hseg_out_thresholds ";
      for (index = 0; index < (nb_hseg_out_thresholds-1); index++)
        oparam_fs << hseg_out_thresholds[index] << ", ";
      oparam_fs << hseg_out_thresholds[index] << endl;
    }
    if (conv_nregions != CONV_NREGIONS)
      oparam_fs << "-conv_nregions " << conv_nregions << endl;
    if (gdissim_flag)
      oparam_fs << "-gdissim 1" << endl;
    
    if (debug != DEBUG_DEFAULT_VALUE)
      oparam_fs << "-debug " << debug << endl;
    if (normind != NORMIND)
      oparam_fs << "-normind " << normind << endl;
    if (init_threshold != INIT_THRESHOLD)
      oparam_fs << "-init_threshold " << init_threshold << endl;
    if (initial_merge_flag)
      oparam_fs << "-initial_merge_npix" << initial_merge_npix << endl;
    if (random_init_seed_flag)
      oparam_fs << "-random_init_seed 1" << endl;
    if (!sort_flag)
      oparam_fs << "-sort 0" << endl;
    if (edge_dissim_option != EDGE_DISSIM_OPTION)
      oparam_fs << "-edge_dissim_option " << edge_dissim_option << endl;
    if (program_mode == 3)
    {
      oparam_fs << "-rnb_levels " << rnb_levels << endl;
#ifndef PARALLEL
      if (ionb_levels_flag)
        oparam_fs << "-ionb_levels " << ionb_levels << endl;
#ifdef RHSEG_SETUP
      int stride, recur_level = 0;
      set_stride_sections(recur_level,stride,nb_sections);
      oparam_fs << "-nb_sections " << nb_sections << endl;
#endif

#endif
    }
    if (min_nregions_flag)
      oparam_fs << "-min_nregions " << min_nregions << endl;
    if (program_mode != 1)
    {
      oparam_fs << "-spclust_min " << spclust_min << endl;
      oparam_fs << "-spclust_max " << spclust_max << endl;
    }
    if (merge_accel_flag)
      oparam_fs << "-merge_acceleration 1" << endl;

    oparam_fs.close( );

    return;
  }
#endif

 // Print parameters
  void Params::print()
  {
    int band;

    if (debug > 0)
    {
  // Print version
      log_fs << "This is " << version << endl << endl;

    switch(program_mode)
    {
       case 1:  log_fs << "Program Mode: Hierarchical Step-Wise Optimization (HSWO)" << endl;
                break;
       case 2:  log_fs << "Program Mode: Hierarchical Segmentation (HSEG)" << endl;
                break;
       case 3:  log_fs << "Program Mode: Recursive Hierarchical Segmentation (RHSEG)" << endl;
                break;
       default: log_fs << "(Unknown Program Mode)" << endl;
                break;
    }

  // Print input parameters
      log_fs << endl << "Required input parameters that specify and describe the input data:" << endl;
      log_fs << "Input image file name: " << input_image_file << endl;
      log_fs << "Input image number of columns = " <<  ncols << endl;
      log_fs << "Input image number of rows = " <<  nrows << endl;
#ifdef THREEDIM
      log_fs << "Input image number of slices = " <<  nslices << endl;
#endif
      log_fs << "Input image number of bands = " <<  nbands << endl;
      switch (dtype)
      {
        case UInt8:   log_fs << "Input image data type is UNSIGNED 8-bit (UInt8)" << endl;
                      break;
        case UInt16:  log_fs << "Input image data type is UNSIGNED 16-bit (UInt16)" << endl;
                      break;
        case Float32: log_fs << "Input image data type is 32-bit FLOAT (Float32)" << endl;
                      break;
        default:      log_fs << "Input image data type in invalid (Unknown)" << endl;
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
      if (std_dev_image_flag)
      {
        log_fs << "std_dev_image_file = " << std_dev_image_file << endl;
      }
      if (std_dev_mask_flag)
      {
        log_fs << "std_dev_mask_file = " << std_dev_mask_file << endl;
      }
      if (edge_image_flag)
      {
        log_fs << "edge_image_file = " << edge_image_file << endl;
      }
      if (edge_mask_flag)
      {
        log_fs << "edge_mask_file = " << edge_mask_file << endl;
      }
      if (region_map_in_flag)
      {
        log_fs << "region_map_in_file = " << region_map_in_file << endl;
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
       log_fs <<  "Minimum number of regions with spectral clustering = " << spclust_min << endl;
       log_fs <<  "Maximum number of regions with spectral clustering = " << spclust_max << endl;
      }
      else
        log_fs <<  "No." << endl;
      log_fs <<  "Dissimilarity Criterion:  ";
      switch(dissim_crit)
      {
       case 1:  log_fs <<  "1-Norm" << endl;
  	        break;
       case 2:  log_fs <<  "2-Norm" << endl;
  	        break;
       case 3:  log_fs <<  "Infinity Norm" << endl;
  	        break;
       case 4:  log_fs <<  "Spectral Angle Mapper" << endl;
  	        break;
       case 5:  log_fs <<  "Spectral Information Divergence" << endl;
  	        break;
#ifdef MSE_SQRT
       case 6:  log_fs <<  "Square Root of Band Sum Mean Squared Error" << endl;
  	        break;
       case 7:  log_fs <<  "Square Root of Band Maximum Mean Squared Error" << endl;
  	        break;
#else
       case 6:  log_fs <<  "Band Sum Mean Squared Error" << endl;
  	        break;
       case 7:  log_fs <<  "Band Maximum Mean Squared Error" << endl;
  	        break;
#endif
       case 8:  log_fs <<  "Normalized Vector Distance" << endl;
  	        break;
       case 9:  log_fs <<  "Entropy" << endl;
  	        break;
       case 10: log_fs <<  "SAR Speckle Noise Criterion" << endl;
  	        break;
//       case 11: log_fs <<  "Feature Range" << endl;
//  	        break;
       default: log_fs <<  "WARNING: Invalid value for dissim_crit parameter" << endl;
  	        break;
      }
      if (std_dev_image_flag)
        log_fs << "Weight for standard deviation spatial feature = " << std_dev_wght << endl;
      if (edge_image_flag)
      {
        log_fs << "Edge threshold = " << edge_threshold << endl;
        log_fs << "Edge power = " << edge_power << endl;
        log_fs << "Edge weight = " << edge_wght << endl;
      }
      if (program_mode == 3)
        log_fs << "Seam edge threshold = " << seam_edge_threshold << endl;
      log_fs << "Number of Nearest Neighbors = " <<  maxnbdir << endl;
      if (chk_nregions_flag)
      {
        log_fs << "Number of Regions at which Hiearchical Segmentation output is ";
        log_fs << "initiated in final stage = " <<  chk_nregions << endl;
      }
      if (hseg_out_thresholds_flag)
      {
        log_fs << "Set of merge thresholds at which hierarchical segmentation outputs are made: " << endl;
        for (band = 0; band < nb_hseg_out_thresholds; band++)
          log_fs << hseg_out_thresholds[band] << " ";
        log_fs << endl;
      }
      if (hseg_out_nregions_flag)
      {
        log_fs << "Set of nregion levels at which hierarchical segmentation outputs are made: " << endl;
        for (band = 0; band < nb_hseg_out_nregions; band++)
          log_fs << hseg_out_nregions[band] << " ";
        log_fs << endl;
      }

      log_fs << endl << "Parameters with recommended default values:" << endl;
      if (program_mode == 3)
      {
        if (min_nregions_flag)
          log_fs << "Number of regions for convergence at intermediate stages = " << min_nregions << endl;
        log_fs << "Number of Recursive Stage Levels  = " << rnb_levels << endl;
#ifdef PARALLEL
        log_fs << "Number of Intermediate Stage Levels  = " << inb_levels << endl;
        log_fs << "Number of Final Stage Levels  = " << onb_levels << endl;
#else
        if (ionb_levels > 1)
          log_fs << "Recursive Level at which Data I/O is Performed  = " << ionb_levels << endl;
#endif
      }
      log_fs << "Normalize Input Data?  ";
      switch(normind)
      {
        case 1:    log_fs <<  "No." << endl << "Do not normalize input data." << endl;
                   break;
        case 2:    log_fs <<  "Yes." << endl << "Normalize input data across all bands." << endl;
  	           break;
        case 3:    log_fs <<  "Yes." << endl << "Normalize each band of input data separately." << endl;
  	           break;
        default:   log_fs <<  "WARNING: INVALID value for normind parameter." << endl;
                   break;
      }
      if (init_threshold > 0.0)
        log_fs << "Threshold for initial fast region merging = " << init_threshold << endl;
      if (initial_merge_flag)
      {
        log_fs << "Suppression of neighbor merges requested for regions larger than " << initial_merge_npix << " pixels after" << endl;
        log_fs << "initial fast region merging" << endl;
      }
      if (random_init_seed_flag)
        log_fs << "Random initialization seed requested for the sampling procedure utilized in the initial fast region merging process" << endl;
      else
        log_fs << "Fixed initialization seed requested for the sampling procedure utilized in the initial fast region merging process" << endl;
      if (sort_flag)
        log_fs << "Region classes and objects are sorted" << endl;
      else
        log_fs << "Region classes and objects are not sorted" << endl;
      log_fs << "Edge dissimilarity option:  ";
      switch(edge_dissim_option)
      {
        case 1:    log_fs <<  "Merge Enhancement Option (edge_dissim_option)" << endl;
                   break;
        case 2:    log_fs <<  "Merge Suppression Option (edge_dissim_option)" << endl;
  	           break;
        default:   log_fs <<  "WARNING: INVALID value for edge_dissim_option parameter." << endl;
                   break;
      }
      log_fs << "Number of regions for final convergence = " <<  conv_nregions << endl;
      if (gdissim_flag)
        log_fs << "Global dissimilarity values calculated for each hierarchical level output" << endl;
      if (merge_accel_flag)
        log_fs << "Small region merge acceleration is utilized" << endl;
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
      log_fs << "Output log file name: " << log_file << endl;
      if (region_sum_flag)
      {
        if (region_sumsq_flag)
          log_fs << "Region sumsq information included." << endl;
        if (region_sumxlogx_flag)
          log_fs << "Region sumxlogx information included." << endl;
      }

#ifdef RHSEG_RUN
      log_fs << endl << "Calculated Parameters: " << endl;
      log_fs << "Number of dimensions = " << nb_dimensions << endl;
      if (rnb_levels > 1)
        log_fs << "Size of seam between processing windows = " << seam_size << endl;
#ifdef PARALLEL
      log_fs << "Lowest level of recursion at which this task is active = " << recur_level << endl;
      log_fs << "Rank of the current parallel process = " << myid << endl;
      log_fs << "Number of parallel processes = " << numprocs << endl;
      log_fs << "Number of subtasks called by this task = " << nb_tasks << endl;
      log_fs << "Final stages section size is: " << onb_ncols << " x " << onb_nrows;
#ifdef THREEDIM
      log_fs << " x " << onb_nslices << endl;
#else
      log_fs << endl;
#endif
      log_fs << "Intermediate stages section size is: " << inb_ncols << " x " << inb_nrows;
#ifdef THREEDIM
      log_fs << " x " << inb_nslices << endl;
#else
      log_fs << endl;
#endif
#else
      if (ionb_levels > 1)
      {
        log_fs << "Data I/O section size is: " << ionb_ncols << " x " << ionb_nrows;
#ifdef THREEDIM
        log_fs << " x " << ionb_nslices << endl;
#else
        log_fs << endl;
#endif
        log_fs << "Number of I/O processing sections = " << nb_sections << endl;
      }
#endif
      log_fs << "At deepest level of recursion, section size is: " << rnb_ncols << " x " << rnb_nrows;
#ifdef THREEDIM
      log_fs << " x " << rnb_nslices << endl;
#else
      log_fs << endl;
#endif
      log_fs << "For pixel data, spatial dimensions are: " << pixel_ncols << " x " << pixel_nrows;
#ifdef THREEDIM
      log_fs << " x " << pixel_nslices << endl;
#else
      log_fs << endl;
#endif
      log_fs << "Number of columns after padding (if any) = " << padded_ncols << endl;
      log_fs << "Number of rows after padding (if any) = " << padded_nrows << endl;
#ifdef THREEDIM
      log_fs << "Number of slices after padding (if any) = " << padded_nslices << endl;
#endif
#endif // RHSEG_RUN
    }
#ifdef PARALLEL
    else if (myid == 0)
    {
  // Print version
      cout << "This is " << version << endl << endl;

    switch(program_mode)
    {
       case 1:  cout << "Program Mode: Hierarchical Step-Wise Optimization (HSWO)" << endl;
                break;
       case 2:  cout << "Program Mode: Hierarchical Segmentation (HSEG)" << endl;
                break;
       case 3:  cout << "Program Mode: Recursive Hierarchical Segmentation (RHSEG)" << endl;
                break;
       default: cout << "(Unknown Program Mode)" << endl;
                break;
    }

  // Print copyright notice
#if (defined(WINDOWS) || defined(CYGWIN))
      cout << "Copyright (C) 2006 United States Government as represented by the" << endl;
#else
      cout << "Copyright \u00a9 2006 United States Government as represented by the" << endl;
#endif
      cout << "Administrator of the National Aeronautics and Space Administration." << endl;
      cout << "No copyright is claimed in the United States under Title 17, U.S. Code." << endl;
      cout << "All Other Rights Reserved." << endl << endl;

  // Print input parameters
      cout << endl << "Required input parameters that specify and describe the input data:" << endl;
      cout << "Input image file name: " << input_image_file << endl;
      cout << "Input image number of columns = " <<  ncols << endl;
      cout << "Input image number of rows = " <<  nrows << endl;
#ifdef THREEDIM
      cout << "Input image number of slices = " <<  nslices << endl;
#endif
      cout << "Input image number of bands = " <<  nbands << endl;
      switch (dtype)
      {
        case UInt8:   cout << "Input image data type is UNSIGNED 8-bit (UInt8)" << endl;
                      break;
        case UInt16:  cout << "Input image data type is UNSIGNED 16-bit (UInt16)" << endl;
                      break;
        case Float32: cout << "Input image data type is 32-bit FLOAT (Float32)" << endl;
                      break;
        default:      cout << "WARNING: Input image data type in invalid (Unknown)" << endl;
                      break;
      }

      cout << endl << "Optional input parameters that specify optional input files and parameter values:" << endl;
      if (nbands < 10)
      {
        cout << "Input data scale factors: ";
        for (band = 0; band < nbands; band++)
          cout << scale[band] << " ";
        cout << endl;
        cout << "Input data offset factors: ";
        for (band = 0; band < nbands; band++)
          cout << offset[band] << " ";
        cout << endl;
      }
      if (mask_flag)
      {
        cout << "Input bad data mask file name: " << mask_file << endl;
        cout << "Bad data mask flag value = " << mask_value << endl;
      }
      if (std_dev_image_flag)
      {
        cout << "std_dev_image_file = " << std_dev_image_file << endl;
      }
      if (std_dev_mask_flag)
      {
        cout << "std_dev_mask_file = " << std_dev_mask_file << endl;
      }
      if (edge_image_flag)
      {
        cout << "edge_image_file = " << edge_image_file << endl;
      }
      if (edge_mask_flag)
      {
        cout << "edge_mask_file = " << edge_mask_file << endl;
      }
      if (region_map_in_flag)
      {
        cout << "region_map_in_file = " << region_map_in_file << endl;
      }

      cout << endl << "Input parameters that specify output files:" << endl;
      cout << "Output parameter file name: " << oparam_file << endl;
      cout << "Output class labels map file name: " << class_labels_map_file << endl;
      if (boundary_map_flag)
        cout << "Optional output hierarchical boundary map file name: " << boundary_map_file << endl;
      cout << "Output region classes file name: " << region_classes_file << endl;
      cout << endl << "Optional parameters that select the contents of the output" << endl;
      cout << "region object files (above):" << endl;
      if (region_sum_flag)
        cout << "Region sum (and, if available, sumsq and sumxlogx) information included." << endl;
      if (region_std_dev_flag)
        cout << "Region standard deviation information included" << endl;
      if (region_boundary_npix_flag)
        cout << "Region region boundary number of pixels information included" << endl;
      if (region_threshold_flag)
        cout << "Region most recent merge threshold information included" << endl;
      if (region_nghbrs_list_flag)
        cout << "List of region clases neighboring each region class included" << endl;
      if (region_nb_objects_flag)
        cout << "Number of region objects contained in each region class included" << endl;
      if (region_objects_list_flag)
        cout << "List of region objects contained in each region class included" << endl;

      cout << endl << "Defaultable required input parameters, along with other associated parameters:" << endl;
      cout <<  "Utilize Spectral Clustering?  ";
      if (spclust_wght_flag)
      {
       cout <<  "Yes, with " << spclust_wght << " weighting factor (relative to region growing)." << endl;
       cout <<  "Minimum number of regions with spectral clustering = " << spclust_min << endl;
       cout <<  "Maximum number of regions with spectral clustering = " << spclust_max << endl;
      }
      else
        cout <<  "No." << endl;
      cout <<  "Dissimilarity Criterion:  ";
      switch(dissim_crit)
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
      if (std_dev_image_flag)
        cout << "Weight for standard deviation spatial feature = " << std_dev_wght << endl;
      if (edge_image_flag)
      {
        cout << "Edge threshold = " << edge_threshold << endl;
        cout << "Edge power = " << edge_power << endl;
        cout << "Edge weight = " << edge_wght << endl;
      }
      if (program_mode == 3)
        log_fs << "Seam edge threshold = " << seam_edge_threshold << endl;
      cout << "Number of Nearest Neighbors = " <<  maxnbdir << endl;
      if (chk_nregions_flag)
      {
        cout << "Number of Regions at which Hiearchical Segmentation output is ";
        cout << "initiated in final stage = " <<  chk_nregions << endl;
      }
      if (hseg_out_thresholds_flag)
      {
        cout << "Set of merge thresholds at which hierarchical segmentation outputs are made: " << endl;
        for (band = 0; band < nb_hseg_out_thresholds; band++)
          cout << hseg_out_thresholds[band] << " ";
        cout << endl;
      }
      if (hseg_out_nregions_flag)
      {
        cout << "Set of nregion levels at which hierarchical segmentation outputs are made: " << endl;
        for (band = 0; band < nb_hseg_out_nregions; band++)
          cout << hseg_out_nregions[band] << " ";
        cout << endl;
      }

      cout << endl << "Parameters with recommended default values:" << endl;
      if (program_mode == 3)
      {
        if (min_nregions_flag)
          cout << "Number of regions for convergence at intermediate stages = " << min_nregions << endl;
        cout << "Number of Recursive Stage Levels  = " << rnb_levels << endl;
#ifdef PARALLEL
        cout << "Number of Intermediate Stage Levels  = " << inb_levels << endl;
        cout << "Number of Final Stage Levels  = " << onb_levels << endl;
#else
        if (ionb_levels > 1)
          cout << "Recursive Level at which Data I/O is Performed  = " << ionb_levels << endl;
#endif
      }
      cout << "Normalize Input Data?  ";
      switch(normind)
      {
        case 1:  cout <<  "No." << endl << "Do not normalize input data." << endl;
                 break;
        case 2:  cout <<  "Yes." << endl << "Normalize input data across all bands." << endl;
   	         break;
        case 3:  cout <<  "Yes." << endl << "Normalize each band of input data separately." << endl;
  	         break;
        default: cout <<  "WARNING: Invalid value for normind parameter" << endl;
                 break;
      }
      if (init_threshold > 0.0)
        cout << "Threshold for initial fast region merging = " << init_threshold << endl;
      if (initial_merge_flag)
      {
        cout << "Suppression of neighbor merges requested for regions larger than " << initial_merge_npix << " pixels after" << endl;
        cout << "initial fast region merging" << endl;
      }
      if (random_init_seed_flag)
        cout << "Random initialization seed requested for the sampling procedure utilized in the initial fast region merging process" << endl;
      else
        cout << "Fixed initialization seed requested for the sampling procedure utilized in the initial fast region merging process" << endl;
      if (sort_flag)
        cout << "Region classes and objects are sorted" << endl;
      else
        cout << "Region classes and objects are not sorted" << endl;
      cout << "Edge dissimilarity option:  ";
      switch(edge_dissim_option)
      {
        case 1:    cout <<  "Merge Enhancement Option (edge_dissim_option)" << endl;
                   break;
        case 2:    cout <<  "Merge Suppression Option (edge_dissim_option)" << endl;
  	           break;
        default:   cout <<  "WARNING: INVALID value for edge_dissim_option parameter." << endl;
                   break;
      }
      cout << "Number of regions for final convergence = " <<  conv_nregions << endl;
      if (gdissim_flag)
        cout << "Global dissimilarity values calculated for each hierarchical level output" << endl;
      if (merge_accel_flag)
        cout << "Small region merge acceleration is utilized" << endl;
      if (region_objects_list_flag)
      {
        cout << endl << "Optional parameters:" << endl;
        if (object_labels_map_flag)
          cout << "Output object labels map file name: " << object_labels_map_file << endl;
        if (region_objects_flag)
          cout << "Output region objects file name: " << region_objects_file << endl;
        if (object_conn_type1_flag)
          cout << "Minimum number of Nearest Neighbors for Region Objects" << endl;
      }
      cout << endl << "Debug option = " <<  debug << endl;
      cout << "Output log file name: " << log_file << endl;
      if (region_sum_flag)
      {
        if (region_sumsq_flag)
          cout << "Region sumsq information included." << endl;
        if (region_sumxlogx_flag)
          cout << "Region sumxlogx information included." << endl;
      }

#ifdef RHSEG_RUN
      cout << endl << "Calculated Parameters: " << endl;
      cout << "Number of dimensions = " << nb_dimensions << endl;
      if (rnb_levels > 1)
        cout << "Size of seam between processing windows = " << seam_size << endl;
      cout << "Lowest level of recursion at which this task is active = " << recur_level << endl;
      cout << "Rank of the current parallel process = " << myid << endl;
      cout << "Number of parallel processes = " << numprocs << endl;
      cout << "Number of subtasks called by this task = " << nb_tasks << endl;
      cout << "Final stages section size is: " << onb_ncols << " x " << onb_nrows;
#ifdef THREEDIM
      cout << " x " << onb_nslices << endl;
#else
      cout << endl;
#endif
      cout << "Intermediate stages section size is: " << inb_ncols << " x " << inb_nrows;
#ifdef THREEDIM
      cout << " x " << inb_nslices << endl;
#else
      cout << endl;
#endif
      cout << "At deepest level of recursion, section size is: " << rnb_ncols << " x " << rnb_nrows;
#ifdef THREEDIM
      cout << " x " << rnb_nslices << endl;
#else
      cout << endl;
#endif
      cout << "For pixel data, spatial dimensions are: " << pixel_ncols << " x " << pixel_nrows;
#ifdef THREEDIM
      cout << " x " << pixel_nslices << endl;
#else
      cout << endl;
#endif
      cout << "Number of columns after padding (if any) = " << padded_ncols << endl;
      cout << "Number of rows after padding (if any) = " << padded_nrows << endl;
#ifdef THREEDIM
      cout << "Number of slices after padding (if any) = " << padded_nslices << endl;
#endif
       }
#endif // RHSEG_RUN
#endif // PARALLEL
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
