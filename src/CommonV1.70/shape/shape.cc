// shape.cc

#include "shape.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>

namespace CommonTilton
{

 // Basic constructor
  Shape::Shape()
  {
    shape_flag = false;
    nb_entities = 0;
    nb_vertices = NULL;

    X = NULL;
    Y = NULL;
    XY_offset = NULL;

#ifdef SHAPEFILE
    nb_records = 0;
#endif

    return;
  }

 // Destructor...
  Shape::~Shape() 
  {
  }

#ifdef SHAPEFILE
 // Open shape data
  void Shape::open(const string& file_name)
  {
    shape_flag = false;

    hSHP = SHPOpen(file_name.c_str(),"rb");
    SHPGetInfo(hSHP, &nb_entities, &shapeType, &minBound[0], &maxBound[0]);
//cout << "shapeType = " << shapeType << endl;
    hDBF = DBFOpen(file_name.c_str(),"rb");
    nb_records = DBFGetRecordCount(hDBF);
    if (nb_records != nb_entities)
    {
      cout << "ERROR(shape): Number of entities/records mismatch between Shape file and DBF file" << endl;
      return;
    }

    shape_flag = true;

    return;
  }

 // Print out available field names
  void Shape::print_fieldNames()
  {
    int index, field_count = DBFGetFieldCount(hDBF);
    int width, decimals;
    DBFFieldType fieldType;
    char *fieldName;
    fieldName = (char *) malloc(12*sizeof(char));

    for (index = 0; index < field_count; index++)
    {
      fieldType = DBFGetFieldInfo(hDBF,index,fieldName,&width,&decimals);
      cout << "Field Name = " << fieldName;
      switch(fieldType)
      {
        case FTString:  cout << ", Field Type = FTString";
                        break;
        case FTInteger: cout << ", Field Type = FTInteger";
                        break;
        case FTDouble:  cout << ", Field Type = FTDouble";
                        break;
        case FTLogical: cout << ", Field Type = FTLogical";
                        break;
        case FTInvalid: cout << ", Field Type = FTInvalid";
                        break;
        default:        cout << ", Unknown Field Type";
                        break;
      }
      cout << ", Field width = " << width << " ";
      cout << ", Field precision = " << decimals;
      cout << endl;
    }

    if (field_count == 1)
    {
      fieldIndex = DBFGetFieldIndex(hDBF,fieldName);
      cout << "Field Index = " << fieldIndex << endl;
    }
    
    return;
  }

 // Set and get fieldIndex based on field_name
  int Shape::get_fieldIndex(const string& field_name)
  {
    const char *fieldName;
    int length = field_name.size() + 1;
    fieldName = (char *) malloc(length*sizeof(char));
    fieldName = field_name.c_str();
    fieldIndex = DBFGetFieldIndex(hDBF,fieldName);
    
    return fieldIndex;
  }

 // Read shape data of SHPT_POINT type for a particular Site Type
  void Shape::read_point(const string& site_type)
  {
    SHPObject *shpObject;
    int record;

    if (shapeType != SHPT_POINT)
    {
      cout << "ERROR: Use read_point only for Point shapes" << endl;
      return;
    }

    nb_vertices = new int[nb_entities];
    X = new double[nb_entities];
    Y = new double[nb_entities];
    XY_offset = new int[nb_entities];
    XY_size = nb_entities;
    nb_entities = 0;
    for (record = 0; record < nb_records; record++)
    {
      if (strcmp(DBFReadStringAttribute(hDBF,record,fieldIndex),site_type.c_str()) == 0)
      {
        shpObject = SHPReadObject(hSHP, record);
        nb_vertices[nb_entities] = 1;
        X[nb_entities] = shpObject->padfX[0];
        Y[nb_entities] = shpObject->padfY[0];
        XY_offset[nb_entities] = nb_entities;
        nb_entities++;
      }
    }
    data_flag = true;

    return;
  }

 // Read shape data of SHPT_POINT type (for any Site Type)
  void Shape::read_point()
  {
    SHPObject *shpObject;
    int record;

    if (shapeType != SHPT_POINT)
    {
      cout << "ERROR: Use read_point only for Point shapes" << endl;
      return;
    }

    nb_vertices = new int[nb_entities];
    X = new double[nb_entities];
    Y = new double[nb_entities];
    XY_offset = new int[nb_entities];
    XY_size = nb_entities;
    for (record = 0; record < nb_records; record++)
    {
      shpObject = SHPReadObject(hSHP, record);
      nb_vertices[record] = 1;
      X[record] = shpObject->padfX[0];
      Y[record] = shpObject->padfY[0];
      XY_offset[record] = record;
    }
    data_flag = true;

    return;
  }

