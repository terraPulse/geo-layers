/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>>       Purpose: General include file for hsegreader
   >>>>
   >>>>    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
   >>>>        E-Mail: James.C.Tilton@nasa.gov
   >>>>
   >>>>          Date: November 15, 2007
   >>>> Modifications: March 24, 2008 - Added gtkmm code for HSEGReader Class
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#ifndef HSEGREADER_H
#define HSEGREADER_H

#include <defines.h>
#include <spatial/spatial.h>
#include <region/region_class.h>
#include <region/region_object.h>
#include <results/results.h>

#ifdef GTKMM
#include <gtkmm.h>
#include <algorithm>
#endif

#ifndef IMAGE_H
  enum RHSEGDType { Unknown, UInt8, UInt16, UInt32, Float32 };
#define IMAGE_H
#endif

using namespace std;

namespace HSEGTilton
{
#ifdef GTKMM
  class HSEGReader : public Gtk::Window
  {
    public:
        HSEGReader();
        virtual ~HSEGReader();

    protected:
      // Signal Handlers:
        void on_help_requested();
        void on_quit_requested();
        void on_segLevelComboBox_changed();
        void on_classNpixButton_clicked();
        void on_classStdDevButton_clicked();
        void on_classBPRatioButton_clicked();
        void on_objectNpixButton_clicked();
        void on_objectStdDevButton_clicked();
        void on_objectBPRatioButton_clicked();
        void on_classSpinButton_value_changed();
        void on_nextClassButton_clicked();
        void on_objectSpinButton_value_changed();
        void on_nextObjectButton_clicked();

      //Child widgets:
        Gtk::VBox vBox;
        Gtk::HBox featureBox;
        
        Glib::RefPtr<Gtk::UIManager> m_refUIManager;
        Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;

        Gtk::Table windowTable;

        Gtk::Label segLevelLabel;
        Gtk::Label classOrderLabel, objectOrderLabel;
        Gtk::Label classLabel, objectLabel;

        Gtk::ComboBoxText segLevelComboBox;
        Gtk::Button       classNpixButton, classStdDevButton, classBPRatioButton;
        Gtk::Button       objectNpixButton, objectStdDevButton, objectBPRatioButton;
        Gtk::SpinButton   classSpinButton;
        Gtk::Button       nextClassButton;
        Gtk::SpinButton   objectSpinButton;
        Gtk::Button       nextObjectButton;
        Gtk::TextView     classTextView, objectTextView;
        Glib::RefPtr<Gtk::TextBuffer> classTextBuffer, objectTextBuffer;

    private:
        Spatial spatial_data;
        vector<RegionClass> region_classes;
        vector<RegionObject> region_objects;
        vector<RegionClass *> class_heap;
        vector<RegionObject *> object_heap;
        Results results_data;
        bool classNpixFlag, classStdDevFlag, classBPRatioFlag;
        bool objectNpixFlag, objectStdDevFlag, objectBPRatioFlag;
        int segLevel;
        int nb_classes, nb_objects;
        int region_classes_size, region_objects_size;
        int class_heap_size, object_heap_size;
  };
#else
    bool hsegreader( );
#endif
#ifdef THREEDIM
    void find_nghbr(const int& col, const int& row, const int& slice,
                    const short unsigned int& nbdir, int& nbcol, int& nbrow, int& nbslice);
#else
    void find_nghbr(const int& col, const int& row,
                    const short unsigned int& nbdir, int& nbcol, int& nbrow);
#endif
    void object_nghbrs_set_init(const short unsigned int& hlevel, Spatial& spatial_data,
                                vector<RegionClass>& region_classes, vector<RegionObject>& region_objects);
} // HSEGTilton

#endif /*-- HSEGREADER_H --*/

