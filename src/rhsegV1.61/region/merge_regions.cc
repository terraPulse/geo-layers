/*-----------------------------------------------------------
|
|  Routine Name: merge_regions
|
|       Purpose: Performs hybrid of hierarchical step-wise optimal region growing
|                and spectral clustering.
|
|         Input: converge_nregions  (Number of regions at which to stop merging)
|                
|  Input/Output: nghbr_heap         (Heap of regions based on best_nghbr_dissim)
|                nghbr_heap_size    (Size of nghbr_heap)
|                region_heap        (Heap of regions based on best_region_dissim)
|                region_heap_size   (Size of region_heap)
|                region_classes     (Class which holds region class related information)
|                nregions           (Current number of regions)
|
|        Output: max_threshold      (Maximum merging threshold encountered)
|
|       Returns: bool
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: September 13, 2002
| Modifications: February 10, 2003 - Changed region_index to region_label
|                June 9, 2003 - Changed certain lists to sets for efficiency
|                November 30, 2004 - Improved region update efficiency
|                December 23, 2004 - Changed region label from short unsigned int to unsigned int
|                January 4, 2005 - Changed from using sorted lists to data heap for finding best merge
|                December 23, 2005 - Added code to enforce merging of smaller regions into larger ones
|                February 4, 2008 - Added code to return when at least one of the regions selected for
|                         merging was involved in a previous merge is the current call to this routine.
|                May 12, 2008 - Revised to work with globally defined Params and oParams class objects
|                January 11, 2010 - Implemented new definition of min_npixels parameter.
|                December 8, 2011 - Modified code to update region_heap for region_heap_size > 0 instead of region_heap_size > 1.
|
------------------------------------------------------------*/

#include "region_class.h"
#include <params/params.h>
#include <iostream>
#include <float.h>
#include <vector>

extern HSEGTilton::Params params;

