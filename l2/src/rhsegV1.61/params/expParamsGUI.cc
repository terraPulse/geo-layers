// expParamsGUI.cc

#include "expParamsGUI.h"
#include "params.h"
#include <iostream>
#include <fstream>

extern HSEGTilton::Params params;

namespace HSEGTilton
{
 // Constructor
  ExpParamsGUI::ExpParamsGUI():
               vBox(false,10), lineTable(13,10,true),
               normindRestore("Restore\nDefault"),
               initThresholdRestore("Restore\nDefault"), 
               initThresholdObject("Threshold for initial fast region merging\nby a region oriented first merge process (init_threshold)"),
               initialMergeNpixelsRestore("Restore\nDefault"), 
               initialMergeNpixelsObject("Suppress neighbor merges for regions with more than this number of pixels after the initial fast region merging (initial_merge_flag)"),
               randomInitSeedRestore("Restore\nDefault"), 
               randomInitSeedButton("Utilize random seed in initial fast region merging (random_init_seed)"),
               sortRestore("Restore\nDefault"), 
               sortButton("Sort region classes and objects (sort)"),
               edgeDissimOptionRestore("Restore\nDefault"), 
               rnbLevelsRestore("Restore\nDefault"), 
               rnbLevelsObject("Number of recursive levels (rnb_levels)"),
               ionbLevelsRestore("Restore\nDefault"), 
               ionbLevelsObject("Recursive level at which data I/O is performed (ionb_levels)"),
               minNregionsRestore("Restore\nDefault"), 
               minNregionsObject("Number of regions for convergence at intermediate stages (min_nregions)"),
               spClustMinRestore("Restore\nDefault"), 
               spClustMinObject("Minimum number of regions for which\nspectral clustering is utilized (spclust_min)"),
               spClustMaxRestore("Restore\nDefault"), 
               spClustMaxObject("Maximum number of regions for which\nspectral clustering is utilized (spclust_max)"),
               mergeAccelRestore("Restore\nDefault"), 
               mergeAccelButton("Utilize small region merge acceleration (merge_acceleration)")
  {
    if (params.program_mode == 1)
      set_title("Specification of Experimental HSWO Parameters");
    else if (params.program_mode == 2)
      set_title("Specification of Experimental HSeg Parameters");
    else
      set_title("Specification of Experimental RHSeg Parameters");
    set_default_size(512,64);

  // Initialize widgets
#ifdef GTKMM3
    normindComboBox.append("    No Normalization (normind)      ");
    normindComboBox.append("  Normalize Across Bands (normind)  ");
    normindComboBox.append("Normalize Bands Separately (normind)");
    edgeDissimOptionComboBox.append("Merge Enhancement Option (edge_dissim_option)");
    edgeDissimOptionComboBox.append("Merge Suppression Option (edge_dissim_option)");
#else
    normindComboBox.append_text("    No Normalization (normind)      ");
    normindComboBox.append_text("  Normalize Across Bands (normind)  ");
    normindComboBox.append_text("Normalize Bands Separately (normind)");
    edgeDissimOptionComboBox.append_text("Merge Enhancement Option (edge_dissim_option)");
    edgeDissimOptionComboBox.append_text("Merge Suppression Option (edge_dissim_option)");
#endif
    switch (params.normind)
    {
      case 1:  normindComboBox.set_active_text("    No Normalization (normind)      ");
               break;
      case 2:  normindComboBox.set_active_text("  Normalize Across Bands (normind)  ");
               break;
      case 3:  normindComboBox.set_active_text("Normalize Bands Separately (normind)");
               break;
      default: normindComboBox.set_active_text("  Normalize Across Bands (normind)  ");
               break;
    }
    switch (params.edge_dissim_option)
    {
      case 1:  edgeDissimOptionComboBox.set_active_text("Merge Enhancement Option (edge_dissim_option)");
               break;
      case 2:  edgeDissimOptionComboBox.set_active_text("Merge Suppression Option (edge_dissim_option)");
               break;
      default: edgeDissimOptionComboBox.set_active_text("Merge Enhancement Option (edge_dissim_option)");
               break;
    }
    initThresholdObject.set_fvalue(params.init_threshold);
    initialMergeFlag = params.initial_merge_flag;
    if (initialMergeFlag)
      initialMergeNpixelsObject.set_value(params.initial_merge_npix);
    else
      initialMergeNpixelsObject.set_value(0);
    randomInitSeedFlag = params.random_init_seed_flag;
    randomInitSeedButton.set_active(randomInitSeedFlag);
    sortFlag = params.sort_flag;
    sortButton.set_active(sortFlag);
    rnbLevelsObject.set_value(params.rnb_levels);
#ifndef PARALLEL
    ionbLevelsObject.set_value(params.ionb_levels);
#endif
    minNregionsObject.set_value(params.min_nregions);
    spClustMinObject.set_value(params.spclust_min);
    spClustMaxObject.set_value(params.spclust_max);
    mergeAccelFlag = params.merge_accel_flag;
    mergeAccelButton.set_active(mergeAccelFlag);
    mergeAccelButton.set_alignment(0.0,0.5);


    add(vBox); // put a MenuBar at the top of the box and other stuff below it.

  //Create actions for menus:
    m_refActionGroup = Gtk::ActionGroup::create();

    //Add normal Actions:
    m_refActionGroup->add( Gtk::Action::create("ActionMenu", "Program _Actions") );
    m_refActionGroup->add( Gtk::Action::create("Help", "_Help"),
                           sigc::mem_fun(*this, &ExpParamsGUI::on_help_requested) );
    m_refActionGroup->add( Gtk::Action::create("Close", "_Close"),
                           sigc::mem_fun(*this, &ExpParamsGUI::on_close_requested) );

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
          "      <menuitem action='Close'/>"
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

  // Set up signal handlers
    normindComboBox.signal_changed().connect(sigc::mem_fun(*this,
                 &ExpParamsGUI::on_normindComboBox_changed) );
    initThresholdObject.signal_activate().connect(sigc::mem_fun(*this,
                 &ExpParamsGUI::on_initThresholdObject_activate) );
    initialMergeNpixelsObject.signal_activate().connect(sigc::mem_fun(*this,
                 &ExpParamsGUI::on_initialMergeNpixelsObject_activate) );
    randomInitSeedButton.signal_clicked().connect(sigc::mem_fun(*this,
                 &ExpParamsGUI::on_randomInitSeedButton_clicked) );
    sortButton.signal_clicked().connect(sigc::mem_fun(*this,
                 &ExpParamsGUI::on_sortButton_clicked) );
    edgeDissimOptionComboBox.signal_changed().connect(sigc::mem_fun(*this,
                 &ExpParamsGUI::on_edgeDissimOptionComboBox_changed) );
    rnbLevelsObject.signal_activate().connect(sigc::mem_fun(*this,
                 &ExpParamsGUI::on_rnbLevelsObject_activate) );
    ionbLevelsObject.signal_activate().connect(sigc::mem_fun(*this,
                 &ExpParamsGUI::on_ionbLevelsObject_activate) );
    minNregionsObject.signal_activate().connect(sigc::mem_fun(*this,
                 &ExpParamsGUI::on_minNregionsObject_activate) );
    spClustMinObject.signal_activate().connect(sigc::mem_fun(*this,
                 &ExpParamsGUI::on_spClustMinObject_activate) );
    spClustMaxObject.signal_activate().connect(sigc::mem_fun(*this,
                 &ExpParamsGUI::on_spClustMaxObject_activate) );
    mergeAccelButton.signal_clicked().connect(sigc::mem_fun(*this,
                 &ExpParamsGUI::on_mergeAccelButton_clicked) );
    normindRestore.signal_clicked().connect(sigc::mem_fun(*this,
                 &ExpParamsGUI::on_normindRestore_clicked) );
    initThresholdRestore.signal_clicked().connect(sigc::mem_fun(*this,
                 &ExpParamsGUI::on_initThresholdRestore_clicked) );
    initialMergeNpixelsRestore.signal_clicked().connect(sigc::mem_fun(*this,
                 &ExpParamsGUI::on_initialMergeNpixelsRestore_clicked) );
    randomInitSeedRestore.signal_clicked().connect(sigc::mem_fun(*this,
                 &ExpParamsGUI::on_randomInitSeedRestore_clicked) );
    sortRestore.signal_clicked().connect(sigc::mem_fun(*this,
                 &ExpParamsGUI::on_sortRestore_clicked) );
    edgeDissimOptionRestore.signal_clicked().connect(sigc::mem_fun(*this,
                 &ExpParamsGUI::on_edgeDissimOptionRestore_clicked) );
    rnbLevelsRestore.signal_clicked().connect(sigc::mem_fun(*this,
                 &ExpParamsGUI::on_rnbLevelsRestore_clicked) );
    ionbLevelsRestore.signal_clicked().connect(sigc::mem_fun(*this,
                 &ExpParamsGUI::on_ionbLevelsRestore_clicked) );
    minNregionsRestore.signal_clicked().connect(sigc::mem_fun(*this,
                 &ExpParamsGUI::on_minNregionsRestore_clicked) );
    spClustMinRestore.signal_clicked().connect(sigc::mem_fun(*this,
                 &ExpParamsGUI::on_spClustMinRestore_clicked) );
    spClustMaxRestore.signal_clicked().connect(sigc::mem_fun(*this,
                 &ExpParamsGUI::on_spClustMaxRestore_clicked) );
    mergeAccelRestore.signal_clicked().connect(sigc::mem_fun(*this,
                 &ExpParamsGUI::on_mergeAccelRestore_clicked) );

  // Pack GUI
    lineTable.attach(normindRestore,0,1,0,1);
    lineTable.attach(normindComboBox,1,10,0,1);
    lineTable.attach(initThresholdRestore,0,1,1,2);
    lineTable.attach(initThresholdObject,1,10,1,2);
    lineTable.attach(initialMergeNpixelsRestore,0,1,2,3);
    lineTable.attach(initialMergeNpixelsObject,1,10,2,3);
    lineTable.attach(randomInitSeedRestore,0,1,3,4);
    lineTable.attach(randomInitSeedButton,1,10,3,4);
    lineTable.attach(sortRestore,0,1,4,5);
    lineTable.attach(sortButton,1,10,4,5);
    lineTable.attach(edgeDissimOptionRestore,0,1,5,6);
    lineTable.attach(edgeDissimOptionComboBox,1,10,5,6);
    lineTable.attach(minNregionsRestore,0,1,6,7);
    lineTable.attach(minNregionsObject,1,10,6,7);
    lineTable.attach(spClustMinRestore,0,1,7,8);
    lineTable.attach(spClustMinObject,1,10,7,8);
    lineTable.attach(spClustMaxRestore,0,1,8,9);
    lineTable.attach(spClustMaxObject,1,10,8,9);
    lineTable.attach(mergeAccelRestore,0,1,9,10);
    lineTable.attach(mergeAccelButton,1,10,9,10);
    lineTable.attach(rnbLevelsRestore,0,1,10,11);
    lineTable.attach(rnbLevelsObject,1,10,10,11);
#ifndef PARALLEL
    lineTable.attach(ionbLevelsRestore,0,1,11,12);
    lineTable.attach(ionbLevelsObject,1,10,11,12);
#endif
    vBox.pack_start(lineTable);

    show_all_children();

    if (params.program_mode == 1)
    {
      spClustMinRestore.hide();
      spClustMinObject.hide();
      spClustMaxRestore.hide();
      spClustMaxObject.hide();
    }

    if (params.program_mode != 3)
    {
      rnbLevelsRestore.hide();
      rnbLevelsObject.hide();
#ifndef PARALLEL
      ionbLevelsRestore.hide();
      ionbLevelsObject.hide();
#endif
      minNregionsRestore.hide();
      minNregionsObject.hide();
    }

    show();

  }

