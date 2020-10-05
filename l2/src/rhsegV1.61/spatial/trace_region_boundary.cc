/*-----------------------------------------------------------
|
|  Routine Name: trace_region_object_boundary
|
|       Purpose: Traces the region boundary for the region object starting at the location first_col and first_row
|
|         Input: first_col    (column coordinate of the initial pixel of the region boundary to be traced)
|                first_row    (row coordinate of the initial pixel of the region boundary to be traced)
|                region_map   (Region Map)
|                
|        Output: X            (X=>column coordinates of region object boundary trace)
|                Y            (Y=>row coordinated of region object boundary trace)
|
|       Returns: (void)
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: May 11, 2011.
| Modifications: August 29, 2011: Corrected code.
|                September 12, 2011: Further corrections.
|
------------------------------------------------------------*/

#include "spatial.h"
#include <params/params.h>
#include <map>

extern HSEGTilton::Params params;

namespace HSEGTilton
{
#ifdef SHAPEFILE 
  bool search_nghbr(const int& col, const int& row, const short int& nbdir, int& nbcol, int& nbrow);
  short int next_search_dir(const short int& search_dir, const bool& clockwise);

  void set_two_label_map(const unsigned int& region_label, unsigned int* region_label_map, 
                         unsigned int* two_label_map, int* label_map_bounds)
  {
    int col, row, index, index2;

  // label_map_bounds[0] = min_col, label_map_bounds[1] = min_row, 
  // label_map_bounds[2] = max_col, label_map_bounds[3] = max_row. 
    label_map_bounds[0] = params.ncols+1;
    label_map_bounds[1] = params.nrows+1;
    label_map_bounds[2] = -1;
    label_map_bounds[3] = -1;

    for (row = -1; row <= params.nrows; row++)
      for (col = -1; col <= params.ncols; col++)
      {
        index = col + row*params.ncols;
        index2 = (col+1) + (row+1)*(params.ncols+2);
        if ((row == -1) || (row == params.nrows) || (col == -1) || (col == params.ncols))
        {
          two_label_map[index2] = 1;
        }
        else
        {
          if (region_label == region_label_map[index])
          {
            two_label_map[index2] = 0;
            if (label_map_bounds[0] > col)
              label_map_bounds[0] = col;
            if (label_map_bounds[1] > row)
              label_map_bounds[1] = row;
            if (label_map_bounds[2] < col)
              label_map_bounds[2] = col;
            if (label_map_bounds[3] < row)
              label_map_bounds[3] = row;
          }
          else
          {
            two_label_map[index2] = 1;
          }
        }
      }

    return;
  }

