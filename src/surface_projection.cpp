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


#include "surface_projection.hpp"
#include <numbers>

// set static members
const std::vector<std::string> surface_projection::parameter_names = std::vector<std::string> { "struct_types", "uc_scale_ab", "uc_scale_c", "surface_level", "vol_prop", "slice_thickness", "slice_height", "slice_width", "slice_position", "miller_h", "miller_k", "miller_l", "membrane_width" };

const std::vector<std::string> surface_projection::parameter_names_hr = std::vector<std::string> { "Structure Type", "Unit Cell Scale Factor (xy)", "Unit Cell Scale Factor (z)", "Level Set", "Volume Proportion", "Slice Thickness", "Slice Height", "Slice Width", "Slice Position", "Orientation h", "Orientation k", "Orientation l", "Membrane(s) width" };

const std::vector<std::string> surface_projection::parameter_names_short = std::vector<std::string> { "Surface", "UC scale xy", "UC scale z", "Level Set", "Vol. prop.", "Thickness", "Height", "Width", "Position", "h", "k", "l", "mem. width" };


#ifdef USE_CGAL

/**
 * This struct is copy & paste from a CGAL example for surface
 * triangulations. I think it defines something like the niehgborhood
 * of points
 */
struct Perimeter {
  double bound;
  
  Perimeter(double bound): bound(bound){
  }


  // overload the () operator of this class making it a Functor
  template <typename AdvancingFront, typename Cell_handle>
  double operator() (const AdvancingFront& adv, Cell_handle& c,
                     const int& index) const
  {
    // bound == 0 is better than bound < infinity
    // as it avoids the distance computations
    if(bound == 0){
      return adv.smallest_radius_delaunay_sphere (c, index);
    }
    
    // If perimeter > bound, return infinity so that facet is not used
    double d  = 0;
    d = sqrt(squared_distance(c->vertex((index+1)%4)->point(),
                              c->vertex((index+2)%4)->point()));
    if(d>bound) return adv.infinity();
    d += sqrt(squared_distance(c->vertex((index+2)%4)->point(),
                               c->vertex((index+3)%4)->point()));
    if(d>bound) return adv.infinity();
    d += sqrt(squared_distance(c->vertex((index+1)%4)->point(),
                               c->vertex((index+3)%4)->point()));
    if(d>bound) return adv.infinity();
    // Otherwise, return usual priority value: smallest radius of
    // delaunay sphere
    return adv.smallest_radius_delaunay_sphere (c, index);
  }
};

// Some typedefs for CGAL
typedef CGAL::Simple_cartesian<double> K;
typedef K::Point_3  Point_3;
typedef K::Vector_3 Vector_3;
typedef std::array<std::size_t,3> Facet;




// overload the output operator for facets 
namespace std {
  std::ostream&
  operator<<(std::ostream& os, const Facet& f)
  {
    os << "3 " << f[0] << " " << f[1] << " " << f[2];
    return os;
  }
}


#endif

invalid_parameter_exception::invalid_parameter_exception( std::string _msg ) : msg(_msg){
}

const char* invalid_parameter_exception::what() const throw() {
  return msg.c_str();//"Invalid parameter choice!";
}

const std::string invalid_parameter_exception::details() const {
  return msg;
}

/**
 * writes all parameters to an ASCI file, so certain settings can be
 * reloaded 
 */
void surface_projection::write_parameters( std::string outfile ){

  std::ofstream out ( outfile );

  out << " # parameters" << std::endl;;


  out << "Lx=" << L[0] << std::endl
      << "Ly=" << L[1] << std::endl
      << "Lz=" << L[2] << std::endl;

  out << "slice_position=" << slice_position << std::endl;
  
  out << "ax=" << a[0] << std::endl
      << "ay=" << a[1] << std::endl
      << "az=" << a[2] << std::endl;  

  out << "uc_scale_ab=" << uc_scale_ab << std::endl
      << "uc_scale_c=" << uc_scale_c << std::endl;

  out << "surface_level=" << surface_level << std::endl;

  out << "<membranes>" << std::endl;
  for( unsigned int ii=0; ii < membranes.size(); ii+=2 ){
    out << "pos=" << membranes[ii] << ";width=" << membranes[ii+1] << std::endl;
  }
  out << "</membranes>" << std::endl;

  out << "channel_filled=";
  for(unsigned int ii=0; ii<channel_filled.size(); ii++){
    out << channel_filled[ii];
    if( ii < channel_filled.size()-1 )
      out<<",";
  }
  out << std::endl;

  out << "h=" << h << std::endl
      << "k=" << k << std::endl
      << "l=" << l << std::endl;

  out << "n_points_x=" << n_points_x << std::endl
      << "n_points_z=" << n_points_z << std::endl;

  out << "type=" << type << std::endl;
  
  out.close();
}

/**
 * reads all parameters from an ASCI file
 */
void surface_projection::read_parameters( std::string infile ){

  std::ifstream in ( infile );

  // read file line by line
  std::string line;  
  while( !in.eof() ){

    std::getline( in, line );

    //check if line starts with a comment
    unsigned int char_pos = 0;
    while( line[char_pos] == ' ' )
      char_pos++;
    if( char_pos < line.size() && line[char_pos] != '#' ){
      // get content

      auto line_split = my_utility::str_split( line, '=' );

      if( line_split[0] == "Lx" ){
	L[0] = std::stod( line_split[1] );
      }
      if( line_split[0] == "Ly" ){
	L[1] = std::stod( line_split[1] );
      }
      if( line_split[0] == "Lz" ){
	L[2] = std::stod( line_split[1] );
      }

      if( line_split[0] == "slice_position" ){
	slice_position = std::stod( line_split[1] );
      }

      if( line_split[0] == "ax" ){
	a[0] = std::stod( line_split[1] );
      }
      if( line_split[0] == "ay" ){
	a[1] = std::stod( line_split[1] );
      }
      if( line_split[0] == "az" ){
	a[2] = std::stod( line_split[1] );
      }


      if( line_split[0] == "uc_scale_ab" ){
	uc_scale_ab = std::stod( line_split[1] );
      }
      if( line_split[0] == "uc_scale_c" ){
	uc_scale_c = std::stod( line_split[1] );
      }

      if( line_split[0] == "surface_level" ){
	surface_level = std::stod( line_split[1] );
      }

      /*
       * membranes
       */
      if( line_split[0] == "<membranes>" ){

	membranes.clear();
	
	// scan until all membranes are read
	std::getline( in, line );
	do{
	  auto mem = my_utility::str_split(line, ';' );

	  membranes.push_back( std::stod( my_utility::str_split(mem[0], '=' )[1] ) );
	  membranes.push_back( std::stod( my_utility::str_split(mem[1], '=' )[1] ) );

	  std::getline( in, line );
	} while( line != "</membranes>" );

      }

      /*
       * channel_filled
       */

      if( line_split[0] == "channel_filled" ){
	channel_filled.clear();
	auto vals = my_utility::str_split( line_split[1], ',' );
	for( unsigned int ii=0; ii<vals.size(); ii++ ){
	  channel_filled.push_back( std::stoi( vals[ii] ) );
	} 
      }


      if( line_split[0] == "h" ){
	h = std::stoi( line_split[1] );
      }
      if( line_split[0] == "k" ){
	k = std::stoi( line_split[1] );
      }
      if( line_split[0] == "l" ){
	l = std::stoi( line_split[1] );
      }

      if( line_split[0] == "n_points_x" ){
	n_points_x = std::stoi( line_split[1] );
      }
      if( line_split[0] == "n_points_z" ){
	n_points_z = std::stoi( line_split[1] );
      }

      if( line_split[0] == "type" ){
	type = std::stoi( line_split[1] );
      }
      

    }
    
  }

  if( membranes.size() + 1 != channel_filled.size() ){
    throw std::string("There was an error reading the "
		      "parameter file; parameters not "
		      "completly restored");
  }
  
  update_geometry();

  compute_projection();

  in.close();
  
}

