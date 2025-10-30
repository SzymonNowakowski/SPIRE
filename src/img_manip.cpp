#include "img_manip.hpp"


image_manipulation::image_manipulation(){

}


double symm_gaussian_2D( double x, double y, double sigma ){
  
  return (1.0/( 2 * M_PI * sigma * sigma ) ) * exp( -(x*x + y*y) / (2*sigma*sigma)   );
  
}


void image_manipulation::gaussian_blur( unsigned char* img, unsigned int width, unsigned height, unsigned int kernel_size ){

  // create a new intermediate image

  unsigned char* soft_img = new unsigned char[width*height]();

  
  // first compute the convolution function (gaussian)

  // choose a 3sigma cutoff
  int conv_size = 6*kernel_size;
  if( conv_size % 2 == 0 ) conv_size++;
  int cv2 = int(conv_size*0.5); // half convolution size

  double* conv_func = new double[conv_size*conv_size]();

  double sum = 0;
  for( int ii=0; ii < conv_size; ii++ ){
    for( int jj=0; jj < conv_size; jj++ ){
      conv_func[ii+conv_size*jj] = symm_gaussian_2D( (int(0.5*conv_size) - ii), (int(0.5*conv_size) - jj), kernel_size );
      sum+=conv_func[ii+conv_size*jj];
    }
  }

  // normalize
  for( int ii=0; ii < conv_size*conv_size; ii++ ){
    conv_func[ii] /= sum;
  }


  // apply the gaussian filter in the width dim first

  // iterate over all pixles
  for( unsigned int yy = 0; yy < height; yy++){
    for( unsigned int xx = 0; xx < width; xx++){

      double pix_val = 0;      

      // do the convolution
      for( unsigned cx = 0; cx < conv_size; cx++){
	for( unsigned cy = 0; cy < conv_size; cy++){

	  int ind_x = xx - cv2 + cx;
	  int ind_y = yy - cv2 + cy;
	  // pbc
	  while( ind_x < 0 )
	    ind_x += width;
	  while( ind_x >= width )
	    ind_x -= width;

	  while( ind_y < 0 )
	    ind_y += height;
	  while( ind_y >= height )
	    ind_y -= height;

	  // total index in 1d array
	  int ind = ind_y * width + ind_x;
	  
	  pix_val += conv_func[cx*conv_size + cy] * img[ind];
	}
      }
      soft_img[ yy*width + xx ] = (unsigned char ) pix_val;      
    }
  }

  // blend images, for now it's just replaced, could get fancier in future
  for( unsigned int i=0; i < width*height; i++){
    img[i] = soft_img[i];
  }

  delete[] (soft_img);
}


void image_manipulation::gaussian_noise( unsigned char* img, unsigned int width, unsigned int height, double magnitude ){
  
  // bring in the random numbers
  std::mt19937 generator;
  auto seed = std::chrono::system_clock::now().time_since_epoch().count();
  generator.seed( seed );
  std::normal_distribution<double> dist (0, 1.0/6.0);

  // create a new image
  short *noisy_img = new short[width*height]();
  
  for( unsigned int ii=0; ii< width*height; ii++){
    noisy_img[ii] = img[ii] + static_cast<short>( dist(generator) * magnitude);
  }

  //rescale
  short min=0, max=0;
  for( unsigned int i=0; i < width*height; i++){
    if( noisy_img[i] > max )
      max = noisy_img[i];
    if( noisy_img[i] < min )
      min = noisy_img[i];
  }

  
  for( unsigned int i=0; i < width*height; i++){
    img[i] = (unsigned char) (((float)noisy_img[i] - min) / (max - min) * 255.0);    
  }

  delete[] (noisy_img);
}




