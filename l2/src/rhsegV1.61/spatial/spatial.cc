/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>>          File:  spatial.cc
   >>>>
   >>>>          See spatial.h for documentation
   >>>>
   >>>>    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
   >>>>        E-Mail: James.C.Tilton@nasa.gov
   >>>>
   >>>>       Written: December 16, 2002
   >>>> Modifications: February 12, 2003 - Added option for classical region growing (instead of HSEG).
   >>>>                June 11, 2003 - Modified the temp_data structure and its use.
   >>>>                June 18, 2003 - Replaces scaled_value variable member with
   >>>>                byte_input_data and short_input_data members (to save disk space)
   >>>>                August 20, 2003 - Reorganized program and added user controlled byteswapping
   >>>>                March 4, 2004 - Changed boundary_flag to boundary_map
   >>>>                December 23, 2004 - Changed region label from short unsigned int to unsigned int
   >>>>                                    (NOTE:  region_class_label_map remains short unsigned int)
   >>>>                May 9, 2005 - Added code for integration with VisiQuest
   >>>>                May 25, 2005 - Added temporary file I/O for faster processing of large data sets
   >>>>                October 14, 2005 - Added slice dimension (extension to three-dimensional analysis)
   >>>>                December 9, 2005 - Reorganized the read_byte and read_short functions to eliminate use of "seekg".
   >>>>                November 1, 2007 - Eliminated VisiQuest related code.
   >>>>                February 20, 2008 - Modified to work with hsegreader program.
   >>>>                May 12, 2008 - Modified to work with new Params object class.
   >>>>                May 12, 2009 - Upgraded to work with float input image data and GDAL image data I/O.
   >>>>                March 24, 2011 - Revised counting of recursive levels to start at level = 0 (instead of level = 1).
   >>>>                April 7, 2011 - Initiated using set_stride_sections to determine stride and nb_sections (nb_tasks for parallel).
   >>>>                August 29, 2011 - Corrected scaling for writing shapefile vectors.
   >>>>                July 30, 2013 - Added code to input std_dev_image and edge_image and corresponding masks.
   >>>>                February 6, 2014 - Modified to convert signed short integer data to 32-bit float when negative values present
   >>>>                                   (instead of converting to unsigned short integer and improperly reading negative values).
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#include "spatial.h"
#include <params/params.h>
#include <region/region_class.h>
#include <region/region_object.h>
#include <pixel/pixel.h>
#include <index/index.h>
#include <iostream>
//#include <cmath>
#ifdef TIME_IT
#include <ctime>
#endif

extern HSEGTilton::Params params;
#ifdef GDAL
extern CommonTilton::Image inputImage;
extern CommonTilton::Image maskImage;
#ifdef RHSEG_RUN
extern CommonTilton::Image stdDevInImage;
extern CommonTilton::Image stdDevMaskImage;
extern CommonTilton::Image edgeInImage;
extern CommonTilton::Image edgeMaskImage;
extern CommonTilton::Image regionMapInImage;
#endif
extern Image classLabelsMapImage;
extern Image boundaryMapImage;
extern Image objectLabelsMapImage;
#endif

namespace HSEGTilton
{
 Spatial::Spatial()
 {
// Initialize member parameters as necessary.
  region_class_label_map = NULL;
  boundary_map = NULL;
  region_object_label_map = NULL;

  return;
 }

