#ifndef PARAMS_H
#define PARAMS_H

#include <string>
#include <vector>

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

    /*-- Formatted input mask data file (required) --*/
      string   mask_file;                 /*-- INPUT MASK FILE NAME --*/

    /*-- Input shape file (required) --*/
      string   shape_file;                /*-- INPUT SHAPE FILE NAME --*/

    /*-- Field names used in translation (optional) --*/
      vector<string>   field_names;       /*-- FIELD NAMES --*/
      bool             field_names_flag;  /*-- EXISTANCE FLAG --*/

    /*-- Output ascii format shape file (required) --*/
      string   ascii_shape_file;          /*-- OUTPUT ASCII SHAPE FILE NAME --*/

    protected:

    private:
  };

  string process_line(const string& line, const bool& list_flag);

} // CommonTilton

#endif /* PARAMS_H */
