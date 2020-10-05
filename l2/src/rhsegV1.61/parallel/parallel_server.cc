/*-----------------------------------------------------------
|
|  Routine Name: parallel_server
|
|       Purpose: Function for executing parallel processing requests.
|
|         Input: section          (Section or window processed by this call to parallel_server)
|                spatial_data     (Class which holds information pertaining to input and output spatial data)
|                pixel_data       (Class which holds information pertaining to the pixel of pixels processed by this task)
|                region_classes      (Class which holds region related information)
|                
|        Output:
|
|         Other: temp_data        (buffers used in communications between parallel tasks)
|
|       Returns: (void)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: December 10, 2002.
| Modifications: January 29, 2003 - Added update_region_index request.
|                February 10, 2003 - Changed region_index to region_label.
|                February 14, 2003 - Added option for classical region growing (instead of HSEG).
|                May 29, 2003 - Changed MPI_SEND for switch_pixel request to better accomodate
|                               large numbers of switches pixels.
|                June 9, 2003 - Changed certain lists to sets for efficiency
|                June 11, 2003 - Modified the temp_data structure and its use.
|                October 3, 2003 - Eliminated index_data
|                October 1, 2004 - Added SPLIT_REMERGE version
|                December 23, 2004 - Changed region label from short unsigned int to unsigned int
|                January 5, 2006 - Added slice dimension (extension to three-dimensional analysis)
|                May 16, 2008 - Reorganized coes and revised to work with globally defined Params 
|                               and oParams class objects
|                August 1, 2013 - Revised to accommodate standard deviation and region edge information.
|                September 18, 2013 - Removed cases 14, 20 and 21 (no longer needed) and associated variables
|                January 9, 2014 - Bug fix (float_buf_position initialization)
|
------------------------------------------------------------*/
#include <defines.h>
#include <params/params.h>
#include <region/region_class.h>
#include <region/region_object.h>
#include <spatial/spatial.h>
#include <pixel/pixel.h>
#include <index/index.h>
#include <iostream>

extern HSEGTilton::Params params;
extern HSEGTilton::oParams oparams;