 // Read shape data of SHPT_ARC type
  void Shape::read_arc()
  {
    SHPObject *shpObject;
    int entity;

    if (shapeType != SHPT_ARC)
    {
      cout << "ERROR: Use read_arc only for Arc shapes" << endl;
      return;
    }

    nb_vertices = new int[nb_entities];
    XY_offset = new int[nb_entities];
    XY_size = 0;
    nb_parts = new int[nb_entities];
    part_start_offset = new int[nb_entities];
    part_size = 0;
    for (entity = 0; entity < nb_entities; entity++)
    {
      shpObject = SHPReadObject(hSHP, entity);
      XY_offset[entity] = XY_size;
      XY_size += shpObject->nVertices;
      part_start_offset[entity] = part_size;
      if (shpObject->nParts > 0)
        part_size += shpObject->nParts;
      else
        part_size++;
    }

    X = new double[XY_size];
    Y = new double[XY_size];
    part_start = new int[part_size];

    int index;
    for (entity = 0; entity < nb_entities; entity++)
    {
      shpObject = SHPReadObject(hSHP, entity);
      nb_vertices[entity] = shpObject->nVertices;
      for (index = 0; index < nb_vertices[entity]; index++)
      {
        X[XY_offset[entity]+index] = shpObject->padfX[index];
        Y[XY_offset[entity]+index] = shpObject->padfY[index];
      }
      nb_parts[entity] = shpObject->nParts;
      if (nb_parts[entity] == 0)
      {
        nb_parts[entity] = 1;
        part_start[part_start_offset[entity]] = 0;
      }
      else
      {
        for (index = 0; index < nb_parts[entity]; index++)
        {
          part_start[part_start_offset[entity]+index] = shpObject->panPartStart[index];
        }
      }
    }

    data_flag = true;

    return;
  }

 // Read shape data of SHPT_POLYGON or SHPT_POLYGONZ type (ignores the "measure" for the SHPT_POLYGONZ type)
  void Shape::read_polygon()
  {
    SHPObject *shpObject;
    int entity;

    if ((shapeType != SHPT_POLYGON) && (shapeType != SHPT_POLYGONZ))
    {
      cout << "ERROR: Use read_arc only for Polygon or PolygonZ shapes" << endl;
      return;
    }

    nb_vertices = new int[nb_entities];
    XY_offset = new int[nb_entities];
    XY_size = 0;
    nb_parts = new int[nb_entities];
    part_start_offset = new int[nb_entities];
    part_size = 0;
    for (entity = 0; entity < nb_entities; entity++)
    {
      shpObject = SHPReadObject(hSHP, entity);
      XY_offset[entity] = XY_size;
      XY_size += shpObject->nVertices;
      part_start_offset[entity] = part_size;
      if (shpObject->nParts > 0)
        part_size += shpObject->nParts;
      else
        part_size++;
    }

    X = new double[XY_size];
    Y = new double[XY_size];
    part_start = new int[part_size];

    int index;
    for (entity = 0; entity < nb_entities; entity++)
    {
      shpObject = SHPReadObject(hSHP, entity);
      nb_vertices[entity] = shpObject->nVertices;
      for (index = 0; index < nb_vertices[entity]; index++)
      {
        X[XY_offset[entity]+index] = shpObject->padfX[index];
        Y[XY_offset[entity]+index] = shpObject->padfY[index];
      }
      nb_parts[entity] = shpObject->nParts;
      if (nb_parts[entity] == 0)
      {
        nb_parts[entity] = 1;
        part_start[part_start_offset[entity]] = 0;
      }
      else
      {
        for (index = 0; index < nb_parts[entity]; index++)
        {
          part_start[part_start_offset[entity]+index] = shpObject->panPartStart[index];
        }
      }
    }

    data_flag = true;

    return;
  }

