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
 * # 5. note, you needn't reallocate the buffer to reuse it, as long as slice_height and slice_width stay fixed together with the image_width (and height)
 * spirepy.generate_spire_image(out, slice_height = 2.0, slice_width = 2.0, image_height = H, image_width = W)
 *
*/

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include "generate_spire_image.h"   //generate_spire_image function definition
#include "img_manip.hpp"            //image_manipulation class

namespace py = pybind11;

/**
 * \brief Computes the vertical image height (in pixels) from physical proportions in unit cells.
 *
 * This function calculates the number of vertical pixels such that the pixel aspect ratio
 * matches the physical aspect ratio defined by the slice dimensions in unit cells [UC].
 * The computed value follows the same truncation rule used internally by SPIRE.
 *
 * \param[in] slice_height  Physical height of the slice in unit cells (UC).
 * \param[in] slice_width   Physical width of the slice in unit cells (UC).
 * \param[in] image_width   Horizontal pixel width of the output image.
 *
 * \return The computed vertical image height in pixels.
 *
 * \throws std::runtime_error If any input parameter is non-positive or the computed height < 1.
 */
int py_compute_image_height(double slice_height,
                            double slice_width,
                            int image_width)
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

// =====================================================
//  Helper: validate 2D uint8 NumPy buffer
// =====================================================
static uint8_t* extract_uint8_buffer_2d(py::array_t<uint8_t>& buffer,
                                        int image_height,
                                        int image_width)
{
    auto info = buffer.request();

    if (info.ndim != 2)
        throw std::runtime_error("buffer must be a 2D NumPy array");

    if (info.shape[0] != image_height || info.shape[1] != image_width)
        throw std::runtime_error("buffer must have shape (image_height, image_width)");

    if (info.itemsize != 1)
        throw std::runtime_error("buffer must be dtype=uint8");

    return static_cast<uint8_t*>(info.ptr);
}

/**
 * \brief Generates a SPIRE-based synthetic image using the provided parameters.
 *
 * This function recomputes an image according to the specified crystallographic,
 * geometric, and rendering parameters. The resulting pixel data are written into
 * the provided NumPy buffer. Optionally, the generated image can be saved to disk
 * if a non-empty filename is provided.
 *
 * \param[in,out] buffer            NumPy array (uint8) into which the image is written.
 * \param[in]     structure_type    Structure selector: 0 = gyroid, 1 = diamond,
 *                                  2 = primitive, etc.
 * \param[in]     uc_scale_ab       Unit-cell scaling factor in the a/b directions.
 * \param[in]     uc_scale_c        Unit-cell scaling factor along the c axis.
 * \param[in]     channel_vol_prop  Target channel volume fraction in the range [0, 1].
 *
 * \param[in]     slice_height      Physical slice height in unit cells (UC).
 * \param[in]     slice_width       Physical slice width in unit cells  (UC).
 *
 * \param[in]     slice_thickness   Projection depth / physical thickness of the slice (UC scale).
 * \param[in]     slice_position    Relative slice position in unit cells (0 = centered).
 * \param[in]     h                 Miller index h.
 * \param[in]     k                 Miller index k.
 * \param[in]     l                 Miller index l.
 *
 * \param[in]     membrane          Membrane parameters (distance and width). For each membrane, those two values are required.
 *
 * \param[in]     image_height      Number of vertical pixels in the output image.
 * \param[in]     image_width       Number of horizontal pixels in the output image.
 *
 * \param[in]     image_depth       Sampling density along the z axis (e.g., 76).
 *
 * \param[in]     filename          Output filename. If empty, or not provided, the image is not saved.
 *
 * \throws std::runtime_error       If the buffer is incorrectly sized or parameters are invalid.
 */
void py_generate_spire_image(py::array_t<uint8_t> buffer,
                             int structure_type,
                             double uc_scale_ab,
                             double uc_scale_c,
                             double channel_vol_prop,
                             double slice_height,
                             double slice_width,
                             double slice_thickness,
                             double slice_position,
                             int h,
                             int k,
                             int l,
                             std::vector<double> membrane,
                             int image_height,
                             int image_width,
                             int image_depth,
                             const std::string& filename)
{
    uint8_t* ptr = extract_uint8_buffer_2d(buffer, image_height, image_width);

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
                         membrane,
                         image_height,
                         image_width,
                         image_depth,
                         filename);
}



/**
 * \brief Apply a Gaussian blur to a 2D uint8 image buffer.
 *
 * This function smooths a grayscale image by convolving it with a Gaussian
 * kernel of the specified size. The input buffer must be a 2D NumPy array
 * of shape ``(image_height, image_width)`` and dtype ``uint8``. The blur
 * operation is applied in-place.
 *
 * \param[in,out] buffer       2D NumPy array (uint8) into which the blurred image is written.
 * \param[in]     image_height Expected height of the image in pixels.
 * \param[in]     image_width  Expected width of the image in pixels.
 * \param[in]     kernel_size  Size of the Gaussian kernel (must be odd and > 0).
 *
 * \throws std::runtime_error If the buffer is not 2D, if its shape does not
 *                            match ``(image_height, image_width)``, or if the
 *                            dtype is not uint8.
 */