/** This function prints the grid to a file. The file has the form 
 * x y z color
 * This function does NOT compute the grid, so make sure it is
 * up to date before calling this function
 */
void surface_projection::print_grid( std::string fn ){

  std::ofstream out ( fn );
  out << "#x y z color channel" << std::endl;
  for( unsigned int ii=0; ii<points.size(); ii+=3 ){
    out << points[ii] << " " << points[ii+1] << " ";
    out << points[ii+2] << " " << grid[ii/3] << " ";
    if( channel[ii/3.] < 0 ){
      out << -channel[ii/3.];
    } else {
      out << channel[ii/3.];
    }
    out << std::endl;
  }

  out.close();
}


/** This function prints all points on the surface of the given cahnnel to a
 * gnuplottable file
 */
void surface_projection::print_channel_surface_points( int mem_id, std::string fn ){

  std::vector<int> ch_s_points = get_surface_points( mem_id, 6 );
  
  std::ofstream out ( fn );
  out << " # x y z" << std::endl;

  for( auto it : ch_s_points ){
    out << points[3*it] << " " << points[3*it+1]
	<< " " << points[3*it+2] << std::endl;
  }


  out.close();
}

/** This function saves all points making up the topological network
 * of the inner or outer channel of the structure
 */
void surface_projection::print_topological_network( int which, std::string fn ){

  std::vector< std::unordered_set<int> > net_points = get_channel_network();

  std::ofstream out ( fn );
  out << " # x y z " << std::endl;

  for( auto it : net_points.at( which ) ){
    out << points[3*it] << " " << points[3*it+1]
	<< " " << points[3*it+2] << std::endl;
  }

  out.close();  
}


void surface_projection::print_channel_width_distribution( std::string fn ){

  std::ofstream out ( fn );

  std::vector< std::unordered_set<int> > net_points = get_channel_network();
  auto dmap = dt.get_distance_map();

  for( auto it : net_points[0] ){
    out << dmap[it] << std::endl;    
  }
  for( auto it : net_points[1] ){
    out << dmap[it] << std::endl;    
  }  

  out.close();
}

void surface_projection::print_max_rad_transform_dist( std::string fn ){

  std::ofstream out ( fn );
  
  auto mrct = dt.get_max_radius_covering();
  
  for( auto it : mrct ){
    out << it << std::endl;    
  }

  out.close();
}


/** Standard constructor initialize with the standard values and
 *  derive some more quantities
 */
surface_projection::surface_projection( global_settings &gs_ ) : gs( gs_ ), slice_position(0), a(3,1), inv_a(3,2*std::numbers::pi), uc_scale_ab( 1.0 ), uc_scale_c( 1.0 ), L(3,1), L_2(3,0.5), n_points_x(76), n_points_y(76), n_points_z(76), type(2), h(0), k(0), l(1), uc_dim_in_orientation(3, 1), surface_level( 0.0f ), s_tables() {


  // init the base vectors
  b1 = {1,0,0};
  b2 = {0,1,0};
  alpha=std::numbers::pi/2.0;

  update_a();
  
  //set the channel proportion to 0.5
  set_channel_vol_prop( 0.5 );
  
  //update orientation from hkl
  //sets theta,phi
  set_orientation_from_hkl();
  
  // compute unit cell
  compute_smallest_uc();

  //update geometry
  //sets dx,dy,dz, L
  update_geometry();

  //resize containers
  update_containers();
  
  //add main membrane. This can't be deleted
  membranes.push_back( 0 );
  membranes.push_back( 0.02 );

  // add the first two channels
  channel_filled.push_back( 0 ); // inner channel
  channel_filled.push_back( 1 ); // main membrane
  channel_filled.push_back( 0 ); // outer channel
}


surface_projection::~surface_projection(){

}
    

/**
 * Computes and sets the theta and phi angles to match the orientation
 * given by the Miller indeces
 */
void surface_projection::set_orientation_from_hkl(){

  std::vector<int> ni = {h, k, l};
  std::vector<double> n = VEC_MAT_MATH::get_unit( VEC_MAT_MATH::dot_prod( A_rec, ni ) );

  // get the euclidean coordinate system
  std::vector<double> x = {1.0, 0.0, 0.0};
  std::vector<double> y = {0.0, 1.0, 0.0};
  std::vector<double> z = {0.0, 0.0, 1.0};

  // the projection of n into the xy plane
  std::vector<double> n_xy = {n[0], n[1], 0};

  if( n[0] == 0 && n[1] == 0 ){
    phi = 0;
  } else {
    phi = acos( VEC_MAT_MATH::dot_prod( n_xy, x ) / sqrt(n[0]*n[0] + n[1]*n[1]) );
  }

  // make it between 0 and 360
  if( n[1] < 0 ){
    phi = 2*std::numbers::pi - phi;
  }
  
  theta = acos( VEC_MAT_MATH::dot_prod( n, z) );  
}


/**
 *  This function set ups the points array, i.e. determines the
 *  positions of the voxels. It does that by rotating the base so that
 *  nz is parallel to n and then put a grid along these rotated base
 *  vectors.
 *
 *  If a minimal base is computed, detected if b1 and b2 have non-zero
 *  length, the system is further rotated, so that the rotated x axes
 *  matches either b1 or b2, whichever is longer
 */
void surface_projection::set_up_points( ){
  
  //vectors of slice
  std::vector<double> nx = {1, 0, 0};
  std::vector<double> ny = {0, 1, 0};
  std::vector<double> nz = {0, 0, 1};  
  
  //rotation matrices
  //substract the angle from 2pi since we're rotating in mathematical
  //negative orientation (with the clock
  Matrix<double> Ry = VEC_MAT_MATH::get_y_rot_m(theta), Rz = VEC_MAT_MATH::get_z_rot_m(phi);
  
  //rotate so nz is aligned with the normal vector
  nx = VEC_MAT_MATH::dot_prod( Rz, VEC_MAT_MATH::dot_prod( Ry, nx));
  ny = VEC_MAT_MATH::dot_prod( Rz, VEC_MAT_MATH::dot_prod( Ry, ny));
  nz = VEC_MAT_MATH::dot_prod( Rz, VEC_MAT_MATH::dot_prod( Ry, nz));  

  // now get the final touch by rotation the vectors around the z axis
  // into a nice direction
  std::vector<double> true_b1 = VEC_MAT_MATH::dot_prod( A_dir, b1 );
  
  // get the angle between b1 and x, they already are in the same
  // plane, so this angle is also in-plane
  double quotient = VEC_MAT_MATH::dot_prod( nx, true_b1 ) /
    sqrt(VEC_MAT_MATH::dot_prod( true_b1, true_b1 ) );

  // some cases cause a quotient slightly higher than 1 (floating
  // point arithmetic errors), which causes the acos to yield nan
  if( std::fabs(1.0 - quotient) < 1e-15 ){
    quotient = 1.0;
  }
  
  double ang = acos( quotient );

  // still do not know which direction to rotate, there must be a
  // better way, but for now, I'll just guess and try the other way if
  // the first guess was incorrect  
  Matrix<double> rot_inplane = VEC_MAT_MATH::get_rot_n( nz, ang );
  nx = VEC_MAT_MATH::dot_prod( rot_inplane, nx );
  ny = VEC_MAT_MATH::dot_prod( rot_inplane, ny );
  nz = VEC_MAT_MATH::dot_prod( rot_inplane, nz );
  
  // correct if rotated in the wrong directon
  if( std::fabs( VEC_MAT_MATH::dot_prod( nx, VEC_MAT_MATH::get_unit(true_b1) ) - 1.0 ) > 1e-15 ){
    rot_inplane = VEC_MAT_MATH::get_rot_n( nz, -2*ang );
    nx = VEC_MAT_MATH::dot_prod( rot_inplane, nx );
    ny = VEC_MAT_MATH::dot_prod( rot_inplane, ny );
    nz = VEC_MAT_MATH::dot_prod( rot_inplane, nz );
  }

  long max_points = n_points_x*n_points_y*n_points_z;
  
  //the total index of that particle in the array
  int ind = 0;
  double iy = -L_2[1];
  
  //a single loop would be nicer, but i find this less confusing  
  for(unsigned int ii=0; ii<n_points_y; ii++){ //height, the row index (vertical)
    double jx = -L_2[0];
    for(unsigned int jj=0; jj<n_points_x; jj++){ //width, the column index! (horizontal)

      double kz;

      // the start of the slice in absolute length units
      kz = slice_position - 0.5*L[2];
      
      for(unsigned int kk=0; kk<n_points_z; kk++){ //depth
	
	
	//now compute the absolute position of all voxels
	
	  double x = jx*nx[0] + iy*ny[0] +kz*nz[0];
	  
	  double y = jx*nx[1] + iy*ny[1] +kz*nz[1];
	  
	  double z = jx*nx[2] + iy*ny[2] +kz*nz[2];
	  
	  //assign the position
	  points[ind]=x;
	  points[ind+1]=y;
	  points[ind+2]=z;
	  
	  kz += dz; // next z-pixel
	  ind += 3; // running index
      }
      jx += dx; // next x-pixel
    }
    iy += dy; // next y-pixel
  }
  
}


