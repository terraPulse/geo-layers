// shape_to_image.cc

#include "shape_to_image.h"
#include "params/params.h"
#include <image/image.h>
#include <shape/shape.h>
#include <iostream>
#include <gdal_priv.h>

using namespace std;

// Externals
extern CommonTilton::Params params;
extern CommonTilton::Image maskImage;
extern CommonTilton::Shape shapeFile;

namespace CommonTilton
{
  bool shape_to_image()
  {
    maskImage.print_info();

    Image outputImage;
    if (maskImage.geotransform_valid())
    {
      outputImage.create(params.output_image_file, maskImage, 1, GDT_UInt16, maskImage.get_driver_description());
    }
    else
    {
      cout << "ERROR(shape_to_image): Invalid geotransform information for the input mask image file." << endl;
      return false;
    }

    outputImage.put_data_values(0);
    outputImage.shape_to_image(shapeFile);
    outputImage.print_info();

    maskImage.close();
    outputImage.close();

    return true;
  }

} // CommonTilton
