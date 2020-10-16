// paramsGUI.cc
#include "paramsGUI.h"
#include "initialParams.h"
#include <iostream>

extern HSEGTilton::InitialParams initialParams;
extern HSEGTilton::Params params;
extern HSEGTilton::oParams oparams;

namespace HSEGTilton
{
 // Constructor
  ParamsGUI::ParamsGUI():
             vBox(false,10), hBox(false,10),
             HSEGOparam(" HSeg/RHSeg Output Parameter File (oparam) for Input to HSegExtract:",
                        "Enter or Select the HSeg/RHSeg Output Paramter File (oparam)",
                        Gtk::FILE_CHOOSER_ACTION_OPEN),
             segLevelLabel("Hierarchical Segmentation Level for Feature Extraction:",1.0),
             classLabelsMapExtFile(" Output Class Labels Map:",
                        "Enter or Select the Output Class Labels Feature Map File",
                        Gtk::FILE_CHOOSER_ACTION_SAVE,false),
             classNpixMapExtFile(" Output Class Number of Pixels Feature Map:",
                        "Enter or Select the Output Class Number of Pixels Map File",
                        Gtk::FILE_CHOOSER_ACTION_SAVE,false),
             classMeanMapExtFile(" Output Class Region Mean Feature Map:",
                        "Enter or Select the Output Class Region Mean Map File",
                        Gtk::FILE_CHOOSER_ACTION_SAVE,false),
             classStdDevMapExtFile(" Output Class Standard Deviation Feature Map:",
                        "Enter or Select the Output Class Standard Deviation Map File",
                        Gtk::FILE_CHOOSER_ACTION_SAVE,false),
             classBPRatioMapExtFile(" Output Class Boundary Pixel Ratio Feature Map:",
                        "Enter or Select the Output Class Boundary Pixel Ratio Map File",
                        Gtk::FILE_CHOOSER_ACTION_SAVE,false),
#ifdef SHAPEFILE
             classShapefileExtFile(" Output Class Shapefile Base File Name:",
                        "Enter or Select the Output Class Shapefile Base File Name",
                        Gtk::FILE_CHOOSER_ACTION_SAVE,false),
#endif
             objectLabelsMapExtFile(" Output Object Labels Map:",
                        "Enter or Select the Output Object Labels Feature Map File",
                        Gtk::FILE_CHOOSER_ACTION_SAVE,false),
             objectNpixMapExtFile(" Output Object Number of Pixels Feature Map:",
                        "Enter or Select the Output Object Number of Pixels Map File",
                        Gtk::FILE_CHOOSER_ACTION_SAVE,false),
             objectMeanMapExtFile(" Output Object Region Mean Feature Map:",
                        "Enter or Select the Output Object Region Mean Map File",
                        Gtk::FILE_CHOOSER_ACTION_SAVE,false),
             objectStdDevMapExtFile(" Output Object Standard Deviation Feature Map:",
                        "Enter or Select the Output Object Standard Deviation Map File",
                        Gtk::FILE_CHOOSER_ACTION_SAVE,false),
#ifdef SHAPEFILE
             objectBPRatioMapExtFile(" Output Object Boundary Pixel Ratio Feature Map:",
                        "Enter or Select the Output Object Boundary Pixel Ratio Map File",
                        Gtk::FILE_CHOOSER_ACTION_SAVE,false),
             objectShapefileExtFile(" Output Object Shapefile Base File Name:",
                        "Enter or Select the Output Object Shapefile Base File Name",
                        Gtk::FILE_CHOOSER_ACTION_SAVE,false)
#else
             objectBPRatioMapExtFile(" Output Object Boundary Pixel Ratio Feature Map:",
                        "Enter or Select the Output Object Boundary Pixel Ratio Map File",
                        Gtk::FILE_CHOOSER_ACTION_SAVE,false)
#endif
  {
    set_title("Hierarchical Segmentation Feature Extract Parameter Input");
    set_default_size(512,64);

    char *cwd;
    cwd = g_get_current_dir();
    params.current_folder = cwd;
    HSEGOparam.add_shortcut_folder(params.current_folder);
    HSEGOparam.set_current_folder(params.current_folder);
    g_free(cwd);

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
    hBox.pack_start(segLevelLabel);
    hBox.pack_start(segLevelComboBox);
    vBox.pack_start(hBox);
    vBox.pack_start(classLabelsMapExtFile);
    vBox.pack_start(classNpixMapExtFile);
    vBox.pack_start(classMeanMapExtFile);
    vBox.pack_start(classStdDevMapExtFile);
    vBox.pack_start(classBPRatioMapExtFile);
#ifdef SHAPEFILE
    vBox.pack_start(classShapefileExtFile);
#endif
    vBox.pack_start(objectLabelsMapExtFile);
    vBox.pack_start(objectNpixMapExtFile);
    vBox.pack_start(objectMeanMapExtFile);
    vBox.pack_start(objectStdDevMapExtFile);
    vBox.pack_start(objectBPRatioMapExtFile);
#ifdef SHAPEFILE
    vBox.pack_start(objectShapefileExtFile);
#endif

    show_all_children();

    classLabelsMapExtFile.set_activeFlag(false);
    classNpixMapExtFile.set_activeFlag(false);
    classMeanMapExtFile.set_activeFlag(false);
    classStdDevMapExtFile.set_activeFlag(false);
    classBPRatioMapExtFile.set_activeFlag(false);
#ifdef SHAPEFILE
    classShapefileExtFile.set_activeFlag(false);
#endif
    objectLabelsMapExtFile.set_activeFlag(false);
    objectNpixMapExtFile.set_activeFlag(false);
    objectMeanMapExtFile.set_activeFlag(false);
    objectStdDevMapExtFile.set_activeFlag(false);
    objectBPRatioMapExtFile.set_activeFlag(false);
#ifdef SHAPEFILE
    objectShapefileExtFile.set_activeFlag(false);
#endif

    segLevelLabel.hide();
    segLevelComboBox.hide();
    classLabelsMapExtFile.hide();
    classNpixMapExtFile.hide();
    classMeanMapExtFile.hide();
    classStdDevMapExtFile.hide();
    classBPRatioMapExtFile.hide();
#ifdef SHAPEFILE
    classShapefileExtFile.hide();
#endif
    objectLabelsMapExtFile.hide();
    objectNpixMapExtFile.hide();
    objectMeanMapExtFile.hide();
    objectStdDevMapExtFile.hide();
    objectBPRatioMapExtFile.hide();
#ifdef SHAPEFILE
    objectShapefileExtFile.hide();
#endif

    show();
  }