/**
 * Evalutes the voxels in the level set. Sets the "colors" of the
 * voxels to 1 if they are within the membrane. Also updates the
 * channel array
 */
void surface_projection::set_grid(){

  // First compute the distance map of the current grid

  //We need to set an interval where points are getting markes. Since
  //the surface is sampled it's highly unlikely that a point has
  //exactly the wanted level set constraint. This is a bit iffy, since
  //the minimal thickness of the membrane is not given in length
  //units, but this level set value. However, this is the only way to do it
  double min_width = gs.level_min_width; 
  
  // the level set value of the present points
  double level = 0; 

  for( unsigned int ii=0; ii<points.size(); ii+=3 ){

    //compute level set
    
    if( type == 0 ){ // gyroid
      level = level_set::gyroid( points[ii], points[ii+1], points[ii+2], inv_a );
    } else if( type == 1 ){ // diamond
      level = level_set::diamond( points[ii], points[ii+1], points[ii+2], inv_a );
    } else if( type == 2 ){ // primitive
      level = level_set::primitive( points[ii], points[ii+1], points[ii+2], inv_a );
    } else if( type == 3 ){ // lonsdaleite
      level = level_set::lonsdaleite( points[ii], points[ii+1], points[ii+2], inv_a );
    } else if( type == 4 ){ // iwf
      level = level_set::iwf( points[ii], points[ii+1], points[ii+2], inv_a );      
    } else if( type == 5 ){ // beta-m rods
      level = level_set::beta_m( points[ii], points[ii+1], points[ii+2], inv_a );            
    } else if( type == 6 ){ // sigma-p rods
      level = level_set::sigma_p( points[ii], points[ii+1], points[ii+2], inv_a );            
    }    
    /*
    else if( type == 3 ){ //lonsdaleite_topo
      level = level_set_lonsdaleite_topo( points[ii], points[ii+1], points[ii+2], inv_a );
    } else if( type == 4 ){ //lonsdaleite_0.05
    */
    else {
      throw std::string("type not supported");
    }
    
    //mark appropriate voxels '1'. Also check if we are in the membrane, outside or inside
    if( level <= (surface_level + min_width) && level > ( surface_level - min_width ) ){
      grid[int(ii/3)] = 1;
    }

    //mark if the pixel is in the "outside" or "inside" channel
    //respective to the main membrane
    if( level > surface_level ){
      //outside
      channel[int(ii/3.)] = -1;
    } else if ( level <= surface_level ) {
      //inside
      channel[int(ii/3.)] = 1;
    } 
  }


  // next step is to compute the distance transform. We need to label
  // all points according to which channel they are in also. We do
  // that by comparing the distances to the main membrane with the
  // distance map

  // compute the distance transform of the grid
  dt.set_parameters( grid, std::vector<unsigned int> {n_points_x, n_points_y, n_points_z},
		     std::vector<double> {dx, dy, dz}, false);
  dt.compute_distance_map();
  std::vector<float> dmap = dt.get_distance_map();  

  // now assign channel number

  // get the boundaries of all the membranes
  std::vector<double> mem_pos ( membranes.size(), 0);
  for(unsigned int ii=0; ii<membranes.size(); ii+=2){
    mem_pos[ii] = (membranes[ii] + membranes[ii+1]/2.);
    mem_pos[ii+1] = (membranes[ii] - membranes[ii+1]/2.);
  }
  mem_pos.push_back( -std::numeric_limits<double>::max() );
  mem_pos.push_back( std::numeric_limits<double>::max() );  

  //sort them in ascending order
  std::sort( mem_pos.begin(), mem_pos.end() );

  //now iterate over all pixels and assign each pixel a channel number
  for( unsigned int ii=0; ii<channel.size(); ii++){

    //check if the distance of the current pixel is within the
    //boundaries of the current membrane. The channel array already
    //holds the sign of the distance value to the main membrane

    //start with the inner most shell
    int ch_id=0;

    //iterate over all the channels
    for( ; ch_id < mem_pos.size()-1; ch_id++ ){     
      if( channel[ii] * sqrt(dmap[ii]) >= mem_pos[ch_id] &&
	  channel[ii] * sqrt(dmap[ii]) < mem_pos[ch_id+1])
	  break;
    }    
    
    //found the appropriate membrane, save the channel number
    //the sign still marks, if in or outside of main membrane
    channel[ii] = (ch_id+1)*channel[ii];
  }

  //now iterate over all pixels again and mark the ones, which are
  //within a secondary membrane
  for( unsigned int ii=0; ii<grid.size(); ii++){

    bool mark_point = false;
    double real_distance = sqrt( dmap[ii] );
    
    //check if this point is within the "main" membrane
    // this will extend symmetrically into both channels
    if( real_distance < 0.5*membranes[1] ){
      mark_point = true;
    }

    //check if this point is within several other membranes
    for( unsigned int jj=2; jj<membranes.size(); jj+=2){

      if( membranes[jj] >= 0 ){
	//outside membrane
	
	if( real_distance > (membranes[jj] - membranes[jj+1]/2.) &&
	    real_distance < (membranes[jj] + membranes[jj+1]/2.) &&
	    channel[ii] > 0 ){
	  
	  mark_point = true;
	}
      } else {
	//inside

	if( real_distance > (-membranes[jj] - membranes[jj+1]/2.) &&
	    real_distance < (-membranes[jj] + membranes[jj+1]/2.) &&
	    channel[ii] < 0 ){
	  
	  mark_point = true;
	}	
      }

    }//end cycle membranes

    //mark the point if applicable
    if( mark_point ){
      grid[ii] = 1;
    }
  }

  // update the distance map, since it changed after more pixels turned "white"
  dt.set_parameters( grid, std::vector<unsigned int> {n_points_x, n_points_y, n_points_z},
		     std::vector<double> {dx, dy, dz}, false);
  dt.compute_distance_map();

  
}

/**
 * This function projects the 3D grid into a 2D picture. Basically
 * only adds up all pixel values in each stack
 */
void surface_projection::project_grid (){
  
    
  unsigned int ind_grid = 0;
  unsigned int ind_proj = 0;
  for(unsigned int ii=0; ii<n_points_y; ii++){ //y direcion=height (vertical)
    for(unsigned int jj=0; jj<n_points_x; jj++){//x direction=width (horizontal)
      for(unsigned int kk=0; kk<n_points_z; kk++){
        
	if(grid[ind_grid] == 1){
	  projection[ind_proj] += 1;
	}
	
	ind_grid++; // run 3d-index
	
      }
      ind_proj++;// run 2d-index
    }
  }
  
}



/**
 * This function resizes the containers for the voxel coordinates,
 * gird and projection after a geometry change
 */