  void Shape::get_Bounds(const int& entity, double& X_minBound, double& Y_minBound, 
                         double& X_maxBound, double& Y_maxBound)
  {
    SHPObject *shpObject = SHPReadObject(hSHP, entity);

    X_minBound = shpObject->dfXMin;
    Y_minBound = shpObject->dfYMin;
    X_maxBound = shpObject->dfXMax;
    Y_maxBound = shpObject->dfYMax;

    return;
  }

  int Shape::get_nParts(const int& entity)
  {
    SHPObject *shpObject = SHPReadObject(hSHP, entity);

    return shpObject->nParts;
  }

  void Shape::get_Vertices(const int& entity, const int& part, vector<dPoint>& vertices)
  {
    int vertex, vertex_start, vertex_end;
    SHPObject *shpObject = SHPReadObject(hSHP, entity);

    vertex_start = 0;
    vertex_end = shpObject->nVertices;
    if (shpObject->nParts > 1)
    {
      vertex_start = shpObject->panPartStart[part];
      if (part == (shpObject->nParts - 1))
        vertex_end = shpObject->nVertices;
      else
        vertex_end = shpObject->panPartStart[part+1];
    }

    vertices.clear();
    for (vertex = vertex_start; vertex < vertex_end; vertex++)
    {
      dPoint new_point(shpObject->padfX[vertex], shpObject->padfY[vertex]);
      vertices.push_back(new_point);
    }
      
    return;
  }

  double Shape::read_double_attribute(const string& field_name, const int& entity)
  {
    return DBFReadDoubleAttribute(hDBF,entity,get_fieldIndex(field_name));
  }

  string Shape::read_string_attribute(const int& field_index, const int& entity)
  {
    return DBFReadStringAttribute(hDBF,entity,field_index);
  }

 // Close shape data file
  void Shape::close()
  {
    if (shape_flag)
    {
      SHPClose(hSHP);
      DBFClose(hDBF);
    }

    return;
  }

 // Print information obtained from SHPGetInfo()
  void Shape::print_shape_info()
  {
    cout << endl << "Number of Entities = " << nb_entities << endl;

    switch (shapeType)
    {
      case SHPT_POINT:      cout << "Shape Type = SHPT_POINT" << endl;
                            break;
      case SHPT_ARC:        cout << "Shape Type = SHPT_ARC" << endl;
                            break;
      case SHPT_POLYGON:    cout << "Shape Type = SHPT_POLYGON" << endl;
                            break;
      case SHPT_MULTIPOINT: cout << "Shape Type = SHPT_MULTIPOINT" << endl;
                            break;
      case SHPT_POLYGONZ:   cout << "Shape Type = SHPT_POLYGONZ" << endl;
                            break;
      default:              cout << "Unknown Shape Type" << endl;
                            break;
    }

    int index;
    cout.precision(12);
    cout << "minBound: ";
    for (index = 0; index < 4; index++)
      cout << minBound[index] << " ";
    cout << endl;
    cout << "maxBound: ";
    for (index = 0; index < 4; index++)
      cout << maxBound[index] << " ";
    cout << endl;
    cout.precision(6); // Default precision is 6.

    return;
  }

 // Print information obtained from DBFGetFieldInfo()
  void Shape::print_field_info()
  {
    int index, field_count = DBFGetFieldCount(hDBF);
    int width, decimals;
    DBFFieldType fieldType;
    char *fieldName;
    fieldName = (char *) malloc(12*sizeof(char));

    cout << endl << "Number of Fields = " << field_count << endl;

    for (index = 0; index < field_count; index++)
    {
      fieldType = DBFGetFieldInfo(hDBF,index,fieldName,&width,&decimals);
      cout << "For Field Number " << index << ", Field Name = " << fieldName;
      switch(fieldType)
      {
        case FTString:  cout << ", Field Type = FTString";
                        break;
        case FTInteger: cout << ", Field Type = FTInteger";
                        break;
        case FTDouble:  cout << ", Field Type = FTDouble";
                        break;
        case FTLogical: cout << ", Field Type = FTLogical";
                        break;
        case FTInvalid: cout << ", Field Type = FTInvalid";
                        break;
        default:        cout << ", Unknown Field Type";
                        break;
      }
      cout << ", Field width = " << width << " ";
      cout << ", Field precision = " << decimals;
      cout << endl;
    }

    return;
  }

