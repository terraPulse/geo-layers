// paramsGUI.cc
#include "paramsGUI.h"
#include "initialParams.h"
#include <params/params.h>
#include <iostream>

extern HSEGTilton::InitialParams initialParams;
extern HSEGTilton::Params params;
#ifdef GLSIMP
extern CommonTilton::Image inputImage;
#endif

namespace HSEGTilton
{
 // Constructor
  ParamsGUI::ParamsGUI():
             vBox(false,10),
#ifdef GLSIMP
             input_imageLabel(" You MUST provide the Input Image File here if it is not provided in the HSeg/RHSeg\n Output Parameter File (below) BEFORE you provide the HSEG/RHSeg Output Parameter File"),
             input_image(" Input Image File (processed by RHSeg/HSeg):",
                        "Enter or Select the Input Image File",
                        Gtk::FILE_CHOOSER_ACTION_OPEN,false),
#endif
             requiredLabel(" Required Files and Parameters (defaults given, if any):",0.0,0.0),
             HSEGOparam(" HSeg/RHSeg Output Parameter File (oparam) for Input to HSegLearn:",
                        "Enter or Select the HSeg/RHSeg Output Paramter File (oparam)",
                        Gtk::FILE_CHOOSER_ACTION_OPEN),
             examplesOut(" Output ASCII Examples List File:",
                         "Enter or Select the Output ASCII Examples List File",
                         Gtk::FILE_CHOOSER_ACTION_SAVE,true),
             labelOut(" Output Class Label Map File:",
                      "Enter or Select the Output Class Label Map File",
                      Gtk::FILE_CHOOSER_ACTION_SAVE,true),
             asciiOut(" Output ASCII Class Label Names List File:",
                      "Enter or Select the Output ASCII Class Label Names List File",
                      Gtk::FILE_CHOOSER_ACTION_SAVE,true),
             redDisplayBand("Red Display Band:"),
             greenDisplayBand("Green Display Band:"),
             blueDisplayBand("Blue Display Band:"),
             rgbImageStretchLabel("RGB Image Stretch Option:"),
             rangeFromObject("Range From: "),
             rangeToObject("To: "),
             viewLabel1("For 3-D data, view the 2-D Representation of "),
             viewElement(""),
             viewLabel2("(give index of specified dimension)"),
             optionalLabel(" Optional Files and Parameters (select the Check Box to enable):",0.0,0.0),
             examplesIn(" Input ASCII Examples List File:",
                     "Enter or Select the Input ASCII Examples List File",
                     Gtk::FILE_CHOOSER_ACTION_OPEN,false),
             panchromatic_image(" Input Panchromatic Image File:",
                        "Enter or Select the Input Panchromatic Image File",
                        Gtk::FILE_CHOOSER_ACTION_OPEN,false),
             reference_image(" Input Reference Image File:",
                        "Enter or Select the Input Reference Image File",
                        Gtk::FILE_CHOOSER_ACTION_OPEN,false)
  {
    set_title("Hierarchical Segmentation Learn by Example program Parameter Input");
    set_default_size(512,64);

    char *cwd;
    cwd = g_get_current_dir();
    params.current_folder = cwd;
#ifdef GLSIMP
    input_image.add_shortcut_folder(params.current_folder);
    input_image.set_current_folder(params.current_folder);
#endif
    HSEGOparam.add_shortcut_folder(params.current_folder);
    HSEGOparam.set_current_folder(params.current_folder);
    g_free(cwd);

#ifdef GTKMM3
    rgbImageStretchComboBox.append("Linear Stretch with Percent Clipping");
    rgbImageStretchComboBox.append("      Histogram Equalization        ");
    rgbImageStretchComboBox.append(" Linear Stretch to Percentile Range ");
    viewComboBox.append("column");
    viewComboBox.append("row");
    viewComboBox.append("slice");
#else
    rgbImageStretchComboBox.append_text("Linear Stretch with Percent Clipping");
    rgbImageStretchComboBox.append_text("      Histogram Equalization        ");
    rgbImageStretchComboBox.append_text(" Linear Stretch to Percentile Range ");
    viewComboBox.append_text("column");
    viewComboBox.append_text("row");
    viewComboBox.append_text("slice");
#endif
    switch (initialParams.rgb_image_stretch)
    {
      case 1:  
#ifdef ARTEMIS
               rgbImageStretchComboBox.set_active(0);
#else
               rgbImageStretchComboBox.set_active_text("Linear Stretch with Percent Clipping");
#endif
               break;
      case 2:  
#ifdef ARTEMIS
               rgbImageStretchComboBox.set_active(1);
#else
               rgbImageStretchComboBox.set_active_text("      Histogram Equalization        ");
#endif
               break;
      case 3:  
#ifdef ARTEMIS
               rgbImageStretchComboBox.set_active(2);
#else
               rgbImageStretchComboBox.set_active_text(" Linear Stretch to Percentile Range ");
#endif
               break;
      default: cout << "Invalid value for rgb_image_stretch" << endl;
               break;
    }
#ifdef ARTEMIS
    viewComboBox.set_active(2);
#else
    viewComboBox.set_active_text("slice");
#endif
    rangeFromObject.set_fvalue(initialParams.range[0]);
    rangeToObject.set_fvalue(initialParams.range[1]);
#ifdef GLSIMP
    input_image.signal_selection_changed().connect(sigc::mem_fun(*this,
                         &ParamsGUI::on_input_image_selection_changed) );
#endif
    HSEGOparam.signal_selection_changed().connect(sigc::mem_fun(*this,
                         &ParamsGUI::on_HSEGOparam_selection_changed) );
    rgbImageStretchComboBox.signal_changed().connect(sigc::mem_fun(*this,
                         &ParamsGUI::on_rgbImageStretchComboBox_changed) );
    viewElement.signal_activated().connect(sigc::mem_fun(*this,
                         &ParamsGUI::on_viewElement_activated) );
    viewComboBox.signal_changed().connect(sigc::mem_fun(*this,
                         &ParamsGUI::on_viewComboBox_changed) );

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
#endif //GLIBMM_EXCEPTIONS_ENABLED

    //Get the menubar widget, and add it to a container widget:
    Gtk::Widget* pMenubar = m_refUIManager->get_widget("/MenuBar");
    if(pMenubar)
      vBox.pack_start(*pMenubar, Gtk::PACK_SHRINK);

#ifdef GLSIMP
    vBox.pack_start(input_imageLabel);
    vBox.pack_start(input_image);
#endif
    vBox.pack_start(requiredLabel);
    vBox.pack_start(HSEGOparam);
    displayBox.pack_start(redDisplayBand);
    displayBox.pack_start(greenDisplayBand);
    displayBox.pack_start(blueDisplayBand);   
    vBox.pack_start(displayBox);
    rgbImageStretchBox.pack_start(rgbImageStretchLabel);
    rgbImageStretchBox.pack_start(rgbImageStretchComboBox);
    vBox.pack_start(rgbImageStretchBox);
    rangeBox.pack_start(rangeFromObject);
    rangeBox.pack_start(rangeToObject);
    vBox.pack_start(rangeBox);
    vBox.pack_start(examplesOut);
    vBox.pack_start(labelOut);
    vBox.pack_start(asciiOut);
    viewBox.pack_start(viewLabel1);
    viewBox.pack_start(viewComboBox);
    viewBox.pack_start(viewElement);
    viewBox.pack_start(viewLabel2);
    vBox.pack_start(viewBox);
    vBox.pack_start(optionalLabel);
    vBox.pack_start(examplesIn);
    vBox.pack_start(panchromatic_image);
    vBox.pack_start(reference_image);

    show_all_children();
    displayBox.hide();
    rgbImageStretchBox.hide();
    rangeBox.hide();
    examplesOut.hide();
    labelOut.hide();
    asciiOut.hide();
    viewBox.hide();
    optionalLabel.hide();
    examplesIn.hide();
    panchromatic_image.hide();
    reference_image.hide();
// Are the next few lines necessary???
#ifdef GLSIMP
    input_image.set_activeFlag(false);
#endif
    examplesIn.set_activeFlag(false);
    panchromatic_image.set_activeFlag(false);
    reference_image.set_activeFlag(false);

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
#ifdef GLSIMP
  void ParamsGUI::on_input_image_selection_changed()
  {
    int    band;
    double min_value;

    params.input_image_file = input_image.get_filename();
    params.input_image_flag = inputImage.open(params.input_image_file);
    if (params.input_image_flag)
    {
      params.ncols = inputImage.get_ncols();
      params.ncols_flag = true;
      params.nrows = inputImage.get_nrows();
      params.nrows_flag = true;
      params.nbands = inputImage.get_nbands();
      params.nbands_flag = true;
      params.dtype = inputImage.get_dtype();
      params.dtype_flag = true;
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
                            cout << "WARNING: For the input image data file " << params.input_image_file << "," << endl;
                            cout << "short integer data will be converted to 32-bit float data." << endl;
                            params.dtype = Float32;
                          }
                          else
                          {
                            cout << "WARNING: For the input image data file " << params.input_image_file << "," << endl;
                            cout << "short integer data will be converted to unsigned short integer data." << endl;
                            params.dtype = UInt16;
                          }
                          break;
        case GDT_UInt32:  cout << "NOTE: For the input image data file " << params.input_image_file << "," << endl;
                          cout << "32-bit unsigned integer data will be converted to 32-bit float data." << endl;
                          params.dtype = Float32;
                          break;
        case GDT_Int32:   cout << "NOTE: For the input image data file " << params.input_image_file << "," << endl;
                          cout << "32-bit integer data will be converted to 32-bit float data." << endl;
                          params.dtype = Float32;
                          break;
        case GDT_Float32: break;
        case GDT_Float64: cout << "WARNING: For the input image data file " << params.input_image_file << "," << endl;
                          cout << "64-bit double data will be converted to 32-bit float data." << endl;
                          cout << "Out of ranges value will not be read properly." << endl;
                          params.dtype = Float32;
                          break;
        default:          cout << "Unknown or unsupported image data type for input image data file ";
                          cout << params.input_image_file << endl;
                          return;
      } 
    }
    else
    {
      cout << "WARNING:  Input image file " << params.input_image_file << " is of unknown format." << endl;
    }