 // Destructor...
  ExpParamsGUI::~ExpParamsGUI() 
  {
  }

  void ExpParamsGUI::on_help_requested()
  {
    Glib::ustring strMessage = "Enter in the program File inputs.\n\nFor more help type "
                               "\"rhseg -help\" on the command line.\n\n";
    Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
    
    dialog.run();

    return;
  }

  void ExpParamsGUI::on_close_requested()
  {
    hide();
    return;
  }

  void ExpParamsGUI::on_normindComboBox_changed()
  {
    string tmp_string;

    tmp_string = normindComboBox.get_active_text();
    if (tmp_string == "    No Normalization (normind)      ")
      params.normind = 1;
    else if (tmp_string == "  Normalize Across Bands (normind)  ")
      params.normind = 2;
    else if (tmp_string == "Normalize Bands Separately (normind)")
      params.normind = 3;
    switch (params.normind)
    {
      case 1:  cout << "normind changed to \"No Normalization (normind)\"" << endl;
               break;
      case 2:  cout << "normind changed to \"Normalize Across Bands (normind)\"" << endl;
               break;
      case 3:  cout << "normind changed to \"Normalize Bands Separately (normind)\"" << endl;
               break;
      default: cout << "Invalid value for normind" << endl;
               break;
    }
    return;
  }

