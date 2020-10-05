/*-----------------------------------------------------------
|
|  Routine Name: hsegextract - Program to extract region features from thhe outputs from the Hierarchical Segmentation (HSEG)
|                              or Recursive HSEG (RHSEG) programs.
|
|       Purpose: Main function for the hsegextract program
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
|       Written: July 6, 2009 (Based on earlier versions of rhseg_view/hsegviewer and feature_extract programs)
| Modifications: May 10, 2011 - Added capability to convert information from selected hierarchical level to a shapefile 
|                               (raster to vector conversion)
|
------------------------------------------------------------*/
#include "hsegextract.h"
#include "params/initialParams.h"
#include <params/params.h>
#include <spatial/spatial.h>
#include <region/region_class.h>
#include <region/region_object.h>
#include <results/results.h>
#include <iostream>

extern HSEGTilton::InitialParams initialParams;
extern HSEGTilton::Params params;
extern HSEGTilton::oParams oparams;

namespace HSEGTilton
{
  bool hsegextract()
  {
    Spatial spatial_data;

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
    results_data.read(initialParams.hseg_level,nb_classes,nb_objects,oparams.nb_levels,oparams.int_buffer_size,region_classes,region_objects);
    results_data.close_input( );

    spatial_data.read_region_maps();
    spatial_data.update_region_maps(initialParams.hseg_level,region_classes,region_objects);

    if (initialParams.class_labels_map_ext_flag)
      spatial_data.write_class_labels_map_ext(initialParams.class_labels_map_ext_file, region_classes);
    if (initialParams.class_npix_map_ext_flag)
      spatial_data.write_class_npix_map_ext(initialParams.class_npix_map_ext_file, region_classes);
    if (initialParams.class_mean_map_ext_flag)
      spatial_data.write_class_mean_map_ext(initialParams.class_mean_map_ext_file, region_classes);
    if (initialParams.class_std_dev_map_ext_flag)
      spatial_data.write_class_std_dev_map_ext(initialParams.class_std_dev_map_ext_file, region_classes);
    if (initialParams.class_bpratio_map_ext_flag)
      spatial_data.write_class_bpratio_map_ext(initialParams.class_bpratio_map_ext_file, region_classes);
    if (initialParams.object_labels_map_ext_flag)
      spatial_data.write_object_labels_map_ext(initialParams.object_labels_map_ext_file, region_objects);
    if (initialParams.object_npix_map_ext_flag)
      spatial_data.write_object_npix_map_ext(initialParams.object_npix_map_ext_file, region_objects);
    if (initialParams.object_mean_map_ext_flag)
      spatial_data.write_object_mean_map_ext(initialParams.object_mean_map_ext_file, region_objects);
    if (initialParams.object_std_dev_map_ext_flag)
      spatial_data.write_object_std_dev_map_ext(initialParams.object_std_dev_map_ext_file, region_objects);
    if (initialParams.object_bpratio_map_ext_flag)
      spatial_data.write_object_bpratio_map_ext(initialParams.object_bpratio_map_ext_file, region_objects);
#ifdef SHAPEFILE
    if (params.region_objects_flag)
    {
      params.set_object_maxnbdir();
      if ((initialParams.class_shapefile_ext_flag) && (initialParams.object_shapefile_ext_flag))
      {
        spatial_data.write_class_object_shapefile_ext(initialParams.class_shapefile_ext_file, initialParams.object_shapefile_ext_file,
                                                      initialParams.hseg_level, region_classes, region_objects);
      }
      else 
      {
        if (initialParams.object_shapefile_ext_flag)
          spatial_data.write_object_shapefile_ext(initialParams.class_shapefile_ext_file, initialParams.object_shapefile_ext_file,
                                                  initialParams.hseg_level, region_classes, region_objects);
        else if (initialParams.class_shapefile_ext_flag)
          cout << "ERROR: A region class shapefile cannot be produced without a region object shapefile." << endl;
      }
    }
    else
    {
      if (initialParams.class_shapefile_ext_flag)
        spatial_data.write_object_shapefile_ext(initialParams.class_shapefile_ext_file, initialParams.object_shapefile_ext_file,
                                                initialParams.hseg_level, region_classes, region_objects);
    // The below case should not happen (program should abort in the initialParams routine).
      if (initialParams.object_shapefile_ext_flag)
        cout << "ERROR: A region object shapefile cannot be produced for this case." << endl;
    }
#endif

    return true;
  }

} // namespace HSEGTilton
