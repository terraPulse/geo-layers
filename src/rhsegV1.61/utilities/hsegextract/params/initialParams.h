#ifndef INITIALPARAMS_H
#define INITIALPARAMS_H

#include <string>
#include <fstream>

using namespace std;

namespace HSEGTilton
{
  class InitialParams 
  {
    public:
    // Constructor and Destructor
      InitialParams();
      virtual ~InitialParams();

    // Member functions
      bool read(const char *param_file);
      bool read_oparam();
      void print();

    /*-- RHSEG output parameter file (required) --*/
      string   oparam_file;                     /*-- USER INPUT FILE NAME --*/
      bool     oparam_flag;                     /*-- FLAG --*/

    /*-- Hierarchical segmentation level at which the selected feature values are extracted. (required) --*/
      short  unsigned int hseg_level;           /*-- USER INPUT PARAMETER --*/

    /*-- File name for extracted class labels map --*/
      string   class_labels_map_ext_file;       /*-- USER OUTPUT FILE NAME --*/
      bool     class_labels_map_ext_flag;       /*-- FLAG --*/

    /*-- File name for extracted class # of pixels map --*/
      string   class_npix_map_ext_file;         /*-- USER OUTPUT FILE NAME --*/
      bool     class_npix_map_ext_flag;         /*-- FLAG --*/

    /*-- File name for extracted class mean feature map --*/
      string   class_mean_map_ext_file;         /*-- USER OUTPUT FILE NAME --*/
      bool     class_mean_map_ext_flag;         /*-- FLAG --*/

    /*-- File name for extracted class standard deviation feature map --*/
      string   class_std_dev_map_ext_file;      /*-- USER OUTPUT FILE NAME --*/
      bool     class_std_dev_map_ext_flag;      /*-- FLAG --*/

    /*-- File name for extracted class boundary pixel ratio feature map --*/
      string   class_bpratio_map_ext_file;      /*-- USER OUTPUT FILE NAME --*/
      bool     class_bpratio_map_ext_flag;      /*-- FLAG --*/

#ifdef SHAPEFILE
    /*-- Base file name for extracted class shapefile --*/
      string   class_shapefile_ext_file;        /*-- USER OUTPUT FILE NAME --*/
      bool     class_shapefile_ext_flag;        /*-- FLAG --*/
#endif

    /*-- File name for extracted object labels map --*/
      string   object_labels_map_ext_file;      /*-- USER OUTPUT FILE NAME --*/
      bool     object_labels_map_ext_flag;      /*-- FLAG --*/

    /*-- File name for extracted object # of pixels map --*/
      string   object_npix_map_ext_file;        /*-- USER OUTPUT FILE NAME --*/
      bool     object_npix_map_ext_flag;        /*-- FLAG --*/

    /*-- File name for extracted object mean feature map --*/
      string   object_mean_map_ext_file;        /*-- USER OUTPUT FILE NAME --*/
      bool     object_mean_map_ext_flag;        /*-- FLAG --*/

    /*-- File name for extracted object standard deviation feature map --*/
      string   object_std_dev_map_ext_file;     /*-- USER OUTPUT FILE NAME --*/
      bool     object_std_dev_map_ext_flag;     /*-- FLAG --*/

    /*-- File name for extracted object boundary pixel ratio feature map --*/
      string   object_bpratio_map_ext_file;     /*-- USER OUTPUT FILE NAME --*/
      bool     object_bpratio_map_ext_flag;     /*-- FLAG --*/

#ifdef SHAPEFILE
    /*-- Base file name for extracted object shapefile --*/
      string   object_shapefile_ext_file;       /*-- USER OUTPUT FILE NAME --*/
      bool     object_shapefile_ext_flag;       /*-- FLAG --*/
#endif

    protected:

    private:
  };

} // HSEGTilton

#endif /* INITIALPARAMS_H */