 Spatial::~Spatial()
 {
// Delete any allocated member arrays.
  if (region_class_label_map != NULL)
    delete [ ] region_class_label_map;
  if (boundary_map != NULL)
    delete [ ] boundary_map;
  if (region_object_label_map != NULL)
    delete [ ] region_object_label_map;

  return;
 }

#ifdef RHSEG_RUN
 void Spatial::read_data(vector<Pixel>& pixel_data, Temp& temp_data)
 {
// Input pixel (or voxel) data from disk.
  int nelements;
  int col, row;
#ifdef THREEDIM
  int slice;
#endif
  int band, index, index2, pixel_index;
  bool               *mask = NULL;           
  unsigned char      *byte_input_data = NULL;
  short unsigned int *short_input_data = NULL;
  float              *float_input_data = NULL;

  if ((params.mask_flag) || (params.std_dev_mask_flag) || (params.edge_mask_flag) || (params.padded_flag))
  {
   // NOTE: The nbands multiplier is included here in anticipation of reading in multi-band input data later.
#ifdef THREEDIM
    nelements = params.inb_ncols*params.inb_nrows*params.inb_nslices*params.nbands;
#else
    nelements = params.inb_ncols*params.inb_nrows*params.nbands;
#endif
    byte_input_data = new unsigned char[nelements];
  }

  if ((params.mask_flag) || (params.padded_flag))
  {
   // Input explicit mask from input mask data file if it exists,
   // or assume mask false only in padded areas
   // NOTE:  In the case where there is no input mask data file, and the data is not padded
   // for processing, the mask array is left as NULL.
#ifdef GDAL
#ifdef THREEDIM
    read_byte(params.inb_ncols,params.inb_nrows,params.inb_nslices,1,
              maskImage,"mask",byte_input_data,temp_data);
#else
    read_byte(params.inb_ncols,params.inb_nrows,1,
              maskImage,"mask",byte_input_data,temp_data);
#endif
#else
    if (params.mask_flag)
    {
     // Read mask from input mask data file
#ifdef THREEDIM
      read_byte(params.inb_ncols,params.inb_nrows,params.inb_nslices,1,
                params.mask_file.c_str(),"mask",byte_input_data,temp_data);
#else
      read_byte(params.inb_ncols,params.inb_nrows,1,
                params.mask_file.c_str(),"mask",byte_input_data,temp_data);
#endif
    }
    else
    {
     // With no input data mask, assume the mask array is false only in padded areas
#ifdef THREEDIM
      read_byte(params.inb_ncols,params.inb_nrows,params.inb_nslices,1,
                NULL,"mask",byte_input_data,temp_data);
#else
      read_byte(params.inb_ncols,params.inb_nrows,1,
                NULL,"mask",byte_input_data,temp_data);
#endif
    }
#endif // GDAL

// Load the mask array with values obtained from the read_byte function
#ifdef THREEDIM
    nelements = params.inb_ncols*params.inb_nrows*params.inb_nslices;
#else
    nelements = params.inb_ncols*params.inb_nrows;
#endif
    mask = new bool[nelements];
#ifndef PARALLEL
// In the serial version for nb_sections > 1, the mask array values are stored in temporary files.
    if (params.nb_sections == 1)
    {
#endif
#ifdef THREEDIM
      for (slice = 0; slice < params.inb_nslices; slice++)
      {
#endif
       for (row = 0; row < params.inb_nrows; row++)
        for (col = 0; col < params.inb_ncols; col++)
        {
#ifdef THREEDIM
          index = col + row*params.inb_ncols + slice*params.inb_ncols*params.inb_nrows;
#else
          index = col + row*params.inb_ncols;
#endif
          mask[index] = (byte_input_data[index] != params.mask_value);
        }
#ifdef THREEDIM
      }
#endif
#ifndef PARALLEL
    } // if (params.nb_sections == 1)
#endif
  } // if ((params.mask_flag) || (params.padded_flag))

#ifdef GDAL
  if (params.data_type == GDT_Byte)
#else
  if (params.dtype == UInt8)
#endif
  {
  // Read in the input data - unsigned byte case
#ifdef THREEDIM
    nelements = params.inb_ncols*params.inb_nrows*params.inb_nslices*params.nbands;
#else
    nelements = params.inb_ncols*params.inb_nrows*params.nbands;
#endif
    if (byte_input_data == NULL)
      byte_input_data = new unsigned char[nelements];
#ifdef GDAL
#ifdef THREEDIM
    read_byte(params.inb_ncols,params.inb_nrows,params.inb_nslices,params.nbands,
              inputImage,"input_image",byte_input_data,temp_data);
#else
    read_byte(params.inb_ncols,params.inb_nrows,params.nbands,
              inputImage,"input_image",byte_input_data,temp_data);
#endif
#else // GDAL
#ifdef THREEDIM
    read_byte(params.inb_ncols,params.inb_nrows,params.inb_nslices,params.nbands,
              params.input_image_file.c_str(),"input_image",byte_input_data,temp_data);
#else
    read_byte(params.inb_ncols,params.inb_nrows,params.nbands,
              params.input_image_file.c_str(),"input_image",byte_input_data,temp_data);
#endif
#endif // GDAL
  }
#ifdef GDAL
  else if (params.data_type == GDT_UInt16)
#else
  else if (params.dtype == UInt16)
#endif
  {
//  Read in the input data - unsigned short case
#ifdef THREEDIM
    nelements = params.inb_ncols*params.inb_nrows*params.inb_nslices*params.nbands;
#else
    nelements = params.inb_ncols*params.inb_nrows*params.nbands;
#endif
    short_input_data = new short unsigned int[nelements];
#ifdef GDAL
#ifdef THREEDIM
    read_short(params.inb_ncols,params.inb_nrows,params.inb_nslices,params.nbands,
              inputImage,"input_image",short_input_data,temp_data);
#else
    read_short(params.inb_ncols,params.inb_nrows,params.nbands,
              inputImage,"input_image",short_input_data,temp_data);
#endif
#else // GDAL
#ifdef THREEDIM
    read_short(params.inb_ncols,params.inb_nrows,params.inb_nslices,params.nbands,
               params.input_image_file.c_str(),"input_image",short_input_data,temp_data);
#else
    read_short(params.inb_ncols,params.inb_nrows,params.nbands,
               params.input_image_file.c_str(),"input_image",short_input_data,temp_data);
#endif
#endif // GDAL
  }
#ifdef GDAL
  else if ((params.data_type == GDT_Int16) || (params.data_type == GDT_UInt32) || (params.data_type == GDT_Int32) ||
           (params.data_type == GDT_Float32) || (params.data_type == GDT_Float64))
#else
  else if (params.dtype == Float32)
#endif
  {
//  Read in the input data - float case
#ifdef THREEDIM
    nelements = params.inb_ncols*params.inb_nrows*params.inb_nslices*params.nbands;
#else
    nelements = params.inb_ncols*params.inb_nrows*params.nbands;
#endif
    float_input_data = new float[nelements];
#ifdef GDAL
#ifdef THREEDIM
    read_float(params.inb_ncols,params.inb_nrows,params.inb_nslices,params.nbands,
              inputImage,"input_image",float_input_data,temp_data);
#else
    read_float(params.inb_ncols,params.inb_nrows,params.nbands,
              inputImage,"input_image",float_input_data,temp_data);
#endif
#else // GDAL
#ifdef THREEDIM
    read_float(params.inb_ncols,params.inb_nrows,params.inb_nslices,params.nbands,
               params.input_image_file.c_str(),"input_image",float_input_data,temp_data);
#else
    read_float(params.inb_ncols,params.inb_nrows,params.nbands,
               params.input_image_file.c_str(),"input_image",float_input_data,temp_data);
#endif
#endif // GDAL
  }

// Allocate region_class_label_map - Allocation must accommodate the largest size needed throughout the whole program
#ifdef THREEDIM
  nelements = params.pixel_ncols*params.pixel_nrows*params.pixel_nslices;  // NOTE: Needs to be "pixel_" rather than "inb_"!!
#else
  nelements = params.pixel_ncols*params.pixel_nrows;  // NOTE: Needs to be "pixel_" rather than "inb_"!!
#endif
  region_class_label_map = new unsigned int[nelements];

  if (params.region_map_in_flag)
  {
  // Assume region_map_in is a complete labeling of the image - will check if this is true later
    params.complete_labeling_flag = true;
  // Read in the input region label map
#ifdef GDAL
#ifdef THREEDIM
    read_rlblmap(params.inb_ncols,params.inb_nrows,params.inb_nslices,
                 regionMapInImage,"class_labels_map",region_class_label_map,temp_data);
#else
    read_rlblmap(params.inb_ncols,params.inb_nrows,
                 regionMapInImage,"class_labels_map",region_class_label_map,temp_data);
#endif
#else // GDAL
#ifdef THREEDIM
    read_rlblmap(params.inb_ncols,params.inb_nrows,params.inb_nslices,
                 params.region_map_in_file.c_str(),"class_labels_map",region_class_label_map,temp_data);
#else
    read_rlblmap(params.inb_ncols,params.inb_nrows,
                 params.region_map_in_file.c_str(),"class_labels_map",region_class_label_map,temp_data);
#endif
#endif // GDAL
  }

// Load pixel_data
  short unsigned int section;
#ifndef PARALLEL
  for (section = 0; section < params.nb_sections; ++section)
  {
    if (params.nb_sections > 1)
    {
      restore_input_data(section,mask,byte_input_data,short_input_data,float_input_data);
      if (params.region_map_in_flag)
        restore_region_class_label_map(section);
    }
#else
    section = (short unsigned int) params.myid;
#endif
#ifdef THREEDIM
    for (slice = 0; slice < params.inb_nslices; slice++)
    {
#endif
      for (row = 0; row < params.inb_nrows; row++)
      {
        for (col = 0; col < params.inb_ncols; col++)
        {
#ifdef THREEDIM
          index = col + row*params.inb_ncols + slice*params.inb_ncols*params.inb_nrows;
          pixel_index = col + row*params.pixel_ncols + slice*params.pixel_ncols*params.pixel_nrows;
#else
          index = col + row*params.inb_ncols;
          pixel_index = col + row*params.pixel_ncols;
#endif
          pixel_data[pixel_index].clear();
          if ((mask == NULL) || (mask[index]))
          {
            if (params.region_map_in_flag)
            {
              pixel_data[pixel_index].region_label = region_class_label_map[index];
              if (pixel_data[pixel_index].region_label == 0)
                params.complete_labeling_flag = false;
            }
            else
              pixel_data[pixel_index].region_label = 0;
            index2 = index;
            for (band = 0; band < params.nbands; ++band)
            {
#ifdef GDAL
              if (params.data_type == GDT_Byte)
#else
              if (params.dtype == UInt8)
#endif
                pixel_data[pixel_index].set_input_data(band,byte_input_data[index2]);
#ifdef GDAL
              else if (params.data_type == GDT_UInt16)
#else
              else if (params.dtype == UInt16)
#endif
                pixel_data[pixel_index].set_input_data(band,short_input_data[index2]);
#ifdef GDAL
              else if ((params.data_type == GDT_Int16) || (params.data_type == GDT_UInt32) || (params.data_type == GDT_Int32) ||
                       (params.data_type == GDT_Float32) || (params.data_type == GDT_Float64))
#else
              else if (params.dtype == Float32)
#endif
                pixel_data[pixel_index].set_input_data(band,float_input_data[index2]);
#ifdef THREEDIM
              index2 += params.inb_ncols*params.inb_nrows*params.inb_nslices;
#else
              index2 += params.inb_ncols*params.inb_nrows;
#endif
            }
            pixel_data[pixel_index].init_flag = false;
            pixel_data[pixel_index].mask = true;
            pixel_data[pixel_index].std_dev_mask = false;
            pixel_data[pixel_index].edge_value = -FLT_MAX;
            pixel_data[pixel_index].edge_mask = false;
          }
          else
          {
            pixel_data[pixel_index].region_label = 0;
            pixel_data[pixel_index].init_flag = true;
            pixel_data[pixel_index].mask = false;
            pixel_data[pixel_index].std_dev_mask = false;
            pixel_data[pixel_index].edge_value = -FLT_MAX;
            pixel_data[pixel_index].edge_mask = false;
          }
        }
      }
#ifdef THREEDIM
    }
#endif
#ifndef PARALLEL
    if (params.nb_sections > 1)
      save_pixel_data(section,pixel_data,temp_data);
  }
#endif

  if (params.std_dev_image_flag)
  {
    if ((params.std_dev_mask_flag) || (params.padded_flag))
    {
     // Input explicit std_dev_mask from input standard deviation mask data file if it exists,
     // or assume std_dev_mask false only in padded areas
     // NOTE:  In the case where there is no input standard deviation data file, and the data is not padded
     // for processing, the std_dev_mask array is left as NULL.
      params.mask_value = 0;
#ifdef GDAL
#ifdef THREEDIM
      read_byte(params.inb_ncols,params.inb_nrows,params.inb_nslices,1,
                stdDevMaskImage,"std_dev_mask",byte_input_data,temp_data);
#else
      read_byte(params.inb_ncols,params.inb_nrows,1,
                stdDevMaskImage,"std_dev_mask",byte_input_data,temp_data);
#endif
#else
      if (params.std_dev_mask_flag)
      {
       // Read std_dev_mask from input standard devation mask data file
#ifdef THREEDIM
        read_byte(params.inb_ncols,params.inb_nrows,params.inb_nslices,1,
                  params.std_dev_mask_file.c_str(),"std_dev_mask",byte_input_data,temp_data);
#else
        read_byte(params.inb_ncols,params.inb_nrows,1,
                  params.std_dev_mask_file.c_str(),"std_dev_mask",byte_input_data,temp_data);
#endif
      }
      else
      {
       // With no input standard deviation mask data, assume the std_dev_mask array is false only in padded areas
#ifdef THREEDIM
        read_byte(params.inb_ncols,params.inb_nrows,params.inb_nslices,1,
                  NULL,"std_dev_mask",byte_input_data,temp_data);
#else
        read_byte(params.inb_ncols,params.inb_nrows,1,
                  NULL,"std_dev_mask",byte_input_data,temp_data);
#endif
      }
#endif // GDAL

  // Load the std_dev_mask array with values obtained from the read_byte function
#ifdef THREEDIM
      nelements = params.inb_ncols*params.inb_nrows*params.inb_nslices;
#else
      nelements = params.inb_ncols*params.inb_nrows;
#endif
      if (mask == NULL)
        mask = new bool[nelements];
#ifndef PARALLEL
  // In the serial version for nb_sections > 1, the std_dev_mask array values are stored in temporary files.
      if (params.nb_sections == 1)
      {
#endif
#ifdef THREEDIM
        for (slice = 0; slice < params.inb_nslices; slice++)
        {
#endif
         for (row = 0; row < params.inb_nrows; row++)
          for (col = 0; col < params.inb_ncols; col++)
          {
#ifdef THREEDIM
            index = col + row*params.inb_ncols + slice*params.inb_ncols*params.inb_nrows;
#else
            index = col + row*params.inb_ncols;
#endif
            mask[index] = (byte_input_data[index] != params.mask_value);
          }
#ifdef THREEDIM
        }
#endif
#ifndef PARALLEL
      } // if (params.nb_sections == 1)
#endif
    } // if ((params.std_dev_mask_flag) || (params.padded_flag))

#ifdef THREEDIM
    nelements = params.inb_ncols*params.inb_nrows*params.inb_nslices*params.nbands;
#else
    nelements = params.inb_ncols*params.inb_nrows*params.nbands;
#endif
    if (float_input_data == NULL)
      float_input_data = new float[nelements];
// Read in the input standard deviation image
#ifdef GDAL
#ifdef THREEDIM
    read_float(params.inb_ncols,params.inb_nrows,params.inb_nslices,params.nbands,
               stdDevInImage,"std_dev_image",float_input_data,temp_data);
#else
    read_float(params.inb_ncols,params.inb_nrows,params.nbands,
               stdDevInImage,"std_dev_image",float_input_data,temp_data);
#endif
#else // GDAL
#ifdef THREEDIM
    read_float(params.inb_ncols,params.inb_nrows,params.inb_nslices,params.nbands,
               params.std_dev_image_file.c_str(),"std_dev_image",float_input_data,temp_data);
#else
    read_float(params.inb_ncols,params.inb_nrows,params.nbands,
               params.std_dev_image_file.c_str(),"std_dev_image",float_input_data,temp_data);
#endif
#endif // GDAL

  // Load pixel_data
#ifndef PARALLEL
    for (section = 0; section < params.nb_sections; ++section)
    {
      if (params.nb_sections > 1)
      {
        restore_pixel_data(section,pixel_data,temp_data);
        restore_std_dev_data(section,mask,byte_input_data,float_input_data);
      }
#endif
#ifdef THREEDIM
      for (slice = 0; slice < params.inb_nslices; slice++)
      {
#endif
        for (row = 0; row < params.inb_nrows; row++)
        {
          for (col = 0; col < params.inb_ncols; col++)
          {
#ifdef THREEDIM
            index = col + row*params.inb_ncols + slice*params.inb_ncols*params.inb_nrows;
            pixel_index = col + row*params.pixel_ncols + slice*params.pixel_ncols*params.pixel_nrows;
#else
            index = col + row*params.inb_ncols;
            pixel_index = col + row*params.pixel_ncols;
#endif
            if (pixel_data[pixel_index].mask && ((mask == NULL) || (mask[index])))
            {
              index2 = index;
              for (band = 0; band < params.nbands; ++band)
              {
                pixel_data[pixel_index].set_local_std_dev(band,float_input_data[index2]);
#ifdef THREEDIM
                index2 += params.inb_ncols*params.inb_nrows*params.inb_nslices;
#else
                index2 += params.inb_ncols*params.inb_nrows;
#endif
              }
              pixel_data[pixel_index].std_dev_mask = true;
            }
            else
            {
              pixel_data[pixel_index].std_dev_mask = false;
            }
          }
        }
#ifdef THREEDIM
      }
#endif
#ifndef PARALLEL
      if (params.nb_sections > 1)
        save_pixel_data(section,pixel_data,temp_data);
    }
#endif
  } // if (params.std_dev_image_flag)

  if (params.edge_image_flag)
  {
    if ((params.edge_mask_flag) || (params.padded_flag))
    { 
     // Input explicit edge_mask from input edge mask data file if it exists,
     // or assume edge_mask false only in padded areas
     // NOTE:  In the case where there is no input edge mask data file, and the data is not padded
     // for processing, the edge_mask array is left as NULL.
      params.mask_value = 0;
#ifdef GDAL
#ifdef THREEDIM
      read_byte(params.inb_ncols,params.inb_nrows,params.inb_nslices,1,
                edgeMaskImage,"edge_mask",byte_input_data,temp_data);
#else
      read_byte(params.inb_ncols,params.inb_nrows,1,
                edgeMaskImage,"edge_mask",byte_input_data,temp_data);
#endif
#else
      if (params.edge_mask_flag)
      {
       // Read edge_mask from input edge mask data file
#ifdef THREEDIM
        read_byte(params.inb_ncols,params.inb_nrows,params.inb_nslices,1,
                  params.edge_mask_file.c_str(),"edge_mask",byte_input_data,temp_data);
#else
        read_byte(params.inb_ncols,params.inb_nrows,1,
                  params.edge_mask_file.c_str(),"edge_mask",byte_input_data,temp_data);
#endif
      }
      else
      {
       // With no input edge mask data, assume the edge_mask array is false only in padded areas
#ifdef THREEDIM
        read_byte(params.inb_ncols,params.inb_nrows,params.inb_nslices,1,
                  NULL,"edge_mask",byte_input_data,temp_data);
#else
        read_byte(params.inb_ncols,params.inb_nrows,1,
                  NULL,"edge_mask",byte_input_data,temp_data);
#endif
      }
#endif // GDAL

  // Load the edge_mask array with values obtained from the read_byte function
#ifdef THREEDIM
      nelements = params.inb_ncols*params.inb_nrows*params.inb_nslices;
#else
      nelements = params.inb_ncols*params.inb_nrows;
#endif
      if (mask == NULL)
        mask = new bool[nelements];
#ifndef PARALLEL
  // In the serial version for nb_sections > 1, the edge_mask array values are stored in temporary files.
      if (params.nb_sections == 1)
      {
#endif
#ifdef THREEDIM
        for (slice = 0; slice < params.inb_nslices; slice++)
        {
#endif
         for (row = 0; row < params.inb_nrows; row++)
          for (col = 0; col < params.inb_ncols; col++)
          {
#ifdef THREEDIM
            index = col + row*params.inb_ncols + slice*params.inb_ncols*params.inb_nrows;
#else
            index = col + row*params.inb_ncols;
#endif
            mask[index] = (byte_input_data[index] != params.mask_value);
          }
#ifdef THREEDIM
        }
#endif
#ifndef PARALLEL
      } // if (params.nb_sections == 1)
#endif
    } // if ((params.edge_mask_flag) || (params.padded_flag))

#ifdef THREEDIM
    nelements = params.inb_ncols*params.inb_nrows*params.inb_nslices;
#else
    nelements = params.inb_ncols*params.inb_nrows;
#endif
    if (float_input_data == NULL)
      float_input_data = new float[nelements];
// Read in the input edge image
#ifdef GDAL
#ifdef THREEDIM
    read_float(params.inb_ncols,params.inb_nrows,params.inb_nslices,1,
               edgeInImage,"edge_image",float_input_data,temp_data);
#else
    read_float(params.inb_ncols,params.inb_nrows,1,
               edgeInImage,"edge_image",float_input_data,temp_data);
#endif
#else // GDAL
#ifdef THREEDIM
    read_float(params.inb_ncols,params.inb_nrows,params.inb_nslices,1,
               params.edge_image_file.c_str(),"edge_image",float_input_data,temp_data);
#else
    read_float(params.inb_ncols,params.inb_nrows,1,
               params.edge_image_file.c_str(),"edge_image",float_input_data,temp_data);
#endif
#endif // GDAL

  // Load pixel_data
#ifndef PARALLEL
    short unsigned int section;
    for (section = 0; section < params.nb_sections; ++section)
    {
      if (params.nb_sections > 1)
      {
        restore_pixel_data(section,pixel_data,temp_data);
        restore_edge_data(section,mask,byte_input_data,float_input_data);
      }
#endif
#ifdef THREEDIM
      for (slice = 0; slice < params.inb_nslices; slice++)
      {
#endif
        for (row = 0; row < params.inb_nrows; row++)
        {
          for (col = 0; col < params.inb_ncols; col++)
          {
#ifdef THREEDIM
            index = col + row*params.inb_ncols + slice*params.inb_ncols*params.inb_nrows;
            pixel_index = col + row*params.pixel_ncols + slice*params.pixel_ncols*params.pixel_nrows;
#else
            index = col + row*params.inb_ncols;
            pixel_index = col + row*params.pixel_ncols;
#endif
            if (pixel_data[pixel_index].mask && ((mask == NULL) || (mask[index])))
            {
              pixel_data[pixel_index].edge_value = float_input_data[index];
              pixel_data[pixel_index].edge_mask = true;
            }
            else
            {
              pixel_data[pixel_index].edge_value = -FLT_MAX;  // Changed from 1.0 to 0.0 on November 19, 2013, and to -FLT_MAX on Jan. 9, 2014
              pixel_data[pixel_index].edge_mask = false;
            }
          }
        }
#ifdef THREEDIM
      }
#endif
#ifndef PARALLEL
      if (params.nb_sections > 1)
        save_pixel_data(section,pixel_data,temp_data);
    }
#endif
  } // if (params.edge_image_flag)

  if (mask != NULL)
    delete [ ] mask;
  mask = NULL;
  if (byte_input_data != NULL)
    delete [ ] byte_input_data;
  byte_input_data = NULL;
  if (short_input_data != NULL)
    delete [ ] short_input_data;
  short_input_data = NULL;
  if (float_input_data != NULL)
    delete [ ] float_input_data;
  float_input_data = NULL;

// Allocate the boundary_map and region_object_label_map as needed.
#ifdef THREEDIM
  nelements = params.pixel_ncols*params.pixel_nrows*params.pixel_nslices;  // NOTE: Needs to be "pixel_" rather than "inb_"!!
#else
  nelements = params.pixel_ncols*params.pixel_nrows;  // NOTE: Needs to be "pixel_" rather than "inb_"!!
#endif
  if ((params.boundary_map_flag) || (params.region_nb_objects_flag))
    boundary_map = new short unsigned int[nelements];
  if (params.region_nb_objects_flag)
    region_object_label_map = new unsigned int[nelements];

  return;
 }

#ifdef GDAL
#ifdef THREEDIM
  void Spatial::read_byte(const int& io_ncols, const int& io_nrows, const int& io_nslices,
                          const int& number_of_bands, Image& byteImage,
                          const string& file_type, unsigned char *byte_buffer,
                          Temp& temp_data)
#else
  void Spatial::read_byte(const int& io_ncols, const int& io_nrows,
                          const int& number_of_bands, Image& byteImage,
                          const string& file_type, unsigned char *byte_buffer,
                          Temp& temp_data)
#endif
#else // GDAL
#ifdef THREEDIM
  void Spatial::read_byte(const int& io_ncols, const int& io_nrows, const int& io_nslices,
                          const int& number_of_bands, const char *byte_file_name,
                          const string& file_type, unsigned char *byte_buffer,
                          Temp& temp_data)
#else
  void Spatial::read_byte(const int& io_ncols, const int& io_nrows,
                          const int& number_of_bands, const char *byte_file_name,
                          const string& file_type, unsigned char *byte_buffer,
                          Temp& temp_data)
#endif
#endif //GDAL
 {
// Reads the specified unsigned char (byte) data from disk (or creates a mask array)
// For the parallel case, the data is sent (and received) in the appropriate parallel task.
// For the serial case with nb_sections > 1, the data written to the appropriate temporary file.
#ifndef GDAL
    unsigned char *io_byte_buffer;
#endif
    short unsigned int band, section;
    int col, row;
#ifndef GDAL
    unsigned int index2, io_size; 
#endif
    unsigned int index, section_size;
    int ionb_rowoffset, ionb_coloffset;
    int max_ncols, max_nrows;
#ifdef THREEDIM
    int slice, ionb_sliceoffset, max_nslices;
#endif

#ifndef GDAL
 // "io_size" is the number of data elements read or written at one time to or from disk
    fstream data_file;
    io_size = params.ncols*io_nrows;
 // "io_byte_buffer" is the buffer used for i/o to and from the disk
    io_byte_buffer = new unsigned char[io_size];
#endif
 // "section_size" is the number of data elements in each processing section
#ifdef THREEDIM
    section_size = io_ncols*io_nrows*io_nslices*number_of_bands;
#else
    section_size = io_ncols*io_nrows*number_of_bands;
#endif
#ifdef PARALLEL
 // "pio_size" is the number of data elements in each parallel data transfer
    unsigned int pio_size;
    pio_size = io_ncols*io_nrows;
    check_buf_size(pio_size,0,0,0,0,temp_data);
#else
    check_buf_size(section_size,0,0,0,0,temp_data);
#endif

#ifdef PARALLEL
    int byte_data_tag = 8;
    if (params.myid == 0)
    {
// In the parallel case, the task at recur_level = 0 (task 0) reads in the data
// and parcels it out the other tasks.
// The data for current task is also retained in the byte_buffer.
#else
// In the serial case with params.nb_sections > 1, the data is read in and
// written out to temporary files corresponding to the processing sections.
#endif // PARALLEL
#ifndef GDAL
      if (((file_type == "mask") || (file_type == "std_dev_mask") || (file_type == "edge_mask")) &&
          (byte_file_name == NULL))
      {
// In this case the mask data is requested for when the data is padded for processing.
// Here the data in io_byte_buffer is initialize to != mask_value, so that it may
// be later reset to == mask_value in the padded areas.
        for (index = 0; index < io_size; ++index)
          io_byte_buffer[index] = params.mask_value + 1;
      }
      else
      {
// In this case the mask data is to be read from a data file.
        data_file.open(byte_file_name, ios_base::in | ios_base::binary );
        if (!data_file)
        {
          if (params.debug > 0)
            params.log_fs << "ERROR:  Failed to open byte data file: " << byte_file_name << endl;
          else
            cout << "ERROR:  Failed to open byte data file: " << byte_file_name << endl;
          return;
        }
      }
#endif // !GDAL

      for (band = 0; band < number_of_bands; ++band)
      {
#ifdef THREEDIM
        for (ionb_sliceoffset = 0;
             ionb_sliceoffset < params.nslices;
             ionb_sliceoffset += io_nslices)
        {
          max_nslices = params.nslices - ionb_sliceoffset;
          if (max_nslices > io_nslices)
            max_nslices = io_nslices;
          for (slice = 0; slice < io_nslices; ++slice)
          {
#endif
            for (ionb_rowoffset = 0;
                 ionb_rowoffset < params.nrows;
                 ionb_rowoffset += io_nrows)
            {
              max_nrows = params.nrows - ionb_rowoffset;
              if (max_nrows > io_nrows)
                max_nrows = io_nrows;
#ifndef GDAL
#ifdef THREEDIM
              if (slice < max_nslices)
              {
#endif
              // The spectral bands are read in separate reads from disk
              // ncols by max_nrows of data are read in each read from disk
                if (byte_file_name != NULL)
                  data_file.read(reinterpret_cast<char *>(io_byte_buffer),(params.ncols*max_nrows));
#ifdef THREEDIM
              }  // if (slice < max_nslices)
#endif
#endif // !GDAL
              for (ionb_coloffset = 0;
                   ionb_coloffset < params.ncols;
                   ionb_coloffset += io_ncols)
              {
                max_ncols = params.ncols - ionb_coloffset;
                if (max_ncols > io_ncols)
                  max_ncols = io_ncols;
              // Determine which section of data is currently to be dealt with
#ifdef THREEDIM
                section = find_section(ionb_coloffset,ionb_rowoffset,ionb_sliceoffset);
#else
                section = find_section(ionb_coloffset,ionb_rowoffset);
#endif
#ifndef PARALLEL
              // If this is not the first time this section is visited, restore to memory the result of the previous visit
#ifdef THREEDIM
                if (((band != 0) || (slice != 0)) && (section > 0))
                {
                  restore_byte_data(file_type,section,section_size,temp_data.byte_buffer);
                }
#else // THREEDIM
                if ((band != 0) && (section > 0))
                {
                  restore_byte_data(file_type,section,section_size,temp_data.byte_buffer);
                }
#endif // THREEDIM
#endif // PARALLEL
                for (row = 0; row < io_nrows; row++)
                  for (col = 0; col < io_ncols; col++)
                  {
#ifndef GDAL
                    index2 = col + ionb_coloffset + row*params.ncols;
#endif
                    if (section == 0)
                    {
                    // For section == 0, place (properly masked) data input the byte_buffer
#ifdef THREEDIM
                      index = col + row*io_ncols + slice*io_nrows*io_ncols + band*io_nslices*io_nrows*io_ncols;
                      if ((slice < max_nslices) && (row < max_nrows) && (col < max_ncols))
#else
                      index = col + row*io_ncols + band*io_nrows*io_ncols;
                      if ((row < max_nrows) && (col < max_ncols))
#endif
                      {
#ifdef GDAL
                        if (byteImage.info_valid())
                        {
#ifdef THREEDIM
                          byte_buffer[index] = 
                            (unsigned char) byteImage.get_data(col+ionb_coloffset,row+ionb_rowoffset,slice+ionb_sliceoffset,band);
#else
                          byte_buffer[index] = 
                            (unsigned char) byteImage.get_data(col+ionb_coloffset,row+ionb_rowoffset,band);
#endif
                        }
                        else
                          byte_buffer[index] = params.mask_value + 1;
#else //GDAL
                        byte_buffer[index] = io_byte_buffer[index2];
#endif //GDAL
                      }
                      else
                        byte_buffer[index] = params.mask_value;
                    }
                    else
                    {
                    // For section != 0, place (properly masked) data input temp_data.byte_buffer
#ifdef PARALLEL
                      index = col + row*io_ncols;
#else // PARALLEL
#ifdef THREEDIM
                      index = col + row*io_ncols + slice*io_nrows*io_ncols + band*io_nslices*io_nrows*io_ncols;
#else // THREEDIM
                      index = col + row*io_ncols + band*io_nrows*io_ncols;
#endif // THREEDIM
#endif // PARALLEL
#ifdef THREEDIM
                      if ((slice < max_nslices) && (row < max_nrows) && (col < max_ncols))
#else
                      if ((row < max_nrows) && (col < max_ncols))
#endif
                      {
#ifdef GDAL
                        if (byteImage.info_valid())
                        {
#ifdef THREEDIM
                          temp_data.byte_buffer[index] = 
                            (unsigned char) byteImage.get_data(col+ionb_coloffset,row+ionb_rowoffset,slice+ionb_sliceoffset,band);
#else
                          temp_data.byte_buffer[index] = 
                            (unsigned char) byteImage.get_data(col+ionb_coloffset,row+ionb_rowoffset,band);
#endif
                        }
                        else
                          temp_data.byte_buffer[index] = params.mask_value + 1;
#else // GDAL
                        temp_data.byte_buffer[index] = io_byte_buffer[index2];
#endif //GDAL
                      }
                      else
                        temp_data.byte_buffer[index] = params.mask_value;
                    }
                  }
                if (section > 0)
                {
#ifdef PARALLEL
                // In the parallel case, send the data to the appropriate processor (from the master process, myid == 0)
                  MPI::COMM_WORLD.Send(temp_data.byte_buffer, pio_size,
                                       MPI::UNSIGNED_CHAR, section, byte_data_tag);
#else
                // In the serial case (with nb_sections > 0), write the data to the appropriate temporary file
                  save_byte_data(file_type,section,section_size,temp_data.byte_buffer);
#endif
                }
              }  // for (ionb_coloffset = 0; ionb_coloffset < params.ncols; ionb_coloffset += io_ncols)
            } // for (ionb_rowoffset = 0; ionb_rowoffset < params.nrows; ionb_rowoffset += io_nrows)
#ifdef THREEDIM
          } // for (slice = 0; slice < io_nslices; ++slice)
        } // for (ionb_sliceoffset = 0; ionb_sliceoffset < params.nslices; ionb_sliceoffset += io_nslices)
#endif
      } // for (band = 0; band < number_of_bands; ++band)
#ifdef PARALLEL
    } // if (params.myid == 0)
    else
    {
   // In the parallel case, for the worker processes (myid != 0), receive the data and place it
   // in the proper place in byte_buffer
#ifdef GDAL
      unsigned int index2;
#endif
      for (band = 0; band < number_of_bands; ++band)
      {
#ifdef THREEDIM
        for (slice = 0; slice < io_nslices; ++slice)
        {
#endif
          MPI::Status status;
          MPI::COMM_WORLD.Recv(temp_data.byte_buffer, pio_size,
                               MPI::UNSIGNED_CHAR, 0, byte_data_tag, status);
          for (row = 0; row < io_nrows; row++)
            for (col = 0; col < io_ncols; col++)
            {
#ifdef THREEDIM
              index = col + row*io_ncols + slice*io_nrows*io_ncols + band*io_nslices*io_nrows*io_ncols;
              index2 = col + row*io_ncols;
#else
              index = col + row*io_ncols + band*io_nrows*io_ncols;
              index2 = col + row*io_ncols;
#endif
              byte_buffer[index] = temp_data.byte_buffer[index2];
            }
#ifdef THREEDIM
        } // for (slice = 0; slice < io_nslices; ++slice)
#endif
      } // for (band = 0; band < number_of_bands; ++band)
    }
#endif // PARALLEL
#ifndef PARALLEL
  // Still need to store the section 0 data into the appropriate temporary file in the serial case
  // with nb_sections > 1.
    if (params.nb_sections > 1)
      save_byte_data(file_type,0,section_size,byte_buffer);
#endif // PARALLEL
#ifndef GDAL
#ifdef PARALLEL
    if (params.myid == 0)
#endif
      if (byte_file_name != NULL)
        data_file.close();
    delete [] io_byte_buffer;
#endif
#ifdef DEBUG
    if (params.debug > 3)
    {
      params.log_fs << "byte_buffer (read_byte):" << endl << endl;
#ifndef PARALLEL
      for (section = 0; section < params.nb_sections; ++section)
      {
        if (params.nb_sections > 1)
          restore_byte_data(file_type,section,section_size,byte_buffer);
#endif
        for (band = 0; band < number_of_bands; band++)
        {
          if (number_of_bands > 1)
            params.log_fs << "For band " << band << ":" << endl;
#ifdef THREEDIM
          for (slice = 0; slice < io_nslices; slice++)
          {
            if (io_nslices > 1)
              params.log_fs << "For slice " << slice << ":" << endl;
#endif
            for (row = 0; row < io_nrows; row++)
            {
              for (col = 0; col < io_ncols; col++)
              {
#ifdef THREEDIM
                index = col + row*io_ncols + slice*io_nrows*io_ncols + band*io_nslices*io_nrows*io_ncols;
#else
                index = col + row*io_ncols + band*io_nrows*io_ncols;
#endif
                params.log_fs << (int) byte_buffer[index] << " ";
              }
              params.log_fs << endl;
            }
            params.log_fs << endl;
#ifdef THREEDIM
          }
          params.log_fs << endl;
#endif
        }
#ifndef PARALLEL
      }
#endif
    }
#endif // DEBUG
  }

#ifdef GDAL
#ifdef THREEDIM
  void Spatial::read_short(const int& io_ncols, const int& io_nrows, const int& io_nslices,
                           const int& number_of_bands, Image& shortImage,
                           const string& file_type, short unsigned int *short_buffer,
                           Temp& temp_data)
#else
  void Spatial::read_short(const int& io_ncols, const int& io_nrows,
                           const int& number_of_bands, Image& shortImage,
                           const string& file_type, short unsigned int *short_buffer,
                           Temp& temp_data)
#endif
#else // GDAL
#ifdef THREEDIM
  void Spatial::read_short(const int& io_ncols, const int& io_nrows, const int& io_nslices,
                           const int& number_of_bands, const char *short_file_name,
                           const string& file_type, short unsigned int *short_buffer,
                           Temp& temp_data)
#else
  void Spatial::read_short(const int& io_ncols, const int& io_nrows,
                           const int& number_of_bands, const char *short_file_name,
                           const string& file_type, short unsigned int *short_buffer,
                           Temp& temp_data)
#endif
#endif // GDAL
  {
// Reads the specified unsigned short data from disk.
// For the parallel case, the data is sent (and received) in the appropriate parallel task.
// For the serial case with nb_sections > 1, the data written to the appropriate temporary file.
#ifndef GDAL
    short unsigned int *io_short_buffer = NULL;
#endif
    short unsigned int band, section;
    int col, row;
#ifndef GDAL
    unsigned int index2, io_size;
#endif
    unsigned int index, section_size;
    int ionb_rowoffset, ionb_coloffset;
    int max_ncols, max_nrows;
#ifdef THREEDIM
    int slice, ionb_sliceoffset, max_nslices;
#endif

#ifndef GDAL
 // "io_size" is the number of data elements read or written at one time to or from disk
    fstream data_file;
    io_size = params.ncols*io_nrows;
 // "io_short_buffer" is the buffer used for i/o to and from the disk
    io_short_buffer = new short unsigned int[io_size];
#endif
 // "section_size" is the number of data elements in each processing section
#ifdef THREEDIM
    section_size = io_ncols*io_nrows*io_nslices*number_of_bands;
#else
    section_size = io_ncols*io_nrows*number_of_bands;
#endif
#ifdef PARALLEL
 // "pio_size" is the number of data elements in each parallel data transfer
    unsigned int pio_size;
    pio_size = io_ncols*io_nrows;
    check_buf_size(0,pio_size,0,0,0,temp_data);
#else
    check_buf_size(0,section_size,0,0,0,temp_data);
#endif

#ifdef PARALLEL
    int short_data_tag = 16;
    if (params.myid == 0)
    {
// In the parallel case, the task at recur_level = 0 (task 0) reads in the data
// and parcels it out the other tasks.
// The data for current task is also retained in the short_buffer.
#else
// In the serial case with params.nb_sections > 1, the data is read in and
// written out to temporary files corresponding to the processing sections.
#endif
#ifndef GDAL
      data_file.open(short_file_name, ios_base::in | ios_base::binary );
      if (!data_file)
      {
        if (params.debug > 0)
          params.log_fs << "ERROR:  Failed to open short data file: " << short_file_name << endl;
        else
          cout << "ERROR:  Failed to open short data file: " << short_file_name << endl;
        return;
      }
#endif // GDAL

      for (band = 0; band < number_of_bands; ++band)
      {
#ifdef THREEDIM
        for (ionb_sliceoffset = 0;
             ionb_sliceoffset < params.nslices;
             ionb_sliceoffset += io_nslices)
        {
          max_nslices = params.nslices - ionb_sliceoffset;
          if (max_nslices > io_nslices)
            max_nslices = io_nslices;
          for (slice = 0; slice < io_nslices; slice++)
          {
#endif
            for (ionb_rowoffset = 0;
                 ionb_rowoffset < params.nrows;
                 ionb_rowoffset += io_nrows)
            {
              max_nrows = params.nrows - ionb_rowoffset;
              if (max_nrows > io_nrows)
                max_nrows = io_nrows;
#ifndef GDAL
#ifdef THREEDIM
              if (slice < max_nslices)
              {
#endif
               // The spectral bands are read in separate reads from disk
               // ncols by max_nrows of data are read in each read from disk
                data_file.read(reinterpret_cast<char *>(io_short_buffer),2*params.ncols*max_nrows);
#ifdef THREEDIM
              }  // if (slice < max_nslices)
#endif
#endif // GDAL
              for (ionb_coloffset = 0;
                   ionb_coloffset < params.ncols;
                   ionb_coloffset += io_ncols)
              {
                max_ncols = params.ncols - ionb_coloffset;
                if (max_ncols > io_ncols)
                  max_ncols = io_ncols;
              // Determine which section of data is currently to be dealt with
#ifdef THREEDIM
                section = find_section(ionb_coloffset,ionb_rowoffset,ionb_sliceoffset);
#else
                section = find_section(ionb_coloffset,ionb_rowoffset);
#endif
#ifndef PARALLEL
              // If this is not the first time this section is visited, restore to memory the result of the previous visit
#ifdef THREEDIM
                if (((band != 0) || (slice != 0)) && (section > 0))
                {
                  restore_short_data(file_type,section,section_size,temp_data.short_buffer);
                }
#else // THREEDIM
                if ((band != 0) && (section > 0))
                {
                  restore_short_data(file_type,section,section_size,temp_data.short_buffer);
                }
#endif // THREEDIM
#endif // PARALLEL
                for (row = 0; row < io_nrows; row++)
                  for (col = 0; col < io_ncols; col++)
                  {
#ifndef GDAL
                    index2 = col + ionb_coloffset + row*params.ncols;
#endif
                    if (section == 0)
                    {
                    // For section == 0, place (properly masked) data input the short_buffer
#ifdef THREEDIM
                      index = col + row*io_ncols + slice*io_ncols*io_nrows + band*io_ncols*io_nrows*io_nslices;
                      if ((slice < max_nslices) && (row < max_nrows) && (col < max_ncols))
#else
                      index = col + row*io_ncols + band*io_ncols*io_nrows;
                      if ((row < max_nrows) && (col < max_ncols))
#endif
#ifdef GDAL
#ifdef THREEDIM
                          short_buffer[index] = 
                            (unsigned short) shortImage.get_data(col+ionb_coloffset,row+ionb_rowoffset,slice+ionb_sliceoffset,band);
#else
                          short_buffer[index] = 
                            (unsigned short) shortImage.get_data(col+ionb_coloffset,row+ionb_rowoffset,band);
#endif
#else // GDAL
                        short_buffer[index] = io_short_buffer[index2];
#endif // GDAL
                      else
                        short_buffer[index] = 0;
                    }
                    else
                    {
                    // For section != 0, place (properly masked) data input temp_data.short_buffer
#ifdef PARALLEL
                      index = col + row*io_ncols;
#else // PARALLEL
#ifdef THREEDIM
                      index = col + row*io_ncols + slice*io_ncols*io_nrows + band*io_ncols*io_nrows*io_nslices;
#else // THREEDIM
                      index = col + row*io_ncols + band*io_ncols*io_nrows;
#endif // THREEDIM
#endif // PARALLEL
#ifdef THREEDIM
                      if ((slice < max_nslices) && (row < max_nrows) && (col < max_ncols))
#else
                      if ((row < max_nrows) && (col < max_ncols))
#endif
#ifdef GDAL
#ifdef THREEDIM
                          temp_data.short_buffer[index] = 
                            (unsigned short) shortImage.get_data(col+ionb_coloffset,row+ionb_rowoffset,slice+ionb_sliceoffset,band);
#else
                          temp_data.short_buffer[index] = 
                            (unsigned short) shortImage.get_data(col+ionb_coloffset,row+ionb_rowoffset,band);
#endif
#else // GDAL
                        temp_data.short_buffer[index] = io_short_buffer[index2];
#endif // GDAL
                      else
                        temp_data.short_buffer[index] = 0;
                    }
                  }
                if (section > 0)
                {
#ifdef PARALLEL
                // In the parallel case, send the data to the appropriate processor (from the master process, myid == 0)
                  MPI::COMM_WORLD.Send(temp_data.short_buffer, pio_size,
                                       MPI::UNSIGNED_SHORT, section, short_data_tag);
#else
                // In the serial case (with nb_sections > 0), write the data to the appropriate temporary file
                  save_short_data(file_type,section,section_size,temp_data.short_buffer);
#endif
                }
              }  // for (ionb_coloffset = 0; ionb_coloffset < params.ncols; ionb_coloffset += io_ncols)
            } // for (ionb_rowoffset = 0; ionb_rowoffset < params.nrows; ionb_rowoffset += io_nrows)
#ifdef THREEDIM
          } // for (slice = 0; slice < io_nslices; ++slice)
        } // for (ionb_sliceoffset = 0; ionb_sliceoffset < params.nslices; ionb_sliceoffset += io_nslices)
#endif
      } // for (band = 0; band < number_of_bands; ++band)
#ifdef PARALLEL
    } // if (params.myid == 0)
    else
    {
   // In the parallel case, for the worker processes (myid != 0), receive the data and place it
   // in the proper place in short_buffer
#ifdef GDAL
      unsigned int index2;
#endif
      for (band = 0; band < number_of_bands; ++band)
      {
#ifdef THREEDIM
        for (slice = 0; slice < io_nslices; ++slice)
        {
#endif
          MPI::Status status;
          MPI::COMM_WORLD.Recv(temp_data.short_buffer, pio_size,
                               MPI::UNSIGNED_SHORT, 0, short_data_tag, status);
            for (row = 0; row < io_nrows; row++)
              for (col = 0; col < io_ncols; col++)
              {
#ifdef THREEDIM
                index = col + row*io_ncols + slice*io_nrows*io_ncols + band*io_nslices*io_nrows*io_ncols;
                index2 = col + row*io_ncols;
#else
                index = col + row*io_ncols + band*io_nrows*io_ncols;
                index2 = col + row*io_ncols;
#endif
                short_buffer[index] = temp_data.short_buffer[index2];
              }
#ifdef THREEDIM
        } // for (slice = 0; slice < io_nslices; ++slice)
#endif
      } // for (band = 0; band < number_of_bands; ++band)
    }
#endif // PARALLEL
#ifndef PARALLEL
  // Still need to store the section 0 data into the appropriate temporary file in the serial case
  // with nb_sections > 1.
    if (params.nb_sections > 1)
      save_short_data(file_type,0,section_size,short_buffer);
#endif
#ifndef GDAL
#ifdef PARALLEL
    if (params.myid == 0)
#endif
      if (short_file_name != NULL)
        data_file.close();
    delete [] io_short_buffer;
#endif
#ifdef DEBUG
    if (params.debug > 3)
    {
      params.log_fs << "short_buffer (read_short):" << endl << endl;
#ifndef PARALLEL
      for (section = 0; section < params.nb_sections; ++section)
      {
        if (params.nb_sections > 1)
          restore_short_data(file_type,section,section_size,short_buffer);
#endif
        for (band = 0; band < number_of_bands; band++)
        {
#ifdef THREEDIM
          for (slice = 0; slice < io_nslices; slice++)
          {
#endif
            for (row = 0; row < io_nrows; row++)
            {
              for (col = 0; col < io_ncols; col++)
              {
#ifdef THREEDIM
                index = col + row*io_ncols + slice*io_nrows*io_ncols + band*io_nslices*io_nrows*io_ncols;
#else
                index = col + row*io_ncols + band*io_nrows*io_ncols;
#endif
                params.log_fs << short_buffer[index] << " ";
              }
              params.log_fs << endl;
            }
            params.log_fs << endl;
#ifdef THREEDIM
          }
          params.log_fs << endl;
#endif
        }
#ifndef PARALLEL
      }
#endif
    }
#endif
  }

#ifdef GDAL
#ifdef THREEDIM
  void Spatial::read_float(const int& io_ncols, const int& io_nrows, const int& io_nslices,
                           const int& number_of_bands, Image& floatImage,
                           const string& file_type, float *float_buffer,
                           Temp& temp_data)
#else
  void Spatial::read_float(const int& io_ncols, const int& io_nrows,
                           const int& number_of_bands, Image& floatImage,
                           const string& file_type, float *float_buffer,
                           Temp& temp_data)
#endif
#else // GDAL
#ifdef THREEDIM
  void Spatial::read_float(const int& io_ncols, const int& io_nrows, const int& io_nslices,
                           const int& number_of_bands, const char *float_file_name,
                           const string& file_type, float *float_buffer,
                           Temp& temp_data)
#else
  void Spatial::read_float(const int& io_ncols, const int& io_nrows,
                           const int& number_of_bands, const char *float_file_name,
                           const string& file_type, float *float_buffer,
                           Temp& temp_data)
#endif
#endif // GDAL
  {
// Reads the specified float data from disk.
// For the parallel case, the data is sent (and received) in the appropriate parallel task.
// For the serial case with nb_sections > 1, the data written to the appropriate temporary file.
#ifndef GDAL
    float *io_float_buffer = NULL;
#endif
    short unsigned int band, section;
    int col, row;
#ifndef GDAL
    unsigned int index2, io_size;
#endif
    unsigned int index, section_size;
    int ionb_rowoffset, ionb_coloffset;
    int max_ncols, max_nrows;
#ifdef THREEDIM
    int slice, ionb_sliceoffset, max_nslices;
#endif

#ifndef GDAL
 // "io_size" is the number of data elements read or written at one time to or from disk
    fstream data_file;
    io_size = params.ncols*io_nrows;
 // "io_float_buffer" is the buffer used for i/o to and from the disk
    io_float_buffer = new float[io_size];
#endif
 // "section_size" is the number of data elements in each processing section
#ifdef THREEDIM
    section_size = io_ncols*io_nrows*io_nslices*number_of_bands;
#else
    section_size = io_ncols*io_nrows*number_of_bands;
#endif
#ifdef PARALLEL
 // "pio_size" is the number of data elements in each parallel data transfer
    unsigned int pio_size;
    pio_size = io_ncols*io_nrows;
    check_buf_size(0,0,0,pio_size,0,temp_data);
#else
    check_buf_size(0,0,0,section_size,0,temp_data);
#endif

#ifdef PARALLEL
    int float_data_tag = 32;
    if (params.myid == 0)
    {
// In the parallel case, the task at recur_level = 0 (task 0) reads in the data
// and parcels it out the other tasks.
// The data for current task is also retained in the float_buffer.
#else
// In the serial case with params.nb_sections > 1, the data is read in and
// written out to temporary files corresponding to the processing sections.
#endif
#ifndef GDAL
      data_file.open(float_file_name, ios_base::in | ios_base::binary );
      if (!data_file)
      {
        if (params.debug > 0)
          params.log_fs << "ERROR:  Failed to open float data file: " << float_file_name << endl;
        else
          cout << "ERROR:  Failed to open float data file: " << float_file_name << endl;
        return;
      }
#endif // GDAL

      for (band = 0; band < number_of_bands; ++band)
      {
#ifdef THREEDIM
        for (ionb_sliceoffset = 0;
             ionb_sliceoffset < params.nslices;
             ionb_sliceoffset += io_nslices)
        {
          max_nslices = params.nslices - ionb_sliceoffset;
          if (max_nslices > io_nslices)
            max_nslices = io_nslices;
          for (slice = 0; slice < io_nslices; slice++)
          {
#endif
            for (ionb_rowoffset = 0;
                 ionb_rowoffset < params.nrows;
                 ionb_rowoffset += io_nrows)
            {
              max_nrows = params.nrows - ionb_rowoffset;
              if (max_nrows > io_nrows)
                max_nrows = io_nrows;
#ifndef GDAL
#ifdef THREEDIM
              if (slice < max_nslices)
              {
#endif
               // The spectral bands are read in separate reads from disk
               // ncols by max_nrows of data are read in each read from disk
                data_file.read(reinterpret_cast<char *>(io_float_buffer),4*params.ncols*max_nrows);
#ifdef THREEDIM
              }  // if (slice < max_nslices)
#endif
#endif // GDAL
              for (ionb_coloffset = 0;
                   ionb_coloffset < params.ncols;
                   ionb_coloffset += io_ncols)
              {
                max_ncols = params.ncols - ionb_coloffset;
                if (max_ncols > io_ncols)
                  max_ncols = io_ncols;
              // Determine which section of data is currently to be dealt with
#ifdef THREEDIM
                section = find_section(ionb_coloffset,ionb_rowoffset,ionb_sliceoffset);
#else
                section = find_section(ionb_coloffset,ionb_rowoffset);
#endif
#ifndef PARALLEL
              // If this is not the first time this section is visited, restore to memory the result of the previous visit
#ifdef THREEDIM
                if (((band != 0) || (slice != 0)) && (section > 0))
                {
                  restore_float_data(file_type,section,section_size,temp_data.float_buffer);
                }
#else // THREEDIM
                if ((band != 0) && (section > 0))
                {
                  restore_float_data(file_type,section,section_size,temp_data.float_buffer);
                }
#endif // THREEDIM
#endif // PARALLEL
                for (row = 0; row < io_nrows; row++)
                  for (col = 0; col < io_ncols; col++)
                  {
#ifndef GDAL
                    index2 = col + ionb_coloffset + row*params.ncols;
#endif
                    if (section == 0)
                    {
                    // For section == 0, place (properly masked) data input the float_buffer
#ifdef THREEDIM
                      index = col + row*io_ncols + slice*io_ncols*io_nrows + band*io_ncols*io_nrows*io_nslices;
                      if ((slice < max_nslices) && (row < max_nrows) && (col < max_ncols))
#else
                      index = col + row*io_ncols + band*io_ncols*io_nrows;
                      if ((row < max_nrows) && (col < max_ncols))
#endif
#ifdef GDAL
#ifdef THREEDIM
                          float_buffer[index] = 
                            (float) floatImage.get_data(col+ionb_coloffset,row+ionb_rowoffset,slice+ionb_sliceoffset,band);
#else
                          float_buffer[index] = 
                            (float) floatImage.get_data(col+ionb_coloffset,row+ionb_rowoffset,band);
#endif
#else // GDAL
                        float_buffer[index] = io_float_buffer[index2];
#endif // GDAL
                      else
                        float_buffer[index] = 0;
                    }
                    else
                    {
                    // For section != 0, place (properly masked) data input temp_data.float_buffer
#ifdef PARALLEL
                      index = col + row*io_ncols;
#else // PARALLEL
#ifdef THREEDIM
                      index = col + row*io_ncols + slice*io_ncols*io_nrows + band*io_ncols*io_nrows*io_nslices;
#else // THREEDIM
                      index = col + row*io_ncols + band*io_ncols*io_nrows;
#endif // THREEDIM
#endif // PARALLEL
#ifdef THREEDIM
                      if ((slice < max_nslices) && (row < max_nrows) && (col < max_ncols))
#else
                      if ((row < max_nrows) && (col < max_ncols))
#endif
#ifdef GDAL
#ifdef THREEDIM
                          temp_data.float_buffer[index] = 
                            (float) floatImage.get_data(col+ionb_coloffset,row+ionb_rowoffset,slice+ionb_sliceoffset,band);
#else
                          temp_data.float_buffer[index] = 
                            (float) floatImage.get_data(col+ionb_coloffset,row+ionb_rowoffset,band);
#endif
#else // GDAL
                        temp_data.float_buffer[index] = io_float_buffer[index2];
#endif // GDAL
                      else
                        temp_data.float_buffer[index] = 0;
                    }
                  }
                if (section > 0)
                {
#ifdef PARALLEL
                // In the parallel case, send the data to the appropriate processor (from the master process, myid == 0)
                  MPI::COMM_WORLD.Send(temp_data.float_buffer, pio_size,
                                       MPI::FLOAT, section, float_data_tag);
#else
                // In the serial case (with nb_sections > 0), write the data to the appropriate temporary file
                  save_float_data(file_type,section,section_size,temp_data.float_buffer);
#endif
                }
              }  // for (ionb_coloffset = 0; ionb_coloffset < params.ncols; ionb_coloffset += io_ncols)
            } // for (ionb_rowoffset = 0; ionb_rowoffset < params.nrows; ionb_rowoffset += io_nrows)
#ifdef THREEDIM
          } // for (slice = 0; slice < io_nslices; ++slice)
        } // for (ionb_sliceoffset = 0; ionb_sliceoffset < params.nslices; ionb_sliceoffset += io_nslices)
#endif
      } // for (band = 0; band < number_of_bands; ++band)
#ifdef PARALLEL
    } // if (params.myid == 0)
    else
    {
   // In the parallel case, for the worker processes (myid != 0), receive the data and place it
   // in the proper place in float_buffer
#ifdef GDAL
      unsigned int index2;
#endif
      for (band = 0; band < number_of_bands; ++band)
      {
#ifdef THREEDIM
        for (slice = 0; slice < io_nslices; ++slice)
        {
#endif
          MPI::Status status;
          MPI::COMM_WORLD.Recv(temp_data.float_buffer, pio_size,
                               MPI::FLOAT, 0, float_data_tag, status);
            for (row = 0; row < io_nrows; row++)
              for (col = 0; col < io_ncols; col++)
              {
#ifdef THREEDIM
                index = col + row*io_ncols + slice*io_nrows*io_ncols + band*io_nslices*io_nrows*io_ncols;
                index2 = col + row*io_ncols;
#else
                index = col + row*io_ncols + band*io_nrows*io_ncols;
                index2 = col + row*io_ncols;
#endif
                float_buffer[index] = temp_data.float_buffer[index2];
              }
#ifdef THREEDIM
        } // for (slice = 0; slice < io_nslices; ++slice)
#endif
      } // for (band = 0; band < number_of_bands; ++band)
    }
#endif // PARALLEL
#ifndef PARALLEL
  // Still need to store the section 0 data into the appropriate temporary file in the serial case
  // with nb_sections > 1.
    if (params.nb_sections > 1)
      save_float_data(file_type,0,section_size,float_buffer);
#endif
#ifndef GDAL
#ifdef PARALLEL
    if (params.myid == 0)
#endif
      if (float_file_name != NULL)
        data_file.close();
    delete [] io_float_buffer;
#endif
#ifdef DEBUG
    if (params.debug > 3)
    {
      params.log_fs << "float_buffer (read_float):" << endl << endl;
#ifndef PARALLEL
      for (section = 0; section < params.nb_sections; ++section)
      {
        if (params.nb_sections > 1)
          restore_float_data(file_type,section,section_size,float_buffer);
#endif
        for (band = 0; band < number_of_bands; band++)
        {
#ifdef THREEDIM
          for (slice = 0; slice < io_nslices; slice++)
          {
#endif
            for (row = 0; row < io_nrows; row++)
            {
              for (col = 0; col < io_ncols; col++)
              {
#ifdef THREEDIM
                index = col + row*io_ncols + slice*io_nrows*io_ncols + band*io_nslices*io_nrows*io_ncols;
#else
                index = col + row*io_ncols + band*io_nrows*io_ncols;
#endif
                params.log_fs << float_buffer[index] << " ";
              }
              params.log_fs << endl;
            }
            params.log_fs << endl;
#ifdef THREEDIM
          }
          params.log_fs << endl;
#endif
        }
#ifndef PARALLEL
      }
#endif
    }
#endif
  }

#ifdef GDAL
#ifdef THREEDIM
  void Spatial::read_rlblmap(const int& io_ncols, const int& io_nrows, const int& io_nslices,
                             Image& rlblmapImage, const string& file_type, 
                             unsigned int *rlblmap_buffer, Temp& temp_data)
#else
  void Spatial::read_rlblmap(const int& io_ncols, const int& io_nrows, Image& rlblmapImage,
                             const string& file_type, unsigned int *rlblmap_buffer,
                             Temp& temp_data)
#endif
#else // GDAL
#ifdef THREEDIM
  void Spatial::read_rlblmap(const int& io_ncols, const int& io_nrows, const int& io_nslices,
                             const char *rlblmap_file_name, const string& file_type,
                             unsigned int *rlblmap_buffer, Temp& temp_data)
#else
  void Spatial::read_rlblmap(const int& io_ncols, const int& io_nrows, const char *rlblmap_file_name,
                             const string& file_type, unsigned int *rlblmap_buffer,
                             Temp& temp_data)
#endif
#endif // GDAL
  {
// Reads the specified unsigned short region label map data from disk.
// For the parallel case, the data is sent (and received) in the appropriate parallel task.
// For the serial case with nb_sections > 1, the data written to the appropriate temporary file.
#ifndef GDAL
    short unsigned int *io_short_buffer = NULL;
#endif
    short unsigned int section;
    int col, row;
#ifndef GDAL
    unsigned int index2, io_size;
#endif
    unsigned int index, section_size;
    int ionb_rowoffset, ionb_coloffset;
    int max_ncols, max_nrows;
#ifdef THREEDIM
    int slice, ionb_sliceoffset, max_nslices;
#endif

#ifndef GDAL
 // "io_size" is the number of data elements read or written at one time to or from disk
    fstream data_file;
    io_size = params.ncols*io_nrows;
 // "io_short_buffer" is the buffer used for i/o to and from the disk
    io_short_buffer = new short unsigned int[io_size];
#endif
 // "section_size" is the number of data elements in each processing section
#ifdef THREEDIM
    section_size = io_ncols*io_nrows*io_nslices;
#else
    section_size = io_ncols*io_nrows;
#endif
#ifdef PARALLEL
 // "pio_size" is the number of data elements in each parallel data transfer
    unsigned int pio_size;
    pio_size = io_ncols*io_nrows;
    check_buf_size(0,0,pio_size,0,0,temp_data);
#else
    check_buf_size(0,0,section_size,0,0,temp_data);
#endif

#ifdef PARALLEL
    int int_data_tag = 24;
    if (params.myid == 0)
    {
// In the parallel case, the task at recur_level = 0 (task 0) reads in the data
// and parcels it out the other tasks.
// The data for current task is also retained in the short_buffer.
#else
// In the serial case with params.nb_sections > 1, the data is read in and
// written out to temporary files corresponding to the processing sections.
#endif
#ifndef GDAL
      data_file.open(rlblmap_file_name, ios_base::in | ios_base::binary );
      if (!data_file)
      {
        if (params.debug > 0)
          params.log_fs << "ERROR:  Failed to open short data file: " << rlblmap_file_name << endl;
        else
          cout << "ERROR:  Failed to open short data file: " << rlblmap_file_name << endl;
        return;
      }
#endif // GDAL

#ifdef THREEDIM
      for (ionb_sliceoffset = 0;
           ionb_sliceoffset < params.nslices;
           ionb_sliceoffset += io_nslices)
      {
          max_nslices = params.nslices - ionb_sliceoffset;
          if (max_nslices > io_nslices)
            max_nslices = io_nslices;
          for (slice = 0; slice < io_nslices; slice++)
          {
#endif
            for (ionb_rowoffset = 0;
                 ionb_rowoffset < params.nrows;
                 ionb_rowoffset += io_nrows)
            {
              max_nrows = params.nrows - ionb_rowoffset;
              if (max_nrows > io_nrows)
                max_nrows = io_nrows;
#ifndef GDAL
#ifdef THREEDIM
              if (slice < max_nslices)
              {
               // The image slices are read in separate reads from disk
               // ncols by max_nrows of data are read in each read from disk
#endif
                data_file.read(reinterpret_cast<char *>(io_short_buffer),2*params.ncols*max_nrows);
#ifdef THREEDIM
              }  // if (slice < max_nslices)
#endif
#endif // GDAL
              for (ionb_coloffset = 0;
                   ionb_coloffset < params.ncols;
                   ionb_coloffset += io_ncols)
              {
                max_ncols = params.ncols - ionb_coloffset;
                if (max_ncols > io_ncols)
                  max_ncols = io_ncols;
              // Determine which section of data is currently to be dealt with
#ifdef THREEDIM
                section = find_section(ionb_coloffset,ionb_rowoffset,ionb_sliceoffset);
#else
                section = find_section(ionb_coloffset,ionb_rowoffset);
#endif
#ifndef PARALLEL
              // If this is not the first time this section is visited, restore to memory the result of the previous visit
#ifdef THREEDIM
                if (((slice != 0)) && (section > 0))
                {
                  restore_int_data(file_type,section,section_size,temp_data.int_buffer);
                }
#else // THREEDIM
                if (section > 0)
                {
                  restore_int_data(file_type,section,section_size,temp_data.int_buffer);
                }
#endif // THREEDIM
#endif // PARALLEL
                for (row = 0; row < io_nrows; row++)
                  for (col = 0; col < io_ncols; col++)
                  {
#ifndef GDAL
                    index2 = col + ionb_coloffset + row*params.ncols;
#endif
                    if (section == 0)
                    {
                    // For section == 0, place (properly masked) data input the rlblmap_buffer
#ifdef THREEDIM
                      index = col + row*io_ncols + slice*io_ncols*io_nrows;
                      if ((slice < max_nslices) && (row < max_nrows) && (col < max_ncols))
#else
                      index = col + row*io_ncols;
                      if ((row < max_nrows) && (col < max_ncols))
#endif
#ifdef GDAL
#ifdef THREEDIM
                        rlblmap_buffer[index] = 
                          (unsigned int) rlblmapImage.get_data(col+ionb_coloffset,row+ionb_rowoffset,slice+ionb_sliceoffset,0);
#else
                        rlblmap_buffer[index] = 
                          (unsigned int) rlblmapImage.get_data(col+ionb_coloffset,row+ionb_rowoffset,0);
#endif
#else // GDAL
                        rlblmap_buffer[index] = (unsigned int) io_short_buffer[index2];
#endif
                      else
                        rlblmap_buffer[index] = 0;
                    }
                    else
                    {
                    // For section != 0, place (properly masked) data input temp_data.int_buffer
#ifdef PARALLEL
                      index = col + row*io_ncols;
#else // PARALLEL
#ifdef THREEDIM
                      index = col + row*io_ncols + slice*io_ncols*io_nrows;
#else // THREEDIM
                      index = col + row*io_ncols;
#endif // THREEDIM
#endif // PARALLEL
#ifdef THREEDIM
                      if ((slice < max_nslices) && (row < max_nrows) && (col < max_ncols))
#else
                      if ((row < max_nrows) && (col < max_ncols))
#endif
#ifdef GDAL
#ifdef THREEDIM
                          temp_data.int_buffer[index] = 
                            (unsigned int) rlblmapImage.get_data(col+ionb_coloffset,row+ionb_rowoffset,slice+ionb_sliceoffset,0);
#else
                          temp_data.int_buffer[index] = 
                            (unsigned int) rlblmapImage.get_data(col+ionb_coloffset,row+ionb_rowoffset,0);
#endif
#else // GDAL
                        temp_data.int_buffer[index] = (unsigned int) io_short_buffer[index2];
#endif // GDAL
                      else
                        temp_data.int_buffer[index] = 0;
                    }
                  }
                if (section > 0)
                {
#ifdef PARALLEL
                // In the parallel case, send the data to the appropriate processor (from the master process, myid == 0)
                  MPI::COMM_WORLD.Send(temp_data.int_buffer, pio_size,
                                       MPI::UNSIGNED, section, int_data_tag);
#else
                // In the serial case (with nb_sections > 0), write the data to the appropriate temporary file
                  save_int_data(file_type,section,section_size,temp_data.int_buffer);
#endif
                }
              }  // for (ionb_coloffset = 0; ionb_coloffset < params.ncols; ionb_coloffset += io_ncols)
            } // for (ionb_rowoffset = 0; ionb_rowoffset < params.nrows; ionb_rowoffset += io_nrows)
#ifdef THREEDIM
          } // for (slice = 0; slice < io_nslices; ++slice)
      } // for (ionb_sliceoffset = 0; ionb_sliceoffset < params.nslices; ionb_sliceoffset += io_nslices)
#endif
#ifdef PARALLEL
    } // if (params.myid == 0)
    else
    {
   // In the parallel case, for the worker processes (myid != 0), receive the data and place it
   // in the proper place in rlblmap_buffer
#ifdef GDAL
      unsigned int index2;
#endif
#ifdef THREEDIM
      for (slice = 0; slice < io_nslices; ++slice)
      {
#endif
          MPI::Status status;
          MPI::COMM_WORLD.Recv(temp_data.int_buffer, pio_size,
                               MPI::UNSIGNED, 0, int_data_tag, status);
            for (row = 0; row < io_nrows; row++)
              for (col = 0; col < io_ncols; col++)
              {
#ifdef THREEDIM
                index = col + row*io_ncols + slice*io_nrows*io_ncols;
                index2 = col + row*io_ncols;
#else
                index = col + row*io_ncols;
                index2 = col + row*io_ncols;
#endif
                rlblmap_buffer[index] = temp_data.int_buffer[index2];
              }
#ifdef THREEDIM
      } // for (slice = 0; slice < io_nslices; ++slice)
#endif
    }
#endif // PARALLEL
#ifndef PARALLEL
  // Still need to store the section 0 data into the appropriate temporary file in the serial case
  // with nb_sections > 1.
    if (params.nb_sections > 1)
      save_int_data(file_type,0,section_size,rlblmap_buffer);
#endif
#ifndef GDAL
#ifdef PARALLEL
    if (params.myid == 0)
#endif
      if (rlblmap_file_name != NULL)
        data_file.close();
    delete [] io_short_buffer;
#endif
#ifdef DEBUG
    if (params.debug > 3)
    {
      params.log_fs << "rlblmap_buffer (read_rlblmap):" << endl << endl;
#ifndef PARALLEL
      for (section = 0; section < params.nb_sections; ++section)
      {
        if (params.nb_sections > 1)
          restore_int_data(file_type,section,section_size,rlblmap_buffer);
        params.log_fs << "For section " << section << ", rlblmap_buffer:" << endl;
#endif
#ifdef THREEDIM
        for (slice = 0; slice < io_nslices; slice++)
        {
#endif
            for (row = 0; row < io_nrows; row++)
            {
              for (col = 0; col < io_ncols; col++)
              {
#ifdef THREEDIM
                index = col + row*io_ncols + slice*io_nrows*io_ncols;
#else
                index = col + row*io_ncols;
#endif
                params.log_fs << rlblmap_buffer[index] << " ";
              }
              params.log_fs << endl;
            }
            params.log_fs << endl;
#ifdef THREEDIM
        }
#endif
#ifndef PARALLEL
      }
#endif
    }
#endif
  }

#ifndef PARALLEL
  void Spatial::restore_input_data(const short unsigned int& section,
                                   bool *mask, unsigned char *byte_input_data,
                                   short unsigned int *short_input_data, float *float_input_data)
  {
// Restores the input_data (and mask) from the appropriate temporary file
   unsigned int index;
#ifdef THREEDIM
   unsigned int nelements = params.ionb_ncols*params.ionb_nrows*params.ionb_nslices;
#else
   unsigned int nelements = params.ionb_ncols*params.ionb_nrows;
#endif

   if (mask != NULL)
   {
     restore_byte_data("mask",section,nelements,byte_input_data);
     for (index = 0; index < nelements; ++index)
       mask[index] = (byte_input_data[index] != params.mask_value);
   }
   nelements *= params.nbands;
   if (params.dtype == UInt8)
     restore_byte_data("input_image",section,nelements,byte_input_data);
   else if (params.dtype == UInt16)
     restore_short_data("input_image",section,nelements,short_input_data);
   else
     restore_float_data("input_image",section,nelements,float_input_data);

   return;
  }

