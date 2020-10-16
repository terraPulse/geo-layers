/*-----------------------------------------------------------
|
|  Routine Name: (Various heap related routines)
|
|       Purpose: Routines for making and managing data heaps.
|
|          Note: These are all standard heap routines with the exception of the following elaboration:
|                The RegionClass member variables nghbr_heap_index and region_heap_index keep track
|                of a region's index location in the nghbr and region heaps, respectively.
|                This provides an efficient way to locate a particular region in the heaps.
|
|         Input: nghbr_heap or region_heap  (Set of data for heap)
|                nghbr_heap_size or region_heap_size   (Size of heap)
|
|        Output:
|
|       Returns: (void)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|          Date: January 7, 2005.
| Modifications: February 27, 3005: Changed pop_nghbr_heap and pop_region_heap to
|                remove_from_nghbr_heap and remove_from_region_heap.
|                May 12, 2008 - Revised to work with globally defined Params and oParams class objects
|                January 11, 2010 - Added insert_into_region_heap function
|
------------------------------------------------------------*/

#include "hseg.h"
#include <params/params.h>
#include <region/region_class.h>
#include <algorithm>

extern HSEGTilton::Params params;

namespace HSEGTilton
{

 unsigned int heap_div2(const unsigned int& num)
 {
   unsigned int result;

   result = (num-1)/2;

   return result;
 }

 bool nghbr_more_than(const RegionClass *region_class1, const RegionClass *region_class2)
 {
   if ((region_class1->best_nghbr_dissim == region_class2->best_nghbr_dissim) &&
       (region_class1->nghbr_heap_npix == region_class2->nghbr_heap_npix))
     return (region_class1->label < region_class2->label);
   else if (region_class1->best_nghbr_dissim == region_class2->best_nghbr_dissim)
     return (region_class1->nghbr_heap_npix > region_class2->nghbr_heap_npix);
   else
     return (region_class1->best_nghbr_dissim > region_class2->best_nghbr_dissim);
 }

 void make_nghbr_heap(vector<RegionClass *>& nghbr_heap,
                      const unsigned int& nghbr_heap_size)
 {
   unsigned int heap_index;

   for (heap_index = 0; heap_index < nghbr_heap_size; ++heap_index)
     nghbr_heap[heap_index]->nghbr_heap_npix = nghbr_heap[heap_index]->npix;
   make_heap(&nghbr_heap[0],&nghbr_heap[nghbr_heap_size],NghbrMoreThan( ));
   for (heap_index = 0; heap_index < nghbr_heap_size; ++heap_index)
     nghbr_heap[heap_index]->nghbr_heap_index = heap_index;
#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << endl << "After making heap, nghbr_heap:  " << endl;
     for (heap_index = 0; heap_index < nghbr_heap_size; ++heap_index)
       nghbr_heap[heap_index]->print();
   }
#endif
 }

 void remove_from_nghbr_heap(const unsigned int& remove_heap_index,
                             vector<RegionClass *>& nghbr_heap, unsigned int& nghbr_heap_size)
 {
   unsigned int heap_index;
//   RegionClass *region_class_ptr;
#ifdef DEGUG
   if (params.debug > 3)
   {
     params.log_fs << endl << "Before removing region " << nghbr_heap[heap_index]->label << " from heap, nghbr_heap:  " << endl;
     for (heap_index = 0; heap_index < nghbr_heap_size; ++heap_index)
       nghbr_heap[heap_index]->print();
   }
#endif
//   region_class_ptr = nghbr_heap[remove_heap_index];
   nghbr_heap[remove_heap_index]->nghbr_heap_index = UINT_MAX;
   nghbr_heap[remove_heap_index] = nghbr_heap[--nghbr_heap_size];
   nghbr_heap[remove_heap_index]->nghbr_heap_index = remove_heap_index; // differs from standard heap algorithm
   nghbr_heap[nghbr_heap_size] = NULL; // differs from standard heap algorithm
//   nghbr_heap[nghbr_heap_size] = region_class_ptr;
//   nghbr_heap[nghbr_heap_size]->nghbr_heap_index = nghbr_heap_size;
   heap_index = remove_heap_index;
   nghbr_downheap(heap_index, nghbr_heap, nghbr_heap_size);
   heap_index = remove_heap_index;
   nghbr_upheap(heap_index, nghbr_heap, nghbr_heap_size);
#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << endl << "After removing region " << nghbr_heap[nghbr_heap_size]->label << " from heap:" << endl;
     for (heap_index = 0; heap_index < nghbr_heap_size; ++heap_index)
       nghbr_heap[heap_index]->print();
   }
