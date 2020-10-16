#ifndef FINAL_H
#define FINAL_H

#include <defines.h>
#include <gtkmm.h>
#include <pthread.h>

using namespace std;

namespace HSEGTilton
{
  class FinalGUI : public Gtk::Window
  {
    public:
        FinalGUI(Glib::ustring& strMessage);
        virtual ~FinalGUI();

        friend void *call_hsegreader(void *threadid);
        friend void *call_hsegviewer(void *threadid);

    protected:
      //Signal handlers:
        void on_hsegreader_requested();
        void on_hsegviewer_requested();
        void on_display_log_requested();
        void on_exit_requested();

      //Member widgets:
        Gtk::VBox vBox;
        Gtk::Label messageLabel;
        Glib::RefPtr<Gtk::UIManager> m_refUIManager;
        Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;

        Gtk::TextView     logTextView;
        Glib::RefPtr<Gtk::TextBuffer> logTextBuffer;
        Gtk::ScrolledWindow scrolled_Window;

    private:
  };

} // HSEGTilton

#endif /* FINAL_H */
