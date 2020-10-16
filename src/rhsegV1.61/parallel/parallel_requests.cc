/*-----------------------------------------------------------
|
|  Routine Name: parallel_recur_requests
|
|       Purpose: Submits parallel recursive requests to the parallel server subtasks
|
|         Input: request_id       (ID of parallel recursive request)
|                recur_level      (Current level of recursion)
|                value            (A value to be passed to the parallel process)
|                ncols            (Number of columns at current level of recursion)
|                nrows            (Number of rows at current level of recursion)
|                nslices          (Number of slices at current level of recursion)
|                short_buf_size   (Size of the short_buffer in temp_data)
|                int_buf_size     (Size of the int_buffer in temp_data)
|                double_buf_size  (Size of the double_buffer in temp_data)
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
|       Written: November 12, 2003
| Modifications: Added additional requests on various dates
|                December 23, 2004 - Changed "value and max_region_label" from short unsigned int to unsigned int
|                January 5, 2006 - Added slice dimension (extension to three-dimensional analysis)
|                May 16, 2008 - Revised to work with globally defined Params and oParams class objects
|                March 24, 2011 - Revised counting of recursive levels to start at level = 0 (instead of level = 1).
|                April 7, 2011 - Initiated using set_stride_sections to determine stride and nb_sections (nb_tasks for parallel).
|
------------------------------------------------------------*/
#include <defines.h>
#include <rhseg/hseg.h>
#include <params/params.h>
#include <iostream>

extern HSEGTilton::Params params;

