#ifndef SHAPE_H
#define SHAPE_H

#include <string>
#include "../point/point.h"
#ifdef SHAPEFILE
#include <shapefil.h>
#endif

using namespace std;

namespace CommonTilton
{
#ifndef SHAPEFILE

  #define SHPT_POINT		 1	// Points
  #define SHPT_ARC		 3	// Arcs (Polylines, possibly in parts)
  #define SHPT_POLYGON		 5	// Polygons (possibly in parts)
  #define SHPT_MULTIPOINT	 8	// MultiPoint (related points)
  #define SHPT_POLYGONZ		15	// Polygon (may include "measure" values for vertices)

#endif

  class Shape 
  {
    public:
    // Constructors and Destructor
      Shape();
      virtual ~Shape();

    // Member functions
#ifdef SHAPEFILE
      void open(const string& file_name);
      void print_fieldNames();
      int  get_fieldIndex(const string& field_name);
      void read_point(const string& site_type);
      void read_point();
      void read_arc();
      void read_polygon();
      void get_Bounds(const int& entity, double& X_minBound, double& Y_minBound, 
                      double& X_maxBound, double& Y_maxBound);
      int  get_nParts(const int& entity);
      void get_Vertices(const int& entity, const int& part, vector<dPoint>& vertices);
      double read_double_attribute(const string& field_name, const int& entity);
      string read_string_attribute(const int& field_index, const int& entity);
      void close();
      void print_shape_info();
      void print_field_info();
      void print_point();
      void print_arc_or_polygon();
      void create_and_copy(const string& file_name, const Shape& source);
#else
      void ascii_read(const string& file_name);
#endif
      void print_info();
      bool valid() const { return shape_flag; }
      void ascii_write(const string& file_name);
      int  get_shapeType() const { return shapeType; }
      int  get_nb_entities() const { return nb_entities; }
      int  get_nb_vertices(const int& entity) const { return nb_vertices[entity]; }
      double get_X(const int& entity, const int& vertex) const { return X[XY_offset[entity]+vertex]; }
      double get_Y(const int& entity, const int& vertex) const { return Y[XY_offset[entity]+vertex]; }
      double get_X_minBound() const { return minBound[0]; }
      double get_Y_minBound() const { return minBound[1]; }
      double get_X_maxBound() const { return maxBound[0]; }
      double get_Y_maxBound() const { return maxBound[1]; }
      int  get_nb_parts(const int& entity) const { return nb_parts[entity]; }
      int  get_part_start(const int& entity, const int& part) const { return  part_start[part_start_offset[entity]+part]; }

    protected:

    private:
   // Shape data file information
      bool shape_flag;             // True if shape data is valid
      int nb_entities;             // Number of shape entities/structures.
      int *nb_vertices;            // Number of vertices in each shape object
      int *nb_parts;               // Number of parts in each shape object
#ifdef SHAPEFILE
      SHPHandle hSHP;		   // Shape File Handle
#endif
      int shapeType;               // Shapetype
      double minBound[4];          // X, Y, Z and M minimum values
      double maxBound[4];          // X, Y, Z and M maximum values
      double *X;                   // X location array
      double *Y;                   // Y location array
      int *XY_offset;              // Offset of X and Y location values for each entity
      int XY_size;
      int *part_start;             // PartStart array
      int *part_start_offset;      // Offset of StartPart value for each entity;
      int part_size;

   // Xbase data file information
#ifdef SHAPEFILE
      DBFHandle hDBF;              // DBF File Handle
      int nb_records;              // Number of DBF records (must match nb_entities)
      int fieldIndex;              // Field index for desired field
#endif

      bool data_flag;
  };

} // CommonTilton

#endif /* SHAPE_H */
