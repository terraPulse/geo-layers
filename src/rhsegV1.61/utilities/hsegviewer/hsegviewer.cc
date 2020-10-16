// HSegViewer.cc
#include "hsegviewer.h"
#include "params/initialParams.h"
#include <params/params.h>
#include <iostream>

extern HSEGTilton::InitialParams initialParams;
extern HSEGTilton::Params params;
extern HSEGTilton::oParams oparams;
extern CommonTilton::Image inputImage;
extern CommonTilton::Image maskImage;
CommonTilton::Image classLabelsMapImage;
CommonTilton::Image boundaryMapImage;
CommonTilton::Image objectLabelsMapImage;
CommonTilton::Image segLevelClassMeanImage;
CommonTilton::Image segLevelClassesMapImage;
CommonTilton::Image segLevelBoundaryMapImage;
CommonTilton::Image segLevelObjectsMapImage;
CommonTilton::Image segLevelClassStdDevImage;
CommonTilton::Image segLevelObjectStdDevImage;
CommonTilton::Image segLevelClassBPRatioImage;
CommonTilton::Image segLevelObjectBPRatioImage;
CommonTilton::Image reference1Image;
CommonTilton::Image reference2Image;
CommonTilton::Image reference3Image;
CommonTilton::Image labelInImage;
CommonTilton::Image labelDataImage;
CommonTilton::Image labelMaskImage;

using namespace CommonTilton;

namespace HSEGTilton
{
 // Constructor
  HSegViewer::HSegViewer():
              vBox(false,10),
              line1Table(1,5,true), line2Table(1,5,true), line3Table(1,3,true), line4Table(1,3,true), 
              line5Table(1,3,true), selectSegTable(1,3,true),
              displayOptionsFrame("  Display Options:"), displayOptionsTable(5,3,true),
              featureValuesFrame(""),
              selectRegionClassButton("Select Region Class at Location of Last Left Mouse Click"),
              selectRegionObjectButton("Select Region Object at Location of Last Left Mouse Click"),
              selectRegionClass("  Select Pixels with Segmentation Level 0 Region Class Label:"),
              labelRegionButton("Label Region"),
              selectRegionObject("  Select Pixels with Segmentation Level 0 Region Object Label:"),
              refocusButton("Refocus on Selected Region"),
              initSegLevel("  Initial Segmentation Level:"),
              selectFinerSegButton("Select Next Finer Segmentation"),
              segLevelObject(""),
              selectCoarserSegButton("Select Next Coarser Segmentation"),
              rgbImageButton("RGB Image"), classesSliceButton("Segmentation Classes Slice"),
              regionMeanButton("Region Class Mean Image"),
              regionLabelButton("Current Region Labels"),
              objectsSliceButton("Segmentation Objects Slice"),
              boundaryMapButton("Hierarchical Boundary Map"),
              classStdDevButton("Region Class Std. Dev. Image"), 
              objectStdDevButton("Region Object Std. Dev. Image"),
              classBPRatioButton("Region Class Boundary Pixel Ratio Image"),
              objectBPRatioButton("Region Object Boundary Pixel Ratio Image"),
              reference1Button("Reference 1"), reference2Button("Reference 2"),
              reference3Button("Reference 3"),
              updateRGBImageButton("Update RGB Image Display"),
              redDisplayBand("Red Display Band:"),
              greenDisplayBand("Green Display Band:"),
              blueDisplayBand("Blue Display Band:"),
              rgbImageStretchLabel("RGB Image Stretch Option:"),
              rangeFromObject("Range From: "),
              rangeToObject("To: ")
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

    initializeImages();

    initSegLevel.set_value(INIT_SEG_LEVEL);
    segLevelObject.set_value(INIT_SEG_LEVEL);
    
    selClickFlag = false;
    selClassLabelFlag = false;
    selObjectLabelFlag = false;
    segLevel = INIT_SEG_LEVEL;

   // Read the region object data (resultsData) for all hierarchical levels,
   // concurrently initializing the region statistical information (statsData). 
    int hlevel;
    resultsData.open_input(params.region_classes_file,params.region_objects_file);
    for (hlevel = 0; hlevel < oparams.nb_levels; hlevel++)
    {
      resultsData.set_int_buffer_size(hlevel);
      resultsData.read(hlevel,nb_classes,nb_objects,statsData);
    }
    resultsData.close_input();

   // Set LabelRegion text and colormap
    labelRegion.set_text_and_colormap();
    labelRegion.write_ascii_out();

   // Copy LabelRegion colormap to labelDataImage
    update_label_data_colormap();

   // Sets segLevelClassesMapImage, segLevelBoundaryMap, segLevelObjectsMapImage, segLevelClassMeanImage, segLevelClassStdDevImage,
   // segLevelObjectStdDevImage, segLevelClassBPRatioImage, and segLevelObjectBPRatioImage for the currently selected 
   // hierarchical segmentation level.
    statsData.set_segLevel(segLevel);

    unsigned int colormap_size = oparams.level0_nb_classes + 1;
    if ((initialParams.grey_scale_flag) && (oparams.level0_nb_classes < 256))
    {
      segLevelClassesMapImage.resize_colormap(colormap_size);
      statsData.compute_and_set_colormap(segLevel,segLevelClassesMapImage);
    }
    else
    {
      if (colormap_size > MAX_COLORMAP_SIZE)
        colormap_size = MAX_COLORMAP_SIZE; 
      segLevelClassesMapImage.compute_colormap(colormap_size);
    }
    if (params.object_labels_map_flag)
    {
      colormap_size = oparams.level0_nb_objects + 1;
      if (colormap_size > MAX_COLORMAP_SIZE)
        colormap_size = MAX_COLORMAP_SIZE; 
      segLevelObjectsMapImage.compute_colormap(colormap_size);
    }

    DisplayImage::set_static_values(inputImage,params.current_folder);

    if (params.mask_flag)
      rgbImageDisplay.init_rgb(inputImage,maskImage,"RGB Image");
    else
      rgbImageDisplay.init_rgb(inputImage,"RGB Image");

    classesSliceDisplay.init_seg(segLevelClassesMapImage,"Segmentation Classes Slice Image");

    if (params.region_sum_flag)
    {
      if (params.mask_flag)
        regionMeanDisplay.init_rgb(segLevelClassMeanImage,maskImage,"Region Class Mean Image");
      else
        regionMeanDisplay.init_rgb(segLevelClassMeanImage,"Region Class Mean Image");
    }

    labelDataDisplay.init_region_label(labelDataImage,labelMaskImage,"Current Region Labels");

    if (params.object_labels_map_flag)
      objectsSliceDisplay.init_seg(segLevelObjectsMapImage,"Segmentation Objects Slice Image");

    if (params.boundary_map_flag)
    {
      boundaryMapDisplay.init_boundary(segLevelBoundaryMapImage,"Hierarchical Boundary Map");
    }

    if (params.region_std_dev_flag)
    {
      classStdDevDisplay.init_float(segLevelClassStdDevImage,"Region Class Std. Dev. Image");
      if (params.object_labels_map_flag)
        objectStdDevDisplay.init_float(segLevelObjectStdDevImage,"Region Object Std. Dev. Image");
    }
    if (params.region_boundary_npix_flag)
    {
      classBPRatioDisplay.init_float(segLevelClassBPRatioImage,"Region Class Boundary Pixel Ratio Image");
      if (params.object_labels_map_flag)
        objectBPRatioDisplay.init_float(segLevelObjectBPRatioImage,"Region Object Boundary Pixel Ratio Image");
    }

    if (initialParams.reference1_flag)
      reference1Display.init_reference(reference1Image,"Reference 1");
    if (initialParams.reference2_flag)
      reference2Display.init_reference(reference2Image,"Reference 2");
    if (initialParams.reference3_flag)
      reference3Display.init_reference(reference3Image,"Reference 3");

  // Link the DisplayImages
    rgbImageDisplay.link_image(&classesSliceDisplay);
    if (params.region_sum_flag)
      rgbImageDisplay.link_image(&regionMeanDisplay);
    rgbImageDisplay.link_image(&labelDataDisplay);
    if (params.object_labels_map_flag)
      rgbImageDisplay.link_image(&objectsSliceDisplay);
    if (params.boundary_map_flag)
      rgbImageDisplay.link_image(&boundaryMapDisplay);
    if (params.region_std_dev_flag)
    {
      rgbImageDisplay.link_image(&classStdDevDisplay);
      if (params.object_labels_map_flag)
        rgbImageDisplay.link_image(&objectStdDevDisplay);
    }
    if (params.region_boundary_npix_flag)
    {
      rgbImageDisplay.link_image(&classBPRatioDisplay);
      if (params.object_labels_map_flag)
        rgbImageDisplay.link_image(&objectBPRatioDisplay);
    }
    if (initialParams.reference1_flag)
      rgbImageDisplay.link_image(&reference1Display);
    if (initialParams.reference2_flag)
      rgbImageDisplay.link_image(&reference2Display);
    if (initialParams.reference3_flag)
      rgbImageDisplay.link_image(&reference3Display);

