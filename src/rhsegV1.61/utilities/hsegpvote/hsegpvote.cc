/*-----------------------------------------------------------
|
|  Routine Name: hsegpvote - Compute the plurality vote region-based classification from a pixel-based classification
|			     and region segmentation for all hierarchical levels from an RHSEG segmentation
|			     and output the overall classification accuracy for all levels to a log file.
|
|       Purpose: Main function for the hsegpvote program
|
|         Input: 
|
|        Output: 
|
|       Returns: TRUE (1) on success, FALSE (0) on failure
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: January 14, 2010
| Modifications: January 17, 2010 - Added 4nn connected component labeling.
|                August 10, 2010 - Added Kappa statistic and average and individual class accuracies
|                August 11, 2010 - Added output of class optimized classification
|
------------------------------------------------------------*/

#include "hsegpvote.h"
#include "params/initialParams.h"
#include <params/params.h>
#include <iostream>

extern HSEGTilton::InitialParams initialParams;
extern HSEGTilton::Params params;
extern HSEGTilton::oParams oparams;
extern Image pixelClassImage;
extern Image trainingLabelImage;

namespace HSEGTilton
{
  bool hsegpvote()
  {
    initialParams.print();

    Image RHSEGLabelImage;
    if ((params.object_labels_map_flag) && (!initialParams.region_class_flag))
    {
      RHSEGLabelImage.open(params.object_labels_map_file);
      if (!RHSEGLabelImage.info_valid())
      {
        cout << "ERROR: " << params.object_labels_map_file << " is not a valid object labels map image file" << endl;
cout << "Exiting with status = false" << endl;
        return EXIT_FAILURE;
      }
    }
    else
    {
      RHSEGLabelImage.open(params.class_labels_map_file);
      if (!RHSEGLabelImage.info_valid())
      {
        cout << "ERROR: " << params.class_labels_map_file << " is not a valid class labels map image file" << endl;
cout << "Exiting with status = false" << endl;
        return EXIT_FAILURE;
      }
    }
    RHSEGLabelImage.print_info();

    int ncols = pixelClassImage.get_ncols();
    int nrows = pixelClassImage.get_nrows();

    if ((ncols != RHSEGLabelImage.get_ncols()) || (nrows != RHSEGLabelImage.get_nrows()))
    {
      cout << "ERROR: Image size mismatch between pixelClass image and regionSeg image" << endl;
      return false;
    }

    Image segLevelLabelImage;
    segLevelLabelImage.create(initialParams.temp_seg_level_label_file, RHSEGLabelImage,
                                       1, RHSEGLabelImage.get_data_type(), "GTiff");
    segLevelLabelImage.registered_data_copy(0,RHSEGLabelImage,0);
    segLevelLabelImage.flush_data();
    segLevelLabelImage.computeMinMax();
    segLevelLabelImage.print_info();

    int max_pixelClass_value, max_regionSeg_value;
    Image objectLabelImage;
    if ((params.maxnbdir > NB_LIMIT) || 
        ((!params.object_labels_map_flag) && (!initialParams.region_class_flag)))
    {
      objectLabelImage.create(initialParams.temp_object_label_file, RHSEGLabelImage,
                                       1, RHSEGLabelImage.get_data_type(), "GTiff");
      connected_component(segLevelLabelImage,objectLabelImage);
      objectLabelImage.computeMinMax();
      objectLabelImage.print_info();
      max_regionSeg_value = (int) objectLabelImage.getMaximum(0) + 1;
    }
    else
      max_regionSeg_value = (int) segLevelLabelImage.getMaximum(0) + 1;
    max_pixelClass_value = (int) pixelClassImage.getMaximum(0) + 1;

    unsigned int *contable;
    int pixelClass_value, regionSeg_value;
    int pixelClass_index, regionSeg_index;
    int index, col, row;
    contable = new unsigned int [max_pixelClass_value*max_regionSeg_value];
    unsigned int *vote_table;
    vote_table = new unsigned int [max_regionSeg_value];
    int vote_value, max_vote_value;

    Image regionClassImage;
    int nbands = 1;
    GDALDataType data_type = GDT_Byte;
    regionClassImage.create(initialParams.temp_region_class_file.c_str(), ncols, nrows, nbands, data_type, "GTiff");
    regionClassImage.set_no_data_value(0.0);
    regionClassImage.put_no_data_value();
    regionClassImage.print_info();

    int training_value, classified_value;
    int training_index, classified_index;
    unsigned int total_entries;
    unsigned int matching_entries;

    int nb_classes = oparams.level0_nb_classes;
    RegionClass::set_static_vals();
    int region_classes_size = nb_classes;
    vector<RegionClass> region_classes(region_classes_size);

    int nb_objects = 1;
    if (params.region_nb_objects_flag)
      nb_objects = oparams.level0_nb_objects;
    RegionObject::set_static_vals(false,false);
    int region_objects_size = nb_objects;
    vector<RegionObject> region_objects(region_objects_size);
    if (!params.region_nb_objects_flag)
      nb_objects = 0;

    Results results_data;
    results_data.set_buffer_sizes(params.nbands,nb_classes,nb_objects);
    results_data.open_input(params.region_classes_file,params.region_objects_file);

    unsigned int *total_for_class, *total_for_training, nb_training_classes = 0;
    float *class_accuracy, *overall_accuracy, *average_accuracy, *kappa;
    double observed_sum, expected_sum;
    total_for_class = new unsigned int [max_pixelClass_value];
    total_for_training = new unsigned int [max_pixelClass_value];
    overall_accuracy = new float [oparams.nb_levels];
    average_accuracy = new float [oparams.nb_levels];
    kappa = new float [oparams.nb_levels];
    class_accuracy = new float [max_pixelClass_value*oparams.nb_levels];

  // Need total_for_training now for the case of non-contiguous labeling!!
    for (training_index = 0; training_index < max_pixelClass_value; training_index++)
      total_for_training[training_index] = 0;
    for (row = 0; row < nrows; row++)
      for (col = 0; col < ncols; col++)
      {
        training_value = (int) trainingLabelImage.get_data(col,row,0);
        if (training_value > 0)
          total_for_training[training_value] += 1;
      }

    params.log_fs << "Accuracies for each hierarchical segmentation level:" << endl << endl;
    params.log_fs << "                     Overall   Average   Kappa  ";
    for (training_index = 1; training_index < max_pixelClass_value; training_index++)
      if (total_for_training[training_index] > 0)
      {
        nb_training_classes++;
        params.log_fs << "   Class " << training_index;
      }
    params.log_fs << endl;

    int segLevel, minBestSegLevel = 0;
    int maxBestSegLevel = 0;
    float best_overall_accuracy = 0.0;
    for (segLevel = 0; segLevel < oparams.nb_levels; segLevel++)
    {
      results_data.read(segLevel,nb_classes,nb_objects,oparams.nb_levels,oparams.int_buffer_size,region_classes,region_objects);

     // Find plurality vote classification
      if (segLevel > 0)
        set_segLevelLabel(ncols,nrows,segLevel,region_classes,region_objects,RHSEGLabelImage,segLevelLabelImage);

      for (regionSeg_index = 0; regionSeg_index < max_regionSeg_value; regionSeg_index++)
        for (pixelClass_index = 0; pixelClass_index < max_pixelClass_value; pixelClass_index++)
        {
          index = pixelClass_index + regionSeg_index*max_pixelClass_value;
          contable[index] = 0;
        }

      if ((params.maxnbdir > NB_LIMIT) || 
          ((!params.object_labels_map_flag) && (!initialParams.region_class_flag)))
      {
        if (segLevel > 0)
          connected_component(segLevelLabelImage,objectLabelImage);
        for (row = 0; row < nrows; row++)
          for (col = 0; col < ncols; col++)
          {
            pixelClass_value = (int) pixelClassImage.get_data(col,row,0);
            regionSeg_value = (int) objectLabelImage.get_data(col,row,0);
            index = pixelClass_value + regionSeg_value*max_pixelClass_value;
            contable[index] += 1;
          }
      }
      else
      {
        for (row = 0; row < nrows; row++)
          for (col = 0; col < ncols; col++)
          {
            pixelClass_value = (int) pixelClassImage.get_data(col,row,0);
            regionSeg_value = (int) segLevelLabelImage.get_data(col,row,0);
            index = pixelClass_value + regionSeg_value*max_pixelClass_value;
            contable[index] += 1;
          }
      }

      for (regionSeg_index = 0; regionSeg_index < max_regionSeg_value; regionSeg_index++)
      {
        index = regionSeg_index*max_pixelClass_value;
        max_vote_value = contable[index];
        vote_table[regionSeg_index] = 0;
        for (pixelClass_index = 1; pixelClass_index < max_pixelClass_value; pixelClass_index++)
        {
          index = pixelClass_index + regionSeg_index*max_pixelClass_value;
          vote_value = contable[index];
          if (vote_value > max_vote_value)
          {
            max_vote_value = vote_value;
            vote_table[regionSeg_index] = pixelClass_index;
          }
        }
      }

      if ((params.maxnbdir > NB_LIMIT) || 
          ((!params.object_labels_map_flag) && (!initialParams.region_class_flag)))
      {
        for (row = 0; row < nrows; row++)
          for (col = 0; col < ncols; col++)
          {
            regionSeg_value = (int) objectLabelImage.get_data(col,row,0);
            pixelClass_value = vote_table[regionSeg_value];
            regionClassImage.put_data(pixelClass_value,col,row,0);
          }
      }
      else
      {
        for (row = 0; row < nrows; row++)
          for (col = 0; col < ncols; col++)
          {
            regionSeg_value = (int) segLevelLabelImage.get_data(col,row,0);
            pixelClass_value = vote_table[regionSeg_value];
            regionClassImage.put_data(pixelClass_value,col,row,0);
          }
      }
      regionClassImage.flush_data();

    // Compute contingency table and associated accuracies
      for (classified_index = 0; classified_index < max_pixelClass_value; classified_index++)
      {
        total_for_class[classified_index] = 0;
        for (training_index = 0; training_index < max_pixelClass_value; training_index++)
        {
          index = training_index + max_pixelClass_value*classified_index;
          contable[index] = 0;
        }
      }

      for (row = 0; row < nrows; row++)
        for (col = 0; col < ncols; col++)
        {
          training_value = (int) trainingLabelImage.get_data(col,row,0);
          classified_value = (int) regionClassImage.get_data(col,row,0);
          index = training_value + max_pixelClass_value*classified_value;
          contable[index] += 1;
        }

      total_entries = 0;
      matching_entries = 0;
      for (classified_index = 1; classified_index < max_pixelClass_value; classified_index++)
      {
        for (training_index = 1; training_index < max_pixelClass_value; training_index++)
        {
          index = training_index + max_pixelClass_value*classified_index;
          total_entries += contable[index];
          if (training_index == classified_index)
            matching_entries += contable[index];
          total_for_class[classified_index] += contable[index];
        }
      }
      overall_accuracy[segLevel] = 100*(((float) matching_entries)/((float) total_entries));
      average_accuracy[segLevel] = 0.0;
      for (training_index = 1; training_index < max_pixelClass_value; training_index++)
        if (total_for_training[training_index] > 0)
        {
          index = training_index + max_pixelClass_value*training_index;
          classified_index = training_index + segLevel*max_pixelClass_value;
          class_accuracy[classified_index] = 100*(((float) contable[index])/((float) total_for_training[training_index]));
          average_accuracy[segLevel] += class_accuracy[classified_index];
        }
      average_accuracy[segLevel] = average_accuracy[segLevel]/((float) nb_training_classes);

      observed_sum = 0.0;
      expected_sum = 0.0;
      for (training_index = 1; training_index < max_pixelClass_value; training_index++)
        if (total_for_training[training_index] > 0)
        {
          index = training_index + max_pixelClass_value*training_index;
          observed_sum += contable[index];
          expected_sum += ((double) total_for_training[training_index])*((double) total_for_class[training_index]);
        }
      kappa[segLevel] = 100*((((double) total_entries)*observed_sum) - expected_sum)/((((double) total_entries)*((double) total_entries)) - expected_sum);

      if (segLevel < 10)
        params.log_fs << "For segLevel   " << segLevel << ": ";
      else if (segLevel < 100)
        params.log_fs << "For segLevel  " << segLevel << ": ";
      else    
        params.log_fs << "For segLevel " << segLevel << ": ";
      params.log_fs.width(10);
      params.log_fs << overall_accuracy[segLevel];
      params.log_fs.width(10);
      params.log_fs << average_accuracy[segLevel];
      params.log_fs.width(10);
      params.log_fs << kappa[segLevel];
      for (training_index = 1; training_index < max_pixelClass_value; training_index++)
        if (total_for_training[training_index] > 0)
        {
          classified_index = training_index + segLevel*max_pixelClass_value;
          params.log_fs.width(10);
          params.log_fs << class_accuracy[classified_index];
        }
      params.log_fs << endl;

      if (overall_accuracy[segLevel] == best_overall_accuracy)
        maxBestSegLevel = segLevel;
      if (overall_accuracy[segLevel] > best_overall_accuracy)
      {
        minBestSegLevel = segLevel;
        maxBestSegLevel = segLevel;
        best_overall_accuracy = overall_accuracy[segLevel];
      }
    }
    results_data.close_input( );

    int lower_limit, upper_limit;
    if (minBestSegLevel != maxBestSegLevel)
    {
      float best_average_accuracy = 0.0;
      lower_limit = minBestSegLevel;
      upper_limit = maxBestSegLevel;
      for (segLevel = lower_limit; segLevel <= upper_limit; segLevel++)
      {
        if (average_accuracy[segLevel] == best_average_accuracy)
          maxBestSegLevel = segLevel;
        if (average_accuracy[segLevel] > best_average_accuracy)
        {
          minBestSegLevel = segLevel;
          maxBestSegLevel = segLevel;
          best_average_accuracy = average_accuracy[segLevel];
        }
      }
    }

    int bestSegLevel = (maxBestSegLevel + minBestSegLevel)/2;
//    int bestSegLevel = maxBestSegLevel;
    int *bestClassLevel, *minBestClassLevel, *maxBestClassLevel;
    bestClassLevel = new int [max_pixelClass_value];
    minBestClassLevel = new int [max_pixelClass_value];
    maxBestClassLevel = new int [max_pixelClass_value];
    float *bestClassAccuracy;
    bestClassAccuracy = new float [max_pixelClass_value];
    for (training_index = 1; training_index < max_pixelClass_value; training_index++)
      if (total_for_training[training_index] > 0)
      {
        classified_index = training_index + bestSegLevel*max_pixelClass_value;
        bestClassLevel[training_index] = bestSegLevel;
        bestClassAccuracy[training_index] = class_accuracy[classified_index];
      }

    for (training_index = 1; training_index < max_pixelClass_value; training_index++)
    {
      if (total_for_training[training_index] > 0)
      {
/*
        for (segLevel = (bestSegLevel - 1); segLevel >= 0; segLevel--)
        {
          classified_index = training_index + segLevel*max_pixelClass_value;
          if (bestClassAccuracy[training_index] < class_accuracy[classified_index])
          {
            bestClassLevel[training_index] = segLevel;
            bestClassAccuracy[training_index] = class_accuracy[classified_index];
          }
        }
        for (segLevel = (bestSegLevel + 1); segLevel < oparams.nb_levels; segLevel++)
        {
          classified_index = training_index + segLevel*max_pixelClass_value;
          if (bestClassAccuracy[training_index] < class_accuracy[classified_index])
          {
            bestClassLevel[training_index] = segLevel;
            bestClassAccuracy[training_index] = class_accuracy[classified_index];
          }
        }
*/
        minBestClassLevel[training_index] = 0;
        maxBestClassLevel[training_index] = 0;
        bestClassAccuracy[training_index] = 0.0;
        for (segLevel = 0; segLevel < oparams.nb_levels; segLevel++)
        {
          classified_index = training_index + segLevel*max_pixelClass_value;
          if (bestClassAccuracy[training_index] == class_accuracy[classified_index])
          {
            maxBestClassLevel[training_index] = segLevel;
          }
          if (bestClassAccuracy[training_index] < class_accuracy[classified_index])
          {
            minBestClassLevel[training_index] = segLevel;
            maxBestClassLevel[training_index] = segLevel;
            bestClassAccuracy[training_index] = class_accuracy[classified_index];
          }
        }
      }
    }    

    params.log_fs << "Best segLevels:   ";
    params.log_fs.width(10);
    params.log_fs << bestSegLevel << "                    ";
    for (training_index = 1; training_index < max_pixelClass_value; training_index++)
      if (total_for_training[training_index] > 0)
      {
        if ((minBestClassLevel[training_index] <= bestSegLevel) && (maxBestClassLevel[training_index] >= bestSegLevel))
          bestClassLevel[training_index] = bestSegLevel;
        else
          bestClassLevel[training_index] = (minBestClassLevel[training_index] + maxBestClassLevel[training_index])/2;
        params.log_fs.width(10);
        params.log_fs << bestClassLevel[training_index];
      }
    params.log_fs << endl;

    Image optClassImage;
    optClassImage.create(initialParams.opt_class_file, pixelClassImage,
                         pixelClassImage.get_nbands(), pixelClassImage.get_data_type(),
                         pixelClassImage.get_driver_description());
    Image maskImage;
    nbands = 1;
    data_type = GDT_Byte;
    maskImage.create(initialParams.temp_mask_file.c_str(), ncols, nrows, nbands, data_type, "GTiff");
    maskImage.put_data_values(1.0);
    maskImage.set_no_data_value(0.0);
    maskImage.put_no_data_value();

    bool process_flag;
    for (segLevel = 0; segLevel < oparams.nb_levels; segLevel++)
    {
      process_flag = false;
      for (training_index = 1; training_index < max_pixelClass_value; training_index++)
        if (total_for_training[training_index] > 0)
        {
          if (bestClassLevel[training_index] == segLevel)
            process_flag = true;
          if (process_flag)
            break;
        }

     if (process_flag)
     {
      params.log_fs << "Doing plurality vote classification at segLevel = " << segLevel << endl;
     // Find plurality vote classification
      results_data.open_input(params.region_classes_file,params.region_objects_file);
      results_data.read(segLevel,nb_classes,nb_objects,oparams.nb_levels,oparams.int_buffer_size,region_classes,region_objects);
      results_data.close_input( );

      if (segLevel > 0)
        set_segLevelLabel(ncols,nrows,segLevel,region_classes,region_objects,RHSEGLabelImage,segLevelLabelImage);

      for (regionSeg_index = 0; regionSeg_index < max_regionSeg_value; regionSeg_index++)
        for (pixelClass_index = 0; pixelClass_index < max_pixelClass_value; pixelClass_index++)
        {
          index = pixelClass_index + regionSeg_index*max_pixelClass_value;
          contable[index] = 0;
        }

      if ((params.maxnbdir > NB_LIMIT) || 
          ((!params.object_labels_map_flag) && (!initialParams.region_class_flag)))
      {
        if (segLevel > 0)
          connected_component(segLevelLabelImage,objectLabelImage);
        for (row = 0; row < nrows; row++)
          for (col = 0; col < ncols; col++)
          {
            pixelClass_value = (int) pixelClassImage.get_data(col,row,0);
            regionSeg_value = (int) objectLabelImage.get_data(col,row,0);
            index = pixelClass_value + regionSeg_value*max_pixelClass_value;
            contable[index] += 1;
          }
      }
      else
      {
        for (row = 0; row < nrows; row++)
          for (col = 0; col < ncols; col++)
          {
            pixelClass_value = (int) pixelClassImage.get_data(col,row,0);
            regionSeg_value = (int) segLevelLabelImage.get_data(col,row,0);
            index = pixelClass_value + regionSeg_value*max_pixelClass_value;
            contable[index] += 1;
          }
      }

      for (regionSeg_index = 0; regionSeg_index < max_regionSeg_value; regionSeg_index++)
      {
        index = regionSeg_index*max_pixelClass_value;
        max_vote_value = contable[index];
        vote_table[regionSeg_index] = 0;
        for (pixelClass_index = 1; pixelClass_index < max_pixelClass_value; pixelClass_index++)
        {
          index = pixelClass_index + regionSeg_index*max_pixelClass_value;
          vote_value = contable[index];
          if (vote_value > max_vote_value)
          {
            max_vote_value = vote_value;
            vote_table[regionSeg_index] = pixelClass_index;
          }
        }
      }

      if ((params.maxnbdir > NB_LIMIT) || 
          ((!params.object_labels_map_flag) && (!initialParams.region_class_flag)))
      {
        for (row = 0; row < nrows; row++)
          for (col = 0; col < ncols; col++)
          {
            regionSeg_value = (int) objectLabelImage.get_data(col,row,0);
            pixelClass_value = vote_table[regionSeg_value];
            regionClassImage.put_data(pixelClass_value,col,row,0);
          }
      }
      else
      {
        for (row = 0; row < nrows; row++)
          for (col = 0; col < ncols; col++)
          {
            regionSeg_value = (int) segLevelLabelImage.get_data(col,row,0);
            pixelClass_value = vote_table[regionSeg_value];
            regionClassImage.put_data(pixelClass_value,col,row,0);
          }
      }
      regionClassImage.flush_data();

      for (row = 0; row < nrows; row++)
        for (col = 0; col < ncols; col++)
        {
          if (segLevel <= bestSegLevel)
          {
            pixelClass_value = (int) regionClassImage.get_data(col,row,0);
            if ((maskImage.get_data(col,row,0) == 1) || (bestClassLevel[pixelClass_value] == segLevel))
            {
              optClassImage.put_data(pixelClass_value,col,row,0);
//              if (bestClassLevel[pixelClass_value] <= segLevel)
              if (bestClassLevel[pixelClass_value] == segLevel)
                maskImage.put_data(0,col,row,0);
            }
          }
          else
          {
            pixelClass_value = (int) regionClassImage.get_data(col,row,0);
            if ((maskImage.get_data(col,row,0) == 1) && (bestClassLevel[pixelClass_value] == segLevel))
            {
              optClassImage.put_data(pixelClass_value,col,row,0);
              maskImage.put_data(0,col,row,0);
            }
          }
        }
      maskImage.flush_data();
      optClassImage.flush_data();
     }
    }

    optClassImage.set_no_data_value(0.0);
    optClassImage.put_no_data_value();
    if (initialParams.color_table_flag)
    {
      GDALColorTable colorTable(GPI_RGB);
      GDALColorEntry *colorEntry;
      colorEntry = new GDALColorEntry;
   
      fstream color_table_fs;
      color_table_fs.open(initialParams.color_table_file.c_str(),ios_base::in);

      int entry_index, entry_value;
      int sub_pos;
      string line,sub_string;

      for (entry_index = 0; entry_index < 256; entry_index++)
      {
        getline(color_table_fs,line);
        sub_pos = line.find(":");
        sub_string = line.substr(0,sub_pos);
        entry_value = atoi(sub_string.c_str());
        if (entry_value != entry_index)
        {
          cout << "ERROR: Color Table has unexpected format" << endl;
          return false;
        }
        line = line.substr(sub_pos+1);
        sub_pos = line.find(",");
        sub_string = line.substr(0,sub_pos);
        colorEntry->c1 = atoi(sub_string.c_str());
        line = line.substr(sub_pos+1);
        sub_pos = line.find(",");
        sub_string = line.substr(0,sub_pos);
        colorEntry->c2 = atoi(sub_string.c_str());
        line = line.substr(sub_pos+1);
        sub_pos = line.find(",");
        sub_string = line.substr(0,sub_pos);
        colorEntry->c3 = atoi(sub_string.c_str());
        line = line.substr(sub_pos+1);
        sub_pos = line.find(",");
        sub_string = line.substr(0,sub_pos);
        colorEntry->c4 = atoi(sub_string.c_str());
        colorTable.SetColorEntry(entry_value,colorEntry);
      }
      color_table_fs.close();

      optClassImage.get_imageDataset()->GetRasterBand(1)->SetColorTable(&colorTable);

    }
    else if (pixelClassImage.get_imageDataset()->GetRasterBand(1)->GetColorTable() != NULL)
      optClassImage.get_imageDataset()->GetRasterBand(1)->SetColorTable(pixelClassImage.get_imageDataset()->GetRasterBand(1)->GetColorTable());
    optClassImage.close();

    maskImage.close();

    RHSEGLabelImage.close();

    segLevelLabelImage.close();
    if ((params.maxnbdir > NB_LIMIT) || 
        ((!params.object_labels_map_flag) && (!initialParams.region_class_flag)))
    regionClassImage.close();

    return true;
  }

