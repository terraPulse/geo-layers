// labelregion.cc

#include "labelregion.h"
#include <params/params.h> // for stringify_int
#include "../params/initialParams.h"
#include <iostream>

extern HSEGTilton::InitialParams initialParams;

namespace HSEGTilton
{

  LabelRegion::LabelRegion():
               colorTable((NUMBER_OF_LABELS/3)+1,6,false)
  {
    set_title("Label Region Panel");
    add(vBox); // put a MenuBar at the top of the box and other stuff below it.

    //Create actions for menus:
    m_refActionGroup = Gtk::ActionGroup::create();
 
    //Action menu:
    m_refActionGroup->add(Gtk::Action::create("ActionMenu", "Actions"));

    //Help Sub-menu:
    m_refActionGroup->add( Gtk::Action::create("Help", Gtk::Stock::HELP),
            sigc::mem_fun(*this, &LabelRegion::on_menu_others) );
    m_refActionGroup->add(Gtk::Action::create("Undo", "Undo Last Labeling"),
            sigc::mem_fun(*this, &LabelRegion::on_menu_undo));

    m_refActionGroup->add(Gtk::Action::create("Close", Gtk::Stock::CLOSE),
            sigc::mem_fun(*this, &LabelRegion::on_menu_close));

    m_refUIManager = Gtk::UIManager::create();
    m_refUIManager->insert_action_group(m_refActionGroup);

    add_accel_group(m_refUIManager->get_accel_group());

    //Layout the actions in a menubar:
    Glib::ustring ui_info = 
          "<ui>"
          "  <menubar name='MenuBar'>"
          "    <menu action='ActionMenu'>"
          "      <menuitem action='Help'/>"
          "      <separator/>"
          "      <menuitem action='Undo'/>"
          "      <separator/>"
          "      <menuitem action='Close'/>"
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

    unsigned char index;

    // Set up rest of GUI

    for (index = 0; index < 3; index++)
    {
      colorsLabel[index].set_label("colors:");
      colorsLabel[index].set_alignment(0.0);
      namesLabel[index].set_label("names:");
      namesLabel[index].set_alignment(0.0);
#ifdef ARTEMIS
      namesLabel[index].set_size_request(210);
#else
      namesLabel[index].set_width_chars(30);
#endif
    }

    write_ascii_out();

    colorTable.attach(colorsLabel[0],0,1,0,1);
    colorTable.attach(namesLabel[0],1,2,0,1);
    colorTable.attach(colorsLabel[1],2,3,0,1);
    colorTable.attach(namesLabel[1],3,4,0,1);
    colorTable.attach(colorsLabel[2],4,5,0,1);
    colorTable.attach(namesLabel[2],5,6,0,1);
    colorTable.attach(colorButton[HIGHLIGHT_COLOR],0,1,1,2);
    colorTable.attach(entry[HIGHLIGHT_COLOR],1,2,1,2);
    colorTable.attach(colorButton[BAD_DATA_QUALITY_LABEL],2,3,1,2);
    colorTable.attach(entry[BAD_DATA_QUALITY_LABEL],3,4,1,2);
    colorTable.attach(colorButton[0],4,5,1,2);
    colorTable.attach(entry[0],5,6,1,2);
    unsigned int top_attach, bottom_attach;
    for (index = 1; index < HIGHLIGHT_COLOR; index += 3)
    {
      top_attach = index/3 + 2;
      bottom_attach = top_attach + 1;
      colorTable.attach(colorButton[index],0,1,top_attach,bottom_attach);
      colorTable.attach(entry[index],1,2,top_attach,bottom_attach);
      colorTable.attach(colorButton[index+1],2,3,top_attach,bottom_attach);
      colorTable.attach(entry[index+1],3,4,top_attach,bottom_attach);
      colorTable.attach(colorButton[index+2],4,5,top_attach,bottom_attach);
      colorTable.attach(entry[index+2],5,6,top_attach,bottom_attach);
    }
    colorTable.set_col_spacings(10);
    colorTable.set_row_spacings(5);

    vBox.pack_start(colorTable);

  // Set up other signals
    for (index = 0; index < NUMBER_OF_LABELS; index++)
    {
      colorButton[index].signal_color_set().connect(sigc::mem_fun(*this,
                                     &LabelRegion::on_color_set) );
    }
    for (index = 1; index < HIGHLIGHT_COLOR; index++)
    {
      entry[index].signal_activate().connect(sigc::bind<unsigned char>
                                     (sigc::mem_fun(*this,&LabelRegion::on_entry_activate),index));
    }

  // Set editable characteristic
    entry[0].set_editable(false);
    entry[HIGHLIGHT_COLOR].set_editable(false);
    entry[BAD_DATA_QUALITY_LABEL].set_editable(false);

    show_all_children();
    return;
  }

