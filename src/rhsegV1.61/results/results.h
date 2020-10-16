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
   >>>>                 November 18, 2003 - Added RegionObject class.
   >>>>                 August 9, 2005 - Changed the max_merge_thresh member variable to conv_crit_value
   >>>>                 November 1, 2007 - Changed the conv_crit_value member variable to merge_threshold
   >>>>                 November 9, 2007 - Combined region feature output into region_object list file
   >>>>                 May 12, 2008 - Modified to work with new Params object class
   >>>>                 June 17, 2009 - Modified to work with the hsegreader and hsegviewer programs.
   >>>>                 February 24, 2011 - Added support for FusedClass and FusedObject object classes
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */
//  CONSTRUCTOR, COPY CONSTRUCTOR and DESTRUCTOR for the Results class:
//    Results(const unsigned int& nb_classes)
//      Postcondition:  The Results data member arrays have been allocated, and the other member parameters initialized.
//
//    Results(const Results& source)
//      Postcondition:  A copy of the Results data class variable has been allocated, and values set to match the source.
//
//    ~Results()
//      Postcondition:  All allocated member arrays are deleted.
//
//  MODIFICATION MEMBER FUNCTIONS for the Results class;
//    void resize(const unsigned int& nb_objects)
//      Postcondition:  The data member arrays have been resized to nb_objects.
//
//  CONSTANT MEMBER FUNCTIONS for the Results class;
//    void open_output()
//      Postcondition:  The associated output files are opened.
//
//    void write(const short unsigned int nslevels)
//      Postcondition:  The public member data is written to the associated output files.
//
//    void close_output()
//      Postcondition:  The associated output files are closed.
//
//  VALUE SEMANTICS for the Results class:
//      Assignments and the copy constructor may be used with Results class objects.

#ifndef RESULTS_H
#define RESULTS_H

#include <defines.h>
#include <fstream>
#include <vector>

using namespace std;

namespace HSEGTilton
{

    class RegionClass;
    class RegionObject;

    class Results
    {
      public:
          //  CONSTRUCTORS and DESTRUCTOR
          Results();
          Results(const Results& source);
          ~Results();
          //  MODIFICATION MEMBER FUNCTIONS
          void operator =(const Results& source);
          void set_buffer_sizes(const int& nb_bands, const int& nb_classes, const int& nb_objects);
          void read(const int& hlevel, int& nb_classes, int& nb_objects, const int& nb_levels, 
                    vector<unsigned int>& int_buffer_size,
                    vector<RegionClass>& region_classes, vector<RegionObject>& region_objects);
          //  CONSTANT MEMBER FUNCTIONS
          void open_output(const string& region_classes_file, const string& region_objects_file);
          void write(const int& nslevels, const int& nb_classes, const int& nb_objects,
                     vector<RegionClass>& region_classes, vector<RegionObject>& region_objects);
          int  get_int_buffer_index() const { return int_buffer_index; }
          void close_output();
          void open_input(const string& region_classes_file, const string& region_objects_file);
          void close_input();
      private:
         //  PRIVATE DATA
          int      nbands;            // Number of spectral bands in input image data
          int      orig_nb_classes;   // Number of region classes at onset of writing hierarchical segmentation results
          int      orig_nb_objects;   // Number of region objects at onset of writing hierarchical segmentation results
          int      int_buffer_size, double_buffer_size, object_int_buffer_size, object_double_buffer_size;
          int      *int_buffer, max_int_buffer_size;
          double   *double_buffer;
          int      int_buffer_index, double_buffer_index, object_int_buffer_index;
          ofstream region_classes_ofs;  // File stream variables for region classes output file
          ofstream region_objects_ofs;  // File stream variables for region objects output file
          ifstream region_classes_ifs;  // File stream variable for the region classes input file
          ifstream region_objects_ifs;  // File stream variable for the region objects input file
  } ;

} // HSEGTilton

#endif /* RESULTS_H */
