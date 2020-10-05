/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>>          File:  results.cc
   >>>>
   >>>>          See results.h for documentation
   >>>>
   >>>>          Date:  December 9, 2002
   >>>> Modifications:  February 10, 2003:  Changed region index to region label
   >>>>                 November 18, 2003 - Added RegionObject class.
   >>>>                 August 9, 2005 - Changed the max_merge_thresh member variable to conv_crit_value
   >>>>                 November 1, 2007 - Changed the conv_crit_value member variable to merge_threshold
   >>>>                 November 9, 2007 - Combined region feature output into region_object list file
   >>>>                 February 24, 2011 - Added support for FusedClass and FusedObject object classes
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#include "results.h"
#include <params/params.h>
#include <region/region_class.h>
#include <region/region_object.h>
#include <iostream>

using namespace std;

extern HSEGTilton::Params params;

namespace HSEGTilton
{

 Results::Results()
 {
  nbands = 0;
  orig_nb_classes = 0;
  orig_nb_objects = 0;

  int_buffer_size = 0;
  max_int_buffer_size = 0;
  double_buffer_size = 0;

  int_buffer = NULL;
  double_buffer = NULL;

  object_int_buffer_size = 0;
  object_double_buffer_size = 0;

  return;
 }

 Results::Results(const Results& source)
 {

  nbands = source.nbands;
  orig_nb_classes = source.orig_nb_classes;
  orig_nb_objects = source.orig_nb_objects;

  int_buffer_size = source.int_buffer_size;
  double_buffer_size = source.double_buffer_size;
  object_int_buffer_size = source.object_int_buffer_size;
  object_double_buffer_size = source.object_double_buffer_size;
  max_int_buffer_size = 0; // This mean the buffers aren't allocated

  return;
 }

 Results::~Results()
 {
  if (int_buffer != NULL)
    delete [ ] int_buffer;
  if (double_buffer != NULL) 
    delete [ ] double_buffer;

  return;
 }

 void Results::operator =(const Results& source)
 {
  if (this == &source)
    return;

  nbands = source.nbands;
  orig_nb_classes = source.orig_nb_classes;
  orig_nb_objects = source.orig_nb_objects;

  int_buffer_size = source.int_buffer_size;
  double_buffer_size = source.double_buffer_size;
  object_int_buffer_size = source.object_int_buffer_size;
  object_double_buffer_size = source.object_double_buffer_size;
  max_int_buffer_size = 0; // This mean the buffers aren't allocated

  return;
 }

