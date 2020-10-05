// utm_convert.cc

#include "utm_convert.h"
#include <iostream>
#include <proj_api.h>

namespace CommonTilton
{

  bool lat_long_to_utm(const double& latitude, const double& longitude, const int& UTM_zone,
                       double& UTM_X, double& UTM_Y)
  {
    projPJ pj_utm, pj_latlong;
    double x, y;

    string pj_utm_string, pj_latlong_string;

    pj_utm_string = "+proj=utm +ellps=WGS84 +zone=" + stringify_int(UTM_zone);
    pj_latlong_string = "+proj=latlong +ellps=WGS84";

    if (!(pj_utm = pj_init_plus(pj_utm_string.c_str())) )
      return false;
    if (!(pj_latlong = pj_init_plus(pj_latlong_string.c_str())) )
      return false;

    x = longitude*DEG_TO_RAD;
    y = latitude*DEG_TO_RAD;
    pj_transform(pj_latlong, pj_utm, 1, 1, &x, &y, NULL );
    UTM_X = x;
    UTM_Y = y;

    pj_free(pj_utm);
    pj_free(pj_latlong);

    return true;
  }

  bool utm_to_lat_long(const double& UTM_X, const double& UTM_Y, const int& UTM_zone,
                       double& latitude, double& longitude)
  {
    projPJ pj_utm, pj_latlong;
    double x, y;
    string pj_utm_string, pj_latlong_string;

    pj_utm_string = "+proj=utm +ellps=WGS84 +zone=" + stringify_int(UTM_zone);
    pj_latlong_string = "+proj=latlong +ellps=WGS84";

    if (!(pj_utm = pj_init_plus(pj_utm_string.c_str())) )
      return false;
    if (!(pj_latlong = pj_init_plus(pj_latlong_string.c_str())) )
      return false;

    x = UTM_X;
    y = UTM_Y;
    pj_transform(pj_utm, pj_latlong, 1, 1, &x, &y, NULL );
    longitude = x/DEG_TO_RAD;
    latitude = y/DEG_TO_RAD;

    pj_free(pj_utm);
    pj_free(pj_latlong);

    return true;
  }

} // CommonTilton
