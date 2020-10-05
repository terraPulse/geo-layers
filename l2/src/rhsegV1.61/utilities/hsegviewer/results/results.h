/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>>       Purpose:  Provides the Results class, which is a class dealing
   >>>>                 with output results data
   >>>>
   >>>>    Written By:  James C. Tilton, MC 606.3 NASA's GSFC, Greenbelt, MD 20771
   >>>>                 e-mail:  James.C.Tilton@nasa.gov
   >>>>          Date:  November 21, 2002
   >>>>
   >>>> Modifications:  August 21, 2003 - Made byteswapping depend on a user controlled parameter
   >>>>                 October 17, 2003 - Added boundary_npix and boundary_npix_fs member variables
   >>>>                 November 18, 2003 - Added ConnRegion class.
   >>>>                 August 9, 2005 - Changed the max_merge_thresh member variable to conv_crit_value
   >>>>                 November 1, 2007 - Changed the conv_crit_value member variable to merge_threshold
   >>>>                 November 9, 2007 - Combined region feature output into region_classes list file
   >>>>                 February 7, 2008 - Modified to work with the hsegviewer program.
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */
//  CONSTRUCTOR, COPY CONSTRUCTOR and DESTRUCTOR for the Results class:
//    Results( )
//      Postcondition:  The Results data member arrays have been allocated, and the other member parameters initialized.
//
//    Results(const Results& source)
//      Postcondition:  A copy of the Region data class variable has been allocated, and values set to match the source.
//
//    ~Results( )
//      Postcondition:  All allocated member arrays are deleted.
//
//  MODIFICATION MEMBER FUNCTIONS for the Results class;
//
//  CONSTANT MEMBER FUNCTIONS for the Results class;
//
//  VALUE SEMANTICS for the Results class:
//      Assignments and the copy constructor may be used with Results class objects.

#ifndef RESULTS_H
#define RESULTS_H

#include "../stats/stats.h"
#include <fstream>

using namespace std;

namespace HSEGTilton
{

    class Results
    {
      public:
          //  CONSTRUCTORS and DESTRUCTOR
          Results( );
          Results(const Results& source);
          ~Results( );
          //  MODIFICATION MEMBER FUNCTIONS
          void operator =(const Results& source);
          void set_int_buffer_size(const unsigned int& hlevel);
          void read(const unsigned int& hlevel, unsigned int& nb_classes, unsigned int& nb_objects,
                    Stats& statsData);
         //  CONSTANT MEMBER FUNCTIONS
          void open_input(const string& region_classes_file, const string& region_objects_file);
          void close_input();
      private:
         //  PRIVATE DATA
          short unsigned int nbands;      // Number of spectral bands in input image data
          unsigned int orig_nb_classes;         // Number of region classes at onset of writing hierarchical segmentation results
          unsigned int orig_nb_objects;   // Number of region objects at onset of writing hierarchical segmentation results
          double  *scale, *offset;   // Scale and offset values used in region mean scaling and normalization
          unsigned int int_buffer_size, double_buffer_size, object_int_buffer_size, object_double_buffer_size;
          unsigned int *int_buffer, max_int_buffer_size;
          double    *double_buffer;
          unsigned int int_buffer_index, double_buffer_index, object_int_buffer_index;
          ifstream region_classes_fs;     // File stream variables for region classes output file
          ifstream region_objects_fs;     // File stream variables for region objects output file
  } ;

} // HSEGTilton

#endif /* RESULTS_H */
