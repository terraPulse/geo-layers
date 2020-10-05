/*-----------------------------------------------------------
|
|  Routine Name: update_nghbrs
|
|       Purpose: Updates best_nghbr_label and best_nghbr_dissim values
|                for selected regions.
|
|         Input: update_nghbrs_set   (Set of regions to be updated)
|                nghbr_heap          (Active regions organized in a heap data structure based on best_nghbr_dissim)
|                nghbr_heap_size     (Size of the nghbr_heap)
|                region_classes      (Class which holds region related information)
|                
|        Output:
|
|       Returns: (void)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|          Date: September 13, 2002.
| Modifications: February 10, 2003 -  Changed region index to region label
|                June 9, 2003 - Added set container class version
|                November 30, 2004 - Improved update sorting efficiency
|                January 4, 2005 - Eliminated use of nghbr_sorted_list
|                January 8, 2005 - Integrated updating neighbor dissimilarities and neighbor data heap
|                May 12, 2008 - Revised to work with globally defined Params and oParams class objects
|
------------------------------------------------------------*/

#include "region_class.h"
#include <params/params.h>
#ifdef DEBUG
#include <rhseg/hseg.h>
#endif
#include <set>
#include <cmath>

extern HSEGTilton::Params params;

namespace HSEGTilton
{
 void update_nghbrs(set<RegionClass *>& update_nghbrs_set,
                    vector<RegionClass *>& nghbr_heap, unsigned int& nghbr_heap_size,
                    vector<RegionClass>& region_classes)
 {
   bool found_flag, edge_process_flag;
   double nghbr_dissim, edge_dissim, edge_factor;
   RegionClass *nghbr_region_ptr, *update_nghbrs_ptr;
   set<RegionClass *> update_nghbrs_set2;
   set<unsigned int>::iterator nghbrs_label_set_iter, best_nghbr_label_iter;
   set<RegionClass *>::iterator update_nghbrs_iter;
#ifdef DEBUG
   if (params.debug > 3)
     params.log_fs << "Calling update_nghbrs..." << endl;
   if (params.debug > 3)
   {
     params.log_fs << "Entering update_nghbrs, update_nghbrs_set is:  " << endl;
     update_nghbrs_iter = update_nghbrs_set.begin();
     while (update_nghbrs_iter != update_nghbrs_set.end())
     {
       update_nghbrs_ptr = *update_nghbrs_iter;
//       update_nghbrs_ptr->print();
       params.log_fs << update_nghbrs_ptr->get_label() << " ";
       ++update_nghbrs_iter;
     }
     params.log_fs << endl;
   }
#endif
// Find regions with a best_nghbr_label in the update_nghbrs_set
// and place in update_nghbrs_set2 (if not already in update_nghbrs_set).
// Also find regions whose best_nghbr_dissim is improved for a region in
// the update_nghbrs_set, and place them in the update_nghbrs_set2 (if
// not already in update_nghbrs_set).
   update_nghbrs_iter = update_nghbrs_set.begin();
   while (update_nghbrs_iter != update_nghbrs_set.end())
   {
     update_nghbrs_ptr = *update_nghbrs_iter;
     nghbrs_label_set_iter = update_nghbrs_ptr->nghbrs_label_set.begin();
     while (nghbrs_label_set_iter != update_nghbrs_ptr->nghbrs_label_set.end())
     {
       nghbr_region_ptr = &region_classes[(*nghbrs_label_set_iter)-1];
       found_flag = false;
       best_nghbr_label_iter = nghbr_region_ptr->best_nghbr_label.begin();
       while ((!found_flag) && (best_nghbr_label_iter != nghbr_region_ptr->best_nghbr_label.end()))
       {
         if ((*best_nghbr_label_iter) == update_nghbrs_ptr->label)
           found_flag = true;
         ++best_nghbr_label_iter;
       }
       if (!found_flag)
       {
         nghbr_dissim = calc_region_dissim(update_nghbrs_ptr,nghbr_region_ptr,params.merge_accel_flag);

         edge_process_flag = (params.edge_wght > 0.0);
         if ((params.initial_merge_flag) && (!update_nghbrs_ptr->seam_flag) && (!nghbr_region_ptr->seam_flag))
         {
           if ((update_nghbrs_ptr->initial_merge_flag) && (nghbr_region_ptr->initial_merge_flag))
           {
             nghbr_dissim /= params.spclust_wght;
             edge_process_flag = false;
           }
         }
         if (edge_process_flag)
         {
           edge_dissim = update_nghbrs_ptr->max_edge_value;
           if (nghbr_region_ptr->max_edge_value > edge_dissim)
             edge_dissim = nghbr_region_ptr->max_edge_value;
           if (edge_dissim < 0.0)
           {
             if (params.edge_dissim_option == 1)
               edge_dissim = params.max_edge_value;
             else
               edge_dissim = params.min_edge_value;
           }
           edge_factor = (edge_dissim - params.min_edge_value)/(params.max_edge_value - params.min_edge_value);
           edge_factor = pow((double) edge_factor, (double) params.edge_power);
           edge_factor = (1.0 - params.edge_wght) + edge_factor*params.edge_wght;
           if (params.edge_dissim_option == 2)
             edge_factor = (params.spclust_wght + (1.0 - params.spclust_wght)*edge_factor)/params.spclust_wght;
           nghbr_dissim *= edge_factor;
         }
         float float_nghbr_dissim = nghbr_dissim;

         if (nghbr_region_ptr->best_nghbr_dissim >= float_nghbr_dissim)
           found_flag = true;
       }
       if (found_flag)
       {
         if (update_nghbrs_set.find(nghbr_region_ptr) == update_nghbrs_set.end())
           update_nghbrs_set2.insert(nghbr_region_ptr);
       }
       ++nghbrs_label_set_iter;
     }
     ++update_nghbrs_iter;
   }
#ifdef DEBUG
   if (params.debug > 3)
   {
     if (update_nghbrs_set2.empty())
     {
       params.log_fs << "update_nghbrs_set2 is empty" << endl;
     }
     else
     {
       params.log_fs << "Found update_nghbrs_set2: " << endl;
       update_nghbrs_iter = update_nghbrs_set2.begin();
       while (update_nghbrs_iter != update_nghbrs_set2.end())
       {
         update_nghbrs_ptr = *update_nghbrs_iter;
//         update_nghbrs_ptr->print();
         params.log_fs << update_nghbrs_ptr->get_label() << " ";
         ++update_nghbrs_iter;
       }
       params.log_fs << endl;
     }
   }
#endif
#ifdef DEBUG
   unsigned int heap_index;
   if (params.debug > 3)
   {
     params.log_fs << "Before updating heap, nghbr_heap is:  " << endl;
     for (heap_index = 0; heap_index < nghbr_heap_size; ++heap_index)
       nghbr_heap[heap_index]->print();
   }
#endif
  // Update best_nghbr and nghbr_heap for each entry in update_nghbrs_set.
   update_nghbrs_iter = update_nghbrs_set.begin();
   while (update_nghbrs_iter != update_nghbrs_set.end())
   {
     update_nghbrs_ptr = *update_nghbrs_iter;
     update_nghbrs_ptr->clear_best_nghbr();
     nghbrs_label_set_iter = update_nghbrs_ptr->nghbrs_label_set.begin();
     while (nghbrs_label_set_iter != update_nghbrs_ptr->nghbrs_label_set.end())
     {
       nghbr_region_ptr = &region_classes[(*nghbrs_label_set_iter)-1];
       update_nghbrs_ptr->update_best_nghbr(nghbr_region_ptr);
       ++nghbrs_label_set_iter;
     }
     update_nghbrs_ptr->update_nghbr_heap(nghbr_heap, nghbr_heap_size);
     ++update_nghbrs_iter;
   }
#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << "After updating update_nghbrs_set:  " << endl;
     update_nghbrs_iter = update_nghbrs_set.begin();
     while (update_nghbrs_iter != update_nghbrs_set.end())
     {
       update_nghbrs_ptr = *update_nghbrs_iter;
       update_nghbrs_ptr->print();
       ++update_nghbrs_iter;
     }
   }
#endif
  // Update best_nghbr and nghbr_heap for each entry in update_nghbrs_set2.
   update_nghbrs_iter = update_nghbrs_set2.begin();
   while (update_nghbrs_iter != update_nghbrs_set2.end())
   {
     update_nghbrs_ptr = *update_nghbrs_iter;
     update_nghbrs_ptr->clear_best_nghbr();
     nghbrs_label_set_iter = update_nghbrs_ptr->nghbrs_label_set.begin();
     while (nghbrs_label_set_iter != update_nghbrs_ptr->nghbrs_label_set.end())
     {
       nghbr_region_ptr = &region_classes[(*nghbrs_label_set_iter)-1];
       update_nghbrs_ptr->update_best_nghbr(nghbr_region_ptr);
       ++nghbrs_label_set_iter;
     }
     update_nghbrs_ptr->update_nghbr_heap(nghbr_heap, nghbr_heap_size);
     ++update_nghbrs_iter;
   }
#ifdef DEBUG
   if ((params.debug > 3) && (!update_nghbrs_set2.empty()))
   {
     params.log_fs << "After updating update_nghbrs_set2:  " << endl;
     update_nghbrs_iter = update_nghbrs_set2.begin();
     while (update_nghbrs_iter != update_nghbrs_set2.end())
     {
       update_nghbrs_ptr = *update_nghbrs_iter;
       update_nghbrs_ptr->print();
       ++update_nghbrs_iter;
     }
   }

   if (params.debug > 3)
   {
     params.log_fs << "After updating heap, nghbr_heap is:  " << endl;
     for (heap_index = 0; heap_index < nghbr_heap_size; ++heap_index)
       nghbr_heap[heap_index]->print(region_classes);
   }
   if (check_nghbr_heap(nghbr_heap,nghbr_heap_size) && (params.debug > 3))
   {
   // Should never happen
     params.log_fs << endl << "After correcting heap in check_nghbr_heap, nghbr_heap:  " << endl;
     for (heap_index = 0; heap_index < nghbr_heap_size; ++heap_index)
       nghbr_heap[heap_index]->print(region_classes);
   }
#endif
   return;
 }

} // namespace HSEGTilton
