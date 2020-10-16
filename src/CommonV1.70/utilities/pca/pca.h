#ifndef PCA_H
#define PCA_H

#include <string>
#include <sstream>
#include <stdexcept>

using namespace std;

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

bool pca();

#endif // PCA_H