    classesSliceDisplay.link_image(&rgbImageDisplay);
    if (params.region_sum_flag)
      classesSliceDisplay.link_image(&regionMeanDisplay);
    classesSliceDisplay.link_image(&labelDataDisplay);
    if (params.object_labels_map_flag)
      classesSliceDisplay.link_image(&objectsSliceDisplay);
    if (params.boundary_map_flag)
      classesSliceDisplay.link_image(&boundaryMapDisplay);
    if (params.region_std_dev_flag)
    {
      classesSliceDisplay.link_image(&classStdDevDisplay);
      if (params.object_labels_map_flag)
        classesSliceDisplay.link_image(&objectStdDevDisplay);
    }
    if (params.region_boundary_npix_flag)
    {
      classesSliceDisplay.link_image(&classBPRatioDisplay);
      if (params.object_labels_map_flag)
        classesSliceDisplay.link_image(&objectBPRatioDisplay);
    }
    if (initialParams.reference1_flag)
      classesSliceDisplay.link_image(&reference1Display);
    if (initialParams.reference2_flag)
      classesSliceDisplay.link_image(&reference2Display);
    if (initialParams.reference3_flag)
      classesSliceDisplay.link_image(&reference3Display);

    if (params.region_sum_flag)
    {
      regionMeanDisplay.link_image(&rgbImageDisplay);
      regionMeanDisplay.link_image(&classesSliceDisplay);
      regionMeanDisplay.link_image(&labelDataDisplay);
      if (params.object_labels_map_flag)
        regionMeanDisplay.link_image(&objectsSliceDisplay);
      if (params.boundary_map_flag)
        regionMeanDisplay.link_image(&boundaryMapDisplay);
      if (params.region_std_dev_flag)
      {
        regionMeanDisplay.link_image(&classStdDevDisplay);
        if (params.object_labels_map_flag)
          regionMeanDisplay.link_image(&objectStdDevDisplay);
      }
      if (params.region_boundary_npix_flag)
      {
        regionMeanDisplay.link_image(&classBPRatioDisplay);
        if (params.object_labels_map_flag)
          regionMeanDisplay.link_image(&objectBPRatioDisplay);
      }
      if (initialParams.reference1_flag)
        regionMeanDisplay.link_image(&reference1Display);
      if (initialParams.reference2_flag)
        regionMeanDisplay.link_image(&reference2Display);
      if (initialParams.reference3_flag)
        regionMeanDisplay.link_image(&reference3Display);
    }

    labelDataDisplay.link_image(&rgbImageDisplay);
    labelDataDisplay.link_image(&classesSliceDisplay);
    if (params.region_sum_flag)
      labelDataDisplay.link_image(&regionMeanDisplay);
    if (params.object_labels_map_flag)
      labelDataDisplay.link_image(&objectsSliceDisplay);
    if (params.boundary_map_flag)
      labelDataDisplay.link_image(&boundaryMapDisplay);
    if (params.region_std_dev_flag)
    {
      labelDataDisplay.link_image(&classStdDevDisplay);
      if (params.object_labels_map_flag)
        labelDataDisplay.link_image(&objectStdDevDisplay);
    }
    if (params.region_boundary_npix_flag)
    {
      labelDataDisplay.link_image(&classBPRatioDisplay);
      if (params.object_labels_map_flag)
        labelDataDisplay.link_image(&objectBPRatioDisplay);
    }
    if (initialParams.reference1_flag)
      labelDataDisplay.link_image(&reference1Display);
    if (initialParams.reference2_flag)
      labelDataDisplay.link_image(&reference2Display);
    if (initialParams.reference3_flag)
      labelDataDisplay.link_image(&reference3Display);

    if (params.object_labels_map_flag)
    {
      objectsSliceDisplay.link_image(&rgbImageDisplay);
      objectsSliceDisplay.link_image(&classesSliceDisplay);
      if (params.region_sum_flag)
        objectsSliceDisplay.link_image(&regionMeanDisplay);
      objectsSliceDisplay.link_image(&labelDataDisplay);
      if (params.boundary_map_flag)
        objectsSliceDisplay.link_image(&boundaryMapDisplay);
      if (params.region_std_dev_flag)
      {
        objectsSliceDisplay.link_image(&classStdDevDisplay);
        if (params.object_labels_map_flag)
          objectsSliceDisplay.link_image(&objectStdDevDisplay);
      }
      if (params.region_boundary_npix_flag)
      {
        objectsSliceDisplay.link_image(&classBPRatioDisplay);
        if (params.object_labels_map_flag)
          objectsSliceDisplay.link_image(&objectBPRatioDisplay);
      }
      if (initialParams.reference1_flag)
        objectsSliceDisplay.link_image(&reference1Display);
      if (initialParams.reference2_flag)
        objectsSliceDisplay.link_image(&reference2Display);
      if (initialParams.reference3_flag)
        objectsSliceDisplay.link_image(&reference3Display);
    }

    if (params.boundary_map_flag)
    {
      boundaryMapDisplay.link_image(&rgbImageDisplay);
      boundaryMapDisplay.link_image(&classesSliceDisplay);
      if (params.region_sum_flag)
        boundaryMapDisplay.link_image(&regionMeanDisplay);
      boundaryMapDisplay.link_image(&labelDataDisplay);
      if (params.object_labels_map_flag)
        boundaryMapDisplay.link_image(&objectsSliceDisplay);
      if (params.region_std_dev_flag)
      {
        boundaryMapDisplay.link_image(&classStdDevDisplay);
        if (params.object_labels_map_flag)
          boundaryMapDisplay.link_image(&objectStdDevDisplay);
      }
      if (params.region_boundary_npix_flag)
      {
        boundaryMapDisplay.link_image(&classBPRatioDisplay);
        if (params.object_labels_map_flag)
          boundaryMapDisplay.link_image(&objectBPRatioDisplay);
      }
      if (initialParams.reference1_flag)
        boundaryMapDisplay.link_image(&reference1Display);
      if (initialParams.reference2_flag)
        boundaryMapDisplay.link_image(&reference2Display);
      if (initialParams.reference3_flag)
        boundaryMapDisplay.link_image(&reference3Display);
    }

    if (params.region_std_dev_flag)
    {
      classStdDevDisplay.link_image(&rgbImageDisplay);
      classStdDevDisplay.link_image(&classesSliceDisplay);
      if (params.region_sum_flag)
        classStdDevDisplay.link_image(&regionMeanDisplay);
      classStdDevDisplay.link_image(&labelDataDisplay);
      if (params.object_labels_map_flag)
        classStdDevDisplay.link_image(&objectsSliceDisplay);
      if (params.boundary_map_flag)
        classStdDevDisplay.link_image(&boundaryMapDisplay);
      if (params.object_labels_map_flag)
      {
        classStdDevDisplay.link_image(&objectStdDevDisplay);
        objectStdDevDisplay.link_image(&rgbImageDisplay);
        objectStdDevDisplay.link_image(&classesSliceDisplay);
        if (params.region_sum_flag)
          objectStdDevDisplay.link_image(&regionMeanDisplay);
        objectStdDevDisplay.link_image(&labelDataDisplay);
        if (params.object_labels_map_flag)
          objectStdDevDisplay.link_image(&objectsSliceDisplay);
        if (params.boundary_map_flag)
          objectStdDevDisplay.link_image(&boundaryMapDisplay);
        objectStdDevDisplay.link_image(&classStdDevDisplay);
        if (params.region_boundary_npix_flag)
        {
          objectStdDevDisplay.link_image(&classBPRatioDisplay);
          if (params.object_labels_map_flag)
            objectStdDevDisplay.link_image(&objectBPRatioDisplay);
        }
        if (initialParams.reference1_flag)
          objectStdDevDisplay.link_image(&reference1Display);
        if (initialParams.reference2_flag)
          objectStdDevDisplay.link_image(&reference2Display);
        if (initialParams.reference3_flag)
          objectStdDevDisplay.link_image(&reference3Display);
      }
      if (params.region_boundary_npix_flag)
      {
        classStdDevDisplay.link_image(&classBPRatioDisplay);
        if (params.object_labels_map_flag)
          classStdDevDisplay.link_image(&objectBPRatioDisplay);
      }
      if (initialParams.reference1_flag)
        classStdDevDisplay.link_image(&reference1Display);
      if (initialParams.reference2_flag)
        classStdDevDisplay.link_image(&reference2Display);
      if (initialParams.reference3_flag)
        classStdDevDisplay.link_image(&reference3Display);
    }