void surface_projection::update_containers(){
  //resize and reset arrays
  points.resize( n_points_x * n_points_y * n_points_z * 3, 0 );
  
  /// Array holding the color (electron density) of the voxels
  grid.resize( n_points_x * n_points_y * n_points_z, 0 );

  /// The channel
  channel.resize( n_points_x * n_points_y * n_points_z, 0 );
  
  /// The 2D projection
  projection.resize( n_points_x * n_points_y, 0 );
  memset( projection.data(), 0, sizeof(float) * projection.size());
}


/**
 * This function updates the geometry, i.e. recomputes all values
 * depending on the unit cell size and number of unit cells.
 */
void surface_projection::update_geometry(){

  // update uc dimensions and base vectors
  update_a();

  // update theta and phi
  set_orientation_from_hkl();
  
  // compute uc in orientation  
  compute_smallest_uc();
  
  L_2[0] = L[0]/2.0;
  L_2[1] = L[1]/2.0;
  L_2[2] = L[2]/2.0; //not really needed, just for completeness

  //update nr. of pixels
  set_n_points_y_to_unitcell();
  
  // update pixel size
  dx = L[0] / n_points_x;
  dy = L[1] / n_points_y;
  dz = L[2] / n_points_z;  
}

/**
 * This function computes the projection. It is basically just a
 * wrapper around the functions performing the single steps. It might
 * be more efficient to do that in a single loop, but with modern
 * compiler optimizations and loop unrolling this should not make too
 * much of a difference
 */
void surface_projection::compute_projection( ){


  update_containers();
  
  //get the points in the slice  
  set_up_points();
  
  //reset grid
  memset( grid.data(), 0, sizeof(short) * grid.size() );
  
  //get grid
  set_grid();

  // apply channel colors
  for( unsigned int ii=0; ii < channel_filled.size(); ii++){

    if( ii % 2 == 0 ){
      //channel, if it is zero, do nothing, otherwise mark it
      if( channel_filled[ii] != 0 ){
	set_channel_color( ii + 1, 1 );
      }
    } else {
      //membrane, if it is one, do nothing, otherwise mark it
      if( channel_filled[ii] != 1 ){
	set_channel_color( ii + 1, 0 );
      }      
    }

  }
  
  //get projection
  memset( projection.data(), 0, sizeof(float) * projection.size() );
  project_grid();
  
}




/**
 * This function computes the topological network of a channel by
 * using homotopic thinning.
 *
 * Though in theory you could compute the network of the channels
 * inbetween the membranes, in most cases that would not make too much
 * sense, so so far only the innermost and outermost channels are
 * computed
 *
 * IMORTANT: make sure \ref set_grid() has been called after any
 * parameter change affecting the grid before calling this function to
 * make sure the distance map is up to date!
 */
void surface_projection::compute_channel_network(){


  update_containers();
  compute_projection();  
  // bring in the homotpoic thinning code
  dt.set_parameters( grid, std::vector<unsigned int> {n_points_x, n_points_y, n_points_z},
		     std::vector<double> {dx, dy, dz}, true);
  dt.compute_distance_map();
  
  

  homotopic_thinning<short> ht ( n_points_x, n_points_y, n_points_z, channel, dt.get_distance_map() );
  
  // resize container
  topological_network.resize( 2 );

  // do the work
  topological_network.at(0) = ht.find_channel_skeleton( 1 );
  topological_network.at(1) = ht.find_channel_skeleton( membranes.size()+1 );

  
}

/**
 * Computes the minimal diameter of the provided channel. Make sure
 * the distance map is up to date!
 *
 * \param[in] channel_id Choices: 0->inner channel, 1->outer channel
 */
double surface_projection::get_minimal_channel_diameter( int channel_id ){
  
  if( channel_id >= topological_network.size() ){
    return -1;
  } else {
    
    auto dmap = dt.get_distance_map();
    
    double min = std::numeric_limits<double>::max();
    for( auto it : topological_network.at( channel_id ) ) {

      
      if( sqrt( dmap[it] ) < min ){
	min = sqrt( dmap[it] );
      }
      
    }
    return min;
  }
}


/**
 * computes the maximum of the distance map in a channel
 */
template<class M>
M surface_projection::compute_max_dist_in_channel( std::vector<M> &dmap, int ch_id ){

  M max = std::numeric_limits<M>::min();

  for( unsigned int ii=0; ii < dmap.size(); ii++ ){
    
    if( std::fabs( channel[ii] ) == std::fabs( ch_id ) ){
      if( sqrt(dmap[ii]) > max ){
	max = sqrt(dmap[ii]);
      }
    }

  }

  return max;
}


/**
 * Performs a percolation threshold analysis in order to find the
 * thinnest channel diameter
 */
void surface_projection::compute_percolation_threshold( bool periodic ) {
  
  // make sure we have an updated distance map
  dt.set_parameters( grid, std::vector<unsigned int> {n_points_x, n_points_y, n_points_z},
		     std::vector<double> {dx, dy, dz}, periodic );
  dt.compute_distance_map();
  auto dmap = dt.get_distance_map();
  
  // get number of channels
  int ch_nr = int( get_membranes().size() / 2 ) + 1;
  percolation_thresholds.resize( ch_nr, 0 );

  // max pore sizes
  max_pore_radius.resize( ch_nr, 0 );
  
  percolation_analysis<short, float> perc ( get_channel(),
					    dmap,
					    n_points_x,
					    n_points_y,
					    n_points_z,
					    false );

  for( unsigned int ii=0; ii<ch_nr; ii++ ){

    // if the channel is filled solid, nothing can enter, so perc. threshold is 0
    if( channel_filled[2*ii] == 1 ){
      percolation_thresholds[ii] = 0;
      max_pore_radius[ii] = 0;
    } else {    
      percolation_thresholds[ii] = perc.get_percolation_threshold( (ii*2)+1 );
      max_pore_radius[ii] = double( compute_max_dist_in_channel( dmap, (ii*2)+1 ) );
    }
  }

}


/**
 * Returns the modulo of two floating point numbers. That's
 * numerically not a nice thing, since we have to take rounding errors
 * into account.
 */
inline double surface_projection::mod( double lhs, double rhs ){
    
  double divisor = lhs / rhs;

  //let's check if we're very close to the next integer
  double divisor_round  = round(divisor);

  //if we are, then let's round to that so we're exact again
  int divisor_int;
  if( std::fabs(divisor_round - divisor) < tolerance ){    
    divisor_int = int(divisor_round);
  } else {
    divisor_int = int(divisor);
  }
  
  double ret = lhs - (divisor_int * rhs);
  return ret;
}



/**
 * This function computes two additional base vectors which together
 * with the normal vector on the currenty hkl plane build up the
 * smalles possible unit cell given the current orientation
 *
 * A nifty way to do this is to interpret the normal vector on the
 * hkl-plane, as a 1x3 matrix:
 * 
 * <A*.n, A.xi> = 0 must hold, so that the xi's are in the hkl=plane
 * A made up of base vectors of direct lattice
 * A* made up of base vectors of reciprocal lattice
 * n^T A*^T A xi = n^T xi = 0 with A*^TA=1 (definition of reciprocal lattice)
 * 
 * then the two base vectors we are looking for is just the integer
 * kernel of the 1x3 matrix n^T
 *
 * This function is using the Integer Matrix Library, which computes
 * the kernel of a matrix and uses lattice reduction to find a
 * smallest, nearly orthogonal base.
 *
 */
