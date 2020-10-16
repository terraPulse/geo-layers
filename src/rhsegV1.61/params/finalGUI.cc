// finalGUI.cc

#include "finalGUI.h"
#include "params.h"
#include <iostream>
#include <ctime>

extern HSEGTilton::Params params;

namespace HSEGTilton
{
 // Constructor
  FinalGUI::FinalGUI(Glib::ustring& strMessage):
               vBox(false,10)
  {
    if (params.program_mode == 1)
      set_title("HSWO Run Completed");
    else if (params.program_mode == 2)
      set_title("HSeg Run Completed");
    else
      set_title("RHSeg Run Completed");
    set_default_size(512,64);

  // Initialize other widgets
    messageLabel.set_text(strMessage);

    scrolled_Window.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrolled_Window.add(logTextView);

    add(vBox); // put a MenuBar at the top of the box and other stuff below it.

  //Create actions for menus:
    m_refActionGroup = Gtk::ActionGroup::create();

    //Add normal Actions:
    m_refActionGroup->add( Gtk::Action::create("ActionMenu", "Program _Actions") );
    m_refActionGroup->add( Gtk::Action::create("HSegReader", "Call HSeg_Reader"),
                           sigc::mem_fun(*this, &FinalGUI::on_hsegreader_requested) );
    m_refActionGroup->add( Gtk::Action::create("HSegViewer", "Call HSeg_Viewer"),
                           sigc::mem_fun(*this, &FinalGUI::on_hsegviewer_requested) );
    m_refActionGroup->add( Gtk::Action::create("DisplayLog", "Display L_og File"),
                           sigc::mem_fun(*this, &FinalGUI::on_display_log_requested) );
    m_refActionGroup->add( Gtk::Action::create("Exit", "_Exit"),
                           sigc::mem_fun(*this, &FinalGUI::on_exit_requested) );

    m_refUIManager = Gtk::UIManager::create();
    m_refUIManager->insert_action_group(m_refActionGroup);
    add_accel_group(m_refUIManager->get_accel_group());

    //Layout the actions in the menubar:
    Glib::ustring ui_info = 
          "<ui>"
          "  <menubar name='MenuBar'>"
          "    <menu action='ActionMenu'>"
          "      <menuitem action='HSegReader'/>"
          "      <menuitem action='HSegViewer'/>"
          "      <menuitem action='DisplayLog'/>"
          "      <separator/>"
          "      <menuitem action='Exit'/>"
          "    </menu>"
          "  </menubar>"
          "</ui>";

#ifdef GLIBMM_EXCEPTIONS_ENABLED
    try
    {
      m_refUIManager->add_ui_from_string(ui_info);
    }
    catch(const Glib::Error& ex)
    {
      std::cerr << "building menus failed: " <<  ex.what();
    }
#else
    std::auto_ptr<Glib::Error> error;
    m_refUIManager->add_ui_from_string(ui_info, error);
    if(error.get())
    {
      std::cerr << "building menus failed: " <<  error->what();
    }
#endif //GLIBMM_EXCEPTIONS_ENABLED

    //Get the menubar widget, and add it to a container widget:
    Gtk::Widget* pMenubar = m_refUIManager->get_widget("/MenuBar");
    if(pMenubar)
      vBox.pack_start(*pMenubar, Gtk::PACK_SHRINK);

    vBox.pack_start(messageLabel);
    vBox.pack_start(scrolled_Window, Gtk::PACK_EXPAND_WIDGET);

    show_all_children();

    logTextView.hide();

    show();

    return;
  }

 // Destructor...
  FinalGUI::~FinalGUI() 
  {
  }

