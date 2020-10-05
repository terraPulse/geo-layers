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
        Gtk::HBox bool1Box, bool2Box;
        FileObject HSEGOparam;
        Gtk::Label bool1Label;
        Gtk::ComboBoxText bool1ComboBox;
        Gtk::Label bool2Label;
        Gtk::ComboBoxText bool2ComboBox;
        NumberObject debugObject;
        FileObject logFile;

        Glib::RefPtr<Gtk::UIManager> m_refUIManager;
        Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;

    private:
  };

} // HSEGTilton

#endif /* PARAMSGUI_H */