 void Results::set_buffer_sizes(const int& nb_bands, const int& nb_classes, const int& nb_objects)
 {

  nbands = nb_bands;
  orig_nb_classes = nb_classes;

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
 
  object_int_buffer_size = 0;
  object_double_buffer_size = 0;
  if (params.region_objects_flag)
  {
    orig_nb_objects = nb_objects;

    if (params.region_nb_objects_flag)
    {
      object_int_buffer_size = 3*orig_nb_objects;
      if (params.region_boundary_npix_flag)
        object_int_buffer_size += orig_nb_objects;
    }

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

  return;
 }

 void Results::open_output(const string& region_classes_file, const string& region_objects_file)
 {
  region_classes_ofs.open(region_classes_file.c_str(), ios_base::out | ios_base::binary );

  if (params.region_objects_list_flag)
    region_objects_ofs.open(region_objects_file.c_str(), ios_base::out | ios_base::binary );

  return;
 }

 void Results::write(const int& nslevels, const int& nb_classes, const int& nb_objects,
                     vector<RegionClass>& region_classes, vector<RegionObject>& region_objects)
 {
  int band;
  int region_index, merge_region_index, merge_region_label;
  int nghbrs_label_set_size, region_objects_set_size;
  set<unsigned int>::iterator nghbrs_label_set_iter, region_objects_set_iter;

  int_buffer_index = 2*orig_nb_classes + nb_classes;
  if (params.region_boundary_npix_flag)
    int_buffer_index += nb_classes;
  if (params.region_nb_objects_flag)
    int_buffer_index += nb_classes;

  for (region_index = 0; region_index < orig_nb_classes; ++region_index)
    if (region_classes[region_index].get_active_flag())
    {
      int_buffer_index += 1 + region_classes[region_index].nghbrs_label_set.size();
      if (params.region_objects_list_flag)
        int_buffer_index += 1 + region_classes[region_index].region_objects_set.size();
    }

  if (max_int_buffer_size == 0)
  {
    int_buffer_size = int_buffer_index;
    if (int_buffer_size < object_int_buffer_size)
      int_buffer_size = object_int_buffer_size;
    max_int_buffer_size = int_buffer_size;
    int_buffer = new int[max_int_buffer_size];
    if (double_buffer_size > object_double_buffer_size)
      double_buffer = new double[double_buffer_size];
    else
      double_buffer = new double[object_double_buffer_size];
  }
  else if (int_buffer_size < int_buffer_index)
  {
    delete [ ] int_buffer;
    int_buffer_size = int_buffer_index;
    int_buffer = new int[int_buffer_size];
  }

  int_buffer_index = double_buffer_index = 0;
  for (region_index = 0; region_index < orig_nb_classes; ++region_index)
  {
    int_buffer[int_buffer_index++] = region_classes[region_index].label;

    merge_region_label = 0;
    if ((nslevels != 0) && (!region_classes[region_index].get_active_flag()))
    {
      merge_region_index = region_index;
      while ((!(region_classes[merge_region_index].get_active_flag())) && (region_classes[merge_region_index].merge_region_label != 0))
        merge_region_index = region_classes[merge_region_index].merge_region_label - 1;
      merge_region_label = region_classes[merge_region_index].label;
    }
    int_buffer[int_buffer_index++] = merge_region_label;

    if (merge_region_label == 0)
    {
      int_buffer[int_buffer_index++] = region_classes[region_index].npix;

      if (params.region_sum_flag)
      {
        for (band = 0; band < nbands; band++)
          double_buffer[double_buffer_index++] = region_classes[region_index].sum[band];

        if (params.region_sumsq_flag)
        {
          for (band = 0; band < nbands; band++)
            double_buffer[double_buffer_index++] = region_classes[region_index].sumsq[band];
        }
  
        if (params.region_sumxlogx_flag)
        {
          for (band = 0; band < nbands; band++)
            double_buffer[double_buffer_index++] = region_classes[region_index].sumxlogx[band];
        }
      }

      if (params.region_std_dev_flag)
      {
        region_classes[region_index].set_band_max_std_dev();
        double_buffer[double_buffer_index++] = region_classes[region_index].band_max_std_dev;
      }

      if (params.region_threshold_flag)
        double_buffer[double_buffer_index++] = region_classes[region_index].merge_threshold;

      if (params.region_boundary_npix_flag)
        int_buffer[int_buffer_index++] = region_classes[region_index].boundary_npix;

      nghbrs_label_set_size = region_classes[region_index].nghbrs_label_set.size();
      int_buffer[int_buffer_index++] = nghbrs_label_set_size;
      
      nghbrs_label_set_iter = region_classes[region_index].nghbrs_label_set.begin();
      while (nghbrs_label_set_iter != region_classes[region_index].nghbrs_label_set.end())
      {
        int_buffer[int_buffer_index++] = (*nghbrs_label_set_iter);
        ++nghbrs_label_set_iter;
      }

      if (params.region_nb_objects_flag)
      {
        region_objects_set_size = region_classes[region_index].region_objects_set.size();
        int_buffer[int_buffer_index++] = region_objects_set_size;
      }
      
      if (params.region_objects_list_flag)
      {
        region_objects_set_iter = region_classes[region_index].region_objects_set.begin();
        while (region_objects_set_iter != region_classes[region_index].region_objects_set.end())
        {
          int_buffer[int_buffer_index++] = *region_objects_set_iter;
          ++region_objects_set_iter;
        }
      }
    } // if (merge_region_label == 0)
  } // for (region_index = 0; region_index < orig_nb_classes; ++region_index)

  region_classes_ofs.write(reinterpret_cast<char *>(int_buffer),4*int_buffer_index);
  region_classes_ofs.write(reinterpret_cast<char *>(double_buffer),8*double_buffer_size);

  object_int_buffer_index = double_buffer_index = 0;
  if (params.region_objects_list_flag)
  {
    for (region_index = 0; region_index < orig_nb_objects; ++region_index)
    {
      int_buffer[object_int_buffer_index++] = region_objects[region_index].label;

      merge_region_label = 0;
      if ((nslevels != 0) && (!region_objects[region_index].get_active_flag()))
      {
        merge_region_index = region_index;
        while ((!(region_objects[merge_region_index].get_active_flag())) && (region_objects[merge_region_index].merge_region_label != 0))
          merge_region_index = region_objects[merge_region_index].merge_region_label - 1;
        merge_region_label = region_objects[merge_region_index].label;
      }
      int_buffer[object_int_buffer_index++] = merge_region_label;

      if (merge_region_label == 0)
      {
        int_buffer[object_int_buffer_index++] = region_objects[region_index].npix;

        if (params.region_sum_flag)
        {
          for (band = 0; band < nbands; band++)
            double_buffer[double_buffer_index++] = region_objects[region_index].sum[band];

          if (params.region_sumsq_flag)
            for (band = 0; band < nbands; band++)
              double_buffer[double_buffer_index++] = region_objects[region_index].sumsq[band];

          if (params.region_sumxlogx_flag) 
            for (band = 0; band < nbands; band++)
              double_buffer[double_buffer_index++] = region_objects[region_index].sumxlogx[band];
        }

        if (params.region_std_dev_flag)
        {
          region_objects[region_index].set_band_max_std_dev();
          double_buffer[double_buffer_index++] = region_objects[region_index].band_max_std_dev;
        }

        if (params.region_boundary_npix_flag)
          int_buffer[object_int_buffer_index++] = region_objects[region_index].boundary_npix;
      } // if (merge_region_label == 0)
    } // for (region_index = 0; region_index < orig_nb_objects; ++region_index)

    region_objects_ofs.write(reinterpret_cast<char *>(int_buffer),4*object_int_buffer_size);
    region_objects_ofs.write(reinterpret_cast<char *>(double_buffer),8*object_double_buffer_size);

  } // if (params.region_objects_list_flag)

  return;
 }

 void Results::close_output()
 {
  region_classes_ofs.close();
  if (params.region_objects_list_flag)
    region_objects_ofs.close();
 }

  void Results::open_input(const string& region_classes_file, const string& region_objects_file)
  {
    region_classes_ifs.open(region_classes_file.c_str( ), ios_base::in | ios_base::binary );
 
    if (params.region_objects_flag)
      region_objects_ifs.open(region_objects_file.c_str( ), ios_base::in | ios_base::binary );

    return;
  }

  void Results::read(const int& segLevel, int& nb_classes, int& nb_objects, const int& nb_levels,
                     vector<unsigned int>& int_buffer_size,
                     vector<RegionClass>& region_classes, vector<RegionObject>& region_objects)
  {
    int band, index, region_index, hlevel;
    int offset, nghbrs_label_set_size;
    int fs_pos;

    if (max_int_buffer_size == 0)
    {
      for (hlevel = 0; hlevel < nb_levels; hlevel++)
        if (max_int_buffer_size < (int) int_buffer_size[hlevel])
          max_int_buffer_size = int_buffer_size[hlevel];
      if (max_int_buffer_size < object_int_buffer_size)
        max_int_buffer_size = object_int_buffer_size;
      int_buffer = new int[max_int_buffer_size];

      if (double_buffer_size > object_double_buffer_size)
        double_buffer = new double[double_buffer_size];
      else
        double_buffer = new double[object_double_buffer_size];
    }

    fs_pos = region_classes_ifs.tellg();
    if ((segLevel > 0) && (fs_pos == 0))
    {
      for (hlevel = 0; hlevel < segLevel; hlevel++)
      {
        offset = 0;
        offset += 4*int_buffer_size[hlevel];
        offset += 8*double_buffer_size;
        region_classes_ifs.seekg(offset, ios_base::cur);
      }
    }

    region_classes_ifs.read(reinterpret_cast<char *>(int_buffer),4*int_buffer_size[segLevel]);
    region_classes_ifs.read(reinterpret_cast<char *>(double_buffer),8*double_buffer_size);

    nb_classes = int_buffer_index = double_buffer_index = 0;
    for (region_index = 0; region_index < orig_nb_classes; ++region_index)
    {
      region_classes[region_index].label = int_buffer[int_buffer_index++];

      region_classes[region_index].merge_region_label = int_buffer[int_buffer_index++];

      region_classes[region_index].active_flag = false;
      if (region_classes[region_index].merge_region_label == 0)
      {
        region_classes[region_index].active_flag = true;
        nb_classes++;

        region_classes[region_index].npix = int_buffer[int_buffer_index++];

        if (params.region_sum_flag)
        {
          for (band = 0; band < nbands; band++)
            region_classes[region_index].sum[band] = double_buffer[double_buffer_index++];

          if (params.region_sumsq_flag)
            for (band = 0; band < nbands; band++)
              region_classes[region_index].sumsq[band] = double_buffer[double_buffer_index++];

          if (params.region_sumxlogx_flag)
            for (band = 0; band < nbands; band++)
              region_classes[region_index].sumxlogx[band] = double_buffer[double_buffer_index++];
        }

        if (params.region_std_dev_flag)
          region_classes[region_index].band_max_std_dev = double_buffer[double_buffer_index++];

        if (params.region_threshold_flag)
          region_classes[region_index].merge_threshold = double_buffer[double_buffer_index++];

        if (params.region_boundary_npix_flag)
          region_classes[region_index].boundary_npix = int_buffer[int_buffer_index++];

        if (params.region_nghbrs_list_flag) // Always true!
        {
          nghbrs_label_set_size = int_buffer[int_buffer_index++];
          region_classes[region_index].nghbrs_label_set.clear();
          for (index = 0; index < nghbrs_label_set_size; index++)
          {
            region_classes[region_index].nghbrs_label_set.insert(int_buffer[int_buffer_index++]);
          }
        }

        if (params.region_nb_objects_flag)
        {
          region_classes[region_index].nb_region_objects = int_buffer[int_buffer_index++];
        }
      
        if (params.region_objects_list_flag)
        {
          region_classes[region_index].region_objects_set.clear();
          for (index = 0; index < (int) region_classes[region_index].nb_region_objects; index++)
          {
            region_classes[region_index].region_objects_set.insert(int_buffer[int_buffer_index++]);
          }
        }
      } // if (region_classes[region_index].merge_region_label == 0)
    } // for (region_index = 0; region_index < orig_nb_classes; ++region_index)

    nb_objects = object_int_buffer_index = double_buffer_index = 0;
    if (params.region_objects_flag)
    {
      fs_pos = region_objects_ifs.tellg();
      if ((segLevel > 0) && (fs_pos == 0))
      {
        for (hlevel = 0; hlevel < segLevel; hlevel++)
        {
          offset = 0;
          offset += 4*object_int_buffer_size;
          offset += 8*object_double_buffer_size;
          region_objects_ifs.seekg(offset, ios_base::cur);
        }
      }

      region_objects_ifs.read(reinterpret_cast<char *>(int_buffer),4*object_int_buffer_size);
      region_objects_ifs.read(reinterpret_cast<char *>(double_buffer),8*object_double_buffer_size);

      for (region_index = 0; region_index < orig_nb_objects; ++region_index)
      {
        region_objects[region_index].label = int_buffer[object_int_buffer_index++];
        region_objects[region_index].merge_region_label = int_buffer[object_int_buffer_index++];

        region_objects[region_index].active_flag = false;
        if (region_objects[region_index].merge_region_label == 0)
        {
          region_objects[region_index].active_flag = true;
          nb_objects++;
          region_objects[region_index].npix = int_buffer[object_int_buffer_index++];

          if (params.region_sum_flag)
          {
            for (band = 0; band < nbands; band++)
              region_objects[region_index].sum[band] = double_buffer[double_buffer_index++];

            if (params.region_sumsq_flag)
              for (band = 0; band < nbands; band++)
                region_objects[region_index].sumsq[band] = double_buffer[double_buffer_index++];

            if (params.region_sumxlogx_flag)
              for (band = 0; band < nbands; band++)
                region_objects[region_index].sumxlogx[band] = double_buffer[double_buffer_index++];
          }

          if (params.region_std_dev_flag)
            region_objects[region_index].band_max_std_dev = double_buffer[double_buffer_index++];

          if (params.region_boundary_npix_flag)
            region_objects[region_index].boundary_npix = int_buffer[object_int_buffer_index++];
        } // if (region_classes[region_index].merge_region_label == 0)
      } // for (region_index = 0; region_index < orig_nb_objects; ++region_index)
    } // if (params.region_objects_flag)

    return;
  }

  void Results::close_input( )
  {
    region_classes_ifs.close( );
    if (params.region_objects_flag)
      region_objects_ifs.close( );
  }

} // namespace HSEGTilton
