/*-----------------------------------------------------------
|
|  Routine Name: connected_component - Create a 4-nn connected region object segmentation from a 
|                                      region class segmentation
|
|       Purpose: Main function for the connected_component program
|
|         Input: 
|
|        Output: 
|
|       Returns: TRUE (1) on success, FALSE (0) on failure
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: January 8, 2010
| Modifications: January 17, 2010 - Modified to be functions called by pv_rhseg
|
------------------------------------------------------------*/

#include <image/image.h>
#include <iostream>
#include <map>

using namespace CommonTilton;

namespace HSEGTilton
{
  bool connected_component(Image& classSegImage, Image& objectSegImage)
  {
    int ncols = classSegImage.get_ncols();
    int nrows = classSegImage.get_nrows();

    if ((ncols != objectSegImage.get_ncols()) || (nrows != objectSegImage.get_nrows()))
    {
      cout << "ERROR: Image size mismatch between classSeg image and objectSet image" << endl;
      return false;
    }

    int col, row, max_object_label = 0;
    int class_label, north_class_label, west_class_label;
    int object_label, north_object_label, west_object_label;
    int prev_col, prev_row, prev_object_label;

  // First row
  // First column
    row = 0;
    col = 0;
    class_label = (int) classSegImage.get_data(col,row,0);
    if (class_label > 0)
      max_object_label++;
    object_label = max_object_label;
    objectSegImage.put_data(object_label,col,row,0);
    west_class_label = class_label;
    west_object_label = object_label;
  // Rest of first row
    for (col = 1; col < ncols; col++)
    {
      object_label = 0;
      class_label = (int) classSegImage.get_data(col,row,0);
      if (class_label != 0)
      {
        if (class_label == west_class_label)
          object_label = west_object_label;
        else
          object_label = ++max_object_label;
      }
      objectSegImage.put_data(object_label,col,row,0);
      west_class_label = class_label;
      west_object_label = object_label;
    }

  // Remaining rows
    for (row = 1; row < nrows; row++)
    {
    // First column
      col = 0;
      class_label = (int) classSegImage.get_data(col,row,0);
      if (class_label > 0)
      {
        north_class_label = (int) classSegImage.get_data(col,row-1,0);
        north_object_label = (int) objectSegImage.get_data(col,row-1,0);
        if (class_label == north_class_label)
          object_label = north_object_label;
        else
          object_label = ++max_object_label;
      }
      objectSegImage.put_data(object_label,col,row,0);
      west_class_label = class_label;
      west_object_label = object_label;
  // Rest of row
      for (col = 1; col < ncols; col++)
      {
        class_label = (int) classSegImage.get_data(col,row,0);
        if (class_label > 0)
        {
          if (class_label == west_class_label)
          {
            object_label = west_object_label;
            north_class_label = (int) classSegImage.get_data(col,row-1,0);
            north_object_label = (int) objectSegImage.get_data(col,row-1,0);
            if ((class_label == north_class_label) && (object_label != north_object_label))
            {
           // Relabel previous instances of north_object_label to object_label
              for (prev_row = 0; prev_row < row; prev_row++)
                for (prev_col = 0; prev_col < ncols; prev_col++)
                {
                  prev_object_label = (int) objectSegImage.get_data(prev_col,prev_row,0);
                  if (prev_object_label == north_object_label)
                    objectSegImage.put_data(object_label,prev_col,prev_row,0);
                }
              for (prev_col = 0; prev_col < col; prev_col++)
              {
                prev_object_label = (int) objectSegImage.get_data(prev_col,prev_row,0);
                if (prev_object_label == north_object_label)
                  objectSegImage.put_data(object_label,prev_col,prev_row,0);
              }
            }
          }
          else
          {
            north_class_label = (int) classSegImage.get_data(col,row-1,0);
            north_object_label = (int) objectSegImage.get_data(col,row-1,0);
            if (class_label == north_class_label)
              object_label = north_object_label;
            else
              object_label = ++max_object_label;
          }
        }
        objectSegImage.put_data(object_label,col,row,0);
        west_class_label = class_label;
        west_object_label = object_label;
      }
    }
    objectSegImage.flush_data();

  // Compact the labeling
    map<int,int> object_relabel_pairs;
    map<int,int>::iterator object_relabel_pair_iter;
    max_object_label = 0;
    for (row = 0; row < nrows; row++)
      for (col = 0; col < ncols; col++)
      {
        object_label = (int) objectSegImage.get_data(col,row,0);
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
        objectSegImage.put_data(object_label,col,row,0);
      }

    objectSegImage.flush_data();
    objectSegImage.set_no_data_value(0.0);
    objectSegImage.put_no_data_value();

    cout << "Exiting connected_component, number of connected components = " << max_object_label << endl;

    return true;
  }
} // namespace HSEGTilton