  void ExpParamsGUI::on_initThresholdObject_activate()
  {
    params.init_threshold = initThresholdObject.get_fvalue();
cout << "init_threshold changed to " << params.init_threshold << endl;
    
    return;
  }

  void ExpParamsGUI::on_initialMergeNpixelsObject_activate()
  {
    params.initial_merge_npix = initialMergeNpixelsObject.get_value();
    if (params.initial_merge_npix > 0)
    {
      initialMergeFlag = true;
    }
    else
    {
      initialMergeFlag = false;
    }
    return;
  }

  void ExpParamsGUI::on_randomInitSeedButton_clicked()
  {
    if (randomInitSeedFlag)
    {
      randomInitSeedFlag = false;
    }
    else
    {
      randomInitSeedFlag = true;
    }
    return;
  }

  void ExpParamsGUI::on_sortButton_clicked()
  {
    if (sortFlag)
    {
      sortFlag = false;
    }
    else
    {
      sortFlag = true;
    }
    return;
  }

  void ExpParamsGUI::on_edgeDissimOptionComboBox_changed()
  {
    string tmp_string;

    tmp_string = edgeDissimOptionComboBox.get_active_text();
    if (tmp_string == "Merge Enhancement Option (edge_dissim_option)")
      params.edge_dissim_option = 1;
    else if (tmp_string == "Merge Suppression Option (edge_dissim_option)")
      params.edge_dissim_option = 2;
    switch (params.edge_dissim_option)
    {
      case 1:  cout << "edge_dissim_option changed to \"Merge Enhancement Option (edge_dissim_option)\"" << endl;
               break;
      case 2:  if ((params.program_mode == 1) || (params.spclust_wght == 0.0))
               {
                 cout << "\"Merge Suppression Option\" not allowed for HSWO program mode" << endl;
                 params.edge_dissim_option = 1;
               }
               else
                 cout << "edge_dissim_option changed to \"Merge Suppression Option (edge_dissim_option)\"" << endl;
               break;
      default: cout << "Invalid value for edge_dissim_option" << endl;
               break;
    }
    return;
  }

