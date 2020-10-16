/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>>          File:  results.cc
   >>>>
   >>>>          See results.h for documentation
   >>>>
   >>>>          Date:  December 9, 2002
   >>>> Modifications:  February 10, 2003:  Changed region index to region label
   >>>>                 November 18, 2003 - Added ConnRegion class.
   >>>>                 August 9, 2005 - Changed the max_merge_thresh member variable to conv_crit_value
   >>>>                 November 1, 2007 - Changed the conv_crit_value member variable to merge_threshold
   >>>>                 November 9, 2007 - Combined region feature output into region_classes list file
   >>>>                 November 15, 2007 - Modified to work with hseg_read program.
   >>>>                 February 7, 2008 - Modified to work with the hsegviewer program.
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#include "results.h"
#include <params/params.h>

extern HSEGTilton::Params params;
extern HSEGTilton::oParams oparams;

namespace HSEGTilton
{

  Results::Results( )
  {
    int band;

    nbands = params.nbands;
    orig_nb_classes = oparams.level0_nb_classes;
    scale = new double[nbands];
    offset = new double[nbands];
    for (band = 0; band < nbands; band++)
    {
      scale[band] = (float) (oparams.scale[band]*params.scale[band]);
      offset[band] = (float) (params.offset[band] + (oparams.offset[band]/params.scale[band]));
    }

    int_buffer_size = max_int_buffer_size = 0;
    double_buffer_size = 0;
    if (params.region_sum_flag)
    {
      double_buffer_size += nbands*orig_nb_classes;
      if (params.region_sumsq_flag)
        double_buffer_size += nbands*orig_nb_classes;
      if (params.region_sumxlogx_flag)
        double_buffer_size += nbands*orig_nb_classes;
    }
    if (params.region_std_dev_flag)
      double_buffer_size += orig_nb_classes;
    if (params.region_threshold_flag)
      double_buffer_size += orig_nb_classes;
 
    orig_nb_objects = 1;
    object_int_buffer_size = 0;
    object_double_buffer_size = 0;
    if (params.region_objects_flag)
    {
      orig_nb_objects = oparams.level0_nb_objects;

      object_int_buffer_size = 0;
      if (params.region_objects_list_flag)
      {
        object_int_buffer_size = 3*orig_nb_objects;
        if (params.region_boundary_npix_flag)
          object_int_buffer_size += orig_nb_objects;
      }

      object_double_buffer_size = 0;
      if (params.region_objects_list_flag)
      {
        if (params.region_sum_flag)
        {
          object_double_buffer_size += nbands*orig_nb_objects;
          if (params.region_sumsq_flag)
            object_double_buffer_size += nbands*orig_nb_objects;
          if (params.region_sumxlogx_flag)
            object_double_buffer_size += nbands*orig_nb_objects;
        }
        if (params.region_std_dev_flag)
          object_double_buffer_size += orig_nb_objects;
      }
    }

    if (double_buffer_size > object_double_buffer_size)
      double_buffer = new double[double_buffer_size];
    else
      double_buffer = new double[object_double_buffer_size];

    return;
  }

  Results::Results(const Results& source)
  {
    nbands = source.nbands;
    orig_nb_classes = source.orig_nb_classes;

    int_buffer_size = source.int_buffer_size;
    double_buffer_size = source.double_buffer_size;
    object_int_buffer_size = source.object_int_buffer_size;
    object_double_buffer_size = source.object_double_buffer_size;

    int_buffer = new unsigned int[int_buffer_size];
    if (double_buffer_size > object_double_buffer_size)
      double_buffer = new double[double_buffer_size];
    else
      double_buffer = new double[object_double_buffer_size];

    return;
  }

  Results::~Results( )
  {
    delete [ ] int_buffer;
    delete [ ] double_buffer;

    return;
  } 

  void Results::operator =(const Results& source)
  {
    unsigned int index;
  
    if (this == &source)
      return;

    nbands = source.nbands;
    orig_nb_classes = source.orig_nb_classes;

    int_buffer_size = source.int_buffer_size;
    double_buffer_size = source.double_buffer_size;
    object_int_buffer_size = source.object_int_buffer_size;
    object_double_buffer_size = source.object_double_buffer_size;

    int_buffer = new unsigned int[int_buffer_size];
    for (index = 0; index < int_buffer_size; index++)
      int_buffer[index] = source.int_buffer[index];

    if (double_buffer_size > object_double_buffer_size)
    {
      double_buffer = new double[double_buffer_size];
      for (index = 0; index < double_buffer_size; index++)
        double_buffer[index] = source.double_buffer[index];
    }
    else
    { 
      double_buffer = new double[object_double_buffer_size];
      for (index = 0; index < object_double_buffer_size; index++)
        double_buffer[index] = source.double_buffer[index];
    }

    return;
  }

  void Results::set_int_buffer_size(const unsigned int& hlevel)
  {

    if (max_int_buffer_size < oparams.int_buffer_size[hlevel])
    {
      if (max_int_buffer_size > 0)
        delete [ ] int_buffer;
      max_int_buffer_size = oparams.int_buffer_size[hlevel];
      if (object_int_buffer_size > max_int_buffer_size)
        max_int_buffer_size = object_int_buffer_size;
      int_buffer = new unsigned int[max_int_buffer_size];
    }
    int_buffer_size = oparams.int_buffer_size[hlevel];

    return;
  }

  void Results::open_input(const string& region_classes_file, const string& region_objects_file)
  {
    region_classes_fs.open(region_classes_file.c_str( ), ios_base::in | ios_base::binary );
 
    if (params.region_objects_flag)
      region_objects_fs.open(region_objects_file.c_str( ), ios_base::in | ios_base::binary );

    return;
  }

