// point.cc

#include "point.h"
#include <iostream>
#include <math.h>

namespace CommonTilton
{

 // Constructor
  Point::Point()
  {
    return;
  }

 // Constructor
  Point::Point(const int& x, const int& y)
  {
    col = x;
    row = y;

    return;
  }

 // Copy Constructor
  Point::Point(const Point& source)
  {
    col = source.col;
    row = source.row;

    return;
  }

 // Destructor...
  Point::~Point() 
  {
    return;
  }

  void Point::operator =(const Point& source)
  {
    col = source.col;
    row = source.row;

    return;
  }

  bool Point::operator ==(const Point& source)
  {
    return ((col == source.col) && (row == source.row));
  }

  double Point::distance(Point& other_point)
  {
     return sqrt((col-other_point.col)*(col-other_point.col) + (row-other_point.row)*(row-other_point.row));
  }

  void Point::print()
  {
     cout << "(x,y) = (" << col << "," << row << ")";
 
     return;
  }

  void print_path(vector<Point>& path)
  {
    vector<Point>::const_iterator path_iter = path.begin();
    cout << "(col,row)" << endl;
    while (path_iter != path.end())
    {
      cout << "(" << (*path_iter).col << "," << (*path_iter).row << ")" << endl;
      ++path_iter;
    }  
     

     return;
  }

  void add_all(vector<Point>& base_path, vector<Point>& add_path)
  {
    int size = add_path.size();

    for (int i=0; i<size; i++)
      base_path.push_back(add_path[i]);

    return;
  }

  void add_all(list<Point>& base_path, vector<Point>& add_path)
  {
    int size = add_path.size();

    for (int i=0; i<size; i++)
      base_path.push_back(add_path[i]);

    return;
  }

  void add_all(vector<Point>& base_path, list<Point>& add_path)
  {
    list<Point>::const_iterator add_iter = add_path.begin();
    while (add_iter != add_path.end())
    {
      base_path.push_back(*add_iter);
      ++add_iter;
    }  

    return;
  }

  bool contains(vector<Point>& base_path, Point& cmp_pt)
  {
    int size = base_path.size();

    for (int i=0; i<size; i++)
      if (base_path[i] == cmp_pt)
        return true;

    return false;
  }

  int lastIndexOf(vector<Point>& base_path, Point& cmp_pt)
  {
    int last_index, size = base_path.size();

    last_index = -1;
    for (int i=0; i<size; i++)
      if (base_path[i] == cmp_pt)
        last_index = i;

    return last_index;
  }

 // Constructor
  dPoint::dPoint()
  {
    return;
  }

 // Constructor
  dPoint::dPoint(const double& x, const double& y)
  {
    X = x;
    Y = y;

    return;
  }

 // Copy Constructor
  dPoint::dPoint(const dPoint& source)
  {
    X = source.X;
    Y = source.Y;

    return;
  }

 // Destructor...
  dPoint::~dPoint() 
  {
    return;
  }

  void dPoint::operator =(const dPoint& source)
  {
    X = source.X;
    Y = source.Y;

    return;
  }

  bool dPoint::operator ==(const dPoint& source)
  {
    return ((X == source.X) && (Y == source.Y));
  }

  double dPoint::distance(dPoint& other_dPoint)
  {
     return sqrt((X-other_dPoint.X)*(X-other_dPoint.X) + (Y-other_dPoint.Y)*(Y-other_dPoint.Y));
  }

  void dPoint::print()
  {
     cout << "(x,y) = (" << X << "," << Y << ")";
 
     return;
  }

  void print_path(vector<dPoint>& path)
  {
    vector<dPoint>::const_iterator path_iter = path.begin();
    cout << "(X,Y)" << endl;
    while (path_iter != path.end())
    {
      cout << "(" << (*path_iter).X << "," << (*path_iter).Y << ")" << endl;
      ++path_iter;
    }  
     

     return;
  }

  void add_all(vector<dPoint>& base_path, vector<dPoint>& add_path)
  {
    int size = add_path.size();

    for (int i=0; i<size; i++)
      base_path.push_back(add_path[i]);

    return;
  }

  void add_all(list<dPoint>& base_path, vector<dPoint>& add_path)
  {
    int size = add_path.size();

    for (int i=0; i<size; i++)
      base_path.push_back(add_path[i]);

    return;
  }

  void add_all(vector<dPoint>& base_path, list<dPoint>& add_path)
  {
    list<dPoint>::const_iterator add_iter = add_path.begin();
    while (add_iter != add_path.end())
    {
      base_path.push_back(*add_iter);
      ++add_iter;
    }  

    return;
  }

  bool contains(vector<dPoint>& base_path, dPoint& cmp_pt)
  {
    int size = base_path.size();

    for (int i=0; i<size; i++)
      if (base_path[i] == cmp_pt)
        return true;

    return false;
  }

  int lastIndexOf(vector<dPoint>& base_path, dPoint& cmp_pt)
  {
    int last_index, size = base_path.size();

    last_index = -1;
    for (int i=0; i<size; i++)
      if (base_path[i] == cmp_pt)
        last_index = i;

    return last_index;
  }

} // namespace CommonTilton
