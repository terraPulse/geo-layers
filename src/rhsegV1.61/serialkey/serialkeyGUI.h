#ifndef SERIALKEYGUI_H
#define SERIALKEYGUI_H

#include <defines.h>
#include <gtkmm.h>

using namespace std;

namespace HSEGTilton
{
  class SerialkeyGUI : public Gtk::Window
  {
    public:
        SerialkeyGUI();
        virtual ~SerialkeyGUI();

    protected:
      //Signal handlers:
        void on_user_name_activate();
        void on_serial_key_activate();
        void on_close_button();

      //Member widgets:
        Gtk::VBox vBox;
        Gtk::HBox user_name_HBox;
        Gtk::Label instruction_Label;
        Gtk::Label user_name_Label;
        Gtk::Entry user_name_Entry;
        Gtk::HBox serial_key_HBox;
        Gtk::Label serial_key_Label;
        Gtk::Entry serial_key_Entry;
        Gtk::Button close_Button;

    private:
        string user_name;
        bool user_name_flag;
        string serial_key;
        bool serial_key_flag;
        void write_serial_key();
  };

} // HSEGTilton

#endif /* SERIALKEYGUI_H */