  void ExpParamsGUI::on_rnbLevelsObject_activate()
  {
    params.rnb_levels = rnbLevelsObject.get_value();
    params.rnb_levels_flag = true;
cout << "rnb_levels changed to " << params.rnb_levels << endl;
    params.calc_defaults();
    ionbLevelsObject.set_value(params.ionb_levels);
    minNregionsObject.set_value(params.min_nregions);
    spClustMinObject.set_value(params.spclust_min);
    spClustMaxObject.set_value(params.spclust_max);

    return;
  }

  void ExpParamsGUI::on_ionbLevelsObject_activate()
  {
    params.ionb_levels = ionbLevelsObject.get_value();
    params.ionb_levels_flag = true;
cout << "ionb_levels changed to " << params.ionb_levels << endl;
    params.calc_defaults();
    rnbLevelsObject.set_value(params.rnb_levels);
    minNregionsObject.set_value(params.min_nregions);
    spClustMinObject.set_value(params.spclust_min);
    spClustMaxObject.set_value(params.spclust_max);
    
    return;
  }

  void ExpParamsGUI::on_minNregionsObject_activate()
  {
    params.min_nregions = minNregionsObject.get_value();
    params.min_nregions_flag = true;
cout << "min_nregions changed to " << params.min_nregions << endl;
    params.calc_defaults();
    rnbLevelsObject.set_value(params.rnb_levels);
    ionbLevelsObject.set_value(params.ionb_levels);
    spClustMinObject.set_value(params.spclust_min);
    spClustMaxObject.set_value(params.spclust_max);
    
    return;
  }

