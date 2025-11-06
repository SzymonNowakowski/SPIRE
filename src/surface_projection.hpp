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

#ifndef SP_PROJ_I
#define SP_PROJ_I

#include <unordered_set>
#include <iostream>
#include <cassert>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <cstring>
#include <cstdio>
#include <exception>
#include "auxiliary.hpp"
#include "vec_mat_math.hpp"

#include <gmp.h>
#include "iml.h"

#ifdef USE_CGAL
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Advancing_front_surface_reconstruction.h>
#include <CGAL/tuple.h>
#include <CGAL/Vector_3.h>

#include <random>
#endif

#include "global_settings.hpp"

#ifdef HAVE_PNG
#include <png.h>
#endif

/** \brief Quick and dirty implementation of an invalid parameter exception
 *
 * This exception is thrown, if a parameter choice would cause
 * faulty maths, unphysical behavior or just makes no sense at all
 */
class invalid_parameter_exception : public std::exception {

public:
  /// Constructor, initializes \ref msg
  invalid_parameter_exception( std::string _msg );

  /// RETURNS a short string what this exception is
  virtual const char* what() const throw();
  /// Returns a string conatining more details
  const std::string details() const;

  /// String holding details on the exception
  std::string msg;
};



#include "img_out.hpp"
#include "distance_transform.hpp"
#include "homotopic_thinning.hpp"
#include "surface_areas_tables.hpp"
#include "percolation_analysis.hpp"
#include "level_sets.hpp"

/** \brief The computational core class of this code. Computes the
 * projection calculations and stores all parameters relevant for the
 * projection.
 *
 * This class contains all neccessary function to compute the 2D
 * projection of a surface. It also stores all relevant
 * parameters. The GUI is basically only a front end to use the code
 * provided in this class. A terminal interface should be easy to
 * implement if wanted.
 */
class surface_projection {
public:

  /// Default Constructor
  surface_projection( global_settings &gs );
  /// Destructor
  ~surface_projection();

  /// Recomputes the geometric slice properties
  void update_geometry();

  /// validates the scaling in cubic symmetrie
  void validate_uc_scaling();
  
  /// Updates container capacities
  void update_containers();

  /// Computes the projection, wrapper around several functions
  void compute_projection();

  /// Computes the periodicity length in the given direction
  std::vector<int> compute_periodicity( std::vector<double> n, double d_lattice, double cutoff );

  /// Computes two base vector normal to the current orientation
  /// forming the smalles possible unit cell
  void compute_smallest_uc( int reduce = 0 );
    
  /// Computes and sets theta and phi from the Miller indeces
  void set_orientation_from_hkl();

  /// Sets the slice dimensions to fit a unit cell
  void set_slice_to_uc( double margin = 0.0 );

  /// Sets the slice dimensions to fit the primitive unit cell
  void set_slice_to_primitive_uc();
  
  /// Adds a membrane
  void add_membrane( double dist, double width );

  /// sets the "color" of a membrane
  void set_channel_color( int mem_id, int val );

  /// merge adjacent, unseparated channels
  void merge_adjacent_channels();
  
  /// updates the size and resets the filled channels control
  void update_channel_fill_container();
  
  /// Computes the volumes of the channels
  void compute_volume();

  /// Computes the surface area of the membranes
  void compute_surface_area();

  /// Computes the minimal channel diameter of the given channel
  void compute_channel_network();

  /// Computes the largest sphere that fits through the strucutre
  void compute_percolation_threshold( bool periodic );

  /// computes the maximum of the distance map in a channel
  template<class M>
  M compute_max_dist_in_channel( std::vector<M> &dmap, int ch_id );
  
  /// computes the minimal diamter of a channel.
  double get_minimal_channel_diameter( int channel_id );
  
  /// Returns all the points which are on the surface of the channel
  std::vector<int> get_surface_points( int ch_id, int n = 26 );

  /// Returns the distance map. Be careful if it is up to date. If in
  /// doubt, call \ref set_grid() to update it
  std::vector<float> get_distance_map() const;
  
