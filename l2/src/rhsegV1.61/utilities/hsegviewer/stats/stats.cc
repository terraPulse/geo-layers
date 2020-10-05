// stats.cc
#include "stats.h"
#include "../params/initialParams.h"
#include <params/params.h>

extern HSEGTilton::InitialParams initialParams;
extern HSEGTilton::Params params;
extern HSEGTilton::oParams oparams;
extern CommonTilton::Image maskImage;
extern CommonTilton::Image segLevelClassMeanImage;
extern CommonTilton::Image classLabelsMapImage;
extern CommonTilton::Image segLevelClassesMapImage;
extern CommonTilton::Image boundaryMapImage;
extern CommonTilton::Image segLevelBoundaryMapImage;
extern CommonTilton::Image objectLabelsMapImage;
extern CommonTilton::Image segLevelObjectsMapImage;
extern CommonTilton::Image segLevelClassStdDevImage;
extern CommonTilton::Image segLevelObjectStdDevImage;
extern CommonTilton::Image segLevelClassBPRatioImage;
extern CommonTilton::Image segLevelObjectBPRatioImage;

namespace HSEGTilton
{

  Stats::Stats( )
  {
#ifdef THREEDIM
    switch (initialParams.view_dimension)
    {
      case COLUMN: view_ncols = params.nrows;
                   view_nrows = params.nslices;
                   break;
      case    ROW: view_ncols = params.ncols;
                   view_nrows = params.nslices;
                   break;
      case  SLICE: view_ncols = params.ncols;
                   view_nrows = params.nrows;
                   break;
    }
#else
    view_ncols = params.ncols;
    view_nrows = params.nrows;
#endif

    nbands = params.nbands;
    nb_hlevels = oparams.nb_levels;
    orig_nb_classes = oparams.level0_nb_classes;
    orig_nb_objects = oparams.level0_nb_objects;

    unsigned int nelements = orig_nb_classes*(nb_hlevels-1);
    classMergesList = new unsigned int[nelements];
    nelements = orig_nb_classes*nb_hlevels;
    classNOPixelsList = new unsigned int[nelements];
    if (params.region_sum_flag)
      classMeanList = new double[nelements*nbands];
    else
      classMeanList = NULL;
    if (params.region_std_dev_flag)
      classStdDevList = new double[nelements];
    else
      classStdDevList = NULL;
    if (params.region_threshold_flag)
      classMergeThreshList = new double[nelements];
    else
      classMergeThreshList = NULL;
    if (params.region_boundary_npix_flag)
      classBoundNOPixelsList = new unsigned int[nelements];
    else
      classBoundNOPixelsList = NULL;
    if (params.region_nb_objects_flag)
      classNOObjectsList = new unsigned int[nelements];
    else
      classNOObjectsList = NULL;

    if (params.region_objects_flag)
    {
      nelements = orig_nb_objects*(nb_hlevels-1);
      objectMergesList = new unsigned int[nelements];
      nelements = orig_nb_objects*nb_hlevels;
      objectNOPixelsList = new unsigned int[nelements];
      if (params.region_std_dev_flag)
        objectStdDevList = new double[nelements];
      else
        objectStdDevList = NULL;
      if (params.region_boundary_npix_flag)
        objectBoundNOPixelsList = new unsigned int[nelements];
      else
        objectBoundNOPixelsList = NULL;
    }
    else
    {
      objectMergesList = NULL;
      objectNOPixelsList = NULL;
      objectStdDevList = NULL;
      objectBoundNOPixelsList = NULL;
    }

    return;
  }

  Stats::~Stats( )
  {

    return;
  }