  void ExpParamsGUI::on_spClustMinObject_activate()
  {
    params.spclust_min = spClustMinObject.get_value();
cout << "spclust_min changed to " << params.spclust_min << endl;
    params.calc_defaults();
    rnbLevelsObject.set_value(params.rnb_levels);
    ionbLevelsObject.set_value(params.ionb_levels);
    minNregionsObject.set_value(params.min_nregions);
    spClustMaxObject.set_value(params.spclust_max);
    
    return;
  }

  void ExpParamsGUI::on_spClustMaxObject_activate()
  {
    params.spclust_max = spClustMaxObject.get_value();
cout << "spclust_max changed to " << params.spclust_max << endl;
    params.calc_defaults();
    rnbLevelsObject.set_value(params.rnb_levels);
    ionbLevelsObject.set_value(params.ionb_levels);
    minNregionsObject.set_value(params.min_nregions);
    spClustMinObject.set_value(params.spclust_min);
    
    return;
  }

  void ExpParamsGUI::on_mergeAccelButton_clicked()
  {
    if (mergeAccelFlag)
    {
      mergeAccelFlag = false;
    }
    else
    {
      mergeAccelFlag = true;
    }
    return;
  }

  void ExpParamsGUI::on_normindRestore_clicked()
  {
    string tmp_string;

    params.normind = NORMIND;
    switch (params.normind)
    {
      case 1:  normindComboBox.set_active_text("    No Normalization (normind)      ");
cout << "normind restored to \"No Normalization (normind)\"" << endl;
               break;
      case 2:  normindComboBox.set_active_text("  Normalize Across Bands (normind)  ");
cout << "normind restored to \"Normalize Across Bands (normind)\"" << endl;
               break;
      case 3:  normindComboBox.set_active_text("Normalize Bands Separately (normind)");
cout << "normind restored to \"Normalize Bands Separately (normind)\"" << endl;
               break;
      default: normindComboBox.set_active_text("  Normalize Across Bands (normind)  ");
cout << "Invalid value for normind" << endl;
               break;
    }

    return;
  }

  void ExpParamsGUI::on_initThresholdRestore_clicked()
  {
    params.init_threshold = INIT_THRESHOLD;
    initThresholdObject.set_fvalue(params.init_threshold);
cout << "init_threshold restored to " << params.init_threshold << endl;
    
    return;
  }

  void ExpParamsGUI::on_initialMergeNpixelsRestore_clicked()
  {
    params.initial_merge_flag = false;
    params.initial_merge_npix = 0;
    initialMergeFlag = false;
cout << "intial_merge_npix restored to 0" << endl;

    return;
  }

  void ExpParamsGUI::on_randomInitSeedRestore_clicked()
  {
    randomInitSeedButton.set_active(true);
    params.random_init_seed_flag = true;
    randomInitSeedFlag = true;
if (randomInitSeedFlag)
  cout << "random_init_seed restored to true" << endl;
else    
  cout << "random_init_seed restored to false" << endl;

    return;
  }

  void ExpParamsGUI::on_sortRestore_clicked()
  {
    sortButton.set_active(true);
    params.sort_flag = true;
    sortFlag = true;
if (sortFlag)
  cout << "sort restored to true" << endl;
else    
  cout << "sort restored to false" << endl;

    return;
  }