  /// Converts the \ref projection array in a rescaled image array
  unsigned char* get_image(bool invert = false, std::string scaling = "LIN");

#ifdef HAVE_PNG
  /// writes the current projection to a png image
  void save_to_png( std::string out_fn, bool invert = true, std::string scaling = "LIN");
  /// writes the given image data to a png image
  void write_png( unsigned char *img,
		  unsigned int width,
		  unsigned int height,
		  std::string out_fn);
#endif
  
  /// Outputs the grid
  void print_grid( std::string fn );
  
  /// Outputs surface points of the given membrane
  void print_channel_surface_points( int mem_id, std::string fn );

  /// Output the points making up the minimal topological network
  void print_topological_network( int which, std::string fn );

  /// Output channel width distribution
  void print_channel_width_distribution( std::string fn );

  /// output the values of maximal radius covering transform
  void print_max_rad_transform_dist( std::string fn );
  
  /// write parameters to an ASCI file
  void write_parameters( std::string outfile );

  /// reads parameters to an ASCI file
  void read_parameters( std::string outfile );  
  
  /// Returns the number of points in the slice in x direction
  int get_width() const;
  /// Returns the number of points in the slice in y direction  
  int get_height() const;
  /// Returns the number of points in the slice in z direction  
  int get_depth() const;
  
  /// Returns h of the Miller indeces of the slice orientation
  int get_h() const;
  /// Returns k of the Miller indeces of the slice orientation  
  int get_k() const;
  /// Returns l of the Miller indeces of the slice orientation  
  int get_l() const;
  /// Returns the currently set type of the surface
  int get_type() const;
  /// Returns the projection array
  std::vector<float> get_projection() const;
  /// Returns the grid
  std::vector<short> get_grid() const;
  /// returns a copy of the channel array
  std::vector<short> get_channel() const;  
  /// Returns the points (=positions) of the points
  std::vector<float> get_points() const;  
  /// returns the channel_filled array
  std::vector<int> get_channel_fill() const;  
  /// Returns the unit cell size the surface in [001] direction
  std::vector<double> get_a() const;
  /// returns unit cell scale in ab direction
  double get_uc_scale_ab() const;
  /// returns unit cell scale in c direction
  double get_uc_scale_c() const;

  /// returns the base vectors of the 2d uc in column major order
  std::vector<double> get_uc_base() const;
  
  /// Returns the surface level
  double get_surface_level() const;
  /// Returns the ratio of the volumes of the two channels
  double get_channel_vol_prop() const;

  /// Returns the membranes
  std::vector<double> get_membranes() const;
  /// Return channel volumes
  std::vector<double> get_channel_volumes() const;
  /// Return the percolation thresholds
  std::vector<double> get_percolation_thresholds() const;
  /// returns the current maximum pore radius
  std::vector<double> get_max_pore_radius() const;
  /// Return the topological network of the channels
  std::vector< std::unordered_set<int> > get_channel_network() const;
  /// Return surface areas of the membranes
  std::vector<double> get_membrane_surface_area() const;
  /// Returns the width of the slice
  double get_slice_thickness() const;
  /// Returns the height of the slice
  double get_slice_height() const;
  /// Returns the length of the slice
  double get_slice_width() const;  
  /// Returns the position of the slice along its normal vector
  double get_slice_position() const;
  /// Returns theta of the current orientation
  double get_theta() const;
  /// Returns phi of the current orientation  
  double get_phi() const;
  /// Returns the distance between two points in x direction
  double get_dx() const;
  /// Returns the distance between two points in y direction  
  double get_dy() const;
  /// Returns the distance between two points in z direction  
  double get_dz() const;
  /// Returns the unit cell size in [100] direction (length of UC base vectors)
  std::vector<double> get_ucdim() const;
  /// Returns the unit cell size (periodicity length) in the current orientation
  std::vector<double> get_uc_dim_in_orientation() const;
  /// Returns the slice dimensions
  std::vector<double> get_L() const;
  /// Returns the available surfaces
  const std::vector<std::string> get_surface_choices();
  /// Returns the available image scaling methods
  const std::vector<std::string> get_img_scaling_choices();

  /// Sets the theta angle of the orientation of the slice
  void set_theta( double ang );
  /// Sets phi angle of the orientation of the slice
  void set_phi( double ang );
  /// Sets the type of the surface to be projected
  void set_type( int val );
  /// Sets the slice width // z 
  void set_slice_thickness( double val );
  /// Sets the slice heigth // y
  void set_slice_height( double val );
  /// Sets the slice length // x
  void set_slice_width( double val );


