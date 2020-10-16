/*-----------------------------------------------------------
|
|  Routine Name: recur_send
|
|       Purpose: Send recursive results to indicated task.
|
|         Input: section          (ID of task to which results are to be sent)
|                recur_level      (Lowest recursive level at which this task is active)
|                max_threshold    (Maximum merging threshold encountered)
|                pixel_data       (Class which holds information pertaining to the pixel of pixels processed by this task)
|                
|        Output: region_classes      (Class which holds region related information)
|                nregions         (Current number of regions)
|
|         Other: temp_data        (buffers used in communications between parallel tasks)
|
|       Returns: (void)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: December 9, 2002.
| Modifications: February 10, 2003:  Changed region index to region label
|                May 29, 2003:  Changed to band-by-band sending of temp_data.double_buffer
|                               for recur_level > (params.onb_levels-1) (to better
|                               accommodate larger data sets).
|                June 2, 2003:  Rewrote portions to reduce memory requirements.
|                June 11, 2003: Modified the temp_data structure and its use.
|                October 2, 2003: Eliminated the index_class
|                November 7, 2003 - Modified Params structure and made class member variables all private
|                December 23, 2004 - Changed region label from short unsigned int to unsigned int
|                May 16, 2008 - Revised to work with globally defined Params and oParams class objects
|                March 24, 2011 - Revised counting of recursive levels to start at level = 0 (instead of level = 1).
|                August 1, 2013 - Revised to accommodate standard deviation and region edge information.
|                March 14, 2014 - Added communication of initial_merge_flag value.
|
------------------------------------------------------------*/
#include <defines.h>
#include <params/params.h>
#include <region/region_class.h>
#include <pixel/pixel.h>
#include <iostream>

extern HSEGTilton::Params params;