#endif
   return;
 }

 bool nghbr_upheap(unsigned int& heap_index, vector<RegionClass *>& nghbr_heap,
                   const unsigned int& nghbr_heap_size)
 {
   bool change_flag = false;
   RegionClass *region_class_ptr;

   if ((heap_index == 0) || (heap_index >= nghbr_heap_size))
     return change_flag;

   region_class_ptr = nghbr_heap[heap_index];
   while (nghbr_more_than(nghbr_heap[heap_div2(heap_index)],region_class_ptr))
   {
     change_flag = true;
     nghbr_heap[heap_index] = nghbr_heap[heap_div2(heap_index)];
     nghbr_heap[heap_index]->nghbr_heap_index = heap_index;
     heap_index = heap_div2(heap_index);
     if (heap_index == 0)
       break;
   }
   nghbr_heap[heap_index] = region_class_ptr;
   nghbr_heap[heap_index]->nghbr_heap_index = heap_index;

   return change_flag;
 }

 bool nghbr_downheap(unsigned int& heap_index, vector<RegionClass *>& nghbr_heap,
                     const unsigned int& nghbr_heap_size)
 {
   bool change_flag = false;
   unsigned int j;
   RegionClass *region_class_ptr;

   if ((nghbr_heap_size <= 1) || (heap_index >= nghbr_heap_size))
     return change_flag;

   region_class_ptr = nghbr_heap[heap_index];
   while (heap_index <= heap_div2(nghbr_heap_size-1))
   {
     j = 2*(heap_index + 1);
     if (j >= nghbr_heap_size)
       j -= 1;
     else if (nghbr_more_than(nghbr_heap[j],nghbr_heap[j-1]))
       j -= 1;
     if (nghbr_more_than(nghbr_heap[j],region_class_ptr))
       break;
     change_flag = true;
     nghbr_heap[heap_index] = nghbr_heap[j];
     nghbr_heap[heap_index]->nghbr_heap_index = heap_index;
     heap_index = j;
   }
   nghbr_heap[heap_index] = region_class_ptr;
   nghbr_heap[heap_index]->nghbr_heap_index = heap_index;

   return change_flag;
 }

 bool check_nghbr_heap(vector<RegionClass *>& nghbr_heap,
                       const unsigned int& nghbr_heap_size)
 {
 // Check for valid heap condition throughout the heap:
   bool error_flag, return_flag = false;
   unsigned int j, check_heap_index, heap_index;

   if (params.debug > 2)
   {
     params.log_fs << endl << "Before checking heap, nghbr_heap:  " << endl;
     for (heap_index = 0; heap_index < nghbr_heap_size; ++heap_index)
       nghbr_heap[heap_index]->print();
   }

   if (nghbr_heap_size > 1)
     for (check_heap_index = 0; check_heap_index <= heap_div2(nghbr_heap_size-1); ++check_heap_index)
     {
       error_flag = false;
       j = 2*(check_heap_index + 1);
       if (j < nghbr_heap_size)
         if (nghbr_more_than(nghbr_heap[check_heap_index],nghbr_heap[j]))
           error_flag = true;
       if (nghbr_more_than(nghbr_heap[check_heap_index],nghbr_heap[j-1]))
         error_flag = true;
       if (error_flag)
       {
         return_flag = true;
         heap_index = check_heap_index;
         if (params.debug > 2)
         {
           params.log_fs << "WARNING:  Invalid heap detected for region " << nghbr_heap[heap_index]->get_label( );
           if (j < nghbr_heap_size)
             params.log_fs << " with sons " << nghbr_heap[j-1]->get_label( ) << " and " << nghbr_heap[j]->get_label( ) << endl;
           else
             params.log_fs << " with son " << nghbr_heap[j-1]->get_label( ) << endl;
           params.log_fs << "   nghbr_downheap and nghbr_upheap called on heap_index " << heap_index << " to correct the problem." << endl;
         }
         nghbr_downheap(heap_index,nghbr_heap,nghbr_heap_size);
         heap_index = check_heap_index;
         nghbr_upheap(heap_index,nghbr_heap,nghbr_heap_size);
         --check_heap_index; // Forces recheck of this heap location.
       }
     }

   if (params.debug > 2)
   {
     params.log_fs << endl << "After checking heap, nghbr_heap:  " << endl;
     for (heap_index = 0; heap_index < nghbr_heap_size; ++heap_index)
       nghbr_heap[heap_index]->print();
   }

   return return_flag;
 }

 bool region_more_than(const RegionClass *region_class1, const RegionClass *region_class2)
 {
   if ((region_class1->best_region_dissim == region_class2->best_region_dissim) &&
       (region_class1->region_heap_npix == region_class2->region_heap_npix))
     return (region_class1->label < region_class2->label);
   else if (region_class1->best_region_dissim == region_class2->best_region_dissim)
     return (region_class1->region_heap_npix > region_class2->region_heap_npix);
   else
     return (region_class1->best_region_dissim > region_class2->best_region_dissim);
 }

 void make_region_heap(vector<RegionClass *>& region_heap,
                       const unsigned int& region_heap_size)
 {
   unsigned int heap_index;

   for (heap_index = 0; heap_index < region_heap_size; ++heap_index)
     region_heap[heap_index]->region_heap_npix = region_heap[heap_index]->npix;
   make_heap(&region_heap[0],&region_heap[region_heap_size],RegionMoreThan( ));
   for (heap_index = 0; heap_index < region_heap_size; ++heap_index)
     region_heap[heap_index]->region_heap_index = heap_index;
#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << endl << "After making heap, region_heap:  " << endl;
     for (heap_index = 0; heap_index < region_heap_size; ++heap_index)
       region_heap[heap_index]->print();
   }
