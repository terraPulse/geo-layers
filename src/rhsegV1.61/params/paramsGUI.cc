// paramsGUI.cc

#include "paramsGUI.h"
#include "params.h"
#include <iostream>
#include <fstream>

extern HSEGTilton::Params params;

namespace HSEGTilton
{
 // Constructor
  ParamsGUI::ParamsGUI():
               vBox(false,10),
               scaleFactorsObject("Input data scale factors (scale, comma delimited list - default 1.0 each band):"),
               offsetFactorsObject("Input data offset factors (offset, comma delimited list - default 0.0 each band):"),
               paramsFrame("Specify Other HSWO/HSeg/RHSeg Parameters (default values provided):"),
               paramsBox(false,10),
               regionSumButton("Include the region sum feature values (region_sum, also sum square and sum logx, if available)"),
               regionStdDevButton("Include the region standard deviation feature values (region_std_dev)"),
               regionBoundaryNpixButton("Include the region boundary number of pixel values (region_boundary_npix)"),
               regionThresholdButton("Include the merge threshold for the most recent merge for each region (region_threshold)"),
//               regionNghbrsListButton("Include the region neighbors list for each region (region_nghbrs_list)"),
               regionNbObjectsButton("Include the number of region objects contained in each region class (region_nb_objects)"),
               regionObjectsListButton("Include the list of region objects contained in each region class (region_objects_list)"),
               stdDevWghtObject("Weight for standard deviation spatial feature (std_dev_wght)",true),
               edgeThresholdObject("Threshold for initial neighborhood merges based on edge information (edge_threshold)",true),
               edgePowerObject("The power to which the edge_value feature is raised  (edge_power)",true),
               edgeWghtObject("Relative weight for edge information (edge_wght)",true),
               seamEdgeThresholdObject("Threshold for across seam edge-based merges for processing window artifact elimination (seam_edge_threshold)",true),
               connTypeLabel("Region Connectivity (conn_type):"),
               chooseFrame("Choose one of the following three methods by which iterations are selected for hierarchical segmentation level output:"),
               chooseBox(false,10),
               chkNregionsObject("Number of regions at which hierarchical segmentation output is initiated (chk_nregions, range 2 to 65535):",false),
               hsegOutNregionsObject("Set of number of regions at which hierarchical segmentation output are to be made (hseg_out_nregions, comma delimited list):",false),
               hsegOutThresholdsObject("Set of merge thresholds at which hierarchical segmentation outputs are to be made (hseg_out_thresholds, comma delimited list):",false),
               convNregionsObject("Number of regions for final convergence (conv_nregions, range 2 to 65535):",true),
               gDissimButton("Include global dissimilarity values in the log file (gdissim)"),
               debugObject("Debug Option (debug)"),
               expParamsButton("Examine/Change Experimental Parameters - Casual users normally would not change these values!")
  {
    if (params.program_mode == 1)
      set_title("HSWO Parameter Specification");
    else if (params.program_mode == 2)
      set_title("HSeg Parameter Specification");
    else
      set_title("RHSeg Parameter Specification");
    set_default_size(512,64);

  // Initialize labels
    inputImageFile.set_label("Input image data file:  " + params.input_image_file);

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

    if (params.mask_flag)
      maskImageFile.set_label("Input mask data file:  " + params.mask_file);

    if (params.std_dev_image_flag)
      stdDevInImageFile.set_label("Input standard deviation image file:  " + params.std_dev_image_file);

    if (params.std_dev_mask_flag)
      stdDevMaskImageFile.set_label("Input standard deviation mask file:  " + params.std_dev_mask_file);

    if (params.edge_image_flag)
      edgeInImageFile.set_label("Input standard deviation image file:  " + params.edge_image_file);

    if (params.edge_mask_flag)
      edgeMaskImageFile.set_label("Input standard deviation mask file:  " + params.edge_mask_file);

    if (params.region_map_in_flag)
      regionMapInImageFile.set_label("Input region map data file:  " + params.region_map_in_file);

    classLabelsMapFile.set_label("Output class labels map file:  " + params.class_labels_map_file);
    if (params.boundary_map_flag)
      boundaryMapFile.set_label("Output hierarchical boundary map file:  " + params.boundary_map_file);
    regionClassesFile.set_label("Output region classes file:" + params.region_classes_file);
    if (params.object_labels_map_flag)
      objectLabelsMapFile.set_label("Output object labels map file:  " + params.object_labels_map_file);
    if (params.region_objects_flag)
      regionObjectsFile.set_label("Output region objects file:  " + params.region_objects_file);
    oparamFile.set_label("Output parameter file:  " + params.oparam_file);

    logFile.set_label("Output log file:  " + params.log_file);

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

  // Initialize other widgets
    regionSumFlag = params.region_sum_flag;
    regionSumButton.set_active(regionSumFlag);
    regionSumButton.set_alignment(0.0,0.5);
    regionStdDevFlag = params.region_std_dev_flag;
    regionStdDevButton.set_active(regionStdDevFlag);
    regionStdDevButton.set_alignment(0.0,0.5);
    regionBoundaryNpixFlag = params.region_boundary_npix_flag;
    regionBoundaryNpixButton.set_active(regionBoundaryNpixFlag);
    regionBoundaryNpixButton.set_alignment(0.0,0.5);
    regionThresholdFlag = params.region_threshold_flag;
    regionThresholdButton.set_active(regionThresholdFlag);
    regionThresholdButton.set_alignment(0.0,0.5);
    regionNghbrsListFlag = params.region_nghbrs_list_flag;
//    regionNghbrsListButton.set_active(regionNghbrsListFlag);
//    regionNghbrsListButton.set_alignment(0.0,0.5);
    regionNbObjectsFlag = params.region_nb_objects_flag;
    regionNbObjectsButton.set_active(regionNbObjectsFlag);
    regionNbObjectsButton.set_alignment(0.0,0.5);
    regionObjectsListFlag = params.region_objects_list_flag;
    regionObjectsListButton.set_active(regionObjectsListFlag);
    regionObjectsListButton.set_alignment(0.0,0.5);
    if (params.std_dev_image_flag)
      stdDevWghtObject.set_fvalue(params.std_dev_wght);
    if (params.edge_image_flag)
    {
      edgeThresholdObject.set_fvalue(params.edge_threshold);
      edgePowerObject.set_fvalue(params.edge_power);
      edgeWghtObject.set_fvalue(params.edge_wght);
    }
    if (params.program_mode == 3)
      seamEdgeThresholdObject.set_fvalue(params.seam_edge_threshold);
    switch (params.nb_dimensions)
    {
#ifdef GTKMM3
       case 1:  connTypeComboBox.append("  Two Nearest Neighbors");
                connTypeComboBox.append(" Four Nearest Neighbors");
                connTypeComboBox.append("  Six Nearest Neighbors");
                connTypeComboBox.append("Eight Nearest Neighbors");
#else
       case 1:  connTypeComboBox.append_text("  Two Nearest Neighbors");
                connTypeComboBox.append_text(" Four Nearest Neighbors");
                connTypeComboBox.append_text("  Six Nearest Neighbors");
                connTypeComboBox.append_text("Eight Nearest Neighbors");
#endif
                switch (params.conn_type)
                {
                  case 1:  connTypeComboBox.set_active_text("  Two Nearest Neighbors");
                           break;
                  case 2:  connTypeComboBox.set_active_text(" Four Nearest Neighbors");
                           break;
                  case 3:  connTypeComboBox.set_active_text("  Six Nearest Neighbors");
                           break;
                  case 4:  connTypeComboBox.set_active_text("Eight Nearest Neighbors");
                           break;
                  default: connTypeComboBox.set_active_text("  Two Nearest Neighbors");
                           break;
                }
                break;
#ifdef GTKMM3
       case 2:  connTypeComboBox.append("  Four Nearest Neighbors");
                connTypeComboBox.append(" Eight Nearest Neighbors");
                connTypeComboBox.append("Twelve Nearest Neighbors");
                connTypeComboBox.append("Twenty Nearest Neighbors");
                connTypeComboBox.append("Twenty-Four Nearest Neighbors");
#else
       case 2:  connTypeComboBox.append_text("  Four Nearest Neighbors");
                connTypeComboBox.append_text(" Eight Nearest Neighbors");
                connTypeComboBox.append_text("Twelve Nearest Neighbors");
                connTypeComboBox.append_text("Twenty Nearest Neighbors");
                connTypeComboBox.append_text("Twenty-Four Nearest Neighbors");
#endif
                switch (params.conn_type)
                {
                  case 1:  connTypeComboBox.set_active_text("  Four Nearest Neighbors");
                           break;
                  case 2:  connTypeComboBox.set_active_text(" Eight Nearest Neighbors");
                           break;
                  case 3:  connTypeComboBox.set_active_text("Twelve Nearest Neighbors");
                           break;
                  case 4:  connTypeComboBox.set_active_text("Twenty Nearest Neighbors");
                           break;
                  case 5:  connTypeComboBox.set_active_text("Twenty-Four Nearest Neighbors");
                           break;
                  default: connTypeComboBox.set_active_text(" Eight Nearest Neighbors");
                           break;
                }
                break;
#ifdef THREEDIM
#ifdef GTKMM3
       case 3:  connTypeComboBox.append("     Six Nearest Neighbors");
                connTypeComboBox.append("Eighteen Nearest Neighbors");
                connTypeComboBox.append("Twenty-Six Nearest Neighbors");
#else
       case 3:  connTypeComboBox.append_text("     Six Nearest Neighbors");
                connTypeComboBox.append_text("Eighteen Nearest Neighbors");
                connTypeComboBox.append_text("Twenty-Six Nearest Neighbors");
#endif
                switch (params.conn_type)
                {
                  case 1:  connTypeComboBox.set_active_text("     Six Nearest Neighbors");
                           break;
                  case 2:  connTypeComboBox.set_active_text("Eighteen Nearest Neighbors");
                           break;
                  case 3:  connTypeComboBox.set_active_text("Twenty-Six Nearest Neighbors");
                           break;
                  default: connTypeComboBox.set_active_text("Twenty-Six Nearest Neighbors");
                           break;
                }
                break;
#endif
       default: cout << "WARNING: Invalid value for nb_dimensions (=" << params.nb_dimensions << ")" << endl;
                break;
    }
    if (params.chk_nregions_flag)
      chkNregionsObject.set_value(params.chk_nregions);
    chkNregionsFlag = params.chk_nregions_flag;
    hsegOutNregionsFlag = params.hseg_out_nregions_flag;
    hsegOutThresholdsFlag = params.hseg_out_thresholds_flag;
    convNregionsObject.set_value(params.conv_nregions);
    gDissimFlag = params.gdissim_flag;
    gDissimButton.set_active(gDissimFlag);
    gDissimButton.set_alignment(0.0,0.5);
    debugObject.set_value(params.debug);

    add(vBox); // put a MenuBar at the top of the box and other stuff below it.

  //Create actions for menus:
    m_refActionGroup = Gtk::ActionGroup::create();

    //Add normal Actions:
    m_refActionGroup->add( Gtk::Action::create("ActionMenu", "Program _Actions") );
    m_refActionGroup->add( Gtk::Action::create("Help", "_Help"),
                      sigc::mem_fun(*this, &ParamsGUI::on_help_requested) );
#ifdef RHSEG_SETUP
    m_refActionGroup->add( Gtk::Action::create("Run", "_Run RHSeg Setup"),
                           sigc::mem_fun(*this, &ParamsGUI::on_run_program) );
#else
    if (params.program_mode == 1)
      m_refActionGroup->add( Gtk::Action::create("Run", "_Run HSWO"),
                        sigc::mem_fun(*this, &ParamsGUI::on_run_program) );
    else if (params.program_mode == 2)
      m_refActionGroup->add( Gtk::Action::create("Run", "_Run HSeg"),
                        sigc::mem_fun(*this, &ParamsGUI::on_run_program) );
    else
      m_refActionGroup->add( Gtk::Action::create("Run", "_Run RHSeg"),
                        sigc::mem_fun(*this, &ParamsGUI::on_run_program) );
#endif
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
          "      <menuitem action='Help'/>"
          "      <separator/>"
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

  // Set up signal handlers
    regionSumButton.signal_clicked().connect(sigc::mem_fun(*this,
                   &ParamsGUI::on_regionSumButton_clicked) );
    regionStdDevButton.signal_clicked().connect(sigc::mem_fun(*this,
                   &ParamsGUI::on_regionStdDevButton_clicked) );
    regionBoundaryNpixButton.signal_clicked().connect(sigc::mem_fun(*this,
                   &ParamsGUI::on_regionBoundaryNpixButton_clicked) );
    regionThresholdButton.signal_clicked().connect(sigc::mem_fun(*this,
                   &ParamsGUI::on_regionThresholdButton_clicked) );
//    regionNghbrsListButton.signal_clicked().connect(sigc::mem_fun(*this,
//                   &ParamsGUI::on_regionNghbrsListButton_clicked) );
    regionNbObjectsButton.signal_clicked().connect(sigc::mem_fun(*this,
                   &ParamsGUI::on_regionNbObjectsButton_clicked) );
    regionObjectsListButton.signal_clicked().connect(sigc::mem_fun(*this,
                   &ParamsGUI::on_regionObjectsListButton_clicked) );
    chkNregionsObject.signal_activated().connect(sigc::mem_fun(*this,
                   &ParamsGUI::on_chkNregionsObject_activated) );
    hsegOutNregionsObject.signal_activated().connect(sigc::mem_fun(*this,
                   &ParamsGUI::on_hsegOutNregionsObject_activated) );
    hsegOutThresholdsObject.signal_activated().connect(sigc::mem_fun(*this,
                   &ParamsGUI::on_hsegOutThresholdsObject_activated) );
    gDissimButton.signal_clicked().connect(sigc::mem_fun(*this,
                   &ParamsGUI::on_gDissimButton_clicked) );
    debugObject.signal_activate().connect(sigc::mem_fun(*this,
                 &ParamsGUI::on_debugObject_activate) );
    expParamsButton.signal_clicked().connect(sigc::mem_fun(*this,
                   &ParamsGUI::on_expParamsButton_clicked) );

  // Pack GUI
    initialBox.pack_start(inputImageFile);
    initialBox.pack_start(inputDescription);
    initialBox.pack_start(scaleFactorsObject);
    initialBox.pack_start(offsetFactorsObject);
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
    vBox.pack_start(initialBox);
    outputFilesBox.pack_start(classLabelsMapFile);
    if (params.boundary_map_flag)
      outputFilesBox.pack_start(boundaryMapFile);
    outputFilesBox.pack_start(regionClassesFile);
    if (params.object_labels_map_flag)
      outputFilesBox.pack_start(objectLabelsMapFile);
    if (params.region_objects_flag)
      outputFilesBox.pack_start(regionObjectsFile);
    outputFilesBox.pack_start(oparamFile);
    outputFilesBox.pack_start(logFile);
    outputFilesBox.pack_start(spClustWght);
    outputFilesBox.pack_start(dissimCrit);
    vBox.pack_start(outputFilesBox);
    paramsBox.pack_start(regionSumButton);
    paramsBox.pack_start(regionStdDevButton);
    paramsBox.pack_start(regionBoundaryNpixButton);
    paramsBox.pack_start(regionThresholdButton);
//    paramsBox.pack_start(regionNghbrsListButton);
    if (params.region_nb_objects_flag)
    {
      paramsBox.pack_start(regionNbObjectsButton);
      paramsBox.pack_start(regionObjectsListButton);
    }
    if (params.std_dev_image_flag)
      paramsBox.pack_start(stdDevWghtObject);
    if (params.edge_image_flag)
    {
      paramsBox.pack_start(edgeThresholdObject);
      paramsBox.pack_start(edgePowerObject);
      paramsBox.pack_start(edgeWghtObject);
    }
    if (params.program_mode == 3)
      paramsBox.pack_start(seamEdgeThresholdObject);
    comboHBox.pack_start(connTypeLabel);
    comboHBox.pack_start(connTypeComboBox);
    paramsBox.pack_start(comboHBox);
    paramsFrame.add(paramsBox);
    vBox.pack_start(paramsFrame);
    chooseBox.pack_start(chkNregionsObject);
    chooseBox.pack_start(hsegOutNregionsObject);
    chooseBox.pack_start(hsegOutThresholdsObject);
    chooseFrame.add(chooseBox);
    vBox.pack_start(chooseFrame);
    vBox.pack_start(convNregionsObject);
    lastHBox.pack_start(gDissimButton);
    lastHBox.pack_start(debugObject);
    vBox.pack_start(lastHBox);
    vBox.pack_start(expParamsButton);

    show_all_children();
    chkNregionsObject.set_activeFlag(chkNregionsFlag);
    hsegOutNregionsObject.set_activeFlag(hsegOutNregionsFlag);
    hsegOutThresholdsObject.set_activeFlag(hsegOutThresholdsFlag);
    expParamsGUI.hide();

    show();
  }