  void Results::read(const unsigned int& hlevel, unsigned int& nb_classes, unsigned int& nb_objects,
                     Stats& statsData)
  {
    unsigned int band, index;
    unsigned int region_index, stats_index;
    unsigned int npix;
    double       sum;
    unsigned int nghbrs_label_set_size;
    unsigned int merge_region_label;

    region_classes_fs.read(reinterpret_cast<char *>(int_buffer),4*int_buffer_size);
    region_classes_fs.read(reinterpret_cast<char *>(double_buffer),8*double_buffer_size);

    nb_classes = int_buffer_index = double_buffer_index = 0;
    statsData.maxStdDev = 0.0;
    for (region_index = 0; region_index < orig_nb_classes; ++region_index)
    {
      int_buffer_index++; // label

      merge_region_label = int_buffer[int_buffer_index++];
      if (hlevel > 0)
      {
        stats_index = region_index + (hlevel-1)*orig_nb_classes;
        statsData.classMergesList[stats_index] = merge_region_label;
      }

      if (merge_region_label == 0)
      {
        nb_classes++;

        npix = int_buffer[int_buffer_index++];
        stats_index = region_index + hlevel*orig_nb_classes;
        statsData.classNOPixelsList[stats_index] = npix;

        if (params.region_sum_flag)
        {
          for (band = 0; band < nbands; band++)
          {
            sum = double_buffer[double_buffer_index++];
            stats_index = band + region_index*nbands + hlevel*orig_nb_classes*nbands;
            statsData.classMeanList[stats_index] = ((sum/npix)/scale[band]) + offset[band];
          }

          if (params.region_sumsq_flag)
            for (band = 0; band < nbands; band++)
              double_buffer_index++;

          if (params.region_sumxlogx_flag)
            for (band = 0; band < nbands; band++)
              double_buffer_index++;
        }

        if (params.region_std_dev_flag)
        {
          stats_index = region_index + hlevel*orig_nb_classes;
          statsData.classStdDevList[stats_index] = double_buffer[double_buffer_index++];
          if (statsData.classStdDevList[stats_index] > statsData.maxStdDev)
            statsData.maxStdDev = statsData.classStdDevList[stats_index];
        }

        if (params.region_threshold_flag)
        {
          stats_index = region_index + hlevel*orig_nb_classes;
          statsData.classMergeThreshList[stats_index] = double_buffer[double_buffer_index++];
        }

        if (params.region_boundary_npix_flag)
        {
          stats_index = region_index + hlevel*orig_nb_classes;
          statsData.classBoundNOPixelsList[stats_index] = int_buffer[int_buffer_index++];
        }

        if (params.region_nghbrs_list_flag)
        {
          nghbrs_label_set_size = int_buffer[int_buffer_index++];
          for (index = 0; index < nghbrs_label_set_size; index++)
          {
            int_buffer_index++;
          }
        }

        if (params.region_nb_objects_flag)
        {
          nb_objects = int_buffer[int_buffer_index++];
          stats_index = region_index + hlevel*orig_nb_classes;
          statsData.classNOObjectsList[stats_index] = nb_objects;
        }
      
        if (params.region_objects_list_flag)
        {
          for (index = 0; index < nb_objects; index++)
          {
            int_buffer_index++;
          }
        }
      } // if (merge_region_label == 0)
    } // for (region_index = 0; region_index < orig_nb_classes; ++region_index)

    nb_objects = object_int_buffer_index = double_buffer_index = 0;
    if (params.region_objects_flag)
    {
      region_objects_fs.read(reinterpret_cast<char *>(int_buffer),4*object_int_buffer_size);
      region_objects_fs.read(reinterpret_cast<char *>(double_buffer),8*object_double_buffer_size);

      for (region_index = 0; region_index < orig_nb_objects; ++region_index)
      {
        object_int_buffer_index++; // label
        merge_region_label = int_buffer[object_int_buffer_index++];
        if (hlevel > 0)
        {
          stats_index = region_index + (hlevel-1)*orig_nb_objects;
          statsData.objectMergesList[stats_index] = merge_region_label;
        }

        if (merge_region_label == 0)
        {
          nb_objects++;
          npix = int_buffer[object_int_buffer_index++];
          stats_index = region_index + hlevel*orig_nb_objects;
          statsData.objectNOPixelsList[stats_index] = npix;

          if (params.region_sum_flag)
          {
            for (band = 0; band < nbands; band++)
              double_buffer_index++;

            if (params.region_sumsq_flag)
              for (band = 0; band < nbands; band++)
                double_buffer_index++;

            if (params.region_sumxlogx_flag)
              for (band = 0; band < nbands; band++)
                double_buffer_index++;
          }

          if (params.region_std_dev_flag)
          {
            stats_index = region_index + hlevel*orig_nb_objects;
            statsData.objectStdDevList[stats_index] = double_buffer[double_buffer_index++];
          }

          if (params.region_boundary_npix_flag)
          {
            stats_index = region_index + hlevel*orig_nb_objects;
            statsData.objectBoundNOPixelsList[stats_index] = int_buffer[object_int_buffer_index++];
          }

        } // if (merge_region_label == 0)
      } // for (region_index = 0; region_index < orig_nb_objects; ++region_index)
    } // if (params.region_objects_flag)

    return;
  }

  void Results::close_input( )
  {
    region_classes_fs.close( );
    if (params.region_objects_flag)
      region_objects_fs.close( );
  }
} // namespace HSEGTilton