    return;
  }
#endif
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
      if (params.current_folder != "")
      {
#ifdef WINDOWS
        initialParams.examples_out_file = params.current_folder + "\\" + initialParams.examples_out_file;
#else
        initialParams.examples_out_file = params.current_folder + "/" + initialParams.examples_out_file;
#endif
        examplesOut.set_current_folder(params.current_folder);
      }
      examplesOut.set_filename(initialParams.examples_out_file);
      if (params.current_folder != "")
      {
#ifdef WINDOWS
        initialParams.label_out_file = params.current_folder + "\\" + initialParams.label_out_file;
#else
        initialParams.label_out_file = params.current_folder + "/" + initialParams.label_out_file;
#endif
        labelOut.set_current_folder(params.current_folder);
      }
      if (params.suffix.length() > 0)
        initialParams.label_out_file += params.suffix;
      labelOut.set_filename(initialParams.label_out_file);
      if (params.current_folder != "")
      {
#ifdef WINDOWS
        initialParams.ascii_out_file = params.current_folder + "\\" + initialParams.ascii_out_file;
#else
        initialParams.ascii_out_file = params.current_folder + "/" + initialParams.ascii_out_file;
#endif
        asciiOut.set_current_folder(params.current_folder);
      }
      asciiOut.set_filename(initialParams.ascii_out_file);