  void set_connected_label_map(unsigned int* class_label_map, int* label_map_bounds, unsigned int* object_label_map)
  {
    int col, row, index;
    int prev_col, prev_row;
    unsigned int class_label, north_class_label, west_class_label;
    unsigned int object_label, prev_object_label, north_object_label, west_object_label, max_object_label = 1;
    int min_col, min_row, max_col, max_row;

    for (row = -1; row <= params.nrows; row++)
      for (col = -1; col <= params.ncols; col++)
      {
        index = (col+1) + (row+1)*(params.ncols+2);
        object_label_map[index] = 0;
      }

    min_col = label_map_bounds[0]-1;
    min_row = label_map_bounds[1]-1;
    max_col = label_map_bounds[2]+1;
    max_row = label_map_bounds[3]+1;

  // First row
  // First column
    row = min_row;
    col = min_col;
    index = (col+1) + (row+1)*(params.ncols+2);
    class_label = class_label_map[index];
    if (class_label > 0)
      max_object_label++;
    object_label = max_object_label;
    object_label_map[index] = object_label;
    west_class_label = class_label;
    west_object_label = object_label;

  // Rest of first row
    for (col = (min_col+1); col <= max_col; col++)
    {
      object_label = 0;
      index = (col+1) + (row+1)*(params.ncols+2);
      class_label = class_label_map[index];
      if (class_label != 0)
      {
        if (class_label == west_class_label)
          object_label = west_object_label;
        else
          object_label = ++max_object_label;
      }
      object_label_map[index] = object_label;
      west_class_label = class_label;
      west_object_label = object_label;
    }

  // Remaining rows
    for (row = (min_row+1); row <= max_row; row++)
    {
    // First column
      col = min_col;
      object_label = 0;
      index = (col+1) + (row+1)*(params.ncols+2);
      class_label = class_label_map[index];
      if (class_label > 0)
      {
        index = (col+1) + row*(params.ncols+2);
        north_class_label = class_label_map[index];
        north_object_label = object_label_map[index];
        if (class_label == north_class_label)
          object_label = north_object_label;
        else
          object_label = ++max_object_label;
      }
      index = (col+1) + (row+1)*(params.ncols+2);
      object_label_map[index] = object_label;
      west_class_label = class_label;
      west_object_label = object_label;
  // Rest of row
      for (col = (min_col+1); col <=  max_col; col++)
      {
        object_label = 0;
        index = (col+1) + (row+1)*(params.ncols+2);
        class_label = class_label_map[index];
        if (class_label > 0)
        {
          if (class_label == west_class_label)
          {
            object_label = west_object_label;
            object_label_map[index] = object_label;
            index = (col+1) + row*(params.ncols+2);
            north_class_label = class_label_map[index];
            north_object_label = object_label_map[index];
            if ((class_label == north_class_label) && (object_label != north_object_label))
            {
         // Relabel previous instances of north_object_label to object_label
              for (prev_row = min_row; prev_row < row; prev_row++)
                for (prev_col = min_col; prev_col <= max_col; prev_col++)
                {
                  index = (prev_col+1) + (prev_row+1)*(params.ncols+2);
                  prev_object_label = object_label_map[index];
                  if (prev_object_label == north_object_label)
                    object_label_map[index] = object_label;
                }
              for (prev_col = min_col; prev_col < col; prev_col++)
              {
                index = (prev_col+1) + (prev_row+1)*(params.ncols+2);
                prev_object_label = object_label_map[index];
                if (prev_object_label == north_object_label)
                  object_label_map[index] = object_label;
              }
            }
          }
          else
          {
            index = (col+1) + row*(params.ncols+2);
            north_class_label = class_label_map[index];
            north_object_label = object_label_map[index];
            if (class_label == north_class_label)
              object_label = north_object_label;
            else
              object_label = ++max_object_label;
          }
        }
        index = (col+1) + (row+1)*(params.ncols+2);
        object_label_map[index] = object_label;
        west_class_label = class_label;
        west_object_label = object_label;
      }
    }

   // Zero out external region object, leaving only the region object holes 
    index = (min_col+1) + (min_row+1)*(params.ncols+2);
    object_label = object_label_map[index];
    for (row = min_row; row <= max_row; row++)
      for (col = min_col; col <= max_col; col++)
      {
        index = (col+1) + (row+1)*(params.ncols+2);
        if (object_label_map[index] == object_label)
          object_label_map[index] = 0;
      }

  // Recover the primary region object
    min_col = label_map_bounds[0];
    min_row = label_map_bounds[1];
    max_col = label_map_bounds[2];
    max_row = label_map_bounds[3];
    for (row = min_row; row <= max_row; row++)
      for (col = min_col; col <= max_col; col++)
      {
        index = (col+1) + (row+1)*(params.ncols+2);
        if (class_label_map[index] == 0)
          object_label_map[index] = 1;
      }

  // Compact the labeling
    map<int,int> object_relabel_pairs;
    map<int,int>::iterator object_relabel_pair_iter;
    max_object_label = 0;
    for (row = min_row; row <= max_row; row++)
      for (col = min_col; col <= max_col; col++)
      {
        index = (col+1) + (row+1)*(params.ncols+2);
        object_label = object_label_map[index];
        if (object_label != 0)
        {
          object_relabel_pair_iter = object_relabel_pairs.find(object_label);
          if (object_relabel_pair_iter != object_relabel_pairs.end())
          {
             object_label = (*object_relabel_pair_iter).second;
          }
          else
          {
            object_relabel_pairs.insert(make_pair(object_label,++max_object_label));
            object_label = max_object_label;
          }
          object_label_map[index] = object_label;
        }
      }

    return;
  }

