#include "statustracker.h"

extern HSEGTilton::oParams oparams;

StatusTracker::StatusTracker() :
  m_Box(false, 5)
{
  set_title("RHSeg/HSeg/HSWO Status");
  set_border_width(5);

  // Add progress bar:
  add(m_Box);
  m_Box.pack_start( *Gtk::manage(new Gtk::Label("Percent completion of the RHSeg/HSeg/HSWO program:")));
  m_Box.pack_start(m_ProgressBar);

  // formatting drive c in timeout signal handler - called once every 50ms
  Glib::signal_timeout().connect( sigc::mem_fun(*this, &StatusTracker::on_timer), 1000);

  show_all_children();
}

// this timer callback function is executed once every 50ms (set in connection
// above).  Use timeouts when speed is not critical. (ie periodically updating
// something).
bool StatusTracker::on_timer()
{
  double value = ((double) oparams.percent_complete)/100.0;

  // Update progressbar:
  m_ProgressBar.set_fraction(value);
 
  if (oparams.percent_complete == 100)
    hide();

  return (oparams.percent_complete < 100);  // return false when done
}

