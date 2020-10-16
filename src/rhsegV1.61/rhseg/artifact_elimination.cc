/*-----------------------------------------------------------
|
|  Routine Name: artifact_elimination
|
|       Purpose: Performs region merges along the processing window seams so as to eliminate processing window artifacts
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
|       Written: January 27, 2014
| Modifications: March 28, 2014 - Revised artifact elimination process.
|
------------------------------------------------------------*/

#include "hseg.h"
#include <params/params.h>
#include <index/index.h>
#include <region/region_class.h>
#include <region/region_seam.h>
#include <pixel/pixel.h>
#include <iostream>

extern HSEGTilton::Params params;
extern HSEGTilton::oParams oparams;

namespace HSEGTilton
{
#ifdef THREEDIM
 void artifact_elimination(const short unsigned int& recur_level, const short unsigned int& section,
                           const int& ncols, const int& nrows, const int& nslices, double& max_threshold,
                           vector<Index>& col_seam_index_data, vector<Index>& row_seam_index_data, vector<Index>& slice_seam_index_data,
                           unsigned int& nregions, vector<RegionClass>& seam_region_classes, vector<RegionClass>& region_classes,
                           vector<RegionClass *>& nghbr_heap, vector<Pixel>& pixel_data, Temp& temp_data)
#else
 void artifact_elimination(const short unsigned int& recur_level, const short unsigned int& section,
                           const int& ncols, const int& nrows, double& max_threshold,
                           vector<Index>& col_seam_index_data, vector<Index>& row_seam_index_data,
                           unsigned int& nregions, vector<RegionClass>& seam_region_classes, vector<RegionClass>& region_classes,
                           vector<RegionClass *>& nghbr_heap, vector<Pixel>& pixel_data, Temp& temp_data)
#endif
 {
   bool col_flag, row_flag;
#ifdef THREEDIM
   bool slice_flag;
#endif
   map<unsigned int,unsigned int> region_class_relabel_pairs;

#ifdef THREEDIM
   set_recur_flags(params.recur_mask_flags[recur_level],col_flag,row_flag,slice_flag);
#else
   set_recur_flags(params.recur_mask_flags[recur_level],col_flag,row_flag);
#endif

   if (col_flag)
   {
#ifdef THREEDIM
     seam_region_classes_init(true,false,false,ncols,nrows,nslices,col_seam_index_data,seam_region_classes);
#else
     seam_region_classes_init(true,false,ncols,nrows,col_seam_index_data,seam_region_classes);
#endif
     if (params.debug > 1)
       params.log_fs << endl << "Performing merges in artifact_elimination based on col_seam_index_data" << endl;
     seam_merge(nregions,seam_region_classes,region_classes,region_class_relabel_pairs);
     if (!(region_class_relabel_pairs.empty()))
     {
       do_region_class_relabel(recur_level,section,region_class_relabel_pairs,pixel_data,temp_data);
#ifdef THREEDIM
       do_region_class_relabel(region_class_relabel_pairs,params.seam_size,nrows,nslices,col_seam_index_data);
       do_region_class_relabel(region_class_relabel_pairs,ncols,params.seam_size,nslices,row_seam_index_data);
       do_region_class_relabel(region_class_relabel_pairs,ncols,nrows,params.seam_size,slice_seam_index_data);
#else
       do_region_class_relabel(region_class_relabel_pairs,params.seam_size,nrows,col_seam_index_data);
       do_region_class_relabel(region_class_relabel_pairs,ncols,params.seam_size,row_seam_index_data);
#endif
     }
   } // if (col_flag)

   if (row_flag)
   {
#ifdef THREEDIM
     seam_region_classes_init(false,true,false,ncols,params.seam_size,nslices,row_seam_index_data,seam_region_classes);
#else
     seam_region_classes_init(false,true,ncols,params.seam_size,row_seam_index_data,seam_region_classes);
#endif
     if (params.debug > 1)
       params.log_fs << endl << "Performing merges in artifact_elimination based on row_seam_index_data" << endl;
     seam_merge(nregions,seam_region_classes,region_classes,region_class_relabel_pairs);
     if (!(region_class_relabel_pairs.empty()))
     {
       do_region_class_relabel(recur_level,section,region_class_relabel_pairs,pixel_data,temp_data);
#ifdef THREEDIM
       do_region_class_relabel(region_class_relabel_pairs,ncols,params.seam_size,nslices,row_seam_index_data);
       do_region_class_relabel(region_class_relabel_pairs,ncols,nrows,params.seam_size,slice_seam_index_data);
#else
       do_region_class_relabel(region_class_relabel_pairs,ncols,params.seam_size,row_seam_index_data);
#endif
     }
   } // if (row_flag)

#ifdef THREEDIM
   if (slice_flag)
   {
     seam_region_classes_init(false,false,true,ncols,nrows,params.seam_size,slice_seam_index_data,seam_region_classes);
     if (params.debug > 1)
       params.log_fs << endl << "Performing merges in artifact_elimination based on slice_seam_index_data" << endl;
     seam_merge(nregions,seam_region_classes,region_classes,region_class_relabel_pairs);
     if (!(region_class_relabel_pairs.empty()))
     {
       do_region_class_relabel(recur_level,section,region_class_relabel_pairs,pixel_data,temp_data);
       do_region_class_relabel(region_class_relabel_pairs,ncols,nrows,params.seam_size,slice_seam_index_data);
     }
   } // if (slice_flag)
#endif

   return;
 }