namespace HSEGTilton
{
 void recur_send(const short unsigned int& section, const short unsigned int& recur_level,
                 vector<Pixel>& pixel_data, vector<RegionClass>& region_classes, 
                 const unsigned int& nregions, const unsigned int& global_nregions, 
                 const double& max_threshold, Temp& temp_data)
 {
  bool mask;
  unsigned int taskid = section;
  unsigned int region_index, pixel_index, pixel_data_size;
  unsigned int byte_buf_size, short_buf_size, int_buf_size, float_buf_size, double_buf_size;
  int recur_data_tag = 100;

  short_buf_size = int_buf_size = float_buf_size = double_buf_size = 0;
  if (nregions > 0)
  {
    short_buf_size = nregions; // initial_merge_flag
    int_buf_size = 2*nregions;  // nghbrs_label_set_size() & npix
    double_buf_size = params.nbands*nregions; // sum[band]
    if (params.region_sumsq_flag)
      double_buf_size += params.nbands*nregions; // sumsq[band]
    if (params.region_sumxlogx_flag)
      double_buf_size += params.nbands*nregions; // sumxlogx[band]
    if (params.std_dev_image_flag)
      double_buf_size += params.nbands*nregions; // sum_pixel_std_dev[band]
    if (params.edge_image_flag)
      float_buf_size += nregions; // max_edge_value
    for (region_index = 0; region_index < nregions; ++region_index)
      int_buf_size += region_classes[region_index].get_nghbrs_label_set_size(); // neighbor region label
    double_buf_size += nregions; // merge_threshold
    check_buf_size(0,short_buf_size,int_buf_size,float_buf_size,double_buf_size,temp_data);

  // region_classes assumed to be compact!!
    short_buf_size = int_buf_size = float_buf_size = double_buf_size = 0;
    for (region_index = 0; region_index < nregions; ++region_index)
        region_classes[region_index].load_all_data(temp_data,short_buf_size,int_buf_size,float_buf_size,double_buf_size);
  }
  temp_data.position = 0;
  MPI::UNSIGNED.Pack(&nregions, 1, temp_data.buffer, temp_data.buf_size, temp_data.position, MPI::COMM_WORLD);
  MPI::UNSIGNED.Pack(&global_nregions, 1, temp_data.buffer, temp_data.buf_size, temp_data.position, MPI::COMM_WORLD);
  if (nregions > 0)
  {
    MPI::DOUBLE.Pack(&max_threshold, 1, temp_data.buffer, temp_data.buf_size, temp_data.position, MPI::COMM_WORLD);
    MPI::UNSIGNED.Pack(&short_buf_size, 1, temp_data.buffer, temp_data.buf_size, temp_data.position, MPI::COMM_WORLD);
    MPI::UNSIGNED.Pack(&int_buf_size, 1, temp_data.buffer, temp_data.buf_size, temp_data.position, MPI::COMM_WORLD);
    MPI::UNSIGNED.Pack(&float_buf_size, 1, temp_data.buffer, temp_data.buf_size, temp_data.position, MPI::COMM_WORLD);
    MPI::UNSIGNED.Pack(&double_buf_size, 1, temp_data.buffer, temp_data.buf_size, temp_data.position, MPI::COMM_WORLD);
  }
#ifdef TIME_IT
  MPI::COMM_WORLD.Send(&nregions, 1, MPI::UNSIGNED, taskid, recur_data_tag);
#endif
  if (temp_data.position > temp_data.buf_size)
  {
    if (params.debug > 1)
    {
      params.log_fs << "WARNING(4): temp_data.position = " << temp_data.position << endl;
      params.log_fs << "Message from taskid = " << params.myid << endl;
    }
    else
    {
      cout << "WARNING(4): temp_data.position = " << temp_data.position << endl;
      cout << "Message from taskid = " << params.myid << endl;
    }
  }
  MPI::COMM_WORLD.Send(temp_data.buffer, temp_data.position, MPI::PACKED, taskid, recur_data_tag);
  if (params.debug > 2)
    params.log_fs << "Successfully sent temp_data.buffer with temp_data.position = " << temp_data.position << endl;
  if (nregions > 0)
  {
    if (short_buf_size > 0)
      MPI::COMM_WORLD.Send(temp_data.short_buffer, short_buf_size, MPI::UNSIGNED_SHORT, taskid, recur_data_tag);
    if (int_buf_size > 0)
      MPI::COMM_WORLD.Send(temp_data.int_buffer, int_buf_size, MPI::UNSIGNED, taskid, recur_data_tag);
    if (float_buf_size > 0)
      MPI::COMM_WORLD.Send(temp_data.float_buffer, float_buf_size, MPI::FLOAT, taskid, recur_data_tag);
    if (double_buf_size > 0)
      MPI::COMM_WORLD.Send(temp_data.double_buffer, double_buf_size, MPI::DOUBLE, taskid, recur_data_tag);
  }

  if (recur_level > (params.onb_levels-1))
  {
    pixel_data_size = pixel_data.size();
    byte_buf_size = pixel_data_size;
    if (params.std_dev_image_flag)
      byte_buf_size += pixel_data_size;
    if (params.edge_image_flag)
      byte_buf_size += pixel_data_size;
    check_buf_size(byte_buf_size,0,0,0,0,temp_data);
    int_buf_size = float_buf_size = 0;
    for (pixel_index = 0; pixel_index < pixel_data_size; ++pixel_index)
    {
      mask = pixel_data[pixel_index].get_mask();
      if (mask)
      {
        int_buf_size++;   // region_label
        temp_data.byte_buffer[pixel_index] = 1;
      }
      else
        temp_data.byte_buffer[pixel_index] = 0;
    }
    if (int_buf_size > 0)
    {
      float_buf_size = int_buf_size*params.nbands;  // input_data
      if (params.std_dev_image_flag)
        float_buf_size += int_buf_size*params.nbands;  // local_std_dev
      if (params.edge_image_flag)
        float_buf_size += int_buf_size;  // edge_value
      check_buf_size(0,0,int_buf_size,float_buf_size,0,temp_data);
      byte_buf_size = pixel_data_size;
      int_buf_size = float_buf_size = 0;
      for (pixel_index = 0; pixel_index < pixel_data_size; ++pixel_index)
        if (pixel_data[pixel_index].get_mask())
          pixel_data[pixel_index].load_all_data(temp_data,byte_buf_size,int_buf_size,float_buf_size);
    }
    else
      byte_buf_size = pixel_data_size;
    MPI::COMM_WORLD.Send(&pixel_data_size, 1, MPI::UNSIGNED, taskid, recur_data_tag);
    MPI::COMM_WORLD.Send(&byte_buf_size, 1, MPI::UNSIGNED, taskid, recur_data_tag);
    MPI::COMM_WORLD.Send(&int_buf_size, 1, MPI::UNSIGNED, taskid, recur_data_tag);
    MPI::COMM_WORLD.Send(&float_buf_size, 1, MPI::UNSIGNED, taskid, recur_data_tag);
    MPI::COMM_WORLD.Send(temp_data.byte_buffer, byte_buf_size, MPI::UNSIGNED_CHAR, taskid, recur_data_tag);
    if (int_buf_size > 0)
      MPI::COMM_WORLD.Send(temp_data.int_buffer, int_buf_size, MPI::UNSIGNED, taskid, recur_data_tag);
    if (float_buf_size > 0)
      MPI::COMM_WORLD.Send(temp_data.float_buffer, float_buf_size, MPI::FLOAT, taskid, recur_data_tag);
  }

  return;
 }
} // namespace HSEGTilton