void image_manipulation::add_grains( unsigned char* img, unsigned int width, unsigned int height,
				     int grain_size_center, int grain_size_width,
				     int grain_number_center, int grain_number_width,
				     double magnitude ){
  
  // bring in the random numbers
  std::mt19937 generator;
  auto seed = std::chrono::system_clock::now().time_since_epoch().count();
  generator.seed( seed );

  std::normal_distribution<double> dist (0, 1.0/6.0);

  std::uniform_int_distribution<int> uniform_w (0, width);
  std::uniform_int_distribution<int> uniform_h (0, height);

  std::normal_distribution<double> grain_number_dist (grain_number_center, grain_number_width);
  std::normal_distribution<double> grain_size_dist (grain_size_center, grain_size_width);  
  
  // create a new image and copy data
  short *noisy_img = new short[width*height]();
  for( unsigned int i=0; i<width*height; i++){
    noisy_img[i] = img[i];
  }
  
  unsigned int N = grain_number_dist( generator );
  
  for( unsigned int ii=0; ii < N; ii++){

    int grain_size = std::abs( grain_size_dist( generator ) );
    
    // pick a random location on the image
    int cx = uniform_w( generator );
    int cy = uniform_h( generator );
    
    //pick a random intensity
    double intensity = magnitude * dist( generator );
    
    for( int yy=-grain_size; yy <= grain_size; yy++ ){
      int xb = sqrt( grain_size*grain_size - yy*yy );
      for( int xx = -xb; xx<=xb; xx++){

	int ind_x = cx + xx;
	int ind_y = cy + yy;

	while( ind_x < 0 )
	  ind_x += width;
	while( ind_x >= width )
	  ind_x -= width;
	while( ind_y < 0 )
	  ind_y += height;
	while( ind_y >= height )
	  ind_y -= height;

	int ind = ind_x + ind_y * width;	

	noisy_img[ind] += intensity;
      }
    }
  }
  

  //rescale
  short min=0, max=0;
  for( unsigned int i=0; i < width*height; i++){
    if( noisy_img[i] > max )
      max = noisy_img[i];
    if( noisy_img[i] < min )
      min = noisy_img[i];
  }

  for( unsigned int i=0; i < width*height; i++){
    img[i] = (unsigned char) (((float)noisy_img[i] - min) / (max - min) * 255.0);    
  }

  delete[] (noisy_img);
}






/*
 * this is not very nice, since this a lot of copy&paste from the code above
 */
unsigned char* image_manipulation::create_grains( unsigned int width, unsigned int height,
					  int grain_size_center, int grain_size_width,
					  int grain_number_center, int grain_number_width,
					  double magnitude ){
  
  // bring in the random numbers
  std::mt19937 generator;
  auto seed = std::chrono::system_clock::now().time_since_epoch().count();
  generator.seed( seed );

  std::normal_distribution<double> dist (0, 1.0/6.0);

  std::uniform_int_distribution<int> uniform_w (0, width);
  std::uniform_int_distribution<int> uniform_h (0, height);

  std::normal_distribution<double> grain_number_dist (grain_number_center, grain_number_width);
  std::normal_distribution<double> grain_size_dist (grain_size_center, grain_size_width);  
  
  // create a new image and copy data
  short *noisy_img = new short[width*height]();
  
  unsigned int N = grain_number_dist( generator );
  
  for( unsigned int ii=0; ii < N; ii++){

    int grain_size = std::abs( grain_size_dist( generator ) );
    
    // pick a random location on the image
    int cx = uniform_w( generator );
    int cy = uniform_h( generator );
    
    //pick a random intensity
    double intensity = magnitude * dist( generator );
    
    for( int yy=-grain_size; yy <= grain_size; yy++ ){
      int xb = sqrt( grain_size*grain_size - yy*yy );
      for( int xx = -xb; xx<=xb; xx++){

	int ind_x = cx + xx;
	int ind_y = cy + yy;

	while( ind_x < 0 )
	  ind_x += width;
	while( ind_x >= width )
	  ind_x -= width;
	while( ind_y < 0 )
	  ind_y += height;
	while( ind_y >= height )
	  ind_y -= height;

	int ind = ind_x + ind_y * width;	

	noisy_img[ind] += intensity;
      }
    }
  }


  //rescale
  short min=0, max=0;
  for( unsigned int i=0; i < width*height; i++){
    if( noisy_img[i] > max )
      max = noisy_img[i];
    if( noisy_img[i] < min )
      min = noisy_img[i];
  }

  unsigned char *img = new unsigned char[width*height]();
  
  for( unsigned int i=0; i < width*height; i++){
    img[i] = (unsigned char) (((float)noisy_img[i] - min) / (max - min) * 255.0);    
  }


  delete[]( noisy_img );

  
  return img;
}