void surface_projection::compute_smallest_uc( int reduce ){

  std::vector<long> n = { get_h(), get_k(), get_l() };

  // a pointer to write the result in as GMP datatyp
  mpz_t *kernel;
  
  // get the kernel base
  long kernel_dim = kernelLong( 1, 3, n.data(), &kernel, reduce );

  std::vector<int> x (3, 0), y (3, 0);
  
  // get the two base vectors
  x[0] = mpz_get_si( kernel[0] );
  x[1] = mpz_get_si( kernel[2] );
  x[2] = mpz_get_si( kernel[4] );

  y[0] = mpz_get_si( kernel[1] );
  y[1] = mpz_get_si( kernel[3] );
  y[2] = mpz_get_si( kernel[5] );

  // probably should use something like gmp_clear() here
  // free( kernel );
  
  // find true length and the longer vector
  std::vector<double> true_x = VEC_MAT_MATH::dot_prod( A_dir, x ); // direct lattice
  std::vector<double> true_y = VEC_MAT_MATH::dot_prod( A_dir, y ); // direct lattice
  std::vector<double> true_n = VEC_MAT_MATH::dot_prod( A_rec, n ); // reciprocal lattice!
  double len_x = sqrt( VEC_MAT_MATH::dot_prod( true_x, true_x ) );
  double len_y = sqrt( VEC_MAT_MATH::dot_prod( true_y, true_y ) );
  double len_n = sqrt( VEC_MAT_MATH::dot_prod( true_n, true_n ) );
  
  // get the angle bewteen the two in-plane vectors. Must use vectors in euclidead base!
  alpha = acos( VEC_MAT_MATH::dot_prod( true_x, true_y ) / (len_x * len_y) );
  
  // assign the longer base vector to b1, the shorter to b2
  if( len_x > len_y ){
    b1 = x; len_b1 = len_x;
    b2 = y; len_b2 = len_y;
  } else {
    b1 = y; len_b1 = len_y;
    b2 = x; len_b2 = len_x;
  }
  uc_dim_in_orientation[0] = len_b1; // periodicity length along longer in-plane vector
  uc_dim_in_orientation[1] = len_b2; // periodicity length along shorter in-plane vector
  

  /*
   * strategy: we need to find the closest lattice point x in direction
   * of the normal vector.
   */

  // direction
  true_n = VEC_MAT_MATH::get_unit( true_n );
  // lattice plane distance
  double plane_dist = 1.0 / len_n;
  
  // compute how many unit cells we need to go in each direction
  std::vector<int> pos = compute_periodicity( true_n, plane_dist, 2000 );


  if( pos.size() > 0 ){ // we found a lattice point in the direction
    std::vector<double> true_pos = VEC_MAT_MATH::dot_prod( A_dir, pos );
    uc_dim_in_orientation[2] = sqrt( VEC_MAT_MATH::dot_prod( true_pos, true_pos ) );
  } else { // did not found a reasonably sized unit cell
    uc_dim_in_orientation[2] = -1;
  }

}

/**
 * This functions sets appropriate slice dimensions which accomodate
 * one unit cell plus some margin so that the unit cell markings are
 * clearly visible
 */
void surface_projection::set_slice_to_uc( double margin ){

  // b2 is not orthogonal to b1 so compute the lenght orthogonal to b1
  double b2_x = std::fabs( cos(alpha) * uc_dim_in_orientation[1] );
  double b2_y = std::fabs( sin(alpha) * uc_dim_in_orientation[1] );

  // set unit cell dimensions plus the margin
  if( uc_dim_in_orientation[2] > 0 ){
    set_slice_thickness( uc_dim_in_orientation[2] );
  }
  set_slice_width( (1.0+margin) * ( uc_dim_in_orientation[0] + b2_x ) );
  set_slice_height( (1.0+margin) * b2_y );  
}


void surface_projection::set_slice_to_primitive_uc(){
  h = 0; k = 0; l = 1;
  L = a;
  slice_position = 0.0;
}

/**
 * This function computes the periodicity length in a given
 * direction. It moves along the normal vecors by steps of the lattice
 * plane distance and checks if the current point is a lattice point
 * of the direct lattice. The function stops at a cutoff value.
 *
 * \param[in] The direction in which to look
 * \param[in] The distance between lattice planes
 * \param[in] The maximum length
 */
std::vector<int> surface_projection::compute_periodicity( std::vector<double> n,
							  double d_lattice,
							  double cutoff  ){

  // get the unit vector
  n = VEC_MAT_MATH::get_unit( n );

  // the position of the closest lattice point in direction of the
  // normal vector
  std::vector<int> pos (0, 0);
  
  //stop after some steps, rather arbitrary number
  long long max_steps = int( cutoff / d_lattice ) + 1;

  //count how many steps we need
  long long step_count = 1;

  bool more_steps = true;

  //increase steps, scale normal vector and see if we are at a
  //periodic origin again
  while( step_count < max_steps && more_steps){

    double xx = step_count * d_lattice * n[0];
    double yy = step_count * d_lattice * n[1];
    double zz = step_count * d_lattice * n[2];

    double ddxx = mod(xx, a[0]);
    double ddyy = mod(yy, a[1]);
    double ddzz = mod(zz, a[2]);
    
    if( std::fabs( ddxx ) < tolerance &&
	std::fabs( ddyy ) < tolerance &&
	std::fabs( ddzz ) < tolerance  ){

      pos.resize( 3, 0);
      pos[0] = int( round( xx / a[0] ) );
      pos[1] = int( round( yy / a[1] ) );
      pos[2] = int( round( zz / a[2] ) );

      more_steps = false;
      break;
    }

    //not there yet, keep on going
    step_count++;
  }

  return pos;
}
  

/**
 * This function basically only converts the projection array to a
 * rescaled array, where the minimal value is set to 0 and the maximum
 * to 255. Also inverts if necessary. Memory management could be
 * imporved I guess
 */
unsigned char* surface_projection::get_image(bool invert, std::string scaling){

  //new image array
  unsigned char* img = new unsigned char[ projection.size() ]();
  //(unsigned char*) malloc( sizeof(unsigned char) * projection.size() );
  
  //find min and max value
  float max= *std::max_element( projection.begin(), projection.end() );
  float min= *std::min_element( projection.begin(), projection.end() );
  
  if( scaling == "LOG" ){
    max = log( max + 1 );
    min = log( min + 1 );    
  }

  //write image data
  for(unsigned int ii=0; ii<projection.size(); ii++){
    //scale
    float pixel_val;
    unsigned char u_scaled;

    if( scaling == "LIN" ){
      pixel_val = ((projection[ii] - min)/(max - min)*255);
    } else if( scaling == "LOG" ){
      pixel_val = log(projection[ii]+1);
      pixel_val = ((pixel_val - min)/(max - min))*255;
    }
    u_scaled = static_cast<unsigned char>( pixel_val );
      //invert
    if(invert)
      u_scaled = 255 - u_scaled;
    //write in array
    img[ii] = u_scaled;
  }  

  return img;  
}


#ifdef HAVE_PNG

void surface_projection::save_to_png( std::string out_fn, bool invert, std::string scaling ){

  unsigned char *img = get_image( invert, scaling );
  write_png( img, get_width(), get_height(), out_fn );
  
}

/**
 * writes the current projection to a png image
 */
void surface_projection::write_png( unsigned char *img,
				    unsigned int width,
				    unsigned int height,
				    std::string out_fn ){

  //unsigned char *img = get_image( invert, scaling );
  
  // convert 1d to 2d array
  png_bytep *rows = new png_bytep[height];
  // set up png array
  for( unsigned int ii=0; ii < height; ii++ ){
    rows[ii] = img + ii * width * sizeof( png_byte );
  }
  
  FILE *fp = fopen( out_fn.c_str(), "wb" );
  if( !fp ){
    throw std::string ("error opening file");
  }

  png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
  png_infop png_info = png_create_info_struct( png_ptr );

  png_init_io(png_ptr, fp);
  
  png_set_IHDR(png_ptr, png_info, width, height,
	       8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
	       PNG_COMPRESSION_TYPE_DEFAULT,
	       PNG_COMPRESSION_TYPE_DEFAULT);

  png_write_info( png_ptr, png_info );
  png_write_image( png_ptr, rows );

  png_write_end( png_ptr, png_info );

  png_destroy_write_struct(&png_ptr, &png_info);
  
  fclose(fp);

  delete[]( rows );
  delete[]( img );
}

#endif

/** 
 * adds a membrane to the membrane array
 *
 *\param[in] dist The distance to the "original" membrane
 *\param[in] width The width of the membrane
 */
void surface_projection::add_membrane(double dist, double width ){
  membranes.push_back( dist );
  membranes.push_back( width );  
}


