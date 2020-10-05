#ifndef UTM_CONVERT_H
#define UTM_CONVERT_H

#include <string>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace CommonTilton
{
  class BadConversion : public runtime_error
  {
    public:
      BadConversion(const string& s) : runtime_error(s)
      { }
  };

  inline string stringify_int(int x)
  {
    ostringstream o;
    if (!(o << x))
      throw BadConversion("stringify_int(int)");
    return o.str();
  }

  bool lat_long_to_utm(const double& latitude, const double& longitude, const int& UTM_zone,
                       double& UTM_X, double& UTM_Y);
  bool utm_to_lat_long(const double& UTM_X, const double& UTM_Y, const int& UTM_zone,
                       double& latitude, double& longitude);

} // CommonTilton

#endif // UTM_CONVERT_H

