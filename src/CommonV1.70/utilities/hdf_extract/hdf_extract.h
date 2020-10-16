#ifndef HDF_EXTRACT_H
#define HDF_EXTRACT_H

#include <string>
#include <sstream>
#include <stdexcept>
#include <gdal_priv.h>
#include <image/image.h>

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

  bool hdf_extract();
  bool hdf_query(const char *filename, double& no_data_value, bool& no_data_value_flag, Image& nullImage);
  void hdf_process(const char *filename, const int& band);

} // CommonTilton

#endif // HDF_EXTRACT_H