void image_manipulation::add_images( unsigned char* rhs, unsigned char* lhs,
				     double rhs_weight, double lhs_weight,
				     unsigned int width, unsigned int height,
				     unsigned char* dest ){
  
  
  unsigned short max=0, min=255;

  
  unsigned short *tmp_img = new unsigned short[width*height]();
  
  for( unsigned int ii=0; ii< width*height; ii++){    
    tmp_img[ii] = rhs_weight * rhs[ii] + lhs_weight * lhs[ii];
    
    if( tmp_img[ii] > max )
	max = tmp_img[ii];
    
    if( tmp_img[ii] < min )
      min = tmp_img[ii];
  }


  for( unsigned int ii=0; ii< width*height; ii++){
    dest[ii] = (unsigned char) (((float)tmp_img[ii] - min) / (max - min) * 255.0 );
  }
  
  delete[](tmp_img);  
}




void image_manipulation::invert( unsigned char* img, unsigned int width,
				 unsigned int height ){
  for( unsigned int ii=0; ii< width*height; ii++){
    img[ii] = 255 - img[ii];
  }
}






#ifdef HAVE_PNG

void image_manipulation::read_png( std::string fn, unsigned char **img, int *width, int *height ){

  // start of by opening the file
  FILE *fp = fopen( fn.c_str(), "rb" );
  if(!fp)
    return;

  unsigned char header[8];
  fread(header, 1, 8, fp);
  if( png_sig_cmp( header, 0, 8 ) ){
    std::cerr << "not a png file!" << std::endl;
  }

  png_structp png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING,
						NULL,
						NULL,
						NULL );

  if( !png_ptr ){
    png_destroy_read_struct( &png_ptr, NULL, NULL );    
    return;
  }

  png_infop info_ptr = png_create_info_struct( png_ptr );
  if( !png_ptr ){
    png_destroy_read_struct( &png_ptr, &info_ptr, NULL );    
    return;
  }
  
  png_infop end_info = png_create_info_struct( png_ptr );
  if( !end_info ){
    png_destroy_read_struct( &png_ptr, &info_ptr, &end_info );
    return;
  }

  png_init_io( png_ptr, fp );
  png_set_sig_bytes( png_ptr, 8 );

  png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

  *width = png_get_image_width( png_ptr,info_ptr);
  *height = png_get_image_height( png_ptr,info_ptr);  
  
  int color_type = png_get_color_type( png_ptr, info_ptr );
  int bit_depth = png_get_bit_depth( png_ptr, info_ptr );
  
  png_bytep *row_pointers;
  row_pointers = png_get_rows(png_ptr, info_ptr );


  // allocate the memory
  *img = new unsigned char[*width*(*height)]();
  
  for(unsigned int ii=0; ii<*height; ii++){
    for( unsigned int jj=0; jj<*width; jj++){
      (*img)[jj+ii*(*width)] = row_pointers[ii][jj];
    }
  }
  
  // free memory
  png_destroy_read_struct( &png_ptr, &info_ptr, &end_info );
  
}


/**
 * writes the current projection to a png image
 */
void image_manipulation::write_png( std::string out_fn,
				    unsigned char *img,
				    unsigned int width,
				    unsigned int height ){

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
}



#endif