/**
 * sets the "color" of a membrane. This means all voxels belonging to
 * that membrane are set to "0" or "1". This way it can be chosen, if
 * the channel contributes towards the projection
 *
 * \param[in] mem_id the id of the channel to color (starting at 1)
 * \param[in] val the value to set the channel voxels to (0,1)
 */
void surface_projection::set_channel_color( int mem_id, int val ){

  channel_filled[mem_id-1] = val;
  
  for( unsigned int ii=0; ii<channel.size(); ii++ ){
    if( std::abs( channel[ii]) == std::abs( mem_id ) ){
      grid[ii] = val;
    }
  }  
}


/**
 * merges channels which are not separated by a membrane
 */
void surface_projection::merge_adjacent_channels(){

  std::unordered_map<int, int> identify;

  // holds the new channel_fill information
  std::vector<int> channel_filled_new;
  channel_filled_new.push_back( channel_filled[0] );

  // the new membranes array, this won't hold the correct values,
  // however, will have the right amount of membranes (entries)
  std::vector<double> membranes_new;

  int counter=0; // new number of channels
  for( int ii=0; ii<channel_filled.size(); ii++){
    
    if( ii>0 ){
      if( channel_filled[ii-1] != channel_filled[ii] ){ // new channel
	channel_filled_new.push_back( channel_filled[ii] );
	counter++;

	// check if we need to add a membrane
	if( ii % 2 == 1 ){ //yes!
	  membranes_new.push_back( membranes[ ii-1 ] );
	  membranes_new.push_back( membranes[ ii ] );	  
	}	
      }
    }
    
    identify.emplace( ii+1, counter+1 );
  }

  // update the channel information, just put everything positive,
  // shouldn't matter for percolation analysis
  for( unsigned int ii=0; ii<channel.size(); ii++ ){    
    channel[ii] = identify[ std::abs( channel[ii] ) ];
  }

  set_channel_fill( channel_filled_new );
  set_membranes( membranes_new );
  
}

/**
 * This function computes the volumes of the channels. It is just
 * adding up the volumes of the appropriate voxels
 *
 */
void surface_projection::compute_volume(){
  
  volumes = std::vector<double> (membranes.size() + 1, 0 );
  
  double vox_vol = dx*dy*dz;
  int ind;
  
  for( unsigned int ii=0; ii<channel.size(); ii++){
    
    ind = channel[ii];
    if( ind < 0 ) ind*=-1;
    volumes[ind-1]+=vox_vol;
  }
}


/**
 * computes the surface area of the membranes. Uses CGAL, in CASE
 * that's not found, it's counting voxel faces - a indeed crude
 * approximation. This works best - or rather works only if the voxels
 * are cubes, i.e. are having the same length in all three spatial
 * dimensions.
 *
 * In future I will probably implement a lookup table, if CGAL is not
 * available
 *
 * TODO: find a good choice for the peremiter parameter!
 *
 */
void surface_projection::compute_surface_area(){
  
  surface_area = std::vector<double> ( int( 0.5*membranes.size() ), 0 );

#ifdef USE_CGAL


  for( unsigned int ii=0; ii<int(0.5*membranes.size()); ii++){
    
    // get all the points making  up the membrane
    auto membrane_points = get_surface_points( (2*ii)+1, 6 );
    
    //bring in some random numbers!
    double rand_mag = 1e-4;
    std::mt19937 rand_gen;
    std::normal_distribution<double> rand_dist_x ( 0, rand_mag * get_dx() );
    std::normal_distribution<double> rand_dist_y ( 0, rand_mag * get_dy() );
    std::normal_distribution<double> rand_dist_z ( 0, rand_mag * get_dz() );
    
    // copy and paste the points into a CGAL compatible array
    std::vector<Point_3> CGAL_membrane_points;
    for(int jj=0; jj<membrane_points.size(); jj++){
      double x = points.at( 3 * membrane_points.at(jj) );
      double y = points.at( 3 * membrane_points.at(jj) + 1 );
      double z = points.at( 3 * membrane_points.at(jj) + 2 );      
      
      // since the Delauny triangulation doesn't like regularily space
      // points, we will give disperse all of them a tiny bit
      x += rand_dist_x( rand_gen );
      y += rand_dist_y( rand_gen );
      z += rand_dist_z( rand_gen );
      
      CGAL_membrane_points.push_back( Point_3( x, y, z) );
    }
    
    // get the surface triangulation
    std::vector<Facet> facets;
    Perimeter perimeter( 0.5 * uc_scale_ab );
    CGAL::advancing_front_surface_reconstruction(CGAL_membrane_points.begin(),
						 CGAL_membrane_points.end(),
						 std::back_inserter(facets),
						 perimeter);
    
    //compute surface area from triangulation
    double total_area = 0;
    
    for( auto it : facets ){
      Vector_3 ab = CGAL_membrane_points.at( it[0] ) - CGAL_membrane_points.at( it[1] );
      Vector_3 ac = CGAL_membrane_points.at( it[0] ) - CGAL_membrane_points.at( it[2] );    
      Vector_3 cross = CGAL::cross_product( ab, ac );
      double area = 0.5 * sqrt( cross.squared_length() );
      total_area+=area;
    }

    //safe area
    surface_area[ii] = total_area;

    
    
    // for debugging: output surfaces
    /*
    std::ofstream triang_out ( "membrane_" + std::to_string(ii) + ".off" );
    triang_out << "OFF\n" << CGAL_membrane_points.size() << " " << facets.size() << " 0\n";
    std::copy(CGAL_membrane_points.begin(),
	      CGAL_membrane_points.end(),
	      std::ostream_iterator<Point_3>(triang_out, "\n"));
    std::copy(facets.begin(),
	      facets.end(),
	      std::ostream_iterator<Facet>(triang_out, "\n"));
    triang_out.close();
    */    
  }

#else  

  iterable_voxel p ( 0, get_width(), get_height(), get_depth() );
  
  // iterate over all cells, and check if they are adjacent to a cell
  // of different channel. Always check front, up, and right
  for( unsigned int ii=0; ii<grid.size(); ii++){

    //only check membrane channels
    int ch_id = channel.at( ii );
    if( ch_id < 0 ) ch_id*=-1;
    if( ch_id % 2 == 0 ){

      p.set( ii );
      
      int u = channel.at( p.u()()  );
      int f = channel.at( p.f()() );
      int r = channel.at( p.r()() );
      int d = channel.at( p.d()()  );
      int b = channel.at( p.b()() );
      int l = channel.at( p.l()() );    
      
      if( u < 0) u *= -1;
      if( f < 0) f *= -1;
      if( r < 0) r *= -1;
      if( d < 0) d *= -1;
      if( b < 0) b *= -1;
      if( l < 0) l *= -1;           
      

      int ind = (ch_id - 2) / 2;
      
      if( ch_id > u ){
	surface_area.at( ind ) += dx*dy;
      }
      if( ch_id > f ){
	surface_area.at( ind ) += dx*dz;
      }
      if( ch_id > r ){
	surface_area.at( ind ) += dy*dz;
      }    
      if( ch_id > d ){
	surface_area.at( ind ) += dx*dy;
      }
      if( ch_id > b ){
	surface_area.at( ind ) += dx*dz;
      }
      if( ch_id > l ){
	surface_area.at( ind ) += dy*dz;
      }     

    }


  }
#endif
}

/*
 * This function finds all the points which make up the interface to
 * the next, outer membrane
 */
std::vector<int> surface_projection::get_surface_points( int ch_id, int n ){

  std::vector<int> interface_points;

  for( unsigned int ii=0; ii<channel.size(); ii++){

    if( std::abs(channel[ii]) == std::abs( ch_id ) ){
    
      iterable_voxel point ( ii, n_points_x, n_points_y, n_points_z );
      
      std::vector<int> nbs;
      if( n == 6 ){
	point.get_6_neighbors(nbs);
      } else if( n == 18 ){
	point.get_18_neighbors(nbs);
      } else if ( n == 26 ){
	point.get_26_neighbors(nbs);
      } else {
	throw std::string("no adjancy model found for n=" + std::to_string(n) );
      }

      bool found_background = false;
      for( auto it : nbs ){
	if( std::abs( channel[it] ) > std::abs( ch_id ) ){
	  found_background = true;
	  break;
	}
      }
      
      
      if( found_background ){
	interface_points.push_back( point() );
      }

    }
  }

  return interface_points;
  
}






