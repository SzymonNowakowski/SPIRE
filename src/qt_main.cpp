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

#include "surface_projection.hpp"  //TODO: SZN changes


int main( int argc, char *argv[]){
    QApplication app( argc, argv );

    QLocale default_locale = QLocale();

    app.setWindowIcon(QIcon(":/resources/icon/icon.ico"));

    global_settings gs ( "global_settings.conf" );


    printf("Start test...\n");

    surface_projection::generate_image_from_parameter_set(1, 1.0, 1.0, 0.5, 1.0, 1.0, 1.0, 0.0, 0, 1, 1, 256, 256, 76, "projection1.data");
    surface_projection::generate_image_from_parameter_set(1, 1.0, 1.0, 0.5, 1.0, 1.0, 1.0, 0.0, 1, 1, 1, 256, 256, 76, "projection2.data");
    printf("Finished test.\n");

    fflush(stdout);
    //return 0;

    GUI gui( &app, &default_locale, gs, time(NULL) );
    gui.show();
    return app.exec();

}
