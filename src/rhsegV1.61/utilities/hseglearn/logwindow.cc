// finalGUI.cc

#include "logwindow.h"
#include <params/params.h>
#include <iostream>

extern HSEGTilton::Params params;

namespace HSEGTilton
{
 // Constructor
  LogWindow::LogWindow():
               vBox(false,10)
  {
    if (params.program_mode == 1)
      set_title("Log of HSWO Run");
    else if (params.program_mode == 2)
      set_title("Log of HSeg Run");
    else
      set_title("Log of RHSeg Run");
    set_default_size(768,512);

    add(vBox);

    logScrolledWindow.add(logTextView);
    logScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

    vBox.pack_start(logScrolledWindow, Gtk::PACK_EXPAND_WIDGET);

  // Initialize other widgets
    get_log_text();
    logTextView.set_buffer(logTextBuffer);

    show_all_children();

    return;
  }

 // Destructor...
  LogWindow::~LogWindow() 
  {
  }

  void LogWindow::get_log_text()
  {
    Glib::ustring strMessage;
    ifstream log_fs(params.log_file.c_str());

    string line;
    getline(log_fs,line);
    strMessage = line + "\n";
    while (!log_fs.eof())
    {
      getline(log_fs,line);
      strMessage += line + "\n";
    }

    logTextBuffer = Gtk::TextBuffer::create();
    logTextBuffer->set_text(Glib::convert(strMessage,"UTF-8","ISO-8859-1"));

    log_fs.close();

    return;
  }

} // namespace HSEGTilton