 // Print point data
  void Shape::print_point()
  {
    if (shapeType != SHPT_POINT)
    {
      cout << "ERROR: Use print_point only for Point shapes" << endl;
      return;
    }

    SHPObject *shpObject;
    int entity, field, field_count = DBFGetFieldCount(hDBF);
    int width, decimals;
    DBFFieldType fieldType;
    char *fieldName;
    fieldName = (char *) malloc(12*sizeof(char));

    cout << endl << "Printing point data for " << nb_entities << " entities" << endl;

    cout.precision(12);
    for (entity = 0; entity < nb_entities; entity++)
    {
      cout << endl << "Entity " << entity << ":" << endl;
      shpObject = SHPReadObject(hSHP, entity);
      cout << "X = " << shpObject->padfX[0] << " and Y = " << shpObject->padfY[0] << endl;
      for (field = 0; field < field_count; field++)
      {
        fieldType = DBFGetFieldInfo(hDBF,field,fieldName,&width,&decimals);
        cout << "For field " << field << " with Name " << fieldName << ", value = ";
        switch(fieldType)
        {
          case FTString:  cout << DBFReadStringAttribute(hDBF,entity,field) << endl;
                          break;
          case FTInteger: cout << DBFReadIntegerAttribute(hDBF,entity,field) << endl;
                          break;
          case FTDouble:  cout << DBFReadDoubleAttribute(hDBF,entity,field) << endl;
                          break;
          case FTLogical: if (DBFReadIntegerAttribute(hDBF,entity,field))
                            cout << "true" << endl;
                          else
                            cout << "false" << endl;
                          break;
          case FTInvalid: cout << " (Invalid field)";
                          break;
          default:        cout << " (Unknown Field Type)";
                          break;
        }
      }
    }
    cout.precision(6);

    return;
  }

 // Print arc or polygon data
  void Shape::print_arc_or_polygon()
  {
    if ((shapeType != SHPT_ARC) && (shapeType != SHPT_POLYGON) && (shapeType != SHPT_POLYGONZ) && 
        (shapeType != SHPT_MULTIPOINT))
    {
      cout << "ERROR: Use print_arc only for Arc, Polygon, PolygonZ or Multipoint shapes" << endl;
      return;
    }

    SHPObject *shpObject;
    int entity, vertex, part, nb_vertices, nb_parts, field, field_count = DBFGetFieldCount(hDBF);
    int width, decimals;
    DBFFieldType fieldType;
    char *fieldName;
    fieldName = (char *) malloc(12*sizeof(char));

    if (shapeType == SHPT_ARC)
      cout << endl << "Printing Arc data for " << nb_entities << " entities" << endl;
    else if (shapeType == SHPT_POLYGON)
      cout << endl << "Printing Polygon data for " << nb_entities << " entities" << endl;
    else if (shapeType == SHPT_POLYGONZ)
      cout << endl << "Printing PolygonZ data for " << nb_entities << " entities" << endl;
    else
      cout << endl << "Printing Multipoint data for " << nb_entities << " entities" << endl;

    cout.precision(12);
    for (entity = 0; entity < nb_entities; entity++)
    {
      shpObject = SHPReadObject(hSHP, entity);
      nb_vertices = shpObject->nVertices;
      nb_parts = shpObject->nParts;
      if (nb_parts < 2)
      {
        nb_parts = 1;
        cout << endl << "Entity " << entity << " has " << nb_vertices << " vertices:" << endl;
      }
      else
      {
        cout << endl << "Entity " << entity << " has " << nb_vertices << " vertices in " << nb_parts << " parts" << endl;
        for (part = 0; part < nb_parts; part++)
          cout << "Part " << part << " starts at vertex " << shpObject->panPartStart[part] << endl;
        cout << "vertices:" << endl;
      }
      for (vertex = 0; vertex < nb_vertices; vertex++)
      {
        cout << "For vertex " << vertex << ", X = " << shpObject->padfX[vertex] << " and Y = " << shpObject->padfY[vertex] << endl;
      }
      cout << endl;
      for (field = 0; field < field_count; field++)
      {
        fieldType = DBFGetFieldInfo(hDBF,field,fieldName,&width,&decimals);
        cout << "For field " << field << " with Name " << fieldName << ", value = ";
        switch(fieldType)
        {
          case FTString:  cout << DBFReadStringAttribute(hDBF,entity,field) << endl;
                          break;
          case FTInteger: cout << DBFReadIntegerAttribute(hDBF,entity,field) << endl;
                          break;
          case FTDouble:  cout << DBFReadDoubleAttribute(hDBF,entity,field) << endl;
                          break;
          case FTLogical: if (DBFReadIntegerAttribute(hDBF,entity,field))
                            cout << "true" << endl;
                          else
                            cout << "false" << endl;
                          break;
          case FTInvalid: cout << " (Invalid field)";
                          break;
          default:        cout << " (Unknown Field Type)";
                          break;
        }
      }
    }
    cout.precision(6);

    return;
  }

