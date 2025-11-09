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

#include <QApplication>
#include <ctime>
#include "qt_gui.hpp"
#include "global_settings.hpp"

#include "generate_spire_image.h"


#include <chrono>  //timing
#include <cstdio>

int main( int argc, char *argv[]){
    QApplication app( argc, argv );

    QLocale default_locale = QLocale();

    app.setWindowIcon(QIcon(":/resources/icon/icon.ico"));

    global_settings gs ( "global_settings.conf" );


    printf("Warm-up...\n");
    unsigned char* buffer = new unsigned char[256 * 256];
    unsigned long long checksum = 0; // antyoptymalizacja

    for (int t = 0; t < 10; ++t) {
        generate_spire_image(buffer, 1, 1.0, 1.0, 0.5, 1.0, 1.0, 1.0, 0.0, 0, 1, 1, 256, 256, 76, "");

        // prosty trik antyoptymalizacyjny
        for (int i = 0; i < 256 * 256; ++i)
            checksum += buffer[i];
    }
    printf("Start test...\n");

    auto start = std::chrono::high_resolution_clock::now();

    for (int t = 0; t < 100; ++t) {
        generate_spire_image(
        buffer,
        rand() % 3,                                // structure_type ∈ {0,1,2}
        0.8 + 0.4 * ((double)rand() / RAND_MAX),   // uc_scale_ab
        0.8 + 0.4 * ((double)rand() / RAND_MAX),   // uc_scale_c
        0.3 + 0.4 * ((double)rand() / RAND_MAX),   // channel_vol_prop
        1.0,   // slice_height
        1.0,   // slice_width
        0.8 + 0.4 * ((double)rand() / RAND_MAX),   // slice_thickness
        -1.0 + 2.0 * ((double)rand() / RAND_MAX),  // slice_position ∈ [-1,1]
        1 + rand() % 10,                           // h
        1 + rand() % 10,                           // k
        1 + rand() % 10,                           // l
        256, 256, 76, ""
    );

        // prosty trik antyoptymalizacyjny
        for (int i = 17; i < 20; ++i)
            checksum += buffer[i];
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    delete[] buffer;

    printf("Finished test. Elapsed time: %.3f s (checksum=%llu)\n", elapsed.count(), checksum);

    fflush(stdout);
    //return 0;

    GUI gui( &app, &default_locale, gs, time(NULL) );
    gui.show();
    return app.exec();

}