namespace HSEGTilton
{
#ifdef THREEDIM
   void parallel_recur_requests(const short unsigned int& request_id, const short unsigned int& recur_level,
                                unsigned int value, const int& ncols, const int& nrows, const int& nslices,
                                unsigned int short_buf_size, unsigned int int_buf_size,
                                unsigned int double_buf_size, Temp& temp_data)
#else
   void parallel_recur_requests(const short unsigned int& request_id, const short unsigned int& recur_level,
                                unsigned int value, const int& ncols, const int& nrows,
                                unsigned int short_buf_size, unsigned int int_buf_size,
                                unsigned int double_buf_size, Temp& temp_data)
#endif
   {
     int stride, nb_tasks;
     set_stride_sections(recur_level,stride,nb_tasks);
     short unsigned int calling_taskid = params.myid;  // Calling taskid
     short unsigned int recur_recur_level = recur_level + 1;  // recur_task recursive level
     int recur_ncols = ncols;
     int recur_nrows = nrows;
     bool col_flag, row_flag;
#ifdef THREEDIM
     int recur_nslices = nslices;
     bool slice_flag;
     set_recur_flags(params.recur_mask_flags[recur_level],col_flag,row_flag,slice_flag);
#else
     set_recur_flags(params.recur_mask_flags[recur_level],col_flag,row_flag);
#endif
     if (col_flag)
       recur_ncols /= 2;
     if (row_flag)
       recur_nrows /= 2;
#ifdef THREEDIM
     if (slice_flag)
       recur_nslices /= 2;
#endif

     short unsigned int nslevels;
     unsigned int max_region_label;
#ifdef TIME_IT
     float end_time, elapsed_time;
#endif

     if (params.debug > 2)
     {
       params.log_fs << "Entering parallel_recur_requests with request_id = " << request_id;
       params.log_fs << ", recur_level = " << recur_level << "," << endl;
       params.log_fs << "value = " << value;
#ifdef THREEDIM
       params.log_fs << ", ncols = " << ncols << ", nrows = " << nrows << ", nslices = " << nslices << "," << endl;
#else
       params.log_fs << ", ncols = " << ncols << ", nrows = " << nrows << "," << endl;
#endif
       params.log_fs << "short_buf_size = " << short_buf_size << ", int_buf_size = ";
       params.log_fs << int_buf_size << " and double_buf_size = " << double_buf_size << endl;
     }

     temp_data.position = 0;
     MPI::UNSIGNED_SHORT.Pack(&request_id, 1, temp_data.buffer, temp_data.buf_size,
                              temp_data.position, MPI::COMM_WORLD);
     MPI::UNSIGNED_SHORT.Pack(&calling_taskid, 1, temp_data.buffer, temp_data.buf_size,
                              temp_data.position, MPI::COMM_WORLD);
     MPI::UNSIGNED_SHORT.Pack(&recur_recur_level, 1, temp_data.buffer, temp_data.buf_size,
                              temp_data.position, MPI::COMM_WORLD);
     if (request_id == 1)
     {
       unsigned int class_label_offset = value;
       unsigned int object_label_offset = short_buf_size;
       MPI::UNSIGNED.Pack(&class_label_offset, 1, temp_data.buffer, temp_data.buf_size,
                          temp_data.position, MPI::COMM_WORLD);
       MPI::UNSIGNED.Pack(&object_label_offset, 1, temp_data.buffer, temp_data.buf_size,
                          temp_data.position, MPI::COMM_WORLD);
     }
     if (request_id == 2)
     {
       nslevels = (short unsigned int) nrows;
       if (value == 0)
       {
         max_region_label = short_buf_size; // (region_classes_size)
         short_buf_size = 0;
         MPI::UNSIGNED.Pack(&value, 1, temp_data.buffer, temp_data.buf_size,
                            temp_data.position, MPI::COMM_WORLD);
         MPI::UNSIGNED_SHORT.Pack(&nslevels, 1, temp_data.buffer, temp_data.buf_size,
                                  temp_data.position, MPI::COMM_WORLD);
         MPI::UNSIGNED.Pack(&max_region_label, 1, temp_data.buffer, temp_data.buf_size,
                            temp_data.position, MPI::COMM_WORLD);
       }
       else if (value == 1)
       {
         max_region_label = short_buf_size;
         short_buf_size = 0;
         MPI::UNSIGNED.Pack(&value, 1, temp_data.buffer, temp_data.buf_size,
                            temp_data.position, MPI::COMM_WORLD);
         MPI::UNSIGNED_SHORT.Pack(&nslevels, 1, temp_data.buffer, temp_data.buf_size,
                                  temp_data.position, MPI::COMM_WORLD);
         MPI::UNSIGNED.Pack(&max_region_label, 1, temp_data.buffer, temp_data.buf_size,
                            temp_data.position, MPI::COMM_WORLD);
         if (max_region_label > 0)
         {
           MPI::UNSIGNED.Pack(&int_buf_size, 1, temp_data.buffer, temp_data.buf_size,
                              temp_data.position, MPI::COMM_WORLD);
           MPI::UNSIGNED.Pack(&double_buf_size, 1, temp_data.buffer, temp_data.buf_size,
                              temp_data.position, MPI::COMM_WORLD);
         }
       }
       else // (value == 3)
       {
         max_region_label = short_buf_size; // (max_region_object_label)
         short_buf_size = 0;
         MPI::UNSIGNED.Pack(&value, 1, temp_data.buffer, temp_data.buf_size,
                                  temp_data.position, MPI::COMM_WORLD);
         MPI::UNSIGNED_SHORT.Pack(&nslevels, 1, temp_data.buffer, temp_data.buf_size,
                                  temp_data.position, MPI::COMM_WORLD);
         MPI::UNSIGNED.Pack(&max_region_label, 1, temp_data.buffer, temp_data.buf_size,
                             temp_data.position, MPI::COMM_WORLD);
         if (max_region_label > 0)
         {
           MPI::UNSIGNED.Pack(&int_buf_size, 1, temp_data.buffer, temp_data.buf_size,
                              temp_data.position, MPI::COMM_WORLD);
           MPI::UNSIGNED.Pack(&double_buf_size, 1, temp_data.buffer, temp_data.buf_size,
                              temp_data.position, MPI::COMM_WORLD);
         }
       }
     }
     if (request_id == 4)
     {
       MPI::UNSIGNED.Pack(&value, 1, temp_data.buffer, temp_data.buf_size,
                          temp_data.position, MPI::COMM_WORLD);
       MPI::UNSIGNED.Pack(&int_buf_size, 1, temp_data.buffer, temp_data.buf_size,
                          temp_data.position, MPI::COMM_WORLD);
     }
     if ((request_id == 3) || (request_id == 5) ||
         (request_id == 6) || (request_id == 9) || (request_id == 22))
     {
       MPI::UNSIGNED.Pack(&value, 1, temp_data.buffer, temp_data.buf_size,
                          temp_data.position, MPI::COMM_WORLD);
     }
     if (request_id == 16)
     {
       MPI::INT.Pack(&params.object_maxnbdir, 1, temp_data.buffer, temp_data.buf_size,
                     temp_data.position, MPI::COMM_WORLD);
     }
     if (request_id == 17)
     {
       MPI::UNSIGNED.Pack(&short_buf_size, 1, temp_data.buffer, temp_data.buf_size,
                          temp_data.position, MPI::COMM_WORLD);
       nslevels = (short unsigned int) int_buf_size;
       MPI::UNSIGNED_SHORT.Pack(&nslevels, 1, temp_data.buffer, temp_data.buf_size,
                                temp_data.position, MPI::COMM_WORLD);
     }
     if (request_id == 18)
     {
       nslevels = (short unsigned int) value;
       MPI::UNSIGNED_SHORT.Pack(&nslevels, 1, temp_data.buffer, temp_data.buf_size,
                                temp_data.position, MPI::COMM_WORLD);
     }
     if (request_id == 3)
     {
       MPI::UNSIGNED.Pack(&short_buf_size, 1, temp_data.buffer, temp_data.buf_size,
                          temp_data.position, MPI::COMM_WORLD);
       MPI::UNSIGNED.Pack(&int_buf_size, 1, temp_data.buffer, temp_data.buf_size,
                          temp_data.position, MPI::COMM_WORLD);
       MPI::UNSIGNED.Pack(&double_buf_size, 1, temp_data.buffer, temp_data.buf_size,
                          temp_data.position, MPI::COMM_WORLD);
     }
     if ((request_id ==  5) || (request_id ==  6) || (request_id == 9) ||
         (request_id == 16) || (request_id == 17) ||
         (request_id == 18) || (request_id == 22) ||
         ((request_id == 4) && (value == 1)))
     {
       MPI::INT.Pack(&recur_ncols, 1, temp_data.buffer, temp_data.buf_size,
                     temp_data.position, MPI::COMM_WORLD);
       MPI::INT.Pack(&recur_nrows, 1, temp_data.buffer, temp_data.buf_size,
                     temp_data.position, MPI::COMM_WORLD);
#ifdef THREEDIM
       MPI::INT.Pack(&recur_nslices, 1, temp_data.buffer, temp_data.buf_size,
                     temp_data.position, MPI::COMM_WORLD);
#endif
     }

   // Send request to the parallel recur_tasks
     int temp_data_tag = 101;
     int min_taskid = params.myid + stride;
     int max_taskid = params.myid + nb_tasks;
#ifdef TIME_IT
     end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
     elapsed_time = end_time - temp_data.start_time;
     if (elapsed_time > 0.0) temp_data.compute += elapsed_time;
     temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
     if (temp_data.position > temp_data.buf_size)
     {
       cout << "WARNING(1): temp_data.position = " << temp_data.position << endl;
       cout << "Message from taskid = " << params.myid << endl;
     }
     for (int recur_taskid = min_taskid; recur_taskid < max_taskid; recur_taskid += stride)
     {
       MPI::COMM_WORLD.Send(temp_data.buffer, temp_data.position, MPI::PACKED, recur_taskid, temp_data_tag);
       if ((request_id ==  2) || (request_id == 3) || (request_id ==  4))
       {
         if (short_buf_size > 0)
           MPI::COMM_WORLD.Send(temp_data.short_buffer, short_buf_size, MPI::UNSIGNED_SHORT, recur_taskid, temp_data_tag);
         if (int_buf_size > 0)
           MPI::COMM_WORLD.Send(temp_data.int_buffer, int_buf_size, MPI::UNSIGNED, recur_taskid, temp_data_tag);
         if (double_buf_size > 0)
           MPI::COMM_WORLD.Send(temp_data.double_buffer, double_buf_size, MPI::DOUBLE, recur_taskid, temp_data_tag);
       }
       if (params.debug > 2)
       {
         if (request_id == 0)
           params.log_fs << "Sent termination request to task " << recur_taskid << endl;
         if (request_id == 1)
         {
           if (short_buf_size > 0)
             params.log_fs << "Sent request to perform connected region label offset of " << short_buf_size;
           if (value > 0)
             params.log_fs << "Sent request to perform region label offset of " << value;
           params.log_fs << " to task " << recur_taskid << endl;
         }
         if (request_id == 2)
         {
           if (value == 0)
             params.log_fs << "Sent update_region_class_info request to task " << recur_taskid << endl;
           else if (value == 1)
             params.log_fs << "Sent update_region_object_info request to task " << recur_taskid << endl;
           else
             params.log_fs << "Sent region_classes (for update_sum_pixel_gdissim) to task " << recur_taskid << endl;
         }
         if (request_id == 3)
           params.log_fs << "Sent switch_pixels request to task " << recur_taskid << endl;
         if (request_id == 4)
         {
           if (value == 0)
             params.log_fs << "Sent region relabel request to task " << recur_taskid << endl;
           if (value == 1)
             params.log_fs << "Sent connected region relabel request to task " << recur_taskid << endl;
         }
         if (request_id == 5)
         {
           if (value > 0)
             params.log_fs << "Sent connected region initialization request to task " << recur_taskid << endl;
           else
             params.log_fs << "Sent region initialization request to task " << recur_taskid << endl;
         }
         if (request_id == 6)
           params.log_fs << "Sent get_border_index_data (pixel_data) request to task " << recur_taskid << endl;
         if (request_id == 7)
           params.log_fs << "Sent region_label_map update request to task " << recur_taskid << endl;
         if (request_id == 8)
           params.log_fs << "Sent region_label_map write request to task " << recur_taskid << endl;
         if (request_id == 9)
           params.log_fs << "Sent get_border_index_data (spatial_data) request to task " << recur_taskid << endl;
         if (request_id == 15)
           params.log_fs << "Sent update region_label request to recur_taskid " << recur_taskid << "." << endl;
         if (request_id == 16)
           params.log_fs << "Sent connected component labeling (initial) request to task " << recur_taskid << endl;
         if (request_id == 17)
           params.log_fs << "Sent connected component labeling (update) request to task " << recur_taskid << endl;
         if (request_id == 18)
         {
           if (value == 0)
             params.log_fs << "Sent boundary flag initialization request to task " << recur_taskid << endl;
           if (value == 1)
             params.log_fs << "Sent boundary flag update request to task " << recur_taskid << endl;
         }
         if (request_id == 22)
           params.log_fs << "Sent nghbrs_label_set_init request to task " << recur_taskid << endl;
       }
       if (params.debug > 3)
       {
         params.log_fs << "with recur_level = " << recur_recur_level << " and temp_data.position = " << temp_data.position << endl;
       }
     }
#ifdef TIME_IT
     end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
     elapsed_time = end_time - temp_data.start_time;
     if (elapsed_time > 0.0) temp_data.transfer += elapsed_time;
     temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
   }

/*-----------------------------------------------------------
|
|  Routine Name: parallel_recur_request
|
|       Purpose: Submits a request to a particular recursive parallel subtask
|
|         Input: request_id       (ID of parallel recursive request)
|                recur_level      (Current level of recursion)
|                recur_taskid     (Parallel subtask to which this request is sent)
|                value1           (For request id 1: The region label offset value to be passed to the parallel process)
|                value2           (For request id 1: The connected region label offset value to be passed to the parallel process)
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
|       Written: November 12, 2003
| Modifications: Added additional requests on various dates
|                June 5, 2007 - Added split_pixels request (request_id 21)
|                June 7, 2007 - Added remerge request (request_id 20)
|                September 18, 2013 - Removed split_pixels and remerge requests
|
------------------------------------------------------------*/
#ifdef THREEDIM
   void parallel_recur_request(const short unsigned int& request_id, const short unsigned int& recur_level,
                               const int& recur_taskid, unsigned int value1, unsigned int value2,
                               const int& ncols, const int& nrows, const int& nslices,
                               unsigned int int_buf_size, unsigned int double_buf_size,
                               Temp& temp_data)
#else
   void parallel_recur_request(const short unsigned int& request_id, const short unsigned int& recur_level,
                               const int& recur_taskid, unsigned int value1, unsigned int value2,
                               const int& ncols, const int& nrows,
                               unsigned int int_buf_size, unsigned int double_buf_size,
                               Temp& temp_data)
#endif
   {
     short unsigned int calling_taskid = params.myid;  // Calling taskid
     int temp_data_tag = 101;
#ifdef TIME_IT
     float end_time, elapsed_time;
#endif

     temp_data.position = 0;
     MPI::UNSIGNED_SHORT.Pack(&request_id, 1, temp_data.buffer, temp_data.buf_size,
                              temp_data.position, MPI::COMM_WORLD);
     MPI::UNSIGNED_SHORT.Pack(&calling_taskid, 1, temp_data.buffer, temp_data.buf_size,
                              temp_data.position, MPI::COMM_WORLD);
     MPI::UNSIGNED_SHORT.Pack(&recur_level, 1, temp_data.buffer, temp_data.buf_size,
                              temp_data.position, MPI::COMM_WORLD);
     if (request_id == 1)
     {
       MPI::UNSIGNED.Pack(&value1, 1, temp_data.buffer, temp_data.buf_size,
                          temp_data.position, MPI::COMM_WORLD);
       MPI::UNSIGNED.Pack(&value2, 1, temp_data.buffer, temp_data.buf_size,
                          temp_data.position, MPI::COMM_WORLD);
     }
#ifdef TIME_IT
     end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
     elapsed_time = end_time - temp_data.start_time;
     if (elapsed_time > 0.0) temp_data.compute += elapsed_time;
     temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
     if (temp_data.position > temp_data.buf_size)
     {
       cout << "WARNING(2): temp_data.position = " << temp_data.position << endl;
       cout << "Message from taskid = " << params.myid << endl;
     }
     MPI::COMM_WORLD.Send(temp_data.buffer, temp_data.position, MPI::PACKED, recur_taskid, temp_data_tag);
#ifdef TIME_IT
     end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
     elapsed_time = end_time - temp_data.start_time;
     if (elapsed_time > 0.0) temp_data.transfer += elapsed_time;
     temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
     if (params.debug > 3)
     {
       if (request_id == 1)
       {
         if (value2 > 0)
           params.log_fs << "Sent request to perform connected region label offset of " << value2;
         else if (value1 > 0)
           params.log_fs << "Sent request to perform region label offset of " << value1;
         params.log_fs << " to task " << recur_taskid << endl;
       }
     }
     if (params.debug > 3)
     {
       params.log_fs << "with recur_level = " << recur_level << " and temp_data.position = " << temp_data.position << endl;
     }
   }

/*-----------------------------------------------------------
|
|  Routine Name: parallel_request
|
|       Purpose: Submits a request to a particular parallel server subtask
|
|         Input: request_id       (ID of parallel recursive request)
|                onb_taskid       (Parallel subtask to which this request is sent)
|                value            (A value to be passed to the parallel process)
|                short_buf_size   (Size of the short_buffer in temp_data)
|                int_buf_size     (Size of the int_buffer in temp_data)
|                double_buf_size  (Size of the double_buffer in temp_data)
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
|       Written: November 12, 2003
| Modifications: Added additional requests on various dates
|
------------------------------------------------------------*/
   void parallel_request(const short unsigned int& request_id,
                         const int& onb_taskid, const unsigned int& value,
                         unsigned int byte_buf_size, unsigned int short_buf_size,
                         unsigned int int_buf_size, unsigned int double_buf_size,
                         Temp& temp_data)
   {
     short unsigned int calling_taskid = params.myid;  // Calling taskid
     short unsigned int recur_level = params.onb_levels - 1;
     int temp_data_tag = 101;
#ifdef TIME_IT
     float end_time, elapsed_time;
#endif

     if (params.debug > 2)
     {
       params.log_fs << "Entering parallel_request with request_id = " << request_id;
       params.log_fs << ", onb_taskid  = " << onb_taskid << "," << endl;
       params.log_fs << "value = " << value << ", byte_buf_size = " << byte_buf_size;
       params.log_fs << ", short_buf_size = " << short_buf_size << ", int_buf_size = ";
       params.log_fs << int_buf_size << " and double_buf_size = " << double_buf_size << endl;
     }

     temp_data.position = 0;
     MPI::UNSIGNED_SHORT.Pack(&request_id, 1, temp_data.buffer, temp_data.buf_size,
                              temp_data.position, MPI::COMM_WORLD);
     MPI::UNSIGNED_SHORT.Pack(&calling_taskid, 1, temp_data.buffer, temp_data.buf_size,
                              temp_data.position, MPI::COMM_WORLD);
     MPI::UNSIGNED_SHORT.Pack(&recur_level, 1, temp_data.buffer, temp_data.buf_size,
                              temp_data.position, MPI::COMM_WORLD);
#ifdef THREEDIM
     if ((request_id == 8) || (request_id == 10))
     {
       unsigned int slice = value;
       MPI::UNSIGNED.Pack(&slice, 1, temp_data.buffer, temp_data.buf_size,
                          temp_data.position, MPI::COMM_WORLD);
     }
#endif
     if (request_id == 14)
     {
       unsigned int nregions = value;
       MPI::UNSIGNED.Pack(&nregions, 1, temp_data.buffer, temp_data.buf_size,
                          temp_data.position, MPI::COMM_WORLD);
       MPI::UNSIGNED.Pack(&int_buf_size, 1, temp_data.buffer, temp_data.buf_size,
                          temp_data.position, MPI::COMM_WORLD);
       MPI::UNSIGNED.Pack(&double_buf_size, 1, temp_data.buffer, temp_data.buf_size,
                          temp_data.position, MPI::COMM_WORLD);
       byte_buf_size = short_buf_size = 0;
     }
#ifdef TIME_IT
     end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
     elapsed_time = end_time - temp_data.start_time;
     if (elapsed_time > 0.0) temp_data.compute += elapsed_time;
     temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
     if (request_id == 19)
     {
       short unsigned int nslevels = (short unsigned int) value;
       MPI::UNSIGNED_SHORT.Pack(&nslevels, 1, temp_data.buffer, temp_data.buf_size,
                                temp_data.position, MPI::COMM_WORLD);
       MPI::UNSIGNED.Pack(&short_buf_size, 1, temp_data.buffer, temp_data.buf_size,
                          temp_data.position, MPI::COMM_WORLD);
       MPI::UNSIGNED.Pack(&int_buf_size, 1, temp_data.buffer, temp_data.buf_size,
                          temp_data.position, MPI::COMM_WORLD);
     }
     if (temp_data.position > temp_data.buf_size)
     {
       cout << "WARNING(3): temp_data.position = " << temp_data.position << endl;
       cout << "Message from taskid = " << params.myid << endl;
     }
     MPI::COMM_WORLD.Send(temp_data.buffer, temp_data.position, MPI::PACKED, onb_taskid, temp_data_tag);
     if ((request_id == 14) || (request_id == 19))
     {
       if (byte_buf_size > 0)
         MPI::COMM_WORLD.Send(temp_data.byte_buffer, byte_buf_size, MPI::UNSIGNED_CHAR, onb_taskid, temp_data_tag);
       if (short_buf_size > 0)
         MPI::COMM_WORLD.Send(temp_data.short_buffer, short_buf_size, MPI::UNSIGNED_SHORT, onb_taskid, temp_data_tag);
       if (int_buf_size > 0)
         MPI::COMM_WORLD.Send(temp_data.int_buffer, int_buf_size, MPI::UNSIGNED, onb_taskid, temp_data_tag);
       if (double_buf_size > 0)
         MPI::COMM_WORLD.Send(temp_data.double_buffer, double_buf_size, MPI::DOUBLE, onb_taskid, temp_data_tag);
     }
#ifdef TIME_IT
     end_time = (((float) clock())/((float) CLOCKS_PER_SEC));
     elapsed_time = end_time - temp_data.start_time;
     if (elapsed_time > 0.0) temp_data.transfer += elapsed_time;
     temp_data.start_time = (((float) clock())/((float) CLOCKS_PER_SEC));
#endif
     if (params.debug > 3)
     {
       if (request_id == 8)
         params.log_fs << "Sent region_label_map write request to task " << onb_taskid << endl;
       if (request_id == 10)
         params.log_fs << "Sent boundary_map write request to task " << onb_taskid << endl;
       if (request_id == 14)
         params.log_fs << "Sent compare_across_seam request to task " << onb_taskid << endl;
       if (request_id == 19)
         params.log_fs << "Sent set boundary_map request to task " << onb_taskid << endl;
     }
     if (params.debug > 3)
     {
       params.log_fs << "with recur_level = " << recur_level << " and temp_data.position = " << temp_data.position << endl;
     }
   }
} // namespace HSEGTilton
