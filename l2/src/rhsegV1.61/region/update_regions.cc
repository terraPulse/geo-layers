/*-----------------------------------------------------------
|
|  Routine Name: update_regions
|
|       Purpose: Updates best_region and best_region_dissim values
|                for selected regions.
|
|         Input: update_regions_set  (Set of regions to be updated)
|                added_regions_set   (Set of regions added to region_heap since last call to update_regions)
|                removed_regions_set (Set of regions removed from region_heap since last call to update_regions)
|                region_heap         (Active regions organized in a heap data structure based on best_region_dissim)
|                region_heap_size    (Size of the region_heap)
|                region_classes      (Class which holds region related information)
|                
|        Output:
|
|       Returns: (void)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: September 13, 2002.
| Modifications: June 9, 2003 - Provided a version for a set container class.
|                November 30, 2004 - Improved update sorting efficiency
|                January 4, 2004 - Eliminated use of region_sorted_list
|                May 12, 2008 - Revised to work with globally defined Params and oParams class objects
|		 January 20, 2010 - Added the added_region_set
|
------------------------------------------------------------*/

#include "region_class.h"
#include <params/params.h>

extern HSEGTilton::Params params;

namespace HSEGTilton
{
 void update_regions(set<RegionClass *>& update_regions_set, set<RegionClass *>& added_regions_set, 
                     set<RegionClass *>& removed_regions_set,
                     vector<RegionClass *>& region_heap, unsigned int& region_heap_size)
 {
   bool found_flag;
   unsigned int heap_index;
   float region_dissim;
   RegionClass *update_regions_ptr;
   set<RegionClass *> update_regions_set2;
   set<unsigned int>::iterator best_region_label_iter;
   set<RegionClass *>::iterator update_regions_iter;
#ifdef DEBUG
   if (params.debug > 2)
     params.log_fs << "Calling update_regions..." << endl;
   if (params.debug > 2)
   {
     params.log_fs << "Entering update_regions, region heap is:  " << endl;
     for (heap_index = 0; heap_index < region_heap_size; ++heap_index)
       region_heap[heap_index]->print();
   }
   if (params.debug > 2)
   {
     params.log_fs << endl << endl << "Entering update_regions, update_regions_set is:  " << endl;
     update_regions_iter = update_regions_set.begin();
     while (update_regions_iter != update_regions_set.end())
     {
       update_regions_ptr = *update_regions_iter;
//       update_regions_ptr->print();
       params.log_fs << update_regions_ptr->get_label() << " ";
       ++update_regions_iter;
     }
     params.log_fs << endl << endl << "Entering update_regions, added_regions_set is:  " << endl;
     update_regions_iter = added_regions_set.begin();
     while (update_regions_iter != added_regions_set.end())
     {
       update_regions_ptr = *update_regions_iter;
//       update_regions_ptr->print();
       params.log_fs << update_regions_ptr->get_label() << " ";
       ++update_regions_iter;
     }
     params.log_fs << endl << endl << "Entering update_regions, removed_regions_set is:  " << endl;
     update_regions_iter = removed_regions_set.begin();
     while (update_regions_iter != removed_regions_set.end())
     {
       update_regions_ptr = *update_regions_iter;
//       update_regions_ptr->print();
       params.log_fs << update_regions_ptr->get_label() << " ";
       ++update_regions_iter;
     }
   }
#endif
// Find regions with a best_region_label in the update_regions_set or removed_regions_set
// and place in update_regions_set2.  Also find regions whose best_region_dissim
// is improved for a region in the update_regions_set or added_regions_set, and place them in
// the update_regions_set2.
   for (heap_index = 0; heap_index < region_heap_size; ++heap_index)
     if (region_heap[heap_index]->get_active_flag())
     {
       found_flag = false;
       best_region_label_iter = region_heap[heap_index]->best_region_label.begin();
       while ((!found_flag) && (best_region_label_iter != region_heap[heap_index]->best_region_label.end()))
       {
         update_regions_iter = update_regions_set.begin();
         while ((!found_flag) && (update_regions_iter != update_regions_set.end()))
         {
           update_regions_ptr = *update_regions_iter;
           if ((*best_region_label_iter) == update_regions_ptr->label)
             found_flag = true;
           ++update_regions_iter;
         }
         update_regions_iter = removed_regions_set.begin();
         while ((!found_flag) && (update_regions_iter != removed_regions_set.end()))
         {
           update_regions_ptr = *update_regions_iter;
           if ((*best_region_label_iter) == update_regions_ptr->label)
             found_flag = true;
           ++update_regions_iter;
         }
         ++best_region_label_iter;
       }
       if (!found_flag)
       {
         update_regions_iter = update_regions_set.begin();
         while ((!found_flag) && (update_regions_iter != update_regions_set.end()))
         {
           update_regions_ptr = *update_regions_iter;
           if (update_regions_ptr->label != region_heap[heap_index]->label)
           {
             region_dissim = (float) calc_region_dissim(update_regions_ptr,region_heap[heap_index],false);
             if (region_heap[heap_index]->best_region_dissim >= region_dissim)
               found_flag = true;
           }
           ++update_regions_iter;
         }
       }
       if (!found_flag)
       {
         update_regions_iter = added_regions_set.begin();
         while ((!found_flag) && (update_regions_iter != added_regions_set.end()))
         {
           update_regions_ptr = *update_regions_iter;
           if (update_regions_ptr->label != region_heap[heap_index]->label)
           {
             region_dissim = (float) calc_region_dissim(update_regions_ptr,region_heap[heap_index],false);
             if (region_heap[heap_index]->best_region_dissim >= region_dissim)
               found_flag = true;
           }
           ++update_regions_iter;
         }
       }
       if (found_flag)
       {
         if (update_regions_set.find(region_heap[heap_index]) == update_regions_set.end())
           update_regions_set2.insert(region_heap[heap_index]);
       }
     } // if (region_heap[heap_index]->active_flag)
#ifdef DEBUG
   if (params.debug > 2)
   {
     params.log_fs << "Before adding added_regions_set to the region heap:  " << endl;
     for (heap_index = 0; heap_index < region_heap_size; ++heap_index)
       region_heap[heap_index]->print();
   }
#endif
  // Initialize best_region label and dissim for regions in the added_regions_set
  // and insert them into the region_heap. Also add each entry in the added_regions_set
  // to the update_regions_set2!
   update_regions_iter = added_regions_set.begin();
   while (update_regions_iter != added_regions_set.end())
   {
     update_regions_ptr = *update_regions_iter;
     update_regions_ptr->clear_best_region();
     for (heap_index = 0; heap_index < region_heap_size; ++heap_index)
       if ((region_heap[heap_index]->get_active_flag()) && (update_regions_ptr != region_heap[heap_index]))
         update_regions_ptr->update_best_region(region_heap[heap_index]);
     insert_into_region_heap(update_regions_ptr,region_heap,region_heap_size);
     update_regions_set2.insert(update_regions_ptr);
     ++update_regions_iter;
   }
#ifdef DEBUG
   if (params.debug > 2)
   {
     params.log_fs << "After adding added_regions_set to the region heap:  " << endl;
     update_regions_iter = update_regions_set.begin();
     while (update_regions_iter != update_regions_set.end())
     {
       update_regions_ptr = *update_regions_iter;
//       update_regions_ptr->print();
       params.log_fs << update_regions_ptr->get_label() << " ";
       ++update_regions_iter;
     }
   }
   if (params.debug > 2)
   {
     if (update_regions_set2.empty())
     {
       params.log_fs << endl << "update_regions_set2 is empty" << endl;
     }
     else
     {
       params.log_fs << endl << "Found update_regions_set2:" << endl;
       update_regions_iter = update_regions_set2.begin();
       while (update_regions_iter != update_regions_set2.end())
       {
         update_regions_ptr = *update_regions_iter;
//         update_regions_ptr->print();
         params.log_fs << update_regions_ptr->get_label() << " ";
         ++update_regions_iter;
       }
     }
   }
#endif
  // Update best_region and region_heap for each entry in update_regions_set.
   update_regions_iter = update_regions_set.begin();
   while (update_regions_iter != update_regions_set.end())
   {
     update_regions_ptr = *update_regions_iter;
     update_regions_ptr->clear_best_region();
     for (heap_index = 0; heap_index < region_heap_size; ++heap_index)
       if ((region_heap[heap_index]->get_active_flag()) && (update_regions_ptr != region_heap[heap_index]))
         update_regions_ptr->update_best_region(region_heap[heap_index]);
     update_regions_ptr->update_region_heap(region_heap, region_heap_size);
     ++update_regions_iter;
   }
#ifdef DEBUG
   if (params.debug > 2)
   {
     params.log_fs << "After updating update_regions_set:  " << endl;
     update_regions_iter = update_regions_set.begin();
     while (update_regions_iter != update_regions_set.end())
     {
       update_regions_ptr = *update_regions_iter;
       update_regions_ptr->print();
       ++update_regions_iter;
     }
   }
#endif
  // Update best_region and region_heap for each entry in update_regions_set2.
   update_regions_iter = update_regions_set2.begin();
   while (update_regions_iter != update_regions_set2.end())
   {
     update_regions_ptr = *update_regions_iter;
     update_regions_ptr->clear_best_region();
     for (heap_index = 0; heap_index < region_heap_size; ++heap_index)
       if ((region_heap[heap_index]->get_active_flag()) && (update_regions_ptr != region_heap[heap_index]))
         update_regions_ptr->update_best_region(region_heap[heap_index]);
     update_regions_ptr->update_region_heap(region_heap, region_heap_size);
     ++update_regions_iter;
   }
#ifdef DEBUG
   if ((params.debug > 2) && (!update_regions_set2.empty()))
   {
     params.log_fs << "After updating update_regions_set2:  " << endl;
     update_regions_iter = update_regions_set2.begin();
     while (update_regions_iter != update_regions_set2.end())
     {
       update_regions_ptr = *update_regions_iter;
       update_regions_ptr->print();
       ++update_regions_iter;
     }
   }
   if (params.debug > 2)
   {
     params.log_fs << "After updating heap, region_heap is:  " << endl;
     for (heap_index = 0; heap_index < region_heap_size; ++heap_index)
       region_heap[heap_index]->print();
   }
#endif
   return;
 }

} // namespace HSEGTilton