  LabelRegion::~LabelRegion()
  {
  }

  void LabelRegion::on_menu_undo()
  {
    m_signal_undo.emit();
  }

  LabelRegion::type_signal_undo LabelRegion::signal_undo()
  {
    return m_signal_undo;
  }

  void LabelRegion::on_menu_close()
  {
    hide();
  }

  void LabelRegion::on_menu_others()
  {
    cout << "A menu item was selected." << endl;
  }

  void LabelRegion::set_text_and_colormap()
  {
    short unsigned int index, red_value, green_value, blue_value;
    string text, tmp_text;
    if (initialParams.ascii_in_flag)
    {
      ifstream ascii_in_fs;
      ascii_in_fs.open(initialParams.ascii_in_file.c_str());
      for (index = 0; index < NUMBER_OF_LABELS; index++)
      {
        ascii_in_fs >> text;
        while (text.rfind("\"") == 0)
        {
          ascii_in_fs >> tmp_text;
          text = text + " " + tmp_text;
        }
        text = text.substr(1,text.length()-2);
        entry[index].set_text(text);
      }
      for (index = 0; index < NUMBER_OF_LABELS; index++)
      {
        ascii_in_fs >> red_value;
        ascii_in_fs >> green_value;
        ascii_in_fs >> blue_value;
        if (red_value > 0)
          red_value = ((red_value+1)*256) - 1;
        if (blue_value > 0)
          blue_value = ((blue_value+1)*256) - 1;
        if (green_value > 0)
          green_value = ((green_value+1)*256) - 1;
        color[index].set_rgb(red_value,green_value,blue_value);
        colorButton[index].set_color(color[index]);
      }
      ascii_in_fs.close();
    }
    else
    {
      index = 0;
      text = "0: Unlabeled Areas";
      entry[index].set_text(text);
      text = stringify_int(HIGHLIGHT_COLOR) + ": Region to be Labeled";
      entry[HIGHLIGHT_COLOR].set_text(text);
      text = stringify_int(BAD_DATA_QUALITY_LABEL) + ": Masked out in input data";
      entry[BAD_DATA_QUALITY_LABEL].set_text(text);
      for (index = 1; index < HIGHLIGHT_COLOR; index++)
      {
        text = stringify_int((int) index) + ": ";
        entry[index].set_text(text);
      }
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
            colorButton[index].set_color(color[index]);
            index++;
          }
      index = 21;
      red_value = 64; green_value = 127; blue_value = 191;
      red_value = ((red_value+1)*256) - 1;
      blue_value = ((blue_value+1)*256) - 1;
      green_value = ((green_value+1)*256) - 1;
      color[index].set_rgb(red_value,green_value,blue_value);
      colorButton[index].set_color(color[index]);
      index = 63;
      red_value = 127; green_value = 63; blue_value = 191;
      red_value = ((red_value+1)*256) - 1;
      blue_value = ((blue_value+1)*256) - 1;
      green_value = ((green_value+1)*256) - 1;
      color[index].set_rgb(red_value,green_value,blue_value);
      colorButton[index].set_color(color[index]);
      index = 64;
      red_value = 63; green_value = 191; blue_value = 127;
      red_value = ((red_value+1)*256) - 1;
      blue_value = ((blue_value+1)*256) - 1;
      green_value = ((green_value+1)*256) - 1;
      color[index].set_rgb(red_value,green_value,blue_value);
      colorButton[index].set_color(color[index]);
      index = 65;
      red_value = 127; green_value = 191; blue_value = 63;
      red_value = ((red_value+1)*256) - 1;
      blue_value = ((blue_value+1)*256) - 1;
      green_value = ((green_value+1)*256) - 1;
      color[index].set_rgb(red_value,green_value,blue_value);
      colorButton[index].set_color(color[index]);
      index = 66;
      red_value = 191; green_value = 63; blue_value = 127;
      red_value = ((red_value+1)*256) - 1;
      blue_value = ((blue_value+1)*256) - 1;
      green_value = ((green_value+1)*256) - 1;
      color[index].set_rgb(red_value,green_value,blue_value);
      colorButton[index].set_color(color[index]);
      index = HIGHLIGHT_COLOR;
      red_value = 255; green_value = 255; blue_value = 255;
      red_value = ((red_value+1)*256) - 1;
      blue_value = ((blue_value+1)*256) - 1;
      green_value = ((green_value+1)*256) - 1;
      color[index].set_rgb(red_value,green_value,blue_value);
      colorButton[index].set_color(color[index]);
      index = BAD_DATA_QUALITY_LABEL;
      red_value = 195; green_value = 195; blue_value = 195;
      red_value = ((red_value+1)*256) - 1;
      blue_value = ((blue_value+1)*256) - 1;
      green_value = ((green_value+1)*256) - 1;
      color[index].set_rgb(red_value,green_value,blue_value);
      colorButton[index].set_color(color[index]);
    }

