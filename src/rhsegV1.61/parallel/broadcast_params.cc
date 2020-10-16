/*-----------------------------------------------------------
|
|  Routine Name: broadcast_params - Broadcast parameter information
|
|       Purpose: Broadcast parameters in the param_info structure
|                to all processes
|
|         Input: params    (Program parameters via Params structure)
|
|       Returns: TRUE (1) on success, FALSE (0) on failure
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: September 16, 2002.
| Modifications: November 5, 2003 - Accomodated new style of Params structure.
|                August 8, 2005 - Added conv_criterion parameter
|                January 5, 2006 - Added slice dimension (extension to three-dimensional analysis)
|                November 1, 2007 - Removed conv_criterion and conv_factor parameters
|                November 1, 2007 - Removed gdissim_crit and gstd_dev_crit parameters and corresponding flag parameters
|                May 16, 2008 - Revised to work with globally defined Params and oParams class objects
|                May 12, 2009 - Revised for revised Params class structure
|		 January 10, 2010: Changed the definition of min_npixels parameter
|		 January 19, 2010: Changed the definition of spclust_start parameter and renamed it spclust_max
|                January 19, 2010: Eliminated max_nregions parameter
|                August 26, 2010: Added broadcast of first element of hseg_out_nregions and hseg_out_thresholds vectors
| 	         December 28, 2010: Added spclust_min parameter
|                July 30, 2013: Added broadcast of std_dev_image_flag, std_dev_mask_flag, edge_image_flag, edge_mask_flag, 
|                               edge_threshold and edge_wght (and dropped broadcast of std_dev_wght_flag)
|                September 16, 2013: Added broadcast of edge_power, seam_edge_threshold and edge_dissim_option.
|                September 16, 2013: Removed broadcast of split_pixels_factor, seam_threshold_factor and region_threshold_factor.
|                February 28, 2014: Added broadcast of initial_merge_flag, random_init_seed_flag and sort_flag.
|                March 19, 2014: Added broadcast of initial_merge_npix.
|
------------------------------------------------------------*/
#include <defines.h>
#include <params/params.h>
#include <string>
#include <cstring>
#include <iostream>

extern HSEGTilton::Params params;

