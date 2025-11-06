/**
 *
 * Created by Szymon Nowakowski on 6.11.2025.
 *
 * This file provides a binding point for Python
 * It creates a module spirepy which exposes the function generate_spire_image
 *
 *
 * Example usage within Python:
 *
 * import spirepy
 *
 * out = np.empty((H, W), dtype=np.uint8)
 * spirepy.generate_spire_image(out, image_height=H, image_width=W)
 * t = torch.from_numpy(out)               # later, zero-copy view (CPU only) to PyTorch tensor
 * spirepy.generate_spire_image(out, image_height=H, image_width=W)   # the same structure gets filled-out
 *
*/

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <fstream>   //file saving in generate_image_from_parameter_set function
namespace py = pybind11;

/**
 * This function returns an image recomputed with provided parameters
 * and optionally saves it
 */

void py_generate_spire_image(
    py::array_t<uint8_t>  buffer,   //buffer to fill
    int structure_type,             // 0 = gyroid, 1 = diamond, 2 = primitive, etc.
    double uc_scale_ab,             // unit cell scaling factor in a/b directions
    double uc_scale_c,              // unit cell scaling factor along c axis
    double channel_vol_prop,        // target volume fraction of the channel (0–1)

    double slice_height,            // physical height of the slice
    double slice_width,             // physical width of the slice

    double slice_thickness,         // physical thickness of the slice (projection depth)
    double slice_position,          // relative position of the slice (0 = centered)
    int h,                          // Miller index h
    int k,                          // Miller index k
    int l,                          // Miller index l

    int image_height,               // number of vertical pixels
    int image_width,                // number of horizontal pixels

    int image_depth,                // sampling density along z (e.g., 76 is reasonable)

    const std::string& filename  // optional output filename; if empty, no file is written
) {
    auto info = buffer.request();
    if (info.ndim != 2 || info.shape[0] != image_height || info.shape[1] != image_width)
        throw std::runtime_error("buffer must be Numpy array sized (image_height, image_width)");
    if (info.itemsize != 1) throw std::runtime_error("buffer must be Numpy array of uint8 dtype");

    auto* ptr = static_cast<uint8_t*>(info.ptr);
    generate_spire_image(ptr, structure_type,
                         uc_scale_ab,
                         uc_scale_c,
                         channel_vol_prop,
                         slice_height,
                         slice_width,
                         slice_thickness,
                         slice_position,
                         h,
                         k,
                         l,
                         image_height,
                         image_width,
                         image_depth,
                         filename);
}

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
  crystal.get_image(buffer, true);

  // Optional save
  if (!filename.empty()) {
    int width  = crystal.get_width();
    int height = crystal.get_height();
    std::ofstream out(filename, std::ios::binary);
    out.write(reinterpret_cast<char*>(buffer), width * height);
    out.close();
  }
}


PYBIND11_MODULE(spirepy, m) {
    m.def(
    "generate_spire_image", &py_generate_spire_image,
    py::arg("buffer"),                     // preallocated NumPy array (H, W), dtype=uint8
    py::arg("structure_type") = 1,         // 0 = gyroid, 1 = diamond, 2 = primitive, etc.
    py::arg("uc_scale_ab") = 1.0,          // unit cell scaling in a/b directions
    py::arg("uc_scale_c") = 1.0,           // unit cell scaling along c axis
    py::arg("channel_vol_prop") = 0.5,     // target volume fraction of the channel (0–1)

    py::arg("slice_height") = 1.0,         // physical height of the slice
    py::arg("slice_width")  = 1.0,         // physical width of the slice

    py::arg("slice_thickness") = 1.0,      // physical thickness (projection depth)
    py::arg("slice_position")  = 0.0,      // relative position of the slice (0 = centered)
    py::arg("h") = 0,                      // Miller index h
    py::arg("k") = 1,                      // Miller index k
    py::arg("l") = 1,                      // Miller index l

    py::arg("image_height") = 256,         // number of vertical pixels (H)
    py::arg("image_width")  = 256,         // number of horizontal pixels (W)

    py::arg("image_depth")  = 76,          // sampling density along z-axis, 76 gives reasonable quality vs time trade-off
    py::arg("filename")     = "",          // optional output path; empty = skip saving
    "Generate images into a pre-initialized NumPy array of the correct size matching the requested image dimensions, i.e. (image_height, image_width), uint8."
);

}