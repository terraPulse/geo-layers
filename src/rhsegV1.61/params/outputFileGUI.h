#ifndef OUTPUTFILEGUI_H
#define OUTPUTFILEGUI_H

#include <defines.h>
#include <gui/fileObject.h>
#include <gui/numberObject.h>
#include <string>
#include <gtkmm.h>

using namespace std;
using namespace CommonTilton;

namespace HSEGTilton
{
  class OutputFileGUI : public Gtk::Window
  {
    public:
        OutputFileGUI();
        virtual ~OutputFileGUI();

    protected:
      //Signal handlers:
        void on_help_requested();
        void on_exit_requested();
        void on_objectConnType1Button_checked();
        void on_regionObjectsButton_checked();
        void on_next_panel();
        void on_run_program();

      //Member widgets:
        Gtk::VBox  vBox, initialBox;
        Gtk::Label inputImageFile;
        Gtk::Label inputDescription;
        Gtk::Label maskImageFile;
        Gtk::Label stdDevInImageFile;
        Gtk::Label stdDevMaskImageFile;
        Gtk::Label edgeInImageFile;
        Gtk::Label edgeMaskImageFile;
        Gtk::Label regionMapInImageFile;
        Gtk::Label spClustWght;
        Gtk::Label dissimCrit;
        Gtk::Label logFile;
        Gtk::Frame outputFilesFrame;
        Gtk::VBox  outputFilesBox;
        Gtk::Label optionalLabel;
        FileObject classLabelsMapFile;
        FileObject boundaryMapFile;
        FileObject regionClassesFile;
        Gtk::CheckButton objectConnType1Button;
        Gtk::CheckButton regionObjectsButton;
        FileObject objectLabelsMapFile;
        FileObject regionObjectsFile;
        FileObject oparamFile;

        Glib::RefPtr<Gtk::UIManager> m_refUIManager;
        Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;

    private:
        bool objectConnType1Flag;
        bool regionObjectsFlag;
        bool on_delete_event(GdkEventAny* event);
  };

} // HSEGTilton

#endif /* OUTPUTFILEGUI_H */