  void set_segLevelLabel(const int& ncols, const int& nrows, const int& segLevel,
                         vector<RegionClass>& region_classes, vector<RegionObject>& region_objects,
                         Image& RHSEGLabelImage, Image& segLevelLabelImage)
  {
    int col, row, region_label, region_index;

    if ((params.region_nb_objects_flag) && (!initialParams.region_class_flag))
    {
      for (row = 0; row < nrows; row++)
        for (col = 0; col < ncols; col++)
        {
          region_label = (int) RHSEGLabelImage.get_data(col,row,0);
          if ((segLevel > 0) && (region_label > 0))
          {
            region_index = region_label - 1;
            if (!region_objects[region_index].get_active_flag())
              region_label = region_objects[region_index].get_merge_region_label();
          }
          segLevelLabelImage.put_data(region_label,col,row,0);
        }
      segLevelLabelImage.flush_data();
    }
    else
    {
      for (row = 0; row < nrows; row++)
        for (col = 0; col < ncols; col++)
        {
          region_label = (int) RHSEGLabelImage.get_data(col,row,0);
          if ((segLevel > 0) && (region_label > 0))
          {
            region_index = region_label - 1;
            if (!region_classes[region_index].get_active_flag())
              region_label = region_classes[region_index].get_merge_region_label();
          }
          segLevelLabelImage.put_data(region_label,col,row,0);
        }
      segLevelLabelImage.flush_data();
    }

    return;
  }

} // namespace HSEGTilton