    if (params.region_boundary_npix_flag)
    {
      classBPRatioDisplay.link_image(&rgbImageDisplay);
      classBPRatioDisplay.link_image(&classesSliceDisplay);
      if (params.region_sum_flag)
        classBPRatioDisplay.link_image(&regionMeanDisplay);
      classBPRatioDisplay.link_image(&labelDataDisplay);
      if (params.object_labels_map_flag)
        classBPRatioDisplay.link_image(&objectsSliceDisplay);
      if (params.boundary_map_flag)
        classBPRatioDisplay.link_image(&boundaryMapDisplay);
      if (params.region_std_dev_flag)
      {
        classBPRatioDisplay.link_image(&classStdDevDisplay);
        if (params.object_labels_map_flag)
          classBPRatioDisplay.link_image(&objectStdDevDisplay);
      }
      if (params.object_labels_map_flag)
      {
        classBPRatioDisplay.link_image(&objectBPRatioDisplay);
        objectBPRatioDisplay.link_image(&rgbImageDisplay);
        objectBPRatioDisplay.link_image(&classesSliceDisplay);
        if (params.region_sum_flag)
          objectBPRatioDisplay.link_image(&regionMeanDisplay);
        objectBPRatioDisplay.link_image(&labelDataDisplay);
        if (params.object_labels_map_flag)
          objectBPRatioDisplay.link_image(&objectsSliceDisplay);
        if (params.boundary_map_flag)
          objectBPRatioDisplay.link_image(&boundaryMapDisplay);
        if (params.region_std_dev_flag)
        {
          objectBPRatioDisplay.link_image(&classStdDevDisplay);
          if (params.object_labels_map_flag)
            objectBPRatioDisplay.link_image(&objectStdDevDisplay);
        }
        objectBPRatioDisplay.link_image(&classBPRatioDisplay);
        if (initialParams.reference1_flag)
          objectBPRatioDisplay.link_image(&reference1Display);
        if (initialParams.reference2_flag)
          objectBPRatioDisplay.link_image(&reference2Display);
        if (initialParams.reference3_flag)
          objectBPRatioDisplay.link_image(&reference3Display);
      }
      if (initialParams.reference1_flag)
        classBPRatioDisplay.link_image(&reference1Display);
      if (initialParams.reference2_flag)
        classBPRatioDisplay.link_image(&reference2Display);
      if (initialParams.reference3_flag)
        classBPRatioDisplay.link_image(&reference3Display);
    }

    if (initialParams.reference1_flag)
    {
      reference1Display.link_image(&rgbImageDisplay);
      reference1Display.link_image(&classesSliceDisplay);
      if (params.region_sum_flag)
        reference1Display.link_image(&regionMeanDisplay);
      reference1Display.link_image(&labelDataDisplay);
      if (params.object_labels_map_flag)
        reference1Display.link_image(&objectsSliceDisplay);
      if (params.boundary_map_flag)
        reference1Display.link_image(&boundaryMapDisplay);
      if (params.region_std_dev_flag)
      {
        reference1Display.link_image(&classStdDevDisplay);
        if (params.object_labels_map_flag)
          reference1Display.link_image(&objectStdDevDisplay);
      }
      if (params.region_boundary_npix_flag)
      {
        reference1Display.link_image(&classBPRatioDisplay);
        if (params.object_labels_map_flag)
          reference1Display.link_image(&objectBPRatioDisplay);
      }
      if (initialParams.reference2_flag)
        reference1Display.link_image(&reference2Display);
      if (initialParams.reference3_flag)
        reference1Display.link_image(&reference3Display);
    }

    if (initialParams.reference2_flag)
    {
      reference2Display.link_image(&rgbImageDisplay);
      reference2Display.link_image(&classesSliceDisplay);
      if (params.region_sum_flag)
        reference2Display.link_image(&regionMeanDisplay);
      reference2Display.link_image(&labelDataDisplay);
      if (params.object_labels_map_flag)
        reference2Display.link_image(&objectsSliceDisplay);
      if (params.boundary_map_flag)
        reference2Display.link_image(&boundaryMapDisplay);
      if (params.region_std_dev_flag)
      {
        reference2Display.link_image(&classStdDevDisplay);
        if (params.object_labels_map_flag)
          reference2Display.link_image(&objectStdDevDisplay);
      }
      if (params.region_boundary_npix_flag)
      {
        reference2Display.link_image(&classBPRatioDisplay);
        if (params.object_labels_map_flag)
          reference2Display.link_image(&objectBPRatioDisplay);
      }
      if (initialParams.reference1_flag)
        reference2Display.link_image(&reference1Display);
      if (initialParams.reference3_flag)
        reference2Display.link_image(&reference3Display);
    }

    if (initialParams.reference3_flag)
    {
      reference3Display.link_image(&rgbImageDisplay);
      reference3Display.link_image(&classesSliceDisplay);
      if (params.region_sum_flag)
        reference3Display.link_image(&regionMeanDisplay);
      reference3Display.link_image(&labelDataDisplay);
      if (params.object_labels_map_flag)
        reference3Display.link_image(&objectsSliceDisplay);
      if (params.boundary_map_flag)
        reference3Display.link_image(&boundaryMapDisplay);
      if (params.region_std_dev_flag)
      {
        reference3Display.link_image(&classStdDevDisplay);
        if (params.object_labels_map_flag)
          reference3Display.link_image(&objectStdDevDisplay);
      }
      if (params.region_boundary_npix_flag)
      {
        reference3Display.link_image(&classBPRatioDisplay);
        if (params.object_labels_map_flag)
          reference3Display.link_image(&objectBPRatioDisplay);
      }
      if (initialParams.reference1_flag)
        reference3Display.link_image(&reference1Display);
      if (initialParams.reference2_flag)
        reference3Display.link_image(&reference2Display);
    }

  //Build the GUI panel
    set_title("Hierarchical Segmentation Results Viewer");

    add(vBox); // put a MenuBar at the top of the box and other stuff below it.

  //Create actions for menus:
    m_refActionGroup = Gtk::ActionGroup::create();