  void Stats::set_segLevel(const unsigned int& segLevel)
  {
    int  view_row, view_col, stats_index;
    unsigned int value;
    float  float_value;
    double double_value;
  // Set the region classes label map for the specified hierarchial segmentation level
    for (view_row = 0; view_row < view_nrows; view_row++)
      for (view_col = 0; view_col < view_ncols; view_col++)
      {
        value = (unsigned int) classLabelsMapImage.get_data(view_col,view_row,0);
        value = get_new_class_label(value,segLevel);
        segLevelClassesMapImage.put_data(value,view_col,view_row,0);
      }
    segLevelClassesMapImage.flush_data();

    if (params.boundary_map_flag)
    {
    // Set the boundary map image for the specified hierarchial segmentation level
      bool  mask_flag, mask_valid = maskImage.data_valid();
      for (view_row = 0; view_row < view_nrows; view_row++)
        for (view_col = 0; view_col < view_ncols; view_col++)
        {
          mask_flag = true;
          if (mask_valid)
            mask_flag = maskImage.data_valid(view_col,view_row,0);
          if (mask_flag)
          {
            value = (unsigned int) boundaryMapImage.get_data(view_col,view_row,0);
            if (value < segLevel)
              value = segLevel;
          }
          else
            value = segLevel;
          segLevelBoundaryMapImage.put_data(value,view_col,view_row,0);
        }
      segLevelBoundaryMapImage.flush_data();
    }

    if (params.object_labels_map_flag)
    {
    // Set the region objects map for the specified hierarchial segmentation level
      for (view_row = 0; view_row < view_nrows; view_row++)
        for (view_col = 0; view_col < view_ncols; view_col++)
        {
          value = (unsigned int) objectLabelsMapImage.get_data(view_col,view_row,0);
          value = get_new_object_label(value,segLevel);
          segLevelObjectsMapImage.put_data(value,view_col,view_row,0);
        }
      segLevelObjectsMapImage.flush_data();
    }

  // Set the region mean image for the specified hierarchical segmentation level
    if (params.region_sum_flag)
    {
      unsigned short band;
      for (view_row = 0; view_row < view_nrows; view_row++)
        for (view_col = 0; view_col < view_ncols; view_col++)
        {
          value = (unsigned int) segLevelClassesMapImage.get_data(view_col,view_row,0);
          if (value != 0)
          {
            for (band = 0; band < params.nbands; band++)
            {
              if ((band == initialParams.red_display_band) || 
                  (band == initialParams.green_display_band) || 
                  (band == initialParams.blue_display_band))
              {
                stats_index = band + (value - 1)*params.nbands + 
                              segLevel*orig_nb_classes*params.nbands;
                double_value = get_classMean(stats_index);
                if (band == initialParams.red_display_band)
                  segLevelClassMeanImage.put_data(double_value,view_col,view_row,segLevelClassMeanImage.get_red_display_band());
                if ((band == initialParams.green_display_band) && 
                    (initialParams.green_display_band != initialParams.red_display_band))
                  segLevelClassMeanImage.put_data(double_value,view_col,view_row,segLevelClassMeanImage.get_green_display_band());
                if ((band == initialParams.blue_display_band) &&
                    ((initialParams.blue_display_band != initialParams.red_display_band) &&
                     (initialParams.blue_display_band != initialParams.green_display_band)))
                  segLevelClassMeanImage.put_data(double_value,view_col,view_row,segLevelClassMeanImage.get_blue_display_band());
              }
            }
          }
          else
          {
            segLevelClassMeanImage.put_data(0,view_col,view_row,segLevelClassMeanImage.get_red_display_band());
            if (segLevelClassMeanImage.get_green_display_band() != segLevelClassMeanImage.get_red_display_band())
              segLevelClassMeanImage.put_data(0,view_col,view_row,segLevelClassMeanImage.get_green_display_band());
            if ((segLevelClassMeanImage.get_blue_display_band() != segLevelClassMeanImage.get_red_display_band()) &&
                (segLevelClassMeanImage.get_blue_display_band() != segLevelClassMeanImage.get_green_display_band()))
              segLevelClassMeanImage.put_data(0,view_col,view_row,segLevelClassMeanImage.get_blue_display_band());
          }
        }
      segLevelClassMeanImage.flush_data();
    }
  
  // Set the standard deviation images for the specified hierarchical segmentation level
    if (params.region_std_dev_flag)
    {
      for (view_row = 0; view_row < view_nrows; view_row++)
        for (view_col = 0; view_col < view_ncols; view_col++)
        {
          value = (unsigned int) segLevelClassesMapImage.get_data(view_col,view_row,0);
          float_value = 0.0;
          if (value != 0)
          {
            stats_index = (value - 1) + segLevel*orig_nb_classes;
            float_value = get_classStdDev(stats_index);
          }
          segLevelClassStdDevImage.put_data(float_value,view_col,view_row,0);
        }
      segLevelClassStdDevImage.flush_data();
      segLevelClassStdDevImage.resetMinMax();
      segLevelClassStdDevImage.computeHistoEqMap(0,256);

      if (params.object_labels_map_flag)
      {
        for (view_row = 0; view_row < view_nrows; view_row++)
          for (view_col = 0; view_col < view_ncols; view_col++)
          {
            value = (unsigned int) segLevelObjectsMapImage.get_data(view_col,view_row,0);
            float_value = 0.0;
            if (value != 0)
            {
              stats_index = (value - 1) + segLevel*orig_nb_objects;
              float_value = get_objectStdDev(stats_index);
            }
            segLevelObjectStdDevImage.put_data(float_value,view_col,view_row,0);
          }
        segLevelObjectStdDevImage.flush_data();
        segLevelObjectStdDevImage.resetMinMax();
        segLevelObjectStdDevImage.computeHistoEqMap(0,256);
      }
    }

  // Set the boundary pixel ratio images for the specified hierarchical segmentation level
    if (params.region_boundary_npix_flag)
    {
      for (view_row = 0; view_row < view_nrows; view_row++)
        for (view_col = 0; view_col < view_ncols; view_col++)
        {
          value = (unsigned int) segLevelClassesMapImage.get_data(view_col,view_row,0);
          float_value = 0.0;
          if (value != 0)
          {
            stats_index = (value - 1) + segLevel*orig_nb_classes;
            float_value = get_classBPRatio(stats_index);
          }
          segLevelClassBPRatioImage.put_data(float_value,view_col,view_row,0);
        }
      segLevelClassBPRatioImage.flush_data();
      segLevelClassBPRatioImage.resetMinMax();
      segLevelClassBPRatioImage.computeHistoEqMap(0,256);

      if (params.object_labels_map_flag)
      {
        for (view_row = 0; view_row < view_nrows; view_row++)
          for (view_col = 0; view_col < view_ncols; view_col++)
          {
            value = (unsigned int) segLevelObjectsMapImage.get_data(view_col,view_row,0);
            float_value = 0.0;
            if (value != 0)
            {
              stats_index = (value - 1) + segLevel*orig_nb_objects;
              float_value = get_objectBPRatio(stats_index);
            }
            segLevelObjectBPRatioImage.put_data(float_value,view_col,view_row,0);
          }
        segLevelObjectBPRatioImage.flush_data();
        segLevelObjectBPRatioImage.resetMinMax();
        segLevelObjectBPRatioImage.computeHistoEqMap(0,256);
      }
    }

    return;
  }

