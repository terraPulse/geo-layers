// register.cc

#include "register.h"
#include "params/params.h"
#include <image/image.h>
#include <iostream>

// Externals
extern CommonTilton::Params params;
extern CommonTilton::Image baseImage;
extern CommonTilton::Image registerImage;

namespace CommonTilton
{
  bool register_image()
  {
    baseImage.print_info();
    registerImage.print_info();

    Image outRegisteredImage;
    outRegisteredImage.create(params.OUT_registered_image_file,baseImage,registerImage.get_nbands(),
                              registerImage.get_data_type(),registerImage.get_driver_description());
    outRegisteredImage.registered_copy(registerImage);
    outRegisteredImage.print_info();

    baseImage.close();
    registerImage.close();
    outRegisteredImage.close();

    return true;
  }

} // CommonTilton