namespace HSEGTilton
{
 bool merge_regions(const bool& last_stage_flag, unsigned int& converge_nregions,
                    vector<RegionClass *>& nghbr_heap, unsigned int& nghbr_heap_size,
                    vector<RegionClass *>& region_heap, unsigned int& region_heap_size,
                    vector<RegionClass>& region_classes, unsigned int& nregions,
                    double& max_threshold)
 {
   bool process_flag;
   unsigned int merge_label, heap_index;
   double threshold, new_max_threshold, nghbr_thresh, spclust_thresh = FLT_MAX;
   RegionClass *best_region, *merge_region;
   set<RegionClass *> update_nghbrs_set;
   set<RegionClass *> update_regions_set;
   set<RegionClass *> added_regions_set;
   set<RegionClass *> removed_regions_set;

   if ((nghbr_heap_size > 0) && (region_heap_size < 2))
   {
     best_region = nghbr_heap[0];
     while (!best_region->get_active_flag()) // Should never happen
     {
       if (params.debug > 0)
         params.log_fs << "WARNING(1): Found inactive region at top of heap!" << endl;
       else
         cout << "WARNING(1): Found inactive region at top of heap!" << endl;
       remove_from_nghbr_heap(0,nghbr_heap,nghbr_heap_size);
       best_region = nghbr_heap[0];
     }
     nghbr_thresh = best_region->get_best_nghbr_dissim();
     new_max_threshold = max_threshold;
     if (nghbr_thresh > new_max_threshold)
       new_max_threshold = nghbr_thresh;
     if (nghbr_thresh < FLT_MAX)
     {
     // Merge all pairs of spatially adjacent (neighboring) regions
     // with dissimilarity less than or equal to the threshold.
       while (((nghbr_thresh <= new_max_threshold) && (nregions > converge_nregions)) || (nghbr_thresh == 0.0))
       {
         threshold = nghbr_thresh;
         merge_label = best_region->get_best_nghbr_label(region_classes);

         if (merge_label != 0)
         {
           merge_region = &region_classes[merge_label-1];
#ifdef DEBUG
           if (best_region->get_npix() > merge_region->get_npix())
           {
             if (params.debug > 2)
             {
               params.log_fs << "WARNING: Found best_region->npix = " << best_region->get_npix() << " > ";
               params.log_fs << "merge region->npix " << merge_region->get_npix() << endl;
               params.log_fs << "(best_region->nghbr_heap_npix = " << best_region->get_nghbr_heap_npix() << " and ";
               params.log_fs << "merge region->nghbr_heap_npix " << merge_region->get_nghbr_heap_npix() << ")" << endl;
               params.log_fs << "(best_region->label = " << best_region->get_label() << " and ";
               params.log_fs << "merge region->label " << merge_region->get_label() << ")" << endl;
               params.log_fs << "Calling merge_region->nghbr_bring_to_top_heap to avoid this." << endl;
             }
             merge_region->nghbr_bring_to_top_heap(nghbr_heap,nghbr_heap_size);
             best_region = nghbr_heap[0];
             merge_label = best_region->get_best_nghbr_label(region_classes);
             merge_region = &region_classes[merge_label-1];
           }
           if ((best_region->get_npix() == merge_region->get_npix()) &&
               (best_region->get_label() < merge_region->get_label()))
           {
             if (params.debug > 2)
             {
               params.log_fs << "WARNING: Found best_region->label = " << best_region->get_label() << " < ";
               params.log_fs << "merge region->label " << merge_region->get_label() << endl;
               params.log_fs << "(best_region->npix = " << best_region->get_npix() << " and ";
               params.log_fs << "merge region->npix " << merge_region->get_npix() << endl;
               params.log_fs << "(best_region->nghbr_heap_npix = " << best_region->get_nghbr_heap_npix() << " and ";
               params.log_fs << "merge region->nghbr_heap_npix " << merge_region->get_nghbr_heap_npix() << ")" << endl;
               params.log_fs << "Calling check_nghbr_heap to correct this." << endl;
             }
             check_nghbr_heap(nghbr_heap,nghbr_heap_size);
             best_region = nghbr_heap[0];
             while (!best_region->get_active_flag())
             {
               if (params.debug > 0)
                 params.log_fs << "WARNING(2): Found inactive region at top of heap!" << endl;
               else
                 cout << "WARNING(2): Found inactive region at top of heap!" << endl;
               remove_from_nghbr_heap(0,nghbr_heap,nghbr_heap_size);
               best_region = nghbr_heap[0];
             }
             if ((params.debug > 2) && (nghbr_thresh != best_region->get_best_nghbr_dissim()))
             {
               params.log_fs << "WARNING: nghbr_thresh = " << nghbr_thresh << " != ";
               params.log_fs << "best_region->best_nghbr_dissim = " << best_region->get_best_nghbr_dissim() << endl;
             }
             merge_label = best_region->get_best_nghbr_label(region_classes);
             merge_region = &region_classes[merge_label-1];
           }
#endif
           if ((last_stage_flag) && (best_region->get_npix() >= params.min_npixels) && (merge_region->get_npix() >= params.min_npixels))
           {
             if ((best_region->get_large_nghbr_merged_flag()) ||
                 (merge_region->get_large_nghbr_merged_flag()))
             {
               if ((!update_regions_set.empty()) || (!added_regions_set.empty()) || (!removed_regions_set.empty()))
               {
                 update_regions(update_regions_set, added_regions_set, removed_regions_set, region_heap, region_heap_size);
               }
               return true;
             }
             else
               merge_region->set_large_nghbr_merged_flag(true);
           }
           if ((params.debug > 3) || (best_region->get_npix() > merge_region->get_npix()) ||
               ((best_region->get_npix() == merge_region->get_npix()) &&
                (best_region->get_label() < merge_region->get_label())))
           {
             if (params.debug > 0)
             {
               params.log_fs << "Merging region " << best_region->get_label() << " with npix = " << best_region->get_npix();
               params.log_fs << " into region " << merge_region->get_label() << " with npix = " << merge_region->get_npix() << endl;
               params.log_fs << "Merge threshold (nghbr_thresh) = " << nghbr_thresh << endl;
               if ((best_region->get_npix() == merge_region->get_npix()) &&
                   (best_region->get_label() < merge_region->get_label()))
                 params.log_fs << "WARNING:  Smaller label region merged into a larger label region" << endl;
               else if (best_region->get_npix() > merge_region->get_npix())
                 params.log_fs << "WARNING:  Larger region merged into a smaller region" << endl;
               best_region->print(region_classes);
               merge_region->print(region_classes);
             }
             else if ((best_region->get_npix() > merge_region->get_npix()) ||
                      ((best_region->get_npix() == merge_region->get_npix()) &&
                       (best_region->get_label() < merge_region->get_label())))
             {
               cout << "Merging region " << best_region->get_label() << " with npix = " << best_region->get_npix();
               cout << " into region " << merge_region->get_label() << " with npix = " << merge_region->get_npix() << endl;
               cout << "Merge threshold (nghbr_thresh) = " << nghbr_thresh << endl;
               if ((best_region->get_npix() == merge_region->get_npix()) &&
                   (best_region->get_label() < merge_region->get_label()))
                 cout << "WARNING:  Smaller label region merged into a larger label region" << endl;
               else if (best_region->get_npix() > merge_region->get_npix())
                 cout << "WARNING:  Larger region merged into a smaller region" << endl;
#ifdef PARALLEL
               cout << "Message from taskid = " << params.myid << endl;
#endif
             }
           }
           remove_from_nghbr_heap(0,nghbr_heap,nghbr_heap_size);
           update_nghbrs_set.insert(merge_region);
           update_nghbrs_set.erase(best_region);
           if (params.spclust_wght_flag && (new_max_threshold != 0.0) &&
               ((merge_region->get_npix() + best_region->get_npix()) >= params.min_npixels))
           {
             if (merge_region->get_npix() < params.min_npixels)
               added_regions_set.insert(merge_region);
             else
               update_regions_set.insert(merge_region);
           }
           if (params.spclust_wght_flag && (new_max_threshold != 0.0) &&
               (best_region->get_npix() >= params.min_npixels))
           {
             heap_index = best_region->get_region_heap_index();
             if (heap_index < UINT_MAX)
               remove_from_region_heap(heap_index,region_heap,region_heap_size);
             removed_regions_set.insert(best_region);
             update_regions_set.erase(best_region);
             added_regions_set.erase(best_region);
           }
           merge_region->merge_regions(last_stage_flag,threshold,best_region,region_classes);
         // new_max_threshold becomes max_threshold only if a merge occurs!
           max_threshold = new_max_threshold;
           --nregions;
         } // if (merge_label != 0)

         update_nghbrs(update_nghbrs_set, nghbr_heap, nghbr_heap_size, region_classes);
         update_nghbrs_set.clear();

         best_region = nghbr_heap[0];
         while (!best_region->get_active_flag()) // Should never happen
         {
           if (params.debug > 0)
             params.log_fs << "WARNING(4): Found inactive region at top of heap!" << endl;
           else
             cout << "WARNING(4): Found inactive region at top of heap!" << endl;
           remove_from_nghbr_heap(0,nghbr_heap,nghbr_heap_size);
           best_region = nghbr_heap[0];
         }
         nghbr_thresh = best_region->get_best_nghbr_dissim();
       } // while (((nghbr_thresh <= new_max_threshold) && (nregions > converge_nregions)) || (nghbr_thresh == 0.0))
       if (params.debug > 2)
       {
         params.log_fs << endl << "After region growing with threshold = " << max_threshold << "," << endl;
         params.log_fs << "the number of regions = " << nregions << endl << endl;
       }
       if (((region_heap_size > 0) || (!added_regions_set.empty())) && 
           ((!update_regions_set.empty()) || (!added_regions_set.empty()) || (!removed_regions_set.empty())))
       {
         update_regions(update_regions_set, added_regions_set, removed_regions_set, region_heap, region_heap_size);
       }
       update_regions_set.clear();
       added_regions_set.clear();
       removed_regions_set.clear();
     } // if (nghbr_thresh < FLT_MAX)
   }
   else if ((nghbr_heap_size > 0) && (region_heap_size > 1))
   {
     process_flag = true;
     nghbr_thresh = max_threshold;
     new_max_threshold = nghbr_thresh;
     best_region = region_heap[0];
     while (!best_region->get_active_flag()) // Should never happen
     {
       if (params.debug > 0)
         params.log_fs << "WARNING(6): Found inactive region at top of heap!" << endl;
       else
         cout << "WARNING(6): Found inactive region at top of heap!" << endl;
       remove_from_region_heap(0,region_heap,region_heap_size);
       best_region = region_heap[0];
     }
     spclust_thresh = best_region->get_best_region_dissim();
     if ((spclust_thresh >= (new_max_threshold*params.spclust_wght)) &&
         (spclust_thresh != 0.0))
       process_flag = false;
     if (process_flag == false)
     {
       best_region = nghbr_heap[0];
       while (!best_region->get_active_flag()) // Should never happen
       {
         if (params.debug > 0)
           params.log_fs << "WARNING(5): Found inactive region at top of heap!" << endl;
         else
           cout << "WARNING(5): Found inactive region at top of heap!" << endl;
         remove_from_nghbr_heap(0,nghbr_heap,nghbr_heap_size);
         best_region = nghbr_heap[0];
       }
       nghbr_thresh = best_region->get_best_nghbr_dissim();
       new_max_threshold = max_threshold;
       if (nghbr_thresh > new_max_threshold)
         new_max_threshold = nghbr_thresh;
     }
     if ((nghbr_thresh > 0.0) || (process_flag))
     {
      // Merge all pairs of regions with dissimilariy less than the threshold.
       best_region = region_heap[0];
       while (!best_region->get_active_flag()) // Should never happen
       {
         if (params.debug > 0)
           params.log_fs << "WARNING(6): Found inactive region at top of heap!" << endl;
         else
           cout << "WARNING(6): Found inactive region at top of heap!" << endl;
         remove_from_region_heap(0,region_heap,region_heap_size);
         best_region = region_heap[0];
       }
       spclust_thresh = best_region->get_best_region_dissim();
       while (((spclust_thresh < (new_max_threshold*params.spclust_wght)) &&
               (nregions > converge_nregions)) || (spclust_thresh == 0.0))
       {
         threshold = spclust_thresh;
         merge_label = best_region->get_best_region_label(region_classes);

         if (merge_label != 0)
         {
           merge_region = &region_classes[merge_label-1];
#ifdef DEBUG
           if (best_region->get_npix() > merge_region->get_npix())
           {
             if (params.debug > 2)
             {
               params.log_fs << "WARNING: Found best_region->npix = " << best_region->get_npix() << " > ";
               params.log_fs << "merge region->npix " << merge_region->get_npix() << endl;
               params.log_fs << "(best_region->region_heap_npix = " << best_region->get_region_heap_npix() << " and ";
               params.log_fs << "merge region->region_heap_npix " << merge_region->get_region_heap_npix() << ")" << endl;
               params.log_fs << "(best_region->label = " << best_region->get_label() << " and ";
               params.log_fs << "merge region->label " << merge_region->get_label() << ")" << endl;
               params.log_fs << "Calling merge_region->region_bring_to_top_heap to avoid this." << endl;
             }
             merge_region->region_bring_to_top_heap(region_heap,region_heap_size);
             best_region = region_heap[0];
             merge_label = best_region->get_best_region_label(region_classes);
             merge_region = &region_classes[merge_label-1];
           }
           if ((best_region->get_npix() == merge_region->get_npix()) &&
               (best_region->get_label() < merge_region->get_label()))
           {
             if (params.debug > 2)
             {
               params.log_fs << "WARNING: Found best_region->label = " << best_region->get_label() << " < ";
               params.log_fs << "merge region->label " << merge_region->get_label() << endl;
               params.log_fs << "(best_region->npix = " << best_region->get_npix() << " and ";
               params.log_fs << "merge region->npix " << merge_region->get_npix() << endl;
               params.log_fs << "(best_region->region_heap_npix = " << best_region->get_region_heap_npix() << " and ";
               params.log_fs << "merge region->region_heap_npix " << merge_region->get_region_heap_npix() << ")" << endl;
               params.log_fs << "Calling check_region_heap to correct this." << endl;
             }
             check_region_heap(region_heap,region_heap_size);
             best_region = region_heap[0];
             while (!best_region->get_active_flag())
             {
               if (params.debug > 0)
                 params.log_fs << "WARNING(7): Found inactive region at top of heap!" << endl;
               else
                 cout << "WARNING(7): Found inactive region at top of heap!" << endl;
               remove_from_region_heap(0,region_heap,region_heap_size);
               best_region = region_heap[0];
             }
             if ((params.debug > 2) && (spclust_thresh != best_region->get_best_region_dissim()))
             {
               params.log_fs << "WARNING: spclust_thresh = " << spclust_thresh << " != ";
               params.log_fs << "best_region->best_region_dissim = " << best_region->get_best_region_dissim() << endl;
             }
             merge_label = best_region->get_best_region_label(region_classes);
             merge_region = &region_classes[merge_label-1];
           }
#endif
           if ((last_stage_flag) && (best_region->get_npix() >= params.min_npixels) && (merge_region->get_npix() >= params.min_npixels))
           {
             if (merge_region->is_nghbr(best_region->get_label()))
             {
               if ((best_region->get_large_nghbr_merged_flag()) ||
                   (merge_region->get_large_nghbr_merged_flag()))
               {
                 if (!update_nghbrs_set.empty())
                 {
                   update_nghbrs(update_nghbrs_set, nghbr_heap, nghbr_heap_size, region_classes);
                 }
                 return true;
               }
               else
                 merge_region->set_large_nghbr_merged_flag(true);
             } // if (merge_region->is_nghbr(best_region->get_label())
           }
           if ((params.debug > 3) || (best_region->get_npix() > merge_region->get_npix()) ||
               ((best_region->get_npix() == merge_region->get_npix()) &&
                (best_region->get_label() < merge_region->get_label())))
           {
             if (params.debug > 0)
             {
               params.log_fs << "Merging region " << best_region->get_label() << " with npix = " << best_region->get_npix();
               params.log_fs << " into region " << merge_region->get_label() << " with npix = " << merge_region->get_npix() << endl;
               params.log_fs << "Merge threshold (spclust_thresh) = " << spclust_thresh << endl;
               if ((best_region->get_npix() == merge_region->get_npix()) &&
                   (best_region->get_label() < merge_region->get_label()))
                 params.log_fs << "WARNING:  Smaller label region merged into a larger label region" << endl;
               else if (best_region->get_npix() > merge_region->get_npix())
                 params.log_fs << "WARNING:  Larger region merged into a smaller region" << endl;
               best_region->print(region_classes);
               merge_region->print(region_classes);
             }
             else if ((best_region->get_npix() > merge_region->get_npix()) ||
                      ((best_region->get_npix() == merge_region->get_npix()) &&
                       (best_region->get_label() < merge_region->get_label())))
             {
               cout << "Merging region " << best_region->get_label() << " with npix = " << best_region->get_npix();
               cout << " into region " << merge_region->get_label() << " with npix = " << merge_region->get_npix() << endl;
               cout << "Merge threshold (spclust_thresh) = " << spclust_thresh << endl;
               if ((best_region->get_npix() == merge_region->get_npix()) &&
                   (best_region->get_label() < merge_region->get_label()))
                 cout << "WARNING:  Smaller label region merged into a larger label region" << endl;
               else if (best_region->get_npix() > merge_region->get_npix())
                 cout << "WARNING:  Larger region merged into a smaller region" << endl;
#ifdef PARALLEL
               cout << "Message from taskid = " << params.myid << endl;
#endif
             }
           }
           remove_from_region_heap(0,region_heap,region_heap_size);
           heap_index = best_region->get_nghbr_heap_index();
           if (heap_index < UINT_MAX)
             remove_from_nghbr_heap(heap_index,nghbr_heap,nghbr_heap_size);
           update_nghbrs_set.insert(merge_region);
           update_nghbrs_set.erase(best_region);
           if ((merge_region->get_npix() + best_region->get_npix()) >= params.min_npixels)
           {
             if (merge_region->get_npix() < params.min_npixels)
               added_regions_set.insert(merge_region);
             else
               update_regions_set.insert(merge_region);
           }
           if (best_region->get_npix() >= params.min_npixels)
           {
             removed_regions_set.insert(best_region);
             update_regions_set.erase(best_region);
             added_regions_set.erase(best_region);
           }
           merge_region->merge_regions(last_stage_flag,threshold,best_region,region_classes);
         // max_threshold is updated only if a merge occurs!
           if (max_threshold < (spclust_thresh/params.spclust_wght))
             max_threshold = spclust_thresh/params.spclust_wght;
           --nregions;
         } // if (merge_label != 0)

         update_regions(update_regions_set, added_regions_set, removed_regions_set, region_heap, region_heap_size);
         update_regions_set.clear();
         added_regions_set.clear();
         removed_regions_set.clear();

         best_region = region_heap[0];
         while (!best_region->get_active_flag())
         {
           if (params.debug > 0)
             params.log_fs << "WARNING(8): Found inactive region at top of heap!" << endl;
           else
             cout << "WARNING(8): Found inactive region at top of heap!" << endl;
           remove_from_region_heap(0,region_heap,region_heap_size);
           best_region = region_heap[0];
         }
         spclust_thresh = best_region->get_best_region_dissim();
       } // while (((spclust_thresh < (new_max_threshold*params.spclust_wght)) &&
         //        (nregions > converge_nregions)) || (spclust_thresh == 0.0))
       if ((!update_nghbrs_set.empty()) && (params.debug > 2))
       {
         params.log_fs << endl << "After spectral clustering with threshold = " << max_threshold << "," << endl;
         params.log_fs << "the number of regions = " << nregions << endl << endl;
       }
       if ((!update_nghbrs_set.empty()) && (nregions > converge_nregions))
       {
         update_nghbrs(update_nghbrs_set, nghbr_heap, nghbr_heap_size, region_classes);
         update_nghbrs_set.clear();
       } // if ((!update_nghbrs_set.empty()) && (nregions > converge_nregions))
     } // if (nghbr_thresh > 0.0)

    // Now go to neighbor merges
     best_region = nghbr_heap[0];
     while (!best_region->get_active_flag())
     {
       if (params.debug > 0)
         params.log_fs << "WARNING(9): Found inactive region at top of heap!" << endl;
       else
         cout << "WARNING(9): Found inactive region at top of heap!" << endl;
       remove_from_nghbr_heap(0,nghbr_heap,nghbr_heap_size);
       best_region = nghbr_heap[0];
     }
     nghbr_thresh = best_region->get_best_nghbr_dissim();
    // NOTE: Keep the previous new_max_threshold!
     if (nghbr_thresh < FLT_MAX)
     {
     // Merge all pairs of spatially adjacent (neighboring) regions
     // with dissimilarity less than or equal to the threshold.
       while (((nghbr_thresh <= new_max_threshold) && (nregions > converge_nregions)) ||
              (nghbr_thresh == 0.0))
       {
         threshold = nghbr_thresh;
         merge_label = best_region->get_best_nghbr_label(region_classes);
         if (merge_label != 0)
         {
           merge_region = &region_classes[merge_label-1];
#ifdef DEBUG
           if (best_region->get_npix() > merge_region->get_npix())
           {
             if (params.debug > 2)
             {
               params.log_fs << "WARNING: Found best_region->npix = " << best_region->get_npix() << " > ";
               params.log_fs << "merge region->npix " << merge_region->get_npix() << endl;
               params.log_fs << "(best_region->nghbr_heap_npix = " << best_region->get_nghbr_heap_npix() << " and ";
               params.log_fs << "merge region->nghbr_heap_npix " << merge_region->get_nghbr_heap_npix() << ")" << endl;
               params.log_fs << "(best_region->label = " << best_region->get_label() << " and ";
               params.log_fs << "merge region->label " << merge_region->get_label() << ")" << endl;
               params.log_fs << "Calling merge_region->nghbr_bring_to_top_heap to avoid this." << endl;
             }
             merge_region->nghbr_bring_to_top_heap(nghbr_heap,nghbr_heap_size);
             best_region = nghbr_heap[0];
             merge_label = best_region->get_best_nghbr_label(region_classes);
             merge_region = &region_classes[merge_label-1];
           }
           if ((best_region->get_npix() == merge_region->get_npix()) &&
               (best_region->get_label() < merge_region->get_label()))
           {
             if (params.debug > 2)
             {
               params.log_fs << "WARNING: Found best_region->label = " << best_region->get_label() << " < ";
               params.log_fs << "merge region->label " << merge_region->get_label() << endl;
               params.log_fs << "(best_region->npix = " << best_region->get_npix() << " and ";
               params.log_fs << "merge region->npix " << merge_region->get_npix() << endl;
               params.log_fs << "(best_region->nghbr_heap_npix = " << best_region->get_nghbr_heap_npix() << " and ";
               params.log_fs << "merge region->nghbr_heap_npix " << merge_region->get_nghbr_heap_npix() << ")" << endl;
               params.log_fs << "Calling check_nghbr_heap to correct this." << endl;
             }
             check_nghbr_heap(nghbr_heap,nghbr_heap_size);
             best_region = nghbr_heap[0];
             while (!best_region->get_active_flag())
             {
               if (params.debug > 0)
                 params.log_fs << "WARNING(10): Found inactive region at top of heap!" << endl;
               else
                 cout << "WARNING(10): Found inactive region at top of heap!" << endl;
               remove_from_nghbr_heap(0,nghbr_heap,nghbr_heap_size);
               best_region = nghbr_heap[0];
             }
             if ((params.debug > 2) && (nghbr_thresh != best_region->get_best_nghbr_dissim()))
             {
               params.log_fs << "WARNING: nghbr_thresh = " << nghbr_thresh << " != ";
               params.log_fs << "best_region->best_nghbr_dissim = " << best_region->get_best_nghbr_dissim() << endl;
             }
             merge_label = best_region->get_best_nghbr_label(region_classes);
             merge_region = &region_classes[merge_label-1];
           }
#endif
           if ((last_stage_flag) && (best_region->get_npix() >= params.min_npixels) && (merge_region->get_npix() >= params.min_npixels))
           {
             if ((best_region->get_large_nghbr_merged_flag()) ||
                 (merge_region->get_large_nghbr_merged_flag()))
             {
               if ((!update_regions_set.empty()) || (!added_regions_set.empty()) || (!removed_regions_set.empty()))
               {
                 update_regions(update_regions_set, added_regions_set, removed_regions_set, region_heap, region_heap_size);
               }
               return true;
             }
             else
               merge_region->set_large_nghbr_merged_flag(true);
           }
           if ((params.debug > 3) || (best_region->get_npix() > merge_region->get_npix()) ||
               ((best_region->get_npix() == merge_region->get_npix()) &&
                (best_region->get_label() < merge_region->get_label())))
           {
             if (params.debug > 0)
             {
               params.log_fs << "Merging region " << best_region->get_label() << " with npix = " << best_region->get_npix();
               params.log_fs << " into region " << merge_region->get_label() << " with npix = " << merge_region->get_npix() << endl;
               params.log_fs << "Merge threshold (nghbr_thresh) = " << nghbr_thresh << endl;
               if ((best_region->get_npix() == merge_region->get_npix()) &&
                   (best_region->get_label() < merge_region->get_label()))
                 params.log_fs << "WARNING:  Smaller label region merged into a larger label region" << endl;
               else if (best_region->get_npix() > merge_region->get_npix())
                 params.log_fs << "WARNING:  Larger region merged into a smaller region" << endl;
               best_region->print(region_classes);
               merge_region->print(region_classes);
             }
             else if ((best_region->get_npix() > merge_region->get_npix()) ||
                      ((best_region->get_npix() == merge_region->get_npix()) &&
                       (best_region->get_label() < merge_region->get_label())))
             {
               cout << "Merging region " << best_region->get_label() << " with npix = " << best_region->get_npix();
               cout << " into region " << merge_region->get_label() << " with npix = " << merge_region->get_npix() << endl;
               cout << "Merge threshold (nghbr_thresh) = " << nghbr_thresh << endl;
               if ((best_region->get_npix() == merge_region->get_npix()) &&
                   (best_region->get_label() < merge_region->get_label()))
                 cout << "WARNING:  Smaller label region merged into a larger label region" << endl;
               else if (best_region->get_npix() > merge_region->get_npix())
                 cout << "WARNING:  Larger region merged into a smaller region" << endl;
#ifdef PARALLEL
               cout << "Message from taskid = " << params.myid << endl;
#endif
             }
           }
           remove_from_nghbr_heap(0,nghbr_heap,nghbr_heap_size);
           update_nghbrs_set.insert(merge_region);
           update_nghbrs_set.erase(best_region);
           if ((merge_region->get_npix() + best_region->get_npix()) >= params.min_npixels)
           {
             if (merge_region->get_npix() < params.min_npixels)
               added_regions_set.insert(merge_region);
             else
               update_regions_set.insert(merge_region);
           }
           if (best_region->get_npix() >= params.min_npixels)
           {
             heap_index = best_region->get_region_heap_index();
             if (heap_index < UINT_MAX)
               remove_from_region_heap(heap_index,region_heap,region_heap_size);
             removed_regions_set.insert(best_region);
             update_regions_set.erase(best_region);
             added_regions_set.erase(best_region);
           }
           merge_region->merge_regions(last_stage_flag,threshold,best_region,region_classes);
         // new_max_threshold becomes max_threshold only if a merge occurs!
           max_threshold = new_max_threshold;
           --nregions;
         } // if (merge_label != 0)

         update_nghbrs(update_nghbrs_set, nghbr_heap, nghbr_heap_size, region_classes);
         update_nghbrs_set.clear();

         best_region = nghbr_heap[0];
         while (!best_region->get_active_flag())
         {
           if (params.debug > 0)
             params.log_fs << "WARNING(12): Found inactive region at top of heap!" << endl;
           else
             cout << "WARNING(12): Found inactive region at top of heap!" << endl;
           remove_from_nghbr_heap(0,nghbr_heap,nghbr_heap_size);
           best_region = nghbr_heap[0];
         }
         nghbr_thresh = best_region->get_best_nghbr_dissim();
       } // while (((nghbr_thresh <= new_max_threshold) && (nregions > converge_nregions)) ||
         //        (nghbr_thresh == 0.0))
       if (params.debug > 2)
       {
         params.log_fs << endl << "After region growing with threshold = " << max_threshold << "," << endl;
         params.log_fs << "the number of regions = " << nregions << endl << endl;
       }
       if (((region_heap_size > 0) || (!added_regions_set.empty())) && 
           ((!update_regions_set.empty()) || (!added_regions_set.empty()) || (!removed_regions_set.empty())))
       {
         update_regions(update_regions_set, added_regions_set, removed_regions_set, region_heap, region_heap_size);
         update_regions_set.clear();
         added_regions_set.clear();
         removed_regions_set.clear();
       }
     } // if (nghbr_thresh < FLT_MAX)
   }
   else
   {
     if (params.debug > 0)
       params.log_fs << "WARNING:  Invalid case reached in merge_regions" << endl;
     else
     {
       cout << "WARNING:  Invalid case reached in merge_regions" << endl;
#ifdef PARALLEL
       cout << "Message from task ID " << params.myid << endl;
#endif
     }
   }

   return false;
 }

