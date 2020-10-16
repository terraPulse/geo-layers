// hseglearn.cc
#include "hseglearn.h"
#include "params/initialParams.h"
#include <params/params.h>
#include <iostream>

extern HSEGTilton::InitialParams initialParams;
extern HSEGTilton::Params params;
extern HSEGTilton::oParams oparams;
extern CommonTilton::Image inputImage;
extern CommonTilton::Image maskImage;
CommonTilton::Image classLabelsMapImage;
CommonTilton::Image panchromaticImage;
CommonTilton::Image referenceImage;
CommonTilton::Image segLevelClassLabelsMapImage;
CommonTilton::Image labelDataImage, saved_labelDataImage;
CommonTilton::Image regionClassImage, saved_regionClassImage;
CommonTilton::Image panSubsetImage;

using namespace CommonTilton;

namespace HSEGTilton
{
 // Constructor
  HSegLearn::HSegLearn():
              vBox(false,10),
              displayOptionsFrame("  Display Options:"), displayOptionsTable(3,4,true),
              selectOptionsFrame("  Region Highlight and Selection Options:"),
#ifdef AMBIGUOUS
              selectOptionsTable(4,4,true), submissionOptionsFrame("  Region Submission Options:"), 
#else
              selectOptionsTable(3,4,true), submissionOptionsFrame("  Region Submission Options:"), 
#endif
#ifdef AMBIGUOUS
              submissionOptionsTable(3,4,true),
#else
              submissionOptionsTable(2,4,true),
#endif
              rgbImageButton("RGB Image"), regionLabelButton("Current Region Labels"),
              panchromaticImageButton("Panchromatic Image"), referenceImageButton("Reference Image"),
              refocusButton("Refocus Displays at location of last Highlighted Region"),
              displayLogButton("Display the HSeg/RHSeg log file"),
              updateRGBImageButton("Update RGB Image Display"),
              redDisplayBand("Red Display Band:"),
              greenDisplayBand("Green Display Band:"),
              blueDisplayBand("Blue Display Band:"),
              rgbImageStretchLabel("RGB Image Stretch Option:"),
              rangeFromObject("Range From: "),
              rangeToObject("To: "),
              highlightOptionsLabel("Choose Highlight Regions Mode from Display Panel Actions Menu."),
              undoLastHighlightedClassesButton("Undo Last Region Highlight(s)"),
              clearAllHighlightedClassesButton("Clear All Highlighted Regions"),
              selectPosExampleButton("Select Highlighted Region(s) as Positive Example(s)"),
              selectNegExampleButton("Select Highlighted Region(s) as Negative Example(s)"),
              selectAmbExampleButton("Select Highlighted Region(s) as Ambiguous Example(s)"),
              undoLastSubmitButton("Undo Last Submit of Positive and/or Negative Example(s)"),
              submitExamplesButton("Submit Selected Positive and/or Negative Example(s)"),
              submitPosExampleButton("Submit Highlighted Region(s) as a Positive Example(s)"),
              submitNegExampleButton("Submit Highlighted Region(s) as a Negative Example(s)"),
              submitAmbExampleButton("Submit Highlighted Region(s) as a Ambiguous Example(s)")
  {
#ifdef THREEDIM
    switch (initialParams.view_dimension)
    {
      case COLUMN: view_ncols = params.nrows;
                   view_nrows = params.nslices;
                   break;
      case    ROW: view_ncols = params.ncols;
                   view_nrows = params.nslices;
                   break;
      case  SLICE: view_ncols = params.ncols;
                   view_nrows = params.nrows;
                   break;
    }
    subsetfrom3d();
#else
    view_ncols = params.ncols;
    view_nrows = params.nrows;
#endif
    nb_classes = oparams.level0_nb_classes;
    region_classes_size = nb_classes;
    region_classes.resize(region_classes_size,RegionClass());

    nb_objects = 0;
    if (params.region_nb_objects_flag)
      nb_objects = oparams.level0_nb_objects;
    region_objects_size = nb_objects;
    if (region_objects_size > 0)
      region_objects.resize(region_objects_size,RegionObject());

    initializeImages();
    
    DisplayImage::set_static_values(inputImage,params.current_folder,true,true);

    if (params.mask_flag)
      rgbImageDisplay.init_rgb(inputImage,maskImage,"RGB Image");
    else
      rgbImageDisplay.init_rgb(inputImage,"RGB Image");

    labelDataDisplay.init_region_label(labelDataImage,"Current Region Labels");

    if (initialParams.panchromatic_image_flag)
      panImageDisplay.init_float(panSubsetImage,"Panchromatic Image");

    if (initialParams.reference_image_flag)
      referenceImageDisplay.init_reference(referenceImage,"Reference Image");

  // Link the DisplayImages
    rgbImageDisplay.link_image(&labelDataDisplay);
    if (initialParams.panchromatic_image_flag)
      rgbImageDisplay.link_image(&panImageDisplay);
    if (initialParams.reference_image_flag)
      rgbImageDisplay.link_image(&referenceImageDisplay);

    labelDataDisplay.link_image(&rgbImageDisplay);
    if (initialParams.panchromatic_image_flag)
      labelDataDisplay.link_image(&panImageDisplay);
    if (initialParams.reference_image_flag)
      labelDataDisplay.link_image(&referenceImageDisplay);

    if (initialParams.panchromatic_image_flag)
    {
      panImageDisplay.link_image(&rgbImageDisplay);
      panImageDisplay.link_image(&labelDataDisplay);
      if (initialParams.reference_image_flag)
        panImageDisplay.link_image(&referenceImageDisplay);
    }

    if (initialParams.reference_image_flag)
    {
      referenceImageDisplay.link_image(&rgbImageDisplay);
      referenceImageDisplay.link_image(&labelDataDisplay);
      if (initialParams.panchromatic_image_flag)
        referenceImageDisplay.link_image(&panImageDisplay);
    }

  //Build the GUI panel
    set_title("Hierarchical Segmentation Learn by Example program");

    add(vBox); // put a MenuBar at the top of the box and other stuff below it.

  //Create actions for menus:
    m_refActionGroup = Gtk::ActionGroup::create();

    //Add normal Actions:
    m_refActionGroup->add( Gtk::Action::create("ActionMenu", "Program _Actions") );
    m_refActionGroup->add( Gtk::Action::create("Help", "_Help"),
                      sigc::mem_fun(*this, &HSegLearn::on_help_requested) );
    m_refActionGroup->add( Gtk::Action::create("Quit", "_Quit"),
                      sigc::mem_fun(*this, &HSegLearn::on_quit_requested) );

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
          "      <menuitem action='Quit'/>"
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
      cerr << "building menus failed: " <<  ex.what();
    }
#else
    auto_ptr<Glib::Error> ex;
    m_refUIManager->add_ui_from_string(ui_info, ex);
    if(ex.get())
    {
      cerr << "building menus failed: " <<  ex->what();
    }
#endif //GLIBMM_EXCEPTIONS_ENABLED

  //Get the menubar widget, and add it to a container widget:
    Gtk::Widget* pMenubar = m_refUIManager->get_widget("/MenuBar");
    if(pMenubar)
      vBox.pack_start(*pMenubar, Gtk::PACK_SHRINK);

  // Set up other signals
    rgbImageButton.signal_clicked().connect(sigc::mem_fun(*this,
                       &HSegLearn::on_rgbImageButton_clicked) );
    regionLabelButton.signal_clicked().connect(sigc::mem_fun(*this,
                       &HSegLearn::on_regionLabelButton_clicked) );
    if (initialParams.panchromatic_image_flag)
      panchromaticImageButton.signal_clicked().connect(sigc::mem_fun(*this,
                         &HSegLearn::on_panchromaticImageButton_clicked) );
    if (initialParams.reference_image_flag)
      referenceImageButton.signal_clicked().connect(sigc::mem_fun(*this,
                         &HSegLearn::on_referenceImageButton_clicked) );
    refocusButton.signal_clicked().connect(sigc::mem_fun(*this,
                       &HSegLearn::on_refocusButton_clicked) );
    displayLogButton.signal_clicked().connect(sigc::mem_fun(*this,
                       &HSegLearn::on_displayLogButton_clicked) );
    updateRGBImageButton.signal_clicked().connect(sigc::mem_fun(*this,
                       &HSegLearn::on_updateRGBImageButton_clicked) );
    rgbImageStretchComboBox.signal_changed().connect(sigc::mem_fun(*this,
                         &HSegLearn::on_rgbImageStretchComboBox_changed) );
    undoLastHighlightedClassesButton.signal_clicked().connect(sigc::mem_fun(*this,
                       &HSegLearn::on_undoLastHighlightedClassesButton_clicked) );
    clearAllHighlightedClassesButton.signal_clicked().connect(sigc::mem_fun(*this,
                       &HSegLearn::on_clearAllHighlightedClassesButton_clicked) );
    selectPosExampleButton.signal_clicked().connect(sigc::mem_fun(*this,
                       &HSegLearn::on_selectPosExampleButton_clicked) );
    selectNegExampleButton.signal_clicked().connect(sigc::mem_fun(*this,
                       &HSegLearn::on_selectNegExampleButton_clicked) );
    selectAmbExampleButton.signal_clicked().connect(sigc::mem_fun(*this,
                       &HSegLearn::on_selectAmbExampleButton_clicked) );
    undoLastSubmitButton.signal_clicked().connect(sigc::mem_fun(*this,
                       &HSegLearn::on_undoLastSubmitButton_clicked) );
    submitExamplesButton.signal_clicked().connect(sigc::mem_fun(*this,
                       &HSegLearn::on_submitExamplesButton_clicked) );
    submitPosExampleButton.signal_clicked().connect(sigc::mem_fun(*this,
                       &HSegLearn::on_submitPosExampleButton_clicked) );
    submitNegExampleButton.signal_clicked().connect(sigc::mem_fun(*this,
                       &HSegLearn::on_submitNegExampleButton_clicked) );
    submitAmbExampleButton.signal_clicked().connect(sigc::mem_fun(*this,
                       &HSegLearn::on_submitAmbExampleButton_clicked) );
    rgbImageDisplay.signal_button_press_event().connect(sigc::mem_fun(*this,
                         &HSegLearn::on_rgbImageDisplay_buttonPress_event) );
    labelDataDisplay.signal_button_press_event().connect(sigc::mem_fun(*this,
                         &HSegLearn::on_labelDataDisplay_buttonPress_event) );
    panImageDisplay.signal_button_press_event().connect(sigc::mem_fun(*this,
                         &HSegLearn::on_panImageDisplay_buttonPress_event) );
    referenceImageDisplay.signal_button_press_event().connect(sigc::mem_fun(*this,
                         &HSegLearn::on_referenceImageDisplay_buttonPress_event) );
    labelDataDisplay.signal_circle_roi_event().connect(sigc::mem_fun(*this,
                         &HSegLearn::on_labelDataDisplay_circleROI_event) );