 // The boundary_mask twice as large (plus one) and is offset one-half pixel NW of the region_label_map. 
 // A boundary_mask pixel is set to true only where at least one, but not all, of the region_label_map 
 // pixels partially covered by the boundary_mask has the targeted region_label value.
  void set_boundary_mask(const unsigned int& region_label, unsigned int* region_label_map, 
                         bool* boundary_mask, int* boundary_mask_bounds)
  {
    int mask_index, index;
    short int nb_true;
    int mask_col, mask_row, col, row;

  // boundary_mask_bounds[0] = min_mask_col, boundary_mask_bounds[1] = min_mask_row, 
  // boundary_mask_bounds[2] = max_mask_col, boundary_mask_bounds[3] = max_mask_row. 
    boundary_mask_bounds[0] = 2*params.ncols + 2;
    boundary_mask_bounds[1] = 2*params.nrows + 2;
    boundary_mask_bounds[2] = 0;
    boundary_mask_bounds[3] = 0;

    for (mask_row = 0; mask_row < (2*params.nrows+1); mask_row++)
    {
      for (mask_col = 0;  mask_col < (2*params.ncols+1); mask_col++)
      {
        if (mask_col > 0)
          col = (mask_col - 1)/2;
        else
          col = -1;
        if (mask_row > 0)
          row = (mask_row - 1)/2;
        else
          row = -1;
        nb_true = 0;
        index = (col+1) + (row+1)*(params.ncols+2);
        if (region_label == region_label_map[index])
          nb_true++;
        col = mask_col/2;
        if (mask_row > 0)
          row = (mask_row - 1)/2;
        else
          row = -1;
        index = (col+1) + (row+1)*(params.ncols+2);
        if (region_label == region_label_map[index])
          nb_true++;
        if (mask_col > 0)
          col = (mask_col - 1)/2;
        else
          col = -1;
        row = mask_row/2;
        index = (col+1) + (row+1)*(params.ncols+2);
        if (region_label == region_label_map[index])
          nb_true++;
        col = mask_col/2;
        row = mask_row/2;
        index = (col+1) + (row+1)*(params.ncols+2);
        if (region_label == region_label_map[index])
          nb_true++;
        mask_index = mask_col + mask_row*(2*params.ncols+1);
        boundary_mask[mask_index] = false;
        if ((nb_true > 0) && (nb_true < 4))
        {
          boundary_mask[mask_index] = true;
          if (boundary_mask_bounds[0] > mask_col)
            boundary_mask_bounds[0] = mask_col;
          if (boundary_mask_bounds[1] > mask_row)
            boundary_mask_bounds[1] = mask_row;
          if (boundary_mask_bounds[2] < mask_col)
            boundary_mask_bounds[2] = mask_col;
          if (boundary_mask_bounds[3] < mask_row)
            boundary_mask_bounds[3] = mask_row;
        }
      }
    }

    return;
  }

  void trace_region_outer_boundary(bool* boundary_mask, int* boundary_mask_bounds, 
                                   vector<double>& X, vector<double>& Y, vector<int>& partStart)
  {
    int index, initial_index, vertex, part;
    int mask_col, mask_row, nbcol, nbrow;
    double dX, dY;

  // boundary_mask_bounds[0] = min_mask_col, boundary_mask_bounds[1] = min_mask_row, 
  // boundary_mask_bounds[2] = max_mask_col, boundary_mask_bounds[3] = max_mask_row. 

  // Scan for the initial region pixel in upper left hand corner of region
    index = 0; mask_col = 0; mask_row = 0;
    for (mask_row = boundary_mask_bounds[1]; mask_row <= boundary_mask_bounds[3]; mask_row++)
    {
      for (mask_col = boundary_mask_bounds[0]; mask_col <= boundary_mask_bounds[2]; mask_col++)
      {
        index = mask_col + mask_row*(2*params.ncols+1);
        if (boundary_mask[index])
          break;        
      }
      if (boundary_mask[index])
        break;        
    }

  // Need to transform from boundary_mask to region_label_map coordinates
    dX = ((double) (mask_col-1))/2.0 + 0.5;
    dY = ((double) (mask_row-1))/2.0 + 0.5;
    X.push_back(dX);
    Y.push_back(dY);
    vertex = 0;
    partStart.push_back(vertex);
    part = 0;
    index = mask_col + mask_row*(2*params.ncols+1);
    boundary_mask[index] = false;

  // Initial previous search direction is west to east.
    short int search_dir, prev_search_dir = 0;
  // Start current search one search increment back from current search direction.
    search_dir = next_search_dir(prev_search_dir,false);
  // Search for next region object boundary pixel in a clockwise search.
    while (!search_nghbr(mask_col,mask_row,search_dir,nbcol,nbrow))
      search_dir = next_search_dir(search_dir,true);
    index = nbcol + nbrow*(2*params.ncols+1);
    initial_index = index;
    while (!boundary_mask[index])
    {    
      search_dir = next_search_dir(search_dir,true);
      while (!search_nghbr(mask_col,mask_row,search_dir,nbcol,nbrow))
        search_dir = next_search_dir(search_dir,true);
      index = nbcol + nbrow*(2*params.ncols+1);
      if (index == initial_index)
        break;
    }
    prev_search_dir = search_dir;
    while (boundary_mask[index])
    {
    // Record result from previous search 
      boundary_mask[index] = false;
      if (search_dir != prev_search_dir)
      {
        dX = ((double) (mask_col-1))/2.0 + 0.5;
        dY = ((double) (mask_row-1))/2.0 + 0.5;
        X.push_back(dX);
        Y.push_back(dY);
        vertex++;
      }

    // Initialize next search
      mask_col = nbcol;
      mask_row = nbrow;
      prev_search_dir = search_dir;
      search_dir = next_search_dir(prev_search_dir,false);
    // Search for next region object boundary pixel in a clockwise search.
      while (!search_nghbr(mask_col,mask_row,search_dir,nbcol,nbrow))
        search_dir = next_search_dir(search_dir,true);
      index = nbcol + nbrow*(2*params.ncols+1);
      initial_index = index;
      while (!boundary_mask[index])
      {    
        search_dir = next_search_dir(search_dir,true);
        while (!search_nghbr(mask_col,mask_row,search_dir,nbcol,nbrow))
          search_dir = next_search_dir(search_dir,true);
        index = nbcol + nbrow*(2*params.ncols+1);
        if (index == initial_index)
          break;
      }
    }
  // Complete polygon
    dX = X[partStart[part]];
    dY = Y[partStart[part]];
    X.push_back(dX);
    Y.push_back(dY);
    vertex++;

    return;
  }

