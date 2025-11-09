/**
 *
 * Created by Szymon Nowakowski on 7.11.2025.
 *
 *
*/

#include "surface_projection.hpp"
#include <fstream>   //file saving in generate_spire_image function

/**
 * This function returns an image recomputed with provided parameters
 * and optionally saves it
 */
void generate_spire_image(
    unsigned char* buffer,        //buffer to fill
    int structure_type,          // 0 = gyroid, 1 = diamond, 2 = primitive, etc.
    double uc_scale_ab,          // unit cell scaling factor in a/b directions
    double uc_scale_c,           // unit cell scaling factor along c axis
    double channel_vol_prop,     // target volume fraction of the channel (0–1)

    double slice_height,         // physical height of the slice
    double slice_width,          // physical width of the slice

    double slice_thickness,      // physical thickness of the slice (projection depth)
    double slice_position,       // relative position of the slice (0 = centered)
    int h,                       // Miller index h
    int k,                       // Miller index k
    int l,                       // Miller index l

    int image_height,            // number of vertical pixels
    int image_width,             // number of horizontal pixels

    int image_depth,             // sampling density along z (e.g., 76 is reasonable)

    const std::string& filename  // optional output filename; if empty, no file is written
) {
  static global_settings gs("global_settings.conf" );
  surface_projection crystal(gs);   //TODO: time if it should be local or initiated once at the beginning, thread-safe way. Or maybe add a new constructor for all those parameters?
  static int count = 0;
  crystal.set_n_points_x(image_width);
  crystal.set_n_points_y(image_height);
  crystal.set_n_points_z(image_depth);   // responsible for image quality

  // Geometry and scaling
  crystal.set_type(structure_type);
  crystal.set_uc_scale_ab(uc_scale_ab);
  crystal.set_uc_scale_c(uc_scale_c);

  // Channel and slice setup
  crystal.set_channel_vol_prop(channel_vol_prop);
  crystal.set_slice_width(slice_width);
  crystal.set_slice_height(slice_height);
  crystal.set_slice_thickness(slice_thickness);
  crystal.set_slice_position(slice_position);

  // Orientation
  crystal.set_h(h);
  crystal.set_k(k);
  crystal.set_l(l);

  // Default membrane configuration (excluded from the argument list)
  crystal.set_membranes(std::vector<double>{0.0, 0.02});

  // Geometry update + projection
  crystal.update_geometry();
  crystal.compute_projection();

  // Retrieve resulting image
  crystal.get_image(buffer, true, "LOG");
  unsigned char *buffer1 = crystal.get_image_deprecated(true, "LOG");
  // --- Compare buffer and buffer1 ---
  size_t total = crystal.get_projection().size();  // lub znana szerokość*wysokość

  for (size_t i = 0; i < total; ++i) {
      if (buffer[i] != buffer1[i])
      {
          count++;
          printf("Found %d in i=%d, %d<>%d !!!!\n", count, i, buffer[i], buffer1[i]);
          printf("structure_type=%d\n", structure_type);
          printf("uc_scale_ab=%.4f, uc_scale_c=%.4f, channel_vol_prop=%.4f\n",
                 uc_scale_ab, uc_scale_c, channel_vol_prop);
          printf("slice_height=%.4f, slice_width=%.4f, slice_thickness=%.4f, slice_position=%.4f\n",
                 slice_height, slice_width, slice_thickness, slice_position);
          printf("h=%d, k=%d, l=%d\n", h, k, l);
          printf("image_height=%d, image_width=%d, image_depth=%d\n",
                 image_height, image_width, image_depth);
      }
  }
  delete[] buffer1;

  // Optional save
  if (!filename.empty()) {
    int width  = crystal.get_width();
    int height = crystal.get_height();
    std::ofstream out(filename, std::ios::binary);
    out.write(reinterpret_cast<char*>(buffer), width * height);
    out.close();
  }
}


