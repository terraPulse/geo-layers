#ifndef STATUS_TRACKER_H
#define STATUS_TRACKER_H

#include <params/params.h>
#include <gtkmm.h>

class StatusTracker : public Gtk::Window
{
public:
  StatusTracker();

protected:
  // Signal Handlers:
  bool on_timer();

  // Member data:
  Gtk::VBox m_Box;
  Gtk::ProgressBar m_ProgressBar;
};

#endif // STATUS_TRACKER_H
