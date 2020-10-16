// shape_ascii_translate.cc

#include "shape_ascii_translate.h"
#include "params/params.h"
#include <image/image.h>
#include <shape/shape.h>
#include <iostream>
#include <gdal_priv.h>

using namespace std;

// Externals
extern CommonTilton::Params params;
extern CommonTilton::Image maskImage;
extern CommonTilton::Shape shapeFile;

namespace CommonTilton
{
 // Forward declaration
  double offset_for_rounding(const double& value);

  bool shape_ascii_translate()
  {
    maskImage.print_info();

    if (shapeFile.get_shapeType() == SHPT_POINT)
      shapeFile.read_point();
    else
      shapeFile.read_polygon();
    int index, size, vertex, nb_vertices, entity, nb_entities = shapeFile.get_nb_entities();
    int part, nb_parts, part_start, part_end;
    vector <int> field_index;
    int col, row, value, nb_sites = 0;
    float UTM_X, UTM_Y, fcol, frow;
    vector<bool> not_in_area_flag;
    if (params.field_names_flag)
    {
      size = params.field_names.size();
      for (index = 0; index < size; index++)
      {
        field_index.push_back(shapeFile.get_fieldIndex(params.field_names[index].c_str()));
      }
    }
    for (entity = 0; entity < nb_entities; entity++)
    {
      not_in_area_flag.push_back(false);
      if (shapeFile.get_shapeType() == SHPT_POINT)
        nb_vertices = 1;
      else
        nb_vertices = shapeFile.get_nb_vertices(entity);
      for (vertex = 0; vertex < nb_vertices; vertex++)
      {
        UTM_X = shapeFile.get_X(entity,vertex);
        UTM_Y = shapeFile.get_Y(entity,vertex);
        fcol = (UTM_X - maskImage.get_X_offset())/maskImage.get_X_gsd();
        frow = (UTM_Y - maskImage.get_Y_offset())/maskImage.get_Y_gsd();
        col = (int) offset_for_rounding(fcol);
        row = (int) offset_for_rounding(frow);
        value = (int) maskImage.get_data(col,row,0);
        if (value == 0)
        {
          not_in_area_flag[entity] = true;
        }
      }
      if (!not_in_area_flag[entity])
        nb_sites++;
    }
    cout << "Shape File " << params.shape_file << " contains " << nb_sites << " sites out of "<< endl;
    cout << nb_entities << " locations within valid locations in mask image " << params.mask_file << endl << endl; 

    ofstream shape_out_fs;
    shape_out_fs.open(params.ascii_shape_file.c_str());
    shape_out_fs << "-nb_sites " << nb_sites << endl;
    nb_sites = 0;
    for (entity = 0; entity < nb_entities; entity++)
    {
      if (!not_in_area_flag[entity])
      {
        if (shapeFile.get_shapeType() == SHPT_POINT)
        {
          nb_vertices = 1;
          nb_parts = 1;
        }
        else
        {
          nb_vertices = shapeFile.get_nb_vertices(entity);
          nb_parts = shapeFile.get_nb_parts(entity);
        }
        nb_sites++;
        if (nb_parts > 1)
          cout << "Site " << nb_sites << " is shape entity " << entity << " with " << nb_parts << " parts" << endl;
        else
          cout << "Site " << nb_sites << " is shape entity " << entity << endl;
        shape_out_fs << endl << "-site " << nb_sites << endl;
        if (params.field_names_flag)
        {
          size = params.field_names.size();
          for (index = 0; index < size; index++)
          {
            shape_out_fs << "-" << params.field_names[index] << " " << shapeFile.read_string_attribute(field_index[index],entity) << endl;
          }
        }
        shape_out_fs << "-nParts " << nb_parts << endl;
        for (part = 0; part < nb_parts; part++)
        {
          part_start = shapeFile.get_part_start(entity,part);
          if (part < (nb_parts - 1))
            part_end = shapeFile.get_part_start(entity,(part+1)) - 1;
          else
            part_end = nb_vertices - 1;
          shape_out_fs << "-UTM_X ";
          for (vertex = part_start; vertex < part_end; vertex++)
          {
            UTM_X = shapeFile.get_X(entity,vertex);
            shape_out_fs << UTM_X << ", ";
          }
          vertex = part_end;
          UTM_X = shapeFile.get_X(entity,vertex);
          shape_out_fs << UTM_X << endl;
        }
        for (part = 0; part < nb_parts; part++)
        {
          part_start = shapeFile.get_part_start(entity,part);
          if (part < (nb_parts - 1))
            part_end = shapeFile.get_part_start(entity,(part+1)) - 1;
          else
            part_end = nb_vertices - 1;
          shape_out_fs << "-UTM_Y ";
          for (vertex = part_start; vertex < part_end; vertex++)
          {
            UTM_Y = shapeFile.get_Y(entity,vertex);
            shape_out_fs << UTM_Y << ", ";
          }
          vertex = part_end;
          UTM_Y = shapeFile.get_Y(entity,vertex);
          shape_out_fs << UTM_Y << endl;
        }
        for (part = 0; part < nb_parts; part++)
        {
          part_start = shapeFile.get_part_start(entity,part);
          if (part < (nb_parts - 1))
            part_end = shapeFile.get_part_start(entity,(part+1)) - 1;
          else
            part_end = nb_vertices - 1;
          shape_out_fs << "-col ";
          for (vertex = part_start; vertex < part_end; vertex++)
          {
            UTM_X = shapeFile.get_X(entity,vertex);
            fcol = (UTM_X - maskImage.get_X_offset())/maskImage.get_X_gsd();
            col = (int) offset_for_rounding(fcol);
            shape_out_fs << col << ", ";
          }
          vertex = part_end;
          UTM_X = shapeFile.get_X(entity,vertex);
          fcol = (UTM_X - maskImage.get_X_offset())/maskImage.get_X_gsd();
          col = (int) offset_for_rounding(fcol);
          shape_out_fs << col << endl;
        }
        for (part = 0; part < nb_parts; part++)
        {
          part_start = shapeFile.get_part_start(entity,part);
          if (part < (nb_parts - 1))
            part_end = shapeFile.get_part_start(entity,(part+1)) - 1;
          else
            part_end = nb_vertices - 1;
          shape_out_fs << "-row ";
          for (vertex = part_start; vertex < part_end; vertex++)
          {
            UTM_Y = shapeFile.get_Y(entity,vertex);
            frow = (UTM_Y - maskImage.get_Y_offset())/maskImage.get_Y_gsd();
            row = (int) offset_for_rounding(frow);
            shape_out_fs << row << ", ";
          }
          vertex = part_end;
          UTM_Y = shapeFile.get_Y(entity,vertex);
          frow = (UTM_Y - maskImage.get_Y_offset())/maskImage.get_Y_gsd();
          row = (int) offset_for_rounding(frow);
          shape_out_fs << row << endl;
        }
      }
    }

    maskImage.close();
    shapeFile.close();
    shape_out_fs.close();

    return true;
  }

  double offset_for_rounding(const double& value)
  {
     if (value >= -0.5)
       return (value + 0.5);
     else
       return (value - 0.5);
  }

} // CommonTilton
