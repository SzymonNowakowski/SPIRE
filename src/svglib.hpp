/* SPIRE - Structure Projection Image Recognition Environment
 * Copyright (C) 2021 Tobias Hain
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see https://www.gnu.org/licenses.
 */

#ifndef SVGLIB_H
#define SVGLIB_H

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>



class svg_color_palette {

public:
  
  static std::string get_color_string( double val, std::string scaling = "lin" ){

    std::stringstream ss;
    double r,g,b; 

    if( scaling == "lin") {
    
      if( val < 0.5 ){
	r = 1;
      } else {
	r = ( 2 - 2 * val);
      }

      if( val < 0.5 ){
	g = 2*val;
      } else {
	g = ( 2 - 2 * val );
      }

      if( val < 0.5 ){
	b = 2*val;
      } else {
	b = 1;
      }

    }
      
    if( scaling == "exp" ){

      double m = 1e-5; //smallest value
      double lm = log(m);
      double im = 1.0 / m;
      
      if( val < 0.5 ){
	r = 1;
      } else {
	r = im * exp( 2 * lm * val );
      }

      if( val < 0.5 ){
	g = m * exp( -2 * lm * val );
      } else {
	g = im * exp( 2 * lm *val );
      }

      if( val < 0.5 ){
	b = m * exp( -2 * lm * val );
      } else {
	b = 1;
      }

    }


    if( scaling == "log" ){

      double m = 1e-5; //smallest value

      double cr = ( (2*m*exp(1) - 1) / (2*m - 1) );
      double br = (1-cr)/m;

      double cf = 2*exp(1) - 1;
      double bf = 1 - cf;
      
      
      if( val < 0.5 ){
	r = 1;
      } else {
	r = log( bf*val+cf);
      }

      if( val < 0.5 ){
	g = log( br * val + cr );
      } else {
	g = log( bf * val + cf );
      }

      if( val < 0.5 ){
	b = log( br * val + cr);
      } else {
	b = 1;
      }

    }

    
    
    ss << "rgb("
       << int(r*255) << ", "
       << int(g*255) << ", "
       << int(b*255) <<")";

      return ss.str();

  }


};



class svg_object {

public:
  virtual std::string to_string() = 0;

  svg_object ( std::string color_ = "black", double stroke_ = 1, std::string fill_ = "black" ) : color(color_), stroke(stroke_), fill(fill_){
  }

  ~svg_object(){
  }
  
  //protected:
  std::string color;
  double stroke;
  std::string fill;
  
};


class svg_line : public svg_object{

public:

  svg_line( double x1_, double y1_, double x2_, double y2_, std::string color_ = "black", double stroke_ = 1, std::string fill_ = "black" ) :
    svg_object(color_, stroke_, fill_), x1(x1_), x2(x2_), y1(y1_), y2(y2_) {

  }
  
  std::string to_string(){
    std::stringstream ss;
    
    ss << "<line x1=\"" << x1 << "\" y1=\"" << y1
       << "\" x2=\"" << x2 << "\" y2=\"" << y2
       << "\" stroke=\"" << color
       << "\" stroke-width=\"" << stroke
       << "\" fill=\"" << fill
       << "\" stroke-linecap=\"round"
       << "\" />";
    
    return ss.str();
  }
  
  double x1, x2, y1, y2;
};

class svg_circle : public svg_object {

public:
  
  svg_circle ( double x_, double y_, double r_, std::string color_ = "black", double stroke_ = 1, std::string fill_ = "black" ) : svg_object(color_, stroke_, fill_), x(x_), y(y_), r(r_)  {

  }

  ~svg_circle(){
  }
    
  
  std::string to_string(){

    std::stringstream ss;

    ss << "<circle cx=\"" << x << "\" cy=\"" << y
       << "\" r=\"" << r
       << "\" stroke=\"" << color
       << "\" stroke-width=\"" << stroke
       << "\" fill=\"" << fill  << "\" />";

    return ss.str();
  }
  
  double x,y;
  double r;

};


class svg_square : public svg_object {
  
public:
  
  svg_square ( double x_, double y_, double dx_, double dy_, std::string color_ = "black", double stroke_ = 1, std::string fill_ = "black" ) : svg_object(color_, stroke_, fill_), x(x_), y(y_), dx(dx_), dy(dy_)  {
      
    }
    
    ~svg_square(){
    }
    
  
  std::string to_string(){

    std::stringstream ss;

    ss << "<rect x=\"" << x-(dx/2.0) << "\" y=\"" << y-(dy/2.0)
       << "\" width=\"" << dx << "\" height=\"" << dy
       << "\" stroke=\"" << color
       << "\" stroke-width=\"" << stroke
       << "\" fill=\"" << fill  << "\" />";
    
    return ss.str();
    
  }
    
