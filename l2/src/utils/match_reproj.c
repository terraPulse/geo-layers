#include "gdal.h"
#include "cpl_conv.h"
#include "cpl_string.h"
#include "ogr_srs_api.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int main(int Argc, char *Argv[]) {
	GDALAllRegister();
	GDALDatasetH  targetDataset = GDALOpen(Argv[1], GA_ReadOnly);
	GDALDatasetH  reprojDataset = GDALOpen(Argv[2], GA_ReadOnly);
	GDALRasterBandH Band;
	int nodata = atoi(Argv[3]);
	int forward_mapping = atoi(Argv[4]);
	int coordinate_transform = atoi(Argv[5]);
	int64_t xsize = GDALGetRasterXSize(targetDataset);
	int64_t ysize = GDALGetRasterYSize(targetDataset);
	int64_t size = xsize*ysize;
	double GeoTransform[6];
	GDALGetGeoTransform(targetDataset,GeoTransform);
	OGRSpatialReferenceH ref = OSRNewSpatialReference(NULL);
	const char *PROJCS = GDALGetProjectionRef(targetDataset);
	char *copy = strdup(PROJCS);
	OSRImportFromWkt(ref,&copy);

	int64_t rxsize = GDALGetRasterXSize(reprojDataset);
	int64_t rysize = GDALGetRasterYSize(reprojDataset);
	int64_t rsize = rxsize*rysize;
	Band = GDALGetRasterBand(reprojDataset, 1);
	GDALDataType data_type = GDALGetRasterDataType(Band);
	double rGeoTransform[6];
	double rInvTransform[6];
	double InvTransform[6];
	GDALGetGeoTransform(reprojDataset,rGeoTransform);
	if( !GDALInvGeoTransform(rGeoTransform, rInvTransform ) ) {
		CPLError(CE_Failure, CPLE_AppDefined, "Cannot invert geotransform");
		return 0;
	}
	if( !GDALInvGeoTransform(GeoTransform, InvTransform ) ) {
		CPLError(CE_Failure, CPLE_AppDefined, "Cannot invert geotransform");
		return 0;
	}
	OGRSpatialReferenceH rref = OSRNewSpatialReference(NULL);
	const char *rPROJCS = GDALGetProjectionRef(reprojDataset);
	char *rcopy = strdup(rPROJCS);
	OSRImportFromWkt(rref,&rcopy);
	OGRCoordinateTransformationH rCT = OCTNewCoordinateTransformation(ref, rref);
	OGRCoordinateTransformationH CT = OCTNewCoordinateTransformation(rref, ref);
    int64_t i;

    char **options = NULL;
	options = CSLSetNameValue( options, "TILED", "YES" );
	options = CSLSetNameValue( options, "COMPRESS", "LZW" );
	GDALDriverH hDriver = GDALGetDriverByName("GTiff");
	GDALRasterBandH outBand;
	GDALDatasetH outDataset = GDALCreate(hDriver,Argv[6], xsize, ysize,1, data_type, options);
	GDALSetGeoTransform(outDataset, GeoTransform);
 	GDALSetProjection(outDataset, PROJCS);
	outBand = GDALGetRasterBand(outDataset,1);
	GDALSetRasterNoDataValue(outBand,nodata);

	Band = GDALGetRasterBand(reprojDataset, 1);

    if (data_type == GDT_Byte) {
		uint8_t *reproject = (uint8_t *)malloc(rsize*sizeof(uint8_t));
		uint8_t *reprojected = (uint8_t *)malloc(size*sizeof(uint8_t));
		GDALRasterIO(Band,GF_Read,0,0,rxsize,rysize,reproject,rxsize,rysize,data_type,0,0);
		if (forward_mapping == 1) {
			for (i=0;i<size;i++) {
				reprojected[i] = nodata;
			}
			for (i=0;i<rsize;i++) {
				int64_t ry = i/rxsize;
				int64_t rx = i%rxsize;
				double rxgeo,rygeo,newx,newy;
				int64_t indx, indy;
				double z = 0.0;
				GDALApplyGeoTransform(rGeoTransform, rx, ry, &rxgeo, &rygeo);
				if (coordinate_transform == 1) OCTTransform(CT, 1, &rxgeo, &rygeo, &z );
				GDALApplyGeoTransform(InvTransform, rxgeo, rygeo, &newx, &newy);
				indy = (int) (newy);
				indx = (int) (newx);
				if ((indy >= 0) && (indy < ysize) && (indx >= 0) && (indx < xsize)) {
					reprojected[indy*xsize+indx] = reproject[i];
				}
			}
		}
		else {
			for (i=0;i<size;i++) {
				int64_t y = i/xsize;
				int64_t x = i%xsize;
				double xgeo,ygeo,newx,newy;
				int64_t indx, indy;
				double z = 0.0;
				GDALApplyGeoTransform(GeoTransform, x, y, &xgeo, &ygeo);
				if (coordinate_transform == 1) OCTTransform(rCT, 1, &xgeo, &ygeo, &z );
				GDALApplyGeoTransform(rInvTransform, xgeo, ygeo, &newx, &newy);
				indy = (int) (newy);
				indx = (int) (newx);
				if ((indy >= 0) && (indy < rysize) && (indx >= 0) && (indx < rxsize)) {
					reprojected[i] = reproject[indy*rxsize+indx];
				}
				else
					reprojected[i] = nodata;
			}
		}
		GDALRasterIO(outBand,GF_Write,0,0,xsize,ysize,reprojected,xsize,ysize,data_type,0,0);
	}
	else if (data_type == GDT_UInt16) {
		uint16_t *reproject = (uint16_t *)malloc(rsize*sizeof(uint16_t));
		uint16_t *reprojected = (uint16_t *)malloc(size*sizeof(uint16_t));
		GDALRasterIO(Band,GF_Read,0,0,rxsize,rysize,reproject,rxsize,rysize,data_type,0,0);
		if (forward_mapping == 1) {
			for (i=0;i<size;i++) {
				reprojected[i] = nodata;
			}
			for (i=0;i<rsize;i++) {
				int64_t ry = i/rxsize;
				int64_t rx = i%rxsize;
				double rxgeo,rygeo,newx,newy;
				int64_t indx, indy;
				double z = 0.0;
				GDALApplyGeoTransform(rGeoTransform, rx, ry, &rxgeo, &rygeo);
				if (coordinate_transform == 1) OCTTransform(CT, 1, &rxgeo, &rygeo, &z );
				GDALApplyGeoTransform(InvTransform, rxgeo, rygeo, &newx, &newy);
				indy = (int) (newy);
				indx = (int) (newx);
				if ((indy >= 0) && (indy < ysize) && (indx >= 0) && (indx < xsize)) {
					reprojected[indy*xsize+indx] = reproject[i];
				}
			}
		}
		else {
			for (i=0;i<size;i++) {
				int64_t y = i/xsize;
				int64_t x = i%xsize;
				double xgeo,ygeo,newx,newy;
				int64_t indx, indy;
				double z = 0.0;
				GDALApplyGeoTransform(GeoTransform, x, y, &xgeo, &ygeo);
				if (coordinate_transform == 1) OCTTransform(rCT, 1, &xgeo, &ygeo, &z );
				GDALApplyGeoTransform(rInvTransform, xgeo, ygeo, &newx, &newy);
				indy = (int) (newy);
				indx = (int) (newx);
				if ((indy >= 0) && (indy < rysize) && (indx >= 0) && (indx < rxsize)) {
					reprojected[i] = reproject[indy*rxsize+indx];
				}
				else
					reprojected[i] = nodata;
			}
		}
		GDALRasterIO(outBand,GF_Write,0,0,xsize,ysize,reprojected,xsize,ysize,data_type,0,0);
	}
	else if (data_type == GDT_Int16) {
		int16_t *reproject = (int16_t *)malloc(rsize*sizeof(int16_t));
		int16_t *reprojected = (int16_t *)malloc(size*sizeof(int16_t));
		GDALRasterIO(Band,GF_Read,0,0,rxsize,rysize,reproject,rxsize,rysize,data_type,0,0);
		if (forward_mapping == 1) {
			for (i=0;i<size;i++) {
				reprojected[i] = nodata;
			}
			for (i=0;i<rsize;i++) {
				int64_t ry = i/rxsize;
				int64_t rx = i%rxsize;
				double rxgeo,rygeo,newx,newy;
				int64_t indx, indy;
				double z = 0.0;
				GDALApplyGeoTransform(rGeoTransform, rx, ry, &rxgeo, &rygeo);
				if (coordinate_transform == 1) OCTTransform(CT, 1, &rxgeo, &rygeo, &z );
				GDALApplyGeoTransform(InvTransform, rxgeo, rygeo, &newx, &newy);
				indy = (int) (newy);
				indx = (int) (newx);
				if ((indy >= 0) && (indy < ysize) && (indx >= 0) && (indx < xsize)) {
					reprojected[indy*xsize+indx] = reproject[i];
				}
			}
		}
		else {
			for (i=0;i<size;i++) {
				int64_t y = i/xsize;
				int64_t x = i%xsize;
				double xgeo,ygeo,newx,newy;
				int64_t indx, indy;
				double z = 0.0;
				GDALApplyGeoTransform(GeoTransform, x, y, &xgeo, &ygeo);
				if (coordinate_transform == 1) OCTTransform(rCT, 1, &xgeo, &ygeo, &z );
				GDALApplyGeoTransform(rInvTransform, xgeo, ygeo, &newx, &newy);
				indy = (int) (newy);
				indx = (int) (newx);
				if ((indy >= 0) && (indy < rysize) && (indx >= 0) && (indx < rxsize)) {
					reprojected[i] = reproject[indy*rxsize+indx];
				}
				else
					reprojected[i] = nodata;
			}
		}
		GDALRasterIO(outBand,GF_Write,0,0,xsize,ysize,reprojected,xsize,ysize,data_type,0,0);
	}
	else if (data_type == GDT_UInt32) {
		uint32_t *reproject = (uint32_t *)malloc(rsize*sizeof(uint32_t));
		uint32_t *reprojected = (uint32_t *)malloc(size*sizeof(uint32_t));
		GDALRasterIO(Band,GF_Read,0,0,rxsize,rysize,reproject,rxsize,rysize,data_type,0,0);
		if (forward_mapping == 1) {
			for (i=0;i<size;i++) {
				reprojected[i] = nodata;
			}
			for (i=0;i<rsize;i++) {
				int64_t ry = i/rxsize;
				int64_t rx = i%rxsize;
				double rxgeo,rygeo,newx,newy;
				int64_t indx, indy;
				double z = 0.0;
				GDALApplyGeoTransform(rGeoTransform, rx, ry, &rxgeo, &rygeo);
				if (coordinate_transform == 1) OCTTransform(CT, 1, &rxgeo, &rygeo, &z );
				GDALApplyGeoTransform(InvTransform, rxgeo, rygeo, &newx, &newy);
				indy = (int) (newy);
				indx = (int) (newx);
				if ((indy >= 0) && (indy < ysize) && (indx >= 0) && (indx < xsize)) {
					reprojected[indy*xsize+indx] = reproject[i];
				}
			}
		}
		else {
			for (i=0;i<size;i++) {
				int64_t y = i/xsize;
				int64_t x = i%xsize;
				double xgeo,ygeo,newx,newy;
				int64_t indx, indy;
				double z = 0.0;
				GDALApplyGeoTransform(GeoTransform, x, y, &xgeo, &ygeo);
				if (coordinate_transform == 1) OCTTransform(rCT, 1, &xgeo, &ygeo, &z );
				GDALApplyGeoTransform(rInvTransform, xgeo, ygeo, &newx, &newy);
				indy = (int) (newy);
				indx = (int) (newx);
				if ((indy >= 0) && (indy < rysize) && (indx >= 0) && (indx < rxsize)) {
					reprojected[i] = reproject[indy*rxsize+indx];
				}
				else
					reprojected[i] = nodata;
			}
		}
		GDALRasterIO(outBand,GF_Write,0,0,xsize,ysize,reprojected,xsize,ysize,data_type,0,0);
	}
	else if (data_type ==  GDT_Int32) {
		int32_t *reproject = (int32_t *)malloc(rsize*sizeof(int32_t));
		int32_t *reprojected = (int32_t *)malloc(size*sizeof(int32_t));
		GDALRasterIO(Band,GF_Read,0,0,rxsize,rysize,reproject,rxsize,rysize,data_type,0,0);
		if (forward_mapping == 1) {
			for (i=0;i<size;i++) {
				reprojected[i] = nodata;
			}
			for (i=0;i<rsize;i++) {
				int64_t ry = i/rxsize;
				int64_t rx = i%rxsize;
				double rxgeo,rygeo,newx,newy;
				int64_t indx, indy;
				double z = 0.0;
				GDALApplyGeoTransform(rGeoTransform, rx, ry, &rxgeo, &rygeo);
				if (coordinate_transform == 1) OCTTransform(CT, 1, &rxgeo, &rygeo, &z );
				GDALApplyGeoTransform(InvTransform, rxgeo, rygeo, &newx, &newy);
				indy = (int) (newy);
				indx = (int) (newx);
				if ((indy >= 0) && (indy < ysize) && (indx >= 0) && (indx < xsize)) {
					reprojected[indy*xsize+indx] = reproject[i];
				}
			}
		}
		else {
			for (i=0;i<size;i++) {
				int64_t y = i/xsize;
				int64_t x = i%xsize;
				double xgeo,ygeo,newx,newy;
				int64_t indx, indy;
				double z = 0.0;
				GDALApplyGeoTransform(GeoTransform, x, y, &xgeo, &ygeo);
				if (coordinate_transform == 1) OCTTransform(rCT, 1, &xgeo, &ygeo, &z );
				GDALApplyGeoTransform(rInvTransform, xgeo, ygeo, &newx, &newy);
				indy = (int) (newy);
				indx = (int) (newx);
				if ((indy >= 0) && (indy < rysize) && (indx >= 0) && (indx < rxsize)) {
					reprojected[i] = reproject[indy*rxsize+indx];
				}
				else
					reprojected[i] = nodata;
			}
		}
		GDALRasterIO(outBand,GF_Write,0,0,xsize,ysize,reprojected,xsize,ysize,data_type,0,0);
	}
	else if (data_type ==  GDT_Float32) {
		float *reproject = (float *)malloc(rsize*sizeof(float));
		float *reprojected = (float *)malloc(size*sizeof(float));
		GDALRasterIO(Band,GF_Read,0,0,rxsize,rysize,reproject,rxsize,rysize,data_type,0,0);
		if (forward_mapping == 1) {
			for (i=0;i<size;i++) {
				reprojected[i] = nodata;
			}
			for (i=0;i<rsize;i++) {
				int64_t ry = i/rxsize;
				int64_t rx = i%rxsize;
				double rxgeo,rygeo,newx,newy;
				int64_t indx, indy;
				double z = 0.0;
				GDALApplyGeoTransform(rGeoTransform, rx, ry, &rxgeo, &rygeo);
				if (coordinate_transform == 1) OCTTransform(CT, 1, &rxgeo, &rygeo, &z );
				GDALApplyGeoTransform(InvTransform, rxgeo, rygeo, &newx, &newy);
				indy = (int) (newy);
				indx = (int) (newx);
				if ((indy >= 0) && (indy < ysize) && (indx >= 0) && (indx < xsize)) {
					reprojected[indy*xsize+indx] = reproject[i];
				}
			}
		}
		else {
			for (i=0;i<size;i++) {
				int64_t y = i/xsize;
				int64_t x = i%xsize;
				double xgeo,ygeo,newx,newy;
				int64_t indx, indy;
				double z = 0.0;
				GDALApplyGeoTransform(GeoTransform, x, y, &xgeo, &ygeo);
				if (coordinate_transform == 1) OCTTransform(rCT, 1, &xgeo, &ygeo, &z );
				GDALApplyGeoTransform(rInvTransform, xgeo, ygeo, &newx, &newy);
				indy = (int) (newy);
				indx = (int) (newx);
				if ((indy >= 0) && (indy < rysize) && (indx >= 0) && (indx < rxsize)) {
					reprojected[i] = reproject[indy*rxsize+indx];
				}
				else
					reprojected[i] = nodata;
			}
		}
		GDALRasterIO(outBand,GF_Write,0,0,xsize,ysize,reprojected,xsize,ysize,data_type,0,0);
	}
	else if (data_type == GDT_Float64) {
		double *reproject = (double *)malloc(rsize*sizeof(double));
		double *reprojected = (double *)malloc(size*sizeof(double));
		GDALRasterIO(Band,GF_Read,0,0,rxsize,rysize,reproject,rxsize,rysize,data_type,0,0);
		if (forward_mapping == 1) {
			for (i=0;i<size;i++) {
				reprojected[i] = nodata;
			}
			for (i=0;i<rsize;i++) {
				int64_t ry = i/rxsize;
				int64_t rx = i%rxsize;
				double rxgeo,rygeo,newx,newy;
				int64_t indx, indy;
				double z = 0.0;
				GDALApplyGeoTransform(rGeoTransform, rx, ry, &rxgeo, &rygeo);
				if (coordinate_transform == 1) OCTTransform(CT, 1, &rxgeo, &rygeo, &z );
				GDALApplyGeoTransform(InvTransform, rxgeo, rygeo, &newx, &newy);
				indy = (int) (newy);
				indx = (int) (newx);
				if ((indy >= 0) && (indy < ysize) && (indx >= 0) && (indx < xsize)) {
					reprojected[indy*xsize+indx] = reproject[i];
				}
			}
		}
		else {
			for (i=0;i<size;i++) {
				int64_t y = i/xsize;
				int64_t x = i%xsize;
				double xgeo,ygeo,newx,newy;
				int64_t indx, indy;
				double z = 0.0;
				GDALApplyGeoTransform(GeoTransform, x, y, &xgeo, &ygeo);
				if (coordinate_transform == 1) OCTTransform(rCT, 1, &xgeo, &ygeo, &z );
				GDALApplyGeoTransform(rInvTransform, xgeo, ygeo, &newx, &newy);
				indy = (int) (newy);
				indx = (int) (newx);
				if ((indy >= 0) && (indy < rysize) && (indx >= 0) && (indx < rxsize)) {
					reprojected[i] = reproject[indy*rxsize+indx];
				}
				else
					reprojected[i] = nodata;
			}
		}
		GDALRasterIO(outBand,GF_Write,0,0,xsize,ysize,reprojected,xsize,ysize,data_type,0,0);
	}
	else {
		printf("Unsupported data type!\n");
		return -1;
	}	
	
	GDALClose(outDataset);
	return 0;
}