    //Add normal Actions:
    m_refActionGroup->add( Gtk::Action::create("ActionMenu", "Program _Actions") );
    m_refActionGroup->add( Gtk::Action::create("Help", "_Help"),
                      sigc::mem_fun(*this, &HSegViewer::on_help_requested) );
    m_refActionGroup->add( Gtk::Action::create("Quit", "_Quit"),
                      sigc::mem_fun(*this, &HSegViewer::on_quit_requested) );

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
    selectRegionClassButton.signal_clicked().connect(sigc::mem_fun(*this,
                       &HSegViewer::on_selectRegionClassButton_clicked) );
    selectRegionObjectButton.signal_clicked().connect(sigc::mem_fun(*this,
                       &HSegViewer::on_selectRegionObjectButton_clicked) );
    selectRegionClass.signal_activate().connect(sigc::mem_fun(*this,
                       &HSegViewer::on_selectRegionClass_activate) );
    labelRegionButton.signal_clicked().connect(sigc::mem_fun(*this,
                       &HSegViewer::on_labelRegionButton_clicked) );
    selectRegionObject.signal_activate().connect(sigc::mem_fun(*this,
                       &HSegViewer::on_selectRegionObject_activate) );
    refocusButton.signal_clicked().connect(sigc::mem_fun(*this,
                       &HSegViewer::on_refocusButton_clicked) );
    initSegLevel.signal_activate().connect(sigc::mem_fun(*this,
                       &HSegViewer::on_initSegLevel_activate) );
    selectFinerSegButton.signal_clicked().connect(sigc::mem_fun(*this,
                       &HSegViewer::on_selectFinerSegButton_clicked) );
    segLevelObject.signal_activate().connect(sigc::mem_fun(*this,
                       &HSegViewer::on_segLevelObject_activate) );
    selectCoarserSegButton.signal_clicked().connect(sigc::mem_fun(*this,
                       &HSegViewer::on_selectCoarserSegButton_clicked) );
    rgbImageButton.signal_clicked().connect(sigc::mem_fun(*this,
                       &HSegViewer::on_rgbImageButton_clicked) );
    classesSliceButton.signal_clicked().connect(sigc::mem_fun(*this,
                       &HSegViewer::on_classesSliceButton_clicked) );
    if (params.region_sum_flag)
      regionMeanButton.signal_clicked().connect(sigc::mem_fun(*this,
                         &HSegViewer::on_regionMeanButton_clicked) );
    regionLabelButton.signal_clicked().connect(sigc::mem_fun(*this,
                       &HSegViewer::on_regionLabelButton_clicked) );
    if (params.object_labels_map_flag)
      objectsSliceButton.signal_clicked().connect(sigc::mem_fun(*this,
                         &HSegViewer::on_objectsSliceButton_clicked) );
    if (params.boundary_map_flag)
      boundaryMapButton.signal_clicked().connect(sigc::mem_fun(*this,
                         &HSegViewer::on_boundaryMapButton_clicked) );
    if (params.region_std_dev_flag)
    {
      classStdDevButton.signal_clicked().connect(sigc::mem_fun(*this,
                         &HSegViewer::on_classStdDevButton_clicked) );
      if (params.object_labels_map_flag)
        objectStdDevButton.signal_clicked().connect(sigc::mem_fun(*this,
                           &HSegViewer::on_objectStdDevButton_clicked) );
    }
    if (params.region_boundary_npix_flag)
    {
      classBPRatioButton.signal_clicked().connect(sigc::mem_fun(*this,
                         &HSegViewer::on_classBPRatioButton_clicked) );
      if (params.object_labels_map_flag)
        objectBPRatioButton.signal_clicked().connect(sigc::mem_fun(*this,
                           &HSegViewer::on_objectBPRatioButton_clicked) );
    }
    if (initialParams.reference1_flag)
      reference1Button.signal_clicked().connect(sigc::mem_fun(*this,
                         &HSegViewer::on_reference1Button_clicked) );
    if (initialParams.reference2_flag)
      reference2Button.signal_clicked().connect(sigc::mem_fun(*this,
                         &HSegViewer::on_reference2Button_clicked) );
    if (initialParams.reference3_flag)
      reference3Button.signal_clicked().connect(sigc::mem_fun(*this,
                         &HSegViewer::on_reference3Button_clicked) );
    labelRegion.signal_activated().connect(sigc::mem_fun(*this,
                       &HSegViewer::on_labelRegion_activated) );
    labelRegion.signal_color_set().connect(sigc::mem_fun(*this,
                       &HSegViewer::on_labelRegion_color_set) );
    labelRegion.signal_undo().connect(sigc::mem_fun(*this,
                       &HSegViewer::on_labelRegion_undo) );
    updateRGBImageButton.signal_clicked().connect(sigc::mem_fun(*this,
                       &HSegViewer::on_updateRGBImageButton_clicked) );
    rgbImageStretchComboBox.signal_changed().connect(sigc::mem_fun(*this,
                         &HSegViewer::on_rgbImageStretchComboBox_changed) );

  // Complete initialization of widgets
    line1Table.attach(selectRegionClassButton,1,4,0,1);
    line1Table.set_col_spacings(10);
    if (params.region_objects_flag)
    {
      line2Table.attach(selectRegionObjectButton,1,4,0,1);
      line2Table.set_col_spacings(10);
    }
    line3Table.attach(selectRegionClass,0,2,0,1);
    line3Table.attach(labelRegionButton,2,3,0,1);
    line3Table.set_col_spacings(10);
    if (params.region_objects_flag)
    {
      line4Table.attach(selectRegionObject,0,2,0,1);
      line4Table.set_col_spacings(10);
    }
    line5Table.attach(initSegLevel,0,2,0,1);
    line5Table.attach(refocusButton,2,3,0,1);
    line5Table.set_col_spacings(10);
    selectSegTable.attach(selectFinerSegButton,0,1,0,1);
    selectSegTable.attach(segLevelObject,1,2,0,1);
    selectSegTable.attach(selectCoarserSegButton,2,3,0,1);
    selectSegTable.set_col_spacings(10);
    displayOptionsTable.attach(rgbImageButton,0,1,0,1);
    displayOptionsTable.attach(classesSliceButton,1,2,0,1);
    if (params.region_sum_flag)
      displayOptionsTable.attach(regionMeanButton,2,3,0,1);
    displayOptionsTable.attach(regionLabelButton,0,1,1,2);
    if (params.object_labels_map_flag)
      displayOptionsTable.attach(objectsSliceButton,1,2,1,2);
    if (params.boundary_map_flag)
      displayOptionsTable.attach(boundaryMapButton,2,3,1,2);
    if (params.region_std_dev_flag)
    {
      displayOptionsTable.attach(classStdDevButton,0,1,2,3);
      if (params.object_labels_map_flag)
        displayOptionsTable.attach(objectStdDevButton,0,1,3,4);
    }
    if (params.region_boundary_npix_flag)
    {
      displayOptionsTable.attach(classBPRatioButton,1,2,2,3);
      if (params.object_labels_map_flag)
        displayOptionsTable.attach(objectBPRatioButton,1,2,3,4);
    }
/*
    if (initialParams.reference1_flag)
      displayOptionsTable.attach(reference1Button,0,1,2,3);
    if (initialParams.reference2_flag)
      displayOptionsTable.attach(reference2Button,1,2,2,3);
    if (initialParams.reference3_flag)
      displayOptionsTable.attach(reference3Button,2,3,2,3);
*/
    if (initialParams.reference1_flag)
      displayOptionsTable.attach(reference1Button,2,3,2,3);
    if (initialParams.reference2_flag)
      displayOptionsTable.attach(reference2Button,2,3,3,4);
    displayOptionsTable.attach(updateRGBImageButton,1,2,4,5);
    displayOptionsTable.set_col_spacings(10);
    displayOptionsFrame.set_label_align(0.0);
    displayOptionsFrame.add(displayOptionsTable);
    table_nrows = oparams.nb_levels + 1;
    table_ncols = 3;
    if (params.region_boundary_npix_flag)
      table_ncols++;
    if (params.region_std_dev_flag)
      table_ncols++;
    featureValuesTable.resize(table_nrows,table_ncols);
    featureValuesEntries = new Gtk::Entry[table_nrows*table_ncols];
    int index = 0;
    for (int row = 0; row < table_nrows; row++)
      for (int col = 0; col < table_ncols; col++)
      {  
        featureValuesEntries[index].set_editable(false);
        if (row == 0)
          featureValuesEntries[index].set_alignment(0.5);
        else
          featureValuesEntries[index].set_alignment(1.0);
        featureValuesTable.attach(featureValuesEntries[index++],col,col+1,row,row+1);
      }
    scrolledTable.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrolledTable.add(featureValuesTable);
    if (table_nrows < 16)
      scrolledTable.set_size_request(100,(int) (27.2*table_nrows));
    else
      scrolledTable.set_size_request(100,400);
    featureValuesFrame.set_label_align(0.0);
    featureValuesFrame.add(scrolledTable);

  //Pack the GUI panel
    vBox.pack_start(line1Table);
    if (params.region_objects_flag)
      vBox.pack_start(line2Table);  
    vBox.pack_start(line3Table);
    if (params.region_objects_flag)
      vBox.pack_start(line4Table);
    vBox.pack_start(line5Table);
    vBox.pack_start(selectSegTable);
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
    vBox.pack_start(featureValuesFrame);

    show_all_children();

    if (initialParams.rgb_image_stretch == 2)
      rangeBox.hide();

    show();