void py_apply_gaussian_blur(py::array_t<uint8_t> buffer,
                      int image_height,
                      int image_width,
                      unsigned int kernel_size)
{
    uint8_t* ptr = extract_uint8_buffer_2d(buffer, image_height, image_width);
    image_manipulation::gaussian_blur(ptr,
                                      static_cast<unsigned int>(image_width),
                                      static_cast<unsigned int>(image_height),
                                      kernel_size);
}



/**
 * \brief Add Gaussian noise to a 2D uint8 image buffer.
 *
 * This function adds zero-mean Gaussian noise with the specified magnitude
 * (standard deviation) to a grayscale image. The input must be a 2D NumPy array
 * of shape ``(image_height, image_width)`` and dtype ``uint8``. The noisy image
 * is written back into the same buffer in-place.
 *
 * \param[in,out] buffer        2D NumPy array (uint8) that will receive the noisy image.
 * \param[in]     image_height  Expected height of the image in pixels.
 * \param[in]     image_width   Expected width of the image in pixels.
 * \param[in]     magnitude     Standard deviation of the Gaussian noise.
 *
 * \throws std::runtime_error If the buffer is not 2D, has incompatible shape,
 *                            or is not dtype uint8.
 */
void py_add_gaussian_noise(py::array_t<uint8_t> buffer,
                       int image_height,
                       int image_width,
                       double magnitude)
{
    uint8_t* ptr = extract_uint8_buffer_2d(buffer, image_height, image_width);
    image_manipulation::gaussian_noise(ptr,
                                       static_cast<unsigned int>(image_width),
                                       static_cast<unsigned int>(image_height),
                                       magnitude);
}



/**
 * \brief Add grain-like perturbations to a 2D uint8 image buffer.
 *
 * This function modifies a grayscale image by inserting randomly distributed
 * “grains” mirroring behaviour of electron microscopy artefacts.
 * Grain sizes and counts are drawn from normal distributions defined
 * by the provided center and width parameters. The resulting perturbation is
 * scaled by ``magnitude`` and blended with the original image according to
 * ``original_image_proportion``. The operation is applied in-place to a
 * 2D NumPy buffer of dtype ``uint8`` and shape ``(image_height, image_width)``.
 *
 * \param[in,out] buffer                   2D NumPy array (uint8) receiving the perturbed image.
 * \param[in]     image_height             Expected height of the image in pixels.
 * \param[in]     image_width              Expected width of the image in pixels.
 * \param[in]     grain_size_center        Mean grain size in pixels.
 * \param[in]     grain_size_stdev         Width (stdev parameter) of the grain-size distribution.
 * \param[in]     grain_number_center      Mean number of grains.
 * \param[in]     grain_number_stdev       Width (stdev parameter) of the grain-count distribution.
 * \param[in]     magnitude                Scaling factor controlling grain intensity.
 * \param[in]     blur_kernel_size         Kernel size for blur of the grains image.
 * \param[in]     original_image_proportion Blend factor controlling preservation of the original image.
 *
 * \throws std::runtime_error If the buffer is not 2D, is not dtype uint8,
 *                            or does not match ``(image_height, image_width)``.
 */
void py_add_grains(py::array_t<uint8_t> buffer,
                   int image_height,
                   int image_width,
                   int grain_size_center,
                   int grain_size_stdev,
                   int grain_number_center,
                   int grain_number_stdev,
                   double magnitude,
                   double blur_kernel_size,
                   double original_image_proportion)
{
    uint8_t* ptr = extract_uint8_buffer_2d(buffer, image_height, image_width);

    image_manipulation::add_grains(ptr,
                                   static_cast<unsigned int>(image_width),
                                   static_cast<unsigned int>(image_height),
                                   grain_size_center,
                                   grain_size_stdev,
                                   grain_number_center,
                                   grain_number_stdev,
                                   magnitude, blur_kernel_size,
                                   original_image_proportion);
}