 // Destructor...
  ParamsGUI::~ParamsGUI() 
  {
  }

  void ParamsGUI::on_help_requested()
  {
    Glib::ustring strMessage = "Enter in the program File inputs.\n\nFor more help type "
                               "\"rhseg -help\" on the command line.\n\n";
    Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
    
    dialog.run();

    return;
  }

  void ParamsGUI::on_exit_requested()
  {
    params.status = 3;
    hide();
    return;
  }

  bool ParamsGUI::on_delete_event(GdkEventAny* event)
  {
    on_exit_requested();
    return true;
  }

  void ParamsGUI::on_regionSumButton_clicked()
  {
    if (regionSumFlag)
    {
      regionSumFlag = false;
    }
    else
    {
      regionSumFlag = true;
    }
  }

  void ParamsGUI::on_regionStdDevButton_clicked()
  {
    if (regionStdDevFlag)
    {
      regionStdDevFlag = false;
    }
    else
    {
      regionStdDevFlag = true;
    }
  }

  void ParamsGUI::on_regionBoundaryNpixButton_clicked()
  {
    if (regionBoundaryNpixFlag)
    {
      regionBoundaryNpixFlag = false;
    }
    else
    {
      regionBoundaryNpixFlag = true;
    }
  }

  void ParamsGUI::on_regionThresholdButton_clicked()
  {
    if (regionThresholdFlag)
    {
      regionThresholdFlag = false;
    }
    else
    {
      regionThresholdFlag = true;
    }
  }

//  void ParamsGUI::on_regionNghbrsListButton_clicked()
//  {
//    if (regionNghbrsListFlag)
//    {
//      regionNghbrsListFlag = false;
//    }
//    else
//    {
//      regionNghbrsListFlag = true;
//   }
//  }
//
  void ParamsGUI::on_regionNbObjectsButton_clicked()
  {
    if (regionNbObjectsFlag)
    {
      regionNbObjectsFlag = false;
    }
    else
    {
      regionNbObjectsFlag = true;
    }
  }

