#ifndef EXPPARAMS_H
#define EXPPARAMS_H

#include <defines.h>
#include <gui/fileObject.h>
#include <gui/numberObject.h>
#include <gtkmm.h>

using namespace std;
using namespace CommonTilton;

namespace HSEGTilton
{
  class ExpParamsGUI : public Gtk::Window
  {
    public:
        ExpParamsGUI();
        virtual ~ExpParamsGUI();

    protected:
      //Signal handlers:
        void on_help_requested();
        void on_close_requested();
        void on_normindComboBox_changed();
        void on_initThresholdObject_activate();
        void on_initialMergeNpixelsObject_activate();
        void on_randomInitSeedButton_clicked();
        void on_sortButton_clicked();
        void on_edgeDissimOptionComboBox_changed();
        void on_rnbLevelsObject_activate();
        void on_ionbLevelsObject_activate();
        void on_minNregionsObject_activate();
        void on_spClustMinObject_activate();
        void on_spClustMaxObject_activate();
        void on_mergeAccelButton_clicked();
        void on_normindRestore_clicked();
        void on_initThresholdRestore_clicked();
        void on_initialMergeNpixelsRestore_clicked();
        void on_randomInitSeedRestore_clicked();
        void on_sortRestore_clicked();
        void on_edgeDissimOptionRestore_clicked();
        void on_rnbLevelsRestore_clicked();
        void on_ionbLevelsRestore_clicked();
        void on_minNregionsRestore_clicked();
        void on_spClustMinRestore_clicked();
        void on_spClustMaxRestore_clicked();
        void on_mergeAccelRestore_clicked();

      //Member widgets:
        Gtk::VBox vBox;
        Gtk::Table lineTable;
        Gtk::Button  normindRestore;
        Gtk::ComboBoxText normindComboBox;
        Gtk::Button  initThresholdRestore;
        NumberObject initThresholdObject;
        Gtk::Button  initialMergeNpixelsRestore;
        NumberObject initialMergeNpixelsObject;
        Gtk::Button  randomInitSeedRestore;
        Gtk::CheckButton randomInitSeedButton;
        Gtk::Button  sortRestore;
        Gtk::CheckButton sortButton;
        Gtk::Button  edgeDissimOptionRestore;
        Gtk::ComboBoxText edgeDissimOptionComboBox;
        Gtk::Button  rnbLevelsRestore;
        NumberObject rnbLevelsObject;
        Gtk::Button  ionbLevelsRestore;
        NumberObject ionbLevelsObject;
        Gtk::Button  minNregionsRestore;
        NumberObject minNregionsObject;
        Gtk::Button  spClustMinRestore;
        NumberObject spClustMinObject;
        Gtk::Button  spClustMaxRestore;
        NumberObject spClustMaxObject;
        Gtk::Button  mergeAccelRestore;
        Gtk::CheckButton mergeAccelButton;

        Glib::RefPtr<Gtk::UIManager> m_refUIManager;
        Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;

    private:
        bool initialMergeFlag;
        bool randomInitSeedFlag;
        bool sortFlag;
        bool mergeAccelFlag;
  };

} // HSEGTilton

#endif /* EXPPARAMS_H */