  void find_region_hole_labels(const unsigned int& region_label, unsigned int* region_label_map, 
                               bool* boundary_mask, int* boundary_mask_bounds, set<unsigned int>& region_hole_label_set)
  {
    int mask_index, index;
    int mask_col, mask_row, col, row;

    for (mask_row = boundary_mask_bounds[1]; mask_row <= boundary_mask_bounds[3]; mask_row++)
    {
      for (mask_col = boundary_mask_bounds[0]; mask_col <= boundary_mask_bounds[2]; mask_col++)
      {
        mask_index = mask_col + mask_row*(2*params.ncols+1);
        if (boundary_mask[mask_index])
        {
          if (mask_col > 0)
            col = (mask_col - 1)/2;
          else
            col = -1;
          if (mask_row > 0)
            row = (mask_row - 1)/2;
          else
            row = -1;
          index = (col+1) + (row+1)*(params.ncols+2);
          if ((0 != region_label_map[index]) && (region_label != region_label_map[index]))
            region_hole_label_set.insert(region_label_map[index]);
          col = mask_col/2;
          if (mask_row > 0)
            row = (mask_row - 1)/2;
          else
            row = -1;
          index = (col+1) + (row+1)*(params.ncols+2);
          if ((0 != region_label_map[index]) && (region_label != region_label_map[index]))
            region_hole_label_set.insert(region_label_map[index]);
          if (mask_col > 0)
            col = (mask_col - 1)/2;
          else
            col = -1;
          row = mask_row/2;
          index = (col+1) + (row+1)*(params.ncols+2);
          if ((0 != region_label_map[index]) && (region_label != region_label_map[index]))
            region_hole_label_set.insert(region_label_map[index]);
          col = mask_col/2;
          row = mask_row/2;
          index = (col+1) + (row+1)*(params.ncols+2);
          if ((0 != region_label_map[index]) && (region_label != region_label_map[index]))
            region_hole_label_set.insert(region_label_map[index]);
        }
      }
    }

    return;
  }

