#include "gdal.h"
#include "cpl_conv.h"
#include "cpl_string.h"
#include "ogr_srs_api.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
int main(int Argc, char *Argv[]) {
	GDALAllRegister();
	GDALDatasetH  inputDataset = GDALOpen(Argv[1], GA_ReadOnly);
	int lower_limit = atoi(Argv[2]); /*Lowever limit for valid value, closed interval*/
	int upper_limt = atoi(Argv[3]); /*Upper limit for valid value, closed interval*/
	int cutting = atoi(Argv[4]); /*Flag for cutting boundaries: 1-true, 0-false*/
	int gapfill = atoi(Argv[5]); /*0: no gapfilling, >0: window width for gapfilling*/
	GDALRasterBandH Band;
	double GeoTransform[6];
	int64_t xsize = GDALGetRasterXSize(inputDataset);
	int64_t ysize = GDALGetRasterYSize(inputDataset);
	int64_t bands = GDALGetRasterCount(inputDataset);
	int64_t size = xsize*ysize;
	GDALGetGeoTransform(inputDataset,GeoTransform);
	char PROJCS[2000];
	strcpy(PROJCS,GDALGetProjectionRef(inputDataset));
    int64_t i,j,k,x,y,m,n;
    Band = GDALGetRasterBand(inputDataset, 1);
	GDALDataType data_type = GDALGetRasterDataType(Band);
	uint8_t *mask = calloc(size,sizeof(uint8_t));
	if (data_type == GDT_Byte) {
		uint8_t *image_band = malloc(size*sizeof(uint8_t));
		for (k=0;k<bands;k++) {
			Band = GDALGetRasterBand(inputDataset, k+1);
			GDALRasterIO(Band,GF_Read,0,0,xsize,ysize,image_band,xsize,ysize,data_type,0,0);
			for (i=0;i<size;i++) {
				if (image_band[i] >= lower_limit && image_band[i] <= upper_limt) mask[i] = 1;
			}
		}
		free(image_band);
	}
	else if (data_type == GDT_UInt16) {
		uint16_t *image_band = malloc(size*sizeof(uint16_t));
		for (k=0;k<bands;k++) {
			Band = GDALGetRasterBand(inputDataset, k+1);
			GDALRasterIO(Band,GF_Read,0,0,xsize,ysize,image_band,xsize,ysize,data_type,0,0);
			for (i=0;i<size;i++) {
				if (image_band[i] >= lower_limit && image_band[i] <= upper_limt) mask[i] = 1;
			}
		}
		free(image_band);
	}
	else if (data_type == GDT_Int16) {
		int16_t *image_band = malloc(size*sizeof(int16_t));
		for (k=0;k<bands;k++) {
			Band = GDALGetRasterBand(inputDataset, k+1);
			GDALRasterIO(Band,GF_Read,0,0,xsize,ysize,image_band,xsize,ysize,data_type,0,0);
			for (i=0;i<size;i++) {
				if (image_band[i] >= lower_limit && image_band[i] <= upper_limt) mask[i] = 1;
			}
		}
		free(image_band);
	}
	else if (data_type == GDT_UInt32) {
		uint32_t *image_band = malloc(size*sizeof(uint32_t));
		for (k=0;k<bands;k++) {
			Band = GDALGetRasterBand(inputDataset, k+1);
			GDALRasterIO(Band,GF_Read,0,0,xsize,ysize,image_band,xsize,ysize,data_type,0,0);
			for (i=0;i<size;i++) {
				if (image_band[i] >= lower_limit && image_band[i] <= upper_limt) mask[i] = 1;
			}
		}
		free(image_band);
	}
	else if (data_type ==  GDT_Int32) {
		int32_t *image_band = malloc(size*sizeof(int32_t));
		for (k=0;k<bands;k++) {
			Band = GDALGetRasterBand(inputDataset, k+1);
			GDALRasterIO(Band,GF_Read,0,0,xsize,ysize,image_band,xsize,ysize,data_type,0,0);
			for (i=0;i<size;i++) {
				if (image_band[i] >= lower_limit && image_band[i] <= upper_limt) mask[i] = 1;
			}
		}
		free(image_band);
	}
	else if (data_type ==  GDT_Float32) {
		float *image_band = malloc(size*sizeof(float));
		for (k=0;k<bands;k++) {
			Band = GDALGetRasterBand(inputDataset, k+1);
			GDALRasterIO(Band,GF_Read,0,0,xsize,ysize,image_band,xsize,ysize,data_type,0,0);
			for (i=0;i<size;i++) {
				if (image_band[i] >= lower_limit && image_band[i] <= upper_limt) mask[i] = 1;
			}
		}
		free(image_band);
	}
	else if (data_type == GDT_Float64) {
		double *image_band = malloc(size*sizeof(double));
		for (k=0;k<bands;k++) {
			Band = GDALGetRasterBand(inputDataset, k+1);
			GDALRasterIO(Band,GF_Read,0,0,xsize,ysize,image_band,xsize,ysize,data_type,0,0);
			for (i=0;i<size;i++) {
				if (image_band[i] >= lower_limit && image_band[i] <= upper_limt) mask[i] = 1;
			}
		}
		free(image_band);
	}
	else {
		printf("Unsupported data type!\n");
		return -1;
	}
	if (cutting == 1) {
		/* Cutting off Edges*/
		int64_t *left = malloc(ysize*sizeof(int64_t));
		int64_t *right = malloc(ysize*sizeof(int64_t));
		int64_t ltx[10000],lty[10000];
		int64_t lbx[10000],lby[10000];
		int64_t rtx[10000],rty[10000];
		int64_t rbx[10000],rby[10000];
		int64_t ltn,lbn,rtn,rbn;
		int64_t lx, ly, rx, ry, topy,btmy;
		lx = xsize;
		rx = 0;
		for(y=0;y<ysize;y++) {
			for (x=0;x<xsize;x++) {
				if (mask[y*xsize+x] == 1) {
					left[y] = x;
					break;
				}
				else {
					left[y] = -1;
				}
			}
			for (x=xsize-1;x>=0;x--) {
				if (mask[y*xsize+x] == 1) {
					right[y] = x;
					break;
				}
				else {
					right[y] = -1;
				}
			}
		}
		for(y=0;y<ysize;y++) {
			if (left[y] > 0) {
				topy = y;
				break;
			}
		}
		for(y=ysize-1;y>=0;y--) {
			if (left[y] > 0) {
				btmy = y;
				break;
			}
		}
		topy = ((int64_t)(topy/100)+1)*100;
		btmy = ((int64_t)((ysize-1-btmy)/100)+1)*100;
		for(y=0;y<ysize;y++) {
			if (left[y] < lx && left[y] >= 0) {
				lx = left[y];
				ly = y;
			}
			if (right[y] > rx && right[y] >= 0) {
				rx = right[y];
				ry = y;
			}
		}

		ltx[0] = left[topy];
		lty[0] = topy;
		ltn = 0;
		rtx[0] = right[topy];
		rty[0] = topy;
		rtn = 0;
		lbx[0] = left[ysize-btmy];
		lby[0] = ysize-btmy;
		lbn = 0;
		rbx[0] = right[ysize-btmy];
		rby[0] = ysize-btmy;
		rbn = 0;
		for (y=topy+1;y<ly-topy;y++) {
			if (left[y]<ltx[ltn] && left[y] >= 0) {
				ltn += 1;
				ltx[ltn] = left[y];
				lty[ltn] = y;
			}
		}

		for (y=topy+1;y<ry-topy;y++) {
			if (right[y]>rtx[rtn] && right[y] >= 0) {
				rtn += 1;
				rtx[rtn] = right[y];
				rty[rtn] = y;
			}
		}

		for (y=ysize-btmy-1;y>ly+btmy;y--) {
			if (left[y]<lbx[lbn] && left[y] >= 0) {
				lbn += 1;
				lbx[lbn] = left[y];
				lby[lbn] = y;
			}
		}

		for (y=ysize-btmy-1;y>ry+btmy;y--) {
			if (right[y]>rbx[rbn] && right[y] >= 0) {
				rbn += 1;
				rbx[rbn] = right[y];
				rby[rbn] = y;
			}
		}

		/* Fitting boundaries*/
		float ltk,ltb,lbk,lbb,rtk,rtb,rbk,rbb;
		float sum_xy,sum_x,sum_y,sum_xx;
		sum_xy = 0.0;
		sum_x = 0.0;
		sum_y = 0.0;
		sum_xx = 0.0;
		for (i=1;i<=ltn;i++) {
			sum_xy += 1.0*ltx[i]*lty[i];
			sum_x += 1.0*ltx[i];
			sum_y += 1.0*lty[i];
			sum_xx += 1.0*ltx[i]*ltx[i];
		}
		ltk = 1.0*(ltn*sum_xy-sum_x*sum_y)/(ltn*sum_xx-sum_x*sum_x);
		ltb = 1.0*(sum_y-ltk*sum_x)/ltn;

		sum_xy = 0.0;
		sum_x = 0.0;
		sum_y = 0.0;
		sum_xx = 0.0;
		for (i=1;i<=lbn;i++) {
			sum_xy += 1.0*lbx[i]*lby[i];
			sum_x += 1.0*lbx[i];
			sum_y += 1.0*lby[i];
			sum_xx += 1.0*lbx[i]*lbx[i];
		}
		lbk = 1.0*(lbn*sum_xy-sum_x*sum_y)/(lbn*sum_xx-sum_x*sum_x);
		lbb = 1.0*(sum_y-lbk*sum_x)/lbn;

		sum_xy = 0.0;
		sum_x = 0.0;
		sum_y = 0.0;
		sum_xx = 0.0;
		for (i=1;i<=rtn;i++) {
			sum_xy += 1.0*rtx[i]*rty[i];
			sum_x += 1.0*rtx[i];
			sum_y += 1.0*rty[i];
			sum_xx += 1.0*rtx[i]*rtx[i];
		}
		rtk = 1.0*(rtn*sum_xy-sum_x*sum_y)/(rtn*sum_xx-sum_x*sum_x);
		rtb = 1.0*(sum_y-rtk*sum_x)/rtn;

		sum_xy = 0.0;
		sum_x = 0.0;
		sum_y = 0.0;
		sum_xx = 0.0;
		for (i=1;i<=rbn;i++) {
			sum_xy += 1.0*rbx[i]*rby[i];
			sum_x += 1.0*rbx[i];
			sum_y += 1.0*rby[i];
			sum_xx += 1.0*rbx[i]*rbx[i];
		}
		rbk = 1.0*(rbn*sum_xy-sum_x*sum_y)/(rbn*sum_xx-sum_x*sum_x);
		rbb = 1.0*(sum_y-rbk*sum_x)/rbn;

		int64_t dist14 = (int64_t) (0.02*abs((ltb-rbb)/ltk));
		int64_t dist23 = (int64_t)(0.02*abs((rtb-lbb)/lbk));
		for (x=0;x<xsize;x++) {
			for (y=0;y<ysize;y++) {
				float cond1 = y-ltk*(x-dist14)-ltb;
				float cond2 = y-lbk*(x-dist23)-lbb;
				float cond3 = y-rtk*(x+dist23)-rtb;
				float cond4 = y-rbk*(x+dist14)-rbb;
				if (cond1>0 && cond2<0 && cond3>0 && cond4<0) {
					continue;
				}
				else {
					mask[y*xsize+x] = 0;
				}
			}
		}
	}

	if (gapfill > 0) {
		uint8_t *mask_expand = malloc(size*sizeof(uint8_t));
		int64_t x_grids = xsize/gapfill+1;
		int64_t y_grids = ysize/gapfill+1;
		int64_t grid_size = x_grids*y_grids;
		int64_t x_grid,y_grid;
		uint8_t *grid_mask = calloc(grid_size,sizeof(uint8_t));
		for (i=0;i<grid_size;i++) {
			y = i/x_grids;
			x = i%x_grids;
			int continue_search = 1;
			for (m=x*gapfill;(m<MIN((x+1)*gapfill,xsize))&&continue_search;m++) {
				for (n=y*gapfill;(n<MIN((y+1)*gapfill,ysize))&&continue_search;n++) {
					if (mask[m+n*xsize] == 1) {
						grid_mask[i] = 1;
						continue_search = 0;
					}
				}
			}
		}
		uint8_t *grid_mask_erosion = malloc(grid_size*sizeof(uint8_t));
		for (i=0;i<grid_size;i++) {
			grid_mask_erosion[i] = grid_mask[i];
		}
		for (i=0;i<grid_size;i++) {
			y = i/x_grids;
			x = i%x_grids;
			int continue_search = 1;
			if (grid_mask[i] == 1) {
				for (m=MAX(0,x-1);(m<MIN(x_grids,x+1))&&continue_search;m++) {
					for (n=MAX(0,y-1);(n<MIN(y_grids,y+1))&&continue_search;n++) {
						if (grid_mask[m+n*x_grids] == 0) {
							grid_mask_erosion[i] = 0;
							continue_search = 0;
						}
					}
				}
			}
		}
		for (i=0;i<size;i++) {
			x = i%xsize;
			y = i/xsize;
			x_grid = x/gapfill;
			y_grid = y/gapfill;
			mask[i] = grid_mask_erosion[x_grid+y_grid*x_grids];
		}
	}

	char **options = NULL;
	options = CSLSetNameValue( options, "TILED", "YES" );
	options = CSLSetNameValue( options, "COMPRESS", "LZW" );
	GDALDriverH hDriver = GDALGetDriverByName("GTiff");
	GDALDatasetH outDataset = GDALCreate(hDriver,Argv[6], xsize, ysize, 1, GDT_Byte, options);
	GDALSetGeoTransform(outDataset, GeoTransform);
 	GDALSetProjection(outDataset, PROJCS);
	GDALRasterBandH outBand;
	outBand = GDALGetRasterBand(outDataset,1);
	GDALRasterIO(outBand,GF_Write,0,0,xsize,ysize,mask,xsize, ysize,GDT_Byte,0,0);
	GDALClose(outDataset);
    return 0;
}
