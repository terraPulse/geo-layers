// shape_overlay.cc

#include "shape_overlay.h"
#include "params/params.h"
#include <image/image.h>
#include <shape/shape.h>
#include <iostream>
#include <gdal_priv.h>

using namespace std;

// Externals
extern CommonTilton::Params params;
extern CommonTilton::Image inputImage;

namespace CommonTilton
{
  bool shape_overlay()
  {
    int band, nbands;

    inputImage.print_info();

    Image tempOutputImage;
    nbands = inputImage.get_nbands();
    if (nbands > 3)
      nbands = 3;
    tempOutputImage.create(params.temp_output_image_file, inputImage, nbands);
    for (band = 0; band < nbands; band++)
      tempOutputImage.stretch(band,inputImage,255);
    tempOutputImage.flush_data();
    Image outputImage;
    outputImage.create(params.output_image_file, tempOutputImage, 3, 
                       GDT_Byte, tempOutputImage.get_driver_description());
    for (band = 0; band < nbands; band++)
      outputImage.registered_data_copy(band, tempOutputImage, band);
    if (nbands == 1)
      outputImage.registered_data_copy(1, tempOutputImage, 0);
    if (nbands == 2)
      outputImage.registered_data_copy(2, tempOutputImage, 1);
    outputImage.flush_data();
    tempOutputImage.close();

    outputImage.overlay_copy(params.shape_file,params.red,params.green,params.blue);
    outputImage.flush_data();

    outputImage.print_info();

    inputImage.close();
    outputImage.close();

    return true;
  }

} // CommonTilton
