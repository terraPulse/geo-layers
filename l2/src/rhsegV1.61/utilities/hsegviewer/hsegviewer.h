#ifndef HSEGVIEWER_H
#define HSEGVIEWER_H

#include <defines.h>
#include <gui/numberObject.h>
#include "stats/stats.h"
#include "results/results.h"
#include "labelregion/labelregion.h"
#include <displayimage/displayimage.h>
#include <gtkmm.h>

using namespace std;
using namespace CommonTilton;

namespace HSEGTilton
{
  class HSegViewer : public Gtk::Window
  {
    public:
        HSegViewer();
        virtual ~HSegViewer();

    protected:
      // Signal Handlers:
        void on_help_requested();
        void on_quit_requested();
        void on_selectRegionClassButton_clicked();
        void on_selectRegionObjectButton_clicked();
        void on_selectRegionClass_activate();
        void on_labelRegionButton_clicked();
        void on_selectRegionObject_activate();
        void on_refocusButton_clicked();
        void on_initSegLevel_activate();
        void on_selectFinerSegButton_clicked();
        void on_segLevelObject_activate();
        void on_selectCoarserSegButton_clicked();
        void on_rgbImageButton_clicked();
        void on_classesSliceButton_clicked();
        void on_regionMeanButton_clicked();
        void on_regionLabelButton_clicked();
        void on_objectsSliceButton_clicked();
        void on_boundaryMapButton_clicked();
        void on_classStdDevButton_clicked();
        void on_classBPRatioButton_clicked();
        void on_objectStdDevButton_clicked();
        void on_objectBPRatioButton_clicked();
        void on_reference1Button_clicked();
        void on_reference2Button_clicked();
        void on_reference3Button_clicked();
        void on_labelRegion_activated();
        void on_labelRegion_color_set();
        void on_labelRegion_undo();
        void on_updateRGBImageButton_clicked();
        void on_rgbImageStretchComboBox_changed();

      //Child widgets:
        Gtk::VBox vBox;
        Gtk::HBox displayBox, rgbImageStretchBox, rangeBox;
        
        Glib::RefPtr<Gtk::UIManager> m_refUIManager;
        Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;

        Gtk::Table line1Table;
        Gtk::Table line2Table;
        Gtk::Table line3Table;
        Gtk::Table line4Table;
        Gtk::Table line5Table;
        Gtk::Table selectSegTable;
        Gtk::Frame displayOptionsFrame;
        Gtk::Table displayOptionsTable;
        Gtk::Frame featureValuesFrame;
        Gtk::Table featureValuesTable;
        Gtk::Entry *featureValuesEntries;
        Gtk::ScrolledWindow scrolledTable;

        Gtk::Button selectRegionClassButton;
        Gtk::Button selectRegionObjectButton;
        NumberObject selectRegionClass;
        Gtk::Button labelRegionButton;
        NumberObject selectRegionObject;
        Gtk::Button refocusButton;
        NumberObject initSegLevel;
        Gtk::Button selectFinerSegButton;
        NumberObject segLevelObject;
        Gtk::Button selectCoarserSegButton;
        Gtk::Button rgbImageButton, classesSliceButton, regionMeanButton;
        Gtk::Button regionLabelButton, objectsSliceButton, boundaryMapButton;
        Gtk::Button classStdDevButton, objectStdDevButton;
        Gtk::Button classBPRatioButton, objectBPRatioButton;
        Gtk::Button reference1Button, reference2Button, reference3Button;
        Gtk::Button updateRGBImageButton;
        NumberObject redDisplayBand, greenDisplayBand, blueDisplayBand;
        Gtk::Label rgbImageStretchLabel;
        Gtk::ComboBoxText rgbImageStretchComboBox;
        NumberObject rangeFromObject, rangeToObject;

        Stats   statsData;
        Results resultsData;
        LabelRegion labelRegion;
        DisplayImage rgbImageDisplay;
        DisplayImage classesSliceDisplay;
        DisplayImage regionMeanDisplay;
        DisplayImage labelDataDisplay;
        DisplayImage objectsSliceDisplay;
        DisplayImage boundaryMapDisplay;
        DisplayImage classStdDevDisplay;
        DisplayImage objectStdDevDisplay;
        DisplayImage classBPRatioDisplay;
        DisplayImage objectBPRatioDisplay;
        DisplayImage reference1Display;
        DisplayImage reference2Display;
        DisplayImage reference3Display;

    private:
#ifdef THREEDIM
        bool subsetfrom3d();
#endif
        bool initializeImages();
        void update_label_data_colormap();
        void update_region_label(unsigned char highlight_label);
        void set_region_class_table();
        void set_region_object_table();
        void set_region_class();
        void set_region_object();
        void redisplay();
        bool selClickFlag, selClassLabelFlag, selObjectLabelFlag;
        int  selColClick, selRowClick;
        unsigned int selClassLabel, selObjectLabel, selLabel;
        int segLevel;
        int  view_ncols, view_nrows, table_nrows, table_ncols;
        unsigned int nb_classes, nb_objects;
        
  };

} // HSEGTilton

#endif // HSEGVIEWER_H

