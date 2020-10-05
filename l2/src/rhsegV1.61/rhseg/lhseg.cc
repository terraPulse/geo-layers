/*-----------------------------------------------------------
|
|  Routine Name: lhseg
|
|       Purpose: Performs hybrid of region growing and spectral clustering
|
|         Input: recur_level      (Current recursive level of this task)
|                section          (Section or window processed by this call to lrhseg)
|                converge_nregions(Number of regions below which to stop the region growing process)
|                pixel_data       (Class which holds information pertaining to the pixels processed by this task)
|                
|        Output: max_threshold    (Maximum merging threshold encountered)
|
|         Other: region_classes   (Class which holds region class related information)
|                nregions         (Current number of regions)
|                temp_data        (buffers used in communications between parallel tasks)
|
|       Returns:
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: September 17, 2002.
| Modifications: February 10, 2003 - Changed region_index to region_label
|                December 23, 2004 - Changed region label from short unsigned int to unsigned int
|                January 4, 2004 - Changed from sorting lists to utilizing a heap structure
|                May 31, 2005 - Added temporary file I/O for faster processing of large data sets
|                May 12, 2008 - Revised to work with globally defined Params and oParams class objects
|                January 10, 2010 - Revised nghbr_heap and region_heap initialization
|		 January 11, 2010 - Implemented new definition of min_npixels parameter.
|                June 7, 2010 - Modified operation of the RHSEG program mode, eliminating the need for the max_min_npixels parameter.
|                July 6, 2010 - Allowed local value of spclust_max to rise above params.spclust_max as needed to keep
|                               region_heap_size > 1.
|                December 28, 2010 - Added spclust_min parameter.
|                January 18, 2011 - Refined code controlling the value of the min_npixels parameter.
|                December 8, 2011 - Modified code to initialize region_heap for region_heap_size > 0 instead of region_heap_size > 1.
|		 February 29, 2012 - Corrected the initialization of params.min_npixels!!!
|		 August 11, 2013 - Added the lhseg_edge function which performs processing window artifact elimination based on edge information.
|		 September 18, 2013 - Renamed lhseg_edge as artifact_elimination and removed the restricted lhseg function.
|                January 27, 2014 - Revised the artifact_elimination function - and moved into its own separate source code file.
|		 August 18, 2014 - Bug fix (search for August 18 in code)
|
------------------------------------------------------------*/

#include "hseg.h"
#include <params/params.h>
#include <pixel/pixel.h>
#include <region/region_class.h>
#include <iostream>
#ifdef DEBUG
#include <ctime>
#endif

extern HSEGTilton::Params params;
extern HSEGTilton::oParams oparams;

namespace HSEGTilton
{

