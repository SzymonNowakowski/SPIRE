//
// Created by Szymon Nowakowski on 7.11.2025.
//

#pragma once
#include <string>
#include <cstdint>

/**
 * Generate an image projection into a pre-allocated buffer.
 *
 * @param buffer          Pointer to pre-allocated image buffer (size = image_height × image_width)
 * @param structure_type  0 = gyroid, 1 = diamond, 2 = primitive, etc.
 * @param uc_scale_ab     Unit cell scaling factor in a/b directions
 * @param uc_scale_c      Unit cell scaling factor along c axis
 * @param channel_vol_prop Target volume fraction of the channel (0–1)
 * @param slice_height    Physical height of the slice
 * @param slice_width     Physical width of the slice
 * @param slice_thickness Physical thickness of the slice (projection depth)
 * @param slice_position  Relative position of the slice (0 = centered)
 * @param h, k, l         Miller indices defining the orientation
 * @param image_height    Number of vertical pixels
 * @param image_width     Number of horizontal pixels
 * @param image_depth     Sampling density along z-axis (higher = better quality)
 * @param filename        Optional output filename; if empty, no file is written
 *
 * @return Pointer to the image data (identical to `buffer`)
 */
void generate_spire_image(
    unsigned char* buffer,
    int structure_type = 1,          // 0 = gyroid, 1 = diamond, 2 = primitive
    double uc_scale_ab = 1.0,        // scaling in a/b directions
    double uc_scale_c = 1.0,         // scaling in c direction
    double channel_vol_prop = 0.5,   // target volume fraction of the channel
    double slice_height = 1.0,       // physical height of the slice
    double slice_width = 1.0,        // physical width of the slice
    double slice_thickness = 1.0,    // projection depth
    double slice_position = 0.0,     // 0 = centered
    int h = 0,                       // Miller index h
    int k = 1,                       // Miller index k
    int l = 1,                       // Miller index l
    int image_height = 256,          // number of vertical pixels
    int image_width = 256,           // number of horizontal pixels
    int image_depth = 76,            // sampling density along z-axis
    const std::string& filename = "" // if empty, skip saving
);