/*************************
 *  Getters
 *************************/

std::vector<short> surface_projection::get_channel() const {
  return channel;
}

std::vector<double> surface_projection::get_channel_volumes() const {
  return volumes;
}

std::vector<double> surface_projection::get_percolation_thresholds() const {
  return percolation_thresholds;
}

std::vector<double> surface_projection::get_max_pore_radius() const {
  return max_pore_radius;
}

std::vector< std::unordered_set<int> > surface_projection::get_channel_network() const {
  return topological_network;
}

std::vector<double> surface_projection::get_membrane_surface_area() const {
  return surface_area;
}

std::vector<float> surface_projection::get_distance_map() const {
  return dt.get_distance_map();
}

std::vector<double> surface_projection::get_membranes() const {
  return membranes;
}

std::vector<float> surface_projection::get_projection() const {
  return projection;
}

int surface_projection::get_width() const {
  return n_points_x;
}

int surface_projection::get_height() const {
  return n_points_y;
}

int surface_projection::get_depth() const {
  return n_points_z;
}

const std::vector<std::string> surface_projection::get_surface_choices(){
  return surface_choices;
}

const std::vector<std::string> surface_projection::get_img_scaling_choices(){
  return img_scaling_choices;
}

int surface_projection::get_h() const{
  return h;
}

int surface_projection::get_k() const{
  return k;
}

int surface_projection::get_l() const{
  return l;
}

int surface_projection::get_type() const {
  return type;
}

std::vector<double> surface_projection::get_a() const{
  return a;
}

double surface_projection::get_uc_scale_ab() const {
  return uc_scale_ab;
}

double surface_projection::get_uc_scale_c() const {
  return uc_scale_c;
}

double surface_projection::get_surface_level() const {
  return surface_level;
}

double surface_projection::get_channel_vol_prop() const {
  double val =  s_tables.get_prop( surface_choices[type], surface_level );
  return val;
}

double surface_projection::get_slice_thickness() const {
  return L[2];
}

double surface_projection::get_slice_height() const {
  return L[1];
}

double surface_projection::get_slice_width() const {
  return L[0];
}

double surface_projection::get_slice_position() const {
  return slice_position;
}

std::vector<double> surface_projection::get_L() const {
  return L;
}

double surface_projection::get_theta() const{
  return theta;
}

double surface_projection::get_phi() const{
  return phi;
}

double surface_projection::get_dx() const{
  return dx;
}

double surface_projection::get_dy() const{
  return dy;
}

double surface_projection::get_dz() const{
  return dz;
}

std::vector<double> surface_projection::get_uc_dim_in_orientation() const{
  return uc_dim_in_orientation;
}

std::vector<short> surface_projection::get_grid() const {
  return grid;
}

std::vector<float> surface_projection::get_points() const {
  return points;
}


/*************************
 * Setters
 *************************/

void surface_projection::edit_membrane( int id, double dist, double width ){

  if( 2*id >= membranes.size() ){
    throw invalid_parameter_exception( "Membrane not found" );
  } else if( width < 0 ){    
    throw invalid_parameter_exception( "Membrane width can't be negative" );
  } else if ( id == 0 && width < 0.02 ) {
    membranes[2*id + 1] = 0.02;
    throw invalid_parameter_exception( "Width of main membrane can't be less than 0.02" );
  } else {
    if( id == 0 && dist != 0 ){
      throw invalid_parameter_exception( "Main membrane distance must be 0" );
    }
    membranes[2*id] = dist;
    membranes[2*id + 1] = width;
  }

}

void surface_projection::delete_membrane( int id ){
  
  if( 2*id >= membranes.size() ){
    throw invalid_parameter_exception( "Membrane not found" );
  } else if ( id == 0 ){
    throw invalid_parameter_exception( "Main membrane can't be deleted" );
  } else {
    membranes.erase( membranes.begin() + 2*id, membranes.begin()+(2*id+2) );
  }
  
}

void surface_projection::set_membranes( std::vector<double> mems ){
  membranes = mems;
  update_channel_fill_container();
}

void surface_projection::set_channel_fill( std::vector<int> fills ){
  if( fills.size() == membranes.size() + 1 ){
    channel_filled = fills;
  }
}

void surface_projection::update_channel_fill_container(){

  // clear previous data
  channel_filled.clear();
  channel_filled.resize( membranes.size() + 1, 0 );

  for( unsigned int ii=0; ii<channel_filled.size(); ii++){
    if( ii % 2 == 1 )
      channel_filled[ii] = 1;
  }
  
}

void surface_projection::set_theta( double ang ){
  if( ang > std::numbers::pi ){
    theta  = std::numbers::pi - tolerance;
    throw invalid_parameter_exception( "Angle too large" );
  } else if( ang < 0 ) {
    theta = 0;
    throw invalid_parameter_exception( "No negative angles allowed" );
  } else {
    theta = ang;
  }
}

void surface_projection::set_phi( double ang ){
  if( ang > 2*std::numbers::pi ){
    phi  = 2*std::numbers::pi - tolerance;
    throw invalid_parameter_exception( "Angle too large" );
  } else if( ang < 0 ) {
    phi = 0;
    throw invalid_parameter_exception( "No negative angles allowed" );
  } else {
    phi = ang;
  }
}

void surface_projection::set_type(int val ){
  if( val < 0 || val >= surface_choices.size() ){
    val = 0;
  } else {
    type = val;
  }
}

void surface_projection::set_slice_thickness ( double val ){
  if( val <= 0 ){
    throw invalid_parameter_exception("Slice thickness can't be negative or zero");
  } else {
    L[2] = val;
  }
}


void surface_projection::set_slice_width ( double val ){
  if( val <= 0 ){
    throw invalid_parameter_exception("Slice width can't be negative or zero");
  } else {
    L[0] = val;
  }
}


void surface_projection::set_slice_height ( double val ){
  if( val <= 0 ){
    throw invalid_parameter_exception("Slice height can't be negative or zero");
  } else {
    L[1] = val;
  }
}

void surface_projection::set_slice_position ( double val ){
  slice_position = val;
}


/*
 * this function is a wrapper for the surface_level parameter. Since
 * this is a rather technical/mathematical parameter, this function
 * takes as an argument the porortion of the *inner* channel and looks
 * up/interpolates the appropriate surface_level value in a table
 */
void surface_projection::set_channel_vol_prop( double vol ){
    surface_level = s_tables.get_level( surface_choices[ type ], vol );
}



void surface_projection::set_surface_level( double val ){
  surface_level = val;
}

void surface_projection::set_uc_scale_ab( double val ){
  uc_scale_ab = val;
}

void surface_projection::set_uc_scale_c( double val ){
  uc_scale_c = val;
}

/**
 * This function checks if the scaling of the unit cell is compatible
 * with the symmetry. Essentially 
 */
void surface_projection::validate_uc_scaling(){
  if( type <= 2 ){
    uc_scale_c = uc_scale_ab;
  }
}


/** 
 * updates the size of the unitcell given in length units as well as
 * the matrices holding the base vectors for the direct and reciprocal
 * lattice
 *
 * Todo: right now only rectangluar unitcells are allowed, in which
 * case the row and column vectors are identical, which is used down
 * here. For a general base we need to change the appropriate lines
 * below
 */