    return;
  }

  HSegViewer::~HSegViewer()
  {
    return;
  }


  void HSegViewer::on_help_requested()
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

  void HSegViewer::on_quit_requested()
  {
    hide();

    rgbImageDisplay.hide();
    classesSliceDisplay.hide();
    regionMeanDisplay.hide();
    labelDataDisplay.hide();
    objectsSliceDisplay.hide();
    boundaryMapDisplay.hide();
    classStdDevDisplay.hide();
    objectStdDevDisplay.hide();
    classBPRatioDisplay.hide();
    objectBPRatioDisplay.hide();
    reference1Display.hide();
    reference2Display.hide();
    reference3Display.hide();

    Image labelOutImage;
    if (inputImage.gdal_valid())
      labelOutImage.create(initialParams.label_out_file,view_ncols,view_nrows,1,GDT_Byte,inputImage.get_driver_description());
    else
      labelOutImage.create(initialParams.label_out_file,view_ncols,view_nrows,1,GDT_Byte,"GTiff");
    if (inputImage.geotransform_valid())
      labelOutImage.set_geotransform(inputImage);
    labelOutImage.registered_data_copy(0,labelDataImage,0);
    labelOutImage.copy_colormap(labelDataImage);
    labelOutImage.close();

   // Remove temporary files
    rgbImageDisplay.remove_display_file();
    classesSliceDisplay.remove_display_file();
    regionMeanDisplay.remove_display_file();
    objectsSliceDisplay.remove_display_file();
    boundaryMapDisplay.remove_display_file();
    classStdDevDisplay.remove_display_file();
    objectStdDevDisplay.remove_display_file();
    classBPRatioDisplay.remove_display_file();
    objectBPRatioDisplay.remove_display_file();
    reference1Display.remove_display_file();
    reference2Display.remove_display_file();
    reference3Display.remove_display_file();

#ifdef THREEDIM
    std::remove(initialParams.view_input_image_file.c_str());
    if (params.mask_flag)
      std::remove(initialParams.view_mask_image_file.c_str());
    std::remove(initialParams.view_classes_map_file.c_str());
    if (params.boundary_map_flag)
      std::remove(initialParams.view_boundary_map_file.c_str());
    if (params.object_labels_map_flag)
      std::remove(initialParams.view_objects_map_file.c_str());
#endif
    if (params.region_sum_flag)
      std::remove(initialParams.seg_level_class_mean_file.c_str());
    std::remove(initialParams.seg_level_classes_map_file.c_str());
    if (params.boundary_map_flag)
      std::remove(initialParams.seg_level_boundary_map_file.c_str());
    if (params.object_labels_map_flag)
      std::remove(initialParams.seg_level_objects_map_file.c_str());
    if (params.region_std_dev_flag)
    {
      std::remove(initialParams.seg_level_class_std_dev_file.c_str());
      if (params.object_labels_map_flag)
        std::remove(initialParams.seg_level_object_std_dev_file.c_str());
    }
    if (params.region_boundary_npix_flag)
    {
      std::remove(initialParams.seg_level_class_bpratio_file.c_str());
      if (params.object_labels_map_flag)
        std::remove(initialParams.seg_level_object_bpratio_file.c_str());
    }
    std::remove(initialParams.label_data_file.c_str());
    std::remove(initialParams.label_mask_file.c_str());

    return;
  }

  void HSegViewer::on_selectRegionClassButton_clicked()
  {
    int selCol, selRow;

    selClickFlag = classesSliceDisplay.get_click_location(selColClick, selRowClick);

    if (selClickFlag)
    {
      selCol = (int) ((selColClick/classesSliceDisplay.get_X_zoom_factor()) + 0.5);
      selRow = (int) ((selRowClick/classesSliceDisplay.get_Y_zoom_factor()) + 0.5);

      selLabel = (unsigned int) labelDataImage.get_data(selCol,selRow,0);
      labelDataDisplay.set_label_mask(selLabel);

      selClassLabel = (unsigned int) classLabelsMapImage.get_data(selCol,selRow,0);
      if (selClassLabel > 0)
      {
        segLevel = initSegLevel.get_value();
        selectRegionClass.set_value(selClassLabel);
        selectRegionObject.clear();
        selClassLabelFlag = true;
        selObjectLabelFlag = false;
        set_region_class_table();
        segLevelObject.set_value(segLevel);
        statsData.set_segLevel(segLevel);
        set_region_class();
      }
      else
      {
        selClassLabelFlag = false;
        selObjectLabelFlag = false;
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
                                 "can select a region class.";
      Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
    
      dialog.run();
      return;
    }

    return;
  }

  void HSegViewer::on_selectRegionObjectButton_clicked()
  {
    int selCol, selRow;

    selClickFlag = classesSliceDisplay.get_click_location(selColClick, selRowClick);

    if (selClickFlag)
    {
      selCol = (int) ((selColClick/classesSliceDisplay.get_X_zoom_factor()) + 0.5);
      selRow = (int) ((selRowClick/classesSliceDisplay.get_Y_zoom_factor()) + 0.5);

      selLabel = (unsigned int) labelDataImage.get_data(selCol,selRow,0);
      labelDataDisplay.set_label_mask(selLabel);

      selObjectLabel = (unsigned int) objectLabelsMapImage.get_data(selCol,selRow,0);
      if (selObjectLabel > 0)
      {
        segLevel = initSegLevel.get_value();
        selectRegionObject.set_value(selObjectLabel);
        selectRegionClass.clear();
        selClassLabelFlag = false;
        selObjectLabelFlag = true;
        set_region_object_table();
        segLevelObject.set_value(segLevel);
        statsData.set_segLevel(segLevel);
        set_region_object();
      }
      else
      {
        selClassLabelFlag = false;
        selObjectLabelFlag = false;
        Glib::ustring strMessage = "Clicked on masked out image pixel.\n"
                                   "No Region Object selected.";
        Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
    
        dialog.run();
        return;
      }
    }
    else
    {
      Glib::ustring strMessage = "You must perform a left mouse button\n"
                                 "click on an image pixel before you\n"
                                 "can select a region object.";
      Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
    
      dialog.run();
      return;
    }

    return;
  }

  void HSegViewer::on_selectRegionClass_activate()
  {
    if ((selectRegionClass.get_value() > 0) && 
        (selectRegionClass.get_value() <= (int) oparams.level0_nb_classes))
    {
      labelDataDisplay.set_label_mask(0);
      segLevel = initSegLevel.get_value();
      selClassLabel = selectRegionClass.get_value();
      selectRegionObject.clear();
      selClassLabelFlag = true;
      selObjectLabelFlag = false;
      selClickFlag = false;
      set_region_class_table();
      segLevelObject.set_value(segLevel);
      statsData.set_segLevel(segLevel);
      set_region_class();
    }
    else
    {
      Glib::ustring strMessage = "Region Class Label must be greater than 0\n"
                                 "and less than "+ stringify_int(oparams.level0_nb_classes);
      Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
    
      dialog.run();
      return;
    }

    return;
  }

  void HSegViewer::on_labelRegionButton_clicked()
  {
    labelRegion.show();
  }

  void HSegViewer::on_selectRegionObject_activate()
  {
    if ((selectRegionObject.get_value() > 0) && 
        (selectRegionObject.get_value() <= (int) oparams.level0_nb_objects))
    {
      labelDataDisplay.set_label_mask(0);
      segLevel = initSegLevel.get_value();
      selObjectLabel = selectRegionObject.get_value();
      selectRegionClass.clear();
      selClassLabelFlag = false;
      selObjectLabelFlag = true;
      selClickFlag = false;
      set_region_object_table();
      segLevelObject.set_value(segLevel);
      statsData.set_segLevel(segLevel);
      set_region_object();
    }
    else
    {
      Glib::ustring strMessage = "Region Object Label must be greater than 0\n"
                                 "and less than "+ stringify_int(oparams.level0_nb_objects);
      Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
    
      dialog.run();
      return;
    }

    return;
  }

  void HSegViewer::on_refocusButton_clicked()
  {
    if (selClickFlag)
    {
      labelDataDisplay.set_click_location(selColClick,selRowClick);
      labelDataDisplay.set_focus(true);
      labelDataDisplay.set_linked_focus(true);
    }
    else
    {
      Glib::ustring strMessage = "You must select a region class or object\n"
                                 "by performing a left mouse button selection\n"
                                 "before you can perform a refocus.";
      Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
    
      dialog.run();
      return;
    }
  }

  void HSegViewer::on_initSegLevel_activate()
  {
    if ((initSegLevel.get_value() < 0) || (initSegLevel.get_value() >= oparams.nb_levels))
    {
      Glib::ustring strMessage = "Initial Segmentation Level must be greater than\n"
                                 "or equal to 0 and less than "+ stringify_int(oparams.nb_levels);
      Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
    
      dialog.run();
      return;
    }
    return;
  }

  void HSegViewer::on_selectFinerSegButton_clicked()
  {
    bool level_changed = false;
    unsigned int new_label, init_npix, new_npix;

    segLevel = segLevelObject.get_value();
    if (segLevel > 0)
    {
      if (selClassLabelFlag)
      {
        selClassLabel = selectRegionClass.get_value();
        new_label = statsData.get_new_class_label(selClassLabel,segLevel);
        init_npix = statsData.get_class_npix(new_label,segLevel);
        new_npix = init_npix;
        while ((new_npix == init_npix) && (segLevel > 0))
        {
          segLevel--;
          new_label = statsData.get_new_class_label(selClassLabel,segLevel);
          new_npix = statsData.get_class_npix(new_label,segLevel);
        }
        if (new_npix != init_npix)
          level_changed = true;
      }
      else if (selObjectLabelFlag)
      {
        selObjectLabel = selectRegionObject.get_value();
        new_label = statsData.get_new_object_label(selObjectLabel,segLevel);
        init_npix = statsData.get_object_npix(new_label,segLevel);
        new_npix = init_npix;
        while ((new_npix == init_npix) && (segLevel > 0))
        {
          segLevel--;
          new_label = statsData.get_new_object_label(selObjectLabel,segLevel);
          new_npix = statsData.get_object_npix(new_label,segLevel);
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
        segLevelObject.set_value(segLevel);
        statsData.set_segLevel(segLevel);
        if (selClassLabelFlag)
        {
          selClassLabel = selectRegionClass.get_value();
          set_region_class();
        }
        else if (selObjectLabelFlag)
        {
          selObjectLabel = selectRegionObject.get_value();
          set_region_object();
        }
        else
          redisplay();
      }
      else
      {
        Glib::ustring strMessage = "Already at Finest Segmentation Level";
        Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
    
        dialog.run();
        return;
      }
    }
    else
    {
      Glib::ustring strMessage = "Already at Finest Segmentation Level";
      Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
    
      dialog.run();
      return;
    }
    return;
  }

  void HSegViewer::on_segLevelObject_activate()
  {
    if ((segLevelObject.get_value() >= 0) && (segLevelObject.get_value() < oparams.nb_levels))
    {
      segLevel = segLevelObject.get_value();
      statsData.set_segLevel(segLevel);
      if (selClassLabelFlag)
      {
        selClassLabel = selectRegionClass.get_value();
        set_region_class();
      }
      else if (selObjectLabelFlag)
      {
        selObjectLabel = selectRegionObject.get_value();
        set_region_object();
      }
      else
        redisplay();
    }
    else
    {
      Glib::ustring strMessage = "Segmentation Level must be greater than\n"
                                 "or equal to 0 and less than "+ stringify_int(oparams.nb_levels);
      Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
    
      dialog.run();
      return;
    }
    return;
  }

  void HSegViewer::on_selectCoarserSegButton_clicked()
  {
    bool level_changed = false;
    unsigned int new_label, init_npix, new_npix;

    segLevel = segLevelObject.get_value();

    if (segLevel < (oparams.nb_levels - 1))
    {
      if (selClassLabelFlag)
      {
        selClassLabel = selectRegionClass.get_value();
        new_label = statsData.get_new_class_label(selClassLabel,segLevel);
        init_npix = statsData.get_class_npix(new_label,segLevel);
        new_npix = init_npix;
        while ((new_npix == init_npix) && (segLevel < (oparams.nb_levels - 1)))
        {
          segLevel++;
          new_label = statsData.get_new_class_label(selClassLabel,segLevel);
          new_npix = statsData.get_class_npix(new_label,segLevel);
        }
        if (new_npix != init_npix)
          level_changed = true;
      }
      else if (selObjectLabelFlag)
      {
        selObjectLabel = selectRegionObject.get_value();
        new_label = statsData.get_new_object_label(selObjectLabel,segLevel);
        init_npix = statsData.get_object_npix(new_label,segLevel);
        new_npix = init_npix;
        while ((new_npix == init_npix) && (segLevel < (oparams.nb_levels - 1)))
        {
          segLevel++;
          new_label = statsData.get_new_object_label(selObjectLabel,segLevel);
          new_npix = statsData.get_object_npix(new_label,segLevel);
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
        segLevelObject.set_value(segLevel);
        statsData.set_segLevel(segLevel);
        if (selClassLabelFlag)
        {
          selClassLabel = selectRegionClass.get_value();
          set_region_class();
        }
        else if (selObjectLabelFlag)
        {
          selObjectLabel = selectRegionObject.get_value();
          set_region_object();
        }
        else
          redisplay();
      }
      else
      {
        Glib::ustring strMessage = "Already at Coarsest Segmentation Level";
        Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
    
        dialog.run();
        return;
      } 
    }
    else
    {
      Glib::ustring strMessage = "Already at Coarsest Segmentation Level";
      Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
    
      dialog.run();
      return;
    }
    return;
  }

  void HSegViewer::on_rgbImageButton_clicked()
  {
    rgbImageDisplay.show();

    return;
  }

  void HSegViewer::on_classesSliceButton_clicked()
  {
    classesSliceDisplay.show();

    return;
  }

  void HSegViewer::on_regionMeanButton_clicked()
  {
    regionMeanDisplay.show();

    return;
  }

  void HSegViewer::on_regionLabelButton_clicked()
  {
    labelDataDisplay.show();

    return;
  }

  void HSegViewer::on_objectsSliceButton_clicked()
  {
    objectsSliceDisplay.show();

    return;
  }

  void HSegViewer::on_boundaryMapButton_clicked()
  {
    boundaryMapDisplay.show();

    return;
  }

  void HSegViewer::on_classStdDevButton_clicked()
  {
    classStdDevDisplay.show();

    return;
  }

  void HSegViewer::on_objectStdDevButton_clicked()
  {
    objectStdDevDisplay.show();

    return;
  }

  void HSegViewer::on_classBPRatioButton_clicked()
  {
    classBPRatioDisplay.show();

    return;
  }

  void HSegViewer::on_objectBPRatioButton_clicked()
  {
    objectBPRatioDisplay.show();

    return;
  }

  void HSegViewer::on_reference1Button_clicked()
  {
    reference1Display.show();

    return;
  }

  void HSegViewer::on_reference2Button_clicked()
  {
    reference2Display.show();

    return;
  }

  void HSegViewer::on_reference3Button_clicked()
  {
    reference3Display.show();

    return;
  }

  void HSegViewer::on_labelRegion_activated()
  {
    Image labelOutImage;
    if (inputImage.gdal_valid())
      labelOutImage.create(initialParams.label_out_file,view_ncols,view_nrows,1,GDT_Byte,inputImage.get_driver_description());
    else
      labelOutImage.create(initialParams.label_out_file,view_ncols,view_nrows,1,GDT_Byte,"GTiff");
    if (inputImage.geotransform_valid())
      labelOutImage.set_geotransform(inputImage);
    labelOutImage.registered_data_copy(0,labelDataImage,0);
    labelOutImage.copy_colormap(labelDataImage);
    labelOutImage.close();

    update_region_label(labelRegion.get_highlight_label());
    labelDataDisplay.reset_display_highlight();
    labelDataDisplay.reinit_region_label();

    return;
  }

  void HSegViewer::on_labelRegion_color_set()
  {
    update_label_data_colormap();

    return;
  }

  void HSegViewer::on_labelRegion_undo()
  {
    Image labelOutImage;

    labelOutImage.open(initialParams.label_out_file);
    labelDataImage.registered_data_copy(0,labelOutImage,0);
    labelOutImage.close();

    labelDataDisplay.reset_display_highlight();
    labelDataDisplay.reinit_region_label();

    return;
  }

  void HSegViewer::on_updateRGBImageButton_clicked()
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
/* Have problems with reinitialization of the segLevelClassMeanImage - just skip it for now
      if (params.region_sum_flag)
      {
      // Reinitialize segLevelClassMeanImage
        if (inputImage.get_nbands() < 3)
        {
          segLevelClassMeanImage.set_rgb_display_bands(inputImage.get_red_display_band(),
                                                       inputImage.get_green_display_band(),
                                                       inputImage.get_blue_display_band());
        }
        segLevelClassMeanImage.copy_histo_lookup(segLevelClassMeanImage.get_red_display_band(),
                                                 inputImage,inputImage.get_red_display_band());
        if (segLevelClassMeanImage.get_green_display_band() != segLevelClassMeanImage.get_red_display_band())
          segLevelClassMeanImage.copy_histo_lookup(segLevelClassMeanImage.get_green_display_band(),
                                                   inputImage,inputImage.get_green_display_band());
        if ((segLevelClassMeanImage.get_blue_display_band() != segLevelClassMeanImage.get_red_display_band()) &&
            (segLevelClassMeanImage.get_blue_display_band() != segLevelClassMeanImage.get_green_display_band()))
          segLevelClassMeanImage.copy_histo_lookup(segLevelClassMeanImage.get_blue_display_band(),
                                                   inputImage,inputImage.get_blue_display_band());
        segLevelClassMeanImage.set_rgb_image_stretch(initialParams.rgb_image_stretch,
                                                     initialParams.range[0],initialParams.range[1]);
        
        regionMeanDisplay.reinit_rgb();
      }
*/
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

  void HSegViewer::on_rgbImageStretchComboBox_changed()
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

#ifdef THREEDIM
  bool HSegViewer::subsetfrom3d() // Creates the 2D subset of Image objects from the input 3D Image objects
  {
    int col, row, slice, band, view_col, view_row;
    int min_ncols, max_ncols, min_nrows, max_nrows, min_nslices, max_nslices;

    min_ncols = initialParams.ncols_offset;
    max_ncols = initialParams.ncols_offset + initialParams.ncols_subset;
    min_nrows = initialParams.nrows_offset;
    max_nrows = initialParams.nrows_offset + initialParams.nrows_subset;
    min_nslices = initialParams.nslices_offset;
    max_nslices = initialParams.nslices_offset + initialParams.nslices_subset;

    Image tempImage;
    tempImage.create(initialParams.view_input_image_file, view_ncols, view_nrows, params.nbands, inputImage.get_data_type(), "GTiff");
    
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
            tempImage.put_data(inputImage.get_data(col,row,slice,band),view_col,view_row,band);
          }
    tempImage.flush_data();
    inputImage.close();
    tempImage.close();
    inputImage.open(initialParams.view_input_image_file);

    if (params.mask_flag)
    {
      tempImage.create(initialParams.view_mask_image_file, view_ncols, view_nrows, 1, maskImage.get_data_type(), "GTiff");
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
            tempImage.put_data(maskImage.get_data(col,row,slice,0),view_col,view_row,0);
          }
      tempImage.flush_data();
      maskImage.close();
      tempImage.close();
      maskImage.open(initialParams.view_mask_image_file);
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

    if (params.boundary_map_flag)
    {
      Image boundaryMap3dImage;
      boundaryMap3dImage.open(params.boundary_map_file, params.ncols, params.nrows, params.nslices, 1, UInt16);
      boundaryMapImage.create(initialParams.view_boundary_map_file, view_ncols, view_nrows, 1, GDT_UInt16, "GTiff");
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
            boundaryMapImage.put_data(boundaryMap3dImage.get_data(col,row,slice,0),view_col,view_row,0);
          }
      boundaryMapImage.flush_data();
      boundaryMap3dImage.close();
      boundaryMapImage.close();
    }

    if (params.object_labels_map_flag)
    {
      Image objectLabelsMap3dImage;
      objectLabelsMap3dImage.open(params.object_labels_map_file, params.ncols, params.nrows, params.nslices, 1, UInt32);
      objectLabelsMapImage.create(initialParams.view_objects_map_file, view_ncols, view_nrows, 1, GDT_UInt32, "GTiff");
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
            objectLabelsMapImage.put_data(objectLabelsMap3dImage.get_data(col,row,slice,0),view_col,view_row,0);
          }
      objectLabelsMapImage.flush_data();
      objectLabelsMap3dImage.close();
      objectLabelsMapImage.close();
    }

    return true;
  }