 void lhseg(const short unsigned int& recur_level, const bool& last_stage_flag, const short unsigned int& section,
            unsigned int& converge_nregions, unsigned int& nregions,
            double& max_threshold, vector<Pixel>& pixel_data, vector<RegionClass>& region_classes,
            vector<RegionClass *>& nghbr_heap, vector<RegionClass *>& region_heap,
            Temp& temp_data)
 {
#ifdef DEBUG
   time_t now;
#endif
   bool process_flag;
   unsigned int region_index, prev_nregions, prev_min_npixels, max_min_npixels;
   unsigned int iteration = 0, prev_print_it = 0;
   unsigned int onregions = nregions;
   unsigned int spclust_min, spclust_min_factor = 2;
   unsigned int spclust_max, spclust_max_factor = 6;
   unsigned int region_classes_size = region_classes.size();
   unsigned int heap_index, nghbr_heap_size;
   unsigned int region_heap_size, save_region_heap_size, prev_region_heap_size;
   map<unsigned int,unsigned int> region_class_relabel_pairs;
   set<RegionClass *> update_nghbrs_set;

   if (params.debug > 2)
   {
     params.log_fs << endl << "At recursive level = " << recur_level;
     params.log_fs << ", calling HSeg with the number of regions = " << nregions;
     params.log_fs << " and converge_nregions = " << converge_nregions;
     params.log_fs << endl;
   }

   if (params.program_mode != 3)
   {
     oparams.percent_complete = (100*(oparams.tot_npixels-nregions))/oparams.tot_npixels;
     if (oparams.percent_complete == 100)
       oparams.percent_complete = 99;
#ifdef PARALLEL
     if ((params.myid == 0) && (params.debug > 0))
       params.log_fs << "Processing is " << oparams.percent_complete << "% complete\r" << flush;
#else
     if (!params.gtkmm_flag)
       cout << "Processing is " << oparams.percent_complete << "% complete\r" << flush;
#endif
   }

   nghbr_heap_size = 0;
   prev_region_heap_size = region_heap_size = 0;
   nghbr_heap.clear();
   params.min_npixels = 1;
   for (region_index = 0; region_index < region_classes_size; ++region_index)
   {
     if (region_classes[region_index].get_active_flag())
     {
       region_classes[region_index].clear_best_nghbr();
       nghbr_heap.push_back(&region_classes[region_index]);
       nghbr_heap_size++;
     }
   }
   nghbr_heap.push_back(NULL);

   for (heap_index = 0; heap_index < nghbr_heap_size; ++heap_index)
     nghbr_heap[heap_index]->best_nghbr_init(region_classes);
   make_nghbr_heap(nghbr_heap, nghbr_heap_size);

   if (max_threshold == 0.0)
   {
    // Do one iteration of neighbor merges at max_threshold = 0.0.
    // This is the most efficient way to merge together a large homogeneous area.
#ifdef DEBUG
     if (params.debug > 3)
     {
       params.log_fs << endl << "After initializing regions, dump of the region data:" << endl << endl;
       for (region_index = 0; region_index < region_classes_size; ++region_index)
         region_classes[region_index].print(region_classes);
     }
#endif
     if (nghbr_heap[0]->get_best_nghbr_dissim() == 0.0)
     {
       if (params.debug > 1)
         params.log_fs << endl << "Performing neighbor merges with max_threshold = 0.0" << endl;

       merge_regions(false, converge_nregions, nghbr_heap, nghbr_heap_size, 
                     region_heap, region_heap_size, region_classes, nregions, max_threshold);

       max_threshold = nghbr_heap[0]->get_best_nghbr_dissim();
       if ((params.hseg_out_thresholds_flag) && (max_threshold > params.hseg_out_thresholds[0]))
         max_threshold = params.hseg_out_thresholds[0];

       if (onregions > nregions)
       {
         ++iteration;
         if (params.debug > 1)
         {
           params.log_fs << endl << "At iteration " << iteration << " lhseg converged";
           params.log_fs << " for neighbor merges with at the number of regions = " << nregions << endl;
           params.log_fs << "with maximum merging threshold = " << max_threshold;
#ifdef DEBUG
           now = time(NULL);
           params.log_fs << " at " << ctime(&now) << endl;
#else
           params.log_fs << endl;
#endif
         }
       }
     }
     else
     { 
       if (nregions > converge_nregions) // Added this July 16, 2013
       {
         max_threshold = nghbr_heap[0]->get_best_nghbr_dissim();
         if ((params.hseg_out_thresholds_flag) && (max_threshold > params.hseg_out_thresholds[0]))
           max_threshold = params.hseg_out_thresholds[0];
         if (params.debug > 1)
           params.log_fs << endl << "No neighbor merges found with max_threshold = 0.0 and max_threshold reset to " << max_threshold << endl;
       }
       else if (params.debug > 1)
         params.log_fs << endl << "No neighbor merges found with max_threshold = 0.0" << endl;
     }
   } // if (max_threshold == 0)

   if (params.program_mode != 3)
   {
     oparams.percent_complete = (100*(oparams.tot_npixels-nregions))/oparams.tot_npixels;
     if (oparams.percent_complete == 100)
       oparams.percent_complete = 99;
#ifdef PARALLEL
     if ((params.myid == 0) && (params.debug > 0))
       params.log_fs << "Processing is " << oparams.percent_complete << "% complete\r" << flush;
#else
     if (!params.gtkmm_flag)
       cout << "Processing is " << oparams.percent_complete << "% complete\r" << flush;
#endif
   }

 // Added the process_flag check here July 16, 2013
   process_flag = (nregions > converge_nregions);
   if ((last_stage_flag) && (params.hseg_out_thresholds_flag))
     process_flag = (max_threshold < params.hseg_out_thresholds[0]);

   if (process_flag && (params.spclust_wght_flag))
   {
    // Find proper value for min_npixels
     prev_min_npixels = params.min_npixels;
     params.min_npixels = 0;
     region_heap_size = params.spclust_max + 1;
     while (region_heap_size > params.spclust_max)
     {
       params.min_npixels++;
       region_heap_size = 0;
       for (region_index = 0; region_index < region_classes_size; ++region_index)
       {
         if ((region_classes[region_index].get_active_flag()) && 
             (region_classes[region_index].get_npix() >= params.min_npixels))
           region_heap_size++;
         if (region_heap_size > params.spclust_max)
           break;
       }
     } // while (region_heap_size > params.spclust_max)

     if (region_heap_size > 0)
     {
      // Initialize region_heap
       region_heap.clear();
       region_heap_size = 0;
       for (region_index = 0; region_index < region_classes_size; ++region_index)
       {
         if (region_classes[region_index].get_active_flag())
         {
           region_classes[region_index].set_region_heap_index(UINT_MAX);
           if (region_classes[region_index].get_npix() >= params.min_npixels)
           {
             region_classes[region_index].clear_best_region();
             region_heap.push_back(&region_classes[region_index]);
             region_heap_size++;
           }
         }
       }

       region_heap.push_back(NULL);

       for (heap_index = 0; heap_index < region_heap_size; ++heap_index)
         region_heap[heap_index]->best_region_init(heap_index,region_heap,region_heap_size);
       make_region_heap(region_heap, region_heap_size);

      // Update nghbr_heap (if necessary)
       if ((params.merge_accel_flag) && (params.min_npixels != prev_min_npixels))
       {
         max_min_npixels = params.min_npixels;
         if (max_min_npixels < prev_min_npixels)
           max_min_npixels = prev_min_npixels;
         update_nghbrs_set.clear();
         for (heap_index = 0; heap_index < nghbr_heap_size; ++heap_index)
         {
           if (nghbr_heap[heap_index]->get_npix() < max_min_npixels)
             update_nghbrs_set.insert(nghbr_heap[heap_index]);
         }
         update_nghbrs(update_nghbrs_set, nghbr_heap, nghbr_heap_size, region_classes);
       }
     } // if (region_heap_size > 0)
     else
     {
       region_heap_size = 0;
       region_heap.push_back(NULL);
     }

     if (params.debug > 1)
     {
       params.log_fs << "Entering lhseg, min_npixels set to " << params.min_npixels;
       params.log_fs << " (region_heap_size = " << region_heap_size << ")." << endl;
     }
   } // if (params.spclust_wght_flag)
   else
   {
     region_heap_size = 0;
     region_heap.push_back(NULL);
   }

  // This process_flag check was already here prior to July 16, 2013
   process_flag = (nregions > converge_nregions);
   if ((last_stage_flag) && (params.hseg_out_thresholds_flag))
     process_flag = (max_threshold < params.hseg_out_thresholds[0]);

   process_flag = (process_flag && (region_heap_size < 2));
   if (process_flag)
   {
    // Perform iterations of neighbor merges
     if (params.debug > 1)
       params.log_fs << endl << "Performing iterations of neighbor merges" << endl;
#ifdef DEBUG
     if (params.debug > 3)
     {
       params.log_fs << endl << "After initializing regions, dump of the region data:" << endl << endl;
       for (region_index = 0; region_index < region_classes_size; ++region_index)
         region_classes[region_index].print(region_classes);
     }
#endif
     while (process_flag)
     {
       prev_nregions = nregions;
       ++iteration;
       merge_regions(false, converge_nregions, nghbr_heap, nghbr_heap_size, 
                     region_heap, region_heap_size, region_classes, nregions, max_threshold);

       process_flag = (nregions > converge_nregions);
       if ((last_stage_flag) && (params.hseg_out_thresholds_flag))
         process_flag = (max_threshold < params.hseg_out_thresholds[0]);
       if ((max_threshold >= FLT_MAX) || (prev_nregions == nregions))
         process_flag = false;

       if (params.spclust_wght_flag)
       {
        // Find proper value for min_npixels
         prev_min_npixels = params.min_npixels;
         params.min_npixels = 0;
         region_heap_size = params.spclust_max + 1;
         while (region_heap_size > params.spclust_max)
         {
           params.min_npixels++;
           region_heap_size = 0;
           for (region_index = 0; region_index < region_classes_size; ++region_index)
           {
             if ((region_classes[region_index].get_active_flag()) && 
                 (region_classes[region_index].get_npix() >= params.min_npixels))
               region_heap_size++;
             if (region_heap_size > params.spclust_max)
               break;
           }
         } // while (region_heap_size > params.spclust_max)

//         process_flag = (region_heap_size < 2);
         process_flag = process_flag && (region_heap_size < 2);  // Made this correction August 18, 2014

       } // if (params.spclust_wght_flag)

       if (params.program_mode != 3)
       {
         oparams.percent_complete = (100*(oparams.tot_npixels-nregions))/oparams.tot_npixels;
         if (oparams.percent_complete == 100)
           oparams.percent_complete = 99;
#ifdef PARALLEL
         if ((params.myid == 0) && (params.debug > 0))
           params.log_fs << "Processing is " << oparams.percent_complete << "% complete\r" << flush;
#else
         if (!params.gtkmm_flag)
           cout << "Processing is " << oparams.percent_complete << "% complete\r" << flush;
#endif
       }
       if ((iteration - prev_print_it) >= PRINT_INTERVAL)
       {
         prev_print_it = iteration;
         if (params.debug > 1)
         {
           params.log_fs << endl << "At iteration " << iteration << ", maximum threshold = " << max_threshold;
           params.log_fs << "," << endl << "and the number of regions = " << nregions;
#ifdef DEBUG
           now = time(NULL);
           params.log_fs << " at " << ctime(&now) << endl;
           unsigned int max_npix = 0;
           for (heap_index = 0; heap_index < nghbr_heap_size; ++heap_index)
             if (nghbr_heap[heap_index]->get_npix() > max_npix)
               max_npix = nghbr_heap[heap_index]->get_npix();
           params.log_fs << "Largest region has " << max_npix << " pixels." << endl;
#else
           params.log_fs << endl;
#endif
         }
       }
     } // while (process_flag)

     if (params.spclust_wght_flag)
     {
       if (params.debug > 1)
       {
         params.log_fs << endl << "At iteration " << iteration << " lhseg converged";
         params.log_fs << " for neighbor merges";
         if (iteration != prev_print_it)
         {
           params.log_fs << ", maximum merging threshold = " << max_threshold << endl;
           params.log_fs << " and the number of regions = " << nregions;
#ifdef DEBUG
           now = time(NULL);
           params.log_fs << " at " << ctime(&now) << endl;
#else
           params.log_fs << endl;
#endif
         }
         else
           params.log_fs << endl;
       }

      // Find proper value for min_npixels
       prev_region_heap_size = region_heap_size;
       prev_min_npixels = params.min_npixels;
       params.min_npixels = 0;
       region_heap_size = params.spclust_max + 1;
       while (region_heap_size > params.spclust_max)
       {
         params.min_npixels++;
         region_heap_size = 0;
         for (region_index = 0; region_index < region_classes_size; ++region_index)
         {
           if ((region_classes[region_index].get_active_flag()) && 
               (region_classes[region_index].get_npix() >= params.min_npixels))
             region_heap_size++;
           if (region_heap_size > params.spclust_max)
             break;
         }
       } // while (region_heap_size > params.spclust_max)

       if ((params.min_npixels > 1) && (region_heap_size < params.spclust_min))
       {
         params.min_npixels--;
         save_region_heap_size = region_heap_size;
         region_heap_size = 0;
         for (region_index = 0; region_index < region_classes_size; ++region_index)
         {
           if ((region_classes[region_index].get_active_flag()) && 
               (region_classes[region_index].get_npix() >= params.min_npixels))
             region_heap_size++;
         }
         if ((region_heap_size > (spclust_max_factor*params.spclust_max)) && (save_region_heap_size > 1))
         {
           region_heap_size = save_region_heap_size;
           params.min_npixels++;
         }
       } // if ((params.min_npixels > 1) && (region_heap_size < params.spclust_min))

       while (region_heap_size < 2)
       {
         params.min_npixels--;
         region_heap_size = 0;
         for (region_index = 0; region_index < region_classes_size; ++region_index)
         {
           if ((region_classes[region_index].get_active_flag()) && 
               (region_classes[region_index].get_npix() >= params.min_npixels))
             region_heap_size++;
         }
       } // while (region_heap_size < 2)

      // Initialize region_heap
       region_heap.clear();
       region_heap_size = 0;
       for (region_index = 0; region_index < region_classes_size; ++region_index)
       {
         if (region_classes[region_index].get_active_flag())
         {
           region_classes[region_index].set_region_heap_index(UINT_MAX);
           if (region_classes[region_index].get_npix() >= params.min_npixels)
           {
             region_classes[region_index].clear_best_region();
             region_heap.push_back(&region_classes[region_index]);
             region_heap_size++;
           }
         }
       }

       region_heap.push_back(NULL);

       for (heap_index = 0; heap_index < region_heap_size; ++heap_index)
         region_heap[heap_index]->best_region_init(heap_index,region_heap,region_heap_size);
       make_region_heap(region_heap, region_heap_size);

      // Update nghbr_heap (if necessary)
       if ((params.merge_accel_flag) && (params.min_npixels != prev_min_npixels))
       {
         max_min_npixels = params.min_npixels;
         if (max_min_npixels < prev_min_npixels)
           max_min_npixels = prev_min_npixels;
         update_nghbrs_set.clear();
         for (heap_index = 0; heap_index < nghbr_heap_size; ++heap_index)
         {
           if (nghbr_heap[heap_index]->get_npix() < max_min_npixels)
             update_nghbrs_set.insert(nghbr_heap[heap_index]);
         }  
         update_nghbrs(update_nghbrs_set, nghbr_heap, nghbr_heap_size, region_classes);
       }

       if (params.debug > 1)
       {
         params.log_fs << "After neighbor merges, min_npixels set to " << params.min_npixels;
         params.log_fs << " (region_heap_size = " << region_heap_size << ")." << endl;
       }
     } // if (params.spclust_wght_flag)
   } // if (process_flag)
   else if (params.debug > 1)
     params.log_fs << endl << "No iterations of neighbor merges performed." << endl;

   if (params.program_mode != 3)
   {
     oparams.percent_complete = (100*(oparams.tot_npixels-nregions))/oparams.tot_npixels;
     if (oparams.percent_complete == 100)
       oparams.percent_complete = 99;
#ifdef PARALLEL
     if ((params.myid == 0) && (params.debug > 0))
       params.log_fs << "Processing is " << oparams.percent_complete << "% complete\r" << flush;
#else
     if (!params.gtkmm_flag)
       cout << "Processing is " << oparams.percent_complete << "% complete\r" << flush;
#endif
   }

   process_flag = (nregions > converge_nregions);
   if ((last_stage_flag) && (params.hseg_out_thresholds_flag))
     process_flag = (max_threshold < params.hseg_out_thresholds[0]);
   if (process_flag && params.spclust_wght_flag) // NOTE: region_heap_size is guaranteed to be > 1 at this point
   {
    // Perform a combination of spectral clustering merges and region neighbor merges
     if (params.debug > 1)
       params.log_fs << endl << "Performing a combination of spectral clustering and neighbor merges" << endl;
#ifdef DEBUG
     if (params.debug > 3)
     {
       params.log_fs << endl << "After initializing regions, dump of the region data:" << endl << endl;
       for (region_index = 0; region_index < region_classes_size; ++region_index)
         region_classes[region_index].print(region_classes);
     }
#endif
     unsigned int max_spclust_min = (unsigned int) (params.spclust_max - 0.05*(params.spclust_max - params.spclust_min));
     int temp_int;

     spclust_min = region_heap_size;
     if (region_heap_size <= params.spclust_max)
     {
       temp_int = spclust_min_factor*(params.spclust_max - region_heap_size);
       temp_int = (int) params.spclust_max - temp_int;
       if (temp_int > (int) params.spclust_min)
         spclust_min = (unsigned int) temp_int;
     }
     if (spclust_min > nregions)
       spclust_min = nregions;
     if (spclust_min > max_spclust_min)
       spclust_min = max_spclust_min;

     spclust_max = region_heap_size;
     if (spclust_max < params.spclust_max)
       spclust_max = params.spclust_max;

     if (params.debug > 1)
     {
       params.log_fs << "Entering main lhseg processing loop, spclust_min = " << spclust_min << ", spclust_max = " << spclust_max; 
       params.log_fs << ", and min_npixels = " << params.min_npixels << endl;
     }
     while (process_flag)
     {
       ++iteration;
       if ((region_heap_size > spclust_max) || ((region_heap_size < spclust_min) && (params.min_npixels > 1)))
       {
        // Readjust min_npixels if indicated
         prev_region_heap_size = region_heap_size;
         prev_min_npixels = params.min_npixels;
        // Find proper value for min_npixels
         params.min_npixels = 0;
         region_heap_size = params.spclust_max + 1;
         while (region_heap_size > params.spclust_max)
         {
           params.min_npixels++;
           region_heap_size = 0;
           for (region_index = 0; region_index < region_classes_size; ++region_index)
           {
             if ((region_classes[region_index].get_active_flag()) && 
                 (region_classes[region_index].get_npix() >= params.min_npixels))
               region_heap_size++;
             if (region_heap_size > params.spclust_max)
               break;
           }
         } // while (region_heap_size > params.spclust_max)

         if ((params.min_npixels > 1) && (region_heap_size < params.spclust_min))
         {
           params.min_npixels--;
           save_region_heap_size = region_heap_size;
           region_heap_size = 0;
           for (region_index = 0; region_index < region_classes_size; ++region_index)
           {
             if ((region_classes[region_index].get_active_flag()) && 
                 (region_classes[region_index].get_npix() >= params.min_npixels))
                region_heap_size++;
           }
           if ((region_heap_size > (spclust_max_factor*params.spclust_max)) && (save_region_heap_size > 1))
           {
             region_heap_size = save_region_heap_size;
             params.min_npixels++;
           }
         } // if ((params.min_npixels > 1) && (region_heap_size < params.spclust_min))

         while (region_heap_size < 2)
         {
           params.min_npixels--;
           region_heap_size = 0;
           for (region_index = 0; region_index < region_classes_size; ++region_index)
           {
             if ((region_classes[region_index].get_active_flag()) && 
                 (region_classes[region_index].get_npix() >= params.min_npixels))
               region_heap_size++;
           }
         } // while (region_heap_size < 2)

         if (params.min_npixels != prev_min_npixels)
         {
           if (params.debug > 1)
           {
             params.log_fs << "Readjusting min_npixels. Currently min_npixels = " << prev_min_npixels;
             params.log_fs << " and region_heap_size = " << prev_region_heap_size << "." << endl;
           }

           if (params.min_npixels < prev_min_npixels)
             max_threshold = 0.0;

         // Reinitialize region_heap
           region_heap.clear();
           region_heap_size = 0;
           for (region_index = 0; region_index < region_classes_size; ++region_index)
           {
             if (region_classes[region_index].get_active_flag())
             {
               region_classes[region_index].set_region_heap_index(UINT_MAX);
               if (region_classes[region_index].get_npix() >= params.min_npixels)
               {
                 region_classes[region_index].clear_best_region();
                 region_heap.push_back(&region_classes[region_index]);
                 region_heap_size++;
               }
             }
           }
           region_heap.push_back(NULL);
         
           if (params.debug > 1)
           {
             params.log_fs << "At iteration " << iteration << " with nregions = " << nregions;
             params.log_fs << ", min_npixels readjusted to " << params.min_npixels;
             params.log_fs << " giving region_heap_size = " << region_heap_size << "." << endl;
           }

           for (heap_index = 0; heap_index < region_heap_size; ++heap_index)
             region_heap[heap_index]->best_region_init(heap_index,region_heap,region_heap_size);
           make_region_heap(region_heap, region_heap_size);

           if (params.merge_accel_flag)
           {
           // Update nghbr_heap
             max_min_npixels = params.min_npixels;
             if (max_min_npixels < prev_min_npixels)
               max_min_npixels = prev_min_npixels;
             update_nghbrs_set.clear();
             for (heap_index = 0; heap_index < nghbr_heap_size; ++heap_index)
             {
               if (nghbr_heap[heap_index]->get_npix() < max_min_npixels)
                 update_nghbrs_set.insert(nghbr_heap[heap_index]);
             }
             update_nghbrs(update_nghbrs_set, nghbr_heap, nghbr_heap_size, region_classes);
           }

         } // if (params.min_npixels != prev_min_npixels)

         spclust_min = region_heap_size;
         if (region_heap_size <= params.spclust_max)
         {
           temp_int = spclust_min_factor*(params.spclust_max - region_heap_size);
           temp_int = (int) params.spclust_max - temp_int;
           if (temp_int > (int) params.spclust_min)
             spclust_min = (unsigned int) temp_int;
         }
         if (spclust_min > nregions)
           spclust_min = nregions;
         if (spclust_min > max_spclust_min)
           spclust_min = max_spclust_min;
         spclust_max = region_heap_size;
         if (spclust_max < params.spclust_max)
          spclust_max = params.spclust_max;
         if (params.debug > 2)
           params.log_fs << "spclust_min readjusted to " << spclust_min << ", and spclust_max readjusted to " << spclust_max << endl; 

       } // if ((region_heap_size > spclust_max) || ((region_heap_size < spclust_min) && (params.min_npixels > 1)))

       prev_nregions = nregions;
       if (converge_nregions == 0)
         process_flag = (params.min_npixels > 1);
       if (process_flag)
       {
         merge_regions(false, converge_nregions, nghbr_heap, nghbr_heap_size, 
                       region_heap, region_heap_size, region_classes, nregions, max_threshold);
       }

       if (converge_nregions > 0)
         process_flag = (nregions > converge_nregions);
       if ((last_stage_flag) && (params.hseg_out_thresholds_flag))
         process_flag = (max_threshold < params.hseg_out_thresholds[0]);
       if ((max_threshold >= FLT_MAX) || (prev_nregions == nregions))
         process_flag = false;

       if (params.program_mode != 3)
       {
         oparams.percent_complete = (100*(oparams.tot_npixels-nregions))/oparams.tot_npixels;
         if (oparams.percent_complete == 100)
           oparams.percent_complete = 99;
#ifdef PARALLEL
         if ((params.myid == 0) && (params.debug > 0))
           params.log_fs << "Processing is " << oparams.percent_complete << "% complete\r" << flush;
#else
         if (!params.gtkmm_flag)
           cout << "Processing is " << oparams.percent_complete << "% complete\r" << flush;
#endif
       }
       if ((iteration - prev_print_it) >= PRINT_INTERVAL)
       {
         prev_print_it = iteration;
         if (params.debug > 1)
         {
           params.log_fs << endl << "At iteration " << iteration << ", maximum threshold = " << max_threshold << "," << endl;
           if (region_heap_size > 0)
             params.log_fs << "region_heap_size = " << region_heap_size << " ";
           params.log_fs << " and the number of regions = " << nregions;
#ifdef DEBUG
           now = time(NULL);
           params.log_fs << " at " << ctime(&now) << endl;
           unsigned int max_npix = 0;
           for (heap_index = 0; heap_index < nghbr_heap_size; ++heap_index)
             if (nghbr_heap[heap_index]->get_npix() > max_npix)
               max_npix = nghbr_heap[heap_index]->get_npix();
           params.log_fs << "Largest region has " << max_npix << " pixels." << endl;
#else
           params.log_fs << endl;
#endif
         }
       }
     } // while (process_flag)

     if (params.debug > 1)
     {
       params.log_fs << endl << "At iteration " << iteration << " lhseg converged";
       params.log_fs << " for spectral clustering merges";
       params.log_fs << ", maximum merging threshold = " << max_threshold << endl;
       if (region_heap_size > 0)
         params.log_fs << "region_heap_size = " << region_heap_size << " ";
       params.log_fs << " and the number of regions = " << nregions;
#ifdef DEBUG
       now = time(NULL);
       params.log_fs << " at " << ctime(&now) << endl;
#else
       params.log_fs << endl;
#endif
     }
   } // if (process_flag)
   else if (params.debug > 1)
     params.log_fs << endl << "Combination of spectral clustering and neighbor merges not performed" << endl;

   if (params.program_mode != 3)
   {
     oparams.percent_complete = (100*(oparams.tot_npixels-nregions))/oparams.tot_npixels;
     if (oparams.percent_complete == 100)
       oparams.percent_complete = 99;
#ifdef PARALLEL
     if ((params.myid == 0) && (params.debug > 0))
       params.log_fs << "Processing is " << oparams.percent_complete << "% complete\r" << flush;
#else
     if (!params.gtkmm_flag)
       cout << "Processing is " << oparams.percent_complete << "% complete\r" << flush;
#endif
   }

   if (params.debug > 1)
   {
     params.log_fs << endl << "At iteration " << iteration << " lhseg converged";
     if (iteration != prev_print_it)
     {
       params.log_fs << ", maximum merging threshold = " << max_threshold << "," << endl;
       if (region_heap_size > 0)
         params.log_fs << "region_heap_size = " << region_heap_size << " ";
       params.log_fs << " and the number of regions = " << nregions;
#ifdef DEBUG
       now = time(NULL);
       params.log_fs << " at " << ctime(&now) << endl;
#else
       params.log_fs << endl;
#endif
     }
     else
       params.log_fs << endl;
   }
#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << endl << "Exiting merge_region loop, dump of the region data:" << endl << endl;
     for (region_index = 0; region_index < region_classes_size; ++region_index)
       region_classes[region_index].print(region_classes);
   }
#endif
   if (onregions != nregions)
   {
    // Update pixel_data.region_label based on merges performed this call to lhseg.
     unsigned int region_label, merge_region_index, merge_region_label;
     for (region_index = 0; region_index < region_classes_size; ++region_index)
     {
       merge_region_index = region_index;
       merge_region_label = region_classes[merge_region_index].get_merge_region_label();
       if (merge_region_label != 0)
       {
         while (merge_region_label != 0)
         {
           merge_region_index = merge_region_label - 1;
           merge_region_label = region_classes[merge_region_index].get_merge_region_label();
         }
         region_classes[region_index].set_merge_region_label(region_classes[merge_region_index].get_label());
       }
     }

     region_class_relabel_pairs.clear();
     for (region_index = 0; region_index < region_classes_size; ++region_index)
     {
       region_label = region_index + 1;
       merge_region_label = region_classes[region_index].get_merge_region_label();
       if (merge_region_label != 0)
         region_class_relabel_pairs.insert(make_pair(region_label,merge_region_label));
     }
     if (!(region_class_relabel_pairs.empty()))
       do_region_class_relabel(recur_level,section,region_class_relabel_pairs,pixel_data,temp_data);
     region_class_relabel_pairs.clear();

     for (region_index = 0; region_index < region_classes_size; ++region_index)
       region_classes[region_index].set_merge_region_label(0);
   } // if (onregions != nregions)
#ifdef DEBUG
   if (params.debug > 3)
   {
     params.log_fs << endl << "Exiting lhseg, dump of the region data:" << endl << endl;
     for (region_index = 0; region_index < region_classes_size; ++region_index)
       region_classes[region_index].print(region_classes);
   }
#endif
   if (params.debug > 2)
   {
     params.log_fs << "Completed region relabeling within lhseg" << endl;
   }
   return;
 }

} // namespace HSEGTilton
