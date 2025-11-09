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

    GUI gui( &app, &default_locale, gs, time(NULL) );
    gui.show();
    return app.exec();

}
