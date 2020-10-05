/*-----------------------------------------------------------
|
|  Routine Name: recur_receive
|
|       Purpose: Receive recursive results from indicated task.
|
|         Input: section          (ID of recursive task from which results are to be received)
|                recur_level      (Lowest recursive level at which this task is active)
|                class_label_offset    (Offset to be added to region labels)
|                pixel_data       (Class which holds information pertaining to the pixel of pixels processed by this task)
|                
|        Output: region_classes      (Class which holds region related information)
|                max_threshold    (Maximum merging threshold encountered)
|
|         Other: temp_data        (buffers used in communications between parallel tasks)
|
|       Returns: nregions         (Current number of regions)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: November 19, 2002.
| Modifications: February 10, 2003:  Changed region index to region label
|                May 29, 2003:  Changed to band-by-band sending of temp_data.double_buffer
|                               for recur_level > (params.onb_levels-1) (to better
|                               accommodate larger data sets).
|                June 2, 2003:  Rewrote portions to reduce memory requirements.
|                June 11, 2003 - Modified the temp_data structure and its use.
|                December 23, 2004 - Changed region label from short unsigned int to unsigned int
|                January 5, 2006 - Added slice dimension (extension to three-dimensional analysis)
|                May 16, 2008 - Revised to work with globally defined Params and oParams class objects
|                March 24, 2011 - Revised counting of recursive levels to start at level = 0 (instead of level = 1).
|                July 29, 2013 - Revised to accommodate local_std_dev and edge_value data.
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
 unsigned int recur_receive(const short unsigned int& section, const short unsigned int& recur_level,
                            unsigned int class_label_offset, vector<Pixel>& pixel_data, vector<RegionClass>& region_classes,
                            unsigned int& global_nregions, double& max_threshold, 
                            Temp& temp_data)
 {
  bool mask;
  unsigned int taskid = section;
  unsigned int nregions, region_index, npixels, pixel_index;
  unsigned int byte_buf_size, short_buf_size, int_buf_size, float_buf_size, double_buf_size;
  int recur_data_tag = 100;

#ifdef TIME_IT
  float end_time, elapsed_time;
  end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
  elapsed_time = end_time - temp_data.start_time;
  if (elapsed_time > 0.0) temp_data.compute += elapsed_time;
  temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
  MPI::COMM_WORLD.Recv(&nregions, 1, MPI::UNSIGNED, taskid, recur_data_tag);
  end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
  elapsed_time = end_time - temp_data.start_time;
  if (elapsed_time > 0.0) temp_data.wait += elapsed_time;
  temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
  MPI::COMM_WORLD.Recv(temp_data.buffer, temp_data.buf_size, MPI::PACKED, taskid, recur_data_tag);
  if (params.debug > 2)
    params.log_fs << "Successfully received temp_data.buffer with temp_data.buf_size = " << temp_data.buf_size << endl;

  temp_data.position = 0;
  MPI::UNSIGNED.Unpack(temp_data.buffer, temp_data.buf_size, &nregions, 1, temp_data.position, MPI::COMM_WORLD);
  MPI::UNSIGNED.Unpack(temp_data.buffer, temp_data.buf_size, &global_nregions, 1, temp_data.position, MPI::COMM_WORLD);
  if (nregions > 0)
  {
    MPI::DOUBLE.Unpack(temp_data.buffer, temp_data.buf_size, &max_threshold, 1, temp_data.position, MPI::COMM_WORLD);
    MPI::UNSIGNED.Unpack(temp_data.buffer, temp_data.buf_size, &short_buf_size, 1, temp_data.position, MPI::COMM_WORLD);
    MPI::UNSIGNED.Unpack(temp_data.buffer, temp_data.buf_size, &int_buf_size, 1, temp_data.position, MPI::COMM_WORLD);
    MPI::UNSIGNED.Unpack(temp_data.buffer, temp_data.buf_size, &float_buf_size, 1, temp_data.position, MPI::COMM_WORLD);
    MPI::UNSIGNED.Unpack(temp_data.buffer, temp_data.buf_size, &double_buf_size, 1, temp_data.position, MPI::COMM_WORLD);

    check_buf_size(0,short_buf_size,int_buf_size,float_buf_size,double_buf_size,temp_data);
    if (short_buf_size > 0)
      MPI::COMM_WORLD.Recv(temp_data.short_buffer, short_buf_size, MPI::UNSIGNED_SHORT, taskid, recur_data_tag);
    if (int_buf_size > 0)
      MPI::COMM_WORLD.Recv(temp_data.int_buffer, int_buf_size, MPI::UNSIGNED, taskid, recur_data_tag);
    if (float_buf_size > 0)
      MPI::COMM_WORLD.Recv(temp_data.float_buffer, float_buf_size, MPI::FLOAT, taskid, recur_data_tag);
    if (double_buf_size > 0)
      MPI::COMM_WORLD.Recv(temp_data.double_buffer, double_buf_size, MPI::DOUBLE, taskid, recur_data_tag);
  }
#ifdef TIME_IT
  end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
  elapsed_time = end_time - temp_data.start_time;
  if (elapsed_time > 0.0) temp_data.transfer += elapsed_time;
  temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif

  if (region_classes.size() < nregions)
    region_classes.resize(nregions);
  short_buf_size = int_buf_size = float_buf_size = double_buf_size = 0;
  for (region_index = 0; region_index < nregions; ++region_index)
  {
    region_classes[region_index].clear();
    region_classes[region_index].set_active_flag(true);
    region_classes[region_index].set_label(region_index + class_label_offset + 1);
    region_classes[region_index].set_all_data(temp_data,class_label_offset,short_buf_size,int_buf_size,float_buf_size,double_buf_size);
  }

  if ((recur_level <= (params.onb_levels-1)) && (class_label_offset > 0))
  {
#ifdef THREEDIM
    parallel_recur_request((short unsigned int) 1, recur_level, taskid, class_label_offset, 0, 0, 0, 0, 0, 0, temp_data);
#else
    parallel_recur_request((short unsigned int) 1, recur_level, taskid, class_label_offset, 0, 0, 0, 0, 0, temp_data);
#endif
    if (params.debug > 2)
      params.log_fs << "Sent parallel_recur_request to taskid = " << taskid << " with class_label_offset = " << class_label_offset << endl;
  }

  if (recur_level > (params.onb_levels-1))
  {
#ifdef TIME_IT
    end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
    elapsed_time = end_time - temp_data.start_time;
    if (elapsed_time > 0.0) temp_data.compute += elapsed_time;
    temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
    MPI::COMM_WORLD.Recv(&npixels, 1, MPI::UNSIGNED, taskid, recur_data_tag);
    MPI::COMM_WORLD.Recv(&byte_buf_size, 1, MPI::UNSIGNED, taskid, recur_data_tag);
    MPI::COMM_WORLD.Recv(&int_buf_size, 1, MPI::UNSIGNED, taskid, recur_data_tag);
    MPI::COMM_WORLD.Recv(&float_buf_size, 1, MPI::UNSIGNED, taskid, recur_data_tag);
#ifdef TIME_IT
    end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
    elapsed_time = end_time - temp_data.start_time;
    if (elapsed_time > 0.0) temp_data.wait += elapsed_time;
    temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
    check_buf_size(byte_buf_size,0,int_buf_size,float_buf_size,0,temp_data);
    MPI::COMM_WORLD.Recv(temp_data.byte_buffer,byte_buf_size, MPI::UNSIGNED_CHAR, taskid, recur_data_tag);
    if (int_buf_size > 0)
      MPI::COMM_WORLD.Recv(temp_data.int_buffer, int_buf_size, MPI::UNSIGNED, taskid, recur_data_tag);
    if (float_buf_size > 0)
      MPI::COMM_WORLD.Recv(temp_data.float_buffer, float_buf_size, MPI::FLOAT, taskid, recur_data_tag);
#ifdef TIME_IT
    end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
    elapsed_time = end_time - temp_data.start_time;
    if (elapsed_time > 0.0) temp_data.transfer += elapsed_time;
    temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
    for (pixel_index = 0; pixel_index < npixels; ++pixel_index)
    {
      mask = (temp_data.byte_buffer[pixel_index] == 1);
      pixel_data[pixel_index].set_mask(mask);
    }
    byte_buf_size = npixels;
    int_buf_size = float_buf_size = 0;
    for (pixel_index = 0; pixel_index < npixels; ++pixel_index)
    {
      mask = pixel_data[pixel_index].get_mask();
      if (mask)
        pixel_data[pixel_index].set_all_data(temp_data,class_label_offset,byte_buf_size,int_buf_size,float_buf_size);
    }
  }

  if ((recur_level <= (params.onb_levels-1)) && (class_label_offset > 0))
  {
    int class_label_offset_tag = 40;
#ifdef TIME_IT
    end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
    elapsed_time = end_time - temp_data.start_time;
    if (elapsed_time > 0.0) temp_data.compute += elapsed_time;
    temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
    MPI::COMM_WORLD.Recv(&class_label_offset, 1, MPI::UNSIGNED, taskid, class_label_offset_tag);
#ifdef TIME_IT
    end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
    elapsed_time = end_time - temp_data.start_time;
    if (elapsed_time > 0.0) temp_data.wait += elapsed_time;
    temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
  }

  return nregions;
 }
} // namespace HSEGTilton