#endif
 }

 void remove_from_region_heap(const unsigned int& remove_heap_index,
                              vector<RegionClass *>& region_heap, unsigned int& region_heap_size)
 {
   unsigned int heap_index;
//   RegionClass *region_class_ptr;
#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << endl << "Before removing region " << region_heap[heap_index]->label << " from heap, region_heap:  " << endl;
     for (heap_index = 0; heap_index < region_heap_size; ++heap_index)
       region_heap[heap_index]->print();
   }
#endif
//   region_class_ptr = region_heap[remove_heap_index];
   region_heap[remove_heap_index]->region_heap_index = UINT_MAX;
   region_heap[remove_heap_index] = region_heap[--region_heap_size];
   region_heap[remove_heap_index]->region_heap_index = remove_heap_index; // differs from standard heap algorithm
   region_heap[region_heap_size] = NULL; // differs from standard heap algorithm
//   region_heap[region_heap_size] = region_class_ptr;
//   region_heap[region_heap_size]->region_heap_index = region_heap_size;
   heap_index = remove_heap_index;
   region_downheap(heap_index, region_heap, region_heap_size);
   heap_index = remove_heap_index;
   region_upheap(heap_index, region_heap, region_heap_size);
#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << endl << "After removing region " << region_heap[region_heap_size]->label << " from heap, region_heap:  " << endl;
     for (heap_index = 0; heap_index < region_heap_size; ++heap_index)
       region_heap[heap_index]->print();
   }
