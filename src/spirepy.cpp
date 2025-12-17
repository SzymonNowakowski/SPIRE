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
 * W = 256
 * # 1. figure out H
 * H = spirepy.compute_image_height(slice_height = 2.0, slice_width = 2.0, image_width = W)
 * # 2. prepare a buffer
 * out = np.empty((H, W), dtype=np.uint8)
 * # 3. fill it out
 * # important: if there is a mismatch between computed height and H passed below, there will be an exception
 * spirepy.generate_spire_image(out, slice_height = 2.0, slice_width = 2.0, image_height = H, image_width = W)
 * # 4. get access to a PyTorch tensor (without performing a copy)
 * t = torch.from_numpy(out)
 * # 5. note, you needn't reallocate the buffer to reuse it, as long as slice_height and slice_width stay fixed
 * spirepy.generate_spire_image(out, slice_height = 2.0, slice_width = 2.0, image_height = H, image_width = W)
 *
*/

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include "generate_spire_image.h"   //generate_spire_image function definition
namespace py = pybind11;

/**
 * This function computes height (number of horizontal pixels) of the image based on image proportions in unit cells [UC]
 */
int py_compute_image_height(double slice_height,  // physical height of the slice [UC]
                      double slice_width,   // physical width of the slice  [UC]
                      int image_width)      // number of horizontal pixels
{
    if (slice_height <= 0.0)
        throw std::runtime_error("slice_height must be > 0");
    if (slice_width <= 0.0)
        throw std::runtime_error("slice_width must be > 0");
    if (image_width <= 0)
        throw std::runtime_error("image_width must be > 0");

    double ratio = slice_height / slice_width;

    // Perform *exact same truncation* SPIRE uses internally
    double raw = image_width * ratio;
    int height = static_cast<int>(raw);   // truncation toward zero

    if (height < 1) {
        height = 1;
        throw std::runtime_error("computed image height must be > 0");
    }

    return height;
}



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

    double slice_height,            // physical height of the slice [UC]
    double slice_width,             // physical width of the slice  [UC]

    double slice_thickness,         // physical thickness (projection depth in relation to UC)
    double slice_position,          // relative position of the slice (in relation to UC, 0 = centered)
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

    // ---- Validate numeric parameters ----
    if (slice_height <= 0.0)
        throw std::runtime_error("slice_height must be > 0");

    if (slice_width <= 0.0)
        throw std::runtime_error("slice_width must be > 0");

    if (slice_thickness <= 0.0)
        throw std::runtime_error("slice_thickness must be > 0");

    if (image_height <= 0)
        throw std::runtime_error("image_height must be > 0");

    if (image_width <= 0)
        throw std::runtime_error("image_width must be > 0");

    if (image_depth <= 0)
        throw std::runtime_error("image_depth must be > 0");

    if (uc_scale_ab <= 0.0)
        throw std::runtime_error("uc_scale_ab must be > 0");

    if (uc_scale_c <= 0.0)
        throw std::runtime_error("uc_scale_c must be > 0");

    if (channel_vol_prop <= 0.0 || channel_vol_prop >= 1.0)
        throw std::runtime_error("channel_vol_prop must be in the interval (0, 1)");

    if (structure_type < 0)
        throw std::runtime_error("structure_type must be non-negative (0 = gyroid, 1 = diamond, 2 = primitive, ...)");

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



PYBIND11_MODULE(spirepy, m) {
    m.def(
    "generate_spire_image", &py_generate_spire_image,
    py::arg("buffer"),                     // preallocated NumPy array (H, W), dtype=uint8
    py::arg("structure_type") = 1,         // 0 = gyroid, 1 = diamond, 2 = primitive, etc.
    py::arg("uc_scale_ab") = 1.0,          // unit cell scaling in a/b directions
    py::arg("uc_scale_c") = 1.0,           // unit cell scaling along c axis
    py::arg("channel_vol_prop") = 0.5,     // target volume fraction of the channel (0–1)

    py::arg("slice_height") = 1.0,         // physical height of the slice [UC]
    py::arg("slice_width")  = 1.0,         // physical width of the slice  [UC]

    py::arg("slice_thickness") = 1.0,      // physical thickness (projection depth in relation to UC)
    py::arg("slice_position")  = 0.0,      // relative position of the slice (in relation to UC, 0 = centered)
    py::arg("h") = 0,                      // Miller index h
    py::arg("k") = 1,                      // Miller index k
    py::arg("l") = 1,                      // Miller index l

    py::arg("image_height") = 256,         // number of vertical pixels (H)
    py::arg("image_width")  = 256,         // number of horizontal pixels (W)

    py::arg("image_depth")  = 76,          // sampling density along z-axis, 76 gives reasonable quality vs time trade-off
    py::arg("filename")     = "",          // optional output path; empty = skip saving
    "Generate images into a pre-initialized NumPy array of the correct size matching the requested image dimensions, i.e. (image_height, image_width), uint8."
);

    m.def(
        "compute_height",
        &py_compute_image_height,
        py::arg("slice_height"),     // physical height of the slice [UC]
        py::arg("slice_width"),      // physical width of the slice  [UC]
        py::arg("image_width"),      // number of horizontal pixels (W)
        "Compute the required image height (number of vertical pixels) given slice dimensions in unit cells and the desired image_width."
    );

}