  // Complete initialization of widgets
    displayOptionsTable.attach(rgbImageButton,0,1,0,1);
    displayOptionsTable.attach(regionLabelButton,1,2,0,1);
    if (initialParams.panchromatic_image_flag)
      displayOptionsTable.attach(panchromaticImageButton,2,3,0,1);
    if (initialParams.reference_image_flag)
      displayOptionsTable.attach(referenceImageButton,3,4,0,1);
    displayOptionsTable.attach(refocusButton,0,2,1,2);
    displayOptionsTable.attach(displayLogButton,2,4,1,2);
    displayOptionsTable.attach(updateRGBImageButton,1,3,2,3);
    displayOptionsTable.set_col_spacings(2);
    displayOptionsFrame.set_label_align(0.0);
    displayOptionsFrame.add(displayOptionsTable);

    selectOptionsTable.attach(highlightOptionsLabel,0,4,0,1);
    selectOptionsTable.attach(undoLastHighlightedClassesButton,0,2,1,2);
    selectOptionsTable.attach(clearAllHighlightedClassesButton,2,4,1,2);
    selectOptionsTable.attach(selectPosExampleButton,0,2,2,3);
    selectOptionsTable.attach(selectNegExampleButton,2,4,2,3);
#ifdef AMBIGUOUS
    selectOptionsTable.attach(selectAmbExampleButton,1,3,3,4);
#endif
    selectOptionsTable.set_col_spacings(2);
    selectOptionsFrame.set_label_align(0.0);
    selectOptionsFrame.add(selectOptionsTable);

    submissionOptionsTable.attach(undoLastSubmitButton,0,4,0,1);
    submissionOptionsTable.attach(submitExamplesButton,0,4,0,1);
    submissionOptionsTable.attach(submitPosExampleButton,0,2,1,2);
    submissionOptionsTable.attach(submitNegExampleButton,2,4,1,2);
#ifdef AMBIGUOUS
    submissionOptionsTable.attach(submitAmbExampleButton,1,3,2,3);
#endif
    submissionOptionsTable.set_col_spacings(2);
    submissionOptionsFrame.set_label_align(0.0);
    submissionOptionsFrame.add(submissionOptionsTable);

  //Pack the GUI panel
    vBox.pack_start(displayOptionsFrame);
    redDisplayBand.set_value(initialParams.red_display_band);
    greenDisplayBand.set_value(initialParams.green_display_band);
    blueDisplayBand.set_value(initialParams.blue_display_band);
    displayBox.pack_start(redDisplayBand);
    displayBox.pack_start(greenDisplayBand);
    displayBox.pack_start(blueDisplayBand);    
    vBox.pack_start(displayBox);
#ifdef GTKMM3
    rgbImageStretchComboBox.append("Linear Stretch with Percent Clipping");
    rgbImageStretchComboBox.append("      Histogram Equalization        ");
    rgbImageStretchComboBox.append(" Linear Stretch to Percentile Range ");
#else
    rgbImageStretchComboBox.append_text("Linear Stretch with Percent Clipping");
    rgbImageStretchComboBox.append_text("      Histogram Equalization        ");
    rgbImageStretchComboBox.append_text(" Linear Stretch to Percentile Range ");
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
    rgbImageStretchBox.pack_start(rgbImageStretchLabel);
    rgbImageStretchBox.pack_start(rgbImageStretchComboBox);
    vBox.pack_start(rgbImageStretchBox);
    rangeFromObject.set_fvalue(initialParams.range[0]);
    rangeToObject.set_fvalue(initialParams.range[1]);
    rangeBox.pack_start(rangeFromObject);
    rangeBox.pack_start(rangeToObject);
    vBox.pack_start(rangeBox);
    vBox.pack_start(selectOptionsFrame);
    vBox.pack_start(submissionOptionsFrame);

   // Read the region class and object data for the initial hierarchical level.
    segLevel = INIT_SEG_LEVEL;
    results_data.set_buffer_sizes(params.nbands,nb_classes,nb_objects);
    results_data.open_input(params.region_classes_file,params.region_objects_file);
    results_data.read(segLevel,nb_classes,nb_objects,oparams.nb_levels,oparams.int_buffer_size,region_classes,region_objects);
    results_data.close_input();

    if (initialParams.examples_in_flag)
    {
      read_examples_in();
      submit_amb_examples();
      submit_neg_examples();
      submit_pos_examples();
      write_examples_out();
      labelDataDisplay.reinit_region_label();
    }
    save_current_state();

    selClickFlag = false;
    selClassLabelFlag = false;
    selROIFlag = false;

    show_all_children();

    if (initialParams.rgb_image_stretch == 2)
      rangeBox.hide();
    undoLastHighlightedClassesButton.hide();
    clearAllHighlightedClassesButton.hide();
    selectPosExampleButton.hide();
    selectNegExampleButton.hide();
    selectAmbExampleButton.hide();
    undoLastSubmitButton.hide();
    submitExamplesButton.hide();
    submitPosExampleButton.hide();
    submitNegExampleButton.hide();
    submitAmbExampleButton.hide();

    show();