#endif
   return;
 }

 void insert_into_region_heap(RegionClass *insert_region,
                              vector<RegionClass *>& region_heap, unsigned int& region_heap_size)
 {
   unsigned int heap_index;
#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << endl << "Before inserting region " << insert_region->label << " into heap, region_heap:  " << endl;
     for (heap_index = 0; heap_index < region_heap_size; ++heap_index)
       region_heap[heap_index]->print();
   }
#endif
   for (heap_index = 0; heap_index < region_heap_size; ++heap_index)
     if (region_heap[heap_index] == insert_region)
     {
       params.log_fs << "WARNING(insert_into_region_heap): region " << insert_region->label << " already in region_heap!" << endl;
       return;
     }
   if ((region_heap_size > 0) && (insert_region->best_region_label.empty()))
   {
     params.log_fs << "ERROR(insert_into_region_heap): region " << insert_region->label << " has an empty best_region_label set!" << endl;
     exit(false);
   }
   heap_index = region_heap.size() - 1;
   if (heap_index == region_heap_size)
   {
     region_heap.push_back(NULL);
     region_heap[region_heap_size] = insert_region;
     insert_region->region_heap_npix = insert_region->npix;
     insert_region->region_heap_index = region_heap_size++;
   }
   else
   {
     region_heap[region_heap_size+1] = NULL;
     region_heap[region_heap_size] = insert_region;
     insert_region->region_heap_npix = insert_region->npix;
     insert_region->region_heap_index = region_heap_size++;
   }
   heap_index = insert_region->region_heap_index;
   region_upheap(heap_index,region_heap,region_heap_size);
#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << endl << "After inserting region " << insert_region->label << " into heap, region_heap:  " << endl;
     for (heap_index = 0; heap_index < region_heap_size; ++heap_index)
       region_heap[heap_index]->print();
   }
   if (check_region_heap(region_heap,region_heap_size))
   {
     params.log_fs << endl << "After call to check_region_heap:" << endl;
     insert_region->print();
   }
