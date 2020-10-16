#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include <defines.h>
#include <gtkmm.h>

using namespace std;

namespace HSEGTilton
{
  class LogWindow : public Gtk::Window
  {
    public:
        LogWindow();
        virtual ~LogWindow();

    protected:
      //Signal handlers:

      //Member widgets:
        Gtk::VBox vBox;
        Gtk::ScrolledWindow logScrolledWindow;
        Gtk::TextView logTextView;
        Glib::RefPtr<Gtk::TextBuffer> logTextBuffer;

    private:
        void get_log_text();
  };

} // HSEGTilton

#endif /* LOGWINDOW_H */
