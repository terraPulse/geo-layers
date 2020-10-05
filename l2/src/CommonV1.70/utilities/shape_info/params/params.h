#ifndef PARAMS_H
#define PARAMS_H

#include <string>

using namespace std;

namespace CommonTilton
{

  class Params 
  {
    public:
    // Constructor and Destructor
      Params(const string& value);
      virtual ~Params();

    // Member functions
      void print_version();
      bool read(const char *param_file);
      void print();

    // Member variables (public)
      string version;              /* -- PROGRAM PARAMETER --*/

    /*-- Input shape file (required) --*/
      string   shape_in_file;            /*-- INPUT SHAPE FILE NAME --*/

    protected:

    private:
  };

  string process_line(const string& line, const bool& list_flag);

} // CommonTilton

#endif /* PARAMS_H */
