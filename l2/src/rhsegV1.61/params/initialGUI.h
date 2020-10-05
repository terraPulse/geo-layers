#ifndef INITIAL_H
#define INITIAL_H

#include <defines.h>
#include <gui/fileObject.h>
#include <gui/numberObject.h>
#include <gtkmm.h>

using namespace std;
using namespace CommonTilton;

namespace HSEGTilton
{
  class InitialGUI : public Gtk::Window
  {
    public:
        InitialGUI();
        virtual ~InitialGUI();

    protected:
      //Signal handlers:
        void on_help_requested();
        void on_version_requested();
        void on_exit_requested();
        void on_modeComboBox_changed();
        void on_inputImageFile_selection_changed();
        void on_maskImageFile_selection_changed();
        void on_stdDevInImageFile_selection_changed();
        void on_stdDevMaskImageFile_selection_changed();
        void on_edgeInImageFile_selection_changed();
        void on_edgeMaskImageFile_selection_changed();
        void on_regionMapInImageFile_selection_changed();
        void on_logFile_selection_changed();
        void set_current_folder(const string& fileName);
        void on_next_panel();
        void on_run_program();

      //Member widgets:
        Gtk::VBox vBox;
        Gtk::HBox modeHBox;
        Gtk::Label modeLabel;
        Gtk::ComboBoxText modeComboBox;
        Gtk::Frame inputFilesFrame;
        Gtk::VBox inputFilesBox;
        Gtk::Label optionalLabel;
        FileObject inputImageFile;
#ifndef GDAL
        NumberObject numberOfColumns;
        NumberObject numberOfRows;
#ifdef THREEDIM
        NumberObject numberOfSlices;
#endif
        NumberObject numberOfBands;
        Gtk::HBox dtypeHBox;
        Gtk::Label dtypeLabel;
        Gtk::ComboBoxText dtypeComboBox;
#endif // !GDAL
        FileObject maskImageFile;
#ifndef GDAL
        NumberObject maskValue;
#endif // !GDAL
        FileObject stdDevInImageFile;
        FileObject stdDevMaskImageFile;
        FileObject edgeInImageFile;
        FileObject edgeMaskImageFile;
        FileObject regionMapInImageFile;
        NumberObject spClustWght;
        Gtk::HBox comboHBox;
        Gtk::Label dissimCritLabel;
        Gtk::ComboBoxText dissimCritComboBox;
        FileObject logFile;
        Gtk::Label logLabel;
        Gtk::Label copyrightLabel;

        Glib::RefPtr<Gtk::UIManager> m_refUIManager;
        Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;

    private:
        string current_folder;
        bool input_image_flag, log_flag;
        bool on_delete_event(GdkEventAny* event);
  };

} // HSEGTilton

#endif /* INITIAL_H */
