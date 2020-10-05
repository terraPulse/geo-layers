// band_differences.cc

#include "band_differences.h"
#include "params/params.h"
#include <image/image.h>
#include <iostream>

// Externals
extern CommonTilton::Params params;
extern CommonTilton::Image inputImage;
extern CommonTilton::Image maskImage;

namespace CommonTilton
{
  bool band_differences()
  {
    inputImage.print_info();
    if (params.mask_flag)
      maskImage.print_info();

    int band, bandi, bandj;
    string band_diff_image_file;
    Image NIRImage, redImage, bandDiffImage;
#ifdef GDAL
    redImage.create(params.temp_red_image_file, inputImage, 1, inputImage.get_data_type(), "ENVI");
    NIRImage.create(params.temp_NIR_image_file, inputImage, 1, inputImage.get_data_type(), "ENVI");
#else
    redImage.create(params.temp_red_image_file, inputImage, 1, inputImage.get_dtype());
    NIRImage.create(params.temp_NIR_image_file, inputImage, 1, inputImage.get_dtype());
#endif
    redImage.print_info();
    NIRImage.print_info();
    band = 0;
    for (bandj = 0; bandj < params.nbands; bandj++)
    {
      redImage.registered_data_copy(0, inputImage, bandj);
      cout << "Finished copy of band " << bandj << " of input image to \"red\" image" << endl;
      for (bandi = (bandj+1); bandi < params.nbands; bandi++)
      {
        NIRImage.registered_data_copy(0, inputImage, bandi);
        cout << "Finished copy of band " << bandi << " of input image to \"NIR\" image" << endl;
        band_diff_image_file = params.band_diff_image_file + stringify_int(band);
#ifdef GDAL
        bandDiffImage.create(band_diff_image_file, inputImage, 1, GDT_Float32, "ENVI");
#else
        bandDiffImage.create(band_diff_image_file, inputImage, 1, Float32);
#endif
        bandDiffImage.print_info();
        cout << "Computing \"NIR\" band " << bandi << " minus \"red\" band " << bandj << " as normalized band difference band " << band << endl;
        if (params.mask_flag)
          compute_ndvi(bandDiffImage,NIRImage,redImage,maskImage);
        else
          compute_ndvi(bandDiffImage,NIRImage,redImage);
        cout << "Finished computing normalize band difference image as band " << band << endl;
        bandDiffImage.close();
        cout << "Finished writing normalize band difference image " << band_diff_image_file << endl;
        band++;
      }
    }

    inputImage.close();
    redImage.close();
    NIRImage.close();

    return true;
  }

} // CommonTilton
