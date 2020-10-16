#ifndef PARAMSGUI_H
#define PARAMSGUI_H

#include <params/params.h>
#include <gui/fileObject.h>
#include <gui/numberObject.h>
#include <gtkmm.h>

using namespace std;
using namespace CommonTilton;

namespace HSEGTilton
{
  class ParamsGUI : public Gtk::Window
  {
    public:
        ParamsGUI();
        virtual ~ParamsGUI();
        void set_oparam_file(const string& file_name);

    protected:
      //Signal handlers:
        void on_run_program();
        void on_help_requested();
        void on_exit_requested();
        void on_HSEGOparam_selection_changed();

      //Member widgets:
        Gtk::VBox vBox;
        Gtk::HBox hBox;
        FileObject HSEGOparam;
        Gtk::Label segLevelLabel;
        Gtk::ComboBoxText segLevelComboBox;
        FileObject classLabelsMapExtFile;
        FileObject classNpixMapExtFile;
        FileObject classMeanMapExtFile;
        FileObject classStdDevMapExtFile;
        FileObject classBPRatioMapExtFile;
#ifdef SHAPEFILE
        FileObject classShapefileExtFile;
#endif
        FileObject objectLabelsMapExtFile;
        FileObject objectNpixMapExtFile;
        FileObject objectMeanMapExtFile;
        FileObject objectStdDevMapExtFile;
        FileObject objectBPRatioMapExtFile;
#ifdef SHAPEFILE
        FileObject objectShapefileExtFile;
#endif

        Glib::RefPtr<Gtk::UIManager> m_refUIManager;
        Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;

    private:
  };

} // HSEGTilton

#endif /* PARAMSGUI_H */
