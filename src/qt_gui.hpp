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

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <sstream>

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QImage>
#include <QLabel>
#include <QPixmap>
#include <QApplication>
#include <QTableWidget>
#include <QHeaderView>
#include <QStatusBar>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QSpacerItem>
#include <QCheckBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QScrollArea>
#include <QTextStream>
#include <QFile>
#include <QProgressBar>
#include <QRegularExpression>
#include <QtSvgWidgets/QSvgWidget>


#include <QThread>
#include <QMutex>

#include "qt_sp.hpp"
#include "sp_gui_tooltips.h"
#include "slice_orientation_visualisation.hpp"
#include "vec_mat_math.hpp"

#include "batch_lib.hpp"
#include "global_settings.hpp"

#include "img_manip.hpp"

/**
 * a short qt wrapper around the batch_lib to it has signals and slots
 */
class qt_bc : public QObject, public batch_creation {

  Q_OBJECT;

public:
  
  qt_bc ( surface_projection &sp_, int seed ) : batch_creation( sp_, seed ) {
  }

  void set_callback( sp_callback *callback ){
    cb = callback;
  }

public slots:
  void start_loop(){
    this->do_loop( *cb );    
    emit finished_batch_loop();
  }
  
signals:
  void finished_batch_loop();
  
private:
  sp_callback *cb;

};

/** 
 * The callback functor for batch creation
 */
class gui_callback : public QObject, public sp_callback {

  Q_OBJECT;
  
public:
  gui_callback( sp_qt &sp,
		batch_creation &bc_,
		std::string prefix ) :
    sp_callback( sp ),
    fn_prefix( prefix ),
    bc( bc_ ),
    internal_counter (0),
    keep_going ( true )
  {

  }

  ~gui_callback(){
  }

  void set_prefix( std::string new_pre ){
    fn_prefix = new_pre;   
  }

  void init(){
    summary.open( fn_prefix + "_summary.txt" );
    internal_counter=0;
    external_counter=0;
  }

  void finalize(){
    summary.close();
  }

  /**
   * this function computes the projection and adds the relevant
   * parameters to the png
   */
  bool operator()( std::vector< std::vector<double>::iterator > pars ){

    // compute the projection and get the image
    std::string fn = fn_prefix + "_" +  std::to_string( internal_counter ) + ".png";       
    sp.validate_uc_scaling();
    sp.update_geometry();
    sp.compute_projection();


    if( render_parameters ){
    
      unsigned char* img = sp.get_image( invert, scaling );
      sp_qt& tmp = static_cast<sp_qt&>( sp );
      img = tmp.save_png_legend( img, sp.get_width(), sp.get_height(),
				 invert ? std::string("#ffffff") : std::string("#000000"),
				 invert ? std::string("#000000") : std::string("#ffffff"),
				 fn, render_all_parameters
				 ? std::vector<std::string> (0, "") : bc.ops.parameters );

      
      free( img );

    } else {
      sp.save_to_png( fn, invert, scaling );
    }
    
    summary << fn << "    ";

    for( auto it : surface_projection::parameter_names ){
      summary << it << "=" << sp.get_parameter( it ) << " ";
    }

    summary << std::endl;
    internal_counter++;
    emit updated_batch_progress( external_counter );

    return keep_going;
  }

  // keeps track of how many projections has been computed, used for
  // filenames
  unsigned int internal_counter;
  
  batch_creation &bc;
  std::string fn_prefix;
  std::ofstream summary;
  
  bool keep_going;
  bool render_parameters;
  bool render_all_parameters;			
		 
		 
public slots:
  void update_status ( bool go_on ){
    keep_going = go_on;
  }

  void set_render_all_parameters( bool val ){
    render_all_parameters = val;
  }

  void set_render_parameters( bool val ){
    render_parameters = val;
  }
  
signals:
  void updated_batch_progress( int p );
  
};



/**
 * short wrapper for an object including a label combined with an
 * object
 */
