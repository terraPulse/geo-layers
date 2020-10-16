#ifndef INITIALPARAMS_H
#define INITIALPARAMS_H

#include <defines.h>
#include <string>
#include <fstream>

using namespace std;

namespace HSEGTilton
{

  enum ViewDimension { COLUMN, ROW, SLICE };

  class InitialParams 
  {
    public:
    // Constructor and Destructor
      InitialParams();
      virtual ~InitialParams();

    // Member functions
      bool read(const char *param_file);
      bool read_oparam();
      bool set_internal_parameters();
      void print();

    /*-- RHSEG output parameter file (required) --*/
      string   oparam_file;        /*-- USER INPUT FILE NAME --*/
      bool     oparam_flag;        /*-- FLAG --*/

    /*-- Spectral bands to display as red, green and blue (required) --*/
      int  red_display_band, green_display_band, blue_display_band;
      bool red_display_flag, green_display_flag, blue_display_flag;

    /*-- RGB Image Stretch Option (optional - default supplied) --*/
      int rgb_image_stretch;

    /*-- Percentile Range for RGB Image Stretch (optional - defaults supplied) --*/
      float range[2];

    /*-- Segmentation Grey Scale Display flag (required) --*/
      bool grey_scale_flag;        /*-- FLAG --*/

    /*-- Output class label map file (required) --*/
      string label_out_file;         /*-- OUTPUT FILE NAME --*/

    /*-- Output ASCII class label names list file (required) --*/
      string ascii_out_file;         /*-- OUTPUT FILE NAME --*/

    /*-- Dimension from which a selected element is displayed in 2-D display --*/
      ViewDimension view_dimension;  /* Defaulted to "SLICE" */
      bool view_dimension_flag;

    /*-- Selected element for 2-D display --*/
      int  view_element;   /* Defaulted to 0 */
      bool view_element_flag;

    /*-- Input class label map file (optional - no default) --*/
      string   label_in_file;         /*-- OUTPUT FILE NAME --*/
      bool     label_in_flag;         /*-- FLAG --*/

    /*-- Input ASCII class label names list file (optional - no default) --*/
      string   ascii_in_file;         /*-- OUTPUT FILE NAME --*/
      bool     ascii_in_flag;         /*-- FLAG --*/

    /*-- Input reference file(1) (optional - no default) --*/
      string   reference1_file;       /*-- OUTPUT FILE NAME --*/
      bool     reference1_flag;       /*-- FLAG --*/

    /*-- Input reference file(2) (optional - no default) --*/
      string   reference2_file;       /*-- OUTPUT FILE NAME --*/
      bool     reference2_flag;       /*-- FLAG --*/

    /*-- Input reference file(3) (optional - no default) --*/
      string   reference3_file;       /*-- OUTPUT FILE NAME --*/
      bool     reference3_flag;       /*-- FLAG --*/

    /*-- Other program parameters (set internally, not user settable) --*/
      int  ncols_subset, nrows_subset, nslices_subset;
      int  ncols_offset, nrows_offset, nslices_offset;
      string   label_data_file;
      string   label_mask_file;
#ifdef THREEDIM
      string   view_input_image_file;
      string   view_mask_image_file;
      string   view_classes_map_file;
      string   view_objects_map_file;
      string   view_boundary_map_file;
#endif
      string   seg_level_classes_map_file;
      string   seg_level_objects_map_file;
      string   seg_level_class_mean_file;
      string   seg_level_boundary_map_file;
      string   seg_level_class_std_dev_file;
      string   seg_level_object_std_dev_file;
      string   seg_level_class_bpratio_file;
      string   seg_level_object_bpratio_file;

    protected:

    private:
  };

} // HSEGTilton

#endif /* INITIALPARAMS_H */
