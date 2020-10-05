// serialkeyGUI.cc

#include "serialkeyGUI.h"
#include <params/params.h>
#include <iostream>
#include <fstream>

extern HSEGTilton::Params params;

namespace HSEGTilton
{
 // Constructor
  SerialkeyGUI::SerialkeyGUI():
                instruction_Label("Press the \"Enter\" key after entering text!"),
                user_name_Label("Enter User Name [max 20 char]: "),
                serial_key_Label("Enter Serial Key [12 char]: "),
                close_Button("I don't have the Serialkey Information!")
  {
    set_size_request(384,128);
    set_title("Input the Serialkey Information");

    user_name_flag = false;
    serial_key_flag = false;

    add(vBox);

    vBox.pack_start(instruction_Label);

    user_name_HBox.pack_start(user_name_Label);
    user_name_Entry.signal_activate().connect( sigc::mem_fun(*this, &SerialkeyGUI::on_user_name_activate) );
    user_name_HBox.pack_start(user_name_Entry);
    vBox.pack_start(user_name_HBox);

    serial_key_HBox.pack_start(serial_key_Label);
    serial_key_Entry.signal_activate().connect( sigc::mem_fun(*this, &SerialkeyGUI::on_serial_key_activate) );
    serial_key_HBox.pack_start(serial_key_Entry);
    vBox.pack_start(serial_key_HBox);

    close_Button.signal_clicked().connect( sigc::mem_fun(*this, &SerialkeyGUI::on_close_button) );
    vBox.pack_start(close_Button);

    params.status = false;

    show_all_children();

    return;
  }

 // Destructor...
  SerialkeyGUI::~SerialkeyGUI() 
  {
    return;
  }

  void SerialkeyGUI::on_close_button()
  {
    params.status = 2;
    hide();
    return;
  }

  void SerialkeyGUI::on_user_name_activate()
  {
    int user_len;

    user_name = user_name_Entry.get_text();
    user_len = strlen(user_name.c_str());

    if (user_len <= 20)
    {
      user_name_flag = true;
      if (serial_key_flag)
        write_serial_key();
    }
    else
    {
      Glib::ustring strMessage = " ";
      strMessage += "\nInvalid User Name (too long).\n";
      Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
      end_dialog.run();
      user_name_Entry.set_text("");
    }

    return;
  }

  void SerialkeyGUI::on_serial_key_activate()
  {
    int serial_len;

    serial_key = serial_key_Entry.get_text();
    serial_len = strlen(serial_key.c_str());

    if (serial_len == 12)
    {
      serial_key_flag = true;
      if (user_name_flag)
        write_serial_key();
    }
    else
    {
      Glib::ustring strMessage = " ";
      strMessage += "\nInvalid Serial Key (not 12 characters long).\n";
      Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
      end_dialog.run();
      serial_key_Entry.set_text("");
    }

    return;
  }

  void SerialkeyGUI::write_serial_key()
  {
    hide();

    const char *tmpdir;
    tmpdir = getenv("TMP");
    if (tmpdir == NULL)
      tmpdir = getenv("TEMP");
    if (tmpdir == NULL)
      tmpdir = getenv("TMPDIR");
    if (tmpdir == NULL)
    {
      tmpdir = (char *) malloc(5*sizeof(char));
      string tmp = "/tmp";
      tmpdir = tmp.c_str();
    }
    if (tmpdir == NULL)
    {
      tmpdir = (char *) malloc(2*sizeof(char));
      tmpdir = ".";
    }

    string tempfile = tmpdir;
    tempfile += "/RHSEG_Serial_Key.txt";

    ofstream out_file;
    out_file.open(tempfile.c_str( ));
    if (out_file.fail( ))
    {
      Glib::ustring strMessage = " ";
      strMessage += "\nFailed to open Serial Key File" + tempfile + "\n";
      Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
      end_dialog.run();
      serial_key_Entry.set_text("");
    }
    else
    {
      out_file << user_name << endl;
      out_file << serial_key << endl;
      out_file.close( );
      params.status = 1;
    }

    return;
  }

} // namespace HSEGTilton