  void Spatial::restore_std_dev_data(const short unsigned int& section, bool *std_dev_mask,
                                  unsigned char *byte_input_data, float *float_std_dev_data)
  {
// Restores the std_dev_data from the appropriate temporary file
   unsigned int index;
#ifdef THREEDIM
   unsigned int nelements = params.ionb_ncols*params.ionb_nrows*params.ionb_nslices;
#else
   unsigned int nelements = params.ionb_ncols*params.ionb_nrows;
#endif

   if (std_dev_mask != NULL)
   {
     restore_byte_data("std_dev_mask",section,nelements,byte_input_data);
     for (index = 0; index < nelements; ++index)
       std_dev_mask[index] = (byte_input_data[index] != params.mask_value);
   }
   nelements *= params.nbands;
   restore_float_data("std_dev_image",section,nelements,float_std_dev_data);

   return;
  }

  void Spatial::restore_edge_data(const short unsigned int& section, bool *edge_mask,
                                  unsigned char *byte_input_data, float *float_edge_data)
  {
// Restores the edge_data from the appropriate temporary file
   unsigned int index;
#ifdef THREEDIM
   unsigned int nelements = params.ionb_ncols*params.ionb_nrows*params.ionb_nslices;
#else
   unsigned int nelements = params.ionb_ncols*params.ionb_nrows;
#endif

   if (edge_mask != NULL)
   {
     restore_byte_data("edge_mask",section,nelements,byte_input_data);
     for (index = 0; index < nelements; ++index)
       edge_mask[index] = (byte_input_data[index] != params.mask_value);
   }
   restore_float_data("edge_image",section,nelements,float_edge_data);

   return;
  }

