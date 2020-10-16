// pca.cc

#include "pca.h"
#include "params/params.h"
#include <gdal_priv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

extern Params params;

using namespace std;
using namespace cv;

///////////////////////
// Functions
bool pca()
{
  int col, row, band, index, mask_index, has_fill;
  int ncols, nrows, nbands;
  double fill_value;
  GDALDataset *inDataset, *outDataset;
  GDALDriver *driver;

  inDataset = (GDALDataset *)GDALOpen(params.input_image_file.c_str(), GA_ReadOnly);
  if (!inDataset)
  {
    cout << "Could not open input_HDF file name = " << params.input_image_file << endl;
    return false;
  }

  ncols = inDataset->GetRasterXSize();
  nrows = inDataset->GetRasterYSize();
  nbands = inDataset->GetRasterCount();
  driver = inDataset->GetDriver();
  if (nbands > 1)
  {
    cout << "Performing Principal Components Analysis on input image " << params.input_image_file << endl;
    cout << "with ncols = " << ncols << ", nrows = " << nrows << " and nbands = " << nbands << endl;
  }
  else
  {
    cout << "Input image " << params.input_image_file << " has only one spectral band." << endl;
    cout << "At least two spectral bands are required for Principal Components Analysis." << endl;
    cout << "Program exiting." << endl;
    return false;
  }

 // mask and input_image
  unsigned char *mask = new unsigned char[ncols*nrows];
  for (row = 0; row < nrows; row++)
    for (col = 0; col < ncols; col++)
    {
      index = col + row*ncols;
      mask[index] = 1;
    }
  float *input_image = new float[ncols*nrows*nbands];
  bool mask_flag = false;
  for (band = 1; band <= nbands; band++)
  {
    GDALRasterBand *rb = inDataset->GetRasterBand(band);
    index = (band - 1)*ncols*nrows;
    rb->RasterIO(GF_Read, 0, 0, ncols, nrows, &input_image[index], ncols, nrows, GDT_Float32, 0, 0);

    has_fill = false;
    fill_value = rb->GetNoDataValue(&has_fill);
    if (has_fill)
    {
      for (row = 0; row < nrows; row++)
        for (col = 0; col < ncols; col++)
        {
          index = col + row*ncols + (band-1)*ncols*nrows;
          if (input_image[index] == fill_value)
          {
            index = col + row*ncols;
            mask[index] = 0;
            mask_flag = true;
          }
        }
    }
  }
  int mask_count = ncols*nrows;
  if (mask_flag)
  {
    mask_count = 0;
    for (row = 0; row < nrows; row++)
      for (col = 0; col < ncols; col++)
      {
        index = col + row*ncols;
        if (mask[index] == 1)
          mask_count++;
      }
  }

  float *masked_image = new float[mask_count*nbands];
  mask_index = 0;
  for (band = 1; band <= nbands; band++)
  {
    for (row = 0; row < nrows; row++)
      for (col = 0; col < ncols; col++)
      {
        index = col + row*ncols;
        if (mask[index] == 1)
        {
          index = col + row*ncols + (band-1)*nrows*ncols;
          masked_image[mask_index] = input_image[index];
          mask_index++;
        }
      }
  }

  Mat data(nbands,mask_count,CV_32FC1,&masked_image[0]);

  // perform PCA
  PCA pca;
  if (params.number_flag)
    pca(data, cv::Mat(), CV_PCA_DATA_AS_ROW, params.number_components);
  else
    pca(data, cv::Mat(), CV_PCA_DATA_AS_ROW, params.variance);

  int out_nbands = (int) pca.eigenvectors.rows;
  char **papszOptions = NULL;
  outDataset = driver->Create(params.output_image_file.c_str(), ncols, nrows, out_nbands, GDT_Float32, papszOptions);
  char **metadata = inDataset->GetMetadata("");
  outDataset->SetMetadata(metadata,"");
  string projection_type = inDataset->GetProjectionRef();
  if ((projection_type != "") && ((projection_type.find("Unknown") == string::npos) || (projection_type.size() > 10)))
    outDataset->SetProjection( projection_type.c_str());
  double imageGeoTransform[6];
  if ( inDataset->GetGeoTransform( imageGeoTransform ) == CE_None )
    outDataset->SetGeoTransform( imageGeoTransform);

cout << (params.variance*100) << " percent variance retained in " << out_nbands << " bands" << endl;

  float *output_image = new float[ncols*nrows];
  float *output_masked_image = new float[mask_count];
  for (band = 1; band <= out_nbands; band++)
  {
cout << "Outputing PCA band " << band << endl;
    Mat point = pca.project(data.row(band-1)); // project into the eigenspace, thus the image becomes a "point"
    Mat reconstruction = pca.backProject(point); // re-create the image from the "point"
    output_masked_image = (float *) &reconstruction.data[0];

    mask_index = 0;
    for (row = 0; row < nrows; row++)
      for (col = 0; col < ncols; col++)
      {
        index = col + row*ncols;
        if (mask[index] == 1)
        {
          output_image[index] = output_masked_image[mask_index];
          mask_index++;
        }
        else
          output_image[index] = fill_value;
      }

    GDALRasterBand *wb = outDataset->GetRasterBand(band);
    wb->RasterIO(GF_Write, 0, 0, ncols, nrows, &output_image[0], ncols, nrows, GDT_Float32, 0, 0);
  }

  GDALClose( (GDALDatasetH) inDataset);
  GDALClose( (GDALDatasetH) outDataset);

  return true;
}