namespace HSEGTilton
{
 int broadcast_params()
 {
  char *temp_file = NULL;
  int length;
  float float_var;
  if (params.myid == 0)
  {
    length = strlen(params.log_file.c_str( )) + 1;
    temp_file = (char *) malloc((unsigned)(length)*sizeof(char));
    sprintf(temp_file,"%s",params.log_file.c_str( ));
  }
  MPI::COMM_WORLD.Bcast(&length, 1, MPI::INT, 0);
  if (params.myid != 0)
    temp_file = (char *) malloc((unsigned)(length)*sizeof(char));
  MPI::COMM_WORLD.Bcast(temp_file, length, MPI::CHAR, 0);
  if (params.myid != 0)
  {
    string home_dir;
#ifdef THUNDERHEAD
    home_dir = "/home/scratch/tilton";
#else
    home_dir = getenv("HOME");
#endif
    char *myid_char;
    if (params.myid < 10)
    {
      myid_char = (char *) malloc((unsigned)(2)*sizeof(char));
      sprintf(myid_char,"%d",params.myid);
      params.log_file = home_dir + "/" + temp_file + ".task00" + myid_char;
    }
    else if (params.myid < 100)
    {
      myid_char = (char *) malloc((unsigned)(3)*sizeof(char));
      sprintf(myid_char,"%d",params.myid);
      params.log_file = home_dir + "/" + temp_file + ".task0" + myid_char;
    }
    else
    {
      myid_char = (char *) malloc((unsigned)(4)*sizeof(char));
      sprintf(myid_char,"%d",params.myid);
      params.log_file = home_dir + "/" + temp_file + ".task" + myid_char;
    }
  }
  MPI::COMM_WORLD.Bcast(&params.program_mode, 1, MPI::INT, 0);
  MPI::COMM_WORLD.Bcast(&params.ncols, 1, MPI::INT, 0);
  MPI::COMM_WORLD.Bcast(&params.nrows, 1, MPI::INT, 0);
#ifdef THREEDIM
  MPI::COMM_WORLD.Bcast(&params.nslices, 1, MPI::INT, 0);
#endif
  MPI::COMM_WORLD.Bcast(&params.nb_dimensions, 1, MPI::INT, 0);
  MPI::COMM_WORLD.Bcast(&params.nbands, 1, MPI::INT, 0);
  int dtype;
  switch (params.dtype)
  {
    case UInt8:   dtype = 8;
                  break;
    case UInt16:  dtype = 16;
                  break;
    case Float32: dtype = 32;
                  break;
    default:      dtype = 0;
                  break;
  }
  MPI::COMM_WORLD.Bcast(&dtype, 1, MPI::INT, 0);
  switch (dtype)
  {
    case  8: params.dtype = UInt8;
#ifdef GDAL
             params.data_type = GDT_Byte;
#endif
             break;
    case 16: params.dtype = UInt16;
#ifdef GDAL
             params.data_type = GDT_UInt16;
#endif
             break;
    case 32: params.dtype = Float32;
#ifdef GDAL
             params.data_type = GDT_Float32;
#endif
             break;
    default: params.dtype = Unknown;
#ifdef GDAL
             params.data_type = GDT_Unknown;
#endif
             break;
  }
  double *scale;
  scale = new double[params.nbands];
  if (params.myid == 0)
  {
    for (length = 0; length < params.nbands; length++)
      scale[length] = params.scale[length];
  }
  MPI::COMM_WORLD.Bcast(scale, params.nbands, MPI::DOUBLE, 0);
  if (params.myid != 0)
  {
    params.scale.resize(params.nbands);
    for (length = 0; length < params.nbands; length++)
      params.scale[length] = scale[length];
  }
  double *offset;
  offset = new double[params.nbands];
  if (params.myid == 0)
  {
    for (length = 0; length < params.nbands; length++)
      offset[length] = params.offset[length];
  }
  MPI::COMM_WORLD.Bcast(offset, params.nbands, MPI::DOUBLE, 0);
  if (params.myid != 0)
  {
    params.offset.resize(params.nbands);
    for (length = 0; length < params.nbands; length++)
      params.offset[length] = offset[length];
  }
  length = (int) params.mask_flag;
  MPI::COMM_WORLD.Bcast(&length, 1, MPI::INT, 0);
  params.mask_flag = (bool) length;
  MPI::COMM_WORLD.Bcast(&params.mask_value, 1, MPI::INT, 0);
  length = (int) params.region_map_in_flag;
  MPI::COMM_WORLD.Bcast(&length, 1, MPI::INT, 0);
  params.region_map_in_flag = (bool) length;
  length = (int) params.boundary_map_flag;
  MPI::COMM_WORLD.Bcast(&length, 1, MPI::INT, 0);
  params.boundary_map_flag = (bool) length;
  length = (int) params.region_sum_flag;
  MPI::COMM_WORLD.Bcast(&length, 1, MPI::INT, 0);
  params.region_sum_flag = (bool) length;
  length = (int) params.region_sumsq_flag;
  MPI::COMM_WORLD.Bcast(&length, 1, MPI::INT, 0);
  params.region_sumsq_flag = (bool) length;
  length = (int) params.region_sumxlogx_flag;
  MPI::COMM_WORLD.Bcast(&length, 1, MPI::INT, 0);
  params.region_sumxlogx_flag = (bool) length;
  length = (int) params.region_boundary_npix_flag;
  MPI::COMM_WORLD.Bcast(&length, 1, MPI::INT, 0);
  params.region_boundary_npix_flag = (bool) length;
  length = (int) params.region_nb_objects_flag;
  MPI::COMM_WORLD.Bcast(&length, 1, MPI::INT, 0);
  params.region_nb_objects_flag = (bool) length;
  length = (int) params.object_labels_map_flag;
  MPI::COMM_WORLD.Bcast(&length, 1, MPI::INT, 0);
  params.object_labels_map_flag = (bool) length;
  length = (int) params.object_conn_type1_flag;
  MPI::COMM_WORLD.Bcast(&length, 1, MPI::INT, 0);
  params.object_conn_type1_flag = (bool) length;
  MPI::COMM_WORLD.Bcast(&params.normind, 1, MPI::INT, 0);
  MPI::COMM_WORLD.Bcast(&params.init_threshold, 1, MPI::FLOAT, 0);
  length = (int) params.initial_merge_flag;
  MPI::COMM_WORLD.Bcast(&length, 1, MPI::INT, 0);
  params.initial_merge_flag = (bool) length;
  if (params.initial_merge_flag)
    MPI::COMM_WORLD.Bcast(&params.initial_merge_npix, 1, MPI::INT, 0);
  length = (int) params.random_init_seed_flag;
  MPI::COMM_WORLD.Bcast(&length, 1, MPI::INT, 0);
  params.random_init_seed_flag = (bool) length;
  length = (int) params.sort_flag;
  MPI::COMM_WORLD.Bcast(&length, 1, MPI::INT, 0);
  params.sort_flag = (bool) length;
  MPI::COMM_WORLD.Bcast(&params.dissim_crit, 1, MPI::INT, 0);
  MPI::COMM_WORLD.Bcast(&params.maxnbdir, 1, MPI::INT, 0);
  length = (int) params.spclust_wght_flag;
  MPI::COMM_WORLD.Bcast(&length, 1, MPI::INT, 0);
  params.spclust_wght_flag = (bool) length;
  if (params.spclust_wght_flag)
    MPI::COMM_WORLD.Bcast(&params.spclust_wght, 1, MPI::FLOAT, 0);
  else
    params.spclust_wght = 0.0;
  MPI::COMM_WORLD.Bcast(&params.spclust_min, 1, MPI::UNSIGNED, 0);
  MPI::COMM_WORLD.Bcast(&params.spclust_max, 1, MPI::UNSIGNED, 0);
  length = (int) params.std_dev_image_flag;
  MPI::COMM_WORLD.Bcast(&length, 1, MPI::INT, 0);
  params.std_dev_image_flag = (bool) length;
  if (params.std_dev_image_flag)
  {
    MPI::COMM_WORLD.Bcast(&params.std_dev_wght, 1, MPI::FLOAT, 0);
    length = (int) params.std_dev_mask_flag;
    MPI::COMM_WORLD.Bcast(&length, 1, MPI::INT, 0);
    params.std_dev_mask_flag = (bool) length;
  }
  length = (int) params.edge_image_flag;
  MPI::COMM_WORLD.Bcast(&length, 1, MPI::INT, 0);
  params.edge_image_flag = (bool) length;
  if (params.edge_image_flag)
  {
    MPI::COMM_WORLD.Bcast(&params.edge_threshold, 1, MPI::FLOAT, 0);
    MPI::COMM_WORLD.Bcast(&params.edge_power, 1, MPI::FLOAT, 0);
    MPI::COMM_WORLD.Bcast(&params.edge_wght, 1, MPI::FLOAT, 0);
    length = (int) params.edge_mask_flag;
    MPI::COMM_WORLD.Bcast(&length, 1, MPI::INT, 0);
    params.edge_mask_flag = (bool) length;
  }
  MPI::COMM_WORLD.Bcast(&params.seam_edge_threshold, 1, MPI::FLOAT, 0);
  MPI::COMM_WORLD.Bcast(&params.edge_dissim_option, 1, MPI::INT, 0);
  length = (int) params.rnb_levels_flag;
  MPI::COMM_WORLD.Bcast(&length, 1, MPI::INT, 0);
  params.rnb_levels_flag = (bool) length;
  if (params.rnb_levels_flag)
    MPI::COMM_WORLD.Bcast(&params.rnb_levels, 1, MPI::INT, 0);
  length = (int) params.min_nregions_flag;
  MPI::COMM_WORLD.Bcast(&length, 1, MPI::INT, 0);
  params.min_nregions_flag = (bool) length;
  if (params.min_nregions_flag)
    MPI::COMM_WORLD.Bcast(&params.min_nregions, 1, MPI::UNSIGNED, 0);
  length = (int) params.merge_accel_flag;
  MPI::COMM_WORLD.Bcast(&length, 1, MPI::INT, 0);
  params.merge_accel_flag = (bool) length;
  length = (int) params.chk_nregions_flag;
  MPI::COMM_WORLD.Bcast(&length, 1, MPI::INT, 0);
  params.chk_nregions_flag = (bool) length;
  if (params.chk_nregions_flag)
    MPI::COMM_WORLD.Bcast(&params.chk_nregions, 1, MPI::UNSIGNED, 0);
  length = (int) params.hseg_out_nregions_flag;
  MPI::COMM_WORLD.Bcast(&length, 1, MPI::INT, 0);
  params.hseg_out_nregions_flag = (bool) length;
  if (params.hseg_out_nregions_flag)
  {
    if (params.myid == 0)
      length = params.hseg_out_nregions[0];
    MPI::COMM_WORLD.Bcast(&length, 1, MPI::INT, 0);
    if (params.myid != 0)
      params.hseg_out_nregions.push_back(length);
  }
  length = (int) params.hseg_out_thresholds_flag;
  MPI::COMM_WORLD.Bcast(&length, 1, MPI::INT, 0);
  params.hseg_out_thresholds_flag = (bool) length;
  if (params.hseg_out_thresholds_flag)
  {
    if (params.myid == 0)
      float_var = params.hseg_out_thresholds[0];
    MPI::COMM_WORLD.Bcast(&float_var, 1, MPI::FLOAT, 0);
    if (params.myid != 0)
      params.hseg_out_thresholds.push_back(float_var);
  }
  MPI::COMM_WORLD.Bcast(&params.conv_nregions, 1, MPI::UNSIGNED, 0);
  int debug = params.debug;
  if (params.debug < 2)
    params.debug = 0;
  MPI::COMM_WORLD.Bcast(&params.debug, 1, MPI::INT, 0);
  if (params.myid == 0)
    params.debug = debug;
  else
    params.calc_defaults();

  return true;
 }
} // namespace HSEGTilton