  void Spatial::save_region_class_label_map(const short unsigned int& section)
  {
// Saves the region_class_label_map to the appropriate temporary file
#ifdef THREEDIM
   unsigned int nelements = params.onb_ncols*params.onb_nrows*params.onb_nslices;
#else
   unsigned int nelements = params.onb_ncols*params.onb_nrows;
#endif

   save_int_data("class_labels_map",section,nelements,region_class_label_map);

   return;
  }

  void Spatial::restore_region_class_label_map(const short unsigned int& section)
  {
// Restores the region_class_label_map from the appropriate temporary file
#ifdef THREEDIM
   unsigned int nelements = params.onb_ncols*params.onb_nrows*params.onb_nslices;
#else
   unsigned int nelements = params.onb_ncols*params.onb_nrows;
#endif

   restore_int_data("class_labels_map",section,nelements,region_class_label_map);

   return;
  }

  void Spatial::save_boundary_map(const short unsigned int& section)
  {
// Saves the boundary_map to the appropriate temporary file
#ifdef THREEDIM
   unsigned int nelements = params.onb_ncols*params.onb_nrows*params.onb_nslices;
#else
   unsigned int nelements = params.onb_ncols*params.onb_nrows;
#endif

   save_short_data("boundary_map",section,nelements,boundary_map);

   return;
  }