  void ExpParamsGUI::on_edgeDissimOptionRestore_clicked()
  {
    string tmp_string;

    params.edge_dissim_option = EDGE_DISSIM_OPTION;
    if (params.program_mode == 1)
      params.edge_dissim_option = 1;
    switch (params.edge_dissim_option)
    {
      case 1:  edgeDissimOptionComboBox.set_active_text("Merge Enhancement Option (edge_dissim_option)");
cout << "edge_dissim_option restored to \"Merge Enhancement Option (edge_dissim_option)\"" << endl;
               break;
      case 2:  edgeDissimOptionComboBox.set_active_text("Merge Suppression Option (edge_dissim_option)");
cout << "edge_dissim_option restored to \"Merge Suppression Option (edge_dissim_option)\"" << endl;
               break;
      default: edgeDissimOptionComboBox.set_active_text("Merge Enhancement Option (edge_dissim_option)");
cout << "Invalid value for edge_dissim_option" << endl;
               break;
    }

    return;
  }

  void ExpParamsGUI::on_rnbLevelsRestore_clicked()
  {
    params.rnb_levels_flag = false;
    params.calc_defaults();
    rnbLevelsObject.set_value(params.rnb_levels);
cout << "rnb_levels restored to " << params.rnb_levels << endl;
    ionbLevelsObject.set_value(params.ionb_levels);
    minNregionsObject.set_value(params.min_nregions);
    spClustMinObject.set_value(params.spclust_min);
    spClustMaxObject.set_value(params.spclust_max);
    
    return;
  }

  void ExpParamsGUI::on_ionbLevelsRestore_clicked()
  {
    params.ionb_levels_flag = false;
    params.calc_defaults();
    ionbLevelsObject.set_value(params.ionb_levels);
cout << "ionb_levels restored to " << params.ionb_levels << endl;
    rnbLevelsObject.set_value(params.rnb_levels);
    minNregionsObject.set_value(params.min_nregions);
    spClustMinObject.set_value(params.spclust_min);
    spClustMaxObject.set_value(params.spclust_max);
  
    return;
  }

  void ExpParamsGUI::on_minNregionsRestore_clicked()
  {
    params.min_nregions_flag = false;
    params.calc_defaults();
    minNregionsObject.set_value(params.min_nregions);
cout << "min_nregions restored to " << params.min_nregions << endl;
    rnbLevelsObject.set_value(params.rnb_levels);
    ionbLevelsObject.set_value(params.ionb_levels);
    spClustMinObject.set_value(params.spclust_min);
    spClustMaxObject.set_value(params.spclust_max);

    return;
  }

  void ExpParamsGUI::on_spClustMinRestore_clicked()
  {
    params.spclust_min = 0;
    if (params.spclust_wght > 0.0)
      params.spclust_min = SPCLUST_MIN;
    params.calc_defaults();
    spClustMinObject.set_value(params.spclust_min);
cout << "spclust_min restored to " << params.spclust_min << endl;
    rnbLevelsObject.set_value(params.rnb_levels);
    ionbLevelsObject.set_value(params.ionb_levels);
    minNregionsObject.set_value(params.min_nregions);
    spClustMaxObject.set_value(params.spclust_max);
    
    return;
  }

  void ExpParamsGUI::on_spClustMaxRestore_clicked()
  {
    params.spclust_max = 0;
    if (params.spclust_wght > 0.0)
      params.spclust_max = SPCLUST_MAX;
    params.calc_defaults();
    spClustMaxObject.set_value(params.spclust_max);
cout << "spclust_max restored to " << params.spclust_max << endl;
    rnbLevelsObject.set_value(params.rnb_levels);
    ionbLevelsObject.set_value(params.ionb_levels);
    minNregionsObject.set_value(params.min_nregions);
    spClustMinObject.set_value(params.spclust_min);
    
    return;
  }

  void ExpParamsGUI::on_mergeAccelRestore_clicked()
  {
    params.merge_accel_flag = MERGE_ACCEL_FLAG;
    mergeAccelFlag = params.merge_accel_flag;
    mergeAccelButton.set_active(mergeAccelFlag);
if (mergeAccelFlag)
  cout << "merge_acceleration restored to true" << endl;
else    
  cout << "merge_acceleration restored to false" << endl;

    return;
  }

} // namespace HSEGTilton