  /// Sets the position of the slice along ints normal vector
  void set_slice_position( double val );
  
  /// set membrane
  void edit_membrane( int id, double dist, double width );
  /// deletes a meberane
  void delete_membrane( int id );
  /// set membranes
  void set_membranes( std::vector<double> mems );
  /// sets the channel fills array (deciding which channel is colored")
  void set_channel_fill( std::vector<int> fills );

  /// Sets the surface_level
  void set_surface_level( double val );
  /// Sets the channel proportion
  void set_channel_vol_prop( double val );

  /// recomputes the unitcell dimensions from \ref uc_scale_ab , \ref uc_scape_c and \ref unitcell_dim
  void update_a( );
  /// Sets the unit cell scaling in c direction
  void set_uc_scale_ab ( double val );
  /// Sets the unit cell scaling in c direction
  void set_uc_scale_c ( double val );  
  /// Sets the number of points in the slice in x direction
  void set_n_points_x( int val );
  /// Sets the number of points in the slice in y direction
  void set_n_points_y( int val );
  /// Sets the number of points in the slice in y direction according
  /// to the dimension of the unitcell
  void set_n_points_y_to_unitcell();  
  /// Sets the number of points in the slice in z direction  
  void set_n_points_z( int val );
  /// Sets the number of points in the slice in z direction according
  /// to the dimension of the unitcell
  void set_n_points_z_to_unitcell();    
  /// Sets the \ref h Miller index
  void set_h( int val );
  /// Sets the \ref k Miller index  
  void set_k( int val );
  /// Sets the \ref l Miller index  
  void set_l( int val );

  /// A wrapper functions allowing setting a parameter identified by a
  /// string
  void set_parameter( std::string par, double val );

  /// A wrapper functions allowing getting a parameter identified by a
  /// string
  double get_parameter( std::string par );  


  /// A vector containing string name for each parameter. It can be
  /// used in combination with \ref set_parameter_string to call
  /// setter functions using a string. However, calling the setters
  /// directly should be cleaner. Since this is a ststic member,
  /// initialization is outside the class definition
  static const std::vector<std::string> parameter_names;

  /// Human readable names of the the Parameters
  static const std::vector<std::string> parameter_names_hr;

  /// short names of the the parameters to print into the images
  static const std::vector<std::string> parameter_names_short;
  
protected:

  /// Numerical tolerance when two floating point numbers are considered equal
  double tolerance = 1e-5;
  
  /// Quick and dirty modulo between two doubles
  inline double mod( double lhs, double rhs );
      
  /// Computes the position of the voxels in the slice
  void set_up_points( );
  
  /// Evaluates all voxels with the chosen level sets. Sets colors of voxel
  void set_grid();
  
  /// Computes the 2D projection
  void project_grid ();

  
  /////////////////////////////////////////////
  // parameters                           ////
  ////////////////////////////////////////////

  /*
   * box parameters
   */

  /// Edge length of the slice
  std::vector<double> L;
  std::vector<double> L_2; // L_2 = L/2
  
  /// Distance between to points in x direction and voxel size in x direction
  double dx;
  /// Distance between to points in y direction and voxel size in y direction  
  double dy;
  /// Distance between to points in z direction and voxel size in z direction  
  double dz;  

  /** \brief Slice position along its normal vector in length units
   *
   * It has the same units as the unit cell dimension
   */
  double slice_position;

  /// Rectangular unit cell size in length units
  std::vector<double> a;
  /// Surface period
  std::vector<double> inv_a;

  /// Scaling of unit cell (ab)
  double uc_scale_ab;
  /// Scaling of unit cell (c) for rectangular/hexagonal symmetries (lonsdaleite so far)
  double uc_scale_c;

  /// This parameter controlls the proportions of the two different
  /// channels. 0 means equal volume in both of them
  double surface_level;


  /// Holds information for the membranes in pairs of (distance,
  /// width). For n membranes this container has then 2*n entries This
  /// does need to be sorted, but needs to have the right size, since
  /// the number of channels is determined using the size of this
  /// array
  std::vector<double> membranes;