  void Shape::create_and_copy(const string& file_name, const Shape& source)
  {
    hSHP = SHPCreate(file_name.c_str(),source.shapeType);
    hDBF = DBFCreate(file_name.c_str());

    char *fieldName;
    fieldName = (char *) malloc(12*sizeof(char));
    int entity, width, decimals, field, field_count = DBFGetFieldCount(source.hDBF);
    DBFFieldType fieldType;
    SHPObject *shpObject, *sourceShpObject;

    for (field = 0; field < field_count; field++)
    {
      fieldType = DBFGetFieldInfo(source.hDBF,field,fieldName,&width,&decimals);
      DBFAddField(hDBF,fieldName,fieldType,width,decimals);
    }

    for (entity = 0; entity < source.nb_entities; entity++)
    {
      sourceShpObject = SHPReadObject(source.hSHP, entity);
      shpObject = SHPCreateSimpleObject(source.shapeType, sourceShpObject->nVertices, 
                                        sourceShpObject->padfX, sourceShpObject->padfY, sourceShpObject->padfZ);
      SHPComputeExtents(shpObject);
      SHPWriteObject(hSHP,-1,shpObject);
      for (field = 0; field < field_count; field++)
      {
        fieldType = DBFGetFieldInfo(source.hDBF,field,fieldName,&width,&decimals);
        switch(fieldType)
        {
          case FTString:  DBFWriteStringAttribute(hDBF,entity,field,DBFReadStringAttribute(source.hDBF,entity,field));
                          break;
          case FTInteger: DBFWriteIntegerAttribute(hDBF,entity,field,DBFReadIntegerAttribute(source.hDBF,entity,field));
                          break;
          case FTDouble:  DBFWriteDoubleAttribute(hDBF,entity,field,DBFReadDoubleAttribute(source.hDBF,entity,field));
                          break;
          case FTLogical: DBFWriteIntegerAttribute(hDBF,entity,field,DBFReadIntegerAttribute(source.hDBF,entity,field));
                          break;
          case FTInvalid: cout << " (Invalid field)";
                          break;
          default:        cout << " (Unknown Field Type)";
                          break;
        }
      }
    }

    shape_flag = true;

    return;
  }
#else
 // Read shape data from an ASCII file
  void Shape::ascii_read(const string& file_name)
  {
    ifstream shape_fs;

    shape_fs.open(file_name.c_str());
    if (!shape_fs)
    {
      cout << "Cannot open shape input file " << file_name << endl;
      shape_flag = false;
      return;
    }
    
  // Read shape file data until End-of-File reached
    string tag, tmp_string;
    int index;
    int entity = -1;
    while (!shape_fs.eof())
    {
      shape_fs >> tag;
      if (tag == "-shape_type")
      {
        shape_fs >> tmp_string;
        if (tmp_string = "SHPT_POINT")
          shapeType = SHPT_POINT;
        if (tmp_string = "SHPT_ARC")
          shapeType = SHPT_ARC;
        if (tmp_string = "SHPT_POLYGON")
          shapeType = SHPT_POLYGON;
        if (tmp_string = "SHPT_MULTIPOINT")
          shapeType = SHPT_MULTIPOINT;
        if (tmp_string = "SHPT_POLYGONZ")
          shapeType = SHPT_POLYGONZ;
      }
      if (tag == "-minBound")
      {
        for (index = 0; index < 4; index++)
          shape_fs >> minBound[index];
      }
      if (tag == "-maxBound")
      {
        for (index = 0; index < 4; index++)
          shape_fs >> maxBound[index];
      }
      if (tag == "-nb_entities")
      {
        shape_fs >> nb_entities;
        nb_vertices = new double[nb_entities];
        XY_offset = new int[nb_entities];
      }
      if (tag == "-XY_size")
      {
        shape_fs >> XY_size;
        X = new double[XY_size];
        Y = new double[XY_size];
      }
      if (tag == "-nb_vertices")
      {
        entity++;
        shape_fs >> nb_vertices[entity];
        if (entity == 0)
          XY_offset[entity] = 0;
        else
          XY_offset[entity] = XY_offset[entity-1] + nb_vertices[entity-1];
      }
      if (tag == "-X")
      {
        XY_offset[entity] += XY_size;
        for (index = 0; index < nb_vertices[entity]; index++)
          shape_fs >> X[XY_offset[entity]+index];
      }
      if (tag == "-Y")
      {
        for (index = 0; index < nb_vertices[entity]; index++)
          shape_fs >> Y[XY_offset[entity]+index];
      }
    }
    shape_fs.close();

    data_flag = true;
  
    return;
  }
#endif