    double x,y;
    double dx,dy;
    double r;

};




class svg_polygon : public svg_object {
public:

  svg_polygon( std::string color_ = "black", double stroke_ = 1, std::string fill_ = "black", double alpha_ = 1 ) : svg_object( color_, stroke_, fill_ ), alpha( alpha_ ) {
  }

  void add_point( double x, double y ){
    points.push_back(x);
    points.push_back(y);
  }
  
  std::string to_string(){

    std::stringstream ss;

    ss << "<polygon points=\"";
    
    for(unsigned int ii=0; ii<points.size(); ii+=2){
      ss << points.at(ii) << "," << points.at(ii+1) << " ";
    }
    
    ss << "\" stroke=\"" << color
       << "\" stroke-width=\"" << stroke
       << "\" fill=\"" << fill
       << "\" fill-opacity=\"" << alpha
       << "\" stroke-linejoin=\"round"
       << "\" />";

    return ss.str();
    
  }

  int size(){
    return points.size();
  }
  
private:

  double alpha;
  std::vector<double> points;
  
};



class svg_arrow : public svg_object {

public:
  
  svg_arrow( double x1_, double y1_, double x2_, double y2_, double l_, std::string color_ = "black", double stroke_ = 1, std::string fill_ = "black"  ) : svg_object( color_, stroke_, fill_ ), x1(x1_), x2(x2_), y1(y1_), y2(y2_), l(l_) {
    
    double angle = std::fabs( atan2( (y2 - y1) , (x2 - x1) ) );

    if( angle > 0.5*M_PI ) angle = M_PI - angle;

    double open_a = M_PI/6.;
    double sigma1, sigma2;
    sigma1 = angle - open_a;
    sigma2 = M_PI - open_a - angle;

    //double l = 0.2 * sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
  
    double dx1 = l * cos( sigma1 );
    double dy1 = l * sin( sigma1 );
  
    double dx2 = l * cos( sigma2 );
    double dy2 = l * sin( sigma2 );    
    
    svg_polygon tip( color, stroke, fill );
    svg_line shaft( x1, y1, x2, y2, color, stroke, fill );
    
    if( x2 > x1 ){
      if( y2 > y1 ){
	tip.add_point( x2 - dx1, y2 - dy1 );
	tip.add_point( x2 + dx2, y2 - dy2 );
	tip.add_point( x2, y2 );
      } else {
	tip.add_point( x2 - dx1, y2 + dy1 ); 
	tip.add_point( x2 + dx2, y2 + dy2 );
	tip.add_point( x2, y2);
      }
    } else {
      if( y2 > y1 ){
	tip.add_point( x2 + dx1, y2 - dy1 );
	tip.add_point( x2 - dx2, y2 - dy2 );
	tip.add_point( x2, y2);
      } else {
	tip.add_point( x2 + dx1, y2 + dy1 );
	tip.add_point( x2 - dx2, y2 + dy2 );
	tip.add_point( x2, y2 );
      }
    }
    
    components.push_back( shaft.to_string() );
    components.push_back( tip.to_string() );
  }

  std::string to_string(){
    return components[0] + "\n" + components[1];
  }


  double x1,x2,y1,y2,l;
  std::vector<std::string> components;
};


class svg_canvas {

public:

  
  svg_canvas(int w, int h) : width(w), height(h) {

  }


  void add_object( svg_object *item ){
    objects.push_back( item->to_string() );
  }


  void save( std::string fn ){

    std::ofstream out ( fn );    

    out << "<svg version=\"1.1\"\n"
	<< "     baseProfile=\"full\"\n"
	<< "     width=\"" << width << "\" height=\"" << height << "\">\n\n";

    for( unsigned int ii=0; ii < objects.size(); ii++){
      out << objects.at(ii) << std::endl;
    }

    out << "</svg>";
    out.close();
  }

  std::string to_string(){

    std::stringstream out;

    out << "<svg version=\"1.1\"\n"
	<< "     baseProfile=\"full\"\n"
	<< "     width=\"" << width << "\" height=\"" << height << "\">\n\n";

    for( unsigned int ii=0; ii < objects.size(); ii++){
      out << objects.at(ii) << std::endl;
    }

    out << "</svg>";
    return out.str();
    
  }


			
  
private:

  std::vector<std::string> objects;

  int width, height;


  
};


#endif