  void *call_hsegreader(void *threadid)
  {
      const char *temp_dir;
      temp_dir = getenv("TMP");
      if (temp_dir == NULL)
        temp_dir = getenv("TEMP");
      if (temp_dir == NULL)
        temp_dir = getenv("TMPDIR");
      if (temp_dir == NULL)
      {
        temp_dir = (char *) malloc(5*sizeof(char));
        string tmp = "/tmp";
        temp_dir = tmp.c_str();
      }
      string temp_directory = temp_dir;

      static char time_buffer[TIME_SIZE];
      time_t now;
      const  struct tm *tm_ptr;
      now = time(NULL);
      tm_ptr = localtime(&now);
/*
      int length;
      length = strftime(time_buffer,TIME_SIZE,"%a%b%d%H%M%S%Y",tm_ptr);
*/
      strftime(time_buffer,TIME_SIZE,"%a%b%d%H%M%S%Y",tm_ptr);

      string temp_file_name = time_buffer;
#ifdef WINDOWS
      temp_file_name = temp_directory + "\\hsegreader_" + temp_file_name + ".params";
#else
      temp_file_name = temp_directory + "/hsegreader_" + temp_file_name + ".params";
#endif

      fstream temp_fs;
      temp_fs.open(temp_file_name.c_str(),ios_base::out);
      temp_fs << "-oparam " << params.oparam_file << endl;
      temp_fs.close();

      string call_hsegreader = "hsegreader " + temp_file_name;
      system(call_hsegreader.c_str());

      std::remove(temp_file_name.c_str());

      pthread_exit(NULL);

      return threadid;
  }

  void *call_hsegviewer(void *threadid)
  {
      const char *temp_dir;
      temp_dir = getenv("TMP");
      if (temp_dir == NULL)
        temp_dir = getenv("TEMP");
      if (temp_dir == NULL)
        temp_dir = getenv("TMPDIR");
      if (temp_dir == NULL)
      {
        temp_dir = (char *) malloc(5*sizeof(char));
        string tmp = "/tmp";
        temp_dir = tmp.c_str();
      }
      string temp_directory = temp_dir;

      static char time_buffer[TIME_SIZE];
      time_t now;
      const  struct tm *tm_ptr;
      now = time(NULL);
      tm_ptr = localtime(&now);
/*
      int length;
      length = strftime(time_buffer,TIME_SIZE,"%a%b%d%H%M%S%Y",tm_ptr);
*/
      strftime(time_buffer,TIME_SIZE,"%a%b%d%H%M%S%Y",tm_ptr);

      string temp_file_name = time_buffer;
#ifdef WINDOWS
      temp_file_name = temp_directory + "\\hsegviewer_" + temp_file_name + ".params";
#else
      temp_file_name = temp_directory + "/hsegviewer_" + temp_file_name + ".params";
#endif

      fstream temp_fs;
      temp_fs.open(temp_file_name.c_str(),ios_base::out);
      temp_fs << "-oparam " << params.oparam_file << endl;
      temp_fs.close();

      string call_hsegviewer = "hsegviewer " + temp_file_name;
      system(call_hsegviewer.c_str());

      std::remove(temp_file_name.c_str());

      pthread_exit(NULL);

      return threadid;
  }

  void FinalGUI::on_hsegreader_requested()
  {
    Glib::ustring strMessage = " ";

    pthread_t pthread;
    int return_code;
    return_code = pthread_create(&pthread, NULL, call_hsegreader, NULL);
    if (return_code)
    {
      strMessage += "\nError returned from pthread_create() for hsegreader\n";
      Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);

      end_dialog.run();
      return;
    }

    return;
  }

  void FinalGUI::on_hsegviewer_requested()
  {
    Glib::ustring strMessage = " ";

    pthread_t pthread;
    int return_code;
    return_code = pthread_create(&pthread, NULL, call_hsegviewer, NULL);
    if (return_code)
    {
      strMessage += "\nError returned from pthread_create() for hsegviewer\n";
      Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);

      end_dialog.run();
      return;
    }

    return;
  }

  void FinalGUI::on_display_log_requested()
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
    logTextBuffer->set_text(strMessage);
    logTextView.set_buffer(logTextBuffer);

    messageLabel.hide();
    logTextView.show();

    log_fs.close();

    return;
  }

  void FinalGUI::on_exit_requested()
  {
    params.status = 1;
    hide();
    return;
  }

} // namespace HSEGTilton

