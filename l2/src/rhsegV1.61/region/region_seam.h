/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>>       Purpose:  Provides the RegionSeam class, which encapsulates region edge information along the processing window seam
   >>>>
   >>>>    Written By:  James C. Tilton, MC 606.3, NASA's GSFC, Greenbelt, MD 20771
   >>>>                 e-mail:  James.C.Tilton@nasa.gov
   >>>>          Date:  March 27, 2014
   >>>>
   >>>> Modifications:  
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#ifndef REGION_SEAM_H
#define REGION_SEAM_H

using namespace std;

namespace HSEGTilton
{

  class RegionSeam
  {
    public:
      //  CONSTRUCTORS and DESTRUCTOR
      RegionSeam();
      RegionSeam(const RegionSeam& source);
      RegionSeam(const unsigned int& npix_value, const float& sum_edge_value);
      ~RegionSeam();

      //  MODIFICATION MEMBER FUNCTIONS
      void operator =(const RegionSeam& source);
      void operator +=(const RegionSeam& source);
      void clear();

      //  CONSTANT MEMBER FUNCTIONS
      unsigned int get_npix() const { return npix; }
      float get_sum_edge() const { return sum_edge; }
      void print();

      // FRIEND FUNCTIONS and CLASSES

    private:
      //  PRIVATE DATA
      unsigned int npix;       // Number of pixels
      float        sum_edge;   // Sum edge value

      //  PRIVATE STATIC DATA

  };
} // HSEGTilton

#endif /* REGION_SEAM_H */