  void trace_region_hole_boundary(bool* boundary_mask, int* boundary_mask_bounds,
                                  vector<double>& X, vector<double>& Y, vector<int>& partStart)
  {
    int index, initial_index, vertex, part;
    int mask_col, mask_row, nbcol, nbrow;
    double dX, dY;

  // Scan for the initial region pixel in upper right hand corner of region
    index = 0; mask_col = 0; mask_row = 0;
    for (mask_row = boundary_mask_bounds[1]; mask_row <= boundary_mask_bounds[3]; mask_row++)
    {
      for (mask_col = boundary_mask_bounds[2]; mask_col >= boundary_mask_bounds[0]; mask_col--)
      {
        index = mask_col + mask_row*(2*params.ncols+1);
        if (boundary_mask[index])
          break;        
      }
      if (boundary_mask[index])
        break;        
    }

  // Need to transform from boundary_mask to region_label_map coordinates
    dX = ((double) (mask_col-1))/2.0 + 0.5;
    dY = ((double) (mask_row-1))/2.0 + 0.5; 
    vertex = X.size();
    X.push_back(dX);
    Y.push_back(dY);
    part = partStart.size();
    partStart.push_back(vertex);
    index = mask_col + mask_row*(2*params.ncols+1);
    boundary_mask[index] = false;
  // Initial previous search direction is west to east.
    short int search_dir, prev_search_dir = 4;
  // Start current search one search increment back from current search direction.
    search_dir = next_search_dir(prev_search_dir,true);
  // Search for next region object boundary pixel in a counter clockwise search.
    while (!search_nghbr(mask_col,mask_row,search_dir,nbcol,nbrow))
      search_dir = next_search_dir(search_dir,false);
    index = nbcol + nbrow*(2*params.ncols+1);
    initial_index = index;
    while (!boundary_mask[index])
    {    
      search_dir = next_search_dir(search_dir,false);
      while (!search_nghbr(mask_col,mask_row,search_dir,nbcol,nbrow))
        search_dir = next_search_dir(search_dir,false);
      index = nbcol + nbrow*(2*params.ncols+1);
      if (index == initial_index)
        break;
    }
    prev_search_dir = search_dir;
    while (boundary_mask[index])
    {
    // Record result from previous search 
      boundary_mask[index] = false;
      if (search_dir != prev_search_dir)
      {
        dX = ((double) (mask_col-1))/2.0 + 0.5;
        dY = ((double) (mask_row-1))/2.0 + 0.5;
        X.push_back(dX);
        Y.push_back(dY);
        vertex++;
      }

    // Initialize next search
      mask_col = nbcol;
      mask_row = nbrow;
      prev_search_dir = search_dir;
      search_dir = next_search_dir(prev_search_dir,true);
    // Search for next region object boundary pixel in a counter clockwise search.
      while (!search_nghbr(mask_col,mask_row,search_dir,nbcol,nbrow))
        search_dir = next_search_dir(search_dir,false);
      index = nbcol + nbrow*(2*params.ncols+1);
      initial_index = index;
      while (!boundary_mask[index])
      {    
        search_dir = next_search_dir(search_dir,false);
        while (!search_nghbr(mask_col,mask_row,search_dir,nbcol,nbrow))
          search_dir = next_search_dir(search_dir,false);
        index = nbcol + nbrow*(2*params.ncols+1);
        if (index == initial_index)
          break;
      }
    }
  // Complete polygon
    dX = X[partStart[part]];
    dY = Y[partStart[part]];
    X.push_back(dX);
    Y.push_back(dY);
    vertex++;

    return;
  }

// Find region label and col and row indices for designated neighbor - only works in 2-D and 4nn and 8nn
/*
| Neighborhood Chart (2-D):
|
|                ----------
|                | 5| 6| 7|
|                ----------
|                | 4| X| 0|
|                ----------
|                | 3| 2| 1|
|                ----------
|
| For 4nn just search the even nbdir values.
*/
  bool search_nghbr(const int& col, const int& row, const short int& nbdir, int& nbcol, int& nbrow)
  {

    bool valid_flag = true;
    nbcol = nbrow = 0;

    switch(nbdir)
    {
      // E
      case 0:  nbcol = col+1; nbrow = row;
               break;
      // SE
      case 1:  nbcol = col+1; nbrow = row+1;
               break;
      // S
      case 2:  nbcol = col; nbrow = row+1;
               break;
      // SW
      case 3:  nbcol = col-1; nbrow = row+1;
               break;
      // W
      case 4:  nbcol = col-1; nbrow = row;
               break;
      // NW
      case 5:  nbcol = col-1; nbrow = row-1;
               break;
      // N
      case 6:  nbcol = col; nbrow = row-1;
               break;
      // NE
      case 7:  nbcol = col+1; nbrow = row-1;
               break;
    }

    if ((nbcol < 0) || (nbrow < 0) || (nbcol >= (2*params.ncols+1)) || (nbrow >= (2*params.nrows+1)))
      valid_flag = false;

    return valid_flag;
  }

  short int next_search_dir(const short int& search_dir, const bool& clockwise)
  {
    short int increment, next_search_dir;

    if (clockwise)
      increment = 1;
    else
      increment = -1;
    if (params.object_maxnbdir <= 4)
      increment *= 2;
    next_search_dir = search_dir + increment;
    
    if (next_search_dir < 0)
      next_search_dir += 8;
    if (next_search_dir > 7)
      next_search_dir -= 8;

    return next_search_dir;
  }

#endif
} // namespace HSEGTilton
