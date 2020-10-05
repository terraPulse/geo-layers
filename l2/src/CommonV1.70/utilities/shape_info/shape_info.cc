// shapeinfo.cc

#include "shape_info.h"
#include "params/params.h"
#include <shape/shape.h>
#include <iostream>

using namespace std;

// Externals
extern CommonTilton::Params params;
extern CommonTilton::Shape shapeInFile;

namespace CommonTilton
{
  bool shape_info()
  {

    if (shapeInFile.valid())
      cout << endl << "shape_in_file = " << params.shape_in_file << " is a valid Input Shape File" << endl;
    else
      return false;

    shapeInFile.print_shape_info();

    shapeInFile.print_field_info();

    if (shapeInFile.get_shapeType() == SHPT_MULTIPOINT)
      shapeInFile.print_arc_or_polygon();
    else if (shapeInFile.get_shapeType() == SHPT_POLYGON)
      shapeInFile.print_arc_or_polygon();
    else if (shapeInFile.get_shapeType() == SHPT_ARC)
      shapeInFile.print_arc_or_polygon();
    else if (shapeInFile.get_shapeType() == SHPT_POINT)
      shapeInFile.print_point();

    return true;
  }

} // CommonTilton
