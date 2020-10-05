// outputFileGUI.cc

#include "outputFileGUI.h"
#include "params.h"
#include <iostream>
#include <fstream>

extern HSEGTilton::Params params;

namespace HSEGTilton
{
 // Constructor
  OutputFileGUI::OutputFileGUI():
               vBox(false,10), 
               outputFilesFrame("Specify Output Files (default names provided):"), 
               outputFilesBox(false,10),
               optionalLabel("(Click on check box to specify optional files)",0.0),
               classLabelsMapFile(" Output class labels map file (class_labels_map):",
                          "Enter or Select the output class labels map file (class_labels_map)",
                          Gtk::FILE_CHOOSER_ACTION_SAVE),
               boundaryMapFile(" Output hierarchical boundary map file (boundary_map):",
                          "Enter or Select the output hierarchical boundary map file (boundary_map)",
                          Gtk::FILE_CHOOSER_ACTION_SAVE,false),
               regionClassesFile(" Output region classes file (region_classes):",
                          "Enter or Select the output parameter file (region_classes)",
                          Gtk::FILE_CHOOSER_ACTION_SAVE),
               objectConnType1Button("Check to use lowest connectivity for object labels map"),
               regionObjectsButton("Check to output region object information"),
               objectLabelsMapFile(" Output object labels map file (object_labels_map):",
                          "Enter or Select the output object labels map file (object_labels_map)",
                          Gtk::FILE_CHOOSER_ACTION_SAVE),
               regionObjectsFile(" Output region objects file (region_objects):",
                          "Enter or Select the output region objects file (region_objects)",
                          Gtk::FILE_CHOOSER_ACTION_SAVE),
               oparamFile(" Output rhseg output parameter file (oparam):",
                          "Enter or Select the rhseg output oparam file (oparam)",
                          Gtk::FILE_CHOOSER_ACTION_SAVE)
  {
    if (params.program_mode == 1)
      set_title("HSWO Output File Specification");
    else if (params.program_mode == 2)
      set_title("HSeg Output File Specification");
    else
      set_title("RHSeg Output File Specification");
    set_default_size(512,64);

  // Set up signal handlers
    objectConnType1Button.signal_clicked().connect(sigc::mem_fun(*this,
                   &OutputFileGUI::on_objectConnType1Button_checked) );
    regionObjectsButton.signal_clicked().connect(sigc::mem_fun(*this,
                   &OutputFileGUI::on_regionObjectsButton_checked) );

  // Initialize labels
    inputImageFile.set_label("Input image data file:  " + params.input_image_file);
    inputImageFile.set_alignment(0.5,0.5);
    string tmp_string = "Number of columns: " + stringify_int(params.ncols);
    tmp_string += ", number of rows: " + stringify_int(params.nrows);
#ifdef THREEDIM
    tmp_string += ", number of slices: " + stringify_int(params.nslices);
#endif
    tmp_string += ", number of bands: " + stringify_int(params.nbands);
    switch (params.dtype)
    {
      case UInt8:   tmp_string += ", data type: UNSIGNED CHAR (8-bit)";
                    break;
      case UInt16:  tmp_string += ", data type: UNSIGNED SHORT (16-bit)";
                    break;
      case Float32: tmp_string += ", data type: FLOAT (32-bit)";
                    break;
      default:      tmp_string += ", data type is invalid (Unknown)";
                    break;
    }
    inputDescription.set_label(tmp_string);
    inputDescription.set_alignment(0.5,0.5);
    if (params.mask_flag)
    {
      maskImageFile.set_label("Input mask data file:  " + params.mask_file);
      maskImageFile.set_alignment(0.5,0.5);
    }
    if (params.std_dev_image_flag)
    {
      stdDevInImageFile.set_label("Input standard deviation image file:  " + params.std_dev_image_file);
      stdDevInImageFile.set_alignment(0.5,0.5);
    }
    if (params.std_dev_mask_flag)
    {
      stdDevMaskImageFile.set_label("Input standard deviation mask file:  " + params.std_dev_mask_file);
      stdDevMaskImageFile.set_alignment(0.5,0.5);
    }
    if (params.edge_image_flag)
    {
      edgeInImageFile.set_label("Input standard deviation image file:  " + params.edge_image_file);
      edgeInImageFile.set_alignment(0.5,0.5);
    }
    if (params.edge_mask_flag)
    {
      edgeMaskImageFile.set_label("Input standard deviation mask file:  " + params.edge_mask_file);
      edgeMaskImageFile.set_alignment(0.5,0.5);
    }
    if (params.region_map_in_flag)
    {
      regionMapInImageFile.set_label("Input region map data file:  " + params.region_map_in_file);
      regionMapInImageFile.set_alignment(0.5,0.5);
    }
    tmp_string = "Relative importance of spectral clustering vs. region growing: ";
    tmp_string += stringify_float(params.spclust_wght);
    spClustWght.set_label(tmp_string);
    tmp_string = "Dissimilarity Criterion:  ";
    switch (params.dissim_crit)
    {
      case 1:  tmp_string += "1-Norm";
               break;
      case 2:  tmp_string += "2-Norm";
               break;
      case 3:  tmp_string += "Infinity Norm";
               break;
      case 4:  tmp_string += "Spectral Angle Mapper";
               break;
      case 5:  tmp_string += "Spectral Information Divergence";
               break;
#ifdef MSE_SQRT
      case 6:  tmp_string += "Square Root of Band Sum Mean Squared Error";
               break;
      case 7:  tmp_string += "Square Root of Band Maximum Mean Squared Error";
               break;
#else
      case 6:  tmp_string += "Band Sum Mean Squared Error";
               break;
      case 7:  tmp_string += "Band Maximum Mean Squared Error";
               break;
#endif
      case 8:  tmp_string += "Normalized Vector Distance";
               break;
      case 9:  tmp_string += "Entropy";
               break;
      case 10: tmp_string += "SAR Speckle Noise Criterion";
               break;
//      case 11: tmp_string += "Feature Range";
//               break;
#ifdef MSE_SQRT
      default: tmp_string += "Square Root of Band Sum Mean Squared Error";
#else
      default: tmp_string += "Band Sum Mean Squared Error";
#endif
               break;
    }
    dissimCrit.set_label(tmp_string);
    logFile.set_label("Output log file:  " + params.log_file);
    logFile.set_alignment(0.5,0.5);

  // Complete initialization of file Objects
    classLabelsMapFile.set_filename(params.class_labels_map_file);
    boundaryMapFile.set_filename(params.boundary_map_file);
    regionClassesFile.set_filename(params.region_classes_file);
    objectConnType1Flag = false;
    objectConnType1Button.set_active(objectConnType1Flag);
    objectConnType1Button.set_alignment(0.0,0.5);
    if (params.spclust_wght == 0.0)
      regionObjectsFlag = false;
    else
      regionObjectsFlag = params.region_objects_flag;
    regionObjectsButton.set_active(regionObjectsFlag);
    regionObjectsButton.set_alignment(0.0,0.5);
    objectLabelsMapFile.set_filename(params.object_labels_map_file);
    regionObjectsFile.set_filename(params.region_objects_file);
    oparamFile.set_filename(params.oparam_file);

    add(vBox); // put a MenuBar at the top of the box and other stuff below it.

  //Create actions for menus:
    m_refActionGroup = Gtk::ActionGroup::create();

    //Add normal Actions:
    m_refActionGroup->add( Gtk::Action::create("ActionMenu", "Program _Actions") );
    m_refActionGroup->add( Gtk::Action::create("Help", "_Help"),
                      sigc::mem_fun(*this, &OutputFileGUI::on_help_requested) );
    m_refActionGroup->add( Gtk::Action::create("Go", "_Go To Next Panel"),
                      sigc::mem_fun(*this, &OutputFileGUI::on_next_panel) );
#ifdef RHSEG_SETUP
    m_refActionGroup->add( Gtk::Action::create("Run", "_Run RHSeg Setup"),
                           sigc::mem_fun(*this, &OutputFileGUI::on_run_program) );
#else
    if (params.program_mode == 1)
      m_refActionGroup->add( Gtk::Action::create("Run", "_Run HSWO"),
                        sigc::mem_fun(*this, &OutputFileGUI::on_run_program) );
    else if (params.program_mode == 2)
      m_refActionGroup->add( Gtk::Action::create("Run", "_Run HSeg"),
                        sigc::mem_fun(*this, &OutputFileGUI::on_run_program) );
    else
      m_refActionGroup->add( Gtk::Action::create("Run", "_Run RHSeg"),
                        sigc::mem_fun(*this, &OutputFileGUI::on_run_program) );
#endif
    m_refActionGroup->add( Gtk::Action::create("Exit", "_Exit"),
                      sigc::mem_fun(*this, &OutputFileGUI::on_exit_requested) );

    m_refUIManager = Gtk::UIManager::create();
    m_refUIManager->insert_action_group(m_refActionGroup);
    add_accel_group(m_refUIManager->get_accel_group());

    //Layout the actions in the menubar:
    Glib::ustring ui_info = 
          "<ui>"
          "  <menubar name='MenuBar'>"
          "    <menu action='ActionMenu'>"
          "      <menuitem action='Help'/>"
          "      <separator/>"
          "      <menuitem action='Go'/>"
          "      <menuitem action='Run'/>"
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

    initialBox.pack_start(inputImageFile);
    initialBox.pack_start(inputDescription);
    if (params.mask_flag)
      initialBox.pack_start(maskImageFile);
    if (params.std_dev_image_flag)
      initialBox.pack_start(stdDevInImageFile);
    if (params.std_dev_mask_flag)
      initialBox.pack_start(stdDevMaskImageFile);
    if (params.edge_image_flag)
      initialBox.pack_start(edgeInImageFile);
    if (params.edge_mask_flag)
      initialBox.pack_start(edgeMaskImageFile);
    if (params.region_map_in_flag)
      initialBox.pack_start(regionMapInImageFile);
    initialBox.pack_start(spClustWght);
    initialBox.pack_start(dissimCrit);
    initialBox.pack_start(logFile);
    vBox.pack_start(initialBox);
    outputFilesBox.pack_start(optionalLabel);
    outputFilesBox.pack_start(classLabelsMapFile);
    outputFilesBox.pack_start(boundaryMapFile);
    outputFilesBox.pack_start(regionClassesFile);
    outputFilesBox.pack_start(objectConnType1Button);
    outputFilesBox.pack_start(regionObjectsButton);
    outputFilesBox.pack_start(objectLabelsMapFile);
    outputFilesBox.pack_start(regionObjectsFile);
    outputFilesBox.pack_start(oparamFile);
    outputFilesFrame.add(outputFilesBox);
    vBox.pack_start(outputFilesFrame);

    show_all_children();
    if (!regionObjectsFlag)
    {
      if (params.spclust_wght == 0)
        regionObjectsButton.hide();
      objectLabelsMapFile.hide();
      regionObjectsFile.hide();
    }
    boundaryMapFile.set_activeFlag(false);

    show();
  }