 // Destructor...
  ParamsGUI::~ParamsGUI() 
  {
  }

  void ParamsGUI::set_oparam_file(const string& file_name)
  {
    HSEGOparam.set_filename(file_name);
    on_HSEGOparam_selection_changed();
  }

  void ParamsGUI::on_run_program()
  {
    string tmp_string;

    params.status = initialParams.oparam_flag;
    if (!params.status)
    {
      HSEGOparam.set_filename("");
      Glib::ustring strMessage = "Invalid HSeg/RHSeg Output Parameter File (oparam).\n"
                                 "Please reselect a valid file.";
      Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
    
      dialog.run();

      return;
    }

    tmp_string = segLevelComboBox.get_active_text();
    params.status = (tmp_string != "(select level)");
    if (!params.status)
    {
      Glib::ustring strMessage = "You must select a segmentation level!";
      Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
    
      dialog.run();

      return;
    }

    initialParams.hseg_level = atoi(tmp_string.c_str());

    initialParams.class_labels_map_ext_file = classLabelsMapExtFile.get_filename();
    initialParams.class_labels_map_ext_flag = (initialParams.class_labels_map_ext_file != "(Select File)");
    initialParams.class_npix_map_ext_file = classNpixMapExtFile.get_filename();
    initialParams.class_npix_map_ext_flag = (initialParams.class_npix_map_ext_file != "(Select File)");
    if (params.region_sum_flag)
    {
      initialParams.class_mean_map_ext_file = classMeanMapExtFile.get_filename();
      initialParams.class_mean_map_ext_flag = (initialParams.class_mean_map_ext_file != "(Select File)");
    }
    else
      initialParams.class_mean_map_ext_flag = false;
    if (params.region_std_dev_flag)
    {
      initialParams.class_std_dev_map_ext_file = classStdDevMapExtFile.get_filename();
      initialParams.class_std_dev_map_ext_flag = (initialParams.class_std_dev_map_ext_file != "(Select File)");
    }
    else
      initialParams.class_std_dev_map_ext_flag = false;
    if (params.region_boundary_npix_flag)
    {
      initialParams.class_bpratio_map_ext_file = classBPRatioMapExtFile.get_filename();
      initialParams.class_bpratio_map_ext_flag = (initialParams.class_bpratio_map_ext_file != "(Select File)");
    }
    else
      initialParams.class_bpratio_map_ext_flag = false;
#ifdef SHAPEFILE
    initialParams.class_shapefile_ext_file = classShapefileExtFile.get_filename();
    initialParams.class_shapefile_ext_flag = (initialParams.class_shapefile_ext_file != "(Select File)");
#endif

    if (params.object_labels_map_flag)
    {
      initialParams.object_labels_map_ext_file = objectLabelsMapExtFile.get_filename();
      initialParams.object_labels_map_ext_flag = (initialParams.object_labels_map_ext_file != "(Select File)");
      initialParams.object_npix_map_ext_file = objectNpixMapExtFile.get_filename();
      initialParams.object_npix_map_ext_flag = (initialParams.object_npix_map_ext_file != "(Select File)");
      if (params.region_sum_flag)
      {  
        initialParams.object_mean_map_ext_file = objectMeanMapExtFile.get_filename();
        initialParams.object_mean_map_ext_flag = (initialParams.object_mean_map_ext_file != "(Select File)");
      }
      else
        initialParams.object_mean_map_ext_flag = false;
      if (params.region_std_dev_flag)
      {
        initialParams.object_std_dev_map_ext_file = objectStdDevMapExtFile.get_filename();
        initialParams.object_std_dev_map_ext_flag = (initialParams.object_std_dev_map_ext_file != "(Select File)");
      }
      else
        initialParams.object_std_dev_map_ext_flag = false;
      if (params.region_boundary_npix_flag)
      {
        initialParams.object_bpratio_map_ext_file = objectBPRatioMapExtFile.get_filename();
        initialParams.object_bpratio_map_ext_flag = (initialParams.object_bpratio_map_ext_file != "(Select File)");
      }
      else
        initialParams.object_bpratio_map_ext_flag = false;
#ifdef SHAPEFILE
      initialParams.object_shapefile_ext_file = objectShapefileExtFile.get_filename();
      initialParams.object_shapefile_ext_flag = (initialParams.object_shapefile_ext_file != "(Select File)");
#endif
    }

    params.status = (initialParams.class_labels_map_ext_flag || initialParams.class_npix_map_ext_flag || 
                     initialParams.class_mean_map_ext_flag || initialParams.class_std_dev_map_ext_flag || 
                     initialParams.class_bpratio_map_ext_flag || initialParams.object_labels_map_ext_flag || 
                     initialParams.object_npix_map_ext_flag || initialParams.object_mean_map_ext_flag ||
#ifdef SHAPEFILE
                     initialParams.object_std_dev_map_ext_flag || initialParams.object_bpratio_map_ext_flag ||
                     initialParams.class_shapefile_ext_flag || initialParams.object_shapefile_ext_flag);
#else
                     initialParams.object_std_dev_map_ext_flag || initialParams.object_bpratio_map_ext_flag);
#endif
    if (params.status)
    {
      hide();
    }
    else
    {
      Glib::ustring strMessage = "You must specify at least one output!";
      Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
    
      dialog.run();
    }

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
      initialParams.oparam_flag = true;
    else
    {
      initialParams.oparam_flag = false;
      HSEGOparam.set_filename("");
      Glib::ustring strMessage = "Invalid HSeg/RHSeg Output Parameter File (oparam).\n"
                                 "Please reselect a valid file.";
      Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
    
      dialog.run();

      return;
    }

    int hlevel;
#ifdef GTKMM3
    segLevelComboBox.append("(select level)");
    for (hlevel = 0; hlevel < oparams.nb_levels; hlevel++)
      segLevelComboBox.append("                   " + stringify_int(hlevel));
#else
    segLevelComboBox.append_text("(select level)");
    for (hlevel = 0; hlevel < oparams.nb_levels; hlevel++)
      segLevelComboBox.append_text("                   " + stringify_int(hlevel));
#endif
#ifdef ARTEMIS
    segLevelComboBox.set_active(0);
#else
    segLevelComboBox.set_active_text("(select level)");
#endif

    segLevelLabel.show();
    segLevelComboBox.show();
    classLabelsMapExtFile.show();
    classNpixMapExtFile.show();
    if (params.region_sum_flag)
      classMeanMapExtFile.show();
    if (params.region_std_dev_flag)
      classStdDevMapExtFile.show();
    if (params.region_boundary_npix_flag)
      classBPRatioMapExtFile.show();
#ifdef SHAPEFILE
    classShapefileExtFile.show();
#endif
    if (params.object_labels_map_flag)
    {
      objectLabelsMapExtFile.show();
      objectNpixMapExtFile.show();
      if (params.region_sum_flag)
        objectMeanMapExtFile.show();
      if (params.region_std_dev_flag)
        objectStdDevMapExtFile.show();
      if (params.region_boundary_npix_flag)
        objectBPRatioMapExtFile.show();
#ifdef SHAPEFILE
      objectShapefileExtFile.show();
#endif
    }

    return;
  }

} // namespace HSEGTilton
