// paramsGUI.cc
#include "paramsGUI.h"
#include "initialParams.h"
#include <iostream>

extern HSEGTilton::InitialParams initialParams;
extern HSEGTilton::Params params;

namespace HSEGTilton
{
 // Constructor
  ParamsGUI::ParamsGUI():
             vBox(false,10),
             HSEGOparam(" HSeg/RHSeg Output Parameter File (oparam) for Input to HSegReader:",
                        "Enter or Select the HSeg/RHSeg Output Paramter File (oparam)",
                        Gtk::FILE_CHOOSER_ACTION_OPEN),
             bool1Label("Include the region class number for each region class\nneighboring each region object:   ",0.0),
             bool2Label("Include the region object number for each region object\nneighboring each region object: ",0.0),
             debugObject("Debug option:"),
             logFile(" Output Log File:",
                      "Enter or Select the Output Log File",
                      Gtk::FILE_CHOOSER_ACTION_SAVE,true)
  {
    set_title("Hierarchical Segmentation Results Reader Parameter Input");
    set_default_size(512,64);

    char *cwd;
    cwd = g_get_current_dir();
    params.current_folder = cwd;
    HSEGOparam.add_shortcut_folder(params.current_folder);
    HSEGOparam.set_current_folder(params.current_folder);
    g_free(cwd);

#ifdef GTKMM3
    bool1ComboBox.append("YES");
    bool1ComboBox.append("NO");
    bool2ComboBox.append("YES");
    bool2ComboBox.append("NO");
#else
    bool1ComboBox.append_text("YES");
    bool1ComboBox.append_text("NO");
    bool2ComboBox.append_text("YES");
    bool2ComboBox.append_text("NO");
#endif
#ifdef ARTEMIS
    bool1ComboBox.set_active(1);
    bool2ComboBox.set_active(1);
#else
    bool1ComboBox.set_active_text("NO");
    bool2ComboBox.set_active_text("NO");
#endif
    debugObject.set_value(1);

    HSEGOparam.signal_selection_changed().connect(sigc::mem_fun(*this,
                         &ParamsGUI::on_HSEGOparam_selection_changed) );

    add(vBox); // put a MenuBar at the top of the box and other stuff below it.

  //Create actions for menus:
    m_refActionGroup = Gtk::ActionGroup::create();

    //Add normal Actions:
    m_refActionGroup->add( Gtk::Action::create("ActionMenu", "Program _Actions") );
    m_refActionGroup->add( Gtk::Action::create("Run", "_Run Program"),
                      sigc::mem_fun(*this, &ParamsGUI::on_run_program) );
    m_refActionGroup->add( Gtk::Action::create("Help", "_Help"),
                      sigc::mem_fun(*this, &ParamsGUI::on_help_requested) );
    m_refActionGroup->add( Gtk::Action::create("Exit", "_Exit"),
                      sigc::mem_fun(*this, &ParamsGUI::on_exit_requested) );

    m_refUIManager = Gtk::UIManager::create();
    m_refUIManager->insert_action_group(m_refActionGroup);
    add_accel_group(m_refUIManager->get_accel_group());

    //Layout the actions in the menubar:
    Glib::ustring ui_info = 
          "<ui>"
          "  <menubar name='MenuBar'>"
          "    <menu action='ActionMenu'>"
          "      <menuitem action='Run'/>"
          "      <separator/>"
          "      <menuitem action='Help'/>"
          "      <separator/>"
          "      <menuitem action='Exit'/>"
          "    </menu>"
          "  </menubar>"
          "</ui>";

#if (defined(GLIBMM_EXCEPTIONS_ENABLED) || (defined(ARTEMIS)))
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
#endif

    //Get the menubar widget, and add it to a container widget:
    Gtk::Widget* pMenubar = m_refUIManager->get_widget("/MenuBar");
    if(pMenubar)
      vBox.pack_start(*pMenubar, Gtk::PACK_SHRINK);

    vBox.pack_start(HSEGOparam);
    bool1Box.pack_start(bool1Label);
    bool1Box.pack_start(bool1ComboBox);
    vBox.pack_start(bool1Box);
    bool2Box.pack_start(bool2Label);
    bool2Box.pack_start(bool2ComboBox);
    vBox.pack_start(bool2Box);
    vBox.pack_start(debugObject);
    vBox.pack_start(logFile);

    show_all_children();

    show();

    return;
  }

 // Destructor...
  ParamsGUI::~ParamsGUI() 
  {
    return;
  }

  void ParamsGUI::set_oparam_file(const string& file_name)
  {
    HSEGOparam.set_filename(file_name);
    on_HSEGOparam_selection_changed();

    return;
  }

  void ParamsGUI::on_run_program()
  {
    string tmp_string;

    params.status = initialParams.oparam_flag;
    if (!params.status)
    {
      Glib::ustring strMessage = "Invalid HSeg/RHSeg Output Parameter File (oparam).\n"
                                 "Please reselect a valid file.";
      Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
     
      dialog.run();
      
      return;
    }

    tmp_string = bool1ComboBox.get_active_text();
    if (tmp_string == "YES")
      initialParams.region_class_nghbrs_list_flag = true;
    else
      initialParams.region_class_nghbrs_list_flag = false;
    tmp_string = bool2ComboBox.get_active_text();
    if (tmp_string == "YES")
      initialParams.region_object_nghbrs_list_flag = true;
    else
      initialParams.region_object_nghbrs_list_flag = false;
    initialParams.debug = debugObject.get_value();
    initialParams.log_file = logFile.get_filename();

    hide();

    return;
  }

  void ParamsGUI::on_help_requested()
  {
    Glib::ustring strMessage = "Enter in program parameters and run the program by\n"
                               "selecting the \"Run Program\" menu item under the\n"
                               "\"Program Actions\" menu.\n\nFor more help type "
                               "\"hsegreader -help\" on the command line.\n\n";
    Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
    
    dialog.run();
  }

  void ParamsGUI::on_exit_requested()
  {
    params.status = false;
    hide();
  }

  void ParamsGUI::on_HSEGOparam_selection_changed()
  {
    initialParams.oparam_file = HSEGOparam.get_filename();

   // Read initial parameters from HSeg/RHSeg output parameter file
    params.status = params.read_init(initialParams.oparam_file.c_str());

   // Read additional parameters from HSeg/RHSeg output parameter file
    if (params.status)
      params.status = params.read(initialParams.oparam_file.c_str());
 
   // Read "output" parameters from HSeg/RHSeg output parameter file
    if (params.status)
      params.status = initialParams.read_oparam();

    if (params.status)
    {
      initialParams.oparam_flag = true;
#ifdef WINDOWS
      string slash = "\\";
#else
      string slash = "/";
#endif
      if (params.current_folder != "")
      {
        initialParams.log_file = params.current_folder + slash + params.log_file;
        logFile.set_current_folder(params.current_folder);
      }
      logFile.set_filename(initialParams.log_file);
    }
    else
    {
      initialParams.oparam_flag = false;
      HSEGOparam.set_filename("");
      Glib::ustring strMessage = "Invalid HSeg/RHSeg Output Parameter File (oparam).\n"
                                 "Please reselect a valid file.";
      Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
    
      dialog.run();
    }
    
    return;
  }

} // namespace HSEGTilton
