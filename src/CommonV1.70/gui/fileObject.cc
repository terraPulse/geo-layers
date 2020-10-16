// fileObject.cc
#include "fileObject.h"
#include <iostream>

namespace CommonTilton
{
  // Constructors
  FileObject::FileObject(const Glib::ustring& label, const Glib::ustring& title, 
                         const Gtk::FileChooserAction action)
  {
    initialize(label,title,action,true,"");
  }

  FileObject::FileObject(const Glib::ustring& label, const Glib::ustring& title, 
                         const Gtk::FileChooserAction action, const bool requiredFlag)
  {
    initialize(label,title,action,requiredFlag,"");
  }

  FileObject::FileObject(const Glib::ustring& label, const Glib::ustring& title, 
                         const Gtk::FileChooserAction action, const bool requiredFlag,
                         const Glib::ustring& fileName)
  {
    initialize(label,title,action,requiredFlag,fileName);
  }

  // Destructor...
  FileObject::~FileObject() 
  {
  }

  void FileObject::initialize(const Glib::ustring& label, const Glib::ustring& title, 
                         const Gtk::FileChooserAction action, const bool requiredFlag,
                         const Glib::ustring& fileName)
  {
    activeFlag = requiredFlag;
    dialogAction = action;
    dialogTitle = title;

    add(vBox);
    
    if (requiredFlag)
    {
      fileLabel.set_label(label);
      fileLabel.set_alignment(0.0,0.5);
      vBox.pack_start(fileLabel);
    }
    else
    {
      activeButton.set_label(label);
      activeButton.set_active(activeFlag);
      activeButton.set_alignment(0.0,0.5);
      activeButton.signal_clicked().connect(sigc::mem_fun(*this,
                   &FileObject::on_activeButton_checked) );
      vBox.pack_start(activeButton);
    }
    if (fileName == "")
    {
      fileDialogButton.set_label("(Select File)");
      validFlag = false;
    }
    else
    {
      fileDialogButton.set_label(fileName);
      validFlag = true;
    }
    fileDialogButton.set_alignment(0.0,0.5);
    fileDialogButton.signal_clicked().connect( sigc::mem_fun(*this,
                                      &FileObject::on_fileDialogButton_clicked) );
    vBox.pack_start(fileDialogButton);
  }

  FileObject::type_signal_selection_changed FileObject::signal_selection_changed()
  {
    return m_signal_selection_changed;
  }

  FileObject::type_signal_activated FileObject::signal_activated()
  {
    return m_signal_activated;
  }

  void FileObject::on_fileDialogButton_clicked()
  {
    Gtk::FileChooserDialog dialog(dialogTitle,dialogAction);

    if (dialogAction == Gtk::FILE_CHOOSER_ACTION_OPEN)
      dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
    else if (dialogAction == Gtk::FILE_CHOOSER_ACTION_SAVE) 
      dialog.add_button("Create or Rewrite", Gtk::RESPONSE_OK);
    else
      dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    if (current_folder != "")
      dialog.set_current_folder(current_folder);
    if (shortcut_folder != "")
      dialog.add_shortcut_folder(shortcut_folder);

    int result = dialog.run();

    //Handle the response:
    switch(result)
    {
      case(Gtk::RESPONSE_ACCEPT):
      {
        validFlag = true;
        fileDialogButton.set_label(dialog.get_filename());
        m_signal_selection_changed.emit();
        break;
      }
      case(Gtk::RESPONSE_OK):
      {
        validFlag = true;
        fileDialogButton.set_label(dialog.get_filename());
        break;
      }
      case(Gtk::RESPONSE_CANCEL):
      {
//        std::cout << "Cancel clicked." << std::endl;
        break;
      }
      default:
      {
//        std::cout << "Unexpected button clicked." << std::endl;
        break;
      }
    }
  }

  void FileObject::on_activeButton_checked()
  {
    activeFlag = activeButton.get_active();
    if (activeFlag)
    {
      fileDialogButton.show();
      m_signal_activated.emit();
    }
    else
      fileDialogButton.hide();
  }

  void FileObject::set_activeFlag(const bool value)
  {
    activeFlag = value;
    activeButton.set_active(activeFlag);
    if (activeFlag)
      fileDialogButton.show();
    else
      fileDialogButton.hide();
  }

  Glib::ustring FileObject::get_filename()
  {
    return fileDialogButton.get_label();
  }

  void FileObject::set_filename(const Glib::ustring& fileName)
  {
    if (fileName == "")
    {
      validFlag = false;
      fileDialogButton.set_label("(Select File)");
    }
    else
    {
      validFlag = true;
      fileDialogButton.set_label(fileName);
    }
  }

  void FileObject::set_current_folder(const Glib::ustring& folder)
  {
    current_folder = folder;
  }

  void FileObject::add_shortcut_folder(const Glib::ustring& folder)
  {
    shortcut_folder = folder;
  }


} // namespace CommonTilton