 // Destructor...
  OutputFileGUI::~OutputFileGUI() 
  {
  }

  void OutputFileGUI::on_help_requested()
  {
    Glib::ustring strMessage = "Enter in the program File inputs.\n\nFor more help type "
                               "\"rhseg -help\" on the command line.\n\n";
    Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
    
    dialog.run();
  }

  void OutputFileGUI::on_exit_requested()
  {
    params.status = 3;
    hide();
    return;
  }

  bool OutputFileGUI::on_delete_event(GdkEventAny* event)
  {
    on_exit_requested();
    return true;
  }

  void OutputFileGUI::on_objectConnType1Button_checked()
  {
    objectConnType1Flag = objectConnType1Button.get_active();
    if (objectConnType1Flag)
    {
      regionObjectsButton.show();
      if (regionObjectsFlag)
      {
        objectLabelsMapFile.show();
        regionObjectsFile.show();
      }
    }
    else
    {
      objectLabelsMapFile.hide();
      regionObjectsFile.hide();
    }

    return;
  }

  void OutputFileGUI::on_regionObjectsButton_checked()
  {
    regionObjectsFlag = regionObjectsButton.get_active();
    if (regionObjectsFlag)
    {
      objectLabelsMapFile.show();
      regionObjectsFile.show();
    }
    else
    {
      objectLabelsMapFile.hide();
      regionObjectsFile.hide();
    }

    return;
  }

