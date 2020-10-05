#ifndef HSEGLEARN_H
#define HSEGLEARN_H

#define AMBIGUOUS   // Flag for including code for explicit submission of ambigous examples
#undef  AMBIGUOUS

#include <defines.h>
#include "logwindow.h"
#include <gui/numberObject.h>
#include <region/region_class.h>
#include <region/region_object.h>
#include <results/results.h>
#include <displayimage/displayimage.h>
#include <gtkmm.h>

using namespace std;
using namespace CommonTilton;

namespace HSEGTilton
{
  class HSegLearn : public Gtk::Window
  {
    public:
        HSegLearn();
        virtual ~HSegLearn();

    protected:
      // Signal Handlers:
        void on_help_requested();
        void on_quit_requested();

        void on_rgbImageButton_clicked();
        void on_regionLabelButton_clicked();
        void on_panchromaticImageButton_clicked();
        void on_referenceImageButton_clicked();
        void on_refocusButton_clicked();
        void on_displayLogButton_clicked();

        void on_updateRGBImageButton_clicked();
        void on_rgbImageStretchComboBox_changed();

        void on_undoLastHighlightedClassesButton_clicked();
        void on_clearAllHighlightedClassesButton_clicked();
        void on_selectPosExampleButton_clicked();
        void on_selectNegExampleButton_clicked();
        void on_selectAmbExampleButton_clicked();

        void on_undoLastSubmitButton_clicked();
        void on_submitExamplesButton_clicked();
        void on_submitPosExampleButton_clicked();
        void on_submitAmbExampleButton_clicked();
        void on_submitNegExampleButton_clicked();

        void on_labelDataDisplay_circleROI_event();
        void on_rgbImageDisplay_buttonPress_event();
        void on_labelDataDisplay_buttonPress_event();
        void on_panImageDisplay_buttonPress_event();
        void on_referenceImageDisplay_buttonPress_event();

      //Child widgets:
        Gtk::VBox vBox;
        Gtk::HBox displayBox, rgbImageStretchBox, rangeBox;
        
        Gdk::Color color[NUMBER_OF_LABELS];

        Glib::RefPtr<Gtk::UIManager> m_refUIManager;
        Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;

        Gtk::Frame displayOptionsFrame;
        Gtk::Table displayOptionsTable;
        Gtk::Frame selectOptionsFrame;
        Gtk::Table selectOptionsTable;
        Gtk::Frame submissionOptionsFrame;
        Gtk::Table submissionOptionsTable;

        Gtk::Button rgbImageButton, regionLabelButton, panchromaticImageButton, referenceImageButton;
        Gtk::Button refocusButton, displayLogButton;
        Gtk::Button updateRGBImageButton;
        NumberObject redDisplayBand, greenDisplayBand, blueDisplayBand;
        Gtk::Label rgbImageStretchLabel;
        Gtk::ComboBoxText rgbImageStretchComboBox;
        NumberObject rangeFromObject, rangeToObject;
        Gtk::Label highlightOptionsLabel;
        Gtk::Button undoLastHighlightedClassesButton, clearAllHighlightedClassesButton;
        Gtk::Button selectPosExampleButton, selectNegExampleButton, selectAmbExampleButton;
        Gtk::Button undoLastSubmitButton, submitExamplesButton;
        Gtk::Button submitPosExampleButton, submitNegExampleButton, submitAmbExampleButton;

        DisplayImage rgbImageDisplay, labelDataDisplay, panImageDisplay, referenceImageDisplay;
        LogWindow logWindow;

    private:
        void highlightCircledClasses();
        void highlightRegionClass();
        void buttonPress_event();
#ifdef THREEDIM
        bool subsetfrom3d();
#endif
        bool initializeImages();
        void set_label_data_colormap();
        void set_segLevelClassLabelsMapImage();
        void read_examples_in();
        void write_examples_out();
        void submit_pos_examples();
        void submit_neg_examples();
        void submit_amb_examples();
        void submit_pos_example();
        void submit_neg_example();
        void submit_amb_example();
        void submit_pos_selection();
        void submit_neg_selection();
        void submit_amb_selection();
        void submit_example();
        void build_selection_list();
        bool find_coarsest_level();
        bool selectFinerSeg();
        bool selectCoarserSeg();
        void update_selection_label();
        void update_region_label();
        void clear_region_label(unsigned int& region_label);
        unsigned int check_conflict();
        void save_current_state();
        void restore_previous_state();

        bool selClickFlag, selClassLabelFlag, selROIFlag;
        int  selColClick, selRowClick;
        unsigned int selClassLabel;
        vector<RegionClass> region_classes;
        vector<RegionObject> region_objects;
        Results results_data;
        int segLevel, view_ncols, view_nrows;
        int nb_classes, nb_objects;
        int region_classes_size, region_objects_size;
        vector<unsigned int> selection_list, saved_selection_list;
        vector<unsigned int> pos_example_list, saved_pos_example_list, pos_selection_list;
        vector<unsigned int> neg_example_list, saved_neg_example_list, neg_selection_list;
        vector<unsigned int> amb_example_list, saved_amb_example_list, amb_selection_list;
        unsigned char highlight_label, conflict_label;
  };

} // HSEGTilton

#endif // HSEGLEARN_H