  void Spatial::restore_boundary_map(const short unsigned int& section)
  {
// Restores the boundary_map from the appropriate temporary file
#ifdef THREEDIM
   unsigned int nelements = params.onb_ncols*params.onb_nrows*params.onb_nslices;
#else
   unsigned int nelements = params.onb_ncols*params.onb_nrows;
#endif

   restore_short_data("boundary_map",section,nelements,boundary_map);

   return;
  }

  void Spatial::save_region_object_label_map(const short unsigned int& section)
  {
// Saves the region_object_label_map to the appropriate temporary file
#ifdef THREEDIM
   unsigned int nelements = params.onb_ncols*params.onb_nrows*params.onb_nslices;
#else
   unsigned int nelements = params.onb_ncols*params.onb_nrows;
#endif

   save_int_data("object_labels_map",section,nelements,region_object_label_map);

   return;
  }

  void Spatial::restore_region_object_label_map(const short unsigned int& section)
  {
// Restores the region_object_label_map from the appropriate temporary file
#ifdef THREEDIM
   unsigned int nelements = params.onb_ncols*params.onb_nrows*params.onb_nslices;
#else
   unsigned int nelements = params.onb_ncols*params.onb_nrows;
#endif

   restore_int_data("object_labels_map",section,nelements,region_object_label_map);

   return;
  }
#endif // !PARALLEL
#endif // RHSEG_RUN
 
  void Spatial::update_region_label(vector<Pixel>& pixel_data)
  {
// Updates the (pixel_data) region_label from the region_object_label_map after
// performing connected component labeling
   unsigned int pixel_index, pixel_data_size = pixel_data.size();
   for (pixel_index = 0; pixel_index < pixel_data_size; ++pixel_index)
   {
     if (pixel_data[pixel_index].mask)
       pixel_data[pixel_index].region_label = region_object_label_map[pixel_index];
     else
       pixel_data[pixel_index].region_label = 0;
     region_object_label_map[pixel_index] = 0;
   }
   return;
  }

  void Spatial::update_region_label_map(vector<Pixel>& pixel_data)
  {
// Updates the region_class_label_map from the (pixel_data) region_label
// for a particular section of data
   unsigned int pixel_index, pixel_data_size = pixel_data.size();

   for (pixel_index = 0; pixel_index < pixel_data_size; ++pixel_index)
   {
     if (pixel_data[pixel_index].mask)
       region_class_label_map[pixel_index] = pixel_data[pixel_index].region_label;
     else
       region_class_label_map[pixel_index] = 0;
   }
   return;
  }

#ifdef RHSEG_RUN
  void Spatial::update_region_label_map(const short unsigned int& recur_level, const short unsigned int& section,
                                        vector<Pixel>& pixel_data,Temp& temp_data)
  {
// Updates the region_class_label_map from the (pixel_data) region_label
// for all data sections (in parallel, for all processors)
   if (params.debug > 3)
   {
     params.log_fs << "Performing update_region_label_map at recur_level = " << recur_level;
     params.log_fs << " for section " << section << endl;
   }

   if (recur_level >= (params.onb_levels-1))
   {
     this->update_region_label_map(pixel_data);
   }
   else
   {
     int stride, nb_sections;
     set_stride_sections(recur_level,stride,nb_sections);
#ifdef PARALLEL
   // Send request to dependent parallel tasks.
#ifdef THREEDIM
     parallel_recur_requests((short unsigned int) 7,recur_level,0,0,0,0,0,0,0,temp_data);
#else
     parallel_recur_requests((short unsigned int) 7,recur_level,0,0,0,0,0,0,temp_data);
#endif
   // Update current task or section
     this->update_region_label_map((recur_level+1),section,pixel_data,temp_data);

     unsigned int pixel_index;
     int update_region_label_map_tag = 107;
     int min_section = section + stride;
#else
     int min_section = section;
#endif
     int max_section = section + nb_sections;
     for (int recur_section = min_section; recur_section < max_section; recur_section += stride)
     {
#ifdef PARALLEL
#ifdef TIME_IT
       float end_time, elapsed_time;
       end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
       elapsed_time = end_time - temp_data.start_time;
       if (elapsed_time > 0.0) temp_data.compute += elapsed_time;
       temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
    // Receive confirmation of completion from dependent parallel task
       if (params.debug > 3)
         params.log_fs << "Waiting for confirmation of region label map update from task " << recur_section << endl;
       MPI::COMM_WORLD.Recv(&pixel_index, 1, MPI::UNSIGNED, recur_section, update_region_label_map_tag);
#ifdef TIME_IT
       end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
       elapsed_time = end_time - temp_data.start_time;
       if (elapsed_time > 0.0) temp_data.wait += elapsed_time;
       temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
#else
    // Process other sections
       if (((recur_level+1) == (params.ionb_levels-1)) && (params.nb_sections > 1))
         restore_pixel_data(recur_section,pixel_data,temp_data);
       this->update_region_label_map((recur_level+1),recur_section,pixel_data,temp_data);
   // In the serial version for nb_sections > 1, save updated pixel data to temporary files
       if (((recur_level+1) == (params.ionb_levels-1)) && (params.nb_sections > 1))
         save_region_class_label_map(recur_section);
#endif
     }
   }
   return;
  }