#ifdef THREEDIM
      viewElement.set_value((params.nslices-1));
#else
      viewElement.set_value(0);
#endif
      
      if (params.current_folder != "")
      {
        examplesIn.set_current_folder(params.current_folder);
        panchromatic_image.set_current_folder(params.current_folder);
        reference_image.set_current_folder(params.current_folder);
      }

      displayBox.show();
      rgbImageStretchBox.show();
      if (initialParams.rgb_image_stretch != 2)
        rangeBox.show();
      examplesOut.show();
      labelOut.show();
      asciiOut.show();
      if (params.nb_dimensions == 3)
        viewBox.show();
      optionalLabel.show();
      examplesIn.show();
      panchromatic_image.show();
      reference_image.show();
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

  void ParamsGUI::on_rgbImageStretchComboBox_changed()
  {
    string tmp_string;

    tmp_string = rgbImageStretchComboBox.get_active_text();
    if (tmp_string == "Linear Stretch with Percent Clipping")
      initialParams.rgb_image_stretch = 1;
    else if (tmp_string == "      Histogram Equalization        ")
      initialParams.rgb_image_stretch = 2;
    else if (tmp_string == " Linear Stretch to Percentile Range ")
      initialParams.rgb_image_stretch = 3;
/*
    switch (initialParams.rgb_image_stretch)
    {
      case 1:  cout << "rgb_image_stretch changed to \"Linear Stretch with Percent Clipping\"" << endl;
               break;
      case 2:  cout << "rgb_image_stretch changed to \"Histogram Equalization\"" << endl;
               break;
      case 3:  cout << "rgb_image_stretch changed to \"Linear Stretch to Percentile Range\"" << endl;
               break;
      default: cout << "Invalid value for rgb_image_stretch" << endl;
               break;
    }
*/
    if (initialParams.rgb_image_stretch == 2)
      rangeBox.hide();
    else
      rangeBox.show();
    return;
  }

  void ParamsGUI::on_viewElement_activated()
  {
    string tmp_string;

    initialParams.view_dimension_flag = true;
    tmp_string = viewComboBox.get_active_text();
    if (tmp_string == "column")
      initialParams.view_dimension = COLUMN;
    else if (tmp_string == "row")
      initialParams.view_dimension = ROW;
    else if (tmp_string == "slice")
      initialParams.view_dimension = SLICE;
    else
      initialParams.view_dimension_flag = false;

    if (initialParams.view_dimension_flag)
    {
      initialParams.view_element = viewElement.get_value();
      initialParams.view_element_flag = true;
    }
    else
    {
      cout << "ERROR (on_viewElement_activated): Invalid view dimension" << endl; // Should never happen!
//      return false;
    }
  
  }
  
  void ParamsGUI::on_viewComboBox_changed()
  {
    string tmp_string;

    initialParams.view_dimension_flag = true;
    tmp_string = viewComboBox.get_active_text();
    if (tmp_string == "column")
      initialParams.view_dimension = COLUMN;
    else if (tmp_string == "row")
      initialParams.view_dimension = ROW;
    else if (tmp_string == "slice")
      initialParams.view_dimension = SLICE;
    else
      initialParams.view_dimension_flag = false;

    if (initialParams.view_dimension_flag)
    {
      initialParams.view_element = viewElement.get_value();
      initialParams.view_element_flag = true;
    }
    else
    {
      cout << "ERROR (on_viewElement_activated): Invalid view dimension" << endl; // Should never happen!
//      return false;
    }
  
  }
  
  void ParamsGUI::on_help_requested()
  {
    Glib::ustring strMessage = "Enter in program parameters and run the program by\n"
                               "selecting the \"Run Program\" menu item under the\n"
                               "\"Program Actions\" menu.\n\nFor more help type "
                               "\"hseglearn -help\" on the command line.\n\n";
    Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
    
    dialog.run();
  }

  void ParamsGUI::on_run_program()
  {
    string tmp_string;

//    cout << "\"Run Program\" menu item selected" << endl;
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

    initialParams.red_display_flag = redDisplayBand.value_valid();
    if (initialParams.red_display_flag)
    {
      initialParams.red_display_band = redDisplayBand.get_value();
      if ((initialParams.red_display_band < 0) || (initialParams.red_display_band >= params.nbands))
        initialParams.red_display_flag = false;
    }
    initialParams.green_display_flag = greenDisplayBand.value_valid();
    if (initialParams.green_display_flag)
    {
      initialParams.green_display_band = greenDisplayBand.get_value();
      if ((initialParams.green_display_band < 0) || (initialParams.green_display_band >= params.nbands))
        initialParams.green_display_flag = false;
    }
    initialParams.blue_display_flag = blueDisplayBand.value_valid();
    if (initialParams.blue_display_flag)
    {
      initialParams.blue_display_band = blueDisplayBand.get_value();
      if ((initialParams.blue_display_band < 0) || (initialParams.blue_display_band >= params.nbands))
        initialParams.blue_display_flag = false;
    }
    params.status = initialParams.red_display_flag &&
                    initialParams.green_display_flag && initialParams.blue_display_flag;
    if (params.status)
    {
      tmp_string = rgbImageStretchComboBox.get_active_text();
      if (tmp_string == "Linear Stretch with Percent Clipping")
        initialParams.rgb_image_stretch = 1;
      else if (tmp_string == "      Histogram Equalization        ")
        initialParams.rgb_image_stretch = 2;
      else if (tmp_string == " Linear Stretch to Percentile Range ")
        initialParams.rgb_image_stretch = 3;
      if (initialParams.rgb_image_stretch != 2)
      {
        initialParams.range[0] = rangeFromObject.get_fvalue();
        initialParams.range[1] = rangeToObject.get_fvalue();
      }
      initialParams.examples_out_file = examplesOut.get_filename();
      initialParams.label_out_file = labelOut.get_filename();
      initialParams.ascii_out_file = asciiOut.get_filename();
      initialParams.view_dimension_flag = true;
      tmp_string = viewComboBox.get_active_text();
      if (tmp_string == "column")
        initialParams.view_dimension = COLUMN;
      else if (tmp_string == "row")
        initialParams.view_dimension = ROW;
      else if (tmp_string == "slice")
        initialParams.view_dimension = SLICE;
      else
        initialParams.view_dimension_flag = false;
      initialParams.view_element = viewElement.get_value();
      initialParams.view_element_flag = true;
      initialParams.examples_in_flag = examplesIn.get_activeFlag();
      if (initialParams.examples_in_flag)
        initialParams.examples_in_file = examplesIn.get_filename();
      initialParams.panchromatic_image_flag = panchromatic_image.get_activeFlag();
      if (initialParams.panchromatic_image_flag)
        initialParams.panchromatic_image_file = panchromatic_image.get_filename();
      initialParams.reference_image_flag = reference_image.get_activeFlag();
      if (initialParams.reference_image_flag)
        initialParams.reference_image_file = reference_image.get_filename();

      initialParams.set_internal_parameters();

      hide();
    }
    else
    {
      Glib::ustring strMessage = "Invalid values entered for red, green and/or blue display band.\n"
                                 "Please reenter valid values.";
      Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
     
      dialog.run();
    }

    return;
  }

  void ParamsGUI::on_exit_requested()
  {
    cout << "\"Exit\" menu item selected" << endl;
    params.status = false;
    hide();
    return;
  }

} // namespace HSEGTilton