template <class QT_O>
class QT_labeled_obj {

public:
  /**
   * Constructor orientation is a char array of length 2. First
   * character: v/h horizontal or vertical laytou, second character
   * l/o label or object first
   *
   * \param[in] orientation Orientation and Layout
   * \param[in] label_str Text of the label
   * param[in] parent The QT parent
   */
  QT_labeled_obj( std::string orientation, std::string label_str, QWidget *parent = NULL ){
    obj = new QT_O ( parent );
    lbl = new QLabel( QString(label_str.c_str()), parent );

    if( orientation[0] == 'v' ){
      lyt = new QVBoxLayout();
    } else {
      lyt = new QHBoxLayout();
    }

    if( orientation[1] == 'l' ){      
      lyt->addWidget( lbl );
      lyt->addWidget( obj );
    } else {
      lyt->addWidget( obj );
      lyt->addWidget( lbl );
    }
  }
  
  ~QT_labeled_obj(){
    delete( obj );
    delete( lbl );
    delete( lyt );
  }

  QBoxLayout* layout(){
    return lyt;
  }
  QT_O* object(){
    return obj;
  }
  QLabel* label(){
    return lbl;
  }  
  
private:
  QT_O *obj;
  QLabel *lbl;
  QBoxLayout *lyt;
};


class GUI : public QWidget {

  Q_OBJECT

public:
  explicit GUI( QApplication *app, QLocale *def_locale_, global_settings &gs, int seed, QWidget *parent = 0 );


  ~GUI();
    
protected:
  
  void paintEvent( QPaintEvent * event );
  
private:

  int sp_state, sp_stat_state;

  const unsigned int space_between_items = 20;
  
  // locale
  QLocale *def_locale;

  // global settings
  global_settings &gs;
  
  //tooltips
  tooltips ttips;

  // functions
  void set_up_ui();
  void set_up_signals_and_slots();
  void set_up_tooltips();

  void update_batch_parameters_gui();
  
  // ui elements
  QLabel *draw_area;
  QSvgWidget *orientation_visualisation;

  QTabWidget *controls;
  QWidget *parameters_widget;
  QWidget *measurement_widget;

  QWidget *batch_widget;
  QScrollArea *batch_scroll_area;
  QWidget *batch_scroll_subwidget;

  QWidget *pp_widget;

  
  QWidget *save_widget;
  QWidget *about_widget;
  QWidget *license_widget;  

  // manual tab
  QPlainTextEdit *manual;

  // measurement tab
  QT_labeled_obj<QTableWidget> *measurements_slice;
  QT_labeled_obj<QTableWidget> *measurements_uc;
  std::vector<QLabel*> measurements_pixinfo_slice;
  std::vector<QLabel*> measurements_pixinfo_uc;  

  // save tab
  QPushButton *choose_path_prefix;
  QPushButton *save_grid_control;
  QPushButton *save_surface_points_control;

  QT_labeled_obj<QLineEdit> *path_prefix_control;
  
  // structure control
  QT_labeled_obj<QDoubleSpinBox> *uc_size_control_a;
  QT_labeled_obj<QDoubleSpinBox> *uc_size_control_c;
  QT_labeled_obj<QDoubleSpinBox> *channel_prop_control;
  QT_labeled_obj<QComboBox> *level_par_type;
  QT_labeled_obj<QComboBox> *surface_type_control;
  

  // resolution control
  QT_labeled_obj<QSpinBox> *z_points_control;
  QT_labeled_obj<QSpinBox> *x_points_control;
  QT_labeled_obj<QComboBox> *image_scaling_control;  
  QT_labeled_obj<QCheckBox> *invert_control;  
  QCheckBox *autoupdate_control;
  QCheckBox *render_pars_to_img_control;


  // slice parameters control
  QT_labeled_obj<QDoubleSpinBox> *slice_thickness_control;
  QT_labeled_obj<QDoubleSpinBox> *slice_width_control;
  QT_labeled_obj<QDoubleSpinBox> *slice_height_control;  
  QT_labeled_obj<QDoubleSpinBox> *slice_position_control;