  void Stats::compute_and_set_colormap(const unsigned int& segLevel, CommonTilton::Image& inputImage)
  {
    int number_of_active_regions, class_label, merge_index;

    if (segLevel > 0)
    {
      number_of_active_regions = 0;
      for (class_label = 1; class_label <= orig_nb_classes; class_label++)
      {
        merge_index = class_label - 1 + (segLevel-1)*orig_nb_classes;
        if (classMergesList[merge_index] == 0)
          number_of_active_regions++;
      }
    }
    else
      number_of_active_regions = orig_nb_classes;

    float float_value, step_size;
    int value = 0;
    float_value = 0.0;
    step_size = 255.0/number_of_active_regions;
    for (class_label = 1; class_label <= orig_nb_classes; class_label++)
    {
      if (segLevel > 0)
      {
        merge_index = class_label - 1 + (segLevel-1)*orig_nb_classes;
        if (classMergesList[merge_index] == 0)
        {
          float_value += step_size;
          value = (int) (float_value + 0.5);
          inputImage.set_colormap_value(class_label, value, value, value);
        }
        else
          inputImage.set_colormap_value(class_label, 0, 0, 0);
      }
      else
      {
        float_value += step_size;
        value = (int) (float_value + 0.5);
        inputImage.set_colormap_value(class_label, value, value, value);
      }
    }


    return;
  }

  unsigned int Stats::get_new_class_label(const unsigned int& classLabel, 
                                          const int& segLevel)
  {
    unsigned int new_class_label, merge_index;

    new_class_label = classLabel;
    if (segLevel > 0)
    {
      if (classLabel != 0)
      {
        merge_index = classLabel - 1 + (segLevel-1)*orig_nb_classes;
        if (classMergesList[merge_index] != 0)
          new_class_label = classMergesList[merge_index];
      }
    }

    return new_class_label;
  }

  unsigned int Stats::get_new_object_label(const unsigned int& objectLabel, 
                                           const int& segLevel)
  {
    unsigned int new_object_label, merge_index;

    new_object_label = objectLabel;
    if (segLevel > 0)
    {
      if (new_object_label != 0)
      {
        merge_index = new_object_label - 1 + (segLevel-1)*orig_nb_objects;
        if (objectMergesList[merge_index] != 0)
          new_object_label = objectMergesList[merge_index];
      }
    }

    return new_object_label;
  }

  unsigned int Stats::get_class_npix(const unsigned int& classLabel, 
                                     const int& segLevel)
  {
    unsigned int index, npix;

    index = classLabel - 1 + segLevel*orig_nb_classes;
    npix = classNOPixelsList[index];

    return npix;
  }

  unsigned int Stats::get_object_npix(const unsigned int& objectLabel, 
                                      const int& segLevel)
  {
    unsigned int index, npix;

    index = objectLabel - 1 + segLevel*orig_nb_objects;
    npix = objectNOPixelsList[index];

    return npix;
  }

  unsigned int Stats::get_class_boundary_npix(const unsigned int& classLabel, 
                                              const int& segLevel)
  {
    unsigned int index, boundary_npix;

    index = classLabel - 1 + segLevel*orig_nb_classes;
    boundary_npix = classBoundNOPixelsList[index];

    return boundary_npix;
  }

  unsigned int Stats::get_object_boundary_npix(const unsigned int& objectLabel, 
                                               const int& segLevel)
  {
    unsigned int index, boundary_npix;

    index = objectLabel - 1 + segLevel*orig_nb_objects;
    boundary_npix = objectBoundNOPixelsList[index];

    return boundary_npix;
  }

  double Stats::get_class_std_dev(const unsigned int& classLabel, 
                                  const int& segLevel)
  {
    unsigned int index;
    double std_dev;

    index = classLabel - 1 + segLevel*orig_nb_classes;
    std_dev = classStdDevList[index];

    return std_dev;
  }

  double Stats::get_object_std_dev(const unsigned int& objectLabel, 
                                  const int& segLevel)
  {
    unsigned int index;
    double std_dev;

    index = objectLabel - 1 + segLevel*orig_nb_objects;
    std_dev = objectStdDevList[index];

    return std_dev;
  }


} // namespace HSEGTilton

