// pan_extract.cc

#include "pan_extract.h"
#include "params/params.h"
#include <image/image.h>
#include <iostream>
#include <gdal_priv.h>

// Externals
extern CommonTilton::Params params;
extern CommonTilton::Image multispectralImage;
extern CommonTilton::Image panchromaticImage;

namespace CommonTilton
{
  bool pan_extract()
  {
    if (multispectralImage.geotransform_valid() && panchromaticImage.geotransform_valid())
    {
      Image extractedPanchromaticImage;
      double X_offset, Y_offset, X_gsd, Y_gsd; //  X and Y offset and ground sampling distance
      int view_col, view_row;
      int ncols, nrows, pan_ncols, pan_nrows;
      double UTM_X, UTM_Y, pan_X_gsd, pan_Y_gsd;
      double UTM_lr_X, UTM_lr_Y;

      X_offset = multispectralImage.get_X_offset();
      Y_offset = multispectralImage.get_Y_offset();
      X_gsd = multispectralImage.get_X_gsd();
      Y_gsd = multispectralImage.get_Y_gsd();
      ncols = multispectralImage.get_ncols();
      nrows = multispectralImage.get_nrows();
      pan_X_gsd = panchromaticImage.get_X_gsd();
      pan_Y_gsd = panchromaticImage.get_Y_gsd();
      UTM_lr_X = X_offset + ncols*X_gsd;
      UTM_lr_Y = Y_offset + nrows*Y_gsd;
      pan_ncols = (UTM_lr_X - X_offset)/pan_X_gsd;
      pan_nrows = (UTM_lr_Y - Y_offset)/pan_Y_gsd;
cout << "Creating extractedPanchromaticImage with ncols = " << pan_ncols << " and pan_nrows = " << pan_nrows << endl;
      extractedPanchromaticImage.create(params.extracted_panchromatic_image_file, pan_ncols, pan_nrows, 1, panchromaticImage.get_data_type(),
                                        panchromaticImage.get_driver_description());
      extractedPanchromaticImage.set_geotransform(X_offset, Y_offset, pan_X_gsd, pan_Y_gsd);
      for (view_row = 0; view_row < pan_nrows; view_row++)
      {
        UTM_Y = Y_offset + view_row*pan_Y_gsd;
        for (view_col = 0; view_col < pan_ncols; view_col++)
        {
          UTM_X = X_offset + view_col*pan_X_gsd;
          extractedPanchromaticImage.put_data(panchromaticImage.get_data(UTM_X,UTM_Y,0),view_col,view_row,0);
        }
      }
      extractedPanchromaticImage.flush_data();
      extractedPanchromaticImage.close();
    }
    multispectralImage.close();
    panchromaticImage.close();

    return true;
  }

} // CommonTilton