  void Spatial::write_region_label_map(Temp& temp_data)
  {
 // Write region_class_label_map and region_object_label_map (if requested) data to specified disk files.
 // For the parallel case, the data is requested (and received) from the appropriate parallel task.
 // For the serial case with nb_sections > 1, the data is read from the appropriate temporary file.
     int row, col;
#ifdef THREEDIM
     int slice;
#ifdef PARALLEL
     int next_slice;
#endif
#endif
     unsigned int section_size, int_buf_size;

#ifndef GDAL
 // "io_size" is the number of data elements read or written at one time to or from disk
     unsigned int io_size;
 // "io_int_buffer" and "io_int_buffer2" are the buffers used for i/o to and from the disk
     unsigned int *io_int_buffer, *io_int_buffer2=NULL;

     io_size = params.ncols*params.onb_nrows;
     io_int_buffer = new unsigned int[io_size];
     if (params.object_labels_map_flag)
       io_int_buffer2 = new unsigned int[io_size];
#endif

 // "section_size" is the number of data elements in each processing section
#ifdef THREEDIM
     section_size = params.onb_ncols*params.onb_nrows*params.onb_nslices;
#else
     section_size = params.onb_ncols*params.onb_nrows;
#endif
#ifdef PARALLEL
 // "pio_size" is the number of data elements in each parallel data transfer
     unsigned int pio_size = params.onb_ncols*params.onb_nrows;
     int_buf_size = pio_size;
#else
     int_buf_size = section_size;
#endif
     if (params.object_labels_map_flag)
       int_buf_size *= 2;
     check_buf_size(0,0,int_buf_size,0,0,temp_data);

     short unsigned int section;
     unsigned int pixel_index;
     int ionb_rowoffset, ionb_coloffset;
#ifdef THREEDIM
     int ionb_sliceoffset;
#endif

#ifdef PARALLEL
     int write_region_label_map_tag = 108;
#ifdef TIME_IT
     float end_time, elapsed_time;
#endif
#endif
#ifdef THREEDIM
     for (ionb_sliceoffset = 0;
          ionb_sliceoffset < params.nslices;
          ionb_sliceoffset += params.onb_nslices)
     {
#endif
       for (ionb_rowoffset = 0;
            ionb_rowoffset < params.nrows;
            ionb_rowoffset += params.onb_nrows)
       {
         for (ionb_coloffset = 0;
              ionb_coloffset < params.ncols;
              ionb_coloffset += params.onb_ncols)
         {
         // Determine which section of data is currently to be dealt with
#ifdef THREEDIM
           section = find_section(ionb_coloffset,ionb_rowoffset,ionb_sliceoffset);
#else
           section = find_section(ionb_coloffset,ionb_rowoffset);
#endif
#ifdef PARALLEL
      // For section != 0, send request for data to dependent parallel tasks
           if (section > 0)
             parallel_request((short unsigned int) 8, section,
                              0, 0, 0, 0, 0, temp_data);
#else // ifndef PARALLEL
           if (params.nb_sections > 1)
           {
      // In the serial case for nb_sections > 1, read data from appropriate temporary file
             this->restore_region_class_label_map(section);
             if (params.object_labels_map_flag)
               this->restore_region_object_label_map(section);
           }

#ifdef THREEDIM
           for (slice = 0; slice < params.onb_nslices; ++slice)
           {
#endif
             for (row = 0; row < params.onb_nrows; ++row)
               for (col = 0; col < params.onb_ncols; ++col)
               {
#ifdef THREEDIM
                 pixel_index = col + row*params.onb_ncols + slice*params.onb_ncols*params.onb_nrows;
                 if ((slice < params.nslices) && (row < params.nrows) && (col < params.ncols))
#else
                 pixel_index = col + row*params.onb_ncols;
                 if ((row < params.nrows) && (col < params.ncols))
#endif
                 {
                   temp_data.int_buffer[pixel_index] = region_class_label_map[pixel_index];
                   if (params.object_labels_map_flag)
                     temp_data.int_buffer[section_size + pixel_index] = region_object_label_map[pixel_index];
                 } // if ((slice < params.nslices) && (row < params.nrows) && (col < params.ncols))
                 else
                 {
                   temp_data.int_buffer[pixel_index] = 0;
                   if (params.object_labels_map_flag)
                     temp_data.int_buffer[section_size + pixel_index] = 0;
                 }
               }
#ifdef THREEDIM
           }
#endif
           if (params.nb_sections > 1)
           {
      // In the serial case for nb_sections > 1, data is save to appropriate temporary file
             save_int_data("int",section,int_buf_size,temp_data.int_buffer);
           }
#endif // #else ifndef PARALLEL
         }
       }
#ifdef THREEDIM
     }
#endif

     int pixel_index2, max_nrows, max_ncols;
#ifdef THREEDIM
     int max_nslices;
     for (ionb_sliceoffset = 0;
          ionb_sliceoffset < params.nslices;
          ionb_sliceoffset += params.onb_nslices)
     {
       max_nslices = params.nslices - ionb_sliceoffset;
       if (max_nslices > params.onb_nslices)
         max_nslices = params.onb_nslices;
       for (slice = 0; slice < max_nslices; ++slice)
       {
#ifdef PARALLEL
         next_slice = slice + 1;
#endif
#endif
         for (ionb_rowoffset = 0;
              ionb_rowoffset < params.nrows;
              ionb_rowoffset += params.onb_nrows)
         {
           max_nrows = params.nrows - ionb_rowoffset;
           if (max_nrows > params.onb_nrows)
             max_nrows = params.onb_nrows;
           for (ionb_coloffset = 0;
                ionb_coloffset < params.ncols;
                ionb_coloffset += params.onb_ncols)
           {
             max_ncols = params.ncols - ionb_coloffset;
             if (max_ncols > params.onb_ncols)
               max_ncols = params.onb_ncols;
              // Determine which section of data is currently to be dealt with
#ifdef THREEDIM
             section = find_section(ionb_coloffset,ionb_rowoffset,ionb_sliceoffset);
#else
             section = find_section(ionb_coloffset,ionb_rowoffset);
#endif
#ifdef PARALLEL
             if (section > 0)
             {
#ifdef TIME_IT
               end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
               elapsed_time = end_time - temp_data.start_time;
               if (elapsed_time > 0.0) temp_data.compute += elapsed_time;
               temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
               MPI::COMM_WORLD.Recv(&int_buf_size, 1, MPI::UNSIGNED, section, write_region_label_map_tag);
               end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
               elapsed_time = end_time - temp_data.start_time;
               if (elapsed_time > 0.0) temp_data.wait += elapsed_time;
               temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
             // Receive data from appropriate dependent parallel task (task 0 already has the section 0 data)
               MPI::COMM_WORLD.Recv(temp_data.int_buffer, int_buf_size, MPI::UNSIGNED, section, write_region_label_map_tag);
#ifdef TIME_IT
               end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
               elapsed_time = end_time - temp_data.start_time;
               if (elapsed_time > 0.0) temp_data.transfer += elapsed_time;
               temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
#ifdef THREEDIM
              // Need to request the next slice of data
               if (next_slice < max_nslices)
                 parallel_request((short unsigned int) 8, section,
                                  next_slice, 0, 0, 0, 0, temp_data);
#endif
             }
#else // #ifndef PARALLEL
             if (params.nb_sections > 1)
             {
             // In the serial case (with nb_sections > 0), restore the data from the appropriate temporary file
               restore_int_data("int",section,int_buf_size,temp_data.int_buffer);
             }
#endif
             for (row = 0; row < max_nrows; ++row)
               for (col = 0; col < max_ncols; ++col)
               {
#ifndef GDAL
                 pixel_index = col + ionb_coloffset + row*params.ncols;
#endif
#ifdef PARALLEL
                 if (section > 0)
                 {
                   pixel_index2 = col + row*params.onb_ncols;
#ifdef GDAL
                   classLabelsMapImage.put_data(temp_data.int_buffer[pixel_index2],(col+ionb_coloffset),(row+ionb_rowoffset),0);
                   if (params.object_labels_map_flag)
                     objectLabelsMapImage.put_data(temp_data.int_buffer[pio_size+pixel_index2],(col+ionb_coloffset),(row+ionb_rowoffset),0);
#else
                   io_int_buffer[pixel_index] = temp_data.int_buffer[pixel_index2];
                   if (params.object_labels_map_flag)
                     io_int_buffer2[pixel_index] = temp_data.int_buffer[pio_size+pixel_index2];
#endif
                 }
                 else
                 {
#ifdef THREEDIM
                   pixel_index2 = col + row*params.onb_ncols + slice*params.onb_ncols*params.onb_nrows;
#else
                   pixel_index2 = col + row*params.onb_ncols;
#endif
#ifdef GDAL
                   classLabelsMapImage.put_data(region_class_label_map[pixel_index2],(col+ionb_coloffset),(row+ionb_rowoffset),0);
                   if (params.object_labels_map_flag)
                     objectLabelsMapImage.put_data(region_object_label_map[pixel_index2],(col+ionb_coloffset),(row+ionb_rowoffset),0);
#else
                   io_int_buffer[pixel_index] = region_class_label_map[pixel_index2];
                   if (params.object_labels_map_flag)
                     io_int_buffer2[pixel_index] = region_object_label_map[pixel_index2];
#endif
                 }
#else // ifndef PARALLEL
               // In serial case just place the data in the appropriate buffer
#ifdef THREEDIM
                 pixel_index2 = col + row*params.onb_ncols + slice*params.onb_ncols*params.onb_nrows;
#else
                 pixel_index2 = col + row*params.onb_ncols;
#endif
#ifdef GDAL
                 classLabelsMapImage.put_data(temp_data.int_buffer[pixel_index2],(col+ionb_coloffset),(row+ionb_rowoffset),0);
                 if (params.object_labels_map_flag)
                   objectLabelsMapImage.put_data(temp_data.int_buffer[section_size+pixel_index2],(col+ionb_coloffset),(row+ionb_rowoffset),0);
#else
                 io_int_buffer[pixel_index] = temp_data.int_buffer[pixel_index2];
                 if (params.object_labels_map_flag)
                   io_int_buffer2[pixel_index] = temp_data.int_buffer[section_size+pixel_index2];
#endif
#endif
               }
           }
#ifdef GDAL
           classLabelsMapImage.flush_data();
           if (params.object_labels_map_flag)
             objectLabelsMapImage.flush_data();
#else
        // Write (current slice) of data
           region_class_label_map_fs.write(reinterpret_cast<char *>(io_int_buffer),4*(params.ncols*max_nrows));
           if (params.object_labels_map_flag)
             region_object_label_map_fs.write(reinterpret_cast<char *>(io_int_buffer2),4*(params.ncols*max_nrows));
#endif
         }
#ifdef THREEDIM
       } // for (slice = 0; slice < max_nslices; ++slice)
     }
#endif
#ifndef GDAL
     delete [] io_int_buffer;
     if (params.object_labels_map_flag)
       delete [] io_int_buffer2;
#endif
     return;
  }
#endif // RHSEG_RUN

  void Spatial::read_region_maps( )
  {
 // Read region_class_label_map and region_object_label_map (if requested) data from specified disk files.
     unsigned int io_size;

 // "io_size" is the number of data elements read or written at one time to or from disk
#ifdef THREEDIM
     io_size = params.ncols*params.nrows*params.nslices;
#else
     io_size = params.ncols*params.nrows;
#endif
     if (region_class_label_map == NULL)
       region_class_label_map = new unsigned int[io_size];
     if ((params.region_objects_list_flag) && (region_object_label_map == NULL))
       region_object_label_map = new unsigned int[io_size];

#ifdef GDAL
     Image classLabelsMapImage;
     Image objectLabelsMapImage;

     classLabelsMapImage.open(params.class_labels_map_file);
     if (params.object_labels_map_flag)
       objectLabelsMapImage.open(params.object_labels_map_file);

     int col,row,index;
     for (row = 0; row < params.nrows; row++)
       for (col = 0; col < params.ncols; col++)
       {
         index = col + row*params.ncols;
         region_class_label_map[index] = classLabelsMapImage.get_data(col,row,0);
         if (params.object_labels_map_flag)
           region_object_label_map[index] = objectLabelsMapImage.get_data(col,row,0);
       }

     classLabelsMapImage.close();
     if (params.object_labels_map_flag)
       objectLabelsMapImage.close();
#else
// Opens the region_class_label_map (and, if requested, the region_object_label_map) for input
     region_class_label_map_fs.open(params.class_labels_map_file.c_str( ), ios_base::in | ios_base::binary );
     region_class_label_map_fs.read(reinterpret_cast<char *>(region_class_label_map),4*io_size);
     region_class_label_map_fs.close();

     if (params.object_labels_map_flag)
     {
       region_object_label_map_fs.open(params.object_labels_map_file.c_str( ), ios_base::in | ios_base::binary );
       region_object_label_map_fs.read(reinterpret_cast<char *>(region_object_label_map),4*io_size);
       region_object_label_map_fs.close();
     }
#endif
     return;
  }

// Update the region_class_label_map and object_labels_map (if requested) for the selected hierarchical segmentation level.
  void Spatial::update_region_maps(const short unsigned int& hseg_level, 
                                 vector<RegionClass>& region_classes, vector<RegionObject>& region_objects)
  {
     unsigned int index, region_label, region_index;
     int row, col;
#ifdef THREEDIM
     int slice;
#endif 

 // Read in the data
     if (hseg_level > 0)
     {
#ifdef THREEDIM
       for (slice = 0; slice < params.nslices; slice++)
       {
#endif
         for (row = 0; row < params.nrows; row++)
         {
           for (col = 0; col < params.ncols; col++)
           {
#ifdef THREEDIM
             index = col + row*params.ncols + slice*params.nrows*params.ncols;
#else
             index = col + row*params.ncols;
#endif
             region_label = region_class_label_map[index];
             if (region_label > 0)
             {
               region_index = region_label - 1;
               if (!region_classes[region_index].get_active_flag())
                 region_label = region_classes[region_index].get_merge_region_label();
             }
             region_class_label_map[index] = region_label;
           }
         }
#ifdef THREEDIM
       }
#endif
     }
     if (params.object_labels_map_flag)
     {
       if (hseg_level > 0)
       {
#ifdef THREEDIM
         for (slice = 0; slice < params.nslices; slice++)
         {
#endif
           for (row = 0; row < params.nrows; row++)
           {
             for (col = 0; col < params.ncols; col++)
             {
#ifdef THREEDIM
               index = col + row*params.ncols + slice*params.nrows*params.ncols;
#else
               index = col + row*params.ncols;
#endif
               region_label = region_object_label_map[index];
               if (region_label > 0)
               {
                 region_index = region_label - 1;
                 if (!region_objects[region_index].get_active_flag())
                  region_label = region_objects[region_index].get_merge_region_label();
               }
               region_object_label_map[index] = region_label;
             }
           }
#ifdef THREEDIM
         }
#endif
       }
     }

     return;
  }

  void Spatial::set_boundary_map(const unsigned int& nslevels,
                                const unsigned int& int_buf_size,
                                Temp& temp_data)
  {
  // Set the boundary_map values from the values contained in the temporary data buffer
    unsigned int int_buf_position;

    for (int_buf_position = 0; int_buf_position < int_buf_size; ++int_buf_position)
    {
      if (temp_data.short_buffer[int_buf_position] == (nslevels+1));
        boundary_map[temp_data.int_buffer[int_buf_position]] = temp_data.short_buffer[int_buf_position];
    }

    return;
  }

#ifdef RHSEG_RUN
  void Spatial::write_boundary_map(Temp& temp_data)
  {
 // Write the boundary_map data to specified disk files.
 // For the parallel case, the data is requested (and received) from the appropriate parallel task.
 // For the serial case with nb_sections > 1, the data is read from the appropriate temporary file.
     short unsigned int section;
     int col, row;
#ifdef THREEDIM
     int slice;
#ifdef PARALLEL
     int next_slice;
#endif
#endif
     unsigned int pixel_index;
#ifdef DEBUG
     if (params.debug > 3)
     {
#ifdef PARALLEL
       section = 0;
#else
       for (section = 0; section < params.nb_sections; section++)
       {
         if (params.nb_sections > 1)
           this->restore_boundary_map(section);
#endif
         params.log_fs << "Boundary_map for section = " << section << ":" << endl;
#ifdef THREEDIM
         for (slice = 0; slice < params.onb_nslices; ++slice)
         {
           params.log_fs << "Boundary_map for slice = " << slice << ":" << endl;
#endif
           for (row = 0; row < params.onb_nrows; ++row)
           {
             for (col = 0; col < params.onb_ncols; ++col)
             {
#ifdef THREEDIM
               pixel_index = col + row*params.onb_ncols + slice*params.onb_ncols*params.onb_nrows;
#else
               pixel_index = col + row*params.onb_ncols;
#endif
               if (boundary_map[pixel_index] < 10)
                 params.log_fs << "0" << boundary_map[pixel_index] << " ";
               else
                 params.log_fs << boundary_map[pixel_index] << " ";
             }
             params.log_fs << endl;
           }
           params.log_fs << endl;
#ifdef THREEDIM
         }
         params.log_fs << endl;
#endif
#ifndef PARALLEL
       } // for (section = 0; section < params.nb_sections; section++)
#endif
     }
#endif

     unsigned int section_size;

#ifndef GDAL
 // "io_size" is the number of data elements read or written at one time to or from disk
 // "io_short_buffer" is the buffer used for i/o to and from the disk
     unsigned int io_size;
     short unsigned int *io_short_buffer;

     io_size = params.ncols*params.onb_nrows;
     io_short_buffer = new short unsigned int[io_size];
#endif

 // "section_size" is the number of data elements in each processing section
#ifdef THREEDIM
     section_size = params.onb_ncols*params.onb_nrows*params.onb_nslices;
#else
     section_size = params.onb_ncols*params.onb_nrows;
#endif
#ifdef PARALLEL
 // "pio_size" is the number of data elements in each parallel data transfer
     unsigned int pio_size = params.onb_ncols*params.onb_nrows;
     check_buf_size(0,pio_size,0,0,0,temp_data);
#else
     check_buf_size(0,section_size,0,0,0,temp_data);
#endif

     int ionb_coloffset, ionb_rowoffset;
#ifdef THREEDIM
     int ionb_sliceoffset;
#endif
#ifdef PARALLEL
     int write_boundary_map_tag = 110;
#ifdef TIME_IT
     float end_time, elapsed_time;
#endif
#endif
#ifdef THREEDIM
     for (ionb_sliceoffset = 0;
          ionb_sliceoffset < params.nslices;
          ionb_sliceoffset += params.onb_nslices)
     {
#endif
       for (ionb_rowoffset = 0;
            ionb_rowoffset < params.nrows;
            ionb_rowoffset += params.onb_nrows)
       {
         for (ionb_coloffset = 0;
              ionb_coloffset < params.ncols;
              ionb_coloffset += params.onb_ncols)
         {
         // Determine which section of data is currently to be dealt with
#ifdef THREEDIM
           section = find_section(ionb_coloffset,ionb_rowoffset,ionb_sliceoffset);
#else
           section = find_section(ionb_coloffset,ionb_rowoffset);
#endif
#ifdef PARALLEL
      // For section != 0, send request for data to dependent parallel tasks
           if (section > 0)
             parallel_request((short unsigned int) 10, section,
                              0, 0, 0, 0, 0, temp_data);
#else // ifndef PARALLEL
      // In the serial case for nb_sections > 1, read data from appropriate temporary file
           if (params.nb_sections > 1)
             this->restore_boundary_map(section);

#ifdef THREEDIM
           for (slice = 0; slice < params.onb_nslices; ++slice)
           {
#endif
             for (row = 0; row < params.onb_nrows; ++row)
               for (col = 0; col < params.onb_ncols; ++col)
               {
#ifdef THREEDIM
                 pixel_index = col + row*params.onb_ncols + slice*params.onb_ncols*params.onb_nrows;
#else
                 pixel_index = col + row*params.onb_ncols;
#endif
                 temp_data.short_buffer[pixel_index] = boundary_map[pixel_index];
               }
#ifdef THREEDIM
            }
#endif
      // In the serial case for nb_sections > 1, data is save to appropriate temporary file
           if (params.nb_sections > 1)
             save_short_data("short",section,section_size,temp_data.short_buffer);
#endif
         }
       }
#ifdef THREEDIM
     }
#endif
     int max_ncols, max_nrows;
#ifdef THREEDIM
     int max_nslices;
#endif
     unsigned pixel_index2;
#ifdef THREEDIM
     for (ionb_sliceoffset = 0;
          ionb_sliceoffset < params.nslices;
          ionb_sliceoffset += params.onb_nslices)
     {
       max_nslices = params.nslices - ionb_sliceoffset;
       if (max_nslices > params.onb_nslices)
         max_nslices = params.onb_nslices;
       for (slice = 0; slice < max_nslices; ++slice)
       {
#ifdef PARALLEL
         next_slice = slice + 1;
#endif
#endif
         for (ionb_rowoffset = 0;
              ionb_rowoffset < params.nrows;
              ionb_rowoffset += params.onb_nrows)
         {
           max_nrows = params.nrows - ionb_rowoffset;
           if (max_nrows > params.onb_nrows)
             max_nrows = params.onb_nrows;
           for (ionb_coloffset = 0;
                ionb_coloffset < params.ncols;
                ionb_coloffset += params.onb_ncols)
           {
             max_ncols = params.ncols - ionb_coloffset;
             if (max_ncols > params.onb_ncols)
               max_ncols = params.onb_ncols;
              // Determine which section of data is currently to be dealt with
#ifdef THREEDIM
             section = find_section(ionb_coloffset,ionb_rowoffset,ionb_sliceoffset);
#else
             section = find_section(ionb_coloffset,ionb_rowoffset);
#endif
#ifdef PARALLEL
             if (section > 0)
             {
#ifdef TIME_IT
               end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
               elapsed_time = end_time - temp_data.start_time;
               if (elapsed_time > 0.0) temp_data.compute += elapsed_time;
               temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
               MPI::COMM_WORLD.Recv(&pixel_index2,1,MPI::UNSIGNED,section,write_boundary_map_tag);
               end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
               elapsed_time = end_time - temp_data.start_time;
               if (elapsed_time > 0.0) temp_data.wait += elapsed_time;
               temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
             // Receive data from appropriate dependent parallel task (task 0 already has the section 0 data)
               MPI::COMM_WORLD.Recv(temp_data.short_buffer,pio_size,MPI::UNSIGNED_SHORT,section,write_boundary_map_tag);
#ifdef TIME_IT
               end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
               elapsed_time = end_time - temp_data.start_time;
               if (elapsed_time > 0.0) temp_data.transfer += elapsed_time;
               temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
#ifdef THREEDIM
              // Need to request the next slice of data
               if (next_slice < max_nslices)
                 parallel_request((short unsigned int) 10, section,
                                  next_slice, 0, 0, 0, 0, temp_data);
#endif
             }
#else // #ifndef PARALLEL
             // In the serial case (with nb_sections > 0), restore the data from the appropriate temporary file
             if (params.nb_sections > 1)
               restore_short_data("short",section,section_size,temp_data.short_buffer);
#endif
             for (row = 0; row < max_nrows; ++row)
             {
               for (col = 0; col < max_ncols; ++col)
               {
#ifndef GDAL
                 pixel_index = col + ionb_coloffset + row*params.ncols;
#endif
#ifdef PARALLEL
                 if (section > 0)
                 {
                   pixel_index2 = col + row*params.onb_ncols;
#ifdef GDAL
                   boundaryMapImage.put_data(temp_data.short_buffer[pixel_index2],(col+ionb_coloffset),(row+ionb_rowoffset),0);
#else
                   io_short_buffer[pixel_index] = temp_data.short_buffer[pixel_index2];
#endif
                 }
                 else
                 {
#ifdef THREEDIM
                   pixel_index2 = col + row*params.onb_ncols + slice*params.onb_ncols*params.onb_nrows;
#else
                   pixel_index2 = col + row*params.onb_ncols;
#endif
#ifdef GDAL
                   boundaryMapImage.put_data(boundary_map[pixel_index2],(col+ionb_coloffset),(row+ionb_rowoffset),0);
#else
                   io_short_buffer[pixel_index] = boundary_map[pixel_index2];
#endif
                 }
#else // #ifndef PARALLEL
               // In serial case just place the data in the appropriate buffer
#ifdef THREEDIM
                 pixel_index2 = col + row*params.onb_ncols + slice*params.onb_ncols*params.onb_nrows;
#else
                 pixel_index2 = col + row*params.onb_ncols;
#endif
#ifdef GDAL
                 boundaryMapImage.put_data(temp_data.short_buffer[pixel_index2],(col+ionb_coloffset),(row+ionb_rowoffset),0);
#else
                 io_short_buffer[pixel_index] = temp_data.short_buffer[pixel_index2];
#endif
#endif
               } // for (col = 0; col < max_ncols; ++col)
             } // for (row = 0; row < max_nrows; ++row)
           } // for (ionb_coloffset = 0; ionb_coloffset < params.ncols; ionb_coloffset += params.onb_ncols)
#ifdef GDAL
           boundaryMapImage.flush_data();
#else
        // Write (current slice) of data
           boundary_map_fs.write(reinterpret_cast<char *>(io_short_buffer),2*(params.ncols*max_nrows));
#endif
         } // for (ionb_rowoffset = 0; ionb_rowoffset < params.nrows; ionb_rowoffset += params.onb_nrows)
#ifdef THREEDIM
       } // for (slice = 0; slice < max_nslices; ++slice)
     } // for (ionb_sliceoffset = 0; ionb_sliceoffset < params.nslices; ionb_sliceoffset += params.onb_nslices)
#endif
#ifndef GDAL
     delete [] io_short_buffer;
#endif
     return;
  }
#endif // RHSEG_RUN

#ifndef GDAL
  void Spatial::open_region_label_map_output()
  {
 // Opens the region_class_label_map (and, if requested, the region_object_label_map) for output
   region_class_label_map_fs.open(params.class_labels_map_file.c_str(), ios_base::out | ios_base::binary );
   if (params.object_labels_map_flag)
     region_object_label_map_fs.open(params.object_labels_map_file.c_str(), ios_base::out | ios_base::binary );
   return;
  }