    return;
  }

  unsigned int LabelRegion::get_red_color(const int& index)
  {
    unsigned int red_value;

    red_value = color[index].get_red();
    if (red_value > 0)
      red_value = ((red_value+1)/256) - 1;
 
    return red_value;
  }

  unsigned int LabelRegion::get_green_color(const int& index)
  {
    unsigned int green_value;

    green_value = color[index].get_green();
    if (green_value > 0)
      green_value = ((green_value+1)/256) - 1;
 
    return green_value;
  }

  unsigned int LabelRegion::get_blue_color(const int& index)
  {
    unsigned int blue_value;

    blue_value = color[index].get_blue();
    if (blue_value > 0)
      blue_value = ((blue_value+1)/256) - 1;
 
    return blue_value;
  }

  void LabelRegion::write_ascii_out()
  {
    unsigned char index;
    unsigned int red_value, green_value, blue_value;
    ofstream ascii_out_fs;

    ascii_out_fs.open(initialParams.ascii_out_file.c_str());
    for (index = 0; index < NUMBER_OF_LABELS; index++)
      ascii_out_fs << "\"" << entry[index].get_text()<< "\"" << endl;
    for (index = 0; index < NUMBER_OF_LABELS; index++)
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
      ascii_out_fs << red_value << " ";
      ascii_out_fs << green_value << " ";
      ascii_out_fs << blue_value << endl;
    }
    ascii_out_fs.close();
  }

  void LabelRegion::on_entry_activate(unsigned char index)
  {
    highlight_label = index;
    write_ascii_out();
    m_signal_activated.emit();
    return;
  }

  LabelRegion::type_signal_activated LabelRegion::signal_activated()
  {
    return m_signal_activated;
  }

  void LabelRegion::on_color_set()
  {
    unsigned char index;

    for (index = 0; index < NUMBER_OF_LABELS; index++)
      color[index] = colorButton[index].get_color();
    write_ascii_out();
    m_signal_color_set.emit();
    return;
  }

  LabelRegion::type_signal_color_set LabelRegion::signal_color_set()
  {
    return m_signal_color_set;
  }

} // namespace HSEGTilton
