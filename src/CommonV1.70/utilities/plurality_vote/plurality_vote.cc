/*-----------------------------------------------------------
|
|  Routine Name: plurality_vote - Create a plurality vote region-based classification from a 
|                                 pixel-based classification and region segmentation
|
|       Purpose: Main function for the plurality_vote program
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
| Modifications: August 17, 2010: Converted into a general utility program.
|                August 17, 2010: Allowed for use without GDAL.
|                July 31, 2012 - Modified for version 1.59 of Common Image software.
|
------------------------------------------------------------*/

#include "plurality_vote.h"
#include "params/params.h"
#include <iostream>

extern CommonTilton::Params params;
extern CommonTilton::Image pixelClassImage;
extern CommonTilton::Image regionSegImage;

namespace CommonTilton
{
  bool plurality_vote()
  {
    int max_pixelClass_value = (int) pixelClassImage.getMaximum(0) + 1;
    int max_regionSeg_value = (int) regionSegImage.getMaximum(0) + 1;

    unsigned int *contable;
    contable = new unsigned int [max_pixelClass_value*max_regionSeg_value];

    int pixelClass_value, regionSeg_value;
    int pixelClass_index, regionSeg_index, index;
    for (regionSeg_index = 0; regionSeg_index < max_regionSeg_value; regionSeg_index++)
      for (pixelClass_index = 0; pixelClass_index < max_pixelClass_value; pixelClass_index++)
      {
        index = pixelClass_index + regionSeg_index*max_pixelClass_value;
        contable[index] = 0;
      }

    int ncols = regionSegImage.get_ncols();
    int nrows = regionSegImage.get_nrows();
    if ((ncols != pixelClassImage.get_ncols()) || (nrows != pixelClassImage.get_nrows()))
    {
      cout << "ERROR: Image size mismatch between pixelClass image and regionSeg image" << endl;
      return false;
    }
    int col, row;
    for (row = 0; row < nrows; row++)
      for (col = 0; col < ncols; col++)
      {
        pixelClass_value = (int) pixelClassImage.get_data(col,row,0);
        regionSeg_value = (int) regionSegImage.get_data(col,row,0);
        index = pixelClass_value + regionSeg_value*max_pixelClass_value;
        contable[index] += 1;
      }

    unsigned int *vote_table;
    vote_table = new unsigned int [max_regionSeg_value];
    int vote_value, max_vote_value;
    for (regionSeg_index = 0; regionSeg_index < max_regionSeg_value; regionSeg_index++)
    {
      index = regionSeg_index*max_pixelClass_value;
      max_vote_value = contable[index];
      vote_table[regionSeg_index] = 0;
      for (pixelClass_index = 1; pixelClass_index < max_pixelClass_value; pixelClass_index++)
      {
        index = pixelClass_index + regionSeg_index*max_pixelClass_value;
        vote_value = contable[index];
        if (vote_value > max_vote_value)
        {
          max_vote_value = vote_value;
          vote_table[regionSeg_index] = pixelClass_index;
        }
      }
    }

    Image regionClassImage;
#ifdef GDAL
    if (!regionClassImage.create_copy(params.region_class_file, pixelClassImage))
      regionClassImage.create(params.region_class_file, pixelClassImage);
#else
    regionClassImage.create(params.region_class_file, pixelClassImage);
#endif

    for (row = 0; row < nrows; row++)
      for (col = 0; col < ncols; col++)
      {
        regionSeg_value = (int) regionSegImage.get_data(col,row,0);
        pixelClass_value = vote_table[regionSeg_value];
        regionClassImage.put_data(pixelClass_value,col,row,0);
      }

    regionClassImage.flush_data();
#ifdef GDAL
    regionClassImage.set_no_data_value(0.0);
    regionClassImage.put_no_data_value();

    if (params.color_table_flag)
    {
      GDALColorTable colorTable(GPI_RGB);
      GDALColorEntry *colorEntry;
      colorEntry = new GDALColorEntry;
   
      fstream color_table_fs;
      color_table_fs.open(params.color_table_file.c_str(),ios_base::in);

      int entry_index, entry_value;
      int sub_pos;
      string line,sub_string;

      for (entry_index = 0; entry_index < 256; entry_index++)
      {
        getline(color_table_fs,line);
        sub_pos = line.find(":");
        sub_string = line.substr(0,sub_pos);
        entry_value = atoi(sub_string.c_str());
        if (entry_value != entry_index)
        {
          cout << "ERROR: Color Table has unexpected format" << endl;
          return false;
        }
        line = line.substr(sub_pos+1);
        sub_pos = line.find(",");
        sub_string = line.substr(0,sub_pos);
        colorEntry->c1 = atoi(sub_string.c_str());
        line = line.substr(sub_pos+1);
        sub_pos = line.find(",");
        sub_string = line.substr(0,sub_pos);
        colorEntry->c2 = atoi(sub_string.c_str());
        line = line.substr(sub_pos+1);
        sub_pos = line.find(",");
        sub_string = line.substr(0,sub_pos);
        colorEntry->c3 = atoi(sub_string.c_str());
        line = line.substr(sub_pos+1);
        sub_pos = line.find(",");
        sub_string = line.substr(0,sub_pos);
        colorEntry->c4 = atoi(sub_string.c_str());
        colorTable.SetColorEntry(entry_value,colorEntry);
      }
      color_table_fs.close();

      regionClassImage.get_imageDataset()->GetRasterBand(1)->SetColorTable(&colorTable);

    }
    else if (pixelClassImage.get_imageDataset()->GetRasterBand(1)->GetColorTable() != NULL)
      regionClassImage.get_imageDataset()->GetRasterBand(1)->SetColorTable(pixelClassImage.get_imageDataset()->GetRasterBand(1)->GetColorTable());
#endif

    regionClassImage.close();

    return true;
  }
} // namespace CommonTilton
