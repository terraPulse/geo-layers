// numberListObject.cc
#include "numberListObject.h"
#include <string>

using namespace std;

namespace CommonTilton
{
  // Constructors
  NumberListObject::NumberListObject(const Glib::ustring& label)
  {
    initialize(label,true);
  }

  NumberListObject::NumberListObject(const Glib::ustring& label, const bool requiredFlag)
  {
    initialize(label,requiredFlag);
  }

  // Destructor...
  NumberListObject::~NumberListObject() 
  {
  }

  void NumberListObject::initialize(const Glib::ustring& label, const bool requiredFlag)
  {
    activeFlag = requiredFlag;

    add(vBox);
    
    if (requiredFlag)
    {
      numberListLabel.set_label(label);
      numberListLabel.set_alignment(0.0,0.5);
      vBox.pack_start(numberListLabel);
    }
    else
    {
      activeButton.set_label(label);
      activeButton.set_active(activeFlag);
      activeButton.set_alignment(0.0,0.5);
      activeButton.signal_clicked().connect(sigc::mem_fun(*this,
                   &NumberListObject::on_activeButton_checked) );
      vBox.pack_start(activeButton);
    }
    vBox.pack_start(numberListEntry);
  }

  NumberListObject::type_signal_activated NumberListObject::signal_activated()
  {
    return m_signal_activated;
  }

  void NumberListObject::on_activeButton_checked()
  {

    activeFlag = activeButton.get_active();
    if (activeFlag)
    {
      numberListEntry.show();
      m_signal_activated.emit();
    }
    else
      numberListEntry.hide();
  }

  void NumberListObject::set_activeFlag(const bool value)
  {
    activeFlag = value;
    activeButton.set_active(activeFlag);
    if (activeFlag)
      numberListEntry.show();
    else
      numberListEntry.hide();
  }

  unsigned short int NumberListObject::get_size()
  {
    string tmp_string, sub_string;
    unsigned short int index;
    int sub_pos;
   
    tmp_string = numberListEntry.get_text();
    index = 0;
    while (tmp_string.size() > 0)
    {
      sub_pos = tmp_string.find_first_of(",");
      if (sub_pos > 0)
      {
        sub_string = tmp_string.substr(0,sub_pos);
        tmp_string = tmp_string.substr(sub_pos+1);
      }
      else
      {
        sub_string = tmp_string;
        tmp_string = "";
      }
      index++;
    }
    return index;
  }

  int NumberListObject::get_entry(const int entry_index)
  {
    string tmp_string, sub_string;
    unsigned short int index;
    int sub_pos;
   
    if ((entry_index >= 0) && (entry_index < get_size()))
    {
      tmp_string = numberListEntry.get_text();
      for (index = 0; index <= entry_index; index++)
      {
        sub_pos = tmp_string.find_first_of(",");
        if (sub_pos > 0)
        {
          sub_string = tmp_string.substr(0,sub_pos);
          tmp_string = tmp_string.substr(sub_pos+1);
        }
        else
        {
          sub_string = tmp_string;
          tmp_string = "";
        }
      }
      return atoi(sub_string.c_str());
    }
    else
      return 0;
  }

  float NumberListObject::get_fentry(const int entry_index)
  {
    string tmp_string, sub_string;
    unsigned short int index;
    int sub_pos;
   
    if ((entry_index >= 0) && (entry_index < get_size()))
    {
      tmp_string = numberListEntry.get_text();
      for (index = 0; index <= entry_index; index++)
      {
        sub_pos = tmp_string.find_first_of(",");
        if (sub_pos > 0)
        {
          sub_string = tmp_string.substr(0,sub_pos);
          tmp_string = tmp_string.substr(sub_pos+1);
        }
        else
        {
          sub_string = tmp_string;
          tmp_string = "";
        }
      }
      return atof(sub_string.c_str());
    }
    else
      return 0;
  }

  void NumberListObject::clear()
  {
    numberListEntry.set_text("");
  }


} // namespace CommonTilton