PYBIND11_MODULE(spirepy, m) {
    m.def(
        "generate_spire_image",
        &py_generate_spire_image,
        py::arg("buffer"),
        py::arg("structure_type") = 1,
        py::arg("uc_scale_ab") = 1.0,
        py::arg("uc_scale_c") = 1.0,
        py::arg("channel_vol_prop") = 0.5,

        py::arg("slice_height") = 1.0,
        py::arg("slice_width")  = 1.0,

        py::arg("slice_thickness") = 1.0,
        py::arg("slice_position")  = 0.0,
        py::arg("h") = 0,
        py::arg("k") = 1,
        py::arg("l") = 1,
        py::arg("membrane") = std::vector<double>{0.0, 5.0},
        py::arg("image_height") = 256,
        py::arg("image_width")  = 256,

        py::arg("image_depth")  = 76,
        py::arg("filename")     = "",
        R"doc(
Generate a synthetic SPIRE image and write it into a preallocated NumPy buffer.

This function recomputes a 2D projection of a 3D periodic structure (gyroid,
diamond, primitive, etc.) using SPIRE's simulation pipeline. The output is
written directly into the provided NumPy array of type ``uint8`` with shape
``(image_height, image_width)``. Optionally, the generated image can also be
saved to disk.

Parameters
----------
buffer : numpy.ndarray (uint8)
    Preallocated array of shape ``(image_height, image_width)`` that will be
    filled in-place.

structure_type : int, default=1
    Identifier of the structure:
    ``0 = gyroid``, ``1 = diamond``, ``2 = primitive``, etc.

uc_scale_ab : float, default=1.0
    Unit-cell scaling factor in the **a** and **b** directions.

uc_scale_c : float, default=1.0
    Unit-cell scaling factor along the **c** axis.

channel_vol_prop : float, default=0.5
    Target volume fraction of the channel (range: 0–1).

slice_height : float, default=1.0
    Physical height of the slice in unit cells (UC).

slice_width : float, default=1.0
    Physical width of the slice in unit cells (UC).

slice_thickness : float, default=1.0
    Projection depth / physical slice thickness (in UC).

slice_position : float, default=0.0
    Relative position of the slice along the **c** axis (0 = centered).

h, k, l : int, default=0, 1, 1
    Miller indices defining the orientation of the structure.

membrane : list of float, default=[0.0, 5.0]
    Membrane parameters (distance and width). For each membrane, those two values are required.

image_height : int, default=256
    Number of vertical pixels.

image_width : int, default=256
    Number of horizontal pixels.

image_depth : int, default=76
    Sampling density along the projection axis.
    Increasing this improves accuracy at the cost of computation time.

filename : str, default=""
    Optional output path.
    If empty, no file is written.

Returns
-------
None
    The result is written directly into ``buffer``.

Raises
------
RuntimeError
    If the buffer is incorrectly sized or if invalid parameters are supplied.
)doc"
);

    m.def(
    "compute_height",
    &py_compute_image_height,
    py::arg("slice_height"),
    py::arg("slice_width"),
    py::arg("image_width"),
    R"doc(
Compute the required vertical image height (in pixels) from slice proportions.

This function computes the number of vertical pixels needed so that a digital
image maintains the same aspect ratio as the physical slice dimensions
expressed in unit cells (UC). The calculation matches SPIRE's internal
truncation rules.

Parameters
----------
slice_height : float
    Physical slice height in unit cells (UC).

slice_width : float
    Physical slice width in unit cells (UC).

image_width : int
    Desired number of horizontal pixels.

Returns
-------
int
    The computed vertical pixel count.

Raises
------
RuntimeError
    If any parameter is non-positive or if the computed height would be < 1.
)doc"
);

    m.def(
    "apply_gaussian_blur",
    &py_apply_gaussian_blur,
    py::arg("img"),
    py::arg("width"),
    py::arg("height"),
    py::arg("kernel_size"),
    R"doc(
Apply a Gaussian blur to an image.

This function smooths a 2D uint8 image by convolving it with a Gaussian kernel
of the specified size. The blur is applied in-place to the provided NumPy array.

Parameters
----------
img : numpy.ndarray (uint8)
    2D grayscale image buffer of shape (height, width).
width : int
    Image width in pixels.
height : int
    Image height in pixels.
kernel_size : int
    Gaussian kernel size (must be odd).

Returns
-------
None
)doc"
);



m.def(
    "add_gaussian_noise",
    &py_add_gaussian_noise,
    py::arg("img"),
    py::arg("width"),
    py::arg("height"),
    py::arg("magnitude"),
    R"doc(
Add Gaussian noise to an image.

Applies zero-mean Gaussian noise to a 2D uint8 image in-place.

Parameters
----------
img : numpy.ndarray (uint8)
    2D grayscale image buffer of shape (height, width).
width : int
    Image width in pixels.
height : int
    Image height in pixels.
magnitude : float
    Standard deviation of the Gaussian noise.

Returns
-------
None
)doc"
);



m.def(
    "add_grains",
    &py_add_grains,
    py::arg("img"),
    py::arg("width"),
    py::arg("height"),
    py::arg("grain_size_center"),
    py::arg("grain_size_stdev"),
    py::arg("grain_number_center"),
    py::arg("grain_number_stdev"),
    py::arg("magnitude"),
    py::arg("blur_kernel_size"),
    py::arg("original_image_proportion"),
    R"doc(
Add random grain-like structures to an image.

This function perturbs a grayscale image by inserting randomly sized and
randomly placed “grains” (additionally blurred) useful for synthetic augmentation or degradation
effects mirroring behaviour of electron microscopy artefacts. All modifications occur in-place.

Parameters
----------
img : numpy.ndarray (uint8)
    2D grayscale image buffer of shape (height, width).
width : int
    Image width in pixels.
height : int
    Image height in pixels.
grain_size_center : int
    Mean grain size in pixels.
grain_size_width : int
    Spread (variance parameter) for grain size.
grain_number_stdev : int
    Mean number of grains inserted.
grain_number_stdev : int
    Spread (variance parameter) for grain count.
magnitude : float
    Strength of the grain effect.
blur_kernel_size : float
    Kernel size for blur of the grains image.
original_image_proportion : float
    Blend factor controlling how much of the original image is preserved.

Returns
-------
None
)doc"
);


}