 void merge_seam_regions(vector<RegionClass *>& nghbr_heap, unsigned int& nghbr_heap_size,
                         vector<RegionClass>& seam_region_classes, vector<RegionClass>& region_classes,
                         unsigned int& nregions, double& max_threshold)
 {
   unsigned int best_label, merge_label;
   double threshold, nghbr_thresh;
   RegionClass *best_seam_region, *merge_seam_region;
   RegionClass *best_region, *merge_region;
   set<RegionClass *> update_nghbrs_set;

   best_seam_region = nghbr_heap[0];
   while (!best_seam_region->get_active_flag()) // Should never happen
   {
       if (params.debug > 0)
         params.log_fs << "WARNING(1): Found inactive region at top of heap!" << endl;
       else
         cout << "WARNING(1): Found inactive region at top of heap!" << endl;
       remove_from_nghbr_heap(0,nghbr_heap,nghbr_heap_size);
       best_seam_region = nghbr_heap[0];
   }
   nghbr_thresh = best_seam_region->get_best_nghbr_dissim();
   threshold = nghbr_thresh;
   if (nghbr_thresh <= max_threshold)
   {
     // Merge all pairs of spatially adjacent (neighboring) regions
     // with dissimilarity less than or equal to the threshold.
       while (nghbr_thresh <= threshold)
       {
         best_label = best_seam_region->get_label();
         merge_label = best_seam_region->get_best_nghbr_label(seam_region_classes);

         if (merge_label != 0)
         {
           merge_seam_region = &seam_region_classes[merge_label-1];
           best_region = &region_classes[best_label-1];
           merge_region = &region_classes[merge_label-1];
#ifdef DEBUG
           if (best_seam_region->get_npix() > merge_seam_region->get_npix())
           {
             if (params.debug > 2)
             {
               params.log_fs << "WARNING: Found best_seam_region->npix = " << best_seam_region->get_npix() << " > ";
               params.log_fs << "merge region->npix " << merge_seam_region->get_npix() << endl;
               params.log_fs << "(best_seam_region->nghbr_heap_npix = " << best_seam_region->get_nghbr_heap_npix() << " and ";
               params.log_fs << "merge region->nghbr_heap_npix " << merge_seam_region->get_nghbr_heap_npix() << ")" << endl;
               params.log_fs << "(best_seam_region->label = " << best_seam_region->get_label() << " and ";
               params.log_fs << "merge region->label " << merge_seam_region->get_label() << ")" << endl;
               params.log_fs << "Calling merge_seam_region->nghbr_bring_to_top_heap to avoid this." << endl;
             }
             merge_seam_region->nghbr_bring_to_top_heap(nghbr_heap,nghbr_heap_size);
             best_seam_region = nghbr_heap[0];
             merge_label = best_seam_region->get_best_nghbr_label(seam_region_classes);
             merge_seam_region = &seam_region_classes[merge_label-1];
           }
           if ((best_seam_region->get_npix() == merge_seam_region->get_npix()) &&
               (best_seam_region->get_label() < merge_seam_region->get_label()))
           {
             if (params.debug > 2)
             {
               params.log_fs << "WARNING: Found best_seam_region->label = " << best_seam_region->get_label() << " < ";
               params.log_fs << "merge region->label " << merge_seam_region->get_label() << endl;
               params.log_fs << "(best_seam_region->npix = " << best_seam_region->get_npix() << " and ";
               params.log_fs << "merge region->npix " << merge_seam_region->get_npix() << endl;
               params.log_fs << "(best_seam_region->nghbr_heap_npix = " << best_seam_region->get_nghbr_heap_npix() << " and ";
               params.log_fs << "merge region->nghbr_heap_npix " << merge_seam_region->get_nghbr_heap_npix() << ")" << endl;
               params.log_fs << "Calling check_nghbr_heap to correct this." << endl;
             }
             check_nghbr_heap(nghbr_heap,nghbr_heap_size);
             best_seam_region = nghbr_heap[0];
             while (!best_seam_region->get_active_flag())
             {
               if (params.debug > 0)
                 params.log_fs << "WARNING(2): Found inactive region at top of heap!" << endl;
               else
                 cout << "WARNING(2): Found inactive region at top of heap!" << endl;
               remove_from_nghbr_heap(0,nghbr_heap,nghbr_heap_size);
               best_seam_region = nghbr_heap[0];
             }
             if ((params.debug > 2) && (nghbr_thresh != best_seam_region->get_best_nghbr_dissim()))
             {
               params.log_fs << "WARNING: nghbr_thresh = " << nghbr_thresh << " != ";
               params.log_fs << "best_seam_region->best_nghbr_dissim = " << best_seam_region->get_best_nghbr_dissim() << endl;
             }
             merge_label = best_seam_region->get_best_nghbr_label(seam_region_classes);
             merge_seam_region = &seam_region_classes[merge_label-1];
           }
#endif
           if ((params.debug > 3) || (best_seam_region->get_npix() > merge_seam_region->get_npix()) ||
               ((best_seam_region->get_npix() == merge_seam_region->get_npix()) &&
                (best_seam_region->get_label() < merge_seam_region->get_label())))
           {
             if (params.debug > 0)
             {
               params.log_fs << "Merging region " << best_seam_region->get_label() << " with npix = " << best_seam_region->get_npix();
               params.log_fs << " into region " << merge_seam_region->get_label() << " with npix = " << merge_seam_region->get_npix() << endl;
               params.log_fs << "Merge threshold (nghbr_thresh) = " << nghbr_thresh << endl;
               if ((best_seam_region->get_npix() == merge_seam_region->get_npix()) &&
                   (best_seam_region->get_label() < merge_seam_region->get_label()))
                 params.log_fs << "WARNING:  Smaller label region merged into a larger label region" << endl;
               else if (best_seam_region->get_npix() > merge_seam_region->get_npix())
                 params.log_fs << "WARNING:  Larger region merged into a smaller region" << endl;
               best_seam_region->print(seam_region_classes);
               merge_seam_region->print(seam_region_classes);
             }
             else if ((best_seam_region->get_npix() > merge_seam_region->get_npix()) ||
                      ((best_seam_region->get_npix() == merge_seam_region->get_npix()) &&
                       (best_seam_region->get_label() < merge_seam_region->get_label())))
             {
               cout << "Merging region " << best_seam_region->get_label() << " with npix = " << best_seam_region->get_npix();
               cout << " into region " << merge_seam_region->get_label() << " with npix = " << merge_seam_region->get_npix() << endl;
               cout << "Merge threshold (nghbr_thresh) = " << nghbr_thresh << endl;
               if ((best_seam_region->get_npix() == merge_seam_region->get_npix()) &&
                   (best_seam_region->get_label() < merge_seam_region->get_label()))
                 cout << "WARNING:  Smaller label region merged into a larger label region" << endl;
               else if (best_seam_region->get_npix() > merge_seam_region->get_npix())
                 cout << "WARNING:  Larger region merged into a smaller region" << endl;
#ifdef PARALLEL
               cout << "Message from taskid = " << params.myid << endl;
#endif
             }
           }
           remove_from_nghbr_heap(0,nghbr_heap,nghbr_heap_size);
           update_nghbrs_set.insert(merge_seam_region);
           update_nghbrs_set.erase(best_seam_region);
           merge_seam_region->merge_regions(false,nghbr_thresh,best_seam_region,seam_region_classes);
           merge_region->merge_regions(false,nghbr_thresh,best_region,region_classes);
           --nregions;
         } // if (merge_label != 0)

         update_nghbrs(update_nghbrs_set, nghbr_heap, nghbr_heap_size, seam_region_classes);
         update_nghbrs_set.clear();

         best_seam_region = nghbr_heap[0];
         while (!best_seam_region->get_active_flag()) // Should never happen
         {
           if (params.debug > 0)
             params.log_fs << "WARNING(4): Found inactive region at top of heap!" << endl;
           else
             cout << "WARNING(4): Found inactive region at top of heap!" << endl;
           remove_from_nghbr_heap(0,nghbr_heap,nghbr_heap_size);
           best_seam_region = nghbr_heap[0];
         }
         nghbr_thresh = best_seam_region->get_best_nghbr_dissim();
       } // while (nghbr_thresh <= threshold)
   } // if (nghbr_thresh <= max_threshold)

   return;
 }

} // namespace HSEGTilton