  void ParamsGUI::on_regionObjectsListButton_clicked()
  {
    if (regionObjectsListFlag)
    {
      regionObjectsListFlag = false;
cout << "regionObjectsListFlag set to false" << endl;
    }
    else
    {
      regionObjectsListFlag = true;
cout << "regionObjectsListFlag set to true" << endl;
    }
  }

  void ParamsGUI::on_chkNregionsObject_activated()
  {
    if (chkNregionsFlag)
    {
      chkNregionsFlag = false;
    }
    else
    {
      chkNregionsFlag = true;
      hsegOutNregionsFlag = false;
      hsegOutThresholdsFlag = false;
      hsegOutNregionsObject.set_activeFlag(hsegOutNregionsFlag);
      hsegOutThresholdsObject.set_activeFlag(hsegOutThresholdsFlag);
    }
  }

  void ParamsGUI::on_hsegOutNregionsObject_activated()
  {
    if (hsegOutNregionsFlag)
    {
      hsegOutNregionsFlag = false;
    }
    else
    {
      chkNregionsFlag = false;
      hsegOutNregionsFlag = true;
      hsegOutThresholdsFlag = false;
      chkNregionsObject.set_activeFlag(chkNregionsFlag);
      hsegOutThresholdsObject.set_activeFlag(hsegOutThresholdsFlag);
    }
  }