  void Spatial::close_region_label_map_output()
  {
 // Close the region_class_label_map (and, if requested, the object_labels_map) disk file(s)
    region_class_label_map_fs.close();
    if (params.object_labels_map_flag)
    {
      region_object_label_map_fs.close();
    }
  }

  void Spatial::open_boundary_map_output()
  {
  // Open the boundary_map disk file
    boundary_map_fs.open(params.boundary_map_file.c_str(), ios_base::out | ios_base::binary );
    return;
  }

  void Spatial::close_boundary_map()
  {
  // Close the boundary_map disk file
    boundary_map_fs.close();
    return;
  }
#endif

#ifdef RHSEG_EXTRACT  // THREEDIM stuff needs to be implemented in GDAL or perhaps DICOMM?
  void Spatial::write_class_labels_map_ext(const string& class_labels_map_ext_file, vector<RegionClass>& region_classes)
  {
    int row, col, index;

    Image tempImage;

#ifdef GDAL
    tempImage.create(class_labels_map_ext_file,inputImage,1,GDT_UInt32);
#else
    tempImage.create(class_labels_map_ext_file,params.ncols,params.nrows,1,UInt32);
#endif

//#ifdef THREEDIM
//    int slice;
//
//    for (slice = 0; slice < params.nslices; slice++)
//#endif
      for (row = 0; row < params.nrows; row++)
        for (col = 0; col < params.ncols; col++)
        {
//#ifdef THREEDIM
//          index = col + row*params.ncols + slice*params.nrows*params.ncols;
//#else
          index = col + row*params.ncols;
//#endif
          tempImage.put_data(region_class_label_map[index],col,row,0);
        }

    tempImage.flush_data();
    tempImage.close();

    return;
  }

  void Spatial::write_class_npix_map_ext(const string& class_npix_map_ext_file, vector<RegionClass>& region_classes)
  {
    int row, col, index;
    unsigned int npix, region_index, region_label;

    Image tempImage;

#ifdef GDAL
    tempImage.create(class_npix_map_ext_file,inputImage,1,GDT_UInt32);
#else
    tempImage.create(class_npix_map_ext_file,params.ncols,params.nrows,1,UInt32);
#endif

//#ifdef THREEDIM
//    int slice;
//
//    for (slice = 0; slice < params.nslices; slice++)
//#endif
      for (row = 0; row < params.nrows; row++)
        for (col = 0; col < params.ncols; col++)
        {
//#ifdef THREEDIM
//          index = col + row*params.ncols + slice*params.nrows*params.ncols;
//#else
          index = col + row*params.ncols;
//#endif
          region_label = region_class_label_map[index];
          region_index = region_label - 1;
          if (region_label == 0)
            npix = 0;
          else
            npix = region_classes[region_index].get_npix();
          tempImage.put_data(npix,col,row,0);
        }

    tempImage.flush_data();
    tempImage.close();

    return;
  }

  void Spatial::write_class_mean_map_ext(const string& class_mean_map_ext_file, vector<RegionClass>& region_classes)
  {
    unsigned int region_label, region_index;
    int index, row, col, band;
    float class_mean;

    Image tempImage;

#ifdef GDAL
    tempImage.create(class_mean_map_ext_file,inputImage,params.nbands,GDT_Float32);
#else
    tempImage.create(class_mean_map_ext_file,params.ncols,params.nrows,params.nbands,Float32);
#endif

//#ifdef THREEDIM
//    int slice;
//#endif
    for (band = 0; band < params.nbands; band++)
    {
//#ifdef THREEDIM
//     for (slice = 0; slice < params.nslices; slice++)
//#endif
      for (row = 0; row < params.nrows; row++)
      {
        for (col = 0; col < params.ncols; col++)
        {
//#ifdef THREEDIM
//          index = col + row*params.ncols + slice*params.nrows*params.ncols;
//#else
          index = col + row*params.ncols;
//#endif
          region_label = region_class_label_map[index];
          region_index = region_label - 1;
          if (region_label == 0)
            class_mean = 0.0;
          else
          {
            class_mean = (float) region_classes[region_index].get_unscaled_mean(band);
          }
          tempImage.put_data(class_mean,col,row,band);
        }
      }
    }

    tempImage.flush_data();
    tempImage.close();

    return;
  }

  void Spatial::write_class_std_dev_map_ext(const string& class_std_dev_map_ext_file, vector<RegionClass>& region_classes)
  {
    unsigned int index, region_label, region_index;
    int row, col;
    float std_dev;

    Image tempImage;

#ifdef GDAL
    tempImage.create(class_std_dev_map_ext_file,inputImage,1,GDT_Float32);
#else
    tempImage.create(class_std_dev_map_ext_file,params.ncols,params.nrows,1,Float32);
#endif

//#ifdef THREEDIM
//    int slice;
//
//    for (slice = 0; slice < params.nslices; slice++)
//    {
//#endif
      for (row = 0; row < params.nrows; row++)
      {
        for (col = 0; col < params.ncols; col++)
        {
//#ifdef THREEDIM
//          index = col + row*params.ncols + slice*params.nrows*params.ncols;
//#else
          index = col + row*params.ncols;
//#endif
          region_label = region_class_label_map[index];
          region_index = region_label - 1;
          index = col + row*params.ncols;
          if (region_label == 0)
            std_dev = 0.0;
          else
            std_dev = (float) region_classes[region_index].get_band_max_std_dev();
          tempImage.put_data(std_dev,col,row,0);
        }
      }

    tempImage.flush_data();
    tempImage.close();

    return;
  }

  void Spatial::write_class_bpratio_map_ext(const string& class_bpratio_map_ext_file, vector<RegionClass>& region_classes)
  {
    unsigned int index, region_label, region_index;
    int row, col;
    float npix, boundary_npix;
    float bpratio;

    Image tempImage;

#ifdef GDAL
    tempImage.create(class_bpratio_map_ext_file,inputImage,1,GDT_Float32);
#else
    tempImage.create(class_bpratio_map_ext_file,params.ncols,params.nrows,1,Float32);
#endif

//#ifdef THREEDIM
//    int slice;
//
//    for (slice = 0; slice < params.nslices; slice++)
//#endif
      for (row = 0; row < params.nrows; row++)
      {
        for (col = 0; col < params.ncols; col++)
        {
//#ifdef THREEDIM
//          index = col + row*params.ncols + slice*params.nrows*params.ncols;
//#else
          index = col + row*params.ncols;
//#endif
          region_label = region_class_label_map[index];
          region_index = region_label - 1;
          index = col + row*params.ncols;
          if (region_label == 0)
            bpratio = 0.0;
          else
          {
            npix = region_classes[region_index].get_npix();
            boundary_npix = region_classes[region_index].get_boundary_npix();
            bpratio = boundary_npix/npix;
          }
          tempImage.put_data(bpratio,col,row,0);
        }
      }

    tempImage.flush_data();
    tempImage.close();

    return;
  }

  void Spatial::write_object_labels_map_ext(const string& object_labels_map_ext_file, vector<RegionObject>& region_objects)
  {
    unsigned int index, region_label;
    int row, col;

    Image tempImage;

#ifdef GDAL
    tempImage.create(object_labels_map_ext_file,inputImage,1,GDT_UInt32);
#else
    tempImage.create(object_labels_map_ext_file,params.ncols,params.nrows,1,UInt32);
#endif

//#ifdef THREEDIM
//    int slice;
//
//    for (slice = 0; slice < params.nslices; slice++)
//#endif
      for (row = 0; row < params.nrows; row++)
      {
        for (col = 0; col < params.ncols; col++)
        {
//#ifdef THREEDIM
//          index = col + row*params.ncols + slice*params.nrows*params.ncols;
//#else
          index = col + row*params.ncols;
//#endif
          region_label = region_object_label_map[index];
          tempImage.put_data(region_label,col,row,0);
        }
      }

    tempImage.flush_data();
    tempImage.close();

    return;
  }

  void Spatial::write_object_npix_map_ext(const string& object_npix_map_ext_file, vector<RegionObject>& region_objects)
  {
    unsigned int index, region_label, region_index, npix;
    int row, col;

    Image tempImage;

#ifdef GDAL
    tempImage.create(object_npix_map_ext_file,inputImage,1,GDT_UInt32);
#else
    tempImage.create(object_npix_map_ext_file,params.ncols,params.nrows,1,UInt32);
#endif

//#ifdef THREEDIM
//    int slice;
//
//    for (slice = 0; slice < params.nslices; slice++)
//#endif
      for (row = 0; row < params.nrows; row++)
      {
        for (col = 0; col < params.ncols; col++)
        {
//#ifdef THREEDIM
//          index = col + row*params.ncols + slice*params.nrows*params.ncols;
//#else
          index = col + row*params.ncols;
//#endif
          region_label = region_object_label_map[index];
          region_index = region_label - 1;
          index = col + row*params.ncols;
          if (region_label == 0)
            npix = 0;
          else
            npix = region_objects[region_index].get_npix();
          tempImage.put_data(npix,col,row,0);
        }
      }

    tempImage.flush_data();
    tempImage.close();

    return;
  }

  void Spatial::write_object_mean_map_ext(const string& object_mean_map_ext_file, vector<RegionObject>& region_objects)
  {
    unsigned int index, region_label, region_index;
    int row, col, band;
    float object_mean;

    Image tempImage;

#ifdef GDAL
    tempImage.create(object_mean_map_ext_file,inputImage,params.nbands,GDT_Float32);
#else
    tempImage.create(object_mean_map_ext_file,params.ncols,params.nrows,params.nbands,Float32);
#endif

//#ifdef THREEDIM
//    int slice;
//#endif
    for (band = 0; band < params.nbands; band++)
    {
//#ifdef THREEDIM
//     for (slice = 0; slice < params.nslices; slice++)
//#endif
      for (row = 0; row < params.nrows; row++)
      {
        for (col = 0; col < params.ncols; col++)
        {
//#ifdef THREEDIM
//          index = col + row*params.ncols + slice*params.nrows*params.ncols;
//#else
          index = col + row*params.ncols;
//#endif
          region_label = region_object_label_map[index];
          region_index = region_label - 1;
          index = col + row*params.ncols;
          if (region_label == 0)
            object_mean = 0.0;
          else
          {
            object_mean = (float) region_objects[region_index].get_unscaled_mean(band);
          }
          tempImage.put_data(object_mean,col,row,band);
        }
      }
    }

    tempImage.flush_data();
    tempImage.close();

    return;
  }

  void Spatial::write_object_std_dev_map_ext(const string& object_std_dev_map_ext_file, vector<RegionObject>& region_objects)
  {
    unsigned int index, region_label, region_index;
    int row, col;
    float std_dev;

    Image tempImage;

#ifdef GDAL
    tempImage.create(object_std_dev_map_ext_file,inputImage,1,GDT_Float32);
#else
    tempImage.create(object_std_dev_map_ext_file,params.ncols,params.nrows,1,Float32);
#endif

//#ifdef THREEDIM
//    int slice;
//
//    for (slice = 0; slice < params.nslices; slice++)
//#endif
      for (row = 0; row < params.nrows; row++)
      {
        for (col = 0; col < params.ncols; col++)
        {
//#ifdef THREEDIM
//          index = col + row*params.ncols + slice*params.nrows*params.ncols;
//#else
          index = col + row*params.ncols;
//#endif
          region_label = region_object_label_map[index];
          region_index = region_label - 1;
          index = col + row*params.ncols;
          if (region_label == 0)
            std_dev = 0.0;
          else
            std_dev = (float) region_objects[region_index].get_band_max_std_dev();
          tempImage.put_data(std_dev,col,row,0);
        }
      }

    tempImage.flush_data();
    tempImage.close();

    return;
  }