 void seam_merge(unsigned int& nregions, vector<RegionClass>& seam_region_classes, 
                 vector<RegionClass>& region_classes, map<unsigned int,unsigned int>& region_class_relabel_pairs)
 {
   bool merge_flag;
   unsigned int region_index, nghbr_index, region_label, nghbr_label;
   unsigned int region_seam_npix, nghbr_seam_npix;
   unsigned int region_npix, nghbr_npix;
   unsigned int onregions = nregions;
   unsigned int region_classes_size = seam_region_classes.size();
   float ave_edge_value, region_sum_seam_edge_value, nghbr_sum_seam_edge_value;
   map<unsigned int, RegionSeam>::const_iterator nghbrs_label_seam_map_iter, nghbr_nghbrs_label_seam_map_iter;

   for (region_index = 0; region_index < region_classes_size; ++region_index)
   {
     if (seam_region_classes[region_index].get_active_flag())
     {
       nghbrs_label_seam_map_iter = seam_region_classes[region_index].nghbrs_label_seam_map.begin();
       while (nghbrs_label_seam_map_iter != seam_region_classes[region_index].nghbrs_label_seam_map.end())
       {
         merge_flag = false;
         region_label = region_index + 1;
         nghbr_label = (*nghbrs_label_seam_map_iter).first;
         region_seam_npix = (*nghbrs_label_seam_map_iter).second.get_npix();
         region_sum_seam_edge_value = (*nghbrs_label_seam_map_iter).second.get_sum_edge();
         nghbr_index = nghbr_label - 1;
         if (seam_region_classes[nghbr_index].get_active_flag())
         {
           nghbr_nghbrs_label_seam_map_iter = seam_region_classes[nghbr_index].nghbrs_label_seam_map.find(region_label);
           if (nghbr_nghbrs_label_seam_map_iter != seam_region_classes[nghbr_index].nghbrs_label_seam_map.end())
           {
             nghbr_seam_npix = (*nghbr_nghbrs_label_seam_map_iter).second.get_npix();
             nghbr_sum_seam_edge_value = (*nghbr_nghbrs_label_seam_map_iter).second.get_sum_edge();
             if ((region_seam_npix + nghbr_seam_npix) >= MIN_SEAM_EDGE_NPIX)
             {
               ave_edge_value = (region_sum_seam_edge_value + nghbr_sum_seam_edge_value)/(region_seam_npix + nghbr_seam_npix);
               if (ave_edge_value <= params.seam_edge_threshold)
               {
                 region_npix = region_classes[region_index].get_npix();
                 nghbr_npix = region_classes[nghbr_index].get_npix();
                 if ((region_npix >= MIN_SEAM_EDGE_REGION_SIZE) || (nghbr_npix >= MIN_SEAM_EDGE_REGION_SIZE))
                 {
                   if (params.debug > 1)
                     params.log_fs << "Merging neighbor region " << nghbr_label << " into region " << region_label << " in artifact_elimination" << endl;
                   region_classes[region_index].merge_regions(false,params.seam_edge_threshold,&region_classes[nghbr_index],region_classes);
                   seam_region_classes[region_index].merge_seam_regions(&seam_region_classes[nghbr_index],seam_region_classes);
                   nregions--;
                   merge_flag = true;
                 } // if ((region_npix >= MIN_SEAM_EDGE_REGION_SIZE) || (nghbr_npix >= MIN_SEAM_EDGE_REGION_SIZE))
               } // if (ave_edge_value <= params.seam_edge_threshold)
             } // if ((region_seam_npix + nghbr_seam_npix) >= MIN_SEAM_EDGE_NPIX)
           } // if (nghbrs_label_seam_map_iter != seam_region_classes[nghbr_index].nghbrs_label_seam_map.end())
         } // if (seam_region_classes[nghbr_index].get_active_flag())
         if (merge_flag)
           nghbrs_label_seam_map_iter = seam_region_classes[region_index].nghbrs_label_seam_map.begin();
         else
           ++nghbrs_label_seam_map_iter;
       } // while (nghbrs_label_seam_map_iter != seam_region_classes[region_index].nghbrs_label_seam_map.end())
     } // if (seam_region_classes[region_index].get_active_flag())
   } // for (region_index = 0; region_index < region_classes_size; ++region_index)

   region_class_relabel_pairs.clear();
   region_classes_size = region_classes.size();
   if (onregions != nregions)
   {
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

     for (region_index = 0; region_index < region_classes_size; ++region_index)
     {
       region_label = region_index + 1;
       merge_region_label = region_classes[region_index].get_merge_region_label();
       if (merge_region_label != 0)
         region_class_relabel_pairs.insert(make_pair(region_label,merge_region_label));
     }
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

   return;
 }

} // namespace HSEGTilton
