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

    /*-- Range values for RGB Image Stretch (optional - defaults supplied) --*/
      float range[2];

    /*-- Output ASCII examples list file (required) --*/
      string examples_out_file;      /*-- OUTPUT FILE NAME --*/

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

    /*-- Input ASCII examples list file (optional - no default) --*/
      string   examples_in_file;         /*-- OUTPUT FILE NAME --*/
      bool     examples_in_flag;         /*-- FLAG --*/

    /*-- Input panchromatic image file (optional - no default) --*/
      string   panchromatic_image_file;       /*-- INPUT FILE NAME --*/
      bool     panchromatic_image_flag;       /*-- FLAG --*/

    /*-- Input reference image file (optional - no default) --*/
      string   reference_image_file;       /*-- INPUT FILE NAME --*/
      bool     reference_image_flag;       /*-- FLAG --*/

    /*-- Other program parameters (set internally, not user settable) --*/
      int  ncols_subset, nrows_subset, nslices_subset;
      int  ncols_offset, nrows_offset, nslices_offset;
      string   label_data_file, saved_label_data_file;
#ifdef THREEDIM
      string   view_input_image_file;
      string   view_mask_image_file;
      string   view_classes_map_file;
#endif
      string   seg_level_classes_map_file;
      string   region_map_file, saved_region_map_file;
      string   pan_subset_image_file;

    protected:

    private:
  };

} // HSEGTilton

#endif /* INITIALPARAMS_H */
