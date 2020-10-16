#ifndef HSEGPVOTE_H
#define HSEGPVOTE_H

#include <defines.h>
#include <region/region_class.h>
#include <region/region_object.h>
#include <image/image.h>
#include <results/results.h>

using namespace std;
using namespace CommonTilton;

namespace HSEGTilton
{
    // Global function definitions
    bool hsegpvote();
    void set_segLevelLabel(const int& ncols, const int& nrows, const int& segLevel,
                           vector<RegionClass>& region_classes, vector<RegionObject>& region_objects,
                           Image& RHSEGLabelImage, Image& segLevelLabelImage);
    bool connected_component(Image& classSegImage, Image& objectSegImage);

} // HSEGTilton

#endif // HSEGPVOTE_H