  QPushButton *button_set_to_uc_dim;
  QCheckBox *draw_uc_control;
  QVBoxLayout *auto_uc_layout;
  
  QT_labeled_obj<QSpinBox> *miller_h_control;
  QT_labeled_obj<QSpinBox> *miller_k_control;
  QT_labeled_obj<QSpinBox> *miller_l_control;  
  
  //membranes
  QLabel *membranes_label;
  QTableWidget *membranes_control;
  QPushButton *add_membrane_control;
  QPushButton *rm_membrane_control;

  QLabel *fill_channels_label;
  QScrollArea *fill_channels_control_container;
  QWidget *fill_channels_control_content;
  QVBoxLayout *fill_channels_container_layout;
  std::vector<QCheckBox*> fill_channels;
  std::vector<bool> channel_states;
  
  //basic buttons
  QPushButton *button_render;
  QPushButton *button_save;

  QPushButton *button_write_pars;
  QPushButton *button_read_pars;

  QComboBox *measurement_object;
  QPushButton *button_measure_vol;
  QPushButton *button_measure_area;
  QPushButton *button_measure_percthres;


  // batch creation
  QLabel *batch_instructions;
  std::vector<QT_labeled_obj<QLineEdit>*> batch_values;
  QT_labeled_obj<QLineEdit> *batch_output_name;
  QT_labeled_obj<QPushButton> *batch_choose_folder;
  QProgressBar *batch_progress;
  QPushButton *batch_compute_start;
  QPushButton *batch_compute_stop;
  QCheckBox *batch_render_parameters;
  QCheckBox *batch_render_all_parameters;




  // post processing
  QT_labeled_obj<QSpinBox> *gaussian_noise_magnitude;
  QT_labeled_obj<QSpinBox> *gaussian_blur_kernelsize;
  QT_labeled_obj<QSpinBox> *grains_gsize_c;
  QT_labeled_obj<QSpinBox> *grains_gsize_s;
  QT_labeled_obj<QSpinBox> *grains_gnumber_c;
  QT_labeled_obj<QSpinBox> *grains_gnumber_s;
  QT_labeled_obj<QSpinBox> *grains_magnitude;
  QT_labeled_obj<QSpinBox> *grains_kernelsize;
  QT_labeled_obj<QDoubleSpinBox> *grains_mix;
  QCheckBox *gaussian_noise_activate, *gaussian_blur_activate, *grains_activate;
  
  //status bar
  QStatusBar *status_bar;
  QLabel *status_bar_status_m;
  QLabel *status_bar_status_p;  
  QLabel *status_bar_uco;   // --> uc in orientation
  QLabel *status_bar_or; //--> orientation
  QLabel *status_bar_pixs; //--> pixel info
  QLabel *status_bar_uc; //-->> uc_dim


  //about
  QLabel *about_us;
  QLabel *refs_ack;

  // license
  QTextEdit *licenses;

  
  // layouts

  // the main layout, holds status bar and sub_main_layout
  QHBoxLayout *main_layout;
  
  // slice properties
  QHBoxLayout *slice_settings;
  QVBoxLayout *slice_orientation_layout;
  QVBoxLayout *slice_dimension_layout;
  
  // hold draw area and status bar
  QVBoxLayout *draw_and_status_layout;
  
  QHBoxLayout *buttons_layout;
  QVBoxLayout *buttons_savewrite;
  QVBoxLayout *buttons_projection;
  
  QHBoxLayout *structure_settings;
  QHBoxLayout *surface_level_settings;
  QHBoxLayout *resolution_settings;

  QHBoxLayout *membrane_settings;
  QHBoxLayout *membrane_buttons_layout;
  
  QVBoxLayout *parameters_widget_layout;
  QVBoxLayout *measurement_widget_layout;
  QHBoxLayout *measurement_widget_buttons_layout;
  QHBoxLayout *measurement_pixinfo_slice_layout;
  QHBoxLayout *measurement_pixinfo_uc_layout;    

