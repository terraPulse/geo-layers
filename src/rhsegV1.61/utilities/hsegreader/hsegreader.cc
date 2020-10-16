/*-----------------------------------------------------------
|
|  Routine Name: hsegreader - Program to read the outputs from the Hierarchical Segmentation (HSEG) or
|                             Recursive HSEG (RHSEG) programs.
|
|       Purpose: Main function for the hsegreader program
|
|         Input: 
|
|        Output: 
|
|       Returns: TRUE (1) on success, FALSE (0) on failure
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: November 15, 2007
| Modifications: March 24, 2008 - Added gtkmm code for HSEGReader Class
|
------------------------------------------------------------*/
#include "hsegreader.h"
#include "params/initialParams.h"
#include <params/params.h>

#include <iostream>

extern HSEGTilton::InitialParams initialParams;
extern HSEGTilton::Params params;
extern HSEGTilton::oParams oparams;

namespace HSEGTilton
{
#ifdef GTKMM
 // Constructor
  HSEGReader::HSEGReader():
              windowTable(5,5,true), 
              segLevelLabel("Select Hierarchical Segmentation Level:",1.0),
              classOrderLabel("Region Class Ordering:",1.0), 
              objectOrderLabel("Region Object Ordering:",1.0),
              classLabel("Region Class Label:",1.0), 
              objectLabel("Region Object Label:",1.0),
              classNpixButton("By Size"),
              classStdDevButton("By Std. Dev."),
              classBPRatioButton("By Boundary Pixel Ratio"),
              objectNpixButton("By Size"),
              objectStdDevButton("By Std. Dev."),
              objectBPRatioButton("By Boundary Pixel Ratio"),
              nextClassButton("Select next largest Region Class"),
              nextObjectButton("Select next largest Region Object in selected Region Class")
  {
    nb_classes = oparams.level0_nb_classes;
    region_classes_size = nb_classes;
    region_classes.resize(region_classes_size,RegionClass());

    nb_objects = 0;
    if (params.region_nb_objects_flag)
      nb_objects = oparams.level0_nb_objects;
    region_objects_size = nb_objects;
    if (region_objects_size > 0)
      region_objects.resize(region_objects_size,RegionObject());
#else
  bool hsegreader()
  {
    Spatial spatial_data;

    int nb_classes = oparams.level0_nb_classes;
    RegionClass::set_static_vals();
    int region_classes_size = nb_classes;
    vector<RegionClass> region_classes(region_classes_size);

    int nb_objects = 1;
    if (params.region_nb_objects_flag)
      nb_objects = oparams.level0_nb_objects;
    RegionObject::set_static_vals(initialParams.region_class_nghbrs_list_flag,
                                  initialParams.region_object_nghbrs_list_flag);
    int region_objects_size = nb_objects;
    vector<RegionObject> region_objects(region_objects_size);
    if (!params.region_nb_objects_flag)
      nb_objects = 0;

    Results results_data;
#endif
    spatial_data.read_region_maps();

    int hlevel, region_index;
    results_data.set_buffer_sizes(params.nbands,nb_classes,nb_objects);
    results_data.open_input(params.region_classes_file,params.region_objects_file);
    for (hlevel = 0; hlevel < oparams.nb_levels; hlevel++)
    {
      results_data.read(hlevel,nb_classes,nb_objects,oparams.nb_levels,oparams.int_buffer_size,region_classes,region_objects);
      if (params.debug > 0)
      {
        params.log_fs << endl << "Data written at hlevel = " << hlevel << " for " << nb_classes << " region classes,";
        params.log_fs << " with maximum merging threshold = " << oparams.max_threshold[hlevel];
        if (params.gdissim_flag)
          params.log_fs << " and with global dissimilarity value = " << oparams.gdissim[hlevel];
        params.log_fs << "." << endl;
        if (params.region_nb_objects_flag)
          params.log_fs << "There are " << nb_objects << " region objects." << endl;
        params.log_fs << endl;
      }
      else
      {  
        cout << endl << "Data written at hlevel = " << hlevel << " for " << nb_classes << " region classes,";
        cout << " with maximum merging threshold = " << oparams.max_threshold[hlevel];
        if (params.gdissim_flag)
          cout << " and with global dissimilarity value = " << oparams.gdissim[hlevel];
        cout << "." << endl;
        if (params.region_nb_objects_flag)
          cout << "There are " << nb_objects << " region objects." << endl;
        cout << endl;
      }
      if (params.debug > 2)
      {
        if (((params.region_objects_flag) && (params.object_labels_map_flag)) &&
            ((initialParams.region_class_nghbrs_list_flag) || (initialParams.region_object_nghbrs_list_flag)))
          object_nghbrs_set_init(hlevel, spatial_data, region_classes, region_objects);
        params.log_fs << endl << "After writing out results, dump of the region class data:" << endl << endl;
        for (region_index = 0; region_index < region_classes_size; ++region_index)
          if (region_classes[region_index].get_active_flag())
            region_classes[region_index].print(region_classes,region_objects);
        if (params.debug > 2)
        {
          params.log_fs << endl << "After writing out results, dump of region class label map:" << endl << endl;
          spatial_data.print_class_label_map(hlevel,region_classes);
          if (params.region_objects_list_flag)
          {
            params.log_fs << endl << "After writing out results, dump of the region object data:" << endl << endl;
            for (region_index = 0; region_index < region_objects_size; ++region_index)
              if (region_objects[region_index].get_active_flag())
                region_objects[region_index].print(region_objects);
            params.log_fs << endl << "After writing out results, dump of region object label map:" << endl << endl;
            spatial_data.print_object_label_map(hlevel,region_objects);
          }
        }
      }
    }
    results_data.close_input();
#ifdef GTKMM
  //Build the GUI panel
    set_title("Hierarchical Segmentation Results Reader");

    add(vBox); // put a MenuBar at the top of the box and other stuff below it.

  //Create actions for menus:
    m_refActionGroup = Gtk::ActionGroup::create();

    //Add normal Actions:
    m_refActionGroup->add( Gtk::Action::create("ActionMenu", "Program _Actions") );
    m_refActionGroup->add( Gtk::Action::create("Help", "_Help"),
                      sigc::mem_fun(*this, &HSEGReader::on_help_requested) );
    m_refActionGroup->add( Gtk::Action::create("Quit", "_Quit"),
                      sigc::mem_fun(*this, &HSEGReader::on_quit_requested) );

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
    segLevelComboBox.signal_changed().connect(sigc::mem_fun(*this,
                      &HSEGReader::on_segLevelComboBox_changed) );
    classNpixButton.signal_clicked().connect(sigc::mem_fun(*this,
                      &HSEGReader::on_classNpixButton_clicked) );
    classStdDevButton.signal_clicked().connect(sigc::mem_fun(*this,
                      &HSEGReader::on_classStdDevButton_clicked) );
    classBPRatioButton.signal_clicked().connect(sigc::mem_fun(*this,
                      &HSEGReader::on_classBPRatioButton_clicked) );
    objectNpixButton.signal_clicked().connect(sigc::mem_fun(*this,
                      &HSEGReader::on_objectNpixButton_clicked) );
    objectStdDevButton.signal_clicked().connect(sigc::mem_fun(*this,
                      &HSEGReader::on_objectStdDevButton_clicked) );
    objectBPRatioButton.signal_clicked().connect(sigc::mem_fun(*this,
                      &HSEGReader::on_objectBPRatioButton_clicked) );
    classSpinButton.signal_value_changed().connect(sigc::mem_fun(*this,
                      &HSEGReader::on_classSpinButton_value_changed) );
    nextClassButton.signal_clicked().connect(sigc::mem_fun(*this,
                      &HSEGReader::on_nextClassButton_clicked) );
    objectSpinButton.signal_value_changed().connect(sigc::mem_fun(*this,
                      &HSEGReader::on_objectSpinButton_value_changed) );
    nextObjectButton.signal_clicked().connect(sigc::mem_fun(*this,
                      &HSEGReader::on_nextObjectButton_clicked) );

  // Complete initialization of widgets
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

    windowTable.attach(segLevelLabel,1,3,0,1);
    windowTable.attach(segLevelComboBox,3,4,0,1);

    windowTable.attach(classOrderLabel,0,1,1,2);
    windowTable.attach(classNpixButton,1,2,1,2);
    windowTable.attach(classStdDevButton,2,3,1,2);
    windowTable.attach(classBPRatioButton,3,5,1,2);

    if (params.region_objects_flag)
    {
      windowTable.attach(objectOrderLabel,0,1,2,3);
      windowTable.attach(objectNpixButton,1,2,2,3);
      windowTable.attach(objectStdDevButton,2,3,2,3);
      windowTable.attach(objectBPRatioButton,3,5,2,3);
    }

    classSpinButton.set_range(1,oparams.level0_nb_classes);
    classSpinButton.set_digits(0);
    classSpinButton.set_increments(1,10);
    classSpinButton.set_numeric(true);
    classSpinButton.set_snap_to_ticks(true);
    classSpinButton.set_alignment(1.0);

    windowTable.attach(classLabel,0,1,3,4);
    windowTable.attach(classSpinButton,1,2,3,4);
    windowTable.attach(nextClassButton,2,5,3,4);

    if (params.region_objects_flag)
    {
      objectSpinButton.set_range(1,oparams.level0_nb_objects);
      objectSpinButton.set_digits(0);
      objectSpinButton.set_increments(1,100);
      objectSpinButton.set_numeric(true);
      objectSpinButton.set_snap_to_ticks(true);
      objectSpinButton.set_alignment(1.0);
 
      windowTable.attach(objectLabel,0,1,4,5);
      windowTable.attach(objectSpinButton,1,2,4,5);
      windowTable.attach(nextObjectButton,2,5,4,5);
    }
    windowTable.set_col_spacings(10);

    featureBox.pack_start(classTextView);
    featureBox.pack_start(objectTextView);

  //Pack the GUI panel
    vBox.pack_start(windowTable);
    vBox.pack_start(featureBox);

    show_all_children();

    show();

    classOrderLabel.hide();
    classNpixButton.hide();
    classStdDevButton.hide();
    classBPRatioButton.hide();
    classLabel.hide();
    classSpinButton.hide();
    nextClassButton.hide();
    classTextView.hide();
    if (params.region_objects_flag)
    {
      objectOrderLabel.hide();
      objectNpixButton.hide();
      objectStdDevButton.hide();
      objectBPRatioButton.hide();
      objectLabel.hide();
      objectSpinButton.hide();
      nextObjectButton.hide();
    }
    objectTextView.hide();

    return;
#else
    return true;
#endif
  }
#ifdef GTKMM
  HSEGReader::~HSEGReader()
  {
  }

  void HSEGReader::on_help_requested()
  {
    Glib::ustring strMessage = "TBD";
    Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
    
    dialog.run();

    return;
  }

  void HSEGReader::on_quit_requested()
  {
    hide();
  }

  void HSEGReader::on_segLevelComboBox_changed()
  {
    string tmp_string;

    tmp_string = segLevelComboBox.get_active_text();

    if (tmp_string == "(select level)")
    {
      classOrderLabel.hide();
      classNpixButton.hide();
      classStdDevButton.hide();
      classBPRatioButton.hide();
      classLabel.hide();
      classSpinButton.hide();
      nextClassButton.hide();
      classTextView.hide();
      if (params.region_objects_flag)
      {
        objectOrderLabel.hide();
        objectNpixButton.hide();
        objectStdDevButton.hide();
        objectBPRatioButton.hide();
        objectLabel.hide();
        objectSpinButton.hide();
        nextObjectButton.hide();
        objectTextView.hide();
      }
      return;
    }

    segLevel = atoi(tmp_string.c_str());

    results_data.open_input(params.region_classes_file,params.region_objects_file);
    results_data.read(segLevel,nb_classes,nb_objects,oparams.nb_levels,oparams.int_buffer_size,region_classes,region_objects);
    results_data.close_input();
    if (((params.region_objects_flag) && (params.object_labels_map_flag)) &&
        ((initialParams.region_class_nghbrs_list_flag) || (initialParams.region_object_nghbrs_list_flag)))
      object_nghbrs_set_init(segLevel, spatial_data, region_classes, region_objects);

    classOrderLabel.show();
    classNpixButton.show();
    classStdDevButton.show();
    classBPRatioButton.show();
    classLabel.hide();
    classSpinButton.hide();
    nextClassButton.hide();
    classTextView.hide();
    if (params.region_objects_flag)
    {
      objectOrderLabel.hide();
      objectNpixButton.hide();
      objectStdDevButton.hide();
      objectBPRatioButton.hide();
      objectLabel.hide();
      objectSpinButton.hide();
      nextObjectButton.hide();
      objectTextView.hide();
    }

    return;
  }

  void HSEGReader::on_classNpixButton_clicked()
  {
    int region_index, heap_index;
    int selectedClassLabel;

    classNpixFlag = true;
    classStdDevFlag = false;
    classBPRatioFlag = false;

    class_heap_size = nb_classes+1;
    class_heap.resize(class_heap_size);
    heap_index = 0;
    for (region_index = 0; region_index < region_classes_size; ++region_index)
      if (region_classes[region_index].get_active_flag())
        class_heap[heap_index++] = &region_classes[region_index];
    class_heap[--class_heap_size] = NULL;
    make_heap(&class_heap[0],&class_heap[class_heap_size],ClassNpixMoreThan());
    selectedClassLabel = class_heap[0]->get_label();
    if (selectedClassLabel == classSpinButton.get_value_as_int())
    {
   // Force a change in classSpinButton value to force proper updating
      if (selectedClassLabel == 1)
        classSpinButton.set_value(2);
      else
        classSpinButton.set_value(1);
    }
    classSpinButton.set_value(selectedClassLabel);
    pop_heap(&class_heap[0],&class_heap[class_heap_size--],ClassNpixMoreThan());

    classLabel.show();
    classSpinButton.show();
    nextClassButton.show();
    classTextView.show();
    objectOrderLabel.show();
    objectNpixButton.show();
    objectStdDevButton.show();
    objectBPRatioButton.show();

    return;
  }

  void HSEGReader::on_classStdDevButton_clicked()
  {
    int region_index, heap_index;
    int selectedClassLabel;

    classNpixFlag = false;
    classStdDevFlag = true;
    classBPRatioFlag = false;

    class_heap_size = nb_classes+1;
    class_heap.resize(class_heap_size);
    heap_index = 0;
    for (region_index = 0; region_index < region_classes_size; ++region_index)
      if (region_classes[region_index].get_active_flag())
        class_heap[heap_index++] = &region_classes[region_index];
    class_heap[--class_heap_size] = NULL;
    make_heap(&class_heap[0],&class_heap[class_heap_size],ClassStdDevLessThan());
    selectedClassLabel = class_heap[0]->get_label();
    if (selectedClassLabel == classSpinButton.get_value_as_int())
    {
   // Force a change in classSpinButton value to force proper updating
      if (selectedClassLabel == 1)
        classSpinButton.set_value(2);
      else
        classSpinButton.set_value(1);
    }
    classSpinButton.set_value(selectedClassLabel);
    pop_heap(&class_heap[0],&class_heap[class_heap_size--],ClassStdDevLessThan());

    classLabel.show();
    classSpinButton.show();
    nextClassButton.show();
    classTextView.show();
    objectOrderLabel.show();
    objectNpixButton.show();
    objectStdDevButton.show();
    objectBPRatioButton.show();

    return;
  }

  void HSEGReader::on_classBPRatioButton_clicked()
  {
    int region_index, heap_index;
    int selectedClassLabel;

    classNpixFlag = false;
    classStdDevFlag = false;
    classBPRatioFlag = true;

    class_heap_size = nb_classes+1;
    class_heap.resize(class_heap_size);
    heap_index = 0;
    for (region_index = 0; region_index < region_classes_size; ++region_index)
      if (region_classes[region_index].get_active_flag())
        class_heap[heap_index++] = &region_classes[region_index];
    class_heap[--class_heap_size] = NULL;
    make_heap(&class_heap[0],&class_heap[class_heap_size],ClassBPRatioLessThan());
    selectedClassLabel = class_heap[0]->get_label();
    if (selectedClassLabel == classSpinButton.get_value_as_int())
    {
   // Force a change in classSpinButton value to force proper updating
      if (selectedClassLabel == 1)
        classSpinButton.set_value(2);
      else
        classSpinButton.set_value(1);
    }
    classSpinButton.set_value(selectedClassLabel);
    pop_heap(&class_heap[0],&class_heap[class_heap_size--],ClassBPRatioLessThan());

    classLabel.show();
    classSpinButton.show();
    nextClassButton.show();
    classTextView.show();
    objectOrderLabel.show();
    objectNpixButton.show();
    objectStdDevButton.show();
    objectBPRatioButton.show();

    return;
  }

  void HSEGReader::on_objectNpixButton_clicked()
  {
    unsigned int heap_index, class_index, object_index;
    int selectedClassLabel, selectedObjectLabel;

    objectNpixFlag = true;
    objectStdDevFlag = false;
    objectBPRatioFlag = false;

    selectedClassLabel = classSpinButton.get_value_as_int();
    class_index = selectedClassLabel - 1;
    set<unsigned int>::iterator object_label_set_iter, object_label_set_end;
    object_heap_size = region_classes[class_index].get_nb_region_objects() + 1;
    object_heap.resize(object_heap_size);
    heap_index = 0;
    object_label_set_iter = region_classes[class_index].get_region_objects_set_begin();
    object_label_set_end = region_classes[class_index].get_region_objects_set_end();
    while (object_label_set_iter != object_label_set_end)
    {
      object_index = (*object_label_set_iter) - 1;
      object_heap[heap_index++] = &region_objects[object_index];
      ++object_label_set_iter;
    }
    object_heap[--object_heap_size] = NULL;
    make_heap(&object_heap[0],&object_heap[object_heap_size],ObjectNpixMoreThan());
    selectedObjectLabel = object_heap[0]->get_label();
    if (selectedObjectLabel == objectSpinButton.get_value_as_int())
    {
   // Force a change in objectSpinButton value to force proper updating
      if (selectedObjectLabel == 1)
        objectSpinButton.set_value(2);
      else
        objectSpinButton.set_value(1);
    }
    objectSpinButton.set_value(selectedObjectLabel);
    pop_heap(&object_heap[0],&object_heap[object_heap_size--],ObjectNpixMoreThan());

    objectLabel.show();
    objectSpinButton.show();
    nextObjectButton.show();
    objectTextView.show();

    return;
  }

  void HSEGReader::on_objectStdDevButton_clicked()
  {
    unsigned int heap_index, class_index, object_index;
    int selectedClassLabel, selectedObjectLabel;

    objectNpixFlag = false;
    objectStdDevFlag = true;
    objectBPRatioFlag = false;

    selectedClassLabel = classSpinButton.get_value_as_int();
    class_index = selectedClassLabel - 1;
    set<unsigned int>::iterator object_label_set_iter, object_label_set_end;
    object_heap_size = region_classes[class_index].get_nb_region_objects() + 1;
    object_heap.resize(object_heap_size);
    heap_index = 0;
    object_label_set_iter = region_classes[class_index].get_region_objects_set_begin();
    object_label_set_end = region_classes[class_index].get_region_objects_set_end();
    while (object_label_set_iter != object_label_set_end)
    {
      object_index = (*object_label_set_iter) - 1;
      object_heap[heap_index++] = &region_objects[object_index];
      ++object_label_set_iter;
    }
    object_heap[--object_heap_size] = NULL;
    make_heap(&object_heap[0],&object_heap[object_heap_size],ObjectStdDevLessThan());
    selectedObjectLabel = object_heap[0]->get_label();
    if (selectedObjectLabel == objectSpinButton.get_value_as_int())
    {
   // Force a change in objectSpinButton value to force proper updating
      if (selectedObjectLabel == 1)
        objectSpinButton.set_value(2);
      else
        objectSpinButton.set_value(1);
    }
    objectSpinButton.set_value(selectedObjectLabel);
    pop_heap(&object_heap[0],&object_heap[object_heap_size--],ObjectStdDevLessThan());

    objectLabel.show();
    objectSpinButton.show();
    nextObjectButton.show();
    objectTextView.show();

    return;
  }

  void HSEGReader::on_objectBPRatioButton_clicked()
  {
    unsigned int heap_index, class_index, object_index;
    int selectedClassLabel, selectedObjectLabel;

    objectNpixFlag = false;
    objectStdDevFlag = false;
    objectBPRatioFlag = true;

    selectedClassLabel = classSpinButton.get_value_as_int();
    class_index = selectedClassLabel - 1;
    set<unsigned int>::iterator object_label_set_iter, object_label_set_end;
    object_heap_size = region_classes[class_index].get_nb_region_objects() + 1;
    object_heap.resize(object_heap_size);
    heap_index = 0;
    object_label_set_iter = region_classes[class_index].get_region_objects_set_begin();
    object_label_set_end = region_classes[class_index].get_region_objects_set_end();
    while (object_label_set_iter != object_label_set_end)
    {
      object_index = (*object_label_set_iter) - 1;
      object_heap[heap_index++] = &region_objects[object_index];
      ++object_label_set_iter;
    }
    object_heap[--object_heap_size] = NULL;
    make_heap(&object_heap[0],&object_heap[object_heap_size],ObjectBPRatioLessThan());
    selectedObjectLabel = object_heap[0]->get_label();
    if (selectedObjectLabel == objectSpinButton.get_value_as_int())
    {
   // Force a change in objectSpinButton value to force proper updating
      if (selectedObjectLabel == 1)
        objectSpinButton.set_value(2);
      else
        objectSpinButton.set_value(1);
    }
    objectSpinButton.set_value(selectedObjectLabel);
    pop_heap(&object_heap[0],&object_heap[object_heap_size--],ObjectBPRatioLessThan());

    objectLabel.show();
    objectSpinButton.show();
    nextObjectButton.show();
    objectTextView.show();

    return;
  }

  void HSEGReader::on_classSpinButton_value_changed()
  {
    unsigned int region_index;
    int selectedClassLabel = classSpinButton.get_value_as_int();

    if (selectedClassLabel > 0)
    {
      region_index = selectedClassLabel - 1;
      string strMessage;
      region_classes[region_index].print(strMessage);
      classTextBuffer = Gtk::TextBuffer::create();
      classTextBuffer->set_text(strMessage);
      classTextView.set_buffer(classTextBuffer);
      if (params.region_objects_flag)
      {
        if (region_classes[region_index].get_active_flag())
        {
          objectOrderLabel.show();
          objectNpixButton.show();
          objectStdDevButton.show();
          objectBPRatioButton.show();
        }
        else
        {
          objectOrderLabel.hide();
          objectNpixButton.hide();
          objectStdDevButton.hide();
          objectBPRatioButton.hide();
        }
        objectLabel.hide();
        objectSpinButton.hide();
        nextObjectButton.hide();
        objectTextView.hide();
      }

    }

    return;
  }

  void HSEGReader::on_nextClassButton_clicked()
  {
    int selectedClassLabel;
    set<unsigned int>::iterator object_label_set_iter, object_label_set_end;

    if (class_heap_size > 0)
    {
      selectedClassLabel = class_heap[0]->get_label();
      classSpinButton.set_value(selectedClassLabel);
      if (classNpixFlag)
        pop_heap(&class_heap[0],&class_heap[class_heap_size--],ClassNpixMoreThan());
      else if (classStdDevFlag)
        pop_heap(&class_heap[0],&class_heap[class_heap_size--],ClassStdDevLessThan());
      else if (classBPRatioFlag)
        pop_heap(&class_heap[0],&class_heap[class_heap_size--],ClassBPRatioLessThan());
    }
    else
    {
      Glib::ustring strMessage = "Smallest Region Class already selected!";
      Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
    
      dialog.run();
    }

    return;
  }

  void HSEGReader::on_objectSpinButton_value_changed()
  {
    unsigned int region_index;
    int selectedObjectLabel = objectSpinButton.get_value_as_int();

    if (selectedObjectLabel > 0)
    {
      region_index = selectedObjectLabel - 1;
      string strMessage;
      region_objects[region_index].print(strMessage);
      Glib::RefPtr<Gtk::TextBuffer> refTextBuffer;
      objectTextBuffer = Gtk::TextBuffer::create();
      objectTextBuffer->set_text(strMessage);
      objectTextView.set_buffer(objectTextBuffer);
    }
    return;
  }

  void HSEGReader::on_nextObjectButton_clicked()
  {
    int selectedObjectLabel;

    if (object_heap_size > 0)
    {
      selectedObjectLabel = object_heap[0]->get_label();
      objectSpinButton.set_value(selectedObjectLabel);
      if (objectNpixFlag)
        pop_heap(&object_heap[0],&object_heap[object_heap_size--],ObjectNpixMoreThan());
      else if (objectStdDevFlag)
        pop_heap(&object_heap[0],&object_heap[object_heap_size--],ObjectStdDevLessThan());
      else if (objectBPRatioFlag)
        pop_heap(&object_heap[0],&object_heap[object_heap_size--],ObjectBPRatioLessThan());
    }
    else
    {
      Glib::ustring strMessage = "Smallest Region Object (in selected Region Class) already selected!";
      Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
    
      dialog.run();
    }

    return;
  }

#endif
} // namespace HSEGTilton