  void ParamsGUI::on_hsegOutThresholdsObject_activated()
  {
    if (hsegOutThresholdsFlag)
    {
      hsegOutThresholdsFlag = false;
    }
    else
    {
      chkNregionsFlag = false;
      hsegOutNregionsFlag = false;
      hsegOutThresholdsFlag = true;
      chkNregionsObject.set_activeFlag(chkNregionsFlag);
      hsegOutNregionsObject.set_activeFlag(hsegOutNregionsFlag);
    }
  }

  void ParamsGUI::on_gDissimButton_clicked()
  {
    if (gDissimFlag)
    {
      gDissimFlag = false;
    }
    else
    {
      gDissimFlag = true;
    }
    return;
  }

  void ParamsGUI::on_debugObject_activate()
  {
    params.debug = debugObject.get_value();
cout << "debug changed to " << params.debug << endl;
    
    return;
  }

  void ParamsGUI::on_expParamsButton_clicked()
  {
    params.set_region_sumsq_flag();

    params.status = params.set_maxnbdir();

    if (params.status)
      params.status = params.calc_defaults();

    expParamsGUI.show();

    return;
  }

  void ParamsGUI::on_run_program()
  {
    int index, list_size;
    string tmp_string;

    params.region_sum_flag = regionSumFlag;
    params.region_std_dev_flag = regionStdDevFlag;
    params.region_boundary_npix_flag = regionBoundaryNpixFlag;
    params.region_threshold_flag = regionThresholdFlag;
    params.region_nghbrs_list_flag = regionNghbrsListFlag;
    params.region_nb_objects_flag = regionNbObjectsFlag;
    params.region_objects_list_flag = regionObjectsListFlag;

    list_size = scaleFactorsObject.get_size();
    if (list_size > 0)
    {
      if (list_size > params.nbands)
        list_size = params.nbands;
      for (index = 0; index < list_size; index++)
        params.scale[index] = scaleFactorsObject.get_fentry(index);
    }
    list_size = offsetFactorsObject.get_size();
    if (list_size > 0)
    {
      if (list_size > params.nbands)
        list_size = params.nbands;
      for (index = 0; index < list_size; index++)
        params.offset[index] = offsetFactorsObject.get_fentry(index);
    }

    if (params.std_dev_image_flag)
      params.std_dev_wght = stdDevWghtObject.get_fvalue();
    if (params.edge_image_flag)
    {
      params.edge_threshold = edgeThresholdObject.get_fvalue();
      params.edge_power = edgePowerObject.get_fvalue();
      params.edge_wght = edgeWghtObject.get_fvalue();
    }
    if (params.program_mode == 3)
    {
      params.seam_edge_threshold = seamEdgeThresholdObject.get_fvalue();
    }

    tmp_string = connTypeComboBox.get_active_text();
    switch (params.nb_dimensions)
    {
       case 1:  if (tmp_string == "  Two Nearest Neighbors")
                  params.conn_type = 1;
                else if (tmp_string == " Four Nearest Neighbors")
                  params.conn_type = 2;
                else if (tmp_string == "  Six Nearest Neighbors")
                  params.conn_type = 3;
                else if (tmp_string == "Eight Nearest Neighbors")
                  params.conn_type = 4;
                break;
       case 2:  if (tmp_string == "  Four Nearest Neighbors")
                  params.conn_type = 1;
                else if (tmp_string == " Eight Nearest Neighbors")
                  params.conn_type = 2;
                else if (tmp_string == "Twelve Nearest Neighbors")
                  params.conn_type = 3;
                else if (tmp_string == "Twenty Nearest Neighbors")
                  params.conn_type = 4;
                else if (tmp_string == "Twenty-Four Nearest Neighbors")
                  params.conn_type = 5;
                break;
#ifdef THREEDIM
       case 3:  if (tmp_string == "     Six Nearest Neighbors")
                  params.conn_type = 1;
                else if (tmp_string == "Eighteen Nearest Neighbors")
                  params.conn_type = 2;
                else if (tmp_string == "Twenty-Six Nearest Neighbors")
                  params.conn_type = 3;
                break;
#endif
       default: cout << "WARNING: Invalid value for nb_dimensions (=" << params.nb_dimensions << ")" << endl;
                break;
    }

    params.chk_nregions_flag = chkNregionsObject.get_activeFlag();
    if (params.chk_nregions_flag)
    {
      params.chk_nregions = chkNregionsObject.get_value();
      params.hseg_out_nregions_flag = false;
      params.hseg_out_thresholds_flag = false;
    }

    params.hseg_out_nregions_flag = hsegOutNregionsObject.get_activeFlag();
    if (params.hseg_out_nregions_flag)
    {
      params.nb_hseg_out_nregions = hsegOutNregionsObject.get_size();
      params.hseg_out_nregions.resize(params.nb_hseg_out_nregions);
      for (index = 0; index < params.nb_hseg_out_nregions; index++)
        params.hseg_out_nregions[index] = hsegOutNregionsObject.get_entry(index);
      params.chk_nregions_flag = false;
      params.hseg_out_thresholds_flag = false;
    }

    params.hseg_out_thresholds_flag = hsegOutThresholdsObject.get_activeFlag();
    if (params.hseg_out_thresholds_flag)
    {
      params.nb_hseg_out_thresholds = hsegOutThresholdsObject.get_size();
      params.hseg_out_thresholds.resize(params.nb_hseg_out_thresholds);
      for (index = 0; index < params.nb_hseg_out_thresholds; index++)
        params.hseg_out_thresholds[index] = hsegOutThresholdsObject.get_fentry(index);
      params.hseg_out_nregions_flag = false;
      params.chk_nregions_flag = false;
    }

    params.conv_nregions = convNregionsObject.get_value();
    params.gdissim_flag = gDissimFlag;
    params.debug = debugObject.get_value();

    params.set_region_sumsq_flag();

    params.status = params.set_maxnbdir();

    if (params.status)
      params.status = params.calc_defaults();

    if (params.status)
      params.status = 2;

    hide();

    return;
  }

} // namespace HSEGTilton