  void Spatial::write_object_bpratio_map_ext(const string& object_bpratio_map_ext_file, vector<RegionObject>& region_objects)
  {
    unsigned int index, region_label, region_index;
    int row, col;
    float npix, boundary_npix, bpratio;

    Image tempImage;

#ifdef GDAL
    tempImage.create(object_bpratio_map_ext_file,inputImage,1,GDT_Float32);
#else
    tempImage.create(object_bpratio_map_ext_file,params.ncols,params.nrows,1,Float32);
#endif

//#ifdef THREEDIM
//    int slice;
//
//    for (slice = 0; slice < params.nslices; slice++)
//    {
//#endif
      for (row = 0; row < params.nrows; row++)
      {
        for (col = 0; col < params.ncols; col++)
        {
//#ifdef THREEDIM
//          index = col + row*params.ncols + slice*params.nrows*params.ncols;
//#else
          index = col + row*params.ncols;
//#endif
          region_label = region_object_label_map[index];
          region_index = region_label - 1;
          index = col + row*params.ncols;
          if (region_label == 0)
            bpratio = 0.0;
          else
          {
            npix = region_objects[region_index].get_npix();
            boundary_npix = region_objects[region_index].get_boundary_npix();
            bpratio = boundary_npix/npix;
          }
          tempImage.put_data(bpratio,col,row,0);
        }
      }

    tempImage.flush_data();
    tempImage.close();

    return;
  }

#ifdef SHAPEFILE
// Does not work for THREEDIM
  void Spatial::write_class_object_shapefile_ext(const string& class_shapefile_ext_file, const string& object_shapefile_ext_file,
                                                 const short unsigned int& hseg_level,
                                                 vector<RegionClass>& region_classes, vector<RegionObject>& region_objects)
  {
    int band;

    SHPHandle class_hSHP, object_hSHP;	// Shape File Handles
    DBFHandle class_hDBF, object_hDBF;     // DBF File Handles

    object_hSHP = SHPCreate(object_shapefile_ext_file.c_str(),SHPT_POLYGON);
    object_hDBF = DBFCreate(object_shapefile_ext_file.c_str());
    class_hSHP = SHPCreate(class_shapefile_ext_file.c_str(),SHPT_MULTIPOINT);
    class_hDBF = DBFCreate(class_shapefile_ext_file.c_str());

    DBFAddField(object_hDBF,"obLabel",FTInteger,9,0);
    DBFAddField(object_hDBF,"clLabel",FTInteger,9,0);
    DBFAddField(object_hDBF,"obNpix",FTInteger,9,0);
    if (params.region_sum_flag)
    {
      for (band = 0; band < params.nbands; band++)
      {
        stringstream objectMean;
        objectMean << "obMean" << band;
        DBFAddField(object_hDBF,objectMean.str().c_str(),FTDouble,19,16);
      }
      if (params.region_sumsq_flag)
      {
        for (band = 0; band < params.nbands; band++)
        {
          stringstream objectStdDev;
          objectStdDev << "obSD" << band;
          DBFAddField(object_hDBF,objectStdDev.str().c_str(),FTDouble,19,16);
        }
      }
    }

    DBFAddField(class_hDBF,"clLabel",FTInteger,9,0);
    DBFAddField(class_hDBF,"clNpix",FTInteger,9,0);
    if (params.region_sum_flag)
    {
      for (band = 0; band < params.nbands; band++)
      {
        stringstream classMean;
        classMean << "clMean" << band;
        DBFAddField(class_hDBF,classMean.str().c_str(),FTDouble,19,16);
      }
      if (params.region_sumsq_flag)
      {
        for (band = 0; band < params.nbands; band++)
        {
          stringstream classStdDev;
          classStdDev << "clSD" << band;
          DBFAddField(class_hDBF,classStdDev.str().c_str(),FTDouble,19,16);
        }
      }
    }

  // Determine which region objects are valid
    unsigned int region_object_label, region_object_index, region_class_label;
    unsigned int index, nb_objects, nb_classes;
    unsigned int *region_class;
    int row, col;
    bool *valid_region_objects;
    nb_objects = region_objects.size();
    nb_classes = region_classes.size();
    valid_region_objects = new bool[nb_objects];
    region_class = new unsigned int[nb_objects];
    for (region_object_index = 0; region_object_index < nb_objects; region_object_index++)
    {
      valid_region_objects[region_object_index] = false;
      region_class[region_object_index] = 0;
    }
    for (row = 0; row < params.nrows; row++)
      for (col = 0; col < params.ncols; col++)
      {
        index = col + row*params.ncols;
        region_object_label = region_object_label_map[index];
        region_class_label = region_class_label_map[index];
        if (region_object_label > 0)
        {
          region_object_index = region_object_label - 1;
          valid_region_objects[region_object_index] = true;
          region_class[region_object_index] = region_class_label;
        }
      }

  // Trace region object boundaries and write shapefile entries
#ifdef GDAL
    Image regionLabelImage;
    bool region_label_image_flag = false;
#endif
    SHPObject *object_shpObject, *class_shpObject;
    int entity, field, vertex, nb_parts, nb_vertices, nb_points;
    unsigned int *two_label_map;
    two_label_map = new unsigned int[(params.ncols+2)*(params.nrows+2)];
    unsigned int *connected_label_map;
    connected_label_map = new unsigned int[(params.ncols+2)*(params.nrows+2)];
    int *label_map_bounds;
    label_map_bounds = new int[4];
    bool *boundary_mask;
    boundary_mask = new bool[(2*params.ncols+1)*(2*params.nrows+1)];
    int *boundary_mask_bounds;
    boundary_mask_bounds = new int[4];
    vector<int> partStart;
    vector<double> vX, vY;
    double *first_X, *first_Y;
    first_X = new double[nb_objects];
    first_Y = new double[nb_objects];
    set<unsigned int> region_hole_label_set;
    set<unsigned int>::const_iterator region_hole_label_set_iter;
    unsigned int region_hole_label;
    double X_offset = 0.0, Y_offset = 0.0, X_gsd = 1.0, Y_gsd = -1.0, *Z;
    Z = new double[1];
    Z[0] = 0;
#ifdef GDAL
    region_label_image_flag = regionLabelImage.open(params.object_labels_map_file);
    if ((region_label_image_flag) && (regionLabelImage.geotransform_valid()))
    {
      X_offset = regionLabelImage.get_X_offset();
      Y_offset = regionLabelImage.get_Y_offset();
      X_gsd = regionLabelImage.get_X_gsd();
      Y_gsd = regionLabelImage.get_Y_gsd();
    }
    regionLabelImage.close();
#endif

  // Write object shape file
    entity = 0;
    for (region_object_index = 0; region_object_index < nb_objects; region_object_index++)
      if (valid_region_objects[region_object_index])
      {
        region_object_label = region_object_index + 1;
        set_two_label_map(region_object_label, region_object_label_map, two_label_map, label_map_bounds);
        set_connected_label_map(two_label_map, label_map_bounds, connected_label_map);
        set_boundary_mask(1, connected_label_map, boundary_mask, boundary_mask_bounds);
        vX.clear();
        vY.clear();
        partStart.clear();
        trace_region_outer_boundary(boundary_mask, boundary_mask_bounds, vX, vY, partStart);
        region_hole_label_set.clear();
        find_region_hole_labels(1, connected_label_map, boundary_mask, boundary_mask_bounds, region_hole_label_set);
        region_hole_label_set_iter = region_hole_label_set.begin();
        while (region_hole_label_set_iter != region_hole_label_set.end())
        {
          region_hole_label = *region_hole_label_set_iter;
          set_boundary_mask(region_hole_label, connected_label_map, boundary_mask, boundary_mask_bounds);
          trace_region_hole_boundary(boundary_mask, boundary_mask_bounds, vX, vY, partStart);
          region_hole_label_set_iter++;
        }

        nb_vertices = vX.size();
        nb_parts = partStart.size();
        double *X, *Y;
        int *Start;
        X = new double[nb_vertices];
        Y = new double[nb_vertices];
        Start = new int[nb_parts];
        for (vertex = 0; vertex < nb_vertices; vertex++)
        {
          X[vertex] = vX[vertex]*X_gsd + X_offset;
          Y[vertex] = vY[vertex]*Y_gsd + Y_offset;
        }
        first_X[region_object_index] = X[0];
        first_Y[region_object_index] = Y[0];
        for (vertex = 0; vertex < nb_parts; vertex++)
        {
          Start[vertex] = partStart[vertex];
        }
        object_shpObject = SHPCreateObject(SHPT_POLYGON, -1, nb_parts, Start, NULL, nb_vertices, X, Y, Z, NULL);
        SHPComputeExtents(object_shpObject);
        SHPWriteObject(object_hSHP,-1,object_shpObject);
        field = 0;
        DBFWriteIntegerAttribute(object_hDBF,entity,field++,region_objects[region_object_index].get_label());
        DBFWriteIntegerAttribute(object_hDBF,entity,field++,region_class[region_object_index]);
        DBFWriteIntegerAttribute(object_hDBF,entity,field++,region_objects[region_object_index].get_npix());
        if (params.region_sum_flag)
        {
          for (band = 0; band < params.nbands; band++)
            DBFWriteDoubleAttribute(object_hDBF,entity,field++,region_objects[region_object_index].get_unscaled_mean(band));
          if (params.region_sumsq_flag)
          {
            for (band = 0; band < params.nbands; band++)
              DBFWriteDoubleAttribute(object_hDBF,entity,field++,region_objects[region_object_index].get_unscaled_std_dev(band));
          }
        }
        entity++;
      }

  // Write class shape file
    unsigned int region_class_index;
    entity = 0;
    for (region_class_index = 0; region_class_index < nb_classes; region_class_index++)
    {
      region_class_label = region_class_index + 1;
      nb_points = 0;
      for (region_object_index = 0; region_object_index < nb_objects; region_object_index++)
        if (region_class[region_object_index] == region_class_label)
          nb_points++;
      if (nb_points > 0)
      {
        double *X, *Y;
        X = new double[nb_points];
        Y = new double[nb_points];
        vertex = 0;
        for (region_object_index = 0; region_object_index < nb_objects; region_object_index++)
          if (region_class[region_object_index] == region_class_label)
          {
            X[vertex] = first_X[region_object_index];
            Y[vertex] = first_Y[region_object_index];
            vertex++;
          }
        class_shpObject = SHPCreateSimpleObject(SHPT_MULTIPOINT, nb_points, X, Y, Z);
        SHPComputeExtents(class_shpObject);
        SHPWriteObject(class_hSHP,-1,class_shpObject);
        field = 0;
        DBFWriteIntegerAttribute(class_hDBF,entity,field++,region_classes[region_class_index].get_label());
        DBFWriteIntegerAttribute(class_hDBF,entity,field++,region_classes[region_class_index].get_npix());
        if (params.region_sum_flag)
        {
          for (band = 0; band < params.nbands; band++)
            DBFWriteDoubleAttribute(class_hDBF,entity,field++,region_classes[region_class_index].get_unscaled_mean(band));
          if (params.region_sumsq_flag)
          {
            for (band = 0; band < params.nbands; band++)
              DBFWriteDoubleAttribute(class_hDBF,entity,field++,region_classes[region_class_index].get_unscaled_std_dev(band));
          }
        }
        entity++;
      }
    }

    SHPClose(object_hSHP);
    DBFClose(object_hDBF);
    SHPClose(class_hSHP);
    DBFClose(class_hDBF);

    return;
  }

// Does not work for THREEDIM
// When params.region_objects_flag is false, the region classes are actually region objects!
  void Spatial::write_object_shapefile_ext(const string& class_shapefile_ext_file, const string& object_shapefile_ext_file,
                                           const short unsigned int& hseg_level,
                                           vector<RegionClass>& region_classes, vector<RegionObject>& region_objects)
  {
    int band;

    SHPHandle hSHP;	// Shape File Handle
    DBFHandle hDBF;     // DBF File Handle

    if (params.region_objects_flag)
    {
      hSHP = SHPCreate(object_shapefile_ext_file.c_str(),SHPT_POLYGON);
      hDBF = DBFCreate(object_shapefile_ext_file.c_str());
    }
    else
    {
      hSHP = SHPCreate(class_shapefile_ext_file.c_str(),SHPT_POLYGON);
      hDBF = DBFCreate(class_shapefile_ext_file.c_str());
    }
    DBFAddField(hDBF,"obLabel",FTInteger,9,0);
    DBFAddField(hDBF,"obNpix",FTInteger,9,0);
    if (params.region_sum_flag)
    {
      for (band = 0; band < params.nbands; band++)
      {
        stringstream objectMean;
        objectMean << "obMean" << band;
        DBFAddField(hDBF,objectMean.str().c_str(),FTDouble,19,16);
      }
      if (params.region_sumsq_flag)
      {
        for (band = 0; band < params.nbands; band++)
        {
          stringstream objectStdDev;
          objectStdDev << "obSD" << band;
          DBFAddField(hDBF,objectStdDev.str().c_str(),FTDouble,19,16);
        }
      }
    }

  // Determine which region objects are valid
    unsigned int region_label, region_index;
    unsigned int index, nb_objects;
    int row, col;
    bool *valid_region_objects;
    if (params.region_objects_flag)
      nb_objects = region_objects.size();
    else
      nb_objects = region_classes.size();
    valid_region_objects = new bool[nb_objects];
    for (region_index = 0; region_index < nb_objects; region_index++)
      valid_region_objects[region_index] = false;
    for (row = 0; row < params.nrows; row++)
      for (col = 0; col < params.ncols; col++)
      {
        index = col + row*params.ncols;
        if (params.region_objects_flag)
          region_label = region_object_label_map[index];
        else
          region_label = region_class_label_map[index];
        if (region_label > 0)
        {
          region_index = region_label - 1;
          valid_region_objects[region_index] = true;
        }
      }

  // Trace region object boundaries and write shapefile entries
#ifdef GDAL
    Image regionLabelImage;
    bool region_label_image_flag = false;
#endif
    SHPObject *shpObject;
    int entity, field, vertex, nb_parts, nb_vertices;
    unsigned int *two_label_map;
    two_label_map = new unsigned int[(params.ncols+2)*(params.nrows+2)];
    unsigned int *connected_label_map;
    connected_label_map = new unsigned int[(params.ncols+2)*(params.nrows+2)];
    int *label_map_bounds;
    label_map_bounds = new int[4];
    bool *boundary_mask;
    boundary_mask = new bool[(2*params.ncols+1)*(2*params.nrows+1)];
    int *boundary_mask_bounds;
    boundary_mask_bounds = new int[4];
    vector<int> partStart;
    vector<double> vX, vY;
    set<unsigned int> region_hole_label_set;
    set<unsigned int>::const_iterator region_hole_label_set_iter;
    unsigned int region_hole_label;
    double X_offset = 0.0, Y_offset = 0.0, X_gsd = 1.0, Y_gsd = -1.0, *Z;
    Z = new double[1];
    Z[0] = 0;
#ifdef GDAL
    if (params.region_objects_flag)
      region_label_image_flag = regionLabelImage.open(params.object_labels_map_file);
    else
      region_label_image_flag = regionLabelImage.open(params.class_labels_map_file);
    if ((region_label_image_flag) && (regionLabelImage.geotransform_valid()))
    {
      X_offset = regionLabelImage.get_X_offset();
      Y_offset = regionLabelImage.get_Y_offset();
      X_gsd = regionLabelImage.get_X_gsd();
      Y_gsd = regionLabelImage.get_Y_gsd();
    }
    regionLabelImage.close();
#endif

  // Write object shape file
    entity = 0;
    for (region_index = 0; region_index < nb_objects; region_index++)
      if (valid_region_objects[region_index])
      {
        region_label = region_index + 1;
        if (params.region_objects_flag)
          set_two_label_map(region_label, region_object_label_map, two_label_map, label_map_bounds);
        else
          set_two_label_map(region_label, region_class_label_map, two_label_map, label_map_bounds);
        set_connected_label_map(two_label_map, label_map_bounds, connected_label_map);
        set_boundary_mask(1, connected_label_map, boundary_mask, boundary_mask_bounds);
        vX.clear();
        vY.clear();
        partStart.clear();
        trace_region_outer_boundary(boundary_mask, boundary_mask_bounds, vX, vY, partStart);
        region_hole_label_set.clear();
        find_region_hole_labels(1, connected_label_map, boundary_mask, boundary_mask_bounds, region_hole_label_set);
        region_hole_label_set_iter = region_hole_label_set.begin();
        while (region_hole_label_set_iter != region_hole_label_set.end())
        {
          region_hole_label = *region_hole_label_set_iter;
          set_boundary_mask(region_hole_label, connected_label_map, boundary_mask, boundary_mask_bounds);
          trace_region_hole_boundary(boundary_mask, boundary_mask_bounds, vX, vY, partStart);
          region_hole_label_set_iter++;
        }

        nb_vertices = vX.size();
        nb_parts = partStart.size();
        double *X, *Y;
        int *Start;
        X = new double[nb_vertices];
        Y = new double[nb_vertices];
        Start = new int[nb_parts];
        for (vertex = 0; vertex < nb_vertices; vertex++)
        {
          X[vertex] = vX[vertex]*X_gsd + X_offset;
          Y[vertex] = vY[vertex]*Y_gsd + Y_offset;
        }
        for (vertex = 0; vertex < nb_parts; vertex++)
        {
          Start[vertex] = partStart[vertex];
        }
        shpObject = SHPCreateObject(SHPT_POLYGON, -1, nb_parts, Start, NULL, nb_vertices, X, Y, Z, NULL);
        SHPComputeExtents(shpObject);
        SHPWriteObject(hSHP,-1,shpObject);
        field = 0;
        if (params.region_objects_flag)
          DBFWriteIntegerAttribute(hDBF,entity,field++,region_objects[region_index].get_label());
        else
          DBFWriteIntegerAttribute(hDBF,entity,field++,region_classes[region_index].get_label());
        if (params.region_objects_flag)
          DBFWriteIntegerAttribute(hDBF,entity,field++,region_objects[region_index].get_npix());
        else
          DBFWriteIntegerAttribute(hDBF,entity,field++,region_classes[region_index].get_npix());
        if (params.region_sum_flag)
        {
          for (band = 0; band < params.nbands; band++)
          {
            if (params.region_objects_flag)
              DBFWriteDoubleAttribute(hDBF,entity,field++,region_objects[region_index].get_unscaled_mean(band));
            else
              DBFWriteDoubleAttribute(hDBF,entity,field++,region_classes[region_index].get_unscaled_mean(band));
          }
          if (params.region_sumsq_flag)
          {
            for (band = 0; band < params.nbands; band++)
            {
              if (params.region_objects_flag)
                DBFWriteDoubleAttribute(hDBF,entity,field++,region_objects[region_index].get_unscaled_std_dev(band));
              else
                DBFWriteDoubleAttribute(hDBF,entity,field++,region_classes[region_index].get_unscaled_std_dev(band));
            }
          }
        }
        entity++;
      }

    SHPClose(hSHP);
    DBFClose(hDBF);

    return;
  }

#endif
#endif // RHSEG_EXTRACT

  void Spatial::print_class_label_map(const short unsigned int& hlevel, vector<RegionClass>& region_classes)
  {
    unsigned int index, region_class_label, region_index;
    int row, col;
#ifdef THREEDIM
    int slice;

    for (slice = 0; slice < params.nslices; slice++)
    {
      params.log_fs << "Region class map for slice = " << slice << ":" << endl<< endl;
#endif
      for (row = 0; row < params.nrows; row++)
      {
        for (col = 0; col < params.ncols; col++)
        {
#ifdef THREEDIM
          index = col + row*params.ncols + slice*params.nrows*params.ncols;
#else
          index = col + row*params.ncols;
#endif
          region_class_label = region_class_label_map[index];
          if ((hlevel > 0) && (region_class_label > 0))
          {
            region_index = region_class_label - 1;
            if (!region_classes[region_index].get_active_flag())
              region_class_label = region_classes[region_index].get_merge_region_label();
          }
          params.log_fs << region_class_label << " ";
        }
        params.log_fs << endl;
      }
#ifdef THREEDIM
      params.log_fs << endl;
    }
#endif

    return;
  }

  void Spatial::print_object_label_map(const short unsigned int& hlevel, vector<RegionObject>& region_objects)
  {
    unsigned int index, region_object_label, region_index;
    int row, col;
#ifdef THREEDIM
    int slice;

    for (slice = 0; slice < params.nslices; slice++)
    {
      params.log_fs << "Region object map for slice = " << slice << ":" << endl<< endl;
#endif
      for (row = 0; row < params.nrows; row++)
      {
        for (col = 0; col < params.ncols; col++)
        {
#ifdef THREEDIM
          index = col + row*params.ncols + slice*params.nrows*params.ncols;
#else
          index = col + row*params.ncols;
#endif
          region_object_label = region_object_label_map[index];
          if ((hlevel > 0) && (region_object_label > 0))
          {
            region_index = region_object_label - 1;
            if (!region_objects[region_index].get_active_flag())
              region_object_label = region_objects[region_index].get_merge_region_label();
          }
          params.log_fs << region_object_label << " ";
        }
        params.log_fs << endl;
      }
#ifdef THREEDIM
      params.log_fs << endl;
    }
#endif

    return;
  }

} // namespace HSEGTilton