 // Print the shape information
  void Shape::print_info()
  {
    switch (shapeType)
    {
      case SHPT_POINT:      cout << "Shape Type = SHPT_POINT" << endl;
                            break;
      case SHPT_ARC:        cout << "Shape Type = SHPT_ARC" << endl;
                            break;
      case SHPT_POLYGON:    cout << "Shape Type = SHPT_POLYGON" << endl;
                            break;
      case SHPT_MULTIPOINT: cout << "Shape Type = SHPT_MULTIPOINT" << endl;
                            break;
      case SHPT_POLYGONZ:   cout << "Shape Type = SHPT_POLYGONZ" << endl;
                            break;
      default:              cout << "Unknown Shape Type" << endl;
                            break;
    }

    int index;
    cout.precision(12);
    cout << "minBound: ";
    for (index = 0; index < 4; index++)
      cout << minBound[index] << " ";
    cout << endl;
    cout << "maxBound: ";
    for (index = 0; index < 4; index++)
      cout << maxBound[index] << " ";
    cout << endl;
    cout << "nb_entities = " << nb_entities << endl;

    int entity;
    for (entity = 0; entity < nb_entities; entity++)
    {
      cout << "For shapefile entity " << entity << ":" << endl << "X: ";
      for (index = 0; index < nb_vertices[entity]; index++)
        cout << X[XY_offset[entity]+index] << " ";
      cout << endl;
      cout << "Y: ";
      for (index = 0; index < nb_vertices[entity]; index++)
        cout << Y[XY_offset[entity]+index] << " ";
      cout << endl;
    }


    return;
  }

 // Write shape data in ASCII format
  void Shape::ascii_write(const string& file_name)
  {
    ofstream shape_fs;

    shape_fs.open(file_name.c_str());
    switch (shapeType)
    {
      case SHPT_POINT:      shape_fs << "-shape_type SHPT_POINT" << endl;
                            break;
      case SHPT_ARC:        shape_fs << "-shape_type SHPT_ARC" << endl;
                            break;
      case SHPT_POLYGON:    shape_fs << "-shape_type SHPT_POLYGON" << endl;
                            break;
      case SHPT_MULTIPOINT: shape_fs << "-shape_type SHPT_MULTIPOINT" << endl;
                            break;
      case SHPT_POLYGONZ:   shape_fs << "-shape_type SHPT_POLYGONZ" << endl;
                            break;
      default:              break;
    }
    
    int index;
    shape_fs.precision(12);
    shape_fs << "-minBound ";
    for (index = 0; index < 4; index++)
      shape_fs << minBound[index] << " ";
    shape_fs << endl;
    shape_fs << "-maxBound ";
    for (index = 0; index < 4; index++)
      shape_fs << maxBound[index] << " ";
    shape_fs << endl;
    shape_fs << "-nb_entities " << nb_entities << endl;
    shape_fs << "-XY_size " << XY_size << endl;
    int entity;
    for (entity = 0; entity < nb_entities; entity++)
    {
      shape_fs << "-nb_vertices " << nb_vertices[entity] << endl;
      shape_fs << "-X ";
      for (index = 0; index < nb_vertices[entity]; index++)
        shape_fs << X[XY_offset[entity]+index] << " ";
      shape_fs << endl;
      shape_fs << "-Y ";
      for (index = 0; index < nb_vertices[entity]; index++)
        shape_fs << Y[XY_offset[entity]+index] << " ";
      shape_fs << endl;
    }

    shape_fs.close();

    return;
  }

} // namespace CommonTilton