  /// Holds information about the channel color (if it is "filled"
  /// thus contributing to the projection). Membranes are by default
  /// "filled", where as space in between channels are not. Same order
  /// as the membranes vector.
  std::vector<int> channel_filled;
  
  /// Theta angle of the orientation of the slice
  double theta;
  /// Phi angle of the orientation of the slice  
  double phi;
  
  /// h Miller index
  int h;
  /// k Miller index  
  int k;
  /// l Miller index  
  int l;


  /// the first base vector spanning the hkl-plane, aligned in the
  /// x-direction (cooresponds to <width> direction in the
  /// projection). Chosen as the longer of the two base vector of the
  /// kernel of N^T
  std::vector<int> b1;
  /// The "true" length of b1 in length units
  double len_b1;
  /// the second base vector spanning the hkl-plane
  std::vector<int> b2;  
  /// The "true" length of b2 in length units
  double len_b2;

  /// The angle in between the two base vectors b1, b2
  double alpha;

  
  /** \brief The length of the unitcell in the given orientation of the slice.
   *
   * In [001] direction, this equals the unitcell_dim. -1 if there if
   * the structure is aperiodic within reasonable lengthscales
   */
  std::vector<double> uc_dim_in_orientation;
  
  /// The number of points in the slice in x direction
  unsigned int n_points_x;
  /// The number of points in the slice in y direction  
  unsigned int n_points_y;
  /// The number of points in the slice in z direction  
  unsigned int n_points_z;

  /// Type of surface to project
  int type;



  /// the distance transform of the grid. This is updated in the \ref
  /// set_grid() function and then can be accessed using the getter
  distance_transform<short> dt;

  /// The volumes of the channels
  std::vector<double> volumes;

  /// The surface area of the membranes
  std::vector<double> surface_area;

  /// The percolation thresholds of the channels
  std::vector<double> percolation_thresholds;

  /// The maximum pore size for all channels (not membranes!)
  std::vector<double> max_pore_radius;
  
  /// Available surfaces
  const std::vector<std::string> surface_choices = {"Gyroid",
						    "Diamond",
						    "Primitive",
						    "Lonsdaleite",
						    "I-WF",
						    "Beta-M rods",
						    "Sigma-P rods",
  };

  /// The dimension of the unitcell to keep the symmetry. Now the
  /// unitcell dimension is set using the parameter a above, however,
  /// some membranes are non-cubic with a fixed a/b a/c ratio. This
  /// ratio needs to be supplied here so 
  const std::vector< std::vector<double> > unitcell_dim= { {1.0,1.0,1.0}, // gyr
							   {1.0,1.0,1.0}, // dia
							   {1.0,1.0,1.0}, // prim 
							   {1.0, sqrt(3.0), 1.732692}, //lon_0.2
							   {1.0,1.0,1.0}, // iwf
							   {1.0, 1.0, 1.0}, // Beta-M rod packing
							   {1.0, 1.0, 1.0}, // Sigma_P rod packing
  };

  /// The base vectors of the direct lattice as column vectors
  Matrix<double> A_dir;
  /// The base vectors of the reciprocal lattice as column vectors
  Matrix<double> A_rec;

  /// Available image scalings
  const std::vector<std::string> img_scaling_choices = {"LIN",
							"LOG"};

  /// Lookup tables for surface properties
  SURFACE_AREAS_TABLES s_tables;
  
  /******************************************************************
   * data, do not access manually unless you know what you're doing
   ******************************************************************/

  /// Array holding coordinates of the voxels
  std::vector<float> points;
  /// Array holding the color (electron density) of the voxels
  std::vector<short> grid;
  /// Array denoting the channel a voxel is in. Membranes are
  /// considered channels. 1 is the innermost channel, 2 the
  /// innermoste membrane, ... (except for the sphere surface, in this
  /// case the order is inverted for whatever reason - I guess wierd
  /// level set values.
  /// negative/positive values denotes if the channel
  /// is inside or outside of the main "membrane" (=the membrane set
  /// by the level set constraint)
  std::vector<short> channel;

  /// This vector holds the minimal topological networks of the
  /// channels
  std::vector< std::unordered_set<int> > topological_network;

  /// The 2D projection
  std::vector<float> projection;

  /// the global settings
  global_settings &gs;
  
};

#endif