#endif
   return;
 }

 bool region_upheap(unsigned int& heap_index, vector<RegionClass *>& region_heap,
                    const unsigned int& region_heap_size)
 {
   bool change_flag = false;
   RegionClass *region_class_ptr;

   if ((heap_index == 0) || (heap_index >= region_heap_size))
     return change_flag;

   region_class_ptr = region_heap[heap_index];
   while (region_more_than(region_heap[heap_div2(heap_index)],region_class_ptr))
   {
     change_flag = true;
     region_heap[heap_index] = region_heap[heap_div2(heap_index)];
     region_heap[heap_index]->region_heap_index = heap_index;
     heap_index = heap_div2(heap_index);
     if (heap_index == 0)
       break;
   }
   region_heap[heap_index] = region_class_ptr;
   region_heap[heap_index]->region_heap_index = heap_index;

   return change_flag;
 }

 bool region_downheap(unsigned int& heap_index, vector<RegionClass *>& region_heap,
                      const unsigned int& region_heap_size)
 {
   bool change_flag = false;
   unsigned int j;
   RegionClass *region_class_ptr;

   if ((region_heap_size <= 1) || (heap_index >= region_heap_size))
     return change_flag;

   region_class_ptr = region_heap[heap_index];
   while (heap_index <= heap_div2(region_heap_size-1))
   {
     j = 2*(heap_index + 1);
     if (j >= region_heap_size)
       j -= 1;
     else if (region_more_than(region_heap[j],region_heap[j-1]))
       j -= 1;
     if (region_more_than(region_heap[j],region_class_ptr))
       break;
     change_flag = true;
     region_heap[heap_index] = region_heap[j];
     region_heap[heap_index]->region_heap_index = heap_index;
     heap_index = j;
   }
   region_heap[heap_index] = region_class_ptr;
   region_heap[heap_index]->region_heap_index = heap_index;

   return change_flag;
 }

 bool check_region_heap(vector<RegionClass *>& region_heap,
                        const unsigned int& region_heap_size)
 {
 // Check for valid heap condition throughout the heap:
   bool error_flag, return_flag = false;
   unsigned int j, check_heap_index, heap_index;

   if (params.debug > 2)
   {
     params.log_fs << endl << "Before checking heap, region_heap:  " << endl;
     for (heap_index = 0; heap_index < region_heap_size; ++heap_index)
       region_heap[heap_index]->print();
   }

   if (region_heap_size > 1)
     for (check_heap_index = 0; check_heap_index <= heap_div2(region_heap_size-1); ++check_heap_index)
     {
       error_flag = false;
       j = 2*(check_heap_index + 1);
       if (j < region_heap_size)
         if (region_more_than(region_heap[check_heap_index],region_heap[j]))
           error_flag = true;
       if (region_more_than(region_heap[check_heap_index],region_heap[j-1]))
         error_flag = true;
       if (error_flag)
       {
         return_flag = true;
         heap_index = check_heap_index;
         if (params.debug > 2)
         {
           params.log_fs << "WARNING:  Invalid heap detected for region " << region_heap[heap_index]->get_label( );
           if (j < region_heap_size)
             params.log_fs << " with sons " << region_heap[j-1]->get_label( ) << " and " << region_heap[j]->get_label( ) << endl;
           else
             params.log_fs << " with son " << region_heap[j-1]->get_label( ) << endl;
           if (j < region_heap_size)
             params.log_fs << " at region_heap_indices " << region_heap[j-1]->get_region_heap_index( ) << " and " << region_heap[j]->get_region_heap_index( ) << endl;
           else
             params.log_fs << " at region_heap_index " << region_heap[j-1]->get_region_heap_index( ) << endl;
           params.log_fs << "   region_downheap and region_upheap called on heap_index " << heap_index << " to correct the problem." << endl;
         }
         region_downheap(heap_index,region_heap,region_heap_size);
         heap_index = check_heap_index;
         region_upheap(heap_index,region_heap,region_heap_size);
         --check_heap_index; // Forces recheck of this heap location.
       }
     }

   if (params.debug > 2)
   {
     params.log_fs << endl << "After checking heap, region_heap:  " << endl;
     for (heap_index = 0; heap_index < region_heap_size; ++heap_index)
       region_heap[heap_index]->print();
   }

   return return_flag;
 }

 void check_region_heap_symmetry(vector<RegionClass>& region_classes,
                                 vector<RegionClass *>& region_heap, const unsigned int& region_heap_size)
 {
   unsigned int heap_index, best_region_index;

   for (heap_index = 0; heap_index < region_heap_size; ++heap_index)
   {
     best_region_index = region_heap[heap_index]->get_best_region_label(region_classes) - 1;
     if ((region_heap[heap_index]->get_best_region_label_set_size() == 1) && 
         (region_classes[best_region_index].get_best_region_label_set_size() == 1) && 
         (region_heap[heap_index]->get_label() != region_classes[best_region_index].get_best_region_label(region_classes)) &&
         (region_heap[heap_index]->get_best_region_dissim() <= region_classes[best_region_index].get_best_region_dissim()))
     {
        params.log_fs << "best_region_label assymmetry detected for regions " << region_heap[heap_index]->get_label();
        params.log_fs << " and " << region_classes[best_region_index].get_label() << endl;
        region_heap[heap_index]->print(region_classes);
        region_classes[best_region_index].print(region_classes);
     }
/*
     else if (region_heap[heap_index]->get_best_region_label_set_size() !=
              region_classes[best_region_index].get_best_region_label_set_size())
     {
        params.log_fs << "best_region_label assymmetry detected for regions " << region_heap[heap_index]->get_label();
        params.log_fs << " and " << region_classes[best_region_index].get_best_region_label(region_classes) << endl;
        region_heap[heap_index]->print(region_classes);
        region_classes[best_region_index].print(region_classes);
     }
*/
   }

   return;
 }
} // namespace HSEGTilton