    return;
  }

  HSegLearn::~HSegLearn()
  {
    return;
  }


  void HSegLearn::on_help_requested()
  {
    Glib::ustring strMessage = "Label the highlighted region, enter a\n"
                               "different region label to be highlighted,\n"
                               "or select the location for the next region\n"
                               "to be highlighted.\n\n"
                               "NOTE: For all number entry boxes, you need to\n"
                               "press the \"Enter\" key in order to register\n"
                               "or \"activate\" the number that you type in.";
    Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
    
    dialog.run();

    return;
  }

  void HSegLearn::on_quit_requested()
  {
    hide();

    logWindow.hide();
    rgbImageDisplay.hide();
    labelDataDisplay.hide();
    panImageDisplay.hide();
    referenceImageDisplay.hide();

    Image labelOutImage;
    labelOutImage.create(initialParams.label_out_file,view_ncols,view_nrows,1,GDT_Byte,inputImage.get_driver_description());
    if (inputImage.geotransform_valid())
      labelOutImage.set_geotransform(inputImage);
    labelOutImage.registered_data_copy(0,labelDataImage,0);
    labelOutImage.copy_colormap(labelDataImage);
    labelOutImage.close();

   // Remove temporary files
    rgbImageDisplay.remove_display_file();
    panImageDisplay.remove_display_file();
    referenceImageDisplay.remove_display_file();

#ifdef THREEDIM
    std::remove(initialParams.view_input_image_file.c_str());
    if (params.mask_flag)
      std::remove(initialParams.view_mask_image_file.c_str());
#endif
    std::remove(initialParams.seg_level_classes_map_file.c_str());
    std::remove(initialParams.label_data_file.c_str());
    std::remove(initialParams.region_map_file.c_str());
    std::remove(initialParams.pan_subset_image_file.c_str());

    return;
  }

  void HSegLearn::on_rgbImageButton_clicked()
  {
    rgbImageDisplay.show();

    return;
  }

  void HSegLearn::on_regionLabelButton_clicked()
  {
    labelDataDisplay.show();

    return;
  }

  void HSegLearn::on_panchromaticImageButton_clicked()
  {
    panImageDisplay.show();

    return;
  }

  void HSegLearn::on_referenceImageButton_clicked()
  {
    referenceImageDisplay.show();

    return;
  }

  void HSegLearn::on_refocusButton_clicked()
  {
    if (selClickFlag)
    {
      labelDataDisplay.set_click_location(selColClick,selRowClick);
      labelDataDisplay.set_focus(true);
      labelDataDisplay.set_linked_focus(true);
    }
    else
    {
      Glib::ustring strMessage = "You must select a region class\n"
                                 "or circle a region of interest\n"
                                 "before you can perform a refocus.";
      Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
    
      dialog.run();
      return;
    }
  }

  void HSegLearn::on_displayLogButton_clicked()
  {
    logWindow.show();

    return;
  }

  void HSegLearn::on_updateRGBImageButton_clicked()
  {
    Glib::ustring tmp_string;

    initialParams.red_display_band = redDisplayBand.get_value();
    initialParams.red_display_flag = true;
    initialParams.green_display_band = greenDisplayBand.get_value();
    initialParams.green_display_flag = true;
    initialParams.blue_display_band = blueDisplayBand.get_value();
    initialParams.blue_display_flag = true;
    if ((initialParams.red_display_band < 0) || (initialParams.red_display_band >= params.nbands))
      initialParams.red_display_flag = false;
    if ((initialParams.green_display_band < 0) || (initialParams.green_display_band >= params.nbands))
      initialParams.green_display_flag = false;
    if ((initialParams.blue_display_band < 0) || (initialParams.blue_display_band >= params.nbands))
      initialParams.blue_display_flag = false;
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
      inputImage.set_rgb_display_bands(initialParams.red_display_band,initialParams.green_display_band,
                                       initialParams.blue_display_band);
      inputImage.computeHistoEqMap(inputImage.get_red_display_band(),256);
      if (inputImage.get_green_display_band() != inputImage.get_red_display_band())
        inputImage.computeHistoEqMap(inputImage.get_green_display_band(),256);
      if ((inputImage.get_blue_display_band() != inputImage.get_red_display_band()) &&
          (inputImage.get_blue_display_band() != inputImage.get_green_display_band()))
        inputImage.computeHistoEqMap(inputImage.get_blue_display_band(),256);
      inputImage.set_rgb_image_stretch(initialParams.rgb_image_stretch,initialParams.range[0],initialParams.range[1]);
      rgbImageDisplay.reinit_rgb();
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

  void HSegLearn::on_rgbImageStretchComboBox_changed()
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

  void HSegLearn::on_undoLastHighlightedClassesButton_clicked()
  {
    int index, size;

    labelDataDisplay.reset_display_highlight();
    labelDataDisplay.reinit_region_label();
    labelDataDisplay.set_button_press_monitor_flag(false);

    selClassLabelFlag = false;
    selROIFlag = false;

    undoLastHighlightedClassesButton.hide();
    selection_list.clear();
    size = saved_selection_list.size();
    if (size == 0)
    {
      selectPosExampleButton.hide();
      selectNegExampleButton.hide();
      selectAmbExampleButton.hide();
      submitExamplesButton.hide();
      submitPosExampleButton.hide();
      submitNegExampleButton.hide();
      submitAmbExampleButton.hide();
    }
    else
    {
      selection_list.clear();
      for (index = 0; index < size; index++)
        selection_list.push_back(saved_selection_list[index]);
      selClassLabel = selection_list[0];
cout << "ReHighlighting region class " << selClassLabel << endl;
      labelDataDisplay.set_display_highlight(selClassLabel,classLabelsMapImage,true);
      for (index = 1; index < size; index++)
      {
        selClassLabel = selection_list[index];
cout << "ReHighlighting region class " << selClassLabel << endl;
        labelDataDisplay.set_display_highlight(selClassLabel,classLabelsMapImage,false);
      }
      labelDataDisplay.reinit_region_label();
    }

    return;
  }

  void HSegLearn::on_clearAllHighlightedClassesButton_clicked()
  {
    labelDataDisplay.reset_display_highlight();
    labelDataDisplay.reinit_region_label();
    labelDataDisplay.set_button_press_monitor_flag(false);

    selClassLabelFlag = false;
    selROIFlag = false;

    undoLastHighlightedClassesButton.hide();
    clearAllHighlightedClassesButton.hide();
    selection_list.clear();

    selectPosExampleButton.hide();
    selectNegExampleButton.hide();
    selectAmbExampleButton.hide();
    submitExamplesButton.hide();
    submitPosExampleButton.hide();
    submitNegExampleButton.hide();
    submitAmbExampleButton.hide();

    return;
  }

  void HSegLearn::on_selectPosExampleButton_clicked()
  {
    int index, size;

    size = selection_list.size();
//    cout << "Entering on_selectPosExampleButton_clicked, selection_list.size = " << size << endl;
    if (size == 0)
    {
      Glib::ustring strMessage = "You must select a region class\n"
                                 "or circle a region of interest\n"
                                 "before selecting positive example(s).";
      Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
      dialog.run();
      return;
    }

    highlight_label = 4;
    for (index = 0; index < size; index++)
    {
      selClassLabel = selection_list[index];
    // If already in amb_selection_list, don't do anything
      if (!(find(amb_selection_list.begin(),amb_selection_list.end(),selClassLabel) != amb_selection_list.end()))
      {
       // If already in neg_selection_list, take out of neg_selection_list and put in amb_selection_list
        if (find(neg_selection_list.begin(),neg_selection_list.end(),selClassLabel) != neg_selection_list.end())
        {
          neg_selection_list.erase(find(neg_selection_list.begin(),neg_selection_list.end(),selClassLabel));
          amb_selection_list.push_back(selClassLabel);
        }
      // Otherwise, if not already in pos_selection_list, put in pos_selection_list and highlight
        else if (!(find(pos_selection_list.begin(),pos_selection_list.end(),selClassLabel) != pos_selection_list.end()))
        {
          pos_selection_list.push_back(selClassLabel);
          labelDataDisplay.set_display_highlight(selClassLabel,classLabelsMapImage);
          update_selection_label();
          labelDataDisplay.reset_display_highlight();
        }
      }
    }
    labelDataDisplay.reinit_region_label();
    labelDataDisplay.set_button_press_monitor_flag(false);

    selection_list.clear();
    saved_selection_list.clear();

    selClassLabelFlag = false;
    selROIFlag = false;
    
    undoLastHighlightedClassesButton.hide();
    clearAllHighlightedClassesButton.hide();

    selectPosExampleButton.hide();
    selectNegExampleButton.hide();
    selectAmbExampleButton.hide();
    undoLastSubmitButton.hide();
    submitExamplesButton.show();
    submitPosExampleButton.hide();
    submitNegExampleButton.hide();
    submitAmbExampleButton.hide();

    return;
  }

  void HSegLearn::on_selectNegExampleButton_clicked()
  {
    int index, size;

    size = selection_list.size();
//    cout << "Entering on_selectNegExampleButton_clicked, selection_list.size = " << size << endl;
    if (size == 0)
    {
      Glib::ustring strMessage = "You must select a region class\n"
                                 "or circle a region of interest\n"
                                 "before selecting negative example(s).";
      Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
      dialog.run();
      return;
    }

    highlight_label = 4;
    for (index = 0; index < size; index++)
    {
      selClassLabel = selection_list[index];
    // If already in amb_selection_list, don't do anything
      if (!(find(amb_selection_list.begin(),amb_selection_list.end(),selClassLabel) != amb_selection_list.end()))
      {
       // If already in pos_selection_list, take out of pos_selection_list and put in amb_selection_list
        if (find(pos_selection_list.begin(),pos_selection_list.end(),selClassLabel) != pos_selection_list.end())
        {
          pos_selection_list.erase(find(pos_selection_list.begin(),pos_selection_list.end(),selClassLabel));
          amb_selection_list.push_back(selClassLabel);
        }
      // Otherwise, if not already in neg_selection_list, put in neg_selection_list and highlight
        else if (!(find(neg_selection_list.begin(),neg_selection_list.end(),selClassLabel) != neg_selection_list.end()))
        {
          neg_selection_list.push_back(selClassLabel);
          labelDataDisplay.set_display_highlight(selClassLabel,classLabelsMapImage);
          update_selection_label();
          labelDataDisplay.reset_display_highlight();
        }
      }
    }
    labelDataDisplay.reinit_region_label();
    labelDataDisplay.set_button_press_monitor_flag(false);

    selection_list.clear();
    saved_selection_list.clear();

    selClassLabelFlag = false;
    selROIFlag = false;
    
    undoLastHighlightedClassesButton.hide();
    clearAllHighlightedClassesButton.hide();

    selectPosExampleButton.hide();
    selectNegExampleButton.hide();
    selectAmbExampleButton.hide();
    undoLastSubmitButton.hide();
    submitExamplesButton.show();
    submitPosExampleButton.hide();
    submitNegExampleButton.hide();
    submitAmbExampleButton.hide();

    return;
  }

  void HSegLearn::on_selectAmbExampleButton_clicked()
  {
    int index, size;

    size = selection_list.size();
//    cout << "Entering on_selectAmbExampleButton_clicked, selection_list.size = " << size << endl;
    if (size == 0)
    {
      Glib::ustring strMessage = "You must select a region class\n"
                                 "or circle a region of interest\n"
                                 "before selecting negative example(s).";
      Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
      dialog.run();
      return;
    }

    highlight_label = 4;
    for (index = 0; index < size; index++)
    {
      selClassLabel = selection_list[index];
    // If already in amb_selection_list, don't do anything
      if (!(find(amb_selection_list.begin(),amb_selection_list.end(),selClassLabel) != amb_selection_list.end()))
      {
       // If already in pos_selection_list, take out of pos_selection_list.
        if (find(pos_selection_list.begin(),pos_selection_list.end(),selClassLabel) != pos_selection_list.end())
        {
          pos_selection_list.erase(find(pos_selection_list.begin(),pos_selection_list.end(),selClassLabel));
        }
      // Otherwise, if already in neg_selection_liset, take out of neg_selection_list
        else if (!(find(neg_selection_list.begin(),neg_selection_list.end(),selClassLabel) != neg_selection_list.end()))
        {
          neg_selection_list.erase(find(neg_selection_list.begin(),neg_selection_list.end(),selClassLabel));
        }
      // Put in neg_selection_list and highlight
        amb_selection_list.push_back(selClassLabel);
        labelDataDisplay.set_display_highlight(selClassLabel,classLabelsMapImage);
        update_selection_label();
        labelDataDisplay.reset_display_highlight();
      }
    }
    labelDataDisplay.reinit_region_label();
    labelDataDisplay.set_button_press_monitor_flag(false);

    selection_list.clear();
    saved_selection_list.clear();

    selClassLabelFlag = false;
    selROIFlag = false;

    undoLastHighlightedClassesButton.hide();
    clearAllHighlightedClassesButton.hide();
    
    selectPosExampleButton.hide();
    selectNegExampleButton.hide();
    selectAmbExampleButton.hide();
    undoLastSubmitButton.hide();
    submitExamplesButton.show();
    submitPosExampleButton.hide();
    submitNegExampleButton.hide();
    submitAmbExampleButton.hide();

    return;
  }

  void HSegLearn::on_undoLastSubmitButton_clicked()
  {
    undoLastSubmitButton.hide();
    restore_previous_state();
    write_examples_out();
    labelDataDisplay.reset_display_highlight();
    labelDataDisplay.reinit_region_label();
    labelDataDisplay.set_button_press_monitor_flag(false);
    selection_list.clear();
    saved_selection_list.clear();

    return;
  }

  void HSegLearn::on_submitExamplesButton_clicked()
  {
    int index, size;

    save_current_state();

    size = amb_selection_list.size();
    for (index = 0; index < size; index++)
    {
      selClassLabel = amb_selection_list[index];
      selClassLabelFlag = true;
//cout << "Submitting ambiguous example region class " << selClassLabel << endl;
      submit_amb_selection();
    }

    size = neg_selection_list.size();
    for (index = 0; index < size; index++)
    {
      selClassLabel = neg_selection_list[index];
      selClassLabelFlag = true;
//cout << "Submitting negative example region class " << selClassLabel << endl;
      submit_neg_selection();
    }

    size = pos_selection_list.size();
    for (index = 0; index < size; index++)
    {
      selClassLabel = pos_selection_list[index];
      selClassLabelFlag = true;
//cout << "Submitting positive example region class " << selClassLabel << endl;
      submit_pos_selection();
    }

    amb_selection_list.clear();
    neg_selection_list.clear();
    pos_selection_list.clear();

    undoLastHighlightedClassesButton.hide();
    clearAllHighlightedClassesButton.hide();

    selectPosExampleButton.hide();
    selectNegExampleButton.hide();
    selectAmbExampleButton.hide();
    submitExamplesButton.hide();
    undoLastSubmitButton.show();
    submitPosExampleButton.hide();
    submitNegExampleButton.hide();
    submitAmbExampleButton.hide();

    return;
  }

  void HSegLearn::on_submitPosExampleButton_clicked()
  {
    int index, size;

    save_current_state();

    size = selection_list.size();
//    cout << "Entering on_submitPosExampleButton_clicked, selection_list.size = " << size << endl;
    if (size > 0)
    {
      for (index = 0; index < size; index++)
      {
        selClassLabel = selection_list[index];
        selClassLabelFlag = true;
//cout << "Submitting positive example region class " << selClassLabel << endl;
        submit_pos_selection();
      }
    }
    else
    {
      Glib::ustring strMessage = "You must highlight a region class\n"
                                 "before submitting a positive example.";
      Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
      dialog.run();
      return;
    }
    labelDataDisplay.set_button_press_monitor_flag(false);

    selection_list.clear();
    saved_selection_list.clear();

    selClassLabelFlag = false;
    selROIFlag = false;
    
    undoLastHighlightedClassesButton.hide();
    clearAllHighlightedClassesButton.hide();

    selectPosExampleButton.hide();
    selectNegExampleButton.hide();
    selectAmbExampleButton.hide();
    submitExamplesButton.hide();
    undoLastSubmitButton.show();
    submitPosExampleButton.hide();
    submitNegExampleButton.hide();
    submitAmbExampleButton.hide();
    
    return;
  }

  void HSegLearn::on_submitNegExampleButton_clicked()
  {
    int index, size;

    save_current_state();

    size = selection_list.size();
//    cout << "Entering on_submitNegExampleButton_clicked, selection_list.size = " << size << endl;
    if (size > 0)
    {
      for (index = 0; index < size; index++)
      {
        selClassLabel = selection_list[index];
        selClassLabelFlag = true;
//cout << "Submitting negative example region class " << selClassLabel << endl;
        submit_neg_selection();
      }
    }
    else
    {
      Glib::ustring strMessage = "You must highlight a region class\n"
                                 "before submitting a negative example.";
      Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
      dialog.run();
      return;
    }
    labelDataDisplay.set_button_press_monitor_flag(false);

    selection_list.clear();
    saved_selection_list.clear();

    selClassLabelFlag = false;
    selROIFlag = false;

    undoLastHighlightedClassesButton.hide();
    clearAllHighlightedClassesButton.hide();

    selectPosExampleButton.hide();
    selectNegExampleButton.hide();
    selectAmbExampleButton.hide();
    submitExamplesButton.hide();
    undoLastSubmitButton.show();
    submitPosExampleButton.hide();
    submitNegExampleButton.hide();
    submitAmbExampleButton.hide();

    return;
  }

  void HSegLearn::on_submitAmbExampleButton_clicked()
  {
    int index, size;

    save_current_state();

    size = selection_list.size();
//    cout << "Entering on_submitAmbExampleButton_clicked, selection_list.size = " << size << endl;
    if (size > 0)
    {
      for (index = 0; index < size; index++)
      {
        selClassLabel = selection_list[index];
        selClassLabelFlag = true;
//cout << "Submitting ambiguous example region class " << selClassLabel << endl;
        submit_amb_selection();
      }
    }
    else
    {
      Glib::ustring strMessage = "You must highlight a region class\n"
                                 "before submitting an ambigous example.";
      Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
      dialog.run();
      return;
    }
    labelDataDisplay.set_button_press_monitor_flag(false);

    selection_list.clear();
    saved_selection_list.clear();

    selClassLabelFlag = false;
    selROIFlag = false;

    undoLastHighlightedClassesButton.hide();
    clearAllHighlightedClassesButton.hide();

    selectPosExampleButton.hide();
    selectNegExampleButton.hide();
    selectAmbExampleButton.hide();
    submitExamplesButton.hide();
    undoLastSubmitButton.show();
    submitPosExampleButton.hide();
    submitNegExampleButton.hide();
    submitAmbExampleButton.hide();

    return;
  }

  void HSegLearn::on_labelDataDisplay_circleROI_event()
  {
    selClassLabelFlag = false;
    selROIFlag = true;

//cout << "Detected labelDataDisplay_circleROI_event" << endl;
    int index, size;

    size = selection_list.size();
    saved_selection_list.clear();
    for (index = 0; index < size; index++)
      saved_selection_list.push_back(selection_list[index]);

    if (selROIFlag)
    { 
      build_selection_list();
    }
    else
    {
      Glib::ustring strMessage = "You must circle a region of interest\n"
                                 "before you an highlight a circled]n"
                                 "classes.";
      Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
    
      dialog.run();
      return;
    }

    size = selection_list.size();
    if (size > 0)
    {
      selClassLabel = selection_list[0];
cout << "Highlighting region class " << selClassLabel << endl;
      labelDataDisplay.set_display_highlight(selClassLabel,classLabelsMapImage,true);
    }
    for (index = 1; index < size; index++)
    {
      selClassLabel = selection_list[index];
cout << "Highlighting region class " << selClassLabel << endl;
      labelDataDisplay.set_display_highlight(selClassLabel,classLabelsMapImage,false);
    }
    labelDataDisplay.reinit_region_label();
    rgbImageDisplay.clear_path();
    if (initialParams.panchromatic_image_flag)
      panImageDisplay.clear_path();
    if (initialParams.reference_image_flag)
      referenceImageDisplay.clear_path();

    undoLastHighlightedClassesButton.show();
    clearAllHighlightedClassesButton.show();

    selectPosExampleButton.show();
    selectNegExampleButton.show();
#ifdef AMBIGUOUS
    selectAmbExampleButton.show();
#endif
    if ((amb_selection_list.size() == 0) && (neg_selection_list.size() == 0) && (pos_selection_list.size() == 0))
    {
      undoLastSubmitButton.hide();
      submitPosExampleButton.show();
      submitNegExampleButton.show();
#ifdef AMBIGUOUS
      submitAmbExampleButton.show();
#endif
    }
//    cout << "Exiting on_highlightCircledClassesButton_clicked, selection_list.size = " << size << endl;
    return;
  }

  void HSegLearn::on_rgbImageDisplay_buttonPress_event()
  {

//cout << "Detected rgbImageDisplay_buttonPress_event" << endl;
    buttonPress_event();

    return;
  }

  void HSegLearn::on_labelDataDisplay_buttonPress_event()
  {

//cout << "Detected labelDataDisplay_buttonPress_event" << endl;
    buttonPress_event();

    return;
  }

  void HSegLearn::on_panImageDisplay_buttonPress_event()
  {

//cout << "Detected panImageDisplay_buttonPress_event" << endl;
    buttonPress_event();

    return;
  }

  void HSegLearn::on_referenceImageDisplay_buttonPress_event()
  {

//cout << "Detected referenceImageDisplay_buttonPress_event" << endl;
    buttonPress_event();

    return;
  }

  void HSegLearn::buttonPress_event()
  {
    int selCol, selRow, size, index;

    selClickFlag = rgbImageDisplay.get_click_location(selColClick, selRowClick);
    
    if (selClickFlag)
    {
      selCol = (int) ((selColClick/rgbImageDisplay.get_X_zoom_factor()) + 0.5);
      selRow = (int) ((selRowClick/rgbImageDisplay.get_Y_zoom_factor()) + 0.5);

      selClassLabel = (unsigned int) classLabelsMapImage.get_data(selCol,selRow,0);

      if (selClassLabel > 0)
      {
        selClassLabelFlag = true;
        if (selROIFlag)
        {
          rgbImageDisplay.clear_path();
          if (initialParams.panchromatic_image_flag)
            panImageDisplay.clear_path();
          if (initialParams.reference_image_flag)
            referenceImageDisplay.clear_path();
          selROIFlag = false;
        }
        size = selection_list.size();
        saved_selection_list.clear();
        for (index = 0; index < size; index++)
          saved_selection_list.push_back(selection_list[index]);
cout << "Highlighting region class " << selClassLabel << endl;
        selection_list.push_back(selClassLabel);
        size = selection_list.size();
        if (size == 1)
          labelDataDisplay.set_display_highlight(selClassLabel,classLabelsMapImage,true);
        else
          labelDataDisplay.set_display_highlight(selClassLabel,classLabelsMapImage,false);
        labelDataDisplay.reinit_region_label();
      }
      else
      {
        selClassLabelFlag = false;
        Glib::ustring strMessage = "Clicked on masked out image pixel.\n"
                                   "No Region Class selected.";
        Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
    
        dialog.run();
        return;
      }
    }
    else
    {
      Glib::ustring strMessage = "You must perform a left mouse button\n"
                                 "click on an image pixel before you\n"
                                 "can highlight a region class.";
      Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
    
      dialog.run();
      return;
    }

    undoLastHighlightedClassesButton.show();
    clearAllHighlightedClassesButton.show();

    selectPosExampleButton.show();
    selectNegExampleButton.show();
#ifdef AMBIGUOUS
    selectAmbExampleButton.show();
#endif
    if ((amb_selection_list.size() == 0) && (neg_selection_list.size() == 0) && (pos_selection_list.size() == 0))
    {
      undoLastSubmitButton.hide();
      submitPosExampleButton.show();
      submitNegExampleButton.show();
#ifdef AMBIGUOUS
      submitAmbExampleButton.show();
#endif
    }
//    cout << "Exiting on_highlightRegionClassButton_clicked, selection_list.size = " << selection_list.size() << endl;
    return;
  }

#ifdef THREEDIM
  bool HSegLearn::subsetfrom3d() // Creates the 2D subset of Image objects from the input 3D Image objects
  {
    int col, row, slice, band, view_col, view_row;
    int min_ncols, max_ncols, min_nrows, max_nrows, min_nslices, max_nslices;

    min_ncols = initialParams.ncols_offset;
    max_ncols = initialParams.ncols_offset + initialParams.ncols_subset;
    min_nrows = initialParams.nrows_offset;
    max_nrows = initialParams.nrows_offset + initialParams.nrows_subset;
    min_nslices = initialParams.nslices_offset;
    max_nslices = initialParams.nslices_offset + initialParams.nslices_subset;

    Image input3dImage;
    input3dImage.open(params.input_image_file, params.ncols, params.nrows, params.nslices, params.nbands, params.dtype);
    inputImage.create(initialParams.view_input_image_file, view_ncols, view_nrows, params.nbands, input3dImage.get_data_type(), "GTiff");
    
    for (band = 0; band < params.nbands; band++)
      for (slice = min_nslices; slice < max_nslices; slice++)
        for (row = min_nrows; row < max_nrows; row++)
          for (col = min_ncols; col < max_ncols; col++)
          {
            switch(initialParams.view_dimension)
            {
              case COLUMN: view_col = row;
                           view_row = slice;
                           break;
              case    ROW: view_col = col;
                           view_row = slice;
                           break;
              case  SLICE: view_col = col;
                           view_row = row;
                           break;
              default:     view_col = view_row = 0; // Should never happen!
            }
            inputImage.put_data(input3dImage.get_data(col,row,slice,band),view_col,view_row,band);
          }
    inputImage.flush_data();
    input3dImage.close();
    inputImage.close();
    inputImage.read_info(initialParams.view_input_image_file);

    if (params.mask_flag)
    {
      Image mask3dImage;
      mask3dImage.open(params.mask_file, params.ncols, params.nrows, params.nslices, 1, UInt8);
      maskImage.create(initialParams.view_mask_image_file, view_ncols, view_nrows, 1, GDT_Byte, "GTiff");
      for (slice = min_nslices; slice < max_nslices; slice++)
        for (row = min_nrows; row < max_nrows; row++)
          for (col = min_ncols; col < max_ncols; col++)
          {
            switch(initialParams.view_dimension)
            {
              case COLUMN: view_col = row;
                           view_row = slice;
                           break;
              case    ROW: view_col = col;
                           view_row = slice;
                           break;
              case  SLICE: view_col = col;
                           view_row = row;
                           break;
              default:     view_col = view_row = 0; // Should never happen!
            }
            maskImage.put_data(mask3dImage.get_data(col,row,slice,0),view_col,view_row,0);
          }
      maskImage.flush_data();
      mask3dImage.close();
      maskImage.close();
      maskImage.read_info(initialParams.view_mask_image_file);
    }

    Image classLabelsMap3dImage;
    classLabelsMap3dImage.open(params.class_labels_map_file, params.ncols, params.nrows, params.nslices, 1, UInt32);
    classLabelsMapImage.create(initialParams.view_classes_map_file, view_ncols, view_nrows, 1, GDT_UInt32, "GTiff");
    for (slice = min_nslices; slice < max_nslices; slice++)
      for (row = min_nrows; row < max_nrows; row++)
        for (col = min_ncols; col < max_ncols; col++)
        {
          switch(initialParams.view_dimension)
          {
            case COLUMN: view_col = row;
                         view_row = slice;
                         break;
            case    ROW: view_col = col;
                         view_row = slice;
                         break;
            case  SLICE: view_col = col;
                         view_row = row;
                         break;
            default:     view_col = view_row = 0; // Should never happen!
          }
          classLabelsMapImage.put_data(classLabelsMap3dImage.get_data(col,row,slice,0),view_col,view_row,0);
        }
    classLabelsMapImage.flush_data();
    classLabelsMap3dImage.close();
    classLabelsMapImage.close();

    return true;
  }
#endif

  bool HSegLearn::initializeImages() // Initializes the Image Class variables
  {
    double X_offset, Y_offset, X_gsd, Y_gsd; //  X and Y offset and ground sampling distance
    int view_col, view_row;

    X_offset = Y_offset = 0.0;
    X_gsd = Y_gsd = 1.0;

  // Initialize inputImage
    if (inputImage.gdal_valid())
    {
      if (inputImage.geotransform_valid())
      {
        X_offset = inputImage.get_X_offset();
        Y_offset = inputImage.get_Y_offset();
        X_gsd = inputImage.get_X_gsd();
        Y_gsd = inputImage.get_Y_gsd();
      }
    }
    else
    {
      if (!inputImage.data_valid())
        inputImage.open(params.input_image_file, params.ncols, params.nrows, params.nbands, params.dtype);
    }
    inputImage.set_rgb_display_bands(initialParams.red_display_band,initialParams.green_display_band,
                                     initialParams.blue_display_band);
    inputImage.computeHistoEqMap(inputImage.get_red_display_band(),256);
    if (inputImage.get_green_display_band() != inputImage.get_red_display_band())
      inputImage.computeHistoEqMap(inputImage.get_green_display_band(),256);
    if ((inputImage.get_blue_display_band() != inputImage.get_red_display_band()) &&
        (inputImage.get_blue_display_band() != inputImage.get_green_display_band()))
      inputImage.computeHistoEqMap(inputImage.get_blue_display_band(),256);
    inputImage.set_rgb_image_stretch(initialParams.rgb_image_stretch,initialParams.range[0],initialParams.range[1]);

    if (params.mask_flag)
    {
    // Initialize maskImage
      if (maskImage.gdal_valid())
      {
      // Geotransform must match that of inputImage
        if (maskImage.geotransform_valid())
        {
          if ((X_offset != maskImage.get_X_offset()) ||
              (Y_offset != maskImage.get_Y_offset()) ||
              (X_gsd != maskImage.get_X_gsd()) ||
              (Y_gsd != maskImage.get_Y_gsd()))
          {
            cout << "ERROR: Geotransform of mask image must match the geotransform of the input image!" << endl;
            return false;
          }
        }
      }
      else
      {
        if (!maskImage.data_valid())
          maskImage.open(params.mask_file, params.ncols, params.nrows, 1, UInt8);
      }
    }

  // Initialize classLabelsMapImage
    if (inputImage.gdal_valid())
    {
#ifdef THREEDIM
      classLabelsMapImage.open(initialParams.view_classes_map_file);
#else
      classLabelsMapImage.open(params.class_labels_map_file);
#endif
      if (!classLabelsMapImage.info_valid())
      {
        cout << "ERROR: " << params.class_labels_map_file << " is not a valid class labels map image" << endl;
        return false;
      }
    }
    else
    {
      classLabelsMapImage.open(params.class_labels_map_file, params.ncols, params.nrows, 1, UInt32);
    }

  // Initialize segLevelClassLabelsMapImage
    segLevelClassLabelsMapImage.create(initialParams.seg_level_classes_map_file, view_ncols, view_nrows, 1, 
                                       classLabelsMapImage.get_data_type(),"GTiff");

    if (initialParams.panchromatic_image_flag)
    {
    // Initialize panchromaticImage
      panchromaticImage.open(initialParams.panchromatic_image_file);
      if (!panchromaticImage.info_valid())
      {
        cout << "ERROR: " << initialParams.panchromatic_image_file << " is not a valid panchromatic image" << endl;
        return false;
      }
      if ((inputImage.gdal_valid()) && (panchromaticImage.gdal_valid()))
      {
        int pan_ncols, pan_nrows;
        double UTM_X, UTM_Y, pan_X_gsd, pan_Y_gsd;
      // Create a subset of the panchromatic image that is not necessarily an even multiple in size relative to the input image
        double UTM_lr_X, UTM_lr_Y;
        pan_X_gsd = panchromaticImage.get_X_gsd();
        pan_Y_gsd = panchromaticImage.get_Y_gsd();
        UTM_lr_X = X_offset + params.ncols*X_gsd;
        UTM_lr_Y = Y_offset + params.nrows*Y_gsd;
        pan_ncols = (UTM_lr_X - X_offset)/pan_X_gsd;
        pan_nrows = (UTM_lr_Y - Y_offset)/pan_Y_gsd;
cout << "Creating panSubsetImage with ncols = " << pan_ncols << " and pan_nrows = " << pan_nrows << endl;
        panSubsetImage.create(initialParams.pan_subset_image_file, pan_ncols, pan_nrows, 1, panchromaticImage.get_data_type(),
                              X_offset, Y_offset, pan_X_gsd, pan_Y_gsd, "GTiff");
        for (view_row = 0; view_row < pan_nrows; view_row++)
        {
          UTM_Y = Y_offset + view_row*pan_Y_gsd;
          for (view_col = 0; view_col < pan_ncols; view_col++)
          {
            UTM_X = X_offset + view_col*pan_X_gsd;
            panSubsetImage.put_data(panchromaticImage.get_data(UTM_X,UTM_Y,0),view_col,view_row,0);
          }
        }
        panSubsetImage.flush_data();
        panSubsetImage.computeHistoEqMap(0,256);
      }
      else
      {
        cout << "ERROR: Cannot utilize panchromatic image without valid geotransform information." << endl;
        return false;
      }
    }

    if (initialParams.reference_image_flag)
    {
    // Initialize referenceImage
      referenceImage.open(initialParams.reference_image_file);
      if (!referenceImage.info_valid())
      {
        cout << "ERROR: " << initialParams.reference_image_file << " is not a valid reference image" << endl;
        return false;
      }
    }

  // Create blank labelDataImage
    labelDataImage.create(initialParams.label_data_file,view_ncols,view_nrows,1,GDT_Byte,"GTiff");
    if (params.mask_flag)
    {
      for (view_row = 0; view_row < view_nrows; view_row++)
        for (view_col = 0; view_col < view_ncols; view_col++)
        {
          if (maskImage.get_data(view_col,view_row,0))
            labelDataImage.put_data(0,view_col,view_row,0);
          else
            labelDataImage.put_data(BAD_DATA_QUALITY_LABEL,view_col,view_row,0);
        }
    }
    else
    {
      for (view_row = 0; view_row < view_nrows; view_row++)
        for (view_col = 0; view_col < view_ncols; view_col++)
          labelDataImage.put_data(0,view_col,view_row,0);
    }
    labelDataImage.flush_data();
    set_label_data_colormap();
    saved_labelDataImage.create(initialParams.saved_label_data_file,view_ncols,view_nrows,1,GDT_Byte,"GTiff");

  // Create and initialize regionClassImage
    regionClassImage.create(initialParams.region_map_file,labelDataImage,1,GDT_UInt32,"GTiff");
    for (view_row = 0; view_row < view_nrows; view_row++)
      for (view_col = 0; view_col < view_ncols; view_col++)
      {
        regionClassImage.put_data(0,view_col,view_row,0);
      }
    regionClassImage.flush_data();
    saved_regionClassImage.create(initialParams.saved_region_map_file,labelDataImage,1,GDT_UInt32,"GTiff");

  // Create labelOutImage as copy of labelDataImage, but with the format of inputImage
    Image labelOutImage;
    labelOutImage.create(initialParams.label_out_file,view_ncols,view_nrows,1,GDT_Byte,inputImage.get_driver_description());
    if (inputImage.geotransform_valid())
      labelOutImage.set_geotransform(inputImage);
    labelOutImage.registered_data_copy(0,labelDataImage,0);
    labelOutImage.copy_colormap(labelDataImage);
    labelOutImage.close();

    return true;
  }

  void HSegLearn::set_label_data_colormap()
  {
    unsigned int index, red_value, green_value, blue_value;
    labelDataImage.resize_colormap(NUMBER_OF_LABELS);
    ofstream ascii_out_fs;

    ascii_out_fs.open(initialParams.ascii_out_file.c_str());
    ascii_out_fs << "\"0: Unlabeled Areas\"" << endl;
    ascii_out_fs << "\"1: Positive Examples\"" << endl;
    ascii_out_fs << "\"2: Negative Examples\"" << endl;
    ascii_out_fs << "\"3: Ambiguous Examples\"" << endl;
    for (index = 4; index < HIGHLIGHT_COLOR; index++)
      ascii_out_fs << "\"" << index << ": \"" << endl;
    ascii_out_fs << "\"" << HIGHLIGHT_COLOR << ": Region to be Labeled\"" << endl;
    ascii_out_fs << "\"" << BAD_DATA_QUALITY_LABEL << ": Masked out in input data\"" << endl;

    unsigned char colormap_values[] = {0,127,191,255};
    int red_index, green_index, blue_index;
    index = 0;
    for (red_index = 0; red_index < 4; red_index++)
      for (green_index = 0; green_index < 4; green_index++)
        for (blue_index = 0; blue_index < 4; blue_index++)
        {
          red_value = colormap_values[red_index];
          green_value = colormap_values[green_index];
          blue_value = colormap_values[blue_index];
          if (red_value > 0)
            red_value = ((red_value+1)*256) - 1;
          else
            red_value = 0;
          if (blue_value > 0)
            blue_value = ((blue_value+1)*256) - 1;
          else
            blue_value = 0;
          if (green_value > 0)
            green_value = ((green_value+1)*256) - 1;
          else
            green_value = 0;
          color[index].set_rgb(red_value,green_value,blue_value);
          index++;
        }
    index = 21;
    red_value = 64; green_value = 127; blue_value = 191;
    red_value = ((red_value+1)*256) - 1;
    blue_value = ((blue_value+1)*256) - 1;
    green_value = ((green_value+1)*256) - 1;
    color[index].set_rgb(red_value,green_value,blue_value);
    index = 63;
    red_value = 127; green_value = 63; blue_value = 191;
    red_value = ((red_value+1)*256) - 1;
    blue_value = ((blue_value+1)*256) - 1;
    green_value = ((green_value+1)*256) - 1;
    color[index].set_rgb(red_value,green_value,blue_value);
    index = 64;
    red_value = 63; green_value = 191; blue_value = 127;
    red_value = ((red_value+1)*256) - 1;
    blue_value = ((blue_value+1)*256) - 1;
    green_value = ((green_value+1)*256) - 1;
    color[index].set_rgb(red_value,green_value,blue_value);
    index = 65;
    red_value = 127; green_value = 191; blue_value = 63;
    red_value = ((red_value+1)*256) - 1;
    blue_value = ((blue_value+1)*256) - 1;
    green_value = ((green_value+1)*256) - 1;
    color[index].set_rgb(red_value,green_value,blue_value);
    index = 66;
    red_value = 191; green_value = 63; blue_value = 127;
    red_value = ((red_value+1)*256) - 1;
    blue_value = ((blue_value+1)*256) - 1;
    green_value = ((green_value+1)*256) - 1;
    color[index].set_rgb(red_value,green_value,blue_value);
    index = HIGHLIGHT_COLOR;
    red_value = 255; green_value = 255; blue_value = 255;
    red_value = ((red_value+1)*256) - 1;
    blue_value = ((blue_value+1)*256) - 1;
    green_value = ((green_value+1)*256) - 1;
    color[index].set_rgb(red_value,green_value,blue_value);
    index = BAD_DATA_QUALITY_LABEL;
    red_value = 195; green_value = 195; blue_value = 195;
    red_value = ((red_value+1)*256) - 1;
    blue_value = ((blue_value+1)*256) - 1;
    green_value = ((green_value+1)*256) - 1;
    color[index].set_rgb(red_value,green_value,blue_value);

    index = 0; // Unlabeled areas (Black)
    red_value = 0; green_value = 0; blue_value = 0;
    labelDataImage.set_colormap_value(index,red_value,green_value,blue_value);
    ascii_out_fs << red_value << " " << green_value << " " << blue_value << endl;
    index = 1;   // Positive Learned Class (Green)
    red_value = 0; green_value = 255; blue_value = 0;
    labelDataImage.set_colormap_value(index,red_value,green_value,blue_value);
    ascii_out_fs << red_value << " " << green_value << " " << blue_value << endl;
    index = 2;   // Negative Learned Class (Red)
    red_value = 255; green_value = 0; blue_value = 0;
    labelDataImage.set_colormap_value(index,red_value,green_value,blue_value);
    ascii_out_fs << red_value << " " << green_value << " " << blue_value << endl;
    index = 3;   // Ambiguous Learned Class (Tourquoise)
    red_value = 127; green_value = 127; blue_value = 255;
    labelDataImage.set_colormap_value(index,red_value,green_value,blue_value);
    ascii_out_fs << red_value << " " << green_value << " " << blue_value << endl;
    index = 4;   // Selected Region Class(es) (Light Yellow)
    red_value = 255; green_value = 255; blue_value = 127;
    labelDataImage.set_colormap_value(index,red_value,green_value,blue_value);
    index = HIGHLIGHT_COLOR;   // Highlighted Region Class(es) (White)
    red_value = 255; green_value = 255; blue_value = 255;
    labelDataImage.set_colormap_value(index,red_value,green_value,blue_value);
    index = BAD_DATA_QUALITY_LABEL;   // Masked out - Bad Data (Grey)
    red_value = 192; green_value = 192; blue_value = 192;
    labelDataImage.set_colormap_value(index,red_value,green_value,blue_value);

    for (index = 4; index < NUMBER_OF_LABELS; index++)
    {
      red_value = color[index].get_red();
      green_value = color[index].get_green();
      blue_value = color[index].get_blue();
      if (red_value > 0)
        red_value = ((red_value+1)/256) - 1;
      if (green_value > 0)
        green_value = ((green_value+1)/256) - 1;
      if (blue_value > 0)
        blue_value = ((blue_value+1)/256) - 1;
      ascii_out_fs << red_value << " " << green_value << " " << blue_value << endl;
    }
    ascii_out_fs.close();


    return;
  }

  void HSegLearn::set_segLevelClassLabelsMapImage()
  {
    int  view_row, view_col;
    unsigned int region_label, region_index;

  // Set the region classes label map for the current hierarchial segmentation level
    for (view_row = 0; view_row < view_nrows; view_row++)
      for (view_col = 0; view_col < view_ncols; view_col++)
      {
        region_label = (unsigned int) classLabelsMapImage.get_data(view_col,view_row,0);
        if (region_label > 0)
        {
          region_index = region_label - 1;
          if (!region_classes[region_index].get_active_flag())
            region_label = region_classes[region_index].get_merge_region_label();
        }
        segLevelClassLabelsMapImage.put_data(region_label,view_col,view_row,0);
      }
    segLevelClassLabelsMapImage.flush_data();

    return;
  }

  void HSegLearn::read_examples_in()
  {
    unsigned int index, size;
    ifstream examples_in_fs;

    examples_in_fs.open(initialParams.examples_in_file.c_str());
    examples_in_fs >> size;
    for (index = 0; index < size; index++)
    {
      examples_in_fs >> selClassLabel;
      pos_example_list.push_back(selClassLabel);
    }
    examples_in_fs >> size;
    for (index = 0; index < size; index++)
    {
      examples_in_fs >> selClassLabel;
      if (find(pos_example_list.begin(),pos_example_list.end(),selClassLabel) != pos_example_list.end())
      {
        pos_example_list.erase(find(pos_example_list.begin(),pos_example_list.end(),selClassLabel));
        amb_example_list.push_back(selClassLabel);
      }
      else
        neg_example_list.push_back(selClassLabel);
    }
    examples_in_fs >> size;
    for (index = 0; index < size; index++)
    {
      examples_in_fs >> selClassLabel;
      if (find(pos_example_list.begin(),pos_example_list.end(),selClassLabel) != pos_example_list.end())
      {
        pos_example_list.erase(find(pos_example_list.begin(),pos_example_list.end(),selClassLabel));
      }
      if (find(neg_example_list.begin(),neg_example_list.end(),selClassLabel) != neg_example_list.end())
      {
        neg_example_list.erase(find(neg_example_list.begin(),neg_example_list.end(),selClassLabel));
      }
      amb_example_list.push_back(selClassLabel);
    }
    examples_in_fs.close();

    return;
  }

  void HSegLearn::write_examples_out()
  {
    unsigned int index, size;
    ofstream examples_out_fs;
    examples_out_fs.open(initialParams.examples_out_file.c_str());
    size = pos_example_list.size();
    examples_out_fs << size << endl;
    for (index = 0; index < size; index++)
      examples_out_fs << pos_example_list[index] << endl;
    size = neg_example_list.size();
    examples_out_fs << size << endl;
    for (index = 0; index < size; index++)
      examples_out_fs << neg_example_list[index] << endl;
    size = amb_example_list.size();
    examples_out_fs << size << endl;
    for (index = 0; index < size; index++)
      examples_out_fs << amb_example_list[index] << endl;
    examples_out_fs.close();

    return;
  }

  void HSegLearn::submit_pos_examples()
  {
    unsigned int index, size, region_label;

    pos_selection_list.clear();
    size = pos_example_list.size();
    for (index = 0; index < size; index++)
    {
      selClassLabel = pos_example_list[index];
      pos_selection_list.push_back(selClassLabel);
    }

    size = pos_selection_list.size();
    pos_example_list.clear();
    for (index = 0; index < size; index++)
    {
      selClassLabel = pos_selection_list[index];
      region_label = check_conflict();
      if (region_label == 0)
      {
cout << "Class " << selClassLabel << " submitted as a positive example." << endl;
        pos_example_list.push_back(selClassLabel);
        submit_pos_example();
      }
      else
      {
cout << "Class " << selClassLabel << " covered by previously submitted positive example class " << region_label;
cout << ". This class not submitted." << endl;
      }
    }
    pos_selection_list.clear();

    return;
  }

  void HSegLearn::submit_neg_examples()
  {
    unsigned int index, size;

    size = neg_example_list.size();
    for (index = 0; index < size; index++)
    {
      selClassLabel = neg_example_list[index];
cout << "Class " << selClassLabel << " submitted as a negative example." << endl;
      submit_neg_example();
    }

    return;
  }

  void HSegLearn::submit_amb_examples()
  {
    unsigned int index, size;

    size = amb_example_list.size();
    for (index = 0; index < size; index++)
    {
      selClassLabel = amb_example_list[index];
cout << "Class " << selClassLabel << " submitted as an ambiguous example." << endl;
      submit_amb_example();
    }

    return;
  }

  void HSegLearn::submit_pos_example()
  {
    unsigned int region_label;

    region_label = check_conflict();

    labelDataDisplay.set_display_highlight(selClassLabel,classLabelsMapImage);

    if (region_label == 0)
    {
      find_coarsest_level();
      segLevel = INIT_SEG_LEVEL;
      results_data.open_input(params.region_classes_file,params.region_objects_file);
      results_data.read(segLevel,nb_classes,nb_objects,oparams.nb_levels,oparams.int_buffer_size,region_classes,region_objects);
      results_data.close_input();
    }
    else
    {
      if (find(pos_example_list.begin(),pos_example_list.end(),selClassLabel) != pos_example_list.end())
      {
        pos_example_list.erase(find(pos_example_list.begin(),pos_example_list.end(),selClassLabel));
      }
cout << "Class " << selClassLabel << " covered by previously submitted positive example class " << region_label;
cout << ". This class not submitted." << endl;
//      Glib::ustring strMessage = "Class " + stringify_int(selClassLabel) + " covered by previously submitted\n"
//                                 "class " + stringify_int(region_label) + ". This class not submitted.\n";
//      Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
//      dialog.run();
    }
    highlight_label = 1;
    update_region_label();
    labelDataDisplay.reset_display_highlight();

    return;
  }

  void HSegLearn::submit_neg_example()
  {
    labelDataDisplay.set_display_highlight(selClassLabel,classLabelsMapImage);
    highlight_label = 2;
    update_region_label();
    labelDataDisplay.reset_display_highlight();

    return;
  }

  void HSegLearn::submit_amb_example()
  {
    labelDataDisplay.set_display_highlight(selClassLabel,classLabelsMapImage);
    highlight_label = 3;
    update_region_label();
    labelDataDisplay.reset_display_highlight();

    return;
  }

  void HSegLearn::submit_pos_selection()
  {
    if (selClassLabelFlag)
    {
      highlight_label = 4;
      if (find(pos_example_list.begin(),pos_example_list.end(),selClassLabel) != pos_example_list.end())
      {
cout << "Class " << selClassLabel << " previously submitted as a positive example. No change." << endl;
//        Glib::ustring strMessage = "Class "+ stringify_int(selClassLabel) +" previously submitted\n"
//                                   "as a positive example. No change.\n";
//        Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
//        dialog.run();
        highlight_label = 1;
        conflict_label = 1;
      }
      if (find(neg_example_list.begin(),neg_example_list.end(),selClassLabel) != neg_example_list.end())
      {
cout << "Class " << selClassLabel << " previously submitted as a negative example.";
cout << " This class will now be labeled as an ambiguous example." << endl;;
//        Glib::ustring strMessage = "Class "+ stringify_int(selClassLabel) +" previously submitted\n"
//                                   "as a negative example. This class will\n"
//                                   "now be labeled as an ambiguous example.\n";
//        Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
//        dialog.run();
        highlight_label = 3;
        conflict_label = 2;
      }
      if (find(amb_example_list.begin(),amb_example_list.end(),selClassLabel) != amb_example_list.end())
      {
cout << "Class " << selClassLabel << " previously considered as an ambiguous example.";
cout << " This class will now be considered as a positive example." << endl;
//        Glib::ustring strMessage = "Class "+ stringify_int(selClassLabel) +" previously considered\n"
//                                   "as an ambiguous example. This class will\n"
//                                   "now be considered as a positive example.\n";
//        Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
//        dialog.run();
        highlight_label = 1;
        conflict_label = 3;
      }
      if (highlight_label == 4)
      {
cout << "Class " << selClassLabel << " submitted as a positive example." << endl;
//        Glib::ustring strMessage = "Class "+ stringify_int(selClassLabel) +" submitted\n"
//                                   "as a positive example.\n";
//        Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
//        dialog.run();
        highlight_label = 1;
        conflict_label = 0;
      }
      submit_example();
    }
    else
    {
      Glib::ustring strMessage = "You must select a region class\n"
                                 "or circle a region of interest\n"
                                 "before submitting a positive example.";
      Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
      dialog.run();
    }

    selClassLabelFlag = false;
    selROIFlag = false;
    
    return;
  }

  void HSegLearn::submit_neg_selection()
  {
    if (selClassLabelFlag)
    {
      highlight_label = 4;
      if (find(pos_example_list.begin(),pos_example_list.end(),selClassLabel) != pos_example_list.end())
      {
cout << "Class " << selClassLabel << " previously submitted as a positive example.";
cout << " This class will now be labeled as an ambiguous example." << endl;
//        Glib::ustring strMessage = "Class "+ stringify_int(selClassLabel) +" previously submitted\n"
//                                   "as a positive example. This class will\n"
//                                   "now be labeled as an ambiguous example.\n";
//        Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
//        dialog.run();
        highlight_label = 3;
        conflict_label = 1;
      }
      if (find(neg_example_list.begin(),neg_example_list.end(),selClassLabel) != neg_example_list.end())
      {
cout << "Class " << selClassLabel << " previously submitted as a negative example. No change." << endl;
//        Glib::ustring strMessage = "Class "+ stringify_int(selClassLabel) +" previously submitted\n"
//                                   "as a negative example. No change.\n";
//        Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
//        dialog.run();
        highlight_label = 2;
        conflict_label = 2;
      }
      if (find(amb_example_list.begin(),amb_example_list.end(),selClassLabel) != amb_example_list.end())
      {
cout << "Class " << selClassLabel << " previously considered as an ambiguous example.";
cout << " This class will now be considered as a negative example." << endl;
//        Glib::ustring strMessage = "Class "+ stringify_int(selClassLabel) +" previously considered\n"
//                                   "as an ambiguous example. This class will\n"
//                                   "now be considered as a negative example.\n";
//        Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
//        dialog.run();
        highlight_label = 2;
        conflict_label = 3;
      }
      if (highlight_label == 4)
      {
cout << "Class " << selClassLabel << " submitted as a negative example." << endl;
//        Glib::ustring strMessage = "Class "+ stringify_int(selClassLabel) +" submitted\n"
//                                   "as a negative example.\n";
//        Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
//        dialog.run();
        highlight_label = 2;
        conflict_label = 0;
      }
      submit_example();
    }
    else
    {
      Glib::ustring strMessage = "You must select a region class\n"
                                 "or circle a region of interest\n"
                                 "before submitting a negative example.";
      Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
      dialog.run();
    }

    selClassLabelFlag = false;
    selROIFlag = false;
    
    return;
  }

  void HSegLearn::submit_amb_selection()
  {

    if (selClassLabelFlag)
    {
      highlight_label = 4;
      if (find(pos_example_list.begin(),pos_example_list.end(),selClassLabel) != pos_example_list.end())
      {
cout << "Class " << selClassLabel << " previously submitted as a positive example.";
cout << " This class will now be labeled as an ambiguous example." << endl;
//        Glib::ustring strMessage = "Class "+ stringify_int(selClassLabel) +" previously submitted\n"
//                                   "as a positive example. This class will\n"
//                                   "now be labeled as an ambiguous example.\n";
//        Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
//        dialog.run();
        highlight_label = 3;
        conflict_label = 1;
      }
      if (find(neg_example_list.begin(),neg_example_list.end(),selClassLabel) != neg_example_list.end())
      {
cout << "Class " << selClassLabel << " previously submitted as a negative example.";
cout << " This class will now be labeled as an ambiguous example." << endl;
//        Glib::ustring strMessage = "Class "+ stringify_int(selClassLabel) +" previously submitted\n"
//                                   "as a negative example. This class will\n"
//                                   "now be labeled as an ambiguous example.\n";
//        Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
//        dialog.run();
        highlight_label = 3;
        conflict_label = 2;
      }
      if (find(amb_example_list.begin(),amb_example_list.end(),selClassLabel) != amb_example_list.end())
      {
cout << "Class " << selClassLabel << " previously considered as an ambiguous example. No change." << endl;
//        Glib::ustring strMessage = "Class "+ stringify_int(selClassLabel) +" previously considered\n"
//                                   "as an ambiguous example. No change.\n";
//        Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
//        dialog.run();
        highlight_label = 3;
        conflict_label = 3;
      }
      if (highlight_label == 4)
      {
cout << "Class " << selClassLabel << " submitted as an ambiguous example." << endl;
//        Glib::ustring strMessage = "Class "+ stringify_int(selClassLabel) +" submitted\n"
//                                   "as an ambiguous example.\n";
//        Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
//        dialog.run();
        highlight_label = 3;
        conflict_label = 0;
      }
      submit_example();
    }
    else
    {
      Glib::ustring strMessage = "You must select a region class\n"
                                 "or circle a region of interest\n"
                                 "before submitting a negative example.";
      Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
      dialog.run();
    }

    selClassLabelFlag = false;
    selROIFlag = false;
    
    return;
  }

  void HSegLearn::submit_example()
  {
    unsigned int region_label;

    switch (conflict_label)
    {
      case 1:  if (find(pos_example_list.begin(),pos_example_list.end(),selClassLabel) != pos_example_list.end())
               {
                 pos_example_list.erase(find(pos_example_list.begin(),pos_example_list.end(),selClassLabel));
               }
               clear_region_label(selClassLabel);
               break;
      case 2:  if (find(neg_example_list.begin(),neg_example_list.end(),selClassLabel) != neg_example_list.end())
               {
                 neg_example_list.erase(find(neg_example_list.begin(),neg_example_list.end(),selClassLabel));
               }
               clear_region_label(selClassLabel);
               break;
      case 3:  if (find(amb_example_list.begin(),amb_example_list.end(),selClassLabel) != amb_example_list.end())
               {
                 amb_example_list.erase(find(amb_example_list.begin(),amb_example_list.end(),selClassLabel));
               }
               clear_region_label(selClassLabel);
               break;
      default: break;
    }

    switch (highlight_label)
    {
      case 1:  pos_example_list.push_back(selClassLabel);
               submit_pos_example();
               break;
      case 2:  neg_example_list.push_back(selClassLabel);
               region_label = check_conflict();
               if (region_label > 0)
               {
cout << "Must readjust positive example class " << region_label << endl;
//                 Glib::ustring strMessage = "Must readjust positive example\n"
//                                            "class "+ stringify_int(region_label);
//                 Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
//                 dialog.run();
                 clear_region_label(region_label);
                 submit_neg_example();
                 selClassLabel = region_label;
                 submit_pos_example();
               }
               else
               {
                 submit_neg_example();
               }
               break;
      case 3:  amb_example_list.push_back(selClassLabel);
               submit_amb_example();
               break;
      default: break;
    }

    if ((highlight_label != 0) || (conflict_label != 0))
      write_examples_out();

    labelDataDisplay.reinit_region_label();

    return;
  }

  void HSegLearn::build_selection_list()
  {
    int row, col;

    for (row = 0; row < view_nrows; row++)
      for (col = 0; col < view_ncols; col++)
      {
        if (labelDataDisplay.get_display_highlight(col,row))
        {
          selClassLabel = (unsigned int) classLabelsMapImage.get_data(col,row,0);
          if (!(find(selection_list.begin(),selection_list.end(),selClassLabel) != selection_list.end()))
          {
            selection_list.push_back(selClassLabel);
            selColClick = col;
            selRowClick = row;
          }
        }
      }

    return;
  }

  bool HSegLearn::find_coarsest_level()
  {
    bool conflict_flag, flag;
    unsigned char data_value;
    int row, col;

    conflict_flag = false;
    for (row = 0; row < view_nrows; row++)
      for (col = 0; col < view_ncols; col++)
      {
        if (labelDataDisplay.get_display_highlight(col,row))
        {
          data_value = (unsigned char) labelDataImage.get_data(col,row,0);
          if ((data_value > 0) && (data_value < 4))
            conflict_flag = true;
        }
      }

    if (conflict_flag)
    {
      cout << "Selected region class was previously selected as a positive or negative example." << endl;
      return false;
    }
    else
    {
      flag = true;
      while (flag)
      {
        if (selectCoarserSeg())
        {
          conflict_flag = false;
          for (row = 0; row < view_nrows; row++)
          {
            for (col = 0; col < view_ncols; col++)
            {
              if (labelDataDisplay.get_display_highlight(col,row))
              {
                data_value = (unsigned char) labelDataImage.get_data(col,row,0);
                if ((data_value > 0) && (data_value < 4))
                  conflict_flag = true;
              }
              if (conflict_flag)
                break;
            }
            if (conflict_flag)
              break;
          }
          if (conflict_flag)
          {
            selectFinerSeg();
            flag = false;
          }
        }
        else
        {
          flag = false;
        }
      }
    }
    return true;
  }

  bool HSegLearn::selectFinerSeg()
  {
    bool level_changed = false;
    unsigned int region_index, init_npix, new_npix, classLabel;

    if (segLevel > 0)
    {
      if (selClassLabelFlag)
      {
        classLabel = selClassLabel;
        region_index = classLabel - 1;
        if (!region_classes[region_index].get_active_flag())
          classLabel = region_classes[region_index].get_merge_region_label();
        region_index = classLabel - 1;
        init_npix = region_classes[region_index].get_npix();
        new_npix = init_npix;
        while ((new_npix == init_npix) && (segLevel > 0))
        {
          segLevel--;
          results_data.open_input(params.region_classes_file,params.region_objects_file);
          results_data.read(segLevel,nb_classes,nb_objects,oparams.nb_levels,oparams.int_buffer_size,region_classes,region_objects);
          results_data.close_input();
          classLabel = selClassLabel;
          region_index = classLabel - 1;
          if (!region_classes[region_index].get_active_flag())
            classLabel = region_classes[region_index].get_merge_region_label();
          region_index = classLabel - 1;
          new_npix = region_classes[region_index].get_npix();
        }
        if (new_npix != init_npix)
          level_changed = true;
      }
      else
      {
        segLevel--;
        level_changed = true;
      }

      if (level_changed)
      {
        results_data.open_input(params.region_classes_file,params.region_objects_file);
        results_data.read(segLevel,nb_classes,nb_objects,oparams.nb_levels,oparams.int_buffer_size,region_classes,region_objects);
        results_data.close_input();
        classLabel = selClassLabel;
        region_index = classLabel - 1;
        if (!region_classes[region_index].get_active_flag())
          classLabel = region_classes[region_index].get_merge_region_label();
        set_segLevelClassLabelsMapImage();
        labelDataDisplay.set_display_highlight(classLabel,segLevelClassLabelsMapImage);
      }
      else
        return false;
    }
    else
      return false;

    return true;
  }

  bool HSegLearn::selectCoarserSeg()
  {
    bool level_changed = false;
    unsigned int region_index, init_npix, new_npix, classLabel;

    if (segLevel < (oparams.nb_levels - 1))
    {
      if (selClassLabelFlag)
      {
        classLabel = selClassLabel;
        region_index = classLabel - 1;
        if (!region_classes[region_index].get_active_flag())
          classLabel = region_classes[region_index].get_merge_region_label();
        region_index = classLabel - 1;
        init_npix = region_classes[region_index].get_npix();
        new_npix = init_npix;
        while ((new_npix == init_npix) && (segLevel < (oparams.nb_levels - 1)))
        {
          segLevel++;
          results_data.open_input(params.region_classes_file,params.region_objects_file);
          results_data.read(segLevel,nb_classes,nb_objects,oparams.nb_levels,oparams.int_buffer_size,region_classes,region_objects);
          results_data.close_input();
          classLabel = selClassLabel;
          region_index = classLabel - 1;
          if (!region_classes[region_index].get_active_flag())
            classLabel = region_classes[region_index].get_merge_region_label();
          region_index = classLabel - 1;
          new_npix = region_classes[region_index].get_npix();
        }
        if (new_npix != init_npix)
          level_changed = true;
      }
      else
      {
        segLevel++;
        level_changed = true;
      }

      if (level_changed)
      {
        results_data.open_input(params.region_classes_file,params.region_objects_file);
        results_data.read(segLevel,nb_classes,nb_objects,oparams.nb_levels,oparams.int_buffer_size,region_classes,region_objects);
        results_data.close_input();
        classLabel = selClassLabel;
        region_index = classLabel - 1;
        if (!region_classes[region_index].get_active_flag())
          classLabel = region_classes[region_index].get_merge_region_label();
        set_segLevelClassLabelsMapImage();
        labelDataDisplay.set_display_highlight(classLabel,segLevelClassLabelsMapImage);
      }
      else
        return false;
    }
    else
      return false;

    return true;
  }

  void HSegLearn::update_selection_label()
  {
    int row, col;

    for (row = 0; row < view_nrows; row++)
      for (col = 0; col < view_ncols; col++)
      {
        if (labelDataDisplay.get_display_highlight(col,row))
        {
          labelDataImage.put_data(highlight_label,col,row,0);
        }
      }
    labelDataImage.flush_data();

    return;
  }

  void HSegLearn::update_region_label()
  {
    int row, col;

    for (row = 0; row < view_nrows; row++)
      for (col = 0; col < view_ncols; col++)
      {
        if (labelDataDisplay.get_display_highlight(col,row))
        {
          labelDataImage.put_data(highlight_label,col,row,0);
          regionClassImage.put_data(selClassLabel,col,row,0);
        }
      }
    labelDataImage.flush_data();
    regionClassImage.flush_data();

    return;
  }

  void HSegLearn::clear_region_label(unsigned int& region_label)
  {
    int row, col;

    for (row = 0; row < view_nrows; row++)
      for (col = 0; col < view_ncols; col++)
      {
        if (region_label == (unsigned int) regionClassImage.get_data(col,row,0))
        {
          labelDataImage.put_data(0,col,row,0);
          regionClassImage.put_data(0,col,row,0);
        }
      }
    labelDataImage.flush_data();
    regionClassImage.flush_data();

    return;
  }

  unsigned int HSegLearn::check_conflict()
  {
    int row, col;
    unsigned int region_label = 0;

    for (row = 0; row < view_nrows; row++)
    {
      region_label = 0;
      for (col = 0; col < view_ncols; col++)
      {
        region_label = 0;
        if (selClassLabel == (unsigned int) classLabelsMapImage.get_data(col,row,0))
          region_label = (unsigned int) regionClassImage.get_data(col,row,0);
        if (region_label != 0)
          break;
      }
      if (region_label != 0)
        break;
    }

    return region_label;
  }

  void HSegLearn::save_current_state()
  {
    unsigned int index, size;
    int view_col, view_row;

    size = pos_example_list.size();
    saved_pos_example_list.clear();
    for (index = 0; index < size; index++)
      saved_pos_example_list.push_back(pos_example_list[index]);
    size = neg_example_list.size();
    saved_neg_example_list.clear();
    for (index = 0; index < size; index++)
      saved_neg_example_list.push_back(neg_example_list[index]);
    size = amb_example_list.size();
    saved_amb_example_list.clear();
    for (index = 0; index < size; index++)
      saved_amb_example_list.push_back(amb_example_list[index]);

    for (view_row = 0; view_row < view_nrows; view_row++)
      for (view_col = 0; view_col < view_ncols; view_col++)
      {
        saved_labelDataImage.put_data(labelDataImage.get_data(view_col,view_row,0),view_col,view_row,0);
        saved_regionClassImage.put_data(regionClassImage.get_data(view_col,view_row,0),view_col,view_row,0);
      }
    saved_labelDataImage.flush_data();
    saved_regionClassImage.flush_data();

    return;
  }

  void HSegLearn::restore_previous_state()
  {
    unsigned int index, size;
    int view_col, view_row;

    size = saved_pos_example_list.size();
    pos_example_list.clear();
    for (index = 0; index < size; index++)
      pos_example_list.push_back(saved_pos_example_list[index]);
    size = saved_neg_example_list.size();
    neg_example_list.clear();
    for (index = 0; index < size; index++)
      neg_example_list.push_back(saved_neg_example_list[index]);
    size = saved_amb_example_list.size();
    amb_example_list.clear();
    for (index = 0; index < size; index++)
      amb_example_list.push_back(saved_amb_example_list[index]);

    highlight_label = 4;
    unsigned char value;
    for (view_row = 0; view_row < view_nrows; view_row++)
      for (view_col = 0; view_col < view_ncols; view_col++)
      {
        value = (unsigned char) saved_labelDataImage.get_data(view_col,view_row,0);
        if (value == highlight_label)
          value = 0;
        labelDataImage.put_data(value,view_col,view_row,0);
        regionClassImage.put_data(saved_regionClassImage.get_data(view_col,view_row,0),view_col,view_row,0);
      }
    labelDataImage.flush_data();
    regionClassImage.flush_data();

    return;
  }

} // HSEGTilton
