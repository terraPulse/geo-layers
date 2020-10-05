/*-----------------------------------------------------------
|
|  Routine Name: find_nghbr - Finds the neighbor row, column and slice coordinates
|                (nbrow,nbcol,nbslice) for the neighbor of (row,col,slice)
|                in the direction specified by "nbdir".
|
|       Purpose:
|
|         Input: col     (column location of a pixel)
|                row     (row location of a pixel)
|                slice   (slice location of a pixel)
|                nbdir   (direction index of the pixel's neighbor)
|
|        Output: nbcol   (column location of neighboring pixel)
|                nbrow   (row location of neighboring pixel)
|                nbslice (slice location of neighboring pixel)
|
|       Returns: void
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|          Date: September 16, 2002
| Modifications: October 21, 2005 - Added slice dimension (extension to three-dimensional analysis)
|                May 12, 2008 - Revised to work with globally defined Params and oParams class objects
|
| Neighborhood Chart for 1-D case:
|
|          ----------------------------
|          | 6| 4| 2| 0| X| 1| 3| 5| 7|
|          ----------------------------
|
| Neighborhood Chart for 2-D case:
|
|                ----------------
|                |20|16|10|14|22|
|                ----------------
|                |12| 4| 2| 6|18|
|                ----------------
|                | 8| 0| X| 1| 9|
|                ----------------
|                |19| 7| 3| 5|13|
|                ----------------
|                |23|15|11|17|21|
|                ----------------
|
| Neighborhood Chart for 3-D case:
|
|       slice-1       slice        slice+1
|     ----------    ----------    ----------
|     |18|10|22|    | 6| 2| 8|    |21|13|25|
|     ----------    ----------    ----------
|     |14| 4|16|    | 0| X| 1|    |17| 5|15|
|     ----------    ----------    ----------
|     |24|12|20|    | 9| 3| 7|    |23|11|19|
|     ----------    ----------    ----------
|
------------------------------------------------------------*/
#include <params/params.h>

extern HSEGTilton::Params params;

