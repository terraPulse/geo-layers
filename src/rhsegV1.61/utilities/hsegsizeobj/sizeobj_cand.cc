#include "gdal_priv.h"
#include "cpl_conv.h"
#include "cpl_string.h"
#include <ogr_spatialref.h>
#include "params/initialParams.h"
#include "params/params.h"
#include <region/region_class.h>
#include <region/region_object.h>
#include <results/results.h>
#include <string>
#include <iostream>

using namespace std;

extern HSEGTilton::InitialParams initialParams;
extern HSEGTilton::Params params;
extern HSEGTilton::oParams oparams;

namespace HSEGTilton {
	void sizeobj() {
        int nb_classes = oparams.level0_nb_classes;
        RegionClass::set_static_vals();
        int region_classes_size = nb_classes;
        vector<RegionClass> region_classes(region_classes_size);
        int nb_objects = 1;
        if (params.region_nb_objects_flag)
            nb_objects = oparams.level0_nb_objects;
        RegionObject::set_static_vals(false,false);
        int region_objects_size = nb_objects;
        vector<RegionObject> region_objects(region_objects_size);
        if (!params.region_nb_objects_flag)
            nb_objects = 0;
        Results results_data;
        results_data.set_buffer_sizes(params.nbands,nb_classes,nb_objects);
        results_data.open_input(params.region_classes_file,params.region_objects_file);
        unsigned int region_label, region_index, region_npix;
        GDALDataset    *objInDataset, *maskDataset;
        GDALDriver     *driver;
        GDALRasterBand *objInBand,    *maskBand;
        objInDataset = (GDALDataset *)GDALOpen(params.object_labels_map_file.c_str(), GA_ReadOnly);
        maskDataset = (GDALDataset *)GDALOpen(params.mask_file.c_str(), GA_ReadOnly);
        driver = objInDataset->GetDriver();
        double GeoTransform[6];
        maskDataset->GetGeoTransform(GeoTransform);
        OGRSpatialReference *ref = new OGRSpatialReference(NULL);
        const char* PROJCS = objInDataset->GetProjectionRef();
        char* copy = strdup(PROJCS);
        ref->importFromWkt(&copy);
        int northflag;
        const int utmzone = ref->GetUTMZone(&northflag);
        int utmcode = 0;
        if (northflag) {
            utmcode = 32600+utmzone;
        }
        else {
            utmcode = 32700+utmzone;
        }
        ref->importFromEPSG(utmcode);
        char *NEWPROJCS = NULL;
        ref->exportToWkt(&NEWPROJCS);
        unsigned int xsize = maskDataset->GetRasterXSize();
        unsigned int ysize = maskDataset->GetRasterYSize();
        unsigned int size = xsize*ysize;
        int i,k;
        unsigned int j;
        const unsigned int levels = initialParams.levels;
        /*size thresholds*/
        const unsigned int sizethres1 = initialParams.sizethres1;
        const unsigned int sizethres2 = initialParams.sizethres2;
        const unsigned int sizethres3 = initialParams.sizethres3;
        /*read object_labels_map */
        objInBand = objInDataset->GetRasterBand(1);
        unsigned int *object_labels_map = new unsigned int[size];
        objInBand->RasterIO(GF_Read,0,0,xsize,ysize,&object_labels_map[0],xsize,ysize,GDT_UInt32,0,0);
        /*read mask band*/
        maskBand = maskDataset->GetRasterBand(1);
        unsigned char *mask = new unsigned char[size];
        maskBand->RasterIO(GF_Read,0,0,xsize,ysize,&mask[0],xsize,ysize,GDT_Byte,0,0);
        /*set up buffer*/
		unsigned int **sobj = new unsigned int*[3];
        unsigned int **sobjnpix = new unsigned int*[3];
		unsigned char **stopmerge = new unsigned char*[3];
		float *thres1 = new float[nb_objects];
		float *thres2 = new float[nb_objects];
		float *thres3 = new float[nb_objects];
        sobj[0] = new unsigned int[size];
        sobj[1] = new unsigned int[size];
        sobj[2] = new unsigned int[size];
        sobjnpix[0] = new unsigned int[size];
        sobjnpix[1] = new unsigned int[size];
        sobjnpix[2] = new unsigned int[size];
		stopmerge[0] = new unsigned char[size];
        stopmerge[1] = new unsigned char[size];
        stopmerge[2] = new unsigned char[size];
        for (j=0;j<size;j++) {
            sobj[0][j] = 0;
			sobj[1][j] = 0;
			sobj[2][j] = 0;
            sobjnpix[0][j] = 0;
			sobjnpix[1][j] = 0;
			sobjnpix[2][j] = 0;
			stopmerge[0][j] = 0;
			stopmerge[1][j] = 0;
			stopmerge[2][j] = 0;
        }
		unsigned int *label = new unsigned int[size];
		unsigned int *npix = new unsigned int[size];
		unsigned int *npix_prev = new unsigned int[size];
		for (i=0;i<oparams.nb_levels;i++) {
			cout << "Processing hierarchical level " << i << endl;
			results_data.read(i,nb_classes,nb_objects,oparams.nb_levels,oparams.int_buffer_size,region_classes,region_objects);
			for (k=0;k<nb_objects;k++) {
					thres1[k] = 0.f;
					thres2[k] = 0.f;
					thres3[k] = 0.f;
			}
			for (j=0;j<size;j++) {
				if (mask[j] == 1) {
					region_label = object_labels_map[j];
					region_index = region_label - 1;
					if (!region_objects[region_index].get_active_flag()) {
						region_label = region_objects[region_index].get_merge_region_label();
						region_index = region_label - 1;
					}
					region_npix = region_objects[region_index].get_npix();
					label[j] = region_label;
					npix[j] = region_npix;
					if (i == 0) {
						sobj[0][j] = label[j];
						sobjnpix[0][j] = npix[j];
						sobj[1][j] = label[j];
						sobjnpix[1][j] = npix[j];
						sobj[2][j] = label[j];
						sobjnpix[2][j] = npix[j];
					}
					else {
						for (j=0;j<size;j++) {
							if (stopmerge[0][j] == 0)
								thres1[label[j]-1] += fabs((float)npix_prev[j] - (float)sizethres1)/ (float)npix[j];
							if (stopmerge[1][j] == 0)
								thres2[label[j]-1] += fabs((float)npix_prev[j] - (float)sizethres2)/ (float)npix[j];
							if (stopmerge[2][j] == 0)
								thres3[label[j]-1] += fabs((float)npix_prev[j] - (float)sizethres3)/ (float)npix[j];
						}
					}
				}
			}
			if (i > 0) {
				for (j=0;j<size;j++) {
					if (mask[j] == 1) {
						if (stopmerge[0][j] == 0 && (fabs((float)npix[j] - (float)sizethres1) - thres1[label[j]-1]) < 0.1*((float)sizethres1)) {
							sobj[0][j] = label[j];
							sobjnpix[0][j] = npix[j];
						}
						else
							stopmerge[0][j] = 1;
						if (stopmerge[1][j] == 0 && (fabs((float)npix[j] - (float)sizethres2) - thres2[label[j]-1]) < 0.1*((float)sizethres2)) {
							sobj[1][j] = label[j];
							sobjnpix[1][j] = npix[j];
						}
						else
							stopmerge[1][j] = 1;
						if (stopmerge[2][j] == 0 && (fabs((float)npix[j] - (float)sizethres3) - thres3[label[j]-1]) < 0.1*((float)sizethres3)) {
							sobj[2][j] = label[j];
							sobjnpix[2][j] = npix[j];
						}
						else
							stopmerge[2][j] = 1;
					}
				}
			}
			memcpy(npix_prev,npix,sizeof(unsigned int)*size);
		}
        results_data.close_input();	
        GDALDataset    *npixOutDataset, *objOutDataset;
        GDALRasterBand *npixOutBand,    *objOutBand;
        char **papszOptions = NULL;
		papszOptions = CSLSetNameValue( papszOptions, "TILED", "YES" );
		papszOptions = CSLSetNameValue( papszOptions, "COMPRESS", "LZW" );
        npixOutDataset = driver->Create(initialParams.sizenpix_file.c_str(), xsize, ysize, levels, GDT_UInt32, papszOptions);
        objOutDataset =  driver->Create(initialParams.sizeobj_file.c_str(), xsize, ysize, levels, GDT_UInt32, papszOptions);
        npixOutDataset->SetGeoTransform(GeoTransform);
        npixOutDataset->SetProjection(NEWPROJCS);
        objOutDataset->SetGeoTransform(GeoTransform);
        objOutDataset->SetProjection(NEWPROJCS);
        for(j=0;j<levels;j++) {
            objOutBand = objOutDataset->GetRasterBand(j+1);
            objOutBand->RasterIO(GF_Write,0,0,xsize,ysize,sobj[j],xsize,ysize,GDT_UInt32,0,0);
        }
        GDALClose( (GDALDatasetH) objOutDataset);
        for(j=0;j<levels;j++) {
            npixOutBand = npixOutDataset->GetRasterBand(j+1);
            npixOutBand->RasterIO(GF_Write,0,0,xsize,ysize,sobjnpix[j],xsize,ysize,GDT_UInt32,0,0);
        }
        GDALClose( (GDALDatasetH) npixOutDataset);
        GDALClose( (GDALDatasetH) objInDataset);
        GDALClose( (GDALDatasetH) maskDataset);
        return;
    }
} // namespace HSEGTilton