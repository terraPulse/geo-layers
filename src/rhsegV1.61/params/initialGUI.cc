// initialGUI.cc

#include "initialGUI.h"
#include "params.h"
#include <iostream>
#include <fstream>

extern HSEGTilton::Params params;
#ifdef GDAL
extern Image inputImage;
extern Image maskImage;
extern Image stdDevInImage;
extern Image stdDevMaskImage;
extern Image edgeInImage;
extern Image edgeMaskImage;
extern Image regionMapInImage;
#endif

namespace HSEGTilton
{
 // Constructor
  InitialGUI::InitialGUI():
               vBox(false,10),
               modeLabel("Select Program Mode:"),
#ifdef GDAL
               inputFilesFrame("Specify Input Files:"),
#else
               inputFilesFrame("Specify Input Files and descriptive parameters:"),
#endif
               inputFilesBox(false,10),
               optionalLabel("(Click on check box to specify optional files)",0.0),
               inputImageFile(" Input image data file (input_image):",
                              "Enter or Select the input image data file (input_image)",
                              Gtk::FILE_CHOOSER_ACTION_OPEN),
#ifndef GDAL
               numberOfColumns("Specify the number of columns in the input image data (ncols):"),
               numberOfRows("Specify the number of rows in the input image data (nrows):"),
#ifdef THREEDIM
               numberOfSlices("Specify the number of slices in the input image data (nslices):"),
#endif
               numberOfBands("Specify the number of spectral bands in the input image data (nbands):"),
               dtypeLabel("Select the data type of the input image data (dtype):"),
#endif // !GDAL
               maskImageFile(" Input mask data file (mask):",
                             "Enter or Select the input mask data file (mask)",
                             Gtk::FILE_CHOOSER_ACTION_OPEN,false),
#ifndef GDAL
               maskValue("Specify the value in the mask data that designates bad data (mask_value)"),
#endif
               stdDevInImageFile(" Input standard deviation image file (std_dev_image):",
                                    "Enter or Select the input standard deviation image file (std_dev_image)",
                                    Gtk::FILE_CHOOSER_ACTION_OPEN,false),
               stdDevMaskImageFile(" Input standard deviation mask file (std_dev_mask):",
                                    "Enter or Select the input standard deviation mask file (std_dev_mask)",
                                    Gtk::FILE_CHOOSER_ACTION_OPEN,false),
               edgeInImageFile(" Input edge image file (edge_image):",
                                    "Enter or Select the input edge image file (edge_image)",
                                    Gtk::FILE_CHOOSER_ACTION_OPEN,false),
               edgeMaskImageFile(" Input edge mask file (edge_mask):",
                                    "Enter or Select the input edge mask file (edge_mask)",
                                    Gtk::FILE_CHOOSER_ACTION_OPEN,false),
               regionMapInImageFile(" Input region map data file (region_map_in):",
                                    "Enter or Select the input region map data file (region_map_in)",
                                    Gtk::FILE_CHOOSER_ACTION_OPEN,false),
               spClustWght("Specify the relative importance of\nspectral clustering vs. region growing\n(spclust_wght, range 0.0 to 1.0):"),
               dissimCritLabel("Dissimilarity Criterion (dissim_crit):"),
               logFile(" Output log file (log):",
                       "Enter or Select the output log file (log)",
                       Gtk::FILE_CHOOSER_ACTION_SAVE),
               logLabel("All output files will be placed in the same directory where\nthe output log file is located (unless otherwise specified)"),
               copyrightLabel("Copyright \u00a9 2006 United States Government as represented by the\nAdministrator of the National Aeronautics and Space Administration.\nNo copyright is claimed in the United States under Title 17, U.S. Code.\nAll Other Rights Reserved.\n")
  {
    set_title("Initial HSWO/HSeg/RHSeg File and Parameter Specification");
    set_default_size(512,64);

    char *cwd;
    cwd = g_get_current_dir();
    current_folder = cwd;
    inputImageFile.add_shortcut_folder(current_folder);
    inputImageFile.set_current_folder(current_folder);
    maskImageFile.add_shortcut_folder(current_folder);
    maskImageFile.set_current_folder(current_folder);
    stdDevInImageFile.add_shortcut_folder(current_folder);
    stdDevInImageFile.set_current_folder(current_folder);
    stdDevMaskImageFile.add_shortcut_folder(current_folder);
    stdDevMaskImageFile.set_current_folder(current_folder);
    edgeInImageFile.add_shortcut_folder(current_folder);
    edgeInImageFile.set_current_folder(current_folder);
    edgeMaskImageFile.add_shortcut_folder(current_folder);
    edgeMaskImageFile.set_current_folder(current_folder);
    regionMapInImageFile.add_shortcut_folder(current_folder);
    regionMapInImageFile.set_current_folder(current_folder);
    logFile.add_shortcut_folder(current_folder);
    logFile.set_current_folder(current_folder);
    g_free(cwd);

  // Initialize other widgets
#ifdef GTKMM3
    modeComboBox.append("(Select Mode)");
    modeComboBox.append("    HSWO    ");
    modeComboBox.append("    HSEG    ");
    modeComboBox.append("   RHSEG    ");
#ifndef GDAL
    dtypeComboBox.append("Unsigned Char (UInt8)");
    dtypeComboBox.append("Unsigned Short (UInt16)");
    dtypeComboBox.append("Float (Float32)");
#endif // !GDAL
    dissimCritComboBox.append("       1-Norm");
    dissimCritComboBox.append("       2-Norm");
    dissimCritComboBox.append("Infinity Norm");
    dissimCritComboBox.append("Spectral Angle Mapper");
    dissimCritComboBox.append("Spectral Information Divergence");
#ifdef MSE_SQRT
    dissimCritComboBox.append("Square Root of Band Sum Mean Squared Error");
    dissimCritComboBox.append("Square Root of Band Maximum Mean Squared Error");
#else
    dissimCritComboBox.append("Band Sum Mean Squared Error");
    dissimCritComboBox.append("Band Maximum Mean Squared Error");
#endif
    dissimCritComboBox.append("Normalized Vector Distance");
    dissimCritComboBox.append("      Entropy");
    dissimCritComboBox.append("SAR Speckle Noise Criterion");
#else
    modeComboBox.append_text("(Select Mode)");
    modeComboBox.append_text("    HSWO    ");
    modeComboBox.append_text("    HSEG    ");
    modeComboBox.append_text("   RHSEG    ");
#ifndef GDAL
    dtypeComboBox.append_text("Unsigned Char (UInt8)");
    dtypeComboBox.append_text("Unsigned Short (UInt16)");
    dtypeComboBox.append_text("Float (Float32)");
#endif // !GDAL
    dissimCritComboBox.append_text("       1-Norm");
    dissimCritComboBox.append_text("       2-Norm");
    dissimCritComboBox.append_text("Infinity Norm");
    dissimCritComboBox.append_text("Spectral Angle Mapper");
    dissimCritComboBox.append_text("Spectral Information Divergence");
#ifdef MSE_SQRT
    dissimCritComboBox.append_text("Square Root of Band Sum Mean Squared Error");
    dissimCritComboBox.append_text("Square Root of Band Maximum Mean Squared Error");
#else
    dissimCritComboBox.append_text("Band Sum Mean Squared Error");
    dissimCritComboBox.append_text("Band Maximum Mean Squared Error");
#endif
    dissimCritComboBox.append_text("Normalized Vector Distance");
    dissimCritComboBox.append_text("      Entropy");
    dissimCritComboBox.append_text("SAR Speckle Noise Criterion");
#endif
    modeComboBox.set_active_text("(Select Mode)");
#ifndef GDAL
    dtypeComboBox.set_active_text("Unsigned Char (UInt8)");
    params.mask_value = MASK_VALUE;
    maskValue.set_value(params.mask_value);
#endif // !GDAL
    params.dissim_crit = DISSIM_CRIT;
    switch (params.dissim_crit)
    {
      case 1:  dissimCritComboBox.set_active_text("       1-Norm");
               break;
      case 2:  dissimCritComboBox.set_active_text("       2-Norm");
               break;
      case 3:  dissimCritComboBox.set_active_text("Infinity Norm");
               break;
      case 4:  dissimCritComboBox.set_active_text("Spectral Angle Mapper");
               break;
      case 5:  dissimCritComboBox.set_active_text("Spectral Information Divergence");
               break;
#ifdef MSE_SQRT
      case 6:  dissimCritComboBox.set_active_text("Square Root of Band Sum Mean Squared Error");
               break;
      case 7:  dissimCritComboBox.set_active_text("Square Root of Band Maximum Mean Squared Error");
               break;
#else
      case 6:  dissimCritComboBox.set_active_text("Band Sum Mean Squared Error");
               break;
      case 7:  dissimCritComboBox.set_active_text("Band Maximum Mean Squared Error");
               break;
#endif
      case 8:  dissimCritComboBox.set_active_text("Normalized Vector Distance");
               break;
      case 9:  dissimCritComboBox.set_active_text("      Entropy");
               break;
      case 10: dissimCritComboBox.set_active_text("SAR Speckle Noise Criterion");
               break;
//      case 11: dissimCritComboBox.set_active_text("Feature Range");
//               break;
#ifdef MSE_SQRT
      default: dissimCritComboBox.set_active_text("Square Root of Band Sum Mean Squared Error");
#else
      default: dissimCritComboBox.set_active_text("Band Sum Mean Squared Error");
#endif
               break;
    }

  // Set up signal handlers
    modeComboBox.signal_changed().connect(sigc::mem_fun(*this,
                    &InitialGUI::on_modeComboBox_changed) );
    inputImageFile.signal_selection_changed().connect(sigc::mem_fun(*this,
                    &InitialGUI::on_inputImageFile_selection_changed) );
    maskImageFile.signal_selection_changed().connect(sigc::mem_fun(*this,
                    &InitialGUI::on_maskImageFile_selection_changed) );
    stdDevInImageFile.signal_selection_changed().connect(sigc::mem_fun(*this,
                    &InitialGUI::on_stdDevInImageFile_selection_changed) );
    stdDevMaskImageFile.signal_selection_changed().connect(sigc::mem_fun(*this,
                    &InitialGUI::on_stdDevMaskImageFile_selection_changed) );
    edgeInImageFile.signal_selection_changed().connect(sigc::mem_fun(*this,
                    &InitialGUI::on_edgeInImageFile_selection_changed) );
    edgeMaskImageFile.signal_selection_changed().connect(sigc::mem_fun(*this,
                    &InitialGUI::on_edgeMaskImageFile_selection_changed) );
    regionMapInImageFile.signal_selection_changed().connect(sigc::mem_fun(*this,
                    &InitialGUI::on_regionMapInImageFile_selection_changed) );
    logFile.signal_selection_changed().connect(sigc::mem_fun(*this,
                    &InitialGUI::on_logFile_selection_changed) );

    add(vBox); // put a MenuBar at the top of the box and other stuff below it.

  //Create actions for menus:
    m_refActionGroup = Gtk::ActionGroup::create();

    //Add normal Actions:
    m_refActionGroup->add( Gtk::Action::create("ActionMenu", "Program _Actions") );
    m_refActionGroup->add( Gtk::Action::create("Help", "_Help"),
                           sigc::mem_fun(*this, &InitialGUI::on_help_requested) );
    m_refActionGroup->add( Gtk::Action::create("Version", "_Version"),
                           sigc::mem_fun(*this, &InitialGUI::on_version_requested) );
    m_refActionGroup->add( Gtk::Action::create("Go", "_Go To Next Panel"),
                           sigc::mem_fun(*this, &InitialGUI::on_next_panel) );
#ifdef RHSEG_SETUP
    m_refActionGroup->add( Gtk::Action::create("Run", "_Run RHSeg Setup"),
                           sigc::mem_fun(*this, &InitialGUI::on_run_program) );
#else
    m_refActionGroup->add( Gtk::Action::create("Run", "_Run Program"),
                           sigc::mem_fun(*this, &InitialGUI::on_run_program) );
#endif
    m_refActionGroup->add( Gtk::Action::create("Exit", "_Exit"),
                           sigc::mem_fun(*this, &InitialGUI::on_exit_requested) );

    m_refUIManager = Gtk::UIManager::create();
    m_refUIManager->insert_action_group(m_refActionGroup);
    add_accel_group(m_refUIManager->get_accel_group());

    //Layout the actions in the menubar:
    Glib::ustring ui_info = 
          "<ui>"
          "  <menubar name='MenuBar'>"
          "    <menu action='ActionMenu'>"
          "      <menuitem action='Help'/>"
          "      <menuitem action='Version'/>"
          "      <separator/>"
          "      <menuitem action='Go'/>"
          "      <menuitem action='Run'/>"
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

    modeHBox.pack_start(modeLabel);
    modeHBox.pack_start(modeComboBox);
    vBox.pack_start(modeHBox);
    inputFilesBox.pack_start(inputImageFile);
#ifndef GDAL
    inputFilesBox.pack_start(numberOfColumns);
    inputFilesBox.pack_start(numberOfRows);
#ifdef THREEDIM
    inputFilesBox.pack_start(numberOfSlices);
#endif
    inputFilesBox.pack_start(numberOfBands);
#endif // !GDAL
    inputFilesBox.pack_start(optionalLabel);
    inputFilesBox.pack_start(maskImageFile);
#ifndef GDAL
    inputFilesBox.pack_start(maskValue);
#endif // !GDAL
    inputFilesBox.pack_start(stdDevInImageFile);
    inputFilesBox.pack_start(stdDevMaskImageFile);
    inputFilesBox.pack_start(edgeInImageFile);
    inputFilesBox.pack_start(edgeMaskImageFile);
    inputFilesBox.pack_start(regionMapInImageFile);
    inputFilesFrame.add(inputFilesBox);
    vBox.pack_start(inputFilesFrame);
    vBox.pack_start(spClustWght);
    comboHBox.pack_start(dissimCritLabel);
    comboHBox.pack_start(dissimCritComboBox);
    vBox.pack_start(comboHBox);
    vBox.pack_start(logFile);
    vBox.pack_start(logLabel);
    vBox.pack_start(copyrightLabel);

    show_all_children();
    maskImageFile.set_activeFlag(false);
    stdDevInImageFile.set_activeFlag(false);
    stdDevMaskImageFile.set_activeFlag(false);
    edgeInImageFile.set_activeFlag(false);
    edgeMaskImageFile.set_activeFlag(false);
    regionMapInImageFile.set_activeFlag(false);

    inputFilesFrame.hide();
    spClustWght.hide();
    comboHBox.hide();
    logFile.hide();
    logLabel.hide();

    show();

    input_image_flag = false;
    log_flag = false;

    return;
  }

 // Destructor...
  InitialGUI::~InitialGUI() 
  {
  }

  void InitialGUI::on_help_requested()
  {
    Glib::ustring strMessage = "Enter in the program File inputs.\n\nFor more help type "
                               "\"rhseg -help\" on the command line.\n\n";
    Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
    
    dialog.run();

    return;
  }

  void InitialGUI::on_version_requested()
  {
    Glib::ustring strMessage = params.version + "\n";
    Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
    
    dialog.run();
  }

  void InitialGUI::on_exit_requested()
  {
    params.status = 3;
    hide();
    return;
  }

  bool InitialGUI::on_delete_event(GdkEventAny* event)
  {
    on_exit_requested();
    return true;
  }

  void InitialGUI::on_modeComboBox_changed()
  {
    string tmp_string;

    tmp_string = modeComboBox.get_active_text();

    if (tmp_string == "(Select Mode)")
    {
      params.program_mode = 0;
      inputFilesFrame.hide();
      spClustWght.hide();
      comboHBox.hide();
      logFile.hide();
      logLabel.hide();
    }
    else if (tmp_string == "    HSWO    ")
    {
      params.program_mode = 1;
      inputFilesFrame.show();
      spClustWght.hide();
      comboHBox.show();
      logFile.show();
      logLabel.show();
    }
    else if (tmp_string == "    HSEG    ")
    {
      params.program_mode = 2;
      inputFilesFrame.show();
      spClustWght.show();
      comboHBox.show();
      logFile.show();
      logLabel.show();
    }
    else if (tmp_string == "   RHSEG    ")
    {
      params.program_mode = 3;
      inputFilesFrame.show();
      edgeInImageFile.set_activeFlag(true);
      spClustWght.show();
      comboHBox.show();
      logFile.show();
      logLabel.show();
    }

    return;
  }

  void InitialGUI::on_inputImageFile_selection_changed()
  {
#ifdef GDAL
    Glib::ustring strMessage = " ";
#endif
    params.input_image_file = inputImageFile.get_filename();
#ifdef GDAL
    inputImage.open(params.input_image_file);
    if (inputImage.info_valid())
    {
#endif
      input_image_flag = true;
      if (!log_flag)
      {
        set_current_folder(params.input_image_file);
        inputImageFile.set_current_folder(current_folder);
        maskImageFile.set_current_folder(current_folder);
        stdDevInImageFile.set_current_folder(current_folder);
        stdDevMaskImageFile.set_current_folder(current_folder);
        edgeInImageFile.set_current_folder(current_folder);
        edgeMaskImageFile.set_current_folder(current_folder);
        regionMapInImageFile.set_current_folder(current_folder);
        logFile.set_current_folder(current_folder);
      }
#ifdef GDAL
    }
    else
    {
      strMessage += "\nInvalid input image data file, please reenter.\n";
      Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
      end_dialog.run();
      inputImageFile.set_filename("");
    }
#endif
    return;
  }

  void InitialGUI::on_maskImageFile_selection_changed()
  {
#ifdef GDAL
    Glib::ustring strMessage = " ";
#endif
    params.mask_file = maskImageFile.get_filename();
#ifdef GDAL
    maskImage.open(params.mask_file);
    if (maskImage.info_valid())
    {
#endif
      params.mask_flag = true;
      if ((!log_flag) && (!input_image_flag) && (!params.region_map_in_flag))
      {
        set_current_folder(params.mask_file);
        inputImageFile.set_current_folder(current_folder);
        maskImageFile.set_current_folder(current_folder);
        stdDevInImageFile.set_current_folder(current_folder);
        stdDevMaskImageFile.set_current_folder(current_folder);
        edgeInImageFile.set_current_folder(current_folder);
        edgeMaskImageFile.set_current_folder(current_folder);
        regionMapInImageFile.set_current_folder(current_folder);
        logFile.set_current_folder(current_folder);
      }
#ifdef GDAL
    }
    else
    {
      strMessage += "\nInvalid input mask data file, please reenter.\n";
      Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
      end_dialog.run();
      maskImageFile.set_filename("");
    }
#endif
    return;
  }

  void InitialGUI::on_stdDevInImageFile_selection_changed()
  {
#ifdef GDAL
    Glib::ustring strMessage = " ";
#endif
    params.std_dev_image_file = stdDevInImageFile.get_filename();
#ifdef GDAL
    stdDevInImage.open(params.std_dev_image_file);
    if (stdDevInImage.info_valid())
    {
#endif
      params.std_dev_image_flag = true;
      if ((!log_flag) && (!input_image_flag) && (!params.mask_flag))
      {
        set_current_folder(params.std_dev_image_file);
        inputImageFile.set_current_folder(current_folder);
        maskImageFile.set_current_folder(current_folder);
        stdDevInImageFile.set_current_folder(current_folder);
        stdDevMaskImageFile.set_current_folder(current_folder);
        edgeInImageFile.set_current_folder(current_folder);
        edgeMaskImageFile.set_current_folder(current_folder);
        regionMapInImageFile.set_current_folder(current_folder);
        logFile.set_current_folder(current_folder);
      }
#ifdef GDAL
    }
    else
    {
      strMessage += "\nInvalid input standard deviation image file, please reenter.\n";
      Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
      end_dialog.run();
      stdDevInImageFile.set_filename("");
    }
#endif
    return;
  }

  void InitialGUI::on_stdDevMaskImageFile_selection_changed()
  {
#ifdef GDAL
    Glib::ustring strMessage = " ";
#endif
    params.std_dev_mask_file = stdDevMaskImageFile.get_filename();
#ifdef GDAL
    stdDevMaskImage.open(params.std_dev_mask_file);
    if (stdDevMaskImage.info_valid())
    {
#endif
      params.std_dev_mask_flag = true;
      if ((!log_flag) && (!input_image_flag) && (!params.mask_flag))
      {
        set_current_folder(params.std_dev_mask_file);
        inputImageFile.set_current_folder(current_folder);
        maskImageFile.set_current_folder(current_folder);
        stdDevInImageFile.set_current_folder(current_folder);
        stdDevMaskImageFile.set_current_folder(current_folder);
        edgeInImageFile.set_current_folder(current_folder);
        edgeMaskImageFile.set_current_folder(current_folder);
        regionMapInImageFile.set_current_folder(current_folder);
        logFile.set_current_folder(current_folder);
      }
#ifdef GDAL
    }
    else
    {
      strMessage += "\nInvalid input standard deviation mask file, please reenter.\n";
      Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
      end_dialog.run();
      stdDevMaskImageFile.set_filename("");
    }
#endif
    return;
  }

  void InitialGUI::on_edgeInImageFile_selection_changed()
  {
#ifdef GDAL
    Glib::ustring strMessage = " ";
#endif
    params.edge_image_file = edgeInImageFile.get_filename();
#ifdef GDAL
    edgeInImage.open(params.edge_image_file);
    if (edgeInImage.info_valid())
    {
#endif
      params.edge_image_flag = true;
      if ((!log_flag) && (!input_image_flag) && (!params.mask_flag))
      {
        set_current_folder(params.edge_image_file);
        inputImageFile.set_current_folder(current_folder);
        maskImageFile.set_current_folder(current_folder);
        stdDevInImageFile.set_current_folder(current_folder);
        stdDevMaskImageFile.set_current_folder(current_folder);
        edgeInImageFile.set_current_folder(current_folder);
        edgeMaskImageFile.set_current_folder(current_folder);
        regionMapInImageFile.set_current_folder(current_folder);
        logFile.set_current_folder(current_folder);
      }
#ifdef GDAL
    }
    else
    {
      strMessage += "\nInvalid input edge image file, please reenter.\n";
      Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
      end_dialog.run();
      edgeInImageFile.set_filename("");
    }
#endif
    return;
  }

  void InitialGUI::on_edgeMaskImageFile_selection_changed()
  {
#ifdef GDAL
    Glib::ustring strMessage = " ";
#endif
    params.edge_mask_file = edgeMaskImageFile.get_filename();
#ifdef GDAL
    edgeMaskImage.open(params.edge_mask_file);
    if (edgeMaskImage.info_valid())
    {
#endif
      params.edge_mask_flag = true;
      if ((!log_flag) && (!input_image_flag) && (!params.mask_flag))
      {
        set_current_folder(params.edge_mask_file);
        inputImageFile.set_current_folder(current_folder);
        maskImageFile.set_current_folder(current_folder);
        stdDevInImageFile.set_current_folder(current_folder);
        stdDevMaskImageFile.set_current_folder(current_folder);
        edgeInImageFile.set_current_folder(current_folder);
        edgeMaskImageFile.set_current_folder(current_folder);
        regionMapInImageFile.set_current_folder(current_folder);
        logFile.set_current_folder(current_folder);
      }
#ifdef GDAL
    }
    else
    {
      strMessage += "\nInvalid input image mask data file, please reenter.\n";
      Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
      end_dialog.run();
      edgeMaskImageFile.set_filename("");
    }
#endif
    return;
  }

  void InitialGUI::on_regionMapInImageFile_selection_changed()
  {
#ifdef GDAL
    Glib::ustring strMessage = " ";
#endif
    params.region_map_in_file = regionMapInImageFile.get_filename();
#ifdef GDAL
    regionMapInImage.open(params.region_map_in_file);
    if (regionMapInImage.info_valid())
    {
#endif
      params.region_map_in_flag = true;
      if ((!log_flag) && (!input_image_flag) && (!params.mask_flag))
      {
        set_current_folder(params.region_map_in_file);
        inputImageFile.set_current_folder(current_folder);
        maskImageFile.set_current_folder(current_folder);
        stdDevInImageFile.set_current_folder(current_folder);
        stdDevMaskImageFile.set_current_folder(current_folder);
        edgeInImageFile.set_current_folder(current_folder);
        edgeMaskImageFile.set_current_folder(current_folder);
        regionMapInImageFile.set_current_folder(current_folder);
        logFile.set_current_folder(current_folder);
      }
#ifdef GDAL
    }
    else
    {
      strMessage += "\nInvalid input region map data file, please reenter.\n";
      Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
      end_dialog.run();
      regionMapInImageFile.set_filename("");
    }
#endif
    return;
  }

  void InitialGUI::on_logFile_selection_changed()
  {
    params.log_file = logFile.get_filename();
    log_flag = true;
    set_current_folder(params.log_file);
    inputImageFile.set_current_folder(current_folder);
    maskImageFile.set_current_folder(current_folder);
    stdDevInImageFile.set_current_folder(current_folder);
    stdDevMaskImageFile.set_current_folder(current_folder);
    edgeInImageFile.set_current_folder(current_folder);
    edgeMaskImageFile.set_current_folder(current_folder);
    regionMapInImageFile.set_current_folder(current_folder);
    logFile.set_current_folder(current_folder);

    return;
  }

 // Set current folder
  void InitialGUI::set_current_folder(const string& fileName)
  {
    if (fileName != "")
    {
#ifdef WINDOWS
      current_folder = fileName.substr(0,fileName.find_last_of("\\")+1);
#else
      current_folder = fileName.substr(0,fileName.find_last_of("/")+1);
#endif
    }
    else
      current_folder = "";
 
    return;
  }

  void InitialGUI::on_run_program()
  {
    if ((params.program_mode < 1) || (params.program_mode > 3))
    {
      params.status = 3;
      hide();
    }
   
    on_next_panel();

    params.set_region_sumsq_flag();

    params.status = params.set_maxnbdir();

    if (params.status)
      params.status = params.calc_defaults();

    if (params.status)
      params.status = 2;

    return;
  }