  void OutputFileGUI::on_run_program()
  {
    on_next_panel();

    params.set_region_sumsq_flag();

    params.status = params.set_maxnbdir();

    if (params.status)
      params.status = params.calc_defaults();

    if (params.status)
      params.status = 2;

    return;
  }

  void OutputFileGUI::on_next_panel()
  {
    Glib::ustring strMessage = " ";

    params.class_labels_map_file = classLabelsMapFile.get_filename();
    if (boundaryMapFile.get_activeFlag())
    {
      params.boundary_map_file = boundaryMapFile.get_filename();
      params.boundary_map_flag = true;
    }
    params.region_classes_file = regionClassesFile.get_filename();

    params.object_conn_type1_flag = objectConnType1Flag;

    if (regionObjectsFlag)
    {
      params.object_labels_map_file = objectLabelsMapFile.get_filename();
      params.object_labels_map_flag = true;

      params.region_objects_file = regionObjectsFile.get_filename();
      params.region_objects_flag = true;
    }

    params.oparam_file = oparamFile.get_filename();

    params.region_nb_objects_flag = ((params.spclust_wght_flag) || (params.object_conn_type1_flag)) && 
                                    (params.object_labels_map_flag) && (params.region_objects_flag);
    params.region_objects_list_flag = params.region_nb_objects_flag;
    if (!params.object_conn_type1_flag)
    {
      if ((!params.spclust_wght_flag) || (!params.object_labels_map_flag) || (!params.region_objects_flag))
      {
        params.region_nb_objects_flag = false;
        params.region_objects_list_flag = false;
      }
    }

    params.status = 1;

    hide();

    return;
  }

} // namespace HSEGTilton
