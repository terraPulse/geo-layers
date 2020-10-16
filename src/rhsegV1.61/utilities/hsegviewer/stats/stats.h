#ifndef STATS_H
#define STATS_H

#include <image/image.h>

using namespace std;

namespace HSEGTilton
{

    class Stats
    {
      public:
         //  CONSTRUCTORS and DESTRUCTOR
          Stats( );
          ~Stats( );

         //  MODIFICATION MEMBER FUNCTIONS
          void set_segLevel(const unsigned int& segLevel);
          void compute_and_set_colormap(const unsigned int& segLevel, CommonTilton::Image& inputImage);

         //  CONSTANT MEMBER FUNCTIONS
          unsigned int get_new_class_label(const unsigned int& classLabel, const int& segLevel);
          unsigned int get_new_object_label(const unsigned int& objectLabel, const int& segLevel);
          unsigned int get_class_npix(const unsigned int& classLabel, const int& segLevel);
          unsigned int get_object_npix(const unsigned int& objectLabel, const int& segLevel);
          unsigned int get_class_boundary_npix(const unsigned int& classLabel, const int& segLevel);
          unsigned int get_object_boundary_npix(const unsigned int& objectLabel, const int& segLevel);
          double get_class_std_dev(const unsigned int& classLabel, const int& segLevel);
          double get_object_std_dev(const unsigned int& objectLabel, const int& segLevel);
          double get_maxStdDev( ) const { return maxStdDev; }
          double get_classMean(const int& index) const { return classMeanList[index]; }
          float get_classStdDev(const int& index) const { return (float) classStdDevList[index]; }
          float get_objectStdDev(const int& index) const { return (float) objectStdDevList[index]; }
          float get_classBPRatio(const int& index) const
                { return ((float) classBoundNOPixelsList[index])/((float) classNOPixelsList[index]); }
          float get_objectBPRatio(const int& index) const 
                { return ((float) objectBoundNOPixelsList[index])/((float) objectNOPixelsList[index]); }
         // FRIEND FUNCTIONS and CLASSES
          friend class Results;

      private:
         //  PRIVATE DATA
          unsigned int *classMergesList;
          unsigned int *classNOPixelsList;
          double       *classMeanList;
          double       *classStdDevList;
          double       *classMergeThreshList;
          unsigned int *classBoundNOPixelsList;
          unsigned int *classNOObjectsList;
          unsigned int *objectMergesList;
          unsigned int *objectNOPixelsList;
          double       *objectStdDevList;
          unsigned int *objectBoundNOPixelsList;

          double       maxStdDev;

          int view_ncols, view_nrows;    // Number of columns and rows viewed in displayImages
          int nbands;            // Number of spectral bands in input image data
          int nb_hlevels;        // Number of hierarchical levels
          int orig_nb_classes;   // Number of region classes at onset of writing hierarchical segmentation results
          int orig_nb_objects;   // Number of region objects at onset of writing hierarchical segmentation results
  } ;

} // HSEGTilton

#endif /* STATS_H */