namespace HSEGTilton
{
#ifdef THREEDIM
 void find_nghbr(const int& col, const int& row, const int& slice,
                 const short unsigned int& nbdir, int& nbcol, int& nbrow, int& nbslice)
#else
 void find_nghbr(const int& col, const int& row,
                 const short unsigned int& nbdir, int& nbcol, int& nbrow)
#endif
 {
#ifdef THREEDIM
    nbcol = nbrow = nbslice = 0;
#else
    nbcol = nbrow = 0;
#endif
    switch(params.nb_dimensions)
    {
      case 1:  switch(nbdir)
               {
                 // W
                 case 0:  nbcol = col-1;
                          break;
                 // E
                 case 1:  nbcol = col+1;
                          break;
                 case 2:  nbcol = col-2;
                          break;
                 case 3:  nbcol = col+2;
                          break;
                 case 4:  nbcol = col-3;
                          break;
                 case 5:  nbcol = col+3;
                          break;
                 case 6:  nbcol = col-4;
                          break;
                 case 7:  nbcol = col+4;
                          break;
               }
               break;
      case 2:  switch(nbdir)
               {
                 // W
                 case 0:  nbcol = col-1; nbrow = row;
                          break;
                 // E
                 case 1:  nbcol = col+1; nbrow = row;
                          break;
                 // N
                 case 2:  nbcol = col; nbrow = row-1;
                          break;
                 // S
                 case 3:  nbcol = col; nbrow = row+1;
                          break;
                 // NW
                 case 4:  nbcol = col-1; nbrow = row-1;
                          break;
                 // SE
                 case 5:  nbcol = col+1; nbrow = row+1;
                          break;
                 // NE
                 case 6:  nbcol = col+1; nbrow = row-1;
                          break;
                 // SW
                 case 7:  nbcol = col-1; nbrow = row+1;
                          break;
                 case 8:  nbcol = col-2; nbrow = row;
                          break;
                 case 9:  nbcol = col+2; nbrow = row;
                          break;
                 case 10: nbcol = col; nbrow = row-2;
                          break;
                 case 11: nbcol = col; nbrow = row+2;
                          break;
                 case 12: nbcol = col-2; nbrow = row-1;
                          break;
                 case 13: nbcol = col+2; nbrow = row+1;
                          break;
                 case 14: nbcol = col+1; nbrow = row-2;
                          break;
                 case 15: nbcol = col-1; nbrow = row+2;
                          break;
                 case 16: nbcol = col-1; nbrow = row-2;
                          break;
                 case 17: nbcol = col+1; nbrow = row+2;
                          break;
                 case 18: nbcol = col+2; nbrow = row-1;
                          break;
                 case 19: nbcol = col-2; nbrow = row+1;
                          break;
                 case 20: nbcol = col-2; nbrow = row-2;
                          break;
                 case 21: nbcol = col+2; nbrow = row+2;
                          break;
                 case 22: nbcol = col+2; nbrow = row-2;
                          break;
                 case 23: nbcol = col-2; nbrow = row+2;
                          break;
               }
               break;
#ifdef THREEDIM
      case 3:  switch(nbdir)
               {
                 case 0:  nbcol = col-1; nbrow = row; nbslice = slice;
                          break;
                 case 1:  nbcol = col+1; nbrow = row; nbslice = slice;
                          break;
                 case 2:  nbcol = col; nbrow = row-1; nbslice = slice;
                          break;
                 case 3:  nbcol = col; nbrow = row+1; nbslice = slice;
                          break;
                 case 4:  nbcol = col; nbrow = row; nbslice = slice-1;
                          break;
                 case 5:  nbcol = col; nbrow = row; nbslice = slice+1;
                          break;
                 case 6:  nbcol = col-1; nbrow = row-1; nbslice = slice;
                          break;
                 case 7:  nbcol = col+1; nbrow = row+1; nbslice = slice;
                          break;
                 case 8:  nbcol = col+1; nbrow = row-1; nbslice = slice;
                          break;
                 case 9:  nbcol = col-1; nbrow = row+1; nbslice = slice;
                          break;
                 case 10: nbcol = col; nbrow = row-1; nbslice = slice-1;
                          break;
                 case 11: nbcol = col; nbrow = row+1; nbslice = slice+1;
                          break;
                 case 12: nbcol = col; nbrow = row+1; nbslice = slice-1;
                          break;
                 case 13: nbcol = col; nbrow = row-1; nbslice = slice+1;
                          break;
                 case 14: nbcol = col-1; nbrow = row; nbslice = slice-1;
                          break;
                 case 15: nbcol = col+1; nbrow = row; nbslice = slice+1;
                          break;
                 case 16: nbcol = col+1; nbrow = row; nbslice = slice-1;
                          break;
                 case 17: nbcol = col-1; nbrow = row; nbslice = slice+1;
                          break;
                 case 18: nbcol = col-1; nbrow = row-1; nbslice = slice-1;
                          break;
                 case 19: nbcol = col+1; nbrow = row+1; nbslice = slice+1;
                          break;
                 case 20: nbcol = col+1; nbrow = row+1; nbslice = slice-1;
                          break;
                 case 21: nbcol = col+1; nbrow = row-1; nbslice = slice+1;
                          break;
                 case 22: nbcol = col+1; nbrow = row-1; nbslice = slice-1;
                          break;
                 case 23: nbcol = col-1; nbrow = row+1; nbslice = slice+1;
                          break;
                 case 24: nbcol = col-1; nbrow = row+1; nbslice = slice-1;
                          break;
                 case 25: nbcol = col-1; nbrow = row-1; nbslice = slice+1;
                          break;
               }
               break;
#endif
    }
 }
} // namespace HSEGTilton
