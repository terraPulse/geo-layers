#ifndef POINT_H
#define POINT_H

#include <vector>
#include <list>

using namespace std;

namespace CommonTilton
{
  class Point
  {
    public:
    //  CONSTRUCTORS and DESTRUCTOR
      Point();
      Point(const int& x, const int& y);
      Point(const Point& source);
      virtual ~Point();
 
   //  MODIFICATION MEMBER FUNCTIONS
      void operator =(const Point& source);
      void set_col(const int& value)
           { col = value; }
      void set_x(const int& value)
           { col = value; }
      void set_row(const int& value)
           { row = value; }
      void set_y(const int& value)
           { row = value; }

   //  CONSTANT MEMBER FUNCTIONS
      bool operator ==(const Point& source);
      double distance(Point& other_point);
      int get_col() const { return col; }
      int get_x() const { return col; }
      int get_row() const { return row; }
      int get_y() const { return row; }
      void print();

   // FRIEND FUNCTIONS and CLASSES
      friend void print_path(vector<Point>& path);
      friend void add_all(vector<Point>& base_path, vector<Point>& add_path);
      friend void add_all(list<Point>& base_path, vector<Point>& add_path);
      friend void add_all(vector<Point>& base_path, list<Point>& add_path);
      friend bool contains(vector<Point>& base_path, Point& cmp_pt);
      friend int lastIndexOf(vector<Point>& base_path, Point& cmp_pt);

    protected:

    private:
   //  PRIVATE DATA
      int col,row;
  };

  class dPoint
  {
    public:
    //  CONSTRUCTORS and DESTRUCTOR
      dPoint();
      dPoint(const double& x, const double& y);
      dPoint(const dPoint& source);
      virtual ~dPoint();
 
   //  MODIFICATION MEMBER FUNCTIONS
      void operator =(const dPoint& source);
      void set_X(const double& value)
           { X = value; }
      void set_Y(const double& value)
           { Y = value; }

   //  CONSTANT MEMBER FUNCTIONS
      bool operator ==(const dPoint& source);
      double distance(dPoint& other_point);
      double get_X() const { return X; }
      double get_Y() const { return Y; }
      void print();

   // FRIEND FUNCTIONS and CLASSES
      friend void print_path(vector<dPoint>& path);
      friend void add_all(vector<dPoint>& base_path, vector<dPoint>& add_path);
      friend void add_all(list<dPoint>& base_path, vector<dPoint>& add_path);
      friend void add_all(vector<dPoint>& base_path, list<dPoint>& add_path);
      friend bool contains(vector<dPoint>& base_path, dPoint& cmp_pt);
      friend int lastIndexOf(vector<dPoint>& base_path, dPoint& cmp_pt);

    protected:

    private:
   //  PRIVATE DATA
      double X,Y;
  };

} // CommonTilton

#endif /* POINT_H */