void surface_projection::update_a(){
    a[0] = uc_scale_ab * unitcell_dim[type][0];
    a[1] = uc_scale_ab * unitcell_dim[type][1];
    a[2] = uc_scale_c * unitcell_dim[type][2];
    inv_a[0] = 2*std::numbers::pi/(a[0]); // period for nodal representations
    inv_a[1] = 2*std::numbers::pi/(a[1]); // period for nodal representations        
    inv_a[2] = 2*std::numbers::pi/(a[2]); // period for nodal representations


    /*
     * Update base vector matrices
     */
    
    // matrix for direct lattice
    A_dir.v = { a[0],    0,    0 };
    A_dir.w = {    0, a[1],    0 };
    A_dir.z = {    0,    0, a[2] };
    
    double scale = (1.0 / VEC_MAT_MATH::dot_prod( A_dir.v, VEC_MAT_MATH::cross_prod(A_dir.w, A_dir.z) ) );
    
    //update matrices
    std::vector<double> a_star = VEC_MAT_MATH::cross_prod( A_dir.w, A_dir.z );
    std::vector<double> b_star = VEC_MAT_MATH::cross_prod( A_dir.z, A_dir.v );
    std::vector<double> c_star = VEC_MAT_MATH::cross_prod( A_dir.v, A_dir.w );
    a_star = VEC_MAT_MATH::s_prod( scale, a_star );
    b_star = VEC_MAT_MATH::s_prod( scale, b_star );
    c_star = VEC_MAT_MATH::s_prod( scale, c_star );    

    A_rec.v = a_star;
    A_rec.w = b_star;
    A_rec.z = c_star;
}


void surface_projection::set_n_points_x( int val ){
  if( val <= 0){
    n_points_x = 1;
    //recompute the resolution
    dx = L[0] / n_points_x;
    throw invalid_parameter_exception("n_points_x: Must have a minimum of 1 point");
  } else {
    n_points_x = val;
    //recompute the resolution
    dx = L[0] / n_points_x;
  }
}

void surface_projection::set_n_points_y( int val ){
  if( val <= 0){
    n_points_y = 1;
    //recompute the resolution
    dy = L[1] / n_points_y;      
    throw invalid_parameter_exception("n_points_y: Must have a minimum of 1 point");
  } else {    
    n_points_y = val;
    //recompute the resolution
    dy = L[1] / n_points_y;      
  }   
}

void surface_projection::set_n_points_y_to_unitcell(){

  int val = n_points_x * ( L[1] / L[0] );

  if( val <= 0){
    n_points_y = 1;
    //recompute the resolution
    dy = L[1] / n_points_y;      
    throw invalid_parameter_exception("n_points_y: Must have a minimum of 1 point");
  } else {    
    n_points_y = val;
    //recompute the resolution
    dy = L[1] / n_points_y;      
  }   
}

void surface_projection::set_n_points_z( int val ){
  if( val <= 0){
    n_points_z = 50;
    //recompute the resolution
    dz = L[2] / n_points_z;      
    throw invalid_parameter_exception("n_points_z: Must have a minimum of 1 point");
  } else {
    n_points_z = val;
    //recompute the resolution
    dz = L[2] / n_points_z;      
  }
}

void surface_projection::set_n_points_z_to_unitcell(){

  int val = n_points_x * ( L[2] / L[0] );

  if( val <= 0){
    n_points_y = 50;
    //recompute the resolution
    dz = L[2] / n_points_z;
    throw invalid_parameter_exception("n_points_z: Must have a minimum of 1 point");
  } else {    
    n_points_z = val;
    //recompute the resolution
    dy = L[2] / n_points_z;
  }   
}



void surface_projection::set_h( int val ){
  if( val == 0 && k == 0 && l == 0){
    h = 1;
    throw invalid_parameter_exception("At least one of hkl must be non-zero");
  }
  h = val;
}

void surface_projection::set_k( int val ){
  if( h == 0 && val == 0 && l == 0){
    k = 1;
    throw invalid_parameter_exception("At least one of hkl must be non-zero");
  }
  k = val;
}

void surface_projection::set_l( int val ){
  if( h == 0 && k == 0 && val == 0){
    l = 1;
    throw invalid_parameter_exception("At least one of hkl must be non-zero");
  }
  l = val;
}



/**
 * A wrapper function, calling the setter functions of the parameter
 * identified by the string. This should only be used if absolutely
 * necessary, since type safety is weakened!
 *
 * The string identifier should only be taken from the array \ref parameter_names
 *
 * \param[in] par The string identifying the parameter
 * \param[in] val The value to set to
 */
void surface_projection::set_parameter( std::string par, double val ){

  if( par == parameter_names[0] ){
    set_type( static_cast<int>( round( val ) ) );
  }
  
  if( par == parameter_names[1] ){
    set_uc_scale_ab( val );
  }
  if( par == parameter_names[2] ){
    set_uc_scale_c( val );
  }  
  if( par == parameter_names[3] ){
    set_surface_level( val );
  }
  if( par == parameter_names[4] ){
    set_channel_vol_prop( val );
  }
  if( par == parameter_names[5] ){
    set_slice_thickness( val );
  }
  if( par == parameter_names[6] ){
    set_slice_height( val );
  }
  if( par == parameter_names[7] ){
    set_slice_width( val );
  }
  if( par == parameter_names[8] ){
    set_slice_position( val );
  }  

  if( par == parameter_names[9] ){
    set_h( static_cast<int>( round( val ) ) );
  }
  if( par == parameter_names[10] ){
    set_k( static_cast<int>( round( val ) ) );
  }
  if( par == parameter_names[11] ){
    set_l( static_cast<int>( round( val ) ) );
  }

  // setting all membrane width to the new value!
  if( par == parameter_names[12] ){
    for( unsigned int ii=1; ii<membranes.size(); ii+=2 ){
      membranes.at(ii) = val;
    }
  }
  
}



/**
 * A wrapper function, calling the getter functions of the parameter
 * identified by the string. This should only be used if absolutely
 * necessary, since type safety is weakened!
 *
 * The string identifier should only be taken from the array \ref parameter_names
 *
 * \param[in] par The string identifying the parameter
 */
double surface_projection::get_parameter( std::string par ){

  if( par == parameter_names[0] ){
    return static_cast<double> ( get_type() );
  }
  
  if( par == parameter_names[1] ){
    return get_uc_scale_ab();
  }
  if( par == parameter_names[2] ){
    return get_uc_scale_c();
  }  
  if( par == parameter_names[3] ){
    return get_surface_level();
  }
  if( par == parameter_names[4] ){
    return get_channel_vol_prop();
  }
  

  if( par == parameter_names[5] ){
    return get_slice_thickness();
  }
  if( par == parameter_names[6] ){
    return get_slice_height();
  }
  if( par == parameter_names[7] ){
    return get_slice_width();
  }
  if( par == parameter_names[8] ){
    return get_slice_position();
  }  

  if( par == parameter_names[9] ){
    return static_cast<double> ( get_h() );
  }
  if( par == parameter_names[10] ){
    return static_cast<double> ( get_k() );    
  }
  if( par == parameter_names[11] ){
    return static_cast<double> ( get_l() );    
  }


  // this one is sketchy beacuse only one membrane width can be
  // returned. So we aim for the main membrane at d=0
  if( par == parameter_names[12] ){
    //find main membrane
    for( unsigned int ii=0; ii<membranes.size(); ii+=2 ){
      if( membranes.at(ii) == 0 ){
	return membranes.at(ii+1);
      }
    }
  }

  return 0;
  
}






std::vector<double> surface_projection::get_ucdim() const{
  return unitcell_dim[type];
}

std::vector<int> surface_projection::get_channel_fill() const{
  return channel_filled;
}

/**
 * Returns the base vectors of the unit cell in Euclidean base as a 1d
 * array in column-major order.
 */
std::vector<double> surface_projection::get_uc_base() const {

  std::vector<int> n = {h, k ,l};
  std::vector<double> a = VEC_MAT_MATH::dot_prod( A_dir, b1 );
  std::vector<double> b = VEC_MAT_MATH::dot_prod( A_dir, b2 );
  std::vector<double> c = VEC_MAT_MATH::dot_prod( A_rec, n );
  
  return std::vector<double> {a[0], a[1], a[2], b[0], b[1], b[2], c[0], c[1], c[2]};
}
