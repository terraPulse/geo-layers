/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>>          File:  index.cc
   >>>>
   >>>>          See index.h for documentation
   >>>>
   >>>>          Date:  September 13, 2002
   >>>> Modifications:  February 7, 2003 - Changed region_index to region_class_label
   >>>>                 October 6, 2003 - Added nghbr_region_label_set member variable
   >>>>                 December 22, 2004 - Changed region label from short unsigned int to unsigned int
   >>>>                 May 12, 2008 - Modified to work with new Params object class
   >>>>                 October 4, 2010 - Took pixel_data.mask into account
   >>>>                 August 5, 2013 - Added edge_value and edge_location member variables
   >>>>                 January 8, 2014 - Removed the edge_location member variable
   >>>>                 January 9, 2014 - Changed default value of edge_value to -FLT_MAX (signifying invalid value)
   >>>>                 January 18, 2014 - Added edge_mask member variable.
   >>>>                 January 18, 2014 - Renamed section to pixel_section.
   >>>>                 January 18, 2014 - Added explicit constructors and destructor.
   >>>>                 January 18, 2014 - Removed nghbr_region_label_set.
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#include "index.h"
#include <params/params.h>
#include <spatial/spatial.h>
#include <region/region_class.h>

extern HSEGTilton::Params params;

namespace HSEGTilton
{
 Index::Index( )
 {
   pixel_index = 0; 
   pixel_section = 0;
   edge_mask = false;
   edge_value = -FLT_MAX;
   region_class_label = region_object_label = 0;
   boundary_map = 0;

   return;
 }

 Index::Index(const Index& source)
 {
   pixel_index = source.pixel_index;
   pixel_section = source.pixel_section;
   edge_mask = source.edge_mask;
   edge_value = source.edge_value;
   region_class_label = source.region_class_label;
   region_object_label = source.region_object_label;
   boundary_map = source.boundary_map;

   return;
 }

 Index::~Index( )
 {
   return;
 }

 void Index::operator =(const Index& source)
 {
   if (this == &source)
     return;

 // Member variables
   pixel_index = source.pixel_index;
   pixel_section = source.pixel_section;
   edge_mask = source.edge_mask;
   edge_value = source.edge_value;
   region_class_label = source.region_class_label;
   region_object_label = source.region_object_label;
   boundary_map = source.boundary_map;

   return;
 }

 void Index::clear( )
 {
   pixel_index = 0;
   pixel_section = 0;
   edge_mask = false;
   edge_value = -FLT_MAX;
   region_class_label = 0;
   region_object_label = 0;
   boundary_map = 0;

   return;
 }

 void Index::set_data(const short unsigned int& section_val,
                      vector<Pixel>& pixel_data, const unsigned int& index)
 {
     pixel_index = index;
     pixel_section = section_val;
     edge_mask = pixel_data[index].get_edge_mask();
     if (edge_mask)
       edge_value = pixel_data[index].get_edge_value();
     else
       edge_value = -FLT_MAX;
     if (pixel_data[index].mask) // Added this check October 4, 2010
       region_class_label = pixel_data[index].get_region_label();
     else
       region_class_label = 0;

     return;
 }

 void Index::set_data(const short unsigned int& section_val,
                      Spatial& spatial_data, const unsigned int& index)
 {
     pixel_index = index;
     pixel_section = section_val;
     region_class_label = (unsigned int) spatial_data.region_class_label_map[index];
     if (params.region_nb_objects_flag)
       region_object_label = spatial_data.region_object_label_map[index];
     boundary_map = spatial_data.boundary_map[index];

     return;
 }

 void Index::print()
 {
    params.log_fs << endl << "This element ";
    params.log_fs << endl << "points to pixel data at index = " << pixel_index << "." << endl;
    params.log_fs << ", located in processing section " << pixel_section << "." << endl;
    if ((params.edge_image_flag) && (edge_mask))
    {
      params.log_fs << "Edge value = " << edge_value << endl;
    }
    if (region_class_label != 0)
      params.log_fs << "This elements's associated region label is " << region_class_label << endl;
    if (region_object_label != 0)
      params.log_fs << "This elements's associated connected region label is " << region_object_label << endl;
    if (boundary_map > 0)
      params.log_fs << "This element is on the boundary of the region up to hierarchical level" << boundary_map << "." << endl;
    else
      params.log_fs << "This element is in the interior of the region." << endl;
 }
#ifdef PARALLEL
#include <parallel/index.cc>
#endif
} // namespace HSEGTilton