#endif

  bool HSegViewer::initializeImages() // Initializes the Image Class variables
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

    if (params.region_sum_flag)
    {
    // Initialize segLevelClassMeanImage
      if (inputImage.get_nbands() > 2)
      {
        segLevelClassMeanImage.create(initialParams.seg_level_class_mean_file, view_ncols, view_nrows, 3, 
                                      inputImage.get_data_type(),"GTiff");
        segLevelClassMeanImage.set_rgb_display_bands(0,1,2);
      }
      else
      {
        segLevelClassMeanImage.create(initialParams.seg_level_class_mean_file, view_ncols, view_nrows, 
                                      inputImage.get_nbands(), inputImage.get_data_type(),"GTiff");
        segLevelClassMeanImage.set_rgb_display_bands(inputImage.get_red_display_band(),
                                                     inputImage.get_green_display_band(),
                                                     inputImage.get_blue_display_band());
      }
      segLevelClassMeanImage.copy_histo_eq_map(segLevelClassMeanImage.get_red_display_band(),
                                               inputImage,inputImage.get_red_display_band());
      segLevelClassMeanImage.copy_histo_lookup(segLevelClassMeanImage.get_red_display_band(),
                                               inputImage,inputImage.get_red_display_band());
      if (segLevelClassMeanImage.get_green_display_band() != segLevelClassMeanImage.get_red_display_band())
      {
        segLevelClassMeanImage.copy_histo_eq_map(segLevelClassMeanImage.get_green_display_band(),
                                                 inputImage,inputImage.get_green_display_band());
        segLevelClassMeanImage.copy_histo_lookup(segLevelClassMeanImage.get_green_display_band(),
                                                 inputImage,inputImage.get_green_display_band());
      }
      if ((segLevelClassMeanImage.get_blue_display_band() != segLevelClassMeanImage.get_red_display_band()) &&
          (segLevelClassMeanImage.get_blue_display_band() != segLevelClassMeanImage.get_green_display_band()))
      {
        segLevelClassMeanImage.copy_histo_eq_map(segLevelClassMeanImage.get_blue_display_band(),
                                                 inputImage,inputImage.get_blue_display_band());
        segLevelClassMeanImage.copy_histo_lookup(segLevelClassMeanImage.get_blue_display_band(),
                                                 inputImage,inputImage.get_blue_display_band());
      }
      segLevelClassMeanImage.set_rgb_image_stretch(initialParams.rgb_image_stretch,
                                                   initialParams.range[0],initialParams.range[1]);
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

  // Initialize segLevelClassesMapImage
    segLevelClassesMapImage.create(initialParams.seg_level_classes_map_file, view_ncols, view_nrows, 1, 
                                   classLabelsMapImage.get_data_type(),"GTiff");

    if (params.boundary_map_flag)
    {
     // Initialize boundaryMapImage
      if (inputImage.gdal_valid())
      {
#ifdef THREEDIM
        boundaryMapImage.open(initialParams.view_boundary_map_file);
#else
        boundaryMapImage.open(params.boundary_map_file);
#endif
        if (!boundaryMapImage.info_valid())
        {
          cout << "ERROR: " << params.boundary_map_file << " is not a valid boundary map image" << endl;
          return false;
        }
      }
      else
      {
        boundaryMapImage.open(params.boundary_map_file, params.ncols, params.nrows, 1, UInt16);
      }
    // Initialize segLevelBoundaryMapImage
      segLevelBoundaryMapImage.create(initialParams.seg_level_boundary_map_file, view_ncols, view_nrows, 1, 
                                      boundaryMapImage.get_data_type(),"GTiff");
    }

    if (params.object_labels_map_flag)
    {
     // Initialize objectLabelsMapImage
      if (inputImage.gdal_valid())
      {
#ifdef THREEDIM
        objectLabelsMapImage.open(initialParams.view_objects_map_file);
#else
        objectLabelsMapImage.open(params.object_labels_map_file);
#endif
        if (!objectLabelsMapImage.info_valid())
        {
          cout << "ERROR: " << params.object_labels_map_file << " is not a valid object labels map image" << endl;
          return false;
        }
      }
      else
      {
        objectLabelsMapImage.open(params.object_labels_map_file, params.ncols, params.nrows, 1, UInt32);
      }
    // Initialize segLevelObjectsMapImage
      segLevelObjectsMapImage.create(initialParams.seg_level_objects_map_file, view_ncols, view_nrows, 1, 
                                     objectLabelsMapImage.get_data_type(),"GTiff");
    }

    if (params.region_std_dev_flag)
    {
  // Create segLevelClassStdDevImage as a empty image
      segLevelClassStdDevImage.create(initialParams.seg_level_class_std_dev_file, view_ncols, view_nrows, 1, 
                                      GDT_Float32,  "GTiff");
  // Create seglevelObjectStdDevImage as a empty image
      if (params.object_labels_map_flag)
      {
        segLevelObjectStdDevImage.create(initialParams.seg_level_object_std_dev_file, view_ncols, view_nrows, 1, 
                                         GDT_Float32,  "GTiff");
      }
    }

    if (params.region_boundary_npix_flag)
    {
  // Create segLevelClassBPRatioImage as a empty image
      segLevelClassBPRatioImage.create(initialParams.seg_level_class_bpratio_file, view_ncols, view_nrows, 1, 
                                       GDT_Float32,  "GTiff");
  // Create segLevelObjectBPRatioImage as a empty image
      if (params.object_labels_map_flag)
      {
        segLevelObjectBPRatioImage.create(initialParams.seg_level_object_bpratio_file, view_ncols, view_nrows, 1, 
                                          GDT_Float32,  "GTiff");
      }
    }

    if (initialParams.reference1_flag)
    {
    // Initialize reference1Image
      reference1Image.open(initialParams.reference1_file);
      if (!reference1Image.info_valid())
      {
        cout << "ERROR: " << initialParams.reference1_file << " is not a valid reference image (1)" << endl;
        return false;
      }
    }

    if (initialParams.reference2_flag)
    {
    // Initialize reference2Image
      reference2Image.open(initialParams.reference2_file);
      if (!reference2Image.info_valid())
      {
        cout << "ERROR: " << initialParams.reference2_file << " is not a valid reference image (2)" << endl;
        return false;
      }
    }

    if (initialParams.reference3_flag)
    {
    // Initialize reference3Image
      reference3Image.open(initialParams.reference3_file);
      if (!reference3Image.info_valid())
      {
        cout << "ERROR: " << initialParams.reference3_file << " is not a valid reference image (3)" << endl;
        return false;
      }
    }

    if (initialParams.label_in_flag)
    {
    // Initialize labelInImage
      labelInImage.open(initialParams.label_in_file);
      if (!labelInImage.info_valid())
      {
        cout << "ERROR: " << initialParams.label_in_file << " is not a valid input label image" << endl;
        return false;
      }
    // Create labelDataImage as a copy of the labelInImage
      labelDataImage.create_copy(initialParams.label_data_file,labelInImage,"GTiff");
    }
    else
    {
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
    }
    labelDataImage.resize_colormap(NUMBER_OF_LABELS);

  // Create and initialize labelMaskImage
    labelMaskImage.create(initialParams.label_mask_file,labelDataImage,1,GDT_Byte,"GTiff");
    for (view_row = 0; view_row < view_nrows; view_row++)
      for (view_col = 0; view_col < view_ncols; view_col++)
        labelMaskImage.put_data(true,view_col,view_row,0);
    labelMaskImage.flush_data();

  // Create labelOutImage as copy of labelDataImage, but with the format of inputImage
    Image labelOutImage;
    if (inputImage.gdal_valid())
      labelOutImage.create(initialParams.label_out_file,view_ncols,view_nrows,1,GDT_Byte,inputImage.get_driver_description());
    else
    {
      initialParams.label_out_file += ".tif";
      labelOutImage.create(initialParams.label_out_file,view_ncols,view_nrows,1,GDT_Byte,"GTiff");
    }
    if (inputImage.geotransform_valid())
      labelOutImage.set_geotransform(inputImage);
    labelOutImage.registered_data_copy(0,labelDataImage,0);
    labelOutImage.copy_colormap(labelDataImage);
    labelOutImage.close();

    return true;
  }

  void HSegViewer::update_label_data_colormap()
  {
    int index;
    for (index = 0; index < NUMBER_OF_LABELS; index++)
      labelDataImage.set_colormap_value(index,labelRegion.get_red_color(index),labelRegion.get_green_color(index),
                                        labelRegion.get_blue_color(index));
    return;
  }

  void HSegViewer::update_region_label(unsigned char highlight_label)
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

  void HSegViewer::set_region_class_table()
  {
    Glib::ustring strLabel = "Feature Values for all Hierarchical Levels for Region Class "
                             + stringify_int(selClassLabel) + ":";
    featureValuesFrame.set_label(strLabel);

    unsigned int index = 0;
    featureValuesEntries[index++].set_text("Segmentation Level"); 
    featureValuesEntries[index++].set_text("Region Class"); 
    featureValuesEntries[index++].set_text("Area (# pixels)");
    if (params.region_boundary_npix_flag)
      featureValuesEntries[index++].set_text("Boundary Pixel Ratio");
    if (params.region_std_dev_flag)
      featureValuesEntries[index++].set_text("Standard Deviation");
    
    unsigned int new_class_label, npix;
    for (int level = 0; level < oparams.nb_levels; level++)
    {
      index = (level+1)*table_ncols;
      strLabel = stringify_int(level);
      featureValuesEntries[index++].set_text(strLabel);
      new_class_label = statsData.get_new_class_label(selClassLabel,level);
      strLabel = stringify_int(new_class_label);
      featureValuesEntries[index++].set_text(strLabel);
      npix = statsData.get_class_npix(new_class_label,level);
      strLabel = stringify_int(npix);
      featureValuesEntries[index++].set_text(strLabel);
      if (params.region_boundary_npix_flag)
      {
        strLabel = stringify_float(((float) statsData.get_class_boundary_npix(new_class_label,level))/((float) npix));
        featureValuesEntries[index++].set_text(strLabel);
      }
      if (params.region_std_dev_flag)
      {
        strLabel = stringify_float(statsData.get_class_std_dev(new_class_label,level));
        featureValuesEntries[index++].set_text(strLabel);
      }
    }

    return;
  }

  void HSegViewer::set_region_object_table()
  {
    Glib::ustring strLabel = "Feature Values for all Hierarchical Levels for Region Object "
                             + stringify_int(selObjectLabel) + ":";

    featureValuesFrame.set_label(strLabel);

    unsigned int index = 0;
    featureValuesEntries[index++].set_text("Segmentation Level"); 
    featureValuesEntries[index++].set_text("Region Object"); 
    featureValuesEntries[index++].set_text("Area (# pixels)");
    if (params.region_boundary_npix_flag)
      featureValuesEntries[index++].set_text("Boundary Pixel Ratio");
    if (params.region_std_dev_flag)
      featureValuesEntries[index++].set_text("Standard Deviation");

    unsigned int new_object_label, npix;
    for (int level = 0; level < oparams.nb_levels; level++)
    {
      index = (level+1)*table_ncols;
      strLabel = stringify_int(level);
      featureValuesEntries[index++].set_text(strLabel);
      new_object_label = statsData.get_new_object_label(selObjectLabel,level);
      strLabel = stringify_int(new_object_label);
      featureValuesEntries[index++].set_text(strLabel);
      npix = statsData.get_object_npix(new_object_label,level);
      strLabel = stringify_int(npix);
      featureValuesEntries[index++].set_text(strLabel);
      if (params.region_boundary_npix_flag)
      {
        strLabel = stringify_float(((float) statsData.get_object_boundary_npix(new_object_label,level))/((float) npix));
        featureValuesEntries[index++].set_text(strLabel);
      }
      if (params.region_std_dev_flag)
      {
        strLabel = stringify_float(statsData.get_object_std_dev(new_object_label,level));
        featureValuesEntries[index++].set_text(strLabel);
      }
    }

    return;
  }

  void HSegViewer::set_region_class()
  {
    bool label_highlight_flag;

    selClassLabel = statsData.get_new_class_label(selClassLabel,segLevel);
    label_highlight_flag = labelDataDisplay.set_display_highlight(selClassLabel,segLevelClassesMapImage);
    redisplay();
    if (!label_highlight_flag)
    {
      Glib::ustring strMessage = "No pixels highlighted!";
      Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
    
      dialog.run();
      return;
    }

    return;
  }

  void HSegViewer::set_region_object()
  {
    bool label_highlight_flag;

    selObjectLabel = statsData.get_new_object_label(selObjectLabel,segLevel);
    label_highlight_flag = labelDataDisplay.set_display_highlight(selObjectLabel,segLevelObjectsMapImage);
    redisplay();

    if (!label_highlight_flag)
    {
      Glib::ustring strMessage = "No pixels highlighted!";
      Gtk::MessageDialog dialog(strMessage, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
    
      dialog.run();
      return;
    }

    return;
  }

  void HSegViewer::redisplay()
  {
    if ((initialParams.grey_scale_flag) && (oparams.level0_nb_classes < 256))
      statsData.compute_and_set_colormap(segLevel,segLevelClassesMapImage);
    classesSliceDisplay.reinit_seg();
    if (params.region_sum_flag)
      regionMeanDisplay.reinit_rgb();
    labelDataDisplay.reinit_region_label();
    if (params.object_labels_map_flag)
      objectsSliceDisplay.reinit_seg();
    if (params.boundary_map_flag)
      boundaryMapDisplay.reinit_boundary();
    if (params.region_std_dev_flag)
    {
      classStdDevDisplay.reinit_float();
      if (params.object_labels_map_flag)
        objectStdDevDisplay.reinit_float();
    }
    if (params.region_boundary_npix_flag)
    {
      classBPRatioDisplay.reinit_float();
      if (params.object_labels_map_flag)
        objectBPRatioDisplay.reinit_float();
    }

    return;
  }

} // HSEGTilton