namespace HSEGTilton
{
 void parallel_server(const short unsigned int& section, Spatial& spatial_data, vector<Pixel>& pixel_data,
                      vector<RegionClass>& region_classes, vector<RegionClass *>& nghbr_heap, vector<RegionClass *>& region_heap,
                      Temp& temp_data)
 {
  unsigned char border_flag = 0;
  short unsigned int recur_level;
  short unsigned int request_id, calling_taskid;
  unsigned int region_classes_size = region_classes.size(), nregions = 0;
  unsigned int region_index, region_label, max_region_label, class_label_offset, value;
  short unsigned int nslevels;
  unsigned int nb_nghbrs, nghbr_index;
  unsigned int new_buf_position;
  unsigned int index_data_size = 1;
  RegionObject::set_static_vals();
  vector<RegionObject> region_objects(1,RegionObject());
  unsigned int pixel_index, pixel_index2, pixel_data_size;
  unsigned int nb_region_pairs, region_object_index, nb_objects = 0;
  unsigned int max_region_object_label, region_object_label, object_label_offset, region_objects_size;
  bool col_border_flag, row_border_flag;
#ifdef THREEDIM
  int slice, nslices;
  bool slice_border_flag;
#endif
  int col, row, ncols, nrows, index, slice_size;
  unsigned int byte_buf_position, short_buf_position, int_buf_position, float_buf_position, double_buf_position;
  unsigned int short_buf_size, int_buf_size, float_buf_size, double_buf_size;
  short unsigned int byte_factor, short_factor, int_factor, float_factor;
  int class_label_offset_tag = 40;
  int temp_data_tag = 101;
  int sum_pixel_gdissim_tag = 102;
  int region_relabel_tag = 104;
  int region_classes_init_tag = 105;
  int border_index_data_tag = 106;
  int update_region_label_map_tag = 107;
  int write_region_label_map_tag = 108;
  int border_index_data_tag2 = 109;
  int write_boundary_map_tag = 110;
  int conn_comp_init_tag = 116;
  int conn_comp_tag = 117;
  int boundary_map_init_tag = 118;
  int set_boundary_map_tag = 119;
  map<unsigned int,unsigned int> region_relabel_pairs;
  map<unsigned int,unsigned int> region_object_relabel_pairs;
  unsigned int nbregion_index, nbregion_label;
  unsigned int max_index;
  int update_region_label_tag = 115;
  float this_region_pixel_dissim, other_region_pixel_dissim;
  unsigned int orig_nregions, recur_region_label;
  int nghbrs_label_set_init_tag = 122;
  double max_threshold;

  if (params.debug > 2)
    params.log_fs << "Entered parallel_server routine..." << endl;

  MPI::COMM_WORLD.Recv(temp_data.buffer, temp_data.buf_size, MPI::PACKED,
                       MPI::ANY_SOURCE, temp_data_tag);

  temp_data.position = 0;
  MPI::UNSIGNED_SHORT.Unpack(temp_data.buffer, temp_data.buf_size, &request_id, 1,
                             temp_data.position, MPI::COMM_WORLD);

  MPI::UNSIGNED_SHORT.Unpack(temp_data.buffer, temp_data.buf_size, &calling_taskid, 1,
                             temp_data.position, MPI::COMM_WORLD);

  MPI::UNSIGNED_SHORT.Unpack(temp_data.buffer, temp_data.buf_size, &recur_level, 1,
                             temp_data.position, MPI::COMM_WORLD);

  bool col_flag, row_flag;
#ifdef THREEDIM
  bool slice_flag;
#endif
#ifdef THREEDIM
  set_recur_flags(params.recur_mask_flags[recur_level],col_flag,row_flag,slice_flag);
#else
  set_recur_flags(params.recur_mask_flags[recur_level],col_flag,row_flag);
#endif
  if (col_flag)
#ifdef THREEDIM
    index_data_size = params.pixel_nrows*params.pixel_nslices*params.seam_size;
#else
    index_data_size = params.pixel_nrows*params.seam_size;
#endif
  vector<Index> col_border_index_data(index_data_size);
  index_data_size = 1;
  if (row_flag)
#ifdef THREEDIM
    index_data_size = params.pixel_ncols*params.pixel_nslices*params.seam_size;
#else
    index_data_size = params.pixel_ncols*params.seam_size;
#endif
  vector<Index> row_border_index_data(index_data_size);
#ifdef THREEDIM
  index_data_size = 1;
  if (slice_flag)
    index_data_size = params.pixel_ncols*params.pixel_nrows*params.seam_size;
  vector<Index> slice_border_index_data(index_data_size);
#endif
  while (request_id > 0)
  {
    if (params.debug > 2)
    {
      params.log_fs << "Received request with ID value = " << request_id;
      params.log_fs << " from task " << calling_taskid << endl;
      params.log_fs << "with recur_level = " << recur_level << " and temp_data.position = " << temp_data.position << endl;
    }
    switch (request_id)
    {
      case 1:    MPI::UNSIGNED.Unpack(temp_data.buffer, temp_data.buf_size, &class_label_offset, 1,
                                      temp_data.position, MPI::COMM_WORLD);
                 MPI::UNSIGNED.Unpack(temp_data.buffer, temp_data.buf_size, &object_label_offset, 1,
                                      temp_data.position, MPI::COMM_WORLD);
                 if (class_label_offset > 0)
                 {
                   if (params.debug > 2)
                   {
                     params.log_fs << "Received do_class_label_offset request";
                     params.log_fs << " from task " << calling_taskid << " with" << endl;
                     params.log_fs << " class_label_offset = " << class_label_offset << endl;
                   }
                   do_class_label_offset(recur_level,class_label_offset,pixel_data,temp_data);
                   MPI::COMM_WORLD.Send(&class_label_offset, 1, MPI::UNSIGNED, calling_taskid, class_label_offset_tag);
                 }
                 if (object_label_offset > 0)
                 {
                   if (params.debug > 2)
                   {
                     params.log_fs << "Received do_class_label_offset request";
                     params.log_fs << " from task " << calling_taskid << " with" << endl;
                     params.log_fs << " object_label_offset = " << object_label_offset << endl;
                   }
                   do_object_label_offset(recur_level,object_label_offset,spatial_data,temp_data);
                   MPI::COMM_WORLD.Send(&object_label_offset, 1, MPI::UNSIGNED, calling_taskid, class_label_offset_tag);
                 }
                 if ((class_label_offset == 0) && (object_label_offset == 0))
                   MPI::COMM_WORLD.Send(&object_label_offset, 1, MPI::UNSIGNED, calling_taskid, class_label_offset_tag);
                 if (params.debug > 2)
                 {
                   params.log_fs << "Completed do_class_label_offset request" << endl;
                 }
                 break;
      case 2:    MPI::UNSIGNED.Unpack(temp_data.buffer, temp_data.buf_size, &value, 1,
                                            temp_data.position, MPI::COMM_WORLD);
                 MPI::UNSIGNED_SHORT.Unpack(temp_data.buffer, temp_data.buf_size, &nslevels, 1,
                                            temp_data.position, MPI::COMM_WORLD);
                 if (value == 0)
                 {
                   if (params.debug > 2)
                   {
                     params.log_fs << "Received update_region_class_info request with nslevels = " << nslevels;
                     params.log_fs << " from task " << calling_taskid << endl;
                   }
                   MPI::UNSIGNED.Unpack(temp_data.buffer, temp_data.buf_size, &region_classes_size, 1,
                                         temp_data.position, MPI::COMM_WORLD);
                   if (region_classes.size() < region_classes_size)
                     region_classes.resize(region_classes_size);
                   for (region_index = 0; region_index < region_classes_size; ++region_index)
                     region_classes[region_index].clear_region_info();
                   update_region_class_info(recur_level,section,nslevels,
                                            spatial_data,region_classes,temp_data);

                   int_buf_size = region_classes.size();
                   if (params.region_nb_objects_flag)
                     for (region_index = 0; region_index < region_classes_size; ++region_index)
                       int_buf_size += (1 + region_classes[region_index].get_region_objects_set_size());
                   check_buf_size(0,0,int_buf_size,0,0,temp_data);

                   int_buf_size = 0;
                   for (region_index = 0; region_index < region_classes_size; ++region_index)
                     region_classes[region_index].get_region_info(temp_data,
                                                               int_buf_size);

                   MPI::COMM_WORLD.Send(&int_buf_size, 1, MPI::UNSIGNED,
                                        calling_taskid, sum_pixel_gdissim_tag);
                   MPI::COMM_WORLD.Send(temp_data.int_buffer, int_buf_size, MPI::UNSIGNED,
                                        calling_taskid, sum_pixel_gdissim_tag);
                   if (params.debug > 3)
                   {
                     params.log_fs << endl << "After call to update_region_class_info, dump of the region data:" << endl << endl;
                     for (region_index = 0; region_index < region_classes_size; ++region_index)
                       if (region_classes[region_index].get_active_flag())
                         region_classes[region_index].print(region_classes);
                   }

                   if (params.debug > 2)
                   {
                     params.log_fs << "Completed update_region_class_info request" << endl;
                   }
                 }
                 else if (value == 1)
                 {
                   MPI::UNSIGNED.Unpack(temp_data.buffer, temp_data.buf_size, &max_region_object_label, 1,
                                        temp_data.position, MPI::COMM_WORLD);
                   if (max_region_object_label > 0)
                   {
                     MPI::UNSIGNED.Unpack(temp_data.buffer, temp_data.buf_size, &int_buf_size, 1,
                                          temp_data.position, MPI::COMM_WORLD);
                     MPI::UNSIGNED.Unpack(temp_data.buffer, temp_data.buf_size, &double_buf_size, 1,
                                          temp_data.position, MPI::COMM_WORLD);

                     region_objects_size = region_objects.size();
                     if (max_region_object_label > region_objects_size)
                     {
                       region_objects_size = max_region_object_label;
                       region_objects.resize(region_objects_size);
                     }
                     for (region_object_index = 0; region_object_index < region_objects_size; ++region_object_index)
                     {
                       region_objects[region_object_index].clear();
                       region_objects[region_object_index].set_label(region_object_index+1);
                     }
                     if (params.debug > 2)
                     {
                       params.log_fs << "Received update_region_object_info request with nslevels = " << nslevels;
                       params.log_fs << " from task " << calling_taskid << endl;
                     }
                     check_buf_size(0,0,int_buf_size,0,double_buf_size,temp_data);

                     MPI::COMM_WORLD.Recv(temp_data.int_buffer, int_buf_size, MPI::UNSIGNED,
                                          calling_taskid, temp_data_tag);
                     MPI::COMM_WORLD.Recv(temp_data.double_buffer, double_buf_size, MPI::DOUBLE,
                                          calling_taskid, temp_data_tag);

                     nb_objects = int_buf_size/2;
                     int_buf_position = double_buf_position = 0;
                     for (pixel_index = 0; pixel_index < nb_objects; ++pixel_index)
                     {
                       region_object_label = temp_data.int_buffer[int_buf_position++];
                       region_object_index = region_object_label-1;
                       region_objects[region_object_index].set_active_flag(true);
                       region_objects[region_object_index].set_label(region_object_label);
                       region_objects[region_object_index].set_data(temp_data,
                                                                    int_buf_position,double_buf_position);
                     }

                     update_region_object_info(recur_level,section,nslevels,
                                             spatial_data,region_objects,temp_data);
                     int_buf_size = nregions;
                     double_buf_size = nregions;
                     check_buf_size(0,0,int_buf_size,0,double_buf_size,temp_data);

                     int_buf_size = double_buf_size = 0;
                     for (region_object_index = 0; region_object_index < region_objects_size; ++region_object_index)
                       if (region_objects[region_object_index].get_active_flag())
                         region_objects[region_object_index].load_boundary_npix(temp_data,int_buf_size);

                     MPI::COMM_WORLD.Send(&int_buf_size, 1, MPI::UNSIGNED,
                                          calling_taskid, sum_pixel_gdissim_tag);
                     MPI::COMM_WORLD.Send(temp_data.int_buffer, int_buf_size, MPI::UNSIGNED,
                                          calling_taskid, sum_pixel_gdissim_tag);
                     if (params.debug > 3)
                     {
                       params.log_fs << endl << "After call to update_region_object_info, dump of the region_object data:" << endl << endl;
                       for (region_object_index = 0; region_object_index < region_objects_size; ++region_object_index)
                         if (region_objects[region_object_index].get_active_flag())
                           region_objects[region_object_index].print(region_objects);
                     }

                     if (params.debug > 2)
                     {
                       params.log_fs << "Completed update_region_object_info request" << endl;
                     }
                   }
                 }
                 else // if (value == 3)
                 {
                   MPI::UNSIGNED.Unpack(temp_data.buffer, temp_data.buf_size, &max_region_label, 1,
                                              temp_data.position, MPI::COMM_WORLD);
                   if (max_region_label > 0)
                   {
                     MPI::UNSIGNED.Unpack(temp_data.buffer, temp_data.buf_size, &int_buf_size, 1,
                                          temp_data.position, MPI::COMM_WORLD);
                     MPI::UNSIGNED.Unpack(temp_data.buffer, temp_data.buf_size, &double_buf_size, 1,
                                          temp_data.position, MPI::COMM_WORLD);

                     region_classes_size = region_classes.size();
                     if (max_region_label > region_classes_size)
                     {
                       region_classes_size = max_region_label;
                       region_classes.resize(region_classes_size);
                     }
                     for (region_index = 0; region_index < region_classes_size; ++region_index)
                     {
                       region_classes[region_index].clear();
                       region_classes[region_index].set_label(region_index+1);
                     }
                     if (params.debug > 2)
                     {
                       params.log_fs << "Received update_sum_pixel_gdissim request";
                       params.log_fs << " from task " << calling_taskid << endl;
                     }
                     check_buf_size(0,0,int_buf_size,0,double_buf_size,temp_data);

                     MPI::COMM_WORLD.Recv(temp_data.int_buffer, int_buf_size, MPI::UNSIGNED,
                                          calling_taskid, temp_data_tag);
                     MPI::COMM_WORLD.Recv(temp_data.double_buffer, double_buf_size, MPI::DOUBLE,
                                          calling_taskid, temp_data_tag);

                     nregions = int_buf_size/2;
                     int_buf_position = double_buf_position = 0;
                     for (short_buf_position = 0; short_buf_position < nregions; ++short_buf_position)
                     {
                       region_label = temp_data.int_buffer[int_buf_position++];
                       region_index = region_label-1;
                       region_classes[region_index].set_active_flag(true);
                       region_classes[region_index].set_label(region_label);
                       region_classes[region_index].set_data(temp_data,int_buf_position,
                                                          double_buf_position);
                     }

                     update_sum_pixel_gdissim(recur_level,section,pixel_data,region_classes,temp_data);
                     double_buf_size = nregions;
                     check_buf_size(0,0,0,0,double_buf_size,temp_data);

                     double_buf_size = 0;
                     for (region_index = 0; region_index < region_classes_size; ++region_index)
                       if (region_classes[region_index].get_active_flag())
                         temp_data.double_buffer[double_buf_size++] = region_classes[region_index].get_sum_pixel_gdissim();

                     MPI::COMM_WORLD.Send(&double_buf_size, 1, MPI::UNSIGNED,
                                          calling_taskid, sum_pixel_gdissim_tag);
                     MPI::COMM_WORLD.Send(temp_data.double_buffer, double_buf_size, MPI::DOUBLE,
                                          calling_taskid, sum_pixel_gdissim_tag);
                     if (params.debug > 3)
                     {
                       params.log_fs << endl << "After call to update_sum_pixel_gdissim, dump of the region data:";
                       params.log_fs << endl << endl;
                       for (region_index = 0; region_index < region_classes_size; ++region_index)
                         if (region_classes[region_index].get_active_flag())
                           region_classes[region_index].print(region_classes);
                     }

                     if (params.debug > 2)
                     {
                       params.log_fs << "Completed update_sum_pixel_gdissim request" << endl;
                     }
                   }
                 }
                 break;
      case 3:    // Obsolete case
                 break;
      case 4:    MPI::UNSIGNED.Unpack(temp_data.buffer, temp_data.buf_size, &value, 1,
                                      temp_data.position, MPI::COMM_WORLD);
                 MPI::UNSIGNED.Unpack(temp_data.buffer, temp_data.buf_size, &int_buf_size, 1,
                                      temp_data.position, MPI::COMM_WORLD);
                 check_buf_size(0,0,int_buf_size,0,0,temp_data);
                 if (value == 0)
                 {
                   nb_region_pairs = int_buf_size/2;
                   if (params.debug > 2)
                   {
                     params.log_fs << "Received do_region_class_relabel request for region_label";
                     params.log_fs << " from task " << calling_taskid << " with nb_region_pairs = " << nb_region_pairs << endl;
                   }
                   MPI::COMM_WORLD.Recv(temp_data.int_buffer, int_buf_size, MPI::UNSIGNED,
                                        calling_taskid, temp_data_tag);
                   region_relabel_pairs.clear();
                   for (region_index = 0; region_index < int_buf_size; region_index += 2)
                     region_relabel_pairs.insert(make_pair(temp_data.int_buffer[region_index],
                                                 temp_data.int_buffer[region_index+1]));

                   do_region_class_relabel(recur_level,section,region_relabel_pairs,pixel_data,temp_data);
                   region_relabel_pairs.clear();
                   MPI::COMM_WORLD.Send(&int_buf_size, 1, MPI::UNSIGNED, calling_taskid, region_relabel_tag);
                   if (params.debug > 3)
                   {
                     params.log_fs << endl << "After completing do_region_class_relabel, dump of the pixel data:" << endl << endl;
                     pixel_data_size = pixel_data.size();
                     for (pixel_index = 0; pixel_index < pixel_data_size; ++pixel_index)
                     {
                       region_label = pixel_data[pixel_index].get_region_label();
                       if (region_label != 0)
                       {
                         params.log_fs << "Element " << pixel_index << " is associated with region label ";
                         params.log_fs << region_label << endl;
                       }
                     }
                   }
                   if (params.debug > 2)
                   {
                     params.log_fs << "Completed do_region_class_relabel request" << endl;
                   }
                 }
                 else if (value == 1)
                 {
                   MPI::INT.Unpack(temp_data.buffer, temp_data.buf_size, &ncols, 1,
                                   temp_data.position, MPI::COMM_WORLD);
                   MPI::INT.Unpack(temp_data.buffer, temp_data.buf_size, &nrows, 1,
                                   temp_data.position, MPI::COMM_WORLD);
#ifdef THREEDIM
                   MPI::INT.Unpack(temp_data.buffer, temp_data.buf_size, &nslices, 1,
                                   temp_data.position, MPI::COMM_WORLD);
#endif
                   nb_region_pairs = int_buf_size/2;
                   if (params.debug > 2)
                   {
                     params.log_fs << "Received region_object_relabel request for region_object_label";
                     params.log_fs << " from task " << calling_taskid << " with nb_region_pairs = " << nb_region_pairs << endl;
                   }
                   MPI::COMM_WORLD.Recv(temp_data.int_buffer, int_buf_size, MPI::UNSIGNED,
                                        calling_taskid, temp_data_tag);

                   region_object_relabel_pairs.clear();
                   for (region_object_index = 0; region_object_index < int_buf_size; region_object_index += 2)
                     region_object_relabel_pairs.insert(make_pair(temp_data.int_buffer[region_object_index],
                                                      temp_data.int_buffer[region_object_index+1]));
#ifdef THREEDIM
                   do_region_object_relabel(recur_level,section,region_object_relabel_pairs,
                                            spatial_data,ncols,nrows,nslices,temp_data);
#else
                   do_region_object_relabel(recur_level,section,region_object_relabel_pairs,
                                            spatial_data,ncols,nrows,temp_data);
#endif
                   region_object_relabel_pairs.clear();

                   MPI::COMM_WORLD.Send(&int_buf_size, 1, MPI::UNSIGNED, calling_taskid, region_relabel_tag);
                   if (params.debug > 2)
                   {
                     params.log_fs << "Completed region_object_relabel request" << endl;
                   }
                 }
                 break;
      case 5:    MPI::UNSIGNED.Unpack(temp_data.buffer, temp_data.buf_size, &nb_objects, 1,
                                      temp_data.position, MPI::COMM_WORLD);
                 MPI::INT.Unpack(temp_data.buffer, temp_data.buf_size, &ncols, 1,
                                      temp_data.position, MPI::COMM_WORLD);
                 MPI::INT.Unpack(temp_data.buffer, temp_data.buf_size, &nrows, 1,
                                      temp_data.position, MPI::COMM_WORLD);
#ifdef THREEDIM
                 MPI::INT.Unpack(temp_data.buffer, temp_data.buf_size, &nslices, 1,
                                      temp_data.position, MPI::COMM_WORLD);
#endif
                 if (nb_objects > 0)
                 {
                   if (params.debug > 2)
                   {
                     params.log_fs << "Received connected region data initialization request";
                     params.log_fs << " from task " << calling_taskid << endl;
                   }
                   region_objects_size = region_objects.size();
                   if (nb_objects > region_objects_size)
                   {
                     region_objects_size = nb_objects;
                     region_objects.resize(region_objects_size,RegionObject());
                   }
                   for (region_object_index = 0; region_object_index < nb_objects; ++region_object_index)
                   {
                     region_objects[region_object_index].clear();
                     region_objects[region_object_index].set_label(region_object_index + 1);
                   }
#ifdef THREEDIM
                   do_region_objects_init(recur_level,section,ncols,nrows,nslices,
                                          pixel_data,spatial_data,region_objects,temp_data);
#else
                   do_region_objects_init(recur_level,section,ncols,nrows,
                                          pixel_data,spatial_data,region_objects,temp_data);
#endif
                   region_objects_size = region_objects.size();

                   nb_objects = 0;
                   for (region_object_index = 0; region_object_index < region_objects_size; ++region_object_index)
                     if (region_objects[region_object_index].get_active_flag())
                       nb_objects++;

                   int_buf_size = 2*nb_objects;
                   double_buf_size = nb_objects*params.nbands;
                   if (params.region_sumsq_flag)
                     double_buf_size += nb_objects*params.nbands;
                   if (params.region_sumxlogx_flag)
                     double_buf_size += nb_objects*params.nbands;
                   if (params.std_dev_image_flag)
                     double_buf_size += nb_objects*params.nbands;
                   check_buf_size(0,0,int_buf_size,0,double_buf_size,temp_data);

                   int_buf_position = double_buf_position = 0;
                   for (region_object_index = 0; region_object_index < region_objects_size; ++region_object_index)
                     if (region_objects[region_object_index].get_active_flag())
                       region_objects[region_object_index].load_data(temp_data,
                                                                     int_buf_position,double_buf_position);

                   MPI::COMM_WORLD.Send(&int_buf_position, 1, MPI::UNSIGNED,
                                        calling_taskid, region_classes_init_tag);
                   MPI::COMM_WORLD.Send(&double_buf_position, 1, MPI::UNSIGNED,
                                        calling_taskid, region_classes_init_tag);

                   if (int_buf_position > 0)
                   {
                     MPI::COMM_WORLD.Send(temp_data.int_buffer, int_buf_position, MPI::UNSIGNED,
                                          calling_taskid, region_classes_init_tag);
                     MPI::COMM_WORLD.Send(temp_data.double_buffer, double_buf_position, MPI::DOUBLE,
                                          calling_taskid, region_classes_init_tag);
                   }
                   if (params.debug > 2)
                   {
                     params.log_fs << "Completed connected region data initialization" << endl;
                   }
                 }
                 else
                 {
                   if (params.debug > 2)
                   {
                     params.log_fs << "Received region data initialization request";
                     params.log_fs << " from task " << calling_taskid << endl;
#ifdef THREEDIM
                     params.log_fs << " with ncols = " << ncols << ", nrows = " << nrows << " and nslices = " << nslices << endl;
#else
                     params.log_fs << " with ncols = " << ncols << " and nrows = " << nrows << endl;
#endif
                   }
                   region_classes_size = region_classes.size();
                   for (region_index = 0; region_index < region_classes_size; ++region_index)
                   {
                     region_classes[region_index].clear();
                     region_classes[region_index].set_label(region_index + 1);
                   }
#ifdef THREEDIM
                   max_region_label = do_region_classes_init(recur_level,section,ncols,nrows,nslices,
                                                             pixel_data,region_classes,temp_data);
#else
                   max_region_label = do_region_classes_init(recur_level,section,ncols,nrows,
                                                             pixel_data,region_classes,temp_data);
#endif
                   region_classes_size = region_classes.size();

                   short_buf_size = int_buf_size = float_buf_size = double_buf_size = 0;
                   nregions = 0;
                   for (region_index = 0; region_index < region_classes_size; ++region_index)
                     if (region_classes[region_index].get_active_flag())
                     {
                       nregions++;
                       int_buf_size += region_classes[region_index].get_nghbrs_label_set_size(); // neighbor region label
                     }
                   short_buf_size += nregions; // initial_merge_flag
                   int_buf_size += 3*nregions;  // nghbrs_label_set_size(), label & npix
                   double_buf_size = params.nbands*nregions; // sum[band]
                   if (params.region_sumsq_flag)
                     double_buf_size += params.nbands*nregions; // sumsq[band]
                   if (params.region_sumxlogx_flag)
                     double_buf_size += params.nbands*nregions; // sumxlogx[band] 
                   if (params.std_dev_image_flag)
                     double_buf_size += params.nbands*nregions; // sum_pixel_std_dev[band]
                   if (params.edge_image_flag)
                     float_buf_size += nregions;  // max_edge_value
                   double_buf_size += nregions; // merge_threshold
                   check_buf_size(0,short_buf_size,int_buf_size,float_buf_size,double_buf_size,temp_data);

                   short_buf_position = int_buf_position = float_buf_position = double_buf_position = 0;
                   for (region_index = 0; region_index < region_classes_size; ++region_index)
                     if (region_classes[region_index].get_active_flag())
                     {
                       temp_data.int_buffer[int_buf_position++] = region_classes[region_index].get_label();
                       region_classes[region_index].load_all_data(temp_data,short_buf_position,int_buf_position,
                                                                  float_buf_position,double_buf_position);
                     }

                   if ((short_buf_position > short_buf_size) || (int_buf_position > int_buf_size) ||
                       (float_buf_position > float_buf_size) || (double_buf_position > double_buf_size))
                   {
                     if (params.debug > 0)
                     {
                       params.log_fs << "WARNING (case 5):  Temp buffer overflow:" << endl;
                       params.log_fs << "short_buf_size = " << short_buf_size << " and short_buf_position = " << short_buf_position << endl;
                       params.log_fs << "int_buf_size = " << int_buf_size << " and int_buf_position = " << int_buf_position << endl;
                       params.log_fs << "float_buf_size = " << float_buf_size << " and float_buf_position = " << float_buf_position << endl;
                       params.log_fs << "double_buf_size = " << double_buf_size << " and double_buf_position = " << double_buf_position << endl;
                     }
                     else
                     {
                       cout << "WARNING (case 5):  Temp buffer overflow:" << endl;
                       cout << "short_buf_size = " << short_buf_size << " and short_buf_position = " << short_buf_position << endl;
                       cout << "int_buf_size = " << int_buf_size << " and int_buf_position = " << int_buf_position << endl;
                       cout << "float_buf_size = " << float_buf_size << " and float_buf_position = " << float_buf_position << endl;
                       cout << "double_buf_size = " << double_buf_size << " and double_buf_position = " << double_buf_position << endl;
                     }
                   }
                 // Don't send short_buf_position or temp_data.short_buffer - not needed by update_all_data in do_region_classes_init!
                   MPI::COMM_WORLD.Send(&max_region_label, 1, MPI::UNSIGNED, calling_taskid, region_classes_init_tag);
                   MPI::COMM_WORLD.Send(&nregions, 1, MPI::UNSIGNED, calling_taskid, region_classes_init_tag);
                   MPI::COMM_WORLD.Send(&int_buf_position, 1, MPI::UNSIGNED, calling_taskid, region_classes_init_tag);
                   MPI::COMM_WORLD.Send(&float_buf_position, 1, MPI::UNSIGNED, calling_taskid, region_classes_init_tag);
                   MPI::COMM_WORLD.Send(&double_buf_position, 1, MPI::UNSIGNED, calling_taskid, region_classes_init_tag);
                   if (nregions > 0)
                   {
                     MPI::COMM_WORLD.Send(temp_data.int_buffer, int_buf_position, MPI::UNSIGNED, calling_taskid, region_classes_init_tag);
                     MPI::COMM_WORLD.Send(temp_data.float_buffer, float_buf_position, MPI::FLOAT, calling_taskid, region_classes_init_tag);
                     MPI::COMM_WORLD.Send(temp_data.double_buffer, double_buf_position, MPI::DOUBLE, calling_taskid, region_classes_init_tag);
                   }
                   if (params.debug > 2)
                   {
                     params.log_fs << "Completed region data initialization" << endl;
                   }
                 }
                 break;
      case 6:    if (params.debug > 2)
                 {
                   params.log_fs << "Received get_border_index_data (pixel) request";
                   params.log_fs << " from task " << calling_taskid << endl;
                 }
                 MPI::UNSIGNED.Unpack(temp_data.buffer, temp_data.buf_size, &value, 1,
                                      temp_data.position, MPI::COMM_WORLD);
                 border_flag = (unsigned char) value;

               // border flag designations: 1 => col only, 2 => row only, 3=> col & row,
               // 4=> slice only, 5=> col & slice, 6 => row & slice, 7 => col, row & slice
                 col_border_flag = false;
                 row_border_flag = false;
#ifdef THREEDIM
                 slice_border_flag = false;
#endif
                 switch (border_flag)
                 {
                   case 1:  col_border_flag = true;
                            break;
                   case 2:  row_border_flag = true;
                            break;
                   case 3:  col_border_flag = true;
                            row_border_flag = true;
                            break;
#ifdef THREEDIM
                   case 4:  slice_border_flag = true;
                            break;
                   case 5:  col_border_flag = true;
                            slice_border_flag = true;
                            break;
                   case 6:  row_border_flag = true;
                            slice_border_flag = true;
                            break;
                   case 7:  col_border_flag = true;
                            row_border_flag = true;
                            slice_border_flag = true;
                            break;
#endif
                   default: params.log_fs << "ERROR (get_border_index_data): Invalid value for border_flag" << endl;
                            break;
                 }

                 MPI::INT.Unpack(temp_data.buffer, temp_data.buf_size, &ncols, 1,
                                      temp_data.position, MPI::COMM_WORLD);
                 MPI::INT.Unpack(temp_data.buffer, temp_data.buf_size, &nrows, 1,
                                      temp_data.position, MPI::COMM_WORLD);
#ifdef THREEDIM
                 MPI::INT.Unpack(temp_data.buffer, temp_data.buf_size, &nslices, 1,
                                      temp_data.position, MPI::COMM_WORLD);
#endif
                 index_data_size = 1;
                 if (col_border_flag)
#ifdef THREEDIM
                   index_data_size = nrows*nslices*params.seam_size;
#else
                   index_data_size = nrows*params.seam_size;
#endif
                 if (col_border_index_data.size() < index_data_size)
                   col_border_index_data.resize(index_data_size,Index());
                 index_data_size = 1;
                 if (row_border_flag)
#ifdef THREEDIM
                   index_data_size = ncols*nslices*params.seam_size;
#else
                   index_data_size = ncols*params.seam_size;
#endif
                 if (row_border_index_data.size() < index_data_size)
                   row_border_index_data.resize(index_data_size,Index());
#ifdef THREEDIM
                 index_data_size = 1;
                 if (slice_border_flag)
                   index_data_size = ncols*nrows*params.seam_size;
                 if (slice_border_index_data.size() < index_data_size)
                   slice_border_index_data.resize(index_data_size,Index());

                 get_border_index_data(recur_level,section,border_flag,pixel_data,ncols,nrows,nslices,
                                       col_border_index_data,row_border_index_data,slice_border_index_data,temp_data);
#else
                 get_border_index_data(recur_level,section,border_flag,pixel_data,ncols,nrows,
                                       col_border_index_data,row_border_index_data,temp_data);
#endif
                 short_buf_size = int_buf_size = float_buf_size = 0;
                 short_factor = 1; int_factor = 2; float_factor = 1;
                 if (col_border_flag)
                 {
#ifdef THREEDIM
                   short_buf_size = short_factor*params.seam_size*nrows*nslices;
                   int_buf_size = int_factor*params.seam_size*nrows*nslices;
                   float_buf_size = float_factor*params.seam_size*nrows*nslices;
#else
                   short_buf_size = short_factor*params.seam_size*nrows;
                   int_buf_size = int_factor*params.seam_size*nrows;
                   float_buf_size = float_factor*params.seam_size*nrows;
#endif
                 }
                 if (row_border_flag)
                 {
#ifdef THREEDIM
                   short_buf_size += short_factor*params.seam_size*ncols*nslices;
                   int_buf_size += int_factor*params.seam_size*ncols*nslices;
                   float_buf_size += float_factor*params.seam_size*ncols*nslices;
#else
                   short_buf_size += short_factor*params.seam_size*ncols;
                   int_buf_size += int_factor*params.seam_size*ncols;
                   float_buf_size += float_factor*params.seam_size*ncols;
#endif
                 }
#ifdef THREEDIM
                 if (slice_border_flag)
                 {
                   short_buf_size += short_factor*params.seam_size*ncols*nrows;
                   int_buf_size += int_factor*params.seam_size*ncols*nrows;
                   float_buf_size += float_factor*params.seam_size*ncols*nrows;
                 }
#endif
                 check_buf_size(0,short_buf_size,int_buf_size,float_buf_size,0,temp_data);
                 short_buf_position = 0; int_buf_position = 0; float_buf_position = 0;
                 if (col_border_flag)
                 {
#ifdef THREEDIM
                   for (index = 0; index < (params.seam_size*nrows*nslices); ++index)
#else
                   for (index = 0; index < (params.seam_size*nrows); ++index)
#endif
                     col_border_index_data[index].load_data(false,temp_data,short_buf_position,int_buf_position,float_buf_position);
                 }
                 if (row_border_flag)
                 {
#ifdef THREEDIM
                   for (index = 0; index < (params.seam_size*ncols*nslices); ++index)
#else
                   for (index = 0; index < (params.seam_size*ncols); ++index)
#endif
                     row_border_index_data[index].load_data(false,temp_data,short_buf_position,int_buf_position,float_buf_position);
                 }
#ifdef THREEDIM
                 if (slice_border_flag)
                 {
                   for (index = 0; index < (params.seam_size*ncols*nrows); ++index)
                     slice_border_index_data[index].load_data(false,temp_data,short_buf_position,int_buf_position,float_buf_position);
                 }
#endif
                 if ((short_buf_position > short_buf_size) || (int_buf_position > int_buf_size) || (float_buf_position > float_buf_size))
                 {
                   if (params.debug > 0)
                   {
                     params.log_fs << "WARNING (case 6):  Temp buffer overflow:" << endl;
                     params.log_fs << "short_buf_size = " << short_buf_size << " and short_buf_position = " << short_buf_position << endl;
                     params.log_fs << "int_buf_size = " << int_buf_size << " and int_buf_position = " << int_buf_position << endl;
                     params.log_fs << "float_buf_size = " << float_buf_size << " and float_buf_position = " << float_buf_position << endl;
                   }
                   else
                   {
                     cout << "WARNING (case 6):  Temp buffer overflow:" << endl;
                     cout << "short_buf_size = " << short_buf_size << " and short_buf_position = " << short_buf_position << endl;
                     cout << "int_buf_size = " << int_buf_size << " and int_buf_position = " << int_buf_position << endl;
                     cout << "float_buf_size = " << float_buf_size << " and float_buf_position = " << float_buf_position << endl;
                   }
                 }

#ifdef TIME_IT
                 MPI::COMM_WORLD.Send(&short_buf_size, 1, MPI::UNSIGNED, calling_taskid, border_index_data_tag);
#endif
                 MPI::COMM_WORLD.Send(temp_data.short_buffer, short_buf_size, MPI::UNSIGNED_SHORT, calling_taskid, border_index_data_tag);
                 MPI::COMM_WORLD.Send(temp_data.int_buffer, int_buf_size, MPI::UNSIGNED, calling_taskid, border_index_data_tag);
                 if (params.edge_image_flag)
                   MPI::COMM_WORLD.Send(temp_data.float_buffer, float_buf_size, MPI::FLOAT, calling_taskid, border_index_data_tag);
                 if (params.debug > 2)
                 {
                   params.log_fs << "Sent border_index_data information";
                   params.log_fs << " to task " << calling_taskid << endl;
                 }
                 break;
      case 7:    if (params.debug > 2)
                 {
                   params.log_fs << "Received region label map update request";
                   params.log_fs << " from task " << calling_taskid << endl;
                 }
                 spatial_data.update_region_label_map(recur_level,section,pixel_data,temp_data);
                 MPI::COMM_WORLD.Send(&pixel_index, 1, MPI::UNSIGNED, calling_taskid, update_region_label_map_tag);
                 if (params.debug > 2)
                 {
                   params.log_fs << "Sent region label map update confirmation";
                   params.log_fs << " to task " << calling_taskid << endl;
                 }
                 break;
      case 8:    if (params.debug > 2)
                 {
                   params.log_fs << "Received write region label map request";
                   params.log_fs << " from task " << calling_taskid << endl;
                 }
#ifdef THREEDIM
                 MPI::UNSIGNED.Unpack(temp_data.buffer, temp_data.buf_size, &slice, 1,
                                      temp_data.position, MPI::COMM_WORLD);
#endif
                 slice_size = params.onb_ncols*params.onb_nrows;
                 int_buf_size = slice_size;
                 short_buf_size = double_buf_size = 0;
                 if (params.object_labels_map_flag)
                   int_buf_size = 2*slice_size;
                 check_buf_size(0,short_buf_size,int_buf_size,0,double_buf_size,
                                temp_data);

                 for (row = 0; row < params.onb_nrows; ++row)
                   for (col = 0; col < params.onb_ncols; ++col)
                   {
#ifdef THREEDIM
                     pixel_index = col + row*params.onb_ncols + slice*params.onb_ncols*params.onb_nrows;
#else
                     pixel_index = col + row*params.onb_ncols;
#endif
                     pixel_index2 = col + row*params.onb_ncols;
                     temp_data.int_buffer[pixel_index2] = spatial_data.get_region_class_label(pixel_index);
                     if (params.object_labels_map_flag)
                     {
                       temp_data.int_buffer[slice_size + pixel_index2] =
                                                          spatial_data.get_region_object_label(pixel_index);
                     }
                   }

#ifdef TIME_IT
                 MPI::COMM_WORLD.Send(&int_buf_size, 1, MPI::UNSIGNED, calling_taskid, write_region_label_map_tag);
#endif
                 MPI::COMM_WORLD.Send(temp_data.int_buffer, int_buf_size, MPI::UNSIGNED,
                                      calling_taskid, write_region_label_map_tag);
                 if (params.debug > 2)
                 {
                   params.log_fs << "Completed write region label map request";
                   params.log_fs << " from task " << calling_taskid << endl;
                 }
                 break;
      case 9:    if (params.debug > 2)
                 {
                   params.log_fs << "Received get_border_index_data (spatial_data) request";
                   params.log_fs << " from task " << calling_taskid << endl;
                 }
                 MPI::UNSIGNED.Unpack(temp_data.buffer, temp_data.buf_size, &value, 1,
                                      temp_data.position, MPI::COMM_WORLD);
                 border_flag = (unsigned char) value;

               // border flag designations: 1 => col only, 2 => row only, 3=> col & row,
               // 4=> slice only, 5=> col & slice, 6 => row & slice, 7 => col, row & slice
                 col_border_flag = false;
                 row_border_flag = false;
#ifdef THREEDIM
                 slice_border_flag = false;
#endif
                 switch (border_flag)
                 {
                   case 1:  col_border_flag = true;
                            break;
                   case 2:  row_border_flag = true;
                            break;
                   case 3:  col_border_flag = true;
                            row_border_flag = true;
                            break;
#ifdef THREEDIM
                   case 4:  slice_border_flag = true;
                            break;
                   case 5:  col_border_flag = true;
                            slice_border_flag = true;
                            break;
                   case 6:  row_border_flag = true;
                            slice_border_flag = true;
                            break;
                   case 7:  col_border_flag = true;
                            row_border_flag = true;
                            slice_border_flag = true;
                            break;
#endif
                   default: params.log_fs << "ERROR (get_border_index_data): Invalid value for border_flag" << endl;
                            break;
                 }

                 MPI::INT.Unpack(temp_data.buffer, temp_data.buf_size, &ncols, 1,
                                      temp_data.position, MPI::COMM_WORLD);
                 MPI::INT.Unpack(temp_data.buffer, temp_data.buf_size, &nrows, 1,
                                      temp_data.position, MPI::COMM_WORLD);
#ifdef THREEDIM
                 MPI::INT.Unpack(temp_data.buffer, temp_data.buf_size, &nslices, 1,
                                      temp_data.position, MPI::COMM_WORLD);
#endif
                 index_data_size = 1;
                 if (col_border_flag)
#ifdef THREEDIM
                   index_data_size = nrows*nslices*params.seam_size;
#else
                   index_data_size = nrows*params.seam_size;
#endif
                 if (col_border_index_data.size() < index_data_size)
                   col_border_index_data.resize(index_data_size,Index());
                 index_data_size = 1;
                 if (row_border_flag)
#ifdef THREEDIM
                   index_data_size = ncols*nslices*params.seam_size;
#else
                   index_data_size = ncols*params.seam_size;
#endif
                 if (row_border_index_data.size() < index_data_size)
                   row_border_index_data.resize(index_data_size,Index());
#ifdef THREEDIM
                 index_data_size = 1;
                 if (slice_border_flag)
                   index_data_size = ncols*nrows*params.seam_size;
                 if (slice_border_index_data.size() < index_data_size)
                   slice_border_index_data.resize(index_data_size,Index());

                 get_border_index_data(recur_level,section,border_flag,spatial_data,ncols,nrows,nslices,
                                       col_border_index_data,row_border_index_data,slice_border_index_data,temp_data);
#else
                 get_border_index_data(recur_level,section,border_flag,spatial_data,ncols,nrows,
                                       col_border_index_data,row_border_index_data,temp_data);
#endif
                 short_buf_size = int_buf_size = 0;
                 short_factor = 2; int_factor = 3;
                 if (col_border_flag)
                 {
#ifdef THREEDIM
                   short_buf_size = short_factor*params.seam_size*nrows*nslices;
                   int_buf_size = int_factor*params.seam_size*nrows*nslices;
#else
                   short_buf_size = short_factor*params.seam_size*nrows;
                   int_buf_size = int_factor*params.seam_size*nrows;
#endif
                 }
                 if (row_border_flag)
                 {
#ifdef THREEDIM
                   short_buf_size += short_factor*params.seam_size*ncols*nslices;
                   int_buf_size += int_factor*params.seam_size*ncols*nslices;
#else
                   short_buf_size += short_factor*params.seam_size*ncols;
                   int_buf_size += int_factor*params.seam_size*ncols;
#endif
                 }
#ifdef THREEDIM
                 if (slice_border_flag)
                 {
                   short_buf_size += short_factor*params.seam_size*ncols*nrows;
                   int_buf_size += int_factor*params.seam_size*ncols*nrows;
                 }
#endif
                 check_buf_size(0,short_buf_size,int_buf_size,0,0,temp_data);
                 short_buf_position = 0; int_buf_position = 0;
                 float_buf_position = 0; // NOTE: float_buf_position not actually used in this case
                 if (col_border_flag)
                 {
#ifdef THREEDIM
                   for (index = 0; index < (params.seam_size*nrows*nslices); ++index)
#else
                   for (index = 0; index < (params.seam_size*nrows); ++index)
#endif
                     col_border_index_data[index].load_data(true,temp_data,short_buf_position,int_buf_position,float_buf_position);
                 }
                 if (row_border_flag)
                 {
#ifdef THREEDIM
                   for (index = 0; index < (params.seam_size*ncols*nslices); ++index)
#else
                   for (index = 0; index < (params.seam_size*ncols); ++index)
#endif
                     row_border_index_data[index].load_data(true,temp_data,short_buf_position,int_buf_position,float_buf_position);
                 }
#ifdef THREEDIM
                 if (slice_border_flag)
                 {
                   for (index = 0; index < (params.seam_size*ncols*nrows); ++index)
                     slice_border_index_data[index].load_data(true,temp_data,short_buf_position,int_buf_position,float_buf_position);
                 }
#endif
                 if ((short_buf_position > short_buf_size) || (int_buf_position > int_buf_size))
                 {
                   if (params.debug > 0)
                   {
                     params.log_fs << "WARNING (case 9):  Temp buffer overflow:" << endl;
                     params.log_fs << "short_buf_size = " << short_buf_size << " and short_buf_position = " << short_buf_position << endl;
                     params.log_fs << "int_buf_size = " << int_buf_size << " and int_buf_position = " << int_buf_position << endl;
                   }
                   else
                   {
                     cout << "WARNING (case 9):  Temp buffer overflow:" << endl;
                     cout << "short_buf_size = " << short_buf_size << " and short_buf_position = " << short_buf_position << endl;
                     cout << "int_buf_size = " << int_buf_size << " and int_buf_position = " << int_buf_position << endl;
                   }
                 }

#ifdef TIME_IT
                 MPI::COMM_WORLD.Send(&short_buf_size, 1, MPI::UNSIGNED, calling_taskid, border_index_data_tag2);
#endif
                 MPI::COMM_WORLD.Send(temp_data.short_buffer, short_buf_size, MPI::UNSIGNED_SHORT,
                                      calling_taskid, border_index_data_tag2);
                 MPI::COMM_WORLD.Send(temp_data.int_buffer, int_buf_size, MPI::UNSIGNED,
                                      calling_taskid, border_index_data_tag2);
                 if (params.debug > 2)
                 {
                   params.log_fs << "Sent border_index_data information";
                   params.log_fs << " to task " << calling_taskid << endl;
                 }
                 break;
      case 10:   if (params.debug > 2)
                 {
                   params.log_fs << "Received write boundary map request";
                   params.log_fs << " from task " << calling_taskid << endl;
                 }
#ifdef THREEDIM
                 MPI::UNSIGNED.Unpack(temp_data.buffer, temp_data.buf_size, &slice, 1,
                                      temp_data.position, MPI::COMM_WORLD);
#endif
                 slice_size = params.onb_ncols*params.onb_nrows;
                 check_buf_size(0,slice_size,0,0,0,temp_data);

                 short_buf_position = 0;
                 for (row = 0; row < params.onb_nrows; ++row)
                  for (col = 0; col < params.onb_ncols; ++col)
                  {
#ifdef THREEDIM
                    pixel_index = col + row*params.onb_ncols + slice*params.onb_ncols*params.onb_nrows;
#else
                    pixel_index = col + row*params.onb_ncols;
#endif
                    temp_data.short_buffer[short_buf_position++] = spatial_data.get_boundary_map(pixel_index);
                  }

#ifdef TIME_IT
                 MPI::COMM_WORLD.Send(&short_buf_position, 1, MPI::UNSIGNED, calling_taskid, write_boundary_map_tag);
#endif
                 MPI::COMM_WORLD.Send(temp_data.short_buffer, slice_size, MPI::UNSIGNED_SHORT,
                                      calling_taskid, write_boundary_map_tag);
                 if (params.debug > 2)
                 {
                   params.log_fs << "Completed write boundary map request";
                   params.log_fs << " from task " << calling_taskid << endl;
                 }
                 break;
      case 11:   if (params.debug > 2)
                 {
                   params.log_fs << "Received undefined request";
                   params.log_fs << " from task " << calling_taskid;
                 }
                 break;
      case 12:   if (params.debug > 2)
                 {
                   params.log_fs << "Received undefined request";
                   params.log_fs << " from task " << calling_taskid;
                 }
                 break;
      case 13:   if (params.debug > 2)
                 {
                   params.log_fs << "Received undefined request";
                   params.log_fs << " from task " << calling_taskid;
                 }
                 break;
      case 14:   if (params.debug > 2)
                 {
                   params.log_fs << "Received undefined request";
                   params.log_fs << " from task " << calling_taskid;
                 }
                 break;
      case 15:   if (params.debug > 2)
                 {
                   params.log_fs << "Received update region_label request";
                   params.log_fs << " from task " << calling_taskid;
                 }

                 update_region_label(recur_level,section,spatial_data,pixel_data,temp_data);

                 if (params.debug > 2)
                 {
                   params.log_fs << "Completed update region_label for request current task." << endl;
                 }
                 MPI::COMM_WORLD.Send(&pixel_index, 1, MPI::UNSIGNED, calling_taskid, update_region_label_tag);
                 break;
      case 16:   if (params.debug > 2)
                 {
                   params.log_fs << "Received connected component labeling request";
                   params.log_fs << " from task " << calling_taskid << endl;
                 }

                 MPI::INT.Unpack(temp_data.buffer, temp_data.buf_size, &params.object_maxnbdir, 1,
                                 temp_data.position, MPI::COMM_WORLD);
                 MPI::INT.Unpack(temp_data.buffer, temp_data.buf_size, &ncols, 1,
                                 temp_data.position, MPI::COMM_WORLD);
                 MPI::INT.Unpack(temp_data.buffer, temp_data.buf_size, &nrows, 1,
                                 temp_data.position, MPI::COMM_WORLD);
#ifdef THREEDIM
                 MPI::INT.Unpack(temp_data.buffer, temp_data.buf_size, &nslices, 1,
                                 temp_data.position, MPI::COMM_WORLD);
#endif
                 nb_objects = 0;
#ifdef THREEDIM
                 connected_component_init(recur_level,section,ncols,nrows,nslices,
                                          nb_objects,region_objects,spatial_data,temp_data);
#else
                 connected_component_init(recur_level,section,ncols,nrows,
                                          nb_objects,region_objects,spatial_data,temp_data);
#endif
                 MPI::COMM_WORLD.Send(&nb_objects, 1, MPI::UNSIGNED, calling_taskid, conn_comp_init_tag);
                 if (params.debug > 2)
                 {
                   params.log_fs << "Completed connected component labeling request current task." << endl;
                 }
                 break;
      case 17:   MPI::UNSIGNED.Unpack(temp_data.buffer, temp_data.buf_size, &region_objects_size, 1,
                                      temp_data.position, MPI::COMM_WORLD);
                 MPI::UNSIGNED_SHORT.Unpack(temp_data.buffer, temp_data.buf_size, &nslevels, 1,
                                            temp_data.position, MPI::COMM_WORLD);
                 if (params.debug > 2)
                 {
                   params.log_fs << "Received connected component labeling update request with nslevels = " << nslevels;
                   params.log_fs << " from task " << calling_taskid << endl;
                 }

                 MPI::INT.Unpack(temp_data.buffer, temp_data.buf_size, &ncols, 1,
                                 temp_data.position, MPI::COMM_WORLD);
                 MPI::INT.Unpack(temp_data.buffer, temp_data.buf_size, &nrows, 1,
                                 temp_data.position, MPI::COMM_WORLD);
#ifdef THREEDIM
                 MPI::INT.Unpack(temp_data.buffer, temp_data.buf_size, &nslices, 1,
                                 temp_data.position, MPI::COMM_WORLD);
#endif
                 region_objects.resize(region_objects_size);
                 for (region_object_index = 0; region_object_index < region_objects_size; ++region_object_index)
                 {
                   region_objects[region_object_index].clear();
                   region_object_label = region_object_index + 1;
                   region_objects[region_object_index].set_label_active(region_object_label);
                   region_objects[region_object_index].set_merge_region_label(region_object_label);
                 }
#ifdef THREEDIM
                 connected_component(recur_level, section, ncols, nrows, nslices, nslevels, 
                                     region_objects, spatial_data, temp_data);
#else
                 connected_component(recur_level, section, ncols, nrows, nslevels, 
                                     region_objects, spatial_data, temp_data);
#endif
                 short_buf_size = int_buf_size = double_buf_size = 0;
                 int_buf_size = region_objects_size;
                 check_buf_size(0,short_buf_size,int_buf_size,0,double_buf_size,
                                temp_data);
                 MPI::COMM_WORLD.Send(&int_buf_size, 1, MPI::UNSIGNED, calling_taskid, conn_comp_tag);
                 if (int_buf_size > 0)
                 {
                   for (region_object_index = 0; region_object_index < region_objects_size; ++region_object_index)
                     temp_data.int_buffer[region_object_index] =
                                          region_objects[region_object_index].get_merge_region_label();
                   MPI::COMM_WORLD.Send(temp_data.int_buffer, int_buf_size, MPI::UNSIGNED,
                                        calling_taskid, conn_comp_tag);
                 }
                 if (params.debug > 2)
                 {
                   params.log_fs << "Completed connected component labeling update request." << endl;
                 }
                 break;
      case 18:   MPI::UNSIGNED_SHORT.Unpack(temp_data.buffer, temp_data.buf_size, &nslevels, 1,
                                            temp_data.position, MPI::COMM_WORLD);
                 if (params.debug > 2)
                 {
                   params.log_fs << "Received boundary map request with nslevels = " << nslevels;
                   params.log_fs << " from task " << calling_taskid << endl;
                 }
                 MPI::INT.Unpack(temp_data.buffer, temp_data.buf_size, &ncols, 1,
                                 temp_data.position, MPI::COMM_WORLD);
                 MPI::INT.Unpack(temp_data.buffer, temp_data.buf_size, &nrows, 1,
                                 temp_data.position, MPI::COMM_WORLD);
#ifdef THREEDIM
                 MPI::INT.Unpack(temp_data.buffer, temp_data.buf_size, &nslices, 1,
                                 temp_data.position, MPI::COMM_WORLD);

                 boundary_map(recur_level,section,ncols,nrows,nslices,
                              nslevels,spatial_data,temp_data);
#else
                 boundary_map(recur_level,section,ncols,nrows,
                              nslevels,spatial_data,temp_data);
#endif
                 MPI::COMM_WORLD.Send(&pixel_index, 1, MPI::UNSIGNED, calling_taskid, boundary_map_init_tag);
                 if (params.debug > 2)
                 {
                   params.log_fs << "Completed boundary map request." << endl;
                 }
                 break;
      case 19:   MPI::UNSIGNED_SHORT.Unpack(temp_data.buffer, temp_data.buf_size, &nslevels, 1,
                                            temp_data.position, MPI::COMM_WORLD);
                 if (params.debug > 2)
                 {
                   params.log_fs << "Received set boundary map request with nslevels = " << nslevels;
                   params.log_fs << " from task " << calling_taskid << endl;
                 }
                 MPI::UNSIGNED.Unpack(temp_data.buffer, temp_data.buf_size, &short_buf_size, 1,
                                      temp_data.position, MPI::COMM_WORLD);
                 MPI::UNSIGNED.Unpack(temp_data.buffer, temp_data.buf_size, &int_buf_size, 1,
                                      temp_data.position, MPI::COMM_WORLD);
                 check_buf_size(0,short_buf_size,int_buf_size,0,0,temp_data);
                 MPI::COMM_WORLD.Recv(temp_data.short_buffer, short_buf_size, MPI::UNSIGNED_SHORT, calling_taskid, temp_data_tag);
                 MPI::COMM_WORLD.Recv(temp_data.int_buffer, int_buf_size, MPI::UNSIGNED, calling_taskid, temp_data_tag);

                 spatial_data.set_boundary_map(nslevels, int_buf_size, temp_data);

                 MPI::COMM_WORLD.Send(&int_buf_size, 1, MPI::UNSIGNED, calling_taskid, set_boundary_map_tag);
                 if (params.debug > 2)
                 {
                   params.log_fs << "Completed set boundary map request." << endl;
                 }
                 break;
      case 20:   if (params.debug > 2)
                 {
                   params.log_fs << "Received undefined request";
                   params.log_fs << " from task " << calling_taskid;
                 }
                 break;
      case 21:   if (params.debug > 2)
                 {
                   params.log_fs << "Received undefined request";
                   params.log_fs << " from task " << calling_taskid;
                 }
                 break;
      case 22:   MPI::UNSIGNED.Unpack(temp_data.buffer, temp_data.buf_size, &max_region_label, 1,
                                      temp_data.position, MPI::COMM_WORLD);
                 MPI::INT.Unpack(temp_data.buffer, temp_data.buf_size, &ncols, 1,
                                      temp_data.position, MPI::COMM_WORLD);
                 MPI::INT.Unpack(temp_data.buffer, temp_data.buf_size, &nrows, 1,
                                      temp_data.position, MPI::COMM_WORLD);
#ifdef THREEDIM
                 MPI::INT.Unpack(temp_data.buffer, temp_data.buf_size, &nslices, 1,
                                      temp_data.position, MPI::COMM_WORLD);
#endif
#ifdef DEBUG
                 if (params.debug > 2)
                 {
                   params.log_fs << "Received region_classes.nghbrs_label_set initialization request";
                   params.log_fs << " from task " << calling_taskid << " with max_region_label = " << max_region_label << endl;
#ifdef THREEDIM
                   params.log_fs << "and ncols = " << ncols << ", nrows = " << nrows << " and nslices = " << nslices << endl;
#else
                   params.log_fs << "and ncols = " << ncols << ", and nrows = " << nrows << endl;
#endif
                 }
#endif
                 region_classes_size = region_classes.size();
                 if (region_classes_size < max_region_label)
                 {
                   region_classes_size = max_region_label;
                   region_classes.resize(region_classes_size);
                 }

                 for (region_index = 0; region_index < max_region_label; ++region_index)
                 {
                   region_classes[region_index].clear();
                   region_classes[region_index].set_label(region_index + 1);
                 }
#ifdef THREEDIM
                 nghbrs_label_set_init(recur_level,section,max_region_label,ncols,nrows,nslices,
                                       pixel_data,region_classes,temp_data);
#else
                 nghbrs_label_set_init(recur_level,section,max_region_label,ncols,nrows,
                                       pixel_data,region_classes,temp_data);
#endif
                 region_classes_size = region_classes.size();

                 int_buf_size = 0;
                 max_region_label = 0;
                 nregions = 0;
                 for (region_index = 0; region_index < region_classes_size; ++region_index)
                   if (region_classes[region_index].get_active_flag())
                   {
                     region_label = region_classes[region_index].get_label();
                     if (region_label > max_region_label)
                       max_region_label = region_label;
                     nregions++;
                     int_buf_size += region_classes[region_index].get_nghbrs_label_set_size(); // neighbor region label
                   }
                 int_buf_size += 2*nregions;  // nghbrs_label_set_size() & label
                 check_buf_size(0,0,int_buf_size,0,0,temp_data);

                 int_buf_position = 0;
                 for (region_index = 0; region_index < region_classes_size; ++region_index)
                   if (region_classes[region_index].get_active_flag())
                   {
                     temp_data.int_buffer[int_buf_position++] = region_classes[region_index].get_label();
                     region_classes[region_index].load_nghbrs_label_set(temp_data,int_buf_position);
                   }

                 if (int_buf_position > int_buf_size)
                 {
                   if (params.debug > 0)
                   {
                     params.log_fs << "WARNING (case 22):  Temp buffer overflow:" << endl;
                     params.log_fs << "int_buf_size = " << int_buf_size << " and int_buf_position = " << int_buf_position << endl;
                   }
                   else
                   {
                     cout << "WARNING (case 22):  Temp buffer overflow:" << endl;
                     cout << "int_buf_size = " << int_buf_size << " and int_buf_position = " << int_buf_position << endl;
                   }
                 }

                 MPI::COMM_WORLD.Send(&max_region_label, 1, MPI::UNSIGNED, calling_taskid, nghbrs_label_set_init_tag);
                 MPI::COMM_WORLD.Send(&int_buf_position, 1, MPI::UNSIGNED, calling_taskid, nghbrs_label_set_init_tag);
                 if (int_buf_position > 0)
                 {
                   MPI::COMM_WORLD.Send(temp_data.int_buffer, int_buf_position, MPI::UNSIGNED, calling_taskid, nghbrs_label_set_init_tag);
                 }
#ifdef DEBUG
                 if (params.debug > 2)
                 {
                   params.log_fs << "Completed region_classes.nghbrs_label_set initialization" << endl;
                 }
#endif
                 break;
    }

    MPI::COMM_WORLD.Recv(temp_data.buffer, temp_data.buf_size, MPI::PACKED,
                         MPI::ANY_SOURCE, temp_data_tag);

    temp_data.position = 0;
    MPI::UNSIGNED_SHORT.Unpack(temp_data.buffer, temp_data.buf_size, &request_id, 1,
                               temp_data.position, MPI::COMM_WORLD);

    MPI::UNSIGNED_SHORT.Unpack(temp_data.buffer, temp_data.buf_size, &calling_taskid, 1,
                               temp_data.position, MPI::COMM_WORLD);

    MPI::UNSIGNED_SHORT.Unpack(temp_data.buffer, temp_data.buf_size, &recur_level, 1,
                               temp_data.position, MPI::COMM_WORLD);

  }
  if (params.debug > 2)
    params.log_fs << "Received termination request from task " << calling_taskid << endl;
  do_termination(recur_level,temp_data);

  return;
 }
} // namespace HSEGTilton

