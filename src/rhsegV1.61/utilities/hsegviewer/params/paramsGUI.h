#ifndef PARAMSGUI_H
#define PARAMSGUI_H

#include <defines.h>
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
#ifdef GLSIMP
        void on_input_image_selection_changed();
#endif
        void on_HSEGOparam_selection_changed();
        void on_rgbImageStretchComboBox_changed();
        void on_viewElement_activated();
        void on_greyScaleComboBox_changed();
        void on_viewComboBox_changed();
        void on_help_requested();
        void on_run_program();
        void on_exit_requested();

      //Member widgets:
        Gtk::VBox vBox;
        Gtk::HBox displayBox, rgbImageStretchBox, rangeBox, greyScaleBox, viewBox;
#ifdef GLSIMP
        Gtk::Label input_imageLabel;
        FileObject input_image;
#endif
        Gtk::Label requiredLabel;
        FileObject HSEGOparam, labelOut, asciiOut;
        NumberObject redDisplayBand, greenDisplayBand, blueDisplayBand;
        Gtk::Label rgbImageStretchLabel;
        Gtk::ComboBoxText rgbImageStretchComboBox;
        NumberObject rangeFromObject, rangeToObject;
        Gtk::Label greyScaleLabel1;
        Gtk::ComboBoxText greyScaleComboBox;
        Gtk::Label greyScaleLabel2;
        Gtk::Label viewLabel1;
        Gtk::ComboBoxText viewComboBox;
        NumberObject viewElement;
        Gtk::Label viewLabel2, optionalLabel;
        FileObject labelIn, asciiIn, reference1, reference2, reference3;

        Glib::RefPtr<Gtk::UIManager> m_refUIManager;
        Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;

    private:
  };

} // HSEGTilton

#endif /* PARAMSGUI_H */
