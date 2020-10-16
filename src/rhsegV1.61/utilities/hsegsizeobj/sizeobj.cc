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
        objInDataset = (GDALDataset *)GDALOpen(params.class_labels_map_file.c_str(), GA_ReadOnly);
        maskDataset = (GDALDataset *)GDALOpen(params.mask_file.c_str(), GA_ReadOnly);
        driver = objInDataset->GetDriver();
        double GeoTransform[6];
        maskDataset->GetGeoTransform(GeoTransform);
        const char* PROJCS = objInDataset->GetProjectionRef();
        unsigned int xsize = maskDataset->GetRasterXSize();
        unsigned int ysize = maskDataset->GetRasterYSize();
        unsigned int size = xsize*ysize;
        int i;
        unsigned int j,k;
        const unsigned int levels = initialParams.levels;
        /*size thresholds*/
        /*read object_labels_map */
        objInBand = objInDataset->GetRasterBand(1);
        unsigned int *class_labels_map = new unsigned int[size];
        objInBand->RasterIO(GF_Read,0,0,xsize,ysize,&class_labels_map[0],xsize,ysize,GDT_UInt32,0,0);
        /*read mask band*/
        maskBand = maskDataset->GetRasterBand(1);
        unsigned char *mask = new unsigned char[size];
        maskBand->RasterIO(GF_Read,0,0,xsize,ysize,&mask[0],xsize,ysize,GDT_Byte,0,0);
        /*set up buffer*/
        unsigned int **sobj = new unsigned int*[levels];
        unsigned int **sobjnpix = new unsigned int*[levels];
        for(k=0;k<levels;k++) {
            sobj[k] = new unsigned int[size];
            sobjnpix[k] = new unsigned int[size];
            for (j=0;j<size;j++) {
                sobj[k][j] = 0;
                sobjnpix[k][j] = 0;
            }
        }
        for (i=0;i<oparams.nb_levels;i++) {
            cout << "Processing hierarchical level " << i<< endl;
            results_data.read(i,nb_classes,nb_objects,oparams.nb_levels,oparams.int_buffer_size,region_classes,region_objects);
            if (i == 0) {
                for (j=0;j<size;j++) {
                    if (mask[j] == 1) {
                        region_label = class_labels_map[j];
                        region_index = region_label - 1;
                        region_npix = region_classes[region_index].get_npix();
                        sobj[0][j] = region_label;
                        sobjnpix[0][j] = region_npix;
                        sobj[1][j] = region_label;
                        sobjnpix[1][j] = region_npix;
                        sobj[2][j] = region_label;
                        sobjnpix[2][j] = region_npix;
                    }
                }
            }
            else {
                for (j=0;j<size;j++) {
                    if (mask[j] == 1) {
                        region_label = class_labels_map[j];
                        region_index = region_label - 1;
                        if (!region_classes[region_index].get_active_flag()) {
                            region_label = region_classes[region_index].get_merge_region_label();
                            region_index = region_label - 1;
                        }
                        region_npix = region_classes[region_index].get_npix();
                        for(k=0;k<levels;k++) {
                            if (region_npix<=initialParams.sizethres[k]) {
                                sobj[k][j] = region_label;
                                sobjnpix[k][j] = region_npix;
                            }
                        }
                    }
                }
            }
        }
        results_data.close_input();
        GDALDataset    *objOutDataset;
        GDALRasterBand *objOutBand;
        char **papszOptions = NULL;
		papszOptions = CSLSetNameValue( papszOptions, "TILED", "YES" );
		papszOptions = CSLSetNameValue( papszOptions, "COMPRESS", "LZW" );
        objOutDataset =  driver->Create(initialParams.sizeobj_file.c_str(), xsize, ysize, levels, GDT_UInt32, papszOptions);
        objOutDataset->SetGeoTransform(GeoTransform);
        objOutDataset->SetProjection(PROJCS);
        for(j=0;j<levels;j++) {
            objOutBand = objOutDataset->GetRasterBand(j+1);
            objOutBand->RasterIO(GF_Write,0,0,xsize,ysize,sobj[j],xsize,ysize,GDT_UInt32,0,0);
        }
        GDALClose( (GDALDatasetH) objOutDataset);
        if (initialParams.sizenpix_flag) {
            GDALDataset    *npixOutDataset;
            GDALRasterBand *npixOutBand;
            npixOutDataset = driver->Create(initialParams.sizenpix_file.c_str(), xsize, ysize, levels, GDT_UInt32, papszOptions);
            npixOutDataset->SetGeoTransform(GeoTransform);
            npixOutDataset->SetProjection(PROJCS);
            for(j=0;j<levels;j++) {
                npixOutBand = npixOutDataset->GetRasterBand(j+1);
                npixOutBand->RasterIO(GF_Write,0,0,xsize,ysize,sobjnpix[j],xsize,ysize,GDT_UInt32,0,0);
            }
            GDALClose( (GDALDatasetH) npixOutDataset);
        }
        GDALClose( (GDALDatasetH) objInDataset);
        GDALClose( (GDALDatasetH) maskDataset);
        return;
    }
} // namespace HSEGTilton