  void InitialGUI::on_next_panel()
  {
#ifdef GDAL
    bool   message_flag;
    int    band;
    double min_value, max_value;
#endif
    string tmp_string;
    Glib::ustring strMessage = " ";

    params.input_image_file = inputImageFile.get_filename();
#ifdef GDAL
    inputImage.open(params.input_image_file);
    input_image_flag = inputImage.info_valid();
    if (input_image_flag)
    {
      params.gdal_input_flag = true;
      message_flag = false;
      params.ncols = inputImage.get_ncols();
      params.nrows = inputImage.get_nrows();
      params.nbands = inputImage.get_nbands();
      params.dtype = inputImage.get_dtype();
      params.data_type = inputImage.get_data_type();
      switch (params.data_type)
      {
        case GDT_Byte:    break;
        case GDT_UInt16:  break;
        case GDT_Int16:   min_value = 0;
                          for (band = 0; band < params.nbands; band++)
                            if (min_value > inputImage.getMinimum(band))
                              min_value = inputImage.getMinimum(band);
                          if (min_value < 0.0)
                          {
                            strMessage += "\nWARNING: For the input image data file " + params.input_image_file + ",\n";
                            strMessage += "short integer data will be converted to 32-bit float data.\n";
                            message_flag = true;
                            params.dtype = Float32;
                          }
                          else
                          {
                            strMessage += "\nWARNING: For the input image data file " + params.input_image_file + ",\n";
                            strMessage += "short integer data will be converted to unsigned short integer data.\n";
                            message_flag = true;
                            params.dtype = UInt16;
                          }
                          break;
        case GDT_UInt32:  strMessage += "\nNOTE: For the input image data file " + params.input_image_file + ",\n";
                          strMessage += "32-bit unsigned integer data will be converted to 32-bit float data.";
                          message_flag = true;
                          params.dtype = Float32;
                          break;
        case GDT_Int32:   strMessage += "\nNOTE: For the input image data file " + params.input_image_file + ",\n";
                          strMessage += "32-bit integer data will be converted to 32-bit float data.";
                          message_flag = true;
                          params.dtype = Float32;
                          break;
        case GDT_Float32: break;
        case GDT_Float64: strMessage += "\nWARNING: For the input image data file " + params.input_image_file + ",\n";
                          strMessage += "64-bit doube data will be converted to 32-bit float data.\n";
                          strMessage += "Out of ranges value will not be read properly.";
                          message_flag = true;
                          params.dtype = Float32;
                          break;
        default:          strMessage += "\nUnknown or unsupported image data type for input image data file, please reenter.\n";
                          Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
                          end_dialog.run();
                          inputImageFile.set_filename("");
                          params.gdal_input_flag = false;
                          return;
      }
      if (message_flag)
      {
        Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
        end_dialog.run();
        strMessage = " ";
      }
    }
    else
#else
    input_image_flag = (params.input_image_file != "(Select File)");
    if (!input_image_flag)
#endif
    {
      strMessage += "\nInvalid input image data file, please reenter.\n";
      Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
      end_dialog.run();
      inputImageFile.set_filename("");
      return;
    }

#ifndef GDAL
    params.ncols = numberOfColumns.get_value();
    params.nrows = numberOfRows.get_value();
#ifdef THREEDIM
    params.nslices = numberOfSlices.get_value();
#endif
    params.nbands = numberOfBands.get_value();
    tmp_string = dtypeComboBox.get_active_text();
#ifdef GTKMM3
    dtypeComboBox.append("Unsigned Char (UInt8)");
    dtypeComboBox.append("Unsigned Short (UInt16)");
    dtypeComboBox.append("Float (Float32)");
#else
    dtypeComboBox.append_text("Unsigned Char (UInt8)");
    dtypeComboBox.append_text("Unsigned Short (UInt16)");
    dtypeComboBox.append_text("Float (Float32)");
#endif
    if (tmp_string == "Unsigned Char (UInt8)")
      params.dtype = UInt8;
    else if (tmp_string == "Unsigned Short (UInt16)")
      params.dtype = UInt16;
    else if (tmp_string == "Float (Float32)")
      params.dtype = Float32;
    else
      params.dtype = Unknown;
#endif // !GDAL

    if (maskImageFile.get_activeFlag())
    {
      params.mask_file = maskImageFile.get_filename();
#ifdef GDAL
      maskImage.open(params.mask_file);
      params.mask_flag = (maskImage.info_valid() && 
                          (params.ncols == maskImage.get_ncols()) && (params.nrows == maskImage.get_nrows()));
      if (params.mask_flag)
      {
        min_value = 0;
        max_value = 255;
        if (min_value > maskImage.getMinimum(0))
          min_value = maskImage.getMinimum(0);
        if (max_value < maskImage.getMaximum(0))
          max_value = maskImage.getMaximum(0);
        if ((min_value < 0.0) || (max_value > 255))
        {
          strMessage += "\nInvalid range for input mask image. Must be in range 0 to 255,\n please reenter.\n";
          Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
          end_dialog.run();
          maskImageFile.set_filename("");
          params.mask_flag = false;
          return;
        }
        if (maskImage.no_data_value_valid(0))
        {
          min_value = maskImage.get_no_data_value(0);
          if ((min_value < 256.0) && (min_value >= 0.0))
            params.mask_value = (unsigned char) min_value;
          else
          {
            strMessage += "\nInvalid input mask data file\n(mask value is out of 8-bit range),\n please reenter.\n";
            Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
            end_dialog.run();
            maskImageFile.set_filename("");
            params.mask_flag = false;
            return;
          }
        }
        else
          params.mask_value = MASK_VALUE;
      }
      else
#else
      params.mask_flag = (params.mask_file != "(Select File)");
      if (!params.mask_flag)
#endif
      {
        strMessage += "\nInvalid input mask data file, please reenter.\n";
        Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
        end_dialog.run();
        maskImageFile.set_filename("");
        return;
      }
    }
    else
      params.mask_value = MASK_VALUE;

#ifndef GDAL
    if (params.mask_flag)
    {
      params.mask_value = maskValue.get_value();
    }
#endif

    tmp_string = dissimCritComboBox.get_active_text();
    if (tmp_string == "       1-Norm")
      params.dissim_crit = 1;
    else if (tmp_string == "       2-Norm")
      params.dissim_crit = 2;
    else if (tmp_string == "Infinity Norm")
      params.dissim_crit = 3;
    else if (tmp_string == "Spectral Angle Mapper")
      params.dissim_crit = 4;
    else if (tmp_string == "Spectral Information Divergence")
      params.dissim_crit = 5;
#ifdef MSE_SQRT
    else if (tmp_string == "Square Root of Band Sum Mean Squared Error")
      params.dissim_crit = 6;
    else if (tmp_string == "Square Root of Band Maximum Mean Squared Error")
      params.dissim_crit = 7;
#else
    else if (tmp_string == "Band Sum Mean Squared Error")
      params.dissim_crit = 6;
    else if (tmp_string == "Band Maximum Mean Squared Error")
      params.dissim_crit = 7;
#endif
    else if (tmp_string == "Normalized Vector Distance")
      params.dissim_crit = 8;
    else if (tmp_string == "      Entropy")
      params.dissim_crit = 9;
    else if (tmp_string == "SAR Speckle Noise Criterion")
      params.dissim_crit = 10;
//    else if (tmp_string == "Feature Range")
//      params.dissim_crit = 11;

    bool std_dev_OK_flag = ((params.dissim_crit != 5) && (params.dissim_crit != 9));

    if ((std_dev_OK_flag) && (stdDevInImageFile.get_activeFlag()))
    {
      params.std_dev_image_file = stdDevInImageFile.get_filename();
#ifdef GDAL
      stdDevInImage.open(params.std_dev_image_file);
      params.std_dev_image_flag = (stdDevInImage.info_valid() && 
                                   (params.ncols == stdDevInImage.get_ncols()) && 
                                   (params.nrows == stdDevInImage.get_nrows()));
      if (params.std_dev_image_flag)
      {
        message_flag = false;
        switch (stdDevInImage.get_data_type())
        {
          case GDT_Byte:    break;
          case GDT_UInt16:  break;
          case GDT_Int16:   min_value = 0;
                            for (band = 0; band < params.nbands; band++)
                              if (min_value > stdDevInImage.getMinimum(band))
                                min_value = stdDevInImage.getMinimum(band);
                            if (min_value < 0.0)
                            {
                              strMessage += "\nWARNING: For the input standard deviation image file " + params.std_dev_image_file + ",\n";
                              strMessage += "short integer data will be converted to unsigned short integer data.\n";
                              strMessage += "Negative values will not be read properly.";
                              Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
                              end_dialog.run();
                              strMessage = " ";
                            }
                            break;
          case GDT_UInt32:  break;
          case GDT_Int32:   min_value = 0;
                            for (band = 0; band < params.nbands; band++)
                              if (min_value > stdDevInImage.getMinimum(band))
                                min_value = stdDevInImage.getMinimum(band);
                            if (min_value < 0.0)
                            {
                              strMessage += "\nWARNING: For the input standard deviation image file " + params.std_dev_image_file + ",\n";
                              strMessage += "integer data will be converted to unsigned integer data.\n";
                              strMessage += "Negative values will not be read properly.";
                              message_flag = true;
                            }
                            break;
          case GDT_Float32: strMessage += "\nWARNING: For the input standard deviation image file " + params.std_dev_image_file + ",\n";
                            strMessage += "32-bit float data will be converted to 32-bit unsigned integer data.\n";
                            strMessage += "Out of ranges value will not be read properly.";
                            message_flag = true;
                            break;
          case GDT_Float64: strMessage += "\nWARNING: For the input standard deviation image file " + params.std_dev_image_file + ",\n";
                            strMessage += "64-bit double data will be converted to 32-bit unsigned integer data.\n";
                            strMessage += "Out of ranges value will not be read properly.";
                            message_flag = true;
                            break;
          default:          strMessage += "\nUnknown or unsupported image data type for input standard deviation image file, please reenter.\n";
                            Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
                            end_dialog.run();
                            inputImageFile.set_filename("");
                            return;
        }
        if (message_flag)
        {
          Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
          end_dialog.run();
          strMessage = " ";
        }
      }
      else
#else
      params.std_dev_image_flag = (params.std_dev_image_file != "(Select File)");
      if (!params.std_dev_image_flag)
#endif
      {
        strMessage += "\nInvalid input standard deviation image file, please reenter.\n";
        Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
        end_dialog.run();
        stdDevInImageFile.set_filename("");
        return;
      }
    }

    if ((std_dev_OK_flag) && (stdDevMaskImageFile.get_activeFlag()))
    {
      params.std_dev_mask_file = stdDevMaskImageFile.get_filename();
#ifdef GDAL
      stdDevMaskImage.open(params.std_dev_mask_file);
      params.std_dev_mask_flag = (stdDevMaskImage.info_valid() && 
                          (params.ncols == stdDevMaskImage.get_ncols()) && (params.nrows == stdDevMaskImage.get_nrows()));
      if (params.std_dev_mask_flag)
      {
        min_value = 0;
        max_value = 255;
        if (min_value > stdDevMaskImage.getMinimum(0))
          min_value = stdDevMaskImage.getMinimum(0);
        if (max_value < stdDevMaskImage.getMaximum(0))
          max_value = stdDevMaskImage.getMaximum(0);
        if ((min_value < 0.0) || (max_value > 255))
        {
          strMessage += "\nInvalid range for input standard deviation mask image. Must be in range 0 to 255,\n please reenter.\n";
          Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
          end_dialog.run();
          stdDevMaskImageFile.set_filename("");
          params.std_dev_mask_flag = false;
          return;
        }
      }
      else
#else
      params.std_dev_mask_flag = (params.std_dev_mask_file != "(Select File)");
      if (!params.std_dev_mask_flag)
#endif
      {
        strMessage += "\nInvalid input standard deviation mask data file, please reenter.\n";
        Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
        end_dialog.run();
        stdDevMaskImageFile.set_filename("");
        return;
      }
    }

    if (edgeInImageFile.get_activeFlag())
    {
      params.edge_image_file = edgeInImageFile.get_filename();
#ifdef GDAL
      edgeInImage.open(params.edge_image_file);
      params.edge_image_flag = (edgeInImage.info_valid() && 
                                   (params.ncols == edgeInImage.get_ncols()) && 
                                   (params.nrows == edgeInImage.get_nrows()));
      if (params.edge_image_flag)
      {
        message_flag = false;
        switch (edgeInImage.get_data_type())
        {
          case GDT_Byte:    break;
          case GDT_UInt16:  break;
          case GDT_Int16:   min_value = 0;
                            for (band = 0; band < params.nbands; band++)
                              if (min_value > edgeInImage.getMinimum(band))
                                min_value = edgeInImage.getMinimum(band);
                            if (min_value < 0.0)
                            {
                              strMessage += "\nWARNING: For the input edge image file " + params.edge_image_file + ",\n";
                              strMessage += "short integer data will be converted to unsigned short integer data.\n";
                              strMessage += "Negative values will not be read properly.";
                              Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
                              end_dialog.run();
                              strMessage = " ";
                            }
                            break;
          case GDT_UInt32:  break;
          case GDT_Int32:   min_value = 0;
                            for (band = 0; band < params.nbands; band++)
                              if (min_value > edgeInImage.getMinimum(band))
                                min_value = edgeInImage.getMinimum(band);
                            if (min_value < 0.0)
                            {
                              strMessage += "\nWARNING: For the input edge image file " + params.edge_image_file + ",\n";
                              strMessage += "integer data will be converted to unsigned integer data.\n";
                              strMessage += "Negative values will not be read properly.";
                              message_flag = true;
                            }
                            break;
          case GDT_Float32: strMessage += "\nWARNING: For the input edge image file " + params.edge_image_file + ",\n";
                            strMessage += "32-bit float data will be converted to 32-bit unsigned integer data.\n";
                            strMessage += "Out of ranges value will not be read properly.";
                            message_flag = true;
                            break;
          case GDT_Float64: strMessage += "\nWARNING: For the input edge image file " + params.edge_image_file + ",\n";
                            strMessage += "64-bit double data will be converted to 32-bit unsigned integer data.\n";
                            strMessage += "Out of ranges value will not be read properly.";
                            message_flag = true;
                            break;
          default:          strMessage += "\nUnknown or unsupported image data type for input edge image file, please reenter.\n";
                            Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
                            end_dialog.run();
                            inputImageFile.set_filename("");
                            return;
        }
        if (message_flag)
        {
          Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
          end_dialog.run();
          strMessage = " ";
        }
      }
      else
#else
      params.edge_image_flag = (params.edge_image_file != "(Select File)");
      if (!params.edge_image_flag)
#endif
      {
        strMessage += "\nInvalid input edge image file, please reenter.\n";
        Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
        end_dialog.run();
        edgeInImageFile.set_filename("");
        return;
      }
    }

    if (edgeMaskImageFile.get_activeFlag())
    {
      params.edge_mask_file = edgeMaskImageFile.get_filename();
#ifdef GDAL
      edgeMaskImage.open(params.edge_mask_file);
      params.edge_mask_flag = (edgeMaskImage.info_valid() && 
                          (params.ncols == edgeMaskImage.get_ncols()) && (params.nrows == edgeMaskImage.get_nrows()));
      if (params.edge_mask_flag)
      {
        min_value = 0;
        max_value = 255;
        if (min_value > edgeMaskImage.getMinimum(0))
          min_value = edgeMaskImage.getMinimum(0);
        if (max_value < edgeMaskImage.getMaximum(0))
          max_value = edgeMaskImage.getMaximum(0);
        if ((min_value < 0.0) || (max_value > 255))
        {
          strMessage += "\nInvalid range for input edge mask image. Must be in range 0 to 255,\n please reenter.\n";
          Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
          end_dialog.run();
          edgeMaskImageFile.set_filename("");
          params.edge_mask_flag = false;
          return;
        }
      }
      else
#else
      params.edge_mask_flag = (params.edge_mask_file != "(Select File)");
      if (!params.edge_mask_flag)
#endif
      {
        strMessage += "\nInvalid input edge mask data file, please reenter.\n";
        Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
        end_dialog.run();
        edgeMaskImageFile.set_filename("");
        return;
      }
    }

    if (regionMapInImageFile.get_activeFlag())
    {
      params.region_map_in_file = regionMapInImageFile.get_filename();
#ifdef GDAL
      regionMapInImage.open(params.region_map_in_file);
      params.region_map_in_flag = (regionMapInImage.info_valid() && 
                                   (params.ncols == regionMapInImage.get_ncols()) && 
                                   (params.nrows == regionMapInImage.get_nrows()));
      if (params.region_map_in_flag)
      {
        message_flag = false;
        switch (regionMapInImage.get_data_type())
        {
          case GDT_Byte:    break;
          case GDT_UInt16:  break;
          case GDT_Int16:   min_value = 0;
                            for (band = 0; band < params.nbands; band++)
                              if (min_value > regionMapInImage.getMinimum(band))
                                min_value = regionMapInImage.getMinimum(band);
                            if (min_value < 0.0)
                            {
                              strMessage += "\nWARNING: For the input region map data file " + params.region_map_in_file + ",\n";
                              strMessage += "short integer data will be converted to unsigned short integer data.\n";
                              strMessage += "Negative values will not be read properly.";
                              Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
                              end_dialog.run();
                              strMessage = " ";
                            }
                            break;
          case GDT_UInt32:  break;
          case GDT_Int32:   min_value = 0;
                            for (band = 0; band < params.nbands; band++)
                              if (min_value > regionMapInImage.getMinimum(band))
                                min_value = regionMapInImage.getMinimum(band);
                            if (min_value < 0.0)
                            {
                              strMessage += "\nWARNING: For the input region map data file " + params.region_map_in_file + ",\n";
                              strMessage += "integer data will be converted to unsigned integer data.\n";
                              strMessage += "Negative values will not be read properly.";
                              message_flag = true;
                            }
                            break;
          case GDT_Float32: strMessage += "\nWARNING: For the input region map data file " + params.region_map_in_file + ",\n";
                            strMessage += "32-bit float data will be converted to 32-bit unsigned integer data.\n";
                            strMessage += "Out of ranges value will not be read properly.";
                            message_flag = true;
                            break;
          case GDT_Float64: strMessage += "\nWARNING: For the input region map data file " + params.region_map_in_file + ",\n";
                            strMessage += "64-bit double data will be converted to 32-bit unsigned integer data.\n";
                            strMessage += "Out of ranges value will not be read properly.";
                            message_flag = true;
                            break;
          default:          strMessage += "\nUnknown or unsupported image data type for input region map data file, please reenter.\n";
                            Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
                            end_dialog.run();
                            inputImageFile.set_filename("");
                            return;
        }
        if (message_flag)
        {
          Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
          end_dialog.run();
          strMessage = " ";
        }
      }
      else
#else
      params.region_map_in_flag = (params.region_map_in_file != "(Select File)");
      if (!params.region_map_in_flag)
#endif
      {
        strMessage += "\nInvalid input region map data file, please reenter.\n";
        Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
        end_dialog.run();
        regionMapInImageFile.set_filename("");
        return;
      }
    }

    if (params.program_mode == 1)
    {
      params.spclust_wght = 0.0;
    }
    else
    {
      params.spclust_wght = spClustWght.get_fvalue();
      params.spclust_wght_flag = (spClustWght.value_valid() && (params.spclust_wght >= 0.0) && (params.spclust_wght <= 1.0));
      if (!params.spclust_wght_flag)
      {
        strMessage += "\nInvalid spectral clustering weight value, please reenter.\n";
        Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
        end_dialog.run();
        spClustWght.clear();
        return;
      }
    }
    params.spclust_wght_flag = (params.spclust_wght != 0.0);

    if (logFile.get_validFlag())
      params.log_file = logFile.get_filename();
    else
    {
      strMessage += "\nAn output log file must be provided.\n";
      Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
      end_dialog.run();
      return;
    }

    if (params.program_mode != 3)
    {
      params.rnb_levels = 1;
      params.rnb_levels_flag = true;
    }

#ifdef GDAL
    if (params.gdal_input_flag)
    {
      params.output_driver_description = inputImage.get_driver_description();
#ifdef RHSEG_RUN
      if ((params.output_driver_description == "BMP") || (params.output_driver_description == "PNG") || (params.output_driver_description == "PNG") ||
          (params.output_driver_description == "JPEG") || (params.output_driver_description == "XPM") || (params.output_driver_description == "GIF"))
      {
        cout << "Since the input image format, " << params.output_driver_description; 
        cout << ", does not support 16 and/or 32 bit data," << endl;
        params.output_driver_description = OUTPUT_DRIVER_DESCRIPTION;
        cout << "output image files will be created in " << params.output_driver_description << " format." << endl;
      }
#endif
    }
#endif
 
    int sub_pos, string_first_pos, string_last_pos;
#ifdef WINDOWS
    params.current_folder = params.log_file.substr(0,params.log_file.find_last_of("\\")+1);
    string_first_pos = params.input_image_file.find_last_of("\\");
#else
    params.current_folder = params.log_file.substr(0,params.log_file.find_last_of("/")+1);
    string_first_pos = params.input_image_file.find_last_of("/");
#endif
    if (string_first_pos == (int) string::npos)
      string_first_pos = 0;
    else
      string_first_pos++;
    string_last_pos = params.input_image_file.find_last_of(".");
#ifdef GDAL
    if (params.output_driver_description == "ENVI")
    {
      params.suffix = "";
    }
    else if (string_last_pos == (int) string::npos)
#else
    if (string_last_pos == (int) string::npos)
#endif
    {
      params.suffix = "";
    }
    else
    {
      params.suffix = params.input_image_file.substr(string_last_pos,string::npos);
    }
    params.prefix = params.input_image_file.substr(string_first_pos,(string_last_pos-string_first_pos));

    sub_pos = params.prefix.find(".");
    while (sub_pos != (int) string::npos)
    {
      params.prefix[sub_pos] = '_';
      sub_pos = params.prefix.find(".");
    }

  // Determine number of dimensions
    params.nb_dimensions = 0;
#ifdef THREEDIM
    if ((params.ncols > 1) && (params.nrows == 1) && (params.nslices == 1))
      params.nb_dimensions = 1;
    if ((params.ncols > 1) && (params.nrows > 1) && (params.nslices == 1))
      params.nb_dimensions = 2;
    if ((params.ncols > 1) && (params.nrows > 1) && (params.nslices > 1))
      params.nb_dimensions = 3;
    if (params.nb_dimensions == 0)
    {
      strMessage += "\nUnexpected dimensions: For 1-D, must have nrows = nslices = 1 and ncols > 1;\n";
      strMessage += "for 2-D must have nslices = 1, ncols > 1 and nrows > 1";
      Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
      end_dialog.run();
      return;
    }
#else
    if ((params.ncols > 1) && (params.nrows == 1))
      params.nb_dimensions = 1;
    if ((params.ncols > 1) && (params.nrows > 1))
      params.nb_dimensions = 2;
    if (params.nb_dimensions == 0)
    {
      strMessage += "\nUnexpected dimensions: For 1-D, must have nrows = 1 and ncols > 1\n";
      Gtk::MessageDialog end_dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
      end_dialog.run();
      return;
    }
#endif

 // Set default parameter values, based on initial parameter values
    params.status = params.set_defaults();

    hide();

    return;
  }

} // namespace HSEGTilton