  QVBoxLayout *save_widget_layout;

  
  QVBoxLayout *batch_widget_layout;
  QHBoxLayout *batch_widget_buttons_layout;
  QVBoxLayout *batch_widget_parameters_layout;
  QHBoxLayout *batch_widget_output_layout;

  QVBoxLayout *pp_widget_layout;
  QHBoxLayout *gaussian_noise_layout;
  QHBoxLayout *gaussian_blur_layout;
  QHBoxLayout *grains_layout;

  QHBoxLayout *gaussian_noise_label_layout,
    *gaussian_blur_label_layout,
    *grains_label_layout;
  
  QFrame *h_line_4, *h_line_5, *h_line_6;
  
  QVBoxLayout *about_widget_layout;
  QHBoxLayout *about_qt_layout;

  QVBoxLayout *license_widget_layout;
  
  std::vector< QSpacerItem* > v_spacer;
  std::vector< QSpacerItem* > h_spacer;  
  QFrame *h_line_1, *h_line_2, *h_line_3;

  
  //
  // background members
  //  
  QPixmap *img_pix;


  QPainter *uc_artist;
  
  // the thread holding the projection class
  QThread *thread;

  // another copy of the projection class, but used for measurements
  QThread *t_stats;

  // a thread to handle the batch computation
  QThread *bc_thread;
  QThread *cb_thread;
  
  // the surface projection class
  sp_qt *sp;

  // surface projection class to compute measurements, not sure if
  // that's needed after all
  sp_qt *sp_stats;

  // the batch_creation controller class
  qt_bc *bc;

  // the callback functor to call the projection thingy
  gui_callback *cb;
  
  // a pointer to the application executing this form
  QApplication *app;
  
signals:


  void call_change_uc_size( double ab, double c );
  void call_change_hkl( int h, int k, int l );

  void call_compute_projection();
  void call_update_stats( QString what );


  void call_save_grid( QString fn );
  void call_save_surface( int id, QString fn );
  void call_save_network( int id, QString fn );

  void call_set_measurement_status( int state );

  void call_change_channel_color( int id, int val );

  void call_show_dialog_box();

  void request_parameter_change( double value );

  void emit_start_batch();
  void emit_stop_batch( bool go_on );			 

  void call_read_pars_from_file( QString fn );				    
				    
public slots:

  void change_surface_par_type( int index );
  
  void check_channel_color();
  
  void update_unitcell_size();
  
  void update_gui_from_sp();
  
  
  void update_view();
  void draw_unitcell( QPaintDevice *image );

  
  void quit_app();
  void save_image_to_file();

  void write_membranes(int row, int col);
  void read_membranes();
  void add_membrane( double first = 0, double second = 0);
  void rm_membrane();

  // argument needed to match signal of QSpinBox
  void change_orientation( int val );

  void output_message( QString msg, int type = 1);

  void update_stats();
  void update_measurements_values( QString what );
  void update_measurements_structure();

  void change_autoupdate( int state );
  void request_compute_projection();

  void measure_vol();
  void measure_area();
  void measure_network();
  void measure_percolation();  

  void deactivate_measurement_buttons();
  void activate_measurement_buttons();

  void deactivate_pp_controls();
  void activate_pp_controls();    
  
  void choose_export_prefix();
  std::string get_prefix();
  void save_grid();
  void save_network();
  void save_surface_points();

  void set_state( int what, int state );

  void set_measurements_status( int state );

  void read_parameters();
  void write_parameters();
  
  void update_fill_channels();
  
  void set_parameter();


  bool check_batch_string_consistency( QString inp );
  std::vector<double> check_struct_type_string_consistency( QString inp );  
  void set_batch_output();
  void start_batch_computing();
  void stop_batch_computing();
  void finalize_batch_loop();
  
  void do_something();
  
};



