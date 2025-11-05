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


#include "qt_gui.hpp"

/// initialize and creates the GUI
void GUI::set_up_ui(){

  // the widgets for the tabs
  controls = new QTabWidget( this );
  parameters_widget = new QWidget( controls );
  measurement_widget = new QWidget( controls );

  batch_widget = new QWidget( controls );
  batch_scroll_area = new QScrollArea( batch_widget );
  batch_scroll_subwidget = new QWidget( batch_scroll_area );
  batch_scroll_subwidget->setMinimumWidth( 350 );
  batch_scroll_subwidget->setSizePolicy( QSizePolicy::Expanding,
					 QSizePolicy::Expanding);


  pp_widget = new QWidget( controls );  
  save_widget = new QWidget( controls );
  about_widget = new QWidget( controls );
  license_widget = new QWidget( controls );  

  controls->addTab( parameters_widget, "Parameters" );
  controls->addTab( measurement_widget, "Measurements" );
  controls->addTab( batch_widget, "Batch creation" );
  controls->addTab( pp_widget, "Post Processing" );
  controls->addTab( save_widget, "Export" );
  controls->addTab( about_widget, "About" );
  controls->addTab( license_widget, "License" );  
  
  // measurements tab
  measurements_slice = new QT_labeled_obj<QTableWidget>( "vl", "Measurements of entire slice", measurement_widget );
  measurements_slice->object()->insertColumn(0);
  measurements_slice->object()->insertColumn(1);
  measurements_slice->object()->insertColumn(2);
  measurements_slice->object()->insertColumn(3);  
  QStringList measurements_slice_header;
  measurements_slice_header << "Volume" << "Area" << "Perc. threshold" << "Max. pore dia.";
  measurements_slice->object()->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  measurements_slice->object()->setHorizontalHeaderLabels( measurements_slice_header );

  measurements_uc = new QT_labeled_obj<QTableWidget>( "vl", "Measurements of a single, primitive unit cell in (001) orientation", measurement_widget );
  measurements_uc->object()->insertColumn(0);
  measurements_uc->object()->insertColumn(1);
  measurements_uc->object()->insertColumn(2);
  measurements_uc->object()->insertColumn(2);  
  QStringList measurements_uc_header;
  measurements_uc_header << "Volume" << "Area" << "Perc. threshold" << "Max. pore dia.";
  measurements_uc->object()->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  measurements_uc->object()->setHorizontalHeaderLabels( measurements_uc_header );

  for( unsigned int ii=0; ii<4; ii++){
    measurements_pixinfo_slice.push_back( new QLabel( measurement_widget ) );
    measurements_pixinfo_uc.push_back( new QLabel( measurement_widget ) );
    measurements_pixinfo_slice[ measurements_pixinfo_slice.size() - 1 ]->setWordWrap(true);
    measurements_pixinfo_uc[ measurements_pixinfo_uc.size() - 1 ]->setWordWrap(true);
    measurements_pixinfo_slice[ measurements_pixinfo_slice.size() - 1 ]->setStyleSheet("font: 9pt;");
    measurements_pixinfo_uc[ measurements_pixinfo_uc.size() - 1 ]->setStyleSheet("font: 9pt;");
  }

  measurements_pixinfo_slice[0]->setText( "Voxel sizes (dx,dy,dz):" );
  measurements_pixinfo_uc[0]->setText( "Voxel sizes (dx,dy,dz):" );  
  
  // save tab
  choose_path_prefix = new QPushButton( "Choose location and prefix", save_widget );
  save_grid_control = new QPushButton( "Save grid", save_widget );
  save_surface_points_control = new QPushButton( "Save membrane points", save_widget );
  path_prefix_control = new QT_labeled_obj<QLineEdit>( "vl", "Path and prefix", save_widget );
  
  // buttons for parameters_widget
  button_save = new QPushButton("Save Image", parameters_widget);
  button_render = new QPushButton("Compute Projection", parameters_widget);

  button_read_pars = new QPushButton("Read parameters", parameters_widget );
  button_write_pars = new QPushButton("Write parameters", parameters_widget );  

  button_measure_vol = new QPushButton("Measure Volume", measurement_widget);
  button_measure_area = new QPushButton("Measure Area", measurement_widget);
  button_measure_percthres = new QPushButton("Compute percolation threshold", measurement_widget);

  measurement_object = new QComboBox( measurement_widget );
  measurement_object->insertItem( 0, "UC" );
  measurement_object->insertItem( 1, "Slice" );


  /*
   * structure control
   */ 
  
  uc_size_control_a = new QT_labeled_obj<QDoubleSpinBox> ( "vl", "Unit Cell Scale Factor (xy)", parameters_widget );
  uc_size_control_a->object()->setMinimum(0.001);
  uc_size_control_a->object()->setMaximum(10000.0);
  uc_size_control_a->object()->setSingleStep(0.01);
  uc_size_control_a->object()->setDecimals( 3 );
  uc_size_control_c = new QT_labeled_obj<QDoubleSpinBox> ( "vl", "Unit Cell Scale Factor  (z)", parameters_widget );
  uc_size_control_c->object()->setMinimum(0.001);
  uc_size_control_a->object()->setMaximum(10000.0);
  uc_size_control_c->object()->setSingleStep(0.01);
  uc_size_control_c->object()->setDecimals( 3 );
  
  channel_prop_control = new QT_labeled_obj<QDoubleSpinBox> ( "vl", "", parameters_widget );
  channel_prop_control->object()->setSingleStep(0.01);  

  level_par_type = new QT_labeled_obj<QComboBox> ( "vl", "Surface control parameter", parameters_widget );
  level_par_type->object()->insertItem( 0, surface_projection::parameter_names_hr[4].c_str() );
  level_par_type->object()->insertItem( 1, surface_projection::parameter_names_hr[3].c_str() );
  
  surface_type_control = new QT_labeled_obj<QComboBox> ( "vl", "Surface type", parameters_widget );
  std::vector<std::string> sfc_types = sp->get_surface_choices();
  for(unsigned int ii=0; ii<sfc_types.size(); ii++){
    surface_type_control->object()->insertItem( ii, QString( sfc_types.at(ii).c_str() ) );
  }

  
  //Set up the draw area
  draw_area = new QLabel( this );
  draw_area->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
  draw_area->setMinimumSize(500,500);
  draw_area->setSizePolicy(QSizePolicy::MinimumExpanding,
			   QSizePolicy::MinimumExpanding);

  orientation_visualisation = new QSvgWidget( parameters_widget );
  orientation_visualisation->setSizePolicy(QSizePolicy::MinimumExpanding,
                     QSizePolicy::MinimumExpanding);
  orientation_visualisation->setMinimumSize(120,120);
  /*
   * Slice control 
   */

  slice_thickness_control = new QT_labeled_obj<QDoubleSpinBox>( "hl", "Slice Thickness", parameters_widget);
  slice_thickness_control->object()->setRange(0.001,10000);
  slice_thickness_control->object()->setSingleStep(0.001);
  slice_thickness_control->object()->setDecimals( 3 );

  slice_height_control = new QT_labeled_obj<QDoubleSpinBox>( "hl", "Slice Height", parameters_widget);
  slice_height_control->object()->setRange(0.001,10000);
  slice_height_control->object()->setSingleStep(0.001);
  slice_height_control->object()->setDecimals( 3 );

  slice_width_control = new QT_labeled_obj<QDoubleSpinBox>( "hl", "Slice Width", parameters_widget);
  slice_width_control->object()->setRange(0.001,10000);
  slice_width_control->object()->setSingleStep(0.001);
  slice_width_control->object()->setDecimals( 3 );
  
  slice_position_control = new QT_labeled_obj<QDoubleSpinBox>( "hl", "Slice Position", parameters_widget);    
  slice_position_control->object()->setSingleStep(0.01);
  slice_position_control->object()->setRange( -1000, 1000 );
  slice_position_control->object()->setDecimals( 3 );

  button_set_to_uc_dim = new QPushButton ("Set to UC", parameters_widget );
  draw_uc_control = new QCheckBox( parameters_widget );
  draw_uc_control->setText("Draw unitcell");
  
  miller_h_control = new QT_labeled_obj<QSpinBox> ("hl", "h", parameters_widget );
  miller_h_control->object()->setRange(-500, 500);
  
  miller_k_control = new QT_labeled_obj<QSpinBox> ("hl", "k", parameters_widget );
  miller_k_control->object()->setRange(-500, 500);
  
  miller_l_control = new QT_labeled_obj<QSpinBox> ( "hl", "l", parameters_widget );  
  miller_l_control->object()->setRange(-500, 500);

  /*
   * membrane control
   */

  membranes_control = new QTableWidget( parameters_widget );
  membranes_control->insertColumn( 0 );
  membranes_control->insertColumn( 0 );

  QStringList table_header;
  table_header << "Distance" << "Width";
  membranes_control->setHorizontalHeaderLabels( table_header );
  membranes_control->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  
  add_membrane_control = new QPushButton ( "Add", parameters_widget );
  rm_membrane_control = new QPushButton( "Remove", parameters_widget );

  membranes_label = new QLabel( "Membranes", parameters_widget );

  fill_channels_label = new QLabel( "Fill in projection:", parameters_widget );
  fill_channels_control_container = new QScrollArea( parameters_widget );
  fill_channels_control_content = new QWidget( fill_channels_control_container );
  fill_channels_container_layout = new QVBoxLayout( fill_channels_control_content );

  fill_channels_control_container->setWidget( fill_channels_control_content );  


  /*
   * Resolution control
   */
  x_points_control = new QT_labeled_obj<QSpinBox> ( "vl", "X resolution", parameters_widget );
  x_points_control->object()->setRange(1, 600);

  z_points_control = new QT_labeled_obj<QSpinBox> ( "vl", "Z resolution", parameters_widget );  
  z_points_control->object()->setRange(1, 250);

  image_scaling_control = new QT_labeled_obj<QComboBox>( "vl", "Scaling", parameters_widget );
  std::vector<std::string> imgs_types = sp->get_img_scaling_choices();
  for(unsigned int ii=0; ii<imgs_types.size(); ii++){
    image_scaling_control->object()->insertItem( ii, QString( imgs_types.at(ii).c_str() ) );
  }
  
  invert_control = new QT_labeled_obj<QCheckBox>( "vl", "", parameters_widget );
  invert_control->object()->setText( "Invert image" );
  invert_control->object()->setCheckState( Qt::Checked );

  autoupdate_control = new QCheckBox( parameters_widget );
  autoupdate_control->setText( "Autoupdate" );
 
  render_pars_to_img_control = new QCheckBox( parameters_widget );
  render_pars_to_img_control->setText( "Render Parameters" );   

  

  /*
   * batch creation
   */

  //batch_instructions->setWordWrap(true);
  batch_instructions = new QLabel ( QString( ttips.batch_creation_instructions.c_str() ), batch_widget );
  batch_instructions->setWordWrap( true );
  
  batch_compute_start = new QPushButton("Go!", batch_widget );
  batch_compute_stop = new QPushButton("Stop", batch_widget );  

  batch_render_parameters = new QCheckBox( batch_widget );
  batch_render_parameters->setText("render parameters");
  
  batch_render_all_parameters = new QCheckBox( batch_widget );
  batch_render_all_parameters->setText("render all parameters");
  
  for( auto &it : surface_projection::parameter_names_hr ){
    // only add vol. prop. or surface level
    // must be possible to do more elegantly!!!
    if( ( it == surface_projection::parameter_names_hr[3] ||
	  it == surface_projection::parameter_names_hr[4] ) ){
      
      if( it == level_par_type->object()->currentText().toStdString() ){
	batch_values.push_back( new QT_labeled_obj<QLineEdit> ( "vl", it, batch_scroll_subwidget ) );
      }
      
    } else {    
      batch_values.push_back( new QT_labeled_obj<QLineEdit> ( "vl", it, batch_scroll_subwidget ) );
    }
  }

  batch_output_name = new QT_labeled_obj<QLineEdit> ( "vl", "Output folder and prefix", batch_widget );
  batch_choose_folder = new QT_labeled_obj<QPushButton> ( "vl", "", batch_widget );
  batch_choose_folder->object()->setText("Open Folder");  

  batch_progress = new QProgressBar( batch_scroll_subwidget );



  /*
   * post processing
   */


  gaussian_noise_magnitude = new QT_labeled_obj<QSpinBox> ( "hl", "Magnitude", pp_widget );
  gaussian_noise_magnitude->object()->setRange(1, 999);
  gaussian_noise_magnitude->object()->setValue( 50 );
  gaussian_blur_kernelsize = new QT_labeled_obj<QSpinBox> ( "hl", "Kernel Size", pp_widget );
  gaussian_blur_kernelsize->object()->setRange(1, 999);
  gaussian_blur_kernelsize->object()->setValue( 2 );  

  grains_gsize_c = new QT_labeled_obj<QSpinBox> ( "vl", "Size Center" );
  grains_gsize_c->object()->setRange(1, 999);
  grains_gsize_c->object()->setValue( 50 );  

  grains_gsize_s = new QT_labeled_obj<QSpinBox> ( "vl", "Size StdDev" );
  grains_gsize_s->object()->setRange(1, 999);
  grains_gsize_s->object()->setValue( 10 );  

  grains_gnumber_c = new QT_labeled_obj<QSpinBox> ( "vl", "Number Center" );
  grains_gnumber_c->object()->setRange(1, 999);
  grains_gnumber_c->object()->setValue( 50  );  

  grains_gnumber_s = new QT_labeled_obj<QSpinBox> ( "vl", "Number StdDev" );
  grains_gnumber_s->object()->setRange(1, 999);
  grains_gnumber_s->object()->setValue( 1 );  

  grains_magnitude = new QT_labeled_obj<QSpinBox> ( "vl", "Magnitude" );
  grains_magnitude->object()->setRange(1, 999);
  grains_magnitude->object()->setValue( 30 );  

  grains_kernelsize = new QT_labeled_obj<QSpinBox> ( "vl", "Blur" );
  grains_kernelsize->object()->setRange(0, 999);
  grains_kernelsize->object()->setValue( 4 );  

  grains_mix = new QT_labeled_obj<QDoubleSpinBox> ( "vl", "Mix" );
  grains_mix->object()->setRange(0.0, 1.0);
  grains_mix->object()->setValue( 0.5 );  

  gaussian_noise_activate = new QCheckBox ( pp_widget );
  gaussian_noise_activate->setText( "Gaussian Noise" );
  gaussian_noise_activate->setSizePolicy( QSizePolicy::Maximum,
					  QSizePolicy::Minimum);
  
  gaussian_blur_activate = new QCheckBox ( pp_widget );
  gaussian_blur_activate->setText( "Gaussian Blur" );
  gaussian_blur_activate->setSizePolicy( QSizePolicy::Maximum,
					 QSizePolicy::Minimum);  

  grains_activate = new QCheckBox ( pp_widget );
  grains_activate->setText( "Grains" );
  grains_activate->setSizePolicy( QSizePolicy::Maximum,
				  QSizePolicy::Minimum);
  
  h_line_4 = new QFrame( pp_widget );
  h_line_4->setFrameShape( QFrame::HLine );
  h_line_4->setFrameShadow( QFrame::Sunken );
  
  h_line_5 = new QFrame( pp_widget );
  h_line_5->setFrameShape( QFrame::HLine );
  h_line_5->setFrameShadow( QFrame::Sunken );
  
  h_line_6 = new QFrame( pp_widget );
  h_line_6->setFrameShape( QFrame::HLine );
  h_line_6->setFrameShadow( QFrame::Sunken );
  
  /*
   * Layout
   */
  
  //set up spacers and lines

  // 8 vspacer
  for( unsigned int ii=0; ii<8; ii++){
    v_spacer.push_back( new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding) );
  }
  // 7 hspacer
  for( unsigned int ii=0; ii<7; ii++){
    h_spacer.push_back( new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum) );
  }

  h_line_1 = new QFrame( parameters_widget );
  h_line_1->setFrameShape( QFrame::HLine );
  h_line_1->setFrameShadow( QFrame::Sunken );

  h_line_2 = new QFrame( parameters_widget );
  h_line_2->setFrameShape( QFrame::HLine );
  h_line_2->setFrameShadow( QFrame::Sunken );

  h_line_3 = new QFrame( parameters_widget );
  h_line_3->setFrameShape( QFrame::HLine );
  h_line_3->setFrameShadow( QFrame::Sunken );

  // status bar
  status_bar = new QStatusBar( this );
  status_bar_status_m = new QLabel( status_bar );
  status_bar_status_m->setAlignment(Qt::AlignCenter);
  status_bar_status_p = new QLabel( status_bar );
  status_bar_status_p->setAlignment(Qt::AlignCenter);  
  
  status_bar_or = new QLabel( status_bar );
  status_bar_pixs = new QLabel( status_bar );
  status_bar_uco = new QLabel( status_bar );
  status_bar_uc = new QLabel( status_bar );

  status_bar_uco->setTextFormat(Qt::RichText);

  status_bar->addPermanentWidget( status_bar_uc );  
  status_bar->addPermanentWidget( status_bar_or );
  status_bar->addPermanentWidget( status_bar_pixs );  
  status_bar->addPermanentWidget( status_bar_uco );  
  status_bar->addPermanentWidget( status_bar_status_m );
  status_bar->addPermanentWidget( status_bar_status_p );  
  
  // about widget


  about_us = new QLabel( about_widget );
  about_us->setText("SPIRE - Structure Projection Image Recognition Environment<br/>"
		    "Created by Tobias Hain"
		    " (<a href=\"mailto:hain@uni-potsdam.de\">hain@uni-potsdam.de</a>)<br/>"
		    "published under the <a href=\"https://www.gnu.org/licenses/gpl-3.0.html\">"
		    "GPLv3 license</a><br/>"
		    "based on the idea by Mark Mieczkowski");
  about_us->setTextFormat(Qt::RichText);
  about_us->setTextInteractionFlags(Qt::TextBrowserInteraction);
  about_us->setOpenExternalLinks(true);

  refs_ack = new QLabel( about_widget );
  refs_ack->setText("Using work from OTHER authors:<br/>"
		    "============================<br/>"
		    "Pedro F. Felzenszwalb and Daniel P. Huttenlocher<br/>"
		    "<a href=\"http://dx.doi.org/10.4086/toc.2012.v008a019\">"
		    " DOI: 10.4086/toc.2012.v008a019 </a><br/>"
		    "Chris Pudney<br/>"
		    "<a href=\"https://doi.org/10.1006/cviu.1998.0680\">"
		    " DOI: https://doi.org/10.1006/cviu.1998.0680 </a><br/>"
		    "J. Hoshen and R. Kopelman<br/>"
		    "<a href=\"https://doi.org/10.1103/PhysRevB.14.3438\">"
		    " DOI: https://doi.org/10.1103/PhysRevB.14.3438 </a><br/>"
		    "using <a href=\"https://www.openblas.net/\">libOpenBLAS</a>"
		    " published under the 3-clause license BSD license<br/>"
		    "using <a href=\"https://gmplib.org/\">libmgp</a>"		    
		    " published under <a href=\"https://www.gnu.org/licenses/lgpl-3.0.html\">GNU LGPL v3</a><br/>"
		    "using <a href=\"https://www.mpfr.org//\">libmpfr</a>"
		    " published under <a href=\"https://www.gnu.org/licenses/lgpl-3.0.html\">GNU LGPL v3</a><br/>"
		    "using <a href=\"https://cs.uwaterloo.ca/~astorjoh/iml.html\">"
		    "Integer Matrix Library</a>"
		    " published under <a href=\"https://www.gnu.org/licenses/old-licenses/gpl-2.0.html\">GNU GPL v2</a><br/>"
		    "using <a href=\"http://www.libpng.org/pub/png/libpng.html\">libpng</a>"
		    " published under <a href=\"http://www.libpng.org/pub/png/src/libpng-LICENSE.txt\">PNG License</a><br/>"
		    "using <a href=\"https://zlib.net/zlib.html\">zlib</a>"
		    " published under <a href=\"https://zlib.net/zlib_license.html\">zlib License</a><br/>"
		    "using <a href=\"https://www.qt.io/\">QT</a>"
		    " published under <a href=\"https://www.gnu.org/licenses/gpl-3.0.html\">GPL v3</a><br/>"
#ifdef USE_CGAL
		    "using <a href=\"https://www.cgal.org/index.html\">CGAL</a>"
		    " published under <a href=\"https://www.gnu.org/licenses/gpl-3.0.html\"></a>GPL v3<br/>"
#endif
		    );
  refs_ack->setTextFormat(Qt::RichText);
  refs_ack->setTextInteractionFlags(Qt::TextBrowserInteraction);
  refs_ack->setOpenExternalLinks(true);    



  // license pane;
  licenses = new QTextEdit();
  licenses->setReadOnly( true );


  QFile reading_device;
  QTextStream textstream;
  QString combined;
  
  std::vector<std::string> licenses_fn = {
    ":/resources/licenses/LICENSE_GPLv3.txt",
    ":/resources/licenses/LICENSE_lGPLv3.txt",
    ":/resources/licenses/LICENSE_GPLv2.txt",
    ":/resources/licenses/LICENSE_mBSD.txt",
    ":/resources/licenses/LICENSE_PNG.txt",
    ":/resources/licenses/LICENSE_ZLIB.txt"};
  
  for( auto &it : licenses_fn ){
    reading_device.setFileName( it.c_str() );
    reading_device.open( QFile::ReadOnly | QFile::Text );
    textstream.setDevice( &reading_device );
    combined.append( textstream.readAll() + "\n\n\n\n" );
    reading_device.close();
  }

  licenses->setText( combined );
  
  //the main layout of the form
  main_layout = new QHBoxLayout( this );
  
  draw_and_status_layout = new QVBoxLayout();
  draw_and_status_layout->addWidget( draw_area );
  draw_and_status_layout->addWidget( status_bar );
  
  main_layout->addWidget( controls );
  main_layout->addLayout( draw_and_status_layout );

  
  // the layout of the buttons
  buttons_layout = new QHBoxLayout();
  buttons_savewrite = new QVBoxLayout();
  buttons_projection = new QVBoxLayout();  

  // the layout of the tabs
  parameters_widget_layout = new QVBoxLayout( parameters_widget );

  save_widget_layout = new QVBoxLayout( save_widget );
  measurement_widget_layout = new QVBoxLayout( measurement_widget );
  measurement_widget_buttons_layout = new QHBoxLayout();
  measurement_pixinfo_uc_layout = new QHBoxLayout();
  measurement_pixinfo_slice_layout = new QHBoxLayout();  

  // batch creation
  batch_widget_layout = new QVBoxLayout( batch_widget );
  batch_widget_buttons_layout = new QHBoxLayout();
  batch_widget_parameters_layout = new QVBoxLayout( batch_scroll_subwidget );
  batch_widget_output_layout = new QHBoxLayout();
  
  // add all value lineedits
  for( auto it : batch_values ){
    batch_widget_parameters_layout->addLayout( it->layout() );
  }  

  // add buttons
  batch_widget_buttons_layout->addWidget( batch_render_parameters );
  batch_widget_buttons_layout->addWidget( batch_render_all_parameters );  
  batch_widget_buttons_layout->addWidget( batch_compute_stop );
  batch_widget_buttons_layout->addWidget( batch_compute_start );

  //output
  batch_widget_output_layout->addLayout( batch_output_name->layout() );
  batch_widget_output_layout->addLayout( batch_choose_folder->layout() );  

  // put it together
  batch_scroll_area->setWidget( batch_scroll_subwidget );

  batch_widget_layout->addWidget( batch_instructions );
  batch_widget_layout->addLayout( batch_widget_output_layout );
  batch_widget_layout->addWidget( batch_scroll_area );
  batch_widget_layout->addLayout( batch_widget_buttons_layout );
  batch_widget_layout->addWidget( batch_progress );


  //
  // post processing
  //
  
  pp_widget_layout = new QVBoxLayout( pp_widget );
  gaussian_noise_layout = new QHBoxLayout();
  gaussian_blur_layout = new QHBoxLayout();
  grains_layout = new QHBoxLayout();

  gaussian_noise_label_layout = new QHBoxLayout();
  gaussian_blur_label_layout = new QHBoxLayout();
  grains_label_layout = new QHBoxLayout();  


  gaussian_noise_label_layout->addWidget( gaussian_noise_activate );
  gaussian_noise_label_layout->addWidget( h_line_4 );

  gaussian_blur_label_layout->addWidget( gaussian_blur_activate );
  gaussian_blur_label_layout->addWidget( h_line_5 );

  grains_label_layout->addWidget( grains_activate );
  grains_label_layout->addWidget( h_line_6 );

  
  pp_widget_layout->addLayout( gaussian_noise_label_layout );
  gaussian_noise_layout->addLayout( gaussian_noise_magnitude->layout() );
  pp_widget_layout->addLayout( gaussian_noise_layout );

  pp_widget_layout->addLayout( gaussian_blur_label_layout );
  gaussian_blur_layout->addLayout( gaussian_blur_kernelsize->layout() );
  pp_widget_layout->addLayout( gaussian_blur_layout );

  pp_widget_layout->addLayout( grains_label_layout );
  grains_layout->addLayout( grains_gsize_c->layout() );
  grains_layout->addLayout( grains_gsize_s->layout() );
  grains_layout->addLayout( grains_gnumber_c->layout() );
  grains_layout->addLayout( grains_gnumber_s->layout() );
  grains_layout->addLayout( grains_magnitude->layout() );

  grains_layout->addLayout( grains_kernelsize->layout() );
  grains_layout->addLayout( grains_mix->layout() );
  
  pp_widget_layout->addLayout( grains_layout );
  pp_widget_layout->addItem( v_spacer[7] );
 

  //
  // measurement
  //

  
  measurement_widget_buttons_layout->addWidget( measurement_object );
  measurement_widget_buttons_layout->addWidget( button_measure_vol );
  measurement_widget_buttons_layout->addWidget( button_measure_area );  
  measurement_widget_buttons_layout->addWidget( button_measure_percthres );

  measurement_widget_layout->addLayout( measurements_uc->layout() );

  for( unsigned int ii=0; ii<measurements_pixinfo_uc.size(); ii++){
    measurement_pixinfo_uc_layout->addWidget( measurements_pixinfo_uc[ii] );
  }
  measurement_widget_layout->addLayout( measurement_pixinfo_uc_layout );

  measurement_widget_layout->addLayout( measurements_slice->layout() );

  for( unsigned int ii=0; ii<measurements_pixinfo_slice.size(); ii++){
    measurement_pixinfo_slice_layout->addWidget( measurements_pixinfo_slice[ii] );
  }
  measurement_widget_layout->addLayout( measurement_pixinfo_slice_layout );

  measurement_widget_layout->addLayout( measurement_widget_buttons_layout );
  
  save_widget_layout->addLayout( path_prefix_control->layout() );
  save_widget_layout->addWidget( choose_path_prefix );
  save_widget_layout->addItem( v_spacer[0] );  
  save_widget_layout->addWidget( save_grid_control );
  save_widget_layout->addWidget( save_surface_points_control );
    
  structure_settings = new QHBoxLayout();
  surface_level_settings = new QHBoxLayout();
  resolution_settings = new QHBoxLayout();
  slice_orientation_layout = new QVBoxLayout();
  slice_dimension_layout = new QVBoxLayout();
  slice_settings = new QHBoxLayout();  
  auto_uc_layout = new QVBoxLayout();
  membrane_settings = new QHBoxLayout();
  membrane_buttons_layout = new QHBoxLayout();
  
  structure_settings->addLayout( surface_type_control->layout() );  
  structure_settings->addLayout( uc_size_control_a->layout() );
  structure_settings->addLayout( uc_size_control_c->layout() );  

  surface_level_settings->addLayout( level_par_type->layout() );
  surface_level_settings->addLayout( channel_prop_control->layout() );
  surface_level_settings->addItem( h_spacer[0] );

  resolution_settings->addLayout( x_points_control->layout() );
  resolution_settings->addLayout( z_points_control->layout() );
  resolution_settings->addLayout( image_scaling_control->layout() );
  resolution_settings->addLayout( invert_control->layout() );
  

  slice_settings->addItem( h_spacer[1] );


  
  slice_dimension_layout->addLayout( slice_thickness_control->layout() );
  slice_dimension_layout->addLayout( slice_height_control->layout() );
  slice_dimension_layout->addLayout( slice_width_control->layout() );  
  slice_dimension_layout->addLayout( slice_position_control->layout() );

  slice_settings->addLayout( slice_dimension_layout );

  
  auto_uc_layout->addWidget( button_set_to_uc_dim );
  auto_uc_layout->addWidget( draw_uc_control );  

  slice_settings->addLayout(auto_uc_layout);
    
  slice_settings->addItem( h_spacer[2] );
  
  slice_orientation_layout->addLayout( miller_h_control->layout() );
  slice_orientation_layout->addLayout( miller_k_control->layout() );
  slice_orientation_layout->addLayout( miller_l_control->layout() );
  slice_orientation_layout->addItem( v_spacer[1] );

  slice_settings->addLayout( slice_orientation_layout );  

  slice_settings->addItem( h_spacer[3] );

  slice_settings->addWidget( orientation_visualisation );

  slice_settings->addItem( h_spacer[4] );
  
  membrane_buttons_layout->addWidget( membranes_label );
  membrane_buttons_layout->addWidget( add_membrane_control );
  membrane_buttons_layout->addWidget( rm_membrane_control );
  membrane_buttons_layout->addItem( h_spacer[5] );
  membrane_buttons_layout->addWidget( fill_channels_label );

  //membrane_settings->addLayout( membrane_buttons_layout );
  membrane_settings->addWidget( membranes_control );
  membrane_settings->addWidget( fill_channels_control_container );



  // put it all together into the main layout
  parameters_widget_layout->addLayout( structure_settings );
  parameters_widget_layout->addLayout( surface_level_settings );


  parameters_widget_layout->addItem( v_spacer[2] );
  parameters_widget_layout->insertSpacing( -1, int(space_between_items/2) );
  parameters_widget_layout->addWidget( h_line_1 );
  parameters_widget_layout->insertSpacing( -1, int(space_between_items/2) );  

  parameters_widget_layout->addLayout( slice_settings );

  parameters_widget_layout->addItem( v_spacer[3] );
  parameters_widget_layout->insertSpacing( -1, int(space_between_items/2) );
  parameters_widget_layout->addWidget( h_line_2 );
  parameters_widget_layout->insertSpacing( -1, int(space_between_items/2) );
  
  parameters_widget_layout->addLayout( membrane_buttons_layout );
  parameters_widget_layout->addLayout( membrane_settings );

  parameters_widget_layout->addItem( v_spacer[4] );
  parameters_widget_layout->insertSpacing( -1, int(space_between_items/2) );
  parameters_widget_layout->addWidget( h_line_3 );
  parameters_widget_layout->insertSpacing( -1, int(space_between_items/2) );  
  
  parameters_widget_layout->addLayout( resolution_settings );

  parameters_widget_layout->addItem( v_spacer[5] );
  parameters_widget_layout->insertSpacing( -1, space_between_items );
  
  parameters_widget_layout->addLayout( buttons_layout );
    
  buttons_projection->addWidget( button_render );
  buttons_projection->addWidget( autoupdate_control );
  buttons_projection->addWidget( render_pars_to_img_control );  

  buttons_savewrite->addWidget( button_write_pars );
  buttons_savewrite->addWidget( button_read_pars );
  buttons_savewrite->addWidget( button_save );

  buttons_layout->addLayout( buttons_savewrite );
  buttons_layout->addItem( h_spacer[6] );
  buttons_layout->addLayout( buttons_projection );


  about_widget_layout = new QVBoxLayout( about_widget );

  about_widget_layout->addWidget( about_us );
  about_widget_layout->addItem( v_spacer[6] );

  about_widget_layout->addWidget( refs_ack );
  
  license_widget_layout = new QVBoxLayout( license_widget );

  license_widget_layout->addWidget( licenses );
  
}


/**
 * Updates the batch parameter tab, depending if level set or
 * vol. proportion is chosen.
 */
void GUI::update_batch_parameters_gui(){

  delete( batch_widget_parameters_layout );
  delete( batch_scroll_subwidget );
  batch_values.clear();
  
  batch_scroll_subwidget = new QWidget( batch_scroll_area );
  batch_scroll_subwidget->setMinimumWidth( 350 );
  batch_scroll_subwidget->setSizePolicy( QSizePolicy::Expanding,
					 QSizePolicy::Expanding);  

  batch_widget_parameters_layout = new QVBoxLayout( batch_scroll_subwidget );

  
  
  for( auto &it : surface_projection::parameter_names_hr ){
    // only add vol. prop. or surface level
    // must be possible to do more elegantly!!!
    if( it == surface_projection::parameter_names_hr[3] ||
	it == surface_projection::parameter_names_hr[4] ) {

      if( it == level_par_type->object()->currentText().toStdString() ){
	batch_values.push_back( new QT_labeled_obj<QLineEdit> ( "vl", it, batch_scroll_subwidget ) );
      }
      
    } else {    
      batch_values.push_back( new QT_labeled_obj<QLineEdit> ( "vl", it, batch_scroll_subwidget ) );
    }
  }


  // add all value lineedits
  for( auto it : batch_values ){
    batch_widget_parameters_layout->addLayout( it->layout() );
  }  
  batch_scroll_area->setWidget( batch_scroll_subwidget );
  
  
}



/// assigns all the tooltips to the objects
void GUI::set_up_tooltips(){

   uc_size_control_a->object()->setToolTip( QString( ttips.aa_tooltip.c_str() ) );
   uc_size_control_c->object()->setToolTip( QString( ttips.ac_tooltip.c_str() ) );
   channel_prop_control->object()->setToolTip( QString( ttips.level_set_tooltip.c_str() ) );

   level_par_type->object()->setToolTip( QString( ttips.level_par_tooltip.c_str() ) );
   
   surface_type_control->object()->setToolTip( QString( ttips.type_tooltip.c_str() ) );

   slice_thickness_control->object()->setToolTip( QString( ttips.slicewidth_tooltip.c_str() ) );
   slice_width_control->object()->setToolTip( QString( ttips.slicelength_tooltip.c_str() ) );
   slice_height_control->object()->setToolTip( QString( ttips.sliceheight_tooltip.c_str() ) );   
   slice_position_control->object()->setToolTip( QString( ttips.sliceposition_tooltip.c_str() ) );   
   
   button_set_to_uc_dim->setToolTip( QString( ttips.set_to_uc_dim_tooltip.c_str() ) );
   draw_uc_control->setToolTip ( QString( ttips.draw_uc_tooltip.c_str() ) );
   
   miller_h_control->object()->setToolTip( QString( ttips.hkl_tooltip.c_str() ) );
   miller_k_control->object()->setToolTip( QString( ttips.hkl_tooltip.c_str() ) );
   miller_l_control->object()->setToolTip( QString( ttips.hkl_tooltip.c_str() ) );

   x_points_control->object()->setToolTip( QString( ttips.nr_points_xy_tooltip.c_str() ) );
   z_points_control->object()->setToolTip( QString( ttips.nr_points_z_tooltip.c_str() ) );

   invert_control->object()->setToolTip( QString( ttips.invert_tooltip.c_str() ) );
   autoupdate_control->setToolTip( QString( ttips.autoupdate_tooltip.c_str() ) );
   render_pars_to_img_control->setToolTip( ttips.render_pars_to_img_tooltip.c_str() );
   image_scaling_control->object()->setToolTip( QString( ttips.image_scaling_tooltip.c_str() ) );

   membranes_control->setToolTip ( QString( ttips.membranes_settings_tooltip.c_str() ) );
   add_membrane_control->setToolTip ( QString( ttips.membranes_add_tooltip.c_str() ) );
   rm_membrane_control->setToolTip ( QString( ttips.membranes_remove_tooltip.c_str() ) );   

   fill_channels_control_container->setToolTip( QString( ttips.channel_fill_tooltip.c_str() ) );

   button_measure_vol->setToolTip( QString( ttips.button_measure_v.c_str() ) );
   button_measure_area->setToolTip( QString( ttips.button_measure_a.c_str() ) );   
   button_measure_percthres->setToolTip( QString( ttips.button_measure_percthres.c_str() ) );
   
   button_render->setToolTip( QString( ttips.button_render.c_str() ) );
   button_save->setToolTip( QString( ttips.button_save.c_str() ) );
   button_read_pars->setToolTip( QString( ttips.button_read_pars.c_str() ) );
   button_write_pars->setToolTip( QString( ttips.button_write_pars.c_str() ) );   

   save_grid_control->setToolTip( QString( ttips.button_save_grid.c_str() ) );
   save_surface_points_control->setToolTip( QString( ttips.button_save_membranes.c_str() ) );
   

   path_prefix_control->object()->setToolTip( QString( ttips.save_prefix_tooltip.c_str() ) );
   
   choose_path_prefix->setToolTip( QString( ttips.choose_prefix_path.c_str() ) );


   batch_output_name->object()->setToolTip( ttips.batch_output_name_tooltip.c_str() );
   batch_choose_folder->object()->setToolTip( ttips.batch_choose_folder_tooltip.c_str() );   
   batch_compute_start->setToolTip( ttips.batch_compute_start_tooltip.c_str() );
   batch_compute_stop->setToolTip( ttips.batch_compute_stop_tooltip.c_str() );
   batch_render_parameters->setToolTip( ttips.batch_render_parameters_tooltip.c_str() );
   batch_render_all_parameters->setToolTip( ttips.batch_render_all_parameters_tooltip.c_str() );

   
   status_bar_uco->setToolTip( QString( ttips.status_uco_tooltip.c_str() ) );
   status_bar_or->setToolTip( QString( ttips.status_or_tooltip.c_str() ) );
   status_bar_pixs->setToolTip( QString( ttips.status_pixs_tooltip.c_str() ) );
   status_bar_uc->setToolTip( QString( ttips.status_uc_tooltip.c_str() ) );   
}


/// connects all the signals and slots
void GUI::set_up_signals_and_slots(){

  
  /*
   * update parameters
   */

  // structure type
  connect(surface_type_control->object(), QOverload<int>::of(&QComboBox::currentIndexChanged),
	  sp, &sp_qt::change_surface_type);

  // x resolution
  connect(x_points_control->object(), &QSpinBox::editingFinished, this, &GUI::set_parameter );
  // z resolution
  connect(z_points_control->object(), &QSpinBox::editingFinished, this, &GUI::set_parameter );

  // unit cell size
  connect( uc_size_control_a->object(), &QDoubleSpinBox::editingFinished, this, &GUI::set_parameter );
  connect( uc_size_control_c->object(), &QDoubleSpinBox::editingFinished, this, &GUI::set_parameter );


  // channel proportion / level set
  connect( channel_prop_control->object(), &QDoubleSpinBox::editingFinished,
	   this, &GUI::set_parameter );
  connect(level_par_type->object(), QOverload<int>::of(&QComboBox::currentIndexChanged),
	  this, &GUI::change_surface_par_type);

  // slice dimensions
  connect( slice_thickness_control->object(), &QDoubleSpinBox::editingFinished, this, &GUI::set_parameter);
  connect( slice_height_control->object(), &QDoubleSpinBox::editingFinished, this, &GUI::set_parameter);
  connect( slice_width_control->object(), &QDoubleSpinBox::editingFinished, this, &GUI::set_parameter);  
  // slice position
  connect( slice_position_control->object(), &QDoubleSpinBox::editingFinished,
	   this, &GUI::set_parameter );

  // set to uc
  connect( button_set_to_uc_dim, SIGNAL( clicked() ), sp, SLOT( set_slice_dim_to_uc() ) );
  // draw unit cell?
  connect( draw_uc_control, SIGNAL( stateChanged(int) ), this, SLOT( update_view() ) );  
  
  // miller hkl
  connect( miller_h_control->object(), QOverload<int>::of(&QSpinBox::valueChanged),
	   this, &GUI::change_orientation );
  connect( miller_k_control->object(), QOverload<int>::of(&QSpinBox::valueChanged),
  	   this, &GUI::change_orientation );
  connect( miller_l_control->object(), QOverload<int>::of(&QSpinBox::valueChanged),
	   this, &GUI::change_orientation );
  connect( this, &GUI::call_change_hkl, sp, &sp_qt::change_hkl );


  /*
   *  post_processing
   */
  connect( gaussian_noise_activate, SIGNAL( stateChanged(int) ), this, SLOT( update_view() ) );
  connect( gaussian_blur_activate, SIGNAL( stateChanged(int) ), this, SLOT( update_view() ) );
  connect( grains_activate, SIGNAL( stateChanged(int) ), this, SLOT( update_view() ) );      

  connect( gaussian_noise_magnitude->object(), &QSpinBox::editingFinished,
	   this, &GUI::update_view );
  connect( gaussian_blur_kernelsize->object(), &QSpinBox::editingFinished,
	   this, &GUI::update_view );

  connect( grains_gnumber_c->object(), &QSpinBox::editingFinished,
	   this, &GUI::update_view );
  connect( grains_gnumber_s->object(), &QSpinBox::editingFinished,
	   this, &GUI::update_view );
  connect( grains_gsize_c->object(), &QSpinBox::editingFinished,
	   this, &GUI::update_view );
  connect( grains_gsize_s->object(), &QSpinBox::editingFinished,
	   this, &GUI::update_view );
  connect( grains_magnitude->object(), &QSpinBox::editingFinished,
	   this, &GUI::update_view );

  connect( grains_mix->object(), &QDoubleSpinBox::editingFinished,
	   this, &GUI::update_view );
  connect( grains_kernelsize->object(), &QSpinBox::editingFinished,
	   this, &GUI::update_view );
  
    

  /*
   * gui stuff
   */

  //buttons
  connect( button_render, SIGNAL( clicked() ), sp, SLOT( compute_projection() ) );  
  connect( button_save, SIGNAL( clicked() ), this, SLOT( save_image_to_file() ) );

  connect( button_measure_vol, SIGNAL( clicked() ), this, SLOT( measure_vol() ) );
  connect( button_measure_area, SIGNAL( clicked() ), this, SLOT( measure_area() ) );  
  connect( button_measure_percthres, SIGNAL( clicked() ), this, SLOT( measure_percolation() ) );  
  
  connect( button_read_pars, SIGNAL( clicked() ), this, SLOT( read_parameters() ) );
  connect( button_write_pars, SIGNAL( clicked() ), this, SLOT( write_parameters() ) );
  connect( this, &GUI::call_read_pars_from_file, sp, &sp_qt::read_pars_from_file );


  connect( add_membrane_control, SIGNAL( clicked() ), this, SLOT( add_membrane() ) );
  connect( rm_membrane_control, SIGNAL( clicked() ), this, SLOT( rm_membrane() ) );  

  connect( membranes_control, &QTableWidget::cellChanged, this, &GUI::write_membranes );
  
  // redraw picture when surface_projection class updated the projection
  connect( sp, &sp_qt::projection_changed, this, &GUI::update_view );


  // read the updated parameters
  connect( sp, &sp_qt::parameter_changed, this, &GUI::update_gui_from_sp );

  connect( sp, &sp_qt::updated_membranes, this, &GUI::read_membranes );

  connect( this, &GUI::call_change_channel_color, sp, &sp_qt::change_channel_color );
  connect( sp, &sp_qt::updated_channel_fill, this, &GUI::update_measurements_structure );

  
  // actiave autoupdate
  connect( autoupdate_control, SIGNAL( stateChanged(int) ), this, SLOT( change_autoupdate(int) ) );

  //invert or not
  connect( invert_control->object(), SIGNAL( stateChanged(int) ), this, SLOT( update_view() ) );  

  // scaling
  connect(image_scaling_control->object(), QOverload<int>::of(&QComboBox::currentIndexChanged),
	  this, &GUI::update_view);  
  
  // get messages from subclass
  connect( sp, &sp_qt::send_message, this, &GUI::output_message );
  connect( sp, &sp_qt::set_status, this, &GUI::set_state );
  connect( sp_stats, &sp_qt::send_message, this, &GUI::output_message );  
  connect( sp_stats, &sp_qt::set_status, this, &GUI::set_state );
  
  //projection
  connect( this, &GUI::call_compute_projection, sp, &sp_qt::compute_projection );

  // measurements
  connect( this, &GUI::call_update_stats, sp_stats, &sp_qt::update_measurements );
  connect( sp_stats, &sp_qt::measurements_updated, this, &GUI::update_measurements_values );  
  connect( this, &GUI::call_set_measurement_status, this, &GUI::set_measurements_status );



  
  // batch
  connect( batch_choose_folder->object(), &QPushButton::clicked, this, &GUI::set_batch_output );
  connect( batch_compute_start, &QPushButton::clicked, this, &GUI::start_batch_computing );
  connect( batch_compute_stop, &QPushButton::clicked, this, &GUI::stop_batch_computing );  
  connect( this, &GUI::emit_start_batch, bc, &qt_bc::start_loop );
  connect( cb, &gui_callback::updated_batch_progress, batch_progress, &QProgressBar::setValue );
  connect( bc, &qt_bc::finished_batch_loop, this, &GUI::finalize_batch_loop );
  connect( this, &GUI::emit_stop_batch, cb, &gui_callback::update_status );

  connect( batch_render_parameters, &QCheckBox::stateChanged,
	   cb, &gui_callback::set_render_parameters );
  connect( batch_render_all_parameters, &QCheckBox::stateChanged,
	   cb, &gui_callback::set_render_all_parameters );
  
  // saving
  connect( choose_path_prefix, &QPushButton::clicked, this, &GUI::choose_export_prefix );
  connect( save_grid_control, &QPushButton::clicked, this, &GUI::save_grid );
  connect( save_surface_points_control, &QPushButton::clicked, this, &GUI::save_surface_points );


  connect( this, &GUI::call_save_grid, sp, &sp_qt::save_grid );
  connect( this, &GUI::call_save_surface, sp, &sp_qt::save_surface_points );
  connect( this, &GUI::call_save_network, sp_stats, &sp_qt::save_topological_network );    
  
  
}


/// Constructor. initializes background objects and threads
GUI::GUI( QApplication *_app, QLocale *def_locale_, global_settings &gs_, int seed, QWidget *parent  ) : QWidget( parent ), app(_app), def_locale( def_locale_ ), gs( gs_ ){
  
  //initialize surface projection
  sp = new sp_qt( gs );
  sp->set_n_points_x( 150 );
  sp->set_n_points_z( 75 );  
  sp->update_geometry();
  sp->update_containers();

  //initialize surface projection
  
  sp_stats = new sp_qt( gs );
  sp_stats->set_n_points_x( 76 );  
  sp_stats->set_n_points_z( 76 );  
  sp_stats->update_geometry();
  sp_stats->update_containers();


  // initialise batch creation object
  bc = new qt_bc( *sp, seed  );

  // initialise the callback instance
  cb = new gui_callback( *sp, *bc, "./projection" );
  // pass the pointer
  bc->set_callback( dynamic_cast<sp_callback*> (cb) );
  
  set_up_ui();
  set_up_tooltips();

  update_gui_from_sp();
  read_membranes();
  update_measurements_structure();
  update_fill_channels();
  
  //set a black background iamge
  img_pix = new QPixmap( 100, 100 );
  img_pix->fill( "#ffffff" );
  
  thread = new QThread();
  sp->moveToThread( thread );
  thread->start();

  t_stats = new QThread();
  sp_stats->moveToThread( t_stats );
  t_stats->start();  
  
  bc_thread = new QThread();
  bc->moveToThread( bc_thread );
  bc_thread->start();

  cb_thread = new QThread();
  cb->moveToThread( cb_thread );
  cb_thread->start();
    
  set_up_signals_and_slots();
  
  //measure();
  emit call_compute_projection();  
}

/// destructor. free memory
GUI::~GUI(){

  // close the threads and wait for them to finish
  thread->exit();
  t_stats->exit();
  thread->wait();
  t_stats->wait();
  bc_thread->exit();
  bc_thread->wait();
  cb_thread->exit();
  cb_thread->wait();  
  
  
  delete( thread );
  delete( t_stats );
  delete( bc_thread );
  delete( cb_thread );
  delete( sp );
  delete( sp_stats );
  delete( bc );
  delete( cb );
  delete (img_pix );

}


/// quit app in a nice way. not used right now
void GUI::quit_app(){

  auto reply = QMessageBox::question( this, "Really quit?", "Are you sure you want to quit?", QMessageBox::Yes|QMessageBox::No );
  
  if( reply == QMessageBox::Yes ){
    thread->quit();
    t_stats->quit();
    bc_thread->quit();
    app->quit();
  } else {
    // do nothing, just close dialog box
  }
  
}


/// overlays the unit map on the provided canvas. Assumes pixsizes and
/// dimension of the canvas equal to projection image
void GUI::draw_unitcell( QPaintDevice *canvas ){

  // get a new painter

  uc_artist = new QPainter();
  uc_artist->begin( canvas );
  
  
  // compute the pixles to draw the line
  
  // get the base vectors already in euclidean base
  std::vector<double> base = sp->get_uc_base();
  std::vector<double> a = {base[0], base[1], base[2]};
  std::vector<double> b = {base[3], base[4], base[5]};
  std::vector<double> c = {base[6], base[7], base[8]};  
  
  // get an orhto-normal base vector system
  std::vector<double> x = VEC_MAT_MATH::get_unit( a );
  std::vector<double> z = VEC_MAT_MATH::get_unit( c );
  std::vector<double> y = VEC_MAT_MATH::cross_prod( z, x );

  // express the base vectors a,b in terms of the new base
  double x_len = VEC_MAT_MATH::dot_prod( a, x);
  double yx = VEC_MAT_MATH::dot_prod( b, x );
  double yy = VEC_MAT_MATH::dot_prod( b, y );
  double yz = VEC_MAT_MATH::dot_prod( b, z );
  
  int mid_x = int( 0.5 * sp->get_width() );
  int mid_y = int( 0.5 * sp->get_height() );  
  
  //convert length to pixel
  int ax = int( x_len / sp->get_dx() );
  int bx = int( yx / sp->get_dx() );
  int by = int( yy / sp->get_dy() );

  // get the corner points of the unit cell centered around the origin
  int tl_x = mid_x - int(0.5*ax) + int(0.5*bx);
  int tl_y = mid_y + int(0.5*by);

  int tr_x = tl_x + ax;
  int tr_y = tl_y;

  int br_x = tr_x - bx;
  int br_y = tr_y - by;

  int bl_x = br_x - ax;
  int bl_y = br_y;

  // draw the lines


  // compute width, want higher widths for higer resolution, otherwise
  // the lies get very thin
  
  int stroke_width = int( (sp->get_width() / 100.0 ) );

  //uc_artist->pen().setWidth(2);
  QPen *pen = new QPen();
  pen->setColor( gs.g_color_b1.c_str() );
  pen->setWidth( stroke_width );
  uc_artist->setPen( *pen );
  
  uc_artist->drawLine( tl_x, tl_y, tr_x, tr_y );
  uc_artist->drawLine( br_x, br_y, bl_x, bl_y );

  pen->setColor( gs.g_color_b2.c_str() );
  uc_artist->setPen( *pen );

  uc_artist->drawLine( tr_x, tr_y, br_x, br_y );
  uc_artist->drawLine( bl_x, bl_y, tl_x, tl_y );
  delete( uc_artist );
  
}

/// updates the projection from the image of the \ref
/// surface_projection object. Does not recompute the projection
void GUI::update_view(){  

  //deactivate_pp_controls();
  
  unsigned char *img_data = sp->get_image( false,
					   image_scaling_control->object()->currentText().toStdString() );

  
  // apply post-processing effects here!

  
  if( gaussian_blur_activate->isChecked() ){
    image_manipulation::gaussian_blur( img_data, sp->get_width(), sp->get_height(),
				       gaussian_blur_kernelsize->object()->value() );
  }
  
  
  if( grains_activate->isChecked() ){
    
    unsigned char* grains = image_manipulation::create_grains( sp->get_width(), sp->get_height(),
							       grains_gsize_c->object()->value(),
							       grains_gsize_s->object()->value(),
							       grains_gnumber_c->object()->value(),
							       grains_gnumber_s->object()->value(),
							       grains_magnitude->object()->value() );
    
    
    if( grains_kernelsize->object()->value() > 0 ){
      image_manipulation::gaussian_blur( grains, sp->get_width(), sp->get_height(),
					 grains_kernelsize->object()->value() );
      
    }
    
    
    
    image_manipulation::add_images( img_data, grains, 1.0-grains_mix->object()->value(), grains_mix->object()->value(), sp->get_width(), sp->get_height(), img_data );
    
    //img_data = grains;
    
  }

  if( gaussian_noise_activate->isChecked() ){
    image_manipulation::gaussian_noise( img_data, sp->get_width(), sp->get_height(),
    					gaussian_noise_magnitude->object()->value() );
    
  }
  

  if( invert_control->object()->isChecked() ){

    image_manipulation::invert( img_data, sp->get_width(), sp->get_height() );
    
  }
  
  // this does *NOT* copy the img_data into its own object, so
  // keep that img_data array around!
  QImage image = QImage( img_data, sp->get_width(), sp->get_height(),
			 sp->get_width(), QImage::Format_Grayscale8 );
  
  
  // load the image into the pixmap
  img_pix->convertFromImage( image );

  
  //draw the unit cell if desired
  if( draw_uc_control->isChecked() ){
    draw_unitcell( img_pix );
  }

  // display
  draw_area->setPixmap( *img_pix);

  delete[] ( img_data );

  //activate_pp_controls();
}



/// reimplement the paintEvet so the image will be redrawn to the
/// correct size and aspect ratio when windows is resized
void GUI::paintEvent( QPaintEvent * event ){

   if( img_pix != NULL ){

     //get label dimensions
    int w = draw_area->width();
    int h = draw_area->height();
    
    //set a scaled pixmap to a w x h window keeping its aspect ratio 
    draw_area->setPixmap(img_pix->scaled(w,h,Qt::KeepAspectRatio));
  }

}




/// reads the parameters from the surface_projection objects and
/// writes them to the GUI
void GUI::update_gui_from_sp(){


  if( level_par_type->object()->currentIndex() == 0 ){
    channel_prop_control->object()->setValue( sp->get_channel_vol_prop() );
  } else {
    channel_prop_control->object()->setValue( sp->get_surface_level() );
  }

  surface_type_control->object()->setCurrentIndex( sp->get_type() );  

  uc_size_control_a->object()->setValue( sp->get_uc_scale_ab() );
  uc_size_control_c->object()->setValue( sp->get_uc_scale_c() );

  if( sp->get_surface_choices()[sp->get_type()].substr(0,8) == "Wurtzite" ){
    uc_size_control_c->object()->setEnabled( true );
  } else {
    uc_size_control_c->object()->setEnabled( false );
  }
  
  z_points_control->object()->setValue( sp->get_depth() );
  x_points_control->object()->setValue( sp->get_width() );

  slice_thickness_control->object()->setValue( sp->get_slice_thickness() );
  slice_height_control->object()->setValue( sp->get_slice_height() );
  slice_width_control->object()->setValue( sp->get_slice_width() );  
  slice_position_control->object()->setValue( sp->get_slice_position() );  
  
  miller_h_control->object()->setValue( sp->get_h() );
  miller_k_control->object()->setValue( sp->get_k() );
  miller_l_control->object()->setValue( sp->get_l() );  

  update_stats( );

  std::vector<double> uc_dim = sp->get_ucdim();
  std::vector<double> base = sp->get_uc_base();
  std::string hkl_visual = draw_slice_visualization( base, uc_dim, gs );
  QByteArray tmp_arr ( hkl_visual.c_str(), -1 );  
  orientation_visualisation->load( tmp_arr );
  
}

/// writes the membranes from the table view object to the surface_projection object
void GUI::write_membranes(int row, int col){
  
  //get the data from the table
  int rows = membranes_control->rowCount();
  int cols = membranes_control->columnCount();

  std::vector<double> new_membranes( rows*2, 0 );

  for(unsigned int rr=0; rr<rows; rr++){

    double dist, width;
    try {

      bool ok;
      
      dist = def_locale->toDouble( membranes_control->item( rr, 0 )->text(), &ok );
      if( !ok ){
	dist = membranes_control->item( rr, 0 )->text().toDouble();
      }
      
      width = def_locale->toDouble( membranes_control->item( rr, 1 )->text(), &ok );
      if( !ok ){
	width = membranes_control->item( rr, 1 )->text().toDouble();
      }
      
    } catch ( std::invalid_argument e ) {
      output_message( "Invalid argument, resetting to last good configuration" );
      read_membranes();
      return;
    }

    if( rr== 0 ){
      if( dist != 0 || width < 0.02 ){
	dist = 0;
	width = 0.02;
	output_message( "Innermost membrane must be at d=0 and a minimum width of 0.02" );
      }  
    }

    new_membranes[2*rr] = dist;
    new_membranes[2*rr+1] = width;
  }

  sp->change_membranes( new_membranes );
  
}


/// reads the membranes from the surface_projection object to the GUI
void GUI::read_membranes(){

  //temporarliry disconnect signals
  disconnect( membranes_control, &QTableWidget::cellChanged, this, &GUI::write_membranes );
  
  std::vector<double> membranes = sp->get_membranes();

  for(unsigned int ii=0; ii<membranes.size(); ii+=2){

    if( ii/2 >= membranes_control->rowCount() ){
      membranes_control->insertRow( ii/2  );

      membranes_control->setItem( ii/2, 0, new QTableWidgetItem() );
      membranes_control->setItem( ii/2, 1, new QTableWidgetItem() );  
    }
    
    membranes_control->item( ii/2, 0)->setText( def_locale->toString( membranes.at(ii) ) );
    membranes_control->item( ii/2, 1)->setText( def_locale->toString( membranes.at(ii+1) ) ); 
  }

  update_fill_channels();

  //reconnect them
  connect( membranes_control, &QTableWidget::cellChanged, this, &GUI::write_membranes );
  
}

/// add a membrane to the tableview and surface_projection object
void GUI::add_membrane( double first, double second ){

  //temporarliry disconnect signals
  disconnect( membranes_control, &QTableWidget::cellChanged, this, &GUI::write_membranes );
  
  int curRow = membranes_control->rowCount();
  membranes_control->insertRow( curRow  );

  membranes_control->setItem( curRow, 0, new QTableWidgetItem() );
  membranes_control->setItem( curRow, 1, new QTableWidgetItem() );  


  membranes_control->item( curRow, 0)->setText( def_locale->toString( first ) );
  membranes_control->item( curRow, 1)->setText( def_locale->toString( second ) );

  write_membranes( 0, 0 );

  update_measurements_structure();
  update_fill_channels();
  
  //reconnect them
  connect( membranes_control, &QTableWidget::cellChanged, this, &GUI::write_membranes );
}


/// delete a membrane
void GUI::rm_membrane(){
  int ind = membranes_control->currentRow();
  if( ind > 0 ){
    membranes_control->removeRow( ind );
    write_membranes(0, 0);
    update_measurements_structure();
    update_fill_channels();    
  }
  else {
    output_message( "Innermost membrane can't be removed" );
  }
}

/// saves the current image to a file. Adds a legend in the margin if desired
void GUI::save_image_to_file(){

  QString filename = QFileDialog::getSaveFileName( this, "Select File", "./", tr("*.png"), NULL, QFileDialog::DontUseNativeDialog );
  
  if( !filename.endsWith( ".png", Qt::CaseInsensitive ) ){
    filename.append(".png");
  }  

  if( render_pars_to_img_control->isChecked() ){

    uchar *img_data = sp->get_image( invert_control->object()->isChecked(),
				     image_scaling_control->object()->currentText().toStdString() );

    QImage qimg = QImage( img_data, sp->get_width(), sp->get_height(),
			  sp->get_width(), QImage::Format_Grayscale8 );
    
    if( draw_uc_control->isChecked() ){
      draw_unitcell( &qimg );
    }img_data = sp->save_png_legend( img_data, qimg.width(), qimg.height(),
				    invert_control->object()->isChecked()
				    ? std::string("#ffffff") : std::string("#000000"),
				    invert_control->object()->isChecked()
				    ? std::string("#000000") : std::string("#ffffff"),
				    filename.toStdString() , std::vector<std::string> (0,"") );
    

    
    free( img_data );
    
  } else {    
    img_pix->save( filename );
  }
}


/*
 * types:
 * 0 = status_p
 * 1 = message
 * 2 = stats
 * 3 = status_m
 */
void GUI::output_message( QString msg, int type ){

  std::stringstream text;
  
  if( type == 0 ){
    text.str("");
    text << "Projection" << std::endl << std::endl << msg.toStdString();
    status_bar_status_p->setText( QString( text.str().c_str() )  );
  } else if ( type == 1 ){
    status_bar->showMessage( msg );
  } else if( type == 3 ){
    text.str("");
    text << "Measurement" << std::endl << std::endl << msg.toStdString();
    status_bar_status_m->setText( text.str().c_str() );
  }

}



void GUI::change_orientation( int val ){

  int h = miller_h_control->object()->value();
  int k = miller_k_control->object()->value();
  int l = miller_l_control->object()->value();  

  emit call_change_hkl( h, k, l );
  
}


/// updates the status bar
void GUI::update_stats(){

  auto uc_dim = sp->get_uc_dim_in_orientation();
  std::stringstream uc_orient_info, orient_info, pix_info, uc_dim_info;
  
  uc_orient_info << "UC in orientation<br/>";
  uc_orient_info << "<font color=\"" << gs.g_color_b1 << "\">v:  " << uc_dim[0] << "</font><br/>";
  uc_orient_info << "<font color=\"" << gs.g_color_b2 << "\">w:  " << uc_dim[1] << "</font><br/>";
  uc_orient_info << "<font color=\"" << gs.g_color_n << "\">n:  " << uc_dim[2] << "</font><br/>"; 
  
  pix_info << "Resolution (pixel size)" << std::endl;
  pix_info << "X: " << std::setw(5) << sp->get_width()
	   << " (" << std::setprecision(2) << sp->get_dx()
	   << ")" << std::endl;
  pix_info << "Y: " << std::setw(5) << sp->get_height()
	   << " (" << std::setprecision(2) << sp->get_dy()
	   << ")" << std::endl;
  pix_info << "Z: " << std::setw(5) << sp->get_depth()
	   << " (" << std::setprecision(2) << sp->get_dz()
	   << ")" << std::endl;  


  uc_dim_info << "Unitcell [001]" << std::endl;
  uc_dim_info << "X: " << sp->get_a()[0] << std::endl;
  uc_dim_info << "Y: " << sp->get_a()[1] << std::endl;
  uc_dim_info << "Z: " << sp->get_a()[2] << std::endl;
  

  orient_info << "Orientation     " << std::endl;
  orient_info << "Phi=" << std::setw(5) << sp->get_phi() << std::endl;
  orient_info << "Theta=" << std::setw(5) << sp->get_theta() << std::endl;
  
  status_bar_uco->setText( QString( uc_orient_info.str().c_str() ) );
  status_bar_uc->setText( QString( uc_dim_info.str().c_str() ) );
  status_bar_or->setText( QString( orient_info.str().c_str() ) );
  status_bar_pixs->setText( QString( pix_info.str().c_str() ) );

}



void GUI::update_measurements_structure(){

  std::vector<QTableWidget*> displays = { measurements_slice->object(), measurements_uc->object() };

  // try to guess the number of channels
  auto filled = sp->get_channel_fill();
  int nr_channels=1;
  for( unsigned int ii=0; ii<filled.size(); ii++ ){
    if( ii>0 ){
      if( filled[ii-1] != filled[ii] ){
	nr_channels++;
      }
    }
  }

  
  for( auto display : displays ){
  
    // reset table view
    display->setRowCount( 0 );
    
    QStringList vertical_labels;

    std::vector<double> mems = sp->get_membranes();
    // create table
    int mem_count=0, ch_count=0;
    for( unsigned int ii=0; ii<nr_channels; ii++){
      display->insertRow( ii );
      if( ii % 2 == 0 ){
	vertical_labels << QString("Channel " ) + def_locale->toString( (ii+2)/2 );
      } else {
	vertical_labels << QString("Membrane " ) + def_locale->toString( (ii+1)/2 );
      }	
    }

    for( int r=0; r<display->rowCount(); r++){
      for( int c=0; c<display->columnCount(); c++){
	display->setItem( r, c, new QTableWidgetItem() );
	display->item( r, c )->setFlags( Qt::ItemIsEnabled );
      }
    }
  
    display->setVerticalHeaderLabels( vertical_labels );

  }

}

void GUI::update_measurements_values( QString what ){ 

  QTableWidget *display;
  std::vector<QLabel*> *pore_display;
  if( measurement_object->currentIndex() == 0 ){
    display = measurements_uc->object();
    pore_display = &measurements_pixinfo_uc;
  } else {
    display = measurements_slice->object();
    pore_display = &measurements_pixinfo_slice;
  }
    

  if( what == "Volumes" ){
    std::vector<double> vol_data = sp_stats->get_channel_volumes();
    for( unsigned int ii=0; ii<vol_data.size(); ii++ ){
      display->item( ii, 0)->setText( def_locale->toString( vol_data[ii] ) );
    }

    std::stringstream pore_content;
    pore_content << "(" << std::setprecision(3) << sp_stats->get_dx()
		 << "," << std::setprecision(3) << sp_stats->get_dy()
		 << "," << std::setprecision(3) << sp_stats->get_dz() << ")";
    (*pore_display)[1]->setText( pore_content.str().c_str() );

  } else if ( what == "Areas" ){
    std::vector<double> area_data = sp_stats->get_membrane_surface_area();
    for( unsigned int ii=0; ii<area_data.size(); ii++ ){    
      display->item( (ii*2)+1, 1 )->setText( def_locale->toString( area_data[ii] ) );
    }

    std::stringstream pore_content;
    pore_content << "(" << std::setprecision(3) << sp_stats->get_dx()
		 << "," << std::setprecision(3) << sp_stats->get_dy()
		 << "," << std::setprecision(3) << sp_stats->get_dz() << ")";    
    (*pore_display)[2]->setText( pore_content.str().c_str() );

  } else if ( what == "Percolation_P" || what == "Percolation_A" ){
    std::vector<double> perc_thres = sp_stats->get_percolation_thresholds();
    std::vector<double> max_pore_rad = sp_stats->get_max_pore_radius();
    for( unsigned int ii=0; ii<perc_thres.size(); ii++){
      display->item( 2*ii, 2)->setText( def_locale->toString( perc_thres[ii] ) );
      display->item( 2*ii, 3)->setText( def_locale->toString( 2*max_pore_rad[ii] ) );      
    }

    std::stringstream pore_content;
    pore_content << "(" << std::setprecision(3) << sp_stats->get_dx()
		 << "," << std::setprecision(3) << sp_stats->get_dy()
		 << "," << std::setprecision(3) << sp_stats->get_dz() << ")";    
    (*pore_display)[3]->setText( pore_content.str().c_str() );
  }

  activate_measurement_buttons();
  
}


/*
 * This is still a bit of a weird one. When directly connecting the sp
 * signal parameter_changed to the compute_projection signal, the gui
 * might get stuck in a loop for whatever reason. When making the
 * detour to the GUI class, and letting the GUI class call for the
 * compute_projection there is no such thing. Don't really know yet why
 */

void GUI::change_autoupdate( int state ){
  
  if( state ){
    //connect( sp, &sp_qt::parameter_changed, sp, &sp_qt::compute_projection );
    connect( sp, &sp_qt::parameter_changed, this, &GUI::request_compute_projection );

    output_message( "Actiavted autoupdate" );
  } else {
    //disconnect( sp, &sp_qt::parameter_changed, sp, &sp_qt::compute_projection );
    disconnect( sp, &sp_qt::parameter_changed, this, &GUI::request_compute_projection );
    output_message( "Deactiavted autoupdate" );    
  }

}


/*
 * This should only send the call_compute_projection signal, if the sp
 * class is not busy. However, as used now for auto update this is not
 * really necessary, since this function is called (if autoupdate is
 * on) by a parameter_changed signal, which can not be emitted, while
 * the sp class is busy. But I leave it in here, just in case.
 */
void GUI::request_compute_projection(){
  
  if( sp_state == 0 ){
    emit call_compute_projection();
  } else {
  }

}


/*
 * Think if we want to make the resolution here a user parameter
 */

void GUI::measure_vol(){

  deactivate_measurement_buttons();

  // this copies all parameters and calls update_geometry, so also all
  // internal should be up-to-date!
  sp_stats->copy_parameters( sp );
  
  if( measurement_object->currentIndex() == 0 ){
    // set to primitive uc
    sp_stats->set_slice_to_primitive_uc();
    // we are changing some parameters, so call update_geometry again
    sp_stats->update_geometry();
  }

  if( gs.res_measure_vol >= 0 ){
    sp_stats->set_n_points_x( gs.res_measure_vol );
    sp_stats->set_n_points_y_to_unitcell();
    sp_stats->set_n_points_z_to_unitcell();
    sp_stats->update_geometry();
  }  

  emit call_update_stats( QString( "Volumes" ) );
}

void GUI::measure_area(){

  deactivate_measurement_buttons();
  
  sp_stats->copy_parameters( sp );

  if( measurement_object->currentIndex() == 0 ){
    // set to primitive uc
    sp_stats->set_slice_to_primitive_uc();
    // we are changing some parameters, so call update_geometry again
    sp_stats->update_geometry();
  }
  
  if( gs.res_measure_area >= 0 ){
    sp_stats->set_n_points_x( gs.res_measure_area );
    sp_stats->set_n_points_y_to_unitcell();
    sp_stats->set_n_points_z_to_unitcell();    
  } 
      
#ifndef USE_CGAL
  auto reply = QMessageBox::warning( this, "Area mesaurement", "You are not using CGAL to "
				     "compute membrane surface area. Please note that the "
				     "implementation used instead is only a crude "
				     "approximation and may vary significantly from "
				     "the real values!" );
#endif
  
  emit call_update_stats( QString( "Areas" ) );  
}



void GUI::measure_network(){
  sp_stats->copy_parameters( sp );

  sp_stats->set_n_points_x( 76 );
  sp_stats->set_n_points_y_to_unitcell();
  sp_stats->set_n_points_z_to_unitcell();

  
  emit call_update_stats( QString("Networks" ));
}


void GUI::measure_percolation(){

  deactivate_measurement_buttons();
  
  sp_stats->copy_parameters( sp );
    
  if( measurement_object->currentIndex() == 0 ){
    // set to primitive uc
    sp_stats->set_slice_to_primitive_uc();
    // we are changing some parameters, so call update_geometry again
    sp_stats->update_geometry();
  }
  
  if( gs.res_measure_perc >= 0 ){
    sp_stats->set_n_points_x( gs.res_measure_perc );
    sp_stats->set_n_points_y_to_unitcell();
    sp_stats->set_n_points_z_to_unitcell();
  }

  if( measurement_object->currentIndex() == 0 ){
    emit call_update_stats( QString("Percolation_P" ));
  } else {
    emit call_update_stats( QString("Percolation_A" ));
  }
}




void GUI::choose_export_prefix(){
  QString prefix = QFileDialog::getSaveFileName( this, "Select Prefix", "./", tr("*"), NULL, QFileDialog::DontUseNativeDialog );

  path_prefix_control->object()->setText( prefix );
}


std::string GUI::get_prefix(){
  
  std::string tmp = path_prefix_control->object()->text().toStdString();
  if( tmp == "" ){
    tmp += "structure";
  }
  return tmp;

}

void GUI::save_grid(){

  std::string fn = get_prefix();
  fn += "_grid.dat";
  emit call_save_grid( QString( fn.c_str() ) );
  
}

void GUI::save_network(){

  std::string fn = get_prefix();
  fn += "_network_inner.dat";
  emit call_save_network( 0, QString( fn.c_str() ) );

  fn = get_prefix();
  fn += "_network_outer.dat";
  emit call_save_network( 1, QString( fn.c_str() ) );
  
}

void GUI::save_surface_points(){

  int nr_membranes = membranes_control->rowCount();

  for( unsigned int i=0; i < nr_membranes; i++){
    std::string fn = get_prefix();
    fn+= "_membrane_" + std::to_string( i ) + ".dat";
    //remember that channel counts are started at 1 not 0
    emit call_save_surface( i+1, QString( fn.c_str() ) );
  }

  
}


void GUI::update_unitcell_size(){
  emit call_change_uc_size( uc_size_control_a->object()->value(), uc_size_control_c->object()->value() );
}

/**
 * switches the state indicator of the GUI
 *
 *  \param[in] what 0: projection, 1: measurement
 * \param[in] state 0: ready, 1: busy
 */ 
void GUI::set_state( int what, int state ){

  std::stringstream text;
  text.str("");
  QLabel *display;
  int *stat_int;  
  
  if( what == 0 ){
    text << "Projection" << std::endl << std::endl;
    display = status_bar_status_p;
    stat_int = &sp_state;
  } else if ( what == 1 ){
    text << "Measurement:" << std::endl << std::endl;
    display = status_bar_status_m;
    stat_int = &sp_stat_state;
  } 


  std::string style;
  
  if( state == 0 ){    
    text << "Ready";
    style = "QLabel { background-color : green; color : black; }";
    *stat_int = 0;
  } else if ( state == 1){
    text << "Busy";
    style = "QLabel { background-color : red; color : black; }";
    *stat_int = 1;
  }
  display->setStyleSheet( QString( style.c_str() ) );
  display->setText( text.str().c_str() );

}


/**
 * Changes the background color of the measurement labels, depending
 * if they are up to date
 *
 * \param[in] state 0: up to date, 1: outdated
 */
void GUI::set_measurements_status( int state ){

  std::string style;
  
  if( state == 0 ){
    style = "QLabel { background-color : red; color : black; }";
  } else if ( state == 1 ){
    style = "QLabel { background-color : green; color : black; }";
  }
  
  status_bar_uco->setStyleSheet( QString( style.c_str() ) );
  status_bar_or->setStyleSheet( QString( style.c_str() ) );
  status_bar_pixs->setStyleSheet( QString( style.c_str() ) );
  status_bar_uc->setStyleSheet( QString( style.c_str() ) );

}

/*
 * This functions gets the number of channels from the sp computation
 * class and adds a checkbox for each channel to the scroll area
 */
void GUI::update_fill_channels(){

  //get filled channels
  std::vector<int> ch_fl = sp->get_channel_fill();
  
  //
  //clear ui, by deleting all old elements and recreating them
  //

  // delete them
  for( unsigned int ii=0; ii<fill_channels.size(); ii++){
    delete fill_channels[ii];
    }
  fill_channels.clear();  
  delete fill_channels_container_layout;
  delete fill_channels_control_content;
  
  
  // recreate them

  fill_channels_control_content = new QWidget( parameters_widget );
  fill_channels_container_layout = new QVBoxLayout( fill_channels_control_content );
  // assign the new QWidget
  fill_channels_control_container->setWidget( fill_channels_control_content );
  
  
  // create Checkboxes for each channel
  int mem_cnt=1, ch_cnt=1;
  for( unsigned int ii=0; ii<ch_fl.size(); ii++){
    std::string lab;
    if( ii % 2 == 0 ){
      lab = "Channel " + std::to_string(ch_cnt);
      ch_cnt++;
    } else {
      lab = "Membrane " + std::to_string(mem_cnt);
      mem_cnt++;
    }
    
    fill_channels.push_back( new QCheckBox( lab.c_str(), fill_channels_control_content ) );
    fill_channels_container_layout->addWidget( fill_channels[ii] );
    
    connect( fill_channels[ii], SIGNAL( stateChanged(int) ), this, SLOT( check_channel_color() ) );
    
    if( ch_fl[ii] == 1 ){
      fill_channels[ii]->setCheckState( Qt::Checked );
    }
    
  }
    
  fill_channels_container_layout->setSizeConstraint(QLayout::SetMinimumSize);
  
}

void GUI::check_channel_color(){

  //get filled channels
  std::vector<int> ch_fl = sp->get_channel_fill();
  
  for( unsigned int ii=0; ii<fill_channels.size(); ii++){
    if( ch_fl[ii] != fill_channels[ii]->isChecked() ){
      emit call_change_channel_color( ii+1, fill_channels[ii]->isChecked() );
    } 
  }  
}


/*
 * changes if the surface parameter level controls the volume
 * proportion or level set
 */
void GUI::change_surface_par_type( int index ){

  if( index == 0 ){
    //volume proportion

    // channel proportion only allows values between 0 and 1
    channel_prop_control->object()->setRange(0, 1);

    // set the value to the channel proportion
    channel_prop_control->object()->setValue( sp->get_channel_vol_prop() );
  } else if ( index == 1 ){
    // level set


    // level set values can be pretty much everything
    channel_prop_control->object()->setRange(-10, 10);
    
    channel_prop_control->object()->setValue( sp->get_surface_level() );
  }

  update_batch_parameters_gui();
  
}

void GUI::write_parameters(){

  QString filename = QFileDialog::getSaveFileName( this, "Select File", "./", tr("*"), NULL, QFileDialog::DontUseNativeDialog );

  if( !filename.isNull() ){

    sp->write_parameters( filename.toStdString() );
    
  }
  
}

void GUI::read_parameters(){

  QString filename = QFileDialog::getOpenFileName( this, "Select File", "./", tr("*"), NULL, QFileDialog::DontUseNativeDialog );

  if( !filename.isNull() ){

    emit call_read_pars_from_file( filename );
    
    //sp->read_parameters( filename.toStdString() );

    //update_gui_from_sp();
    //emit call_compute_projection();    
  }
  
}




void GUI::set_batch_output(){

  QString filename = QFileDialog::getSaveFileName( this, "Select File", "./", tr("*"), NULL, QFileDialog::DontUseNativeDialog );

  batch_output_name->object()->setText( filename );
  
}



bool GUI::check_batch_string_consistency( QString inp ){

  QString regex_csl_identifier ( "^([+-]?([0-9]+[.])?[0-9]+)+(,([+-]?([0-9]+[.])?[0-9]+))*$" );
  QString regex_range_identifier ( "^(([+-]?([0-9]+[.])?[0-9]+):([+-]?([0-9]+[.])?[0-9]+):([+-]?([0-9]+[.])?[0-9]+))?$" );
  QRegularExpression re_list( regex_csl_identifier );
  QRegularExpression re_range( regex_range_identifier  );
  
  bool is_list = re_list.match( inp ).hasMatch();
  bool is_range = re_range.match( inp ).hasMatch();    
  
  return is_list || is_range;
}

/*
 * checks for consistency of the string by checking if each comma
 * separated value is found as structure type. If so, converts the
 * string to the appropriate index in the array. If any substring is
 * not found returns an empty list
 */
std::vector<double> GUI::check_struct_type_string_consistency( QString inp ){


  std::vector<double> ret;
  bool valid = true;
    
  std::vector<std::string> as = sp->get_surface_choices();     
  QStringList split = inp.split ( "," );
  
  for( auto &it : split ){
    auto surface_found = std::find( as.begin(), as.end(), it.toStdString() );
    if( surface_found != as.end() ){
      ret.push_back( std::distance( as.begin(), surface_found ) );
    } else {
      valid = false;
    }
  }
  

  // if any entry is invalid return an empty list
  if( valid ){
    return ret;
  } else {
    return std::vector<double> (0, 0);
  }
}


void GUI::start_batch_computing(){

  cb->update_status( true );
  bc->reset_parameters();

  bool all_valid = true;
  std::vector<double> prov_surface_types;
  
  for( size_t ii=0; ii<batch_values.size(); ii++ ){
    
    // reset color
    batch_values[ii]->object()->setStyleSheet("QLineEdit { background: rgb(255, 255, 255); }");

    // check all parameter inputs

    // handle struct type differentyl since we allow strings here
    if( batch_values[ii]->label()->text().toStdString() == surface_projection::parameter_names_hr[0] ){

      // check string for consistency
      prov_surface_types = check_struct_type_string_consistency( batch_values[ii]->object()->text() );

      //if list is empty, string is not consistent
      if( batch_values[ii]->object()->text() != "" && prov_surface_types.size() == 0 ){
	all_valid = all_valid && false;
	batch_values[ii]->object()->setStyleSheet("QLineEdit { background: rgb(255, 0, 0); }");
      }
      
    } else {
    
      bool this_valid = check_batch_string_consistency( batch_values[ii]->object()->text() );
      all_valid = all_valid && this_valid;
      if( !this_valid ){
	batch_values[ii]->object()->setStyleSheet("QLineEdit { background: rgb(255, 0, 0); }");
      }

    }
  }

  // stop if there are invalid expressions
  if( !all_valid ){
    QMessageBox::QMessageBox::critical( this, "Invalid parameters", "Some parameter inputs are "
					"invalid\nPlease provide a comma separated list or a range!");
    return;
  }

  batch_compute_start->setEnabled( false );
  
  // set all the parameters
  for( size_t ii=0; ii<batch_values.size(); ii++ ){

    // get the index of the current value in the parameter_names array
    size_t ind = std::distance( surface_projection::parameter_names_hr.begin(),
				std::find( surface_projection::parameter_names_hr.begin(),
					   surface_projection::parameter_names_hr.end(),
					   batch_values[ii]->label()->text().toStdString() ) );

    std::string par_name = surface_projection::parameter_names[ind];
    
    if( batch_values[ii]->object()->text() == "" ){
      // this parameter is not changed and should be set correctly already
    } else {

      if( par_name  == surface_projection::parameter_names[0] ){
	bc->add_parameter( par_name, prov_surface_types );
      } else {
	bc->add_parameter( par_name, batch_values[ii]->object()->text().toStdString() );
      }
    }

  }

  // pass the new path
  if( !(batch_output_name->object()->text() == "") ){
    cb->set_prefix ( batch_output_name->object()->text().toStdString() );
  }
  // open summary file
  cb->init();
  cb->set_img_mode ( invert_control->object()->isChecked(),
		     image_scaling_control->object()->currentText().toStdString() );
  
  batch_progress->setMaximum( bc->total_combinations() );
  
  // start the loop
  emit emit_start_batch();
  
}


void GUI::stop_batch_computing(){

  batch_progress->setValue( 0 );
  emit emit_stop_batch( false );
  
}

/**
 * function is called when batch loop is finished succesfully
 */
void GUI::finalize_batch_loop(){
  // close summary file
  cb->finalize();
  
  batch_progress->setMaximum(1);
  batch_progress->setValue( 1 );
  batch_compute_start->setEnabled( true );  
}



void GUI::deactivate_measurement_buttons(){
  button_measure_vol->setEnabled( false );
  button_measure_area->setEnabled( false );
  button_measure_percthres->setEnabled( false );
}


void GUI::activate_measurement_buttons(){
  button_measure_vol->setEnabled( true );
  button_measure_area->setEnabled( true );
  button_measure_percthres->setEnabled( true );
}


void GUI::deactivate_pp_controls(){
  gaussian_noise_magnitude->object()->setEnabled( false );
  gaussian_blur_kernelsize->object()->setEnabled( false );
  grains_gsize_c->object()->setEnabled( false );
  grains_gsize_s->object()->setEnabled( false );
  grains_gnumber_c->object()->setEnabled( false );
  grains_gnumber_s->object()->setEnabled( false );
  grains_magnitude->object()->setEnabled( false );
  gaussian_noise_activate->setEnabled( false );
  gaussian_blur_activate->setEnabled( false );
  grains_activate->setEnabled( false );
}


void GUI::activate_pp_controls(){

  gaussian_noise_magnitude->object()->setEnabled( true );
  gaussian_blur_kernelsize->object()->setEnabled( true );
  grains_gsize_c->object()->setEnabled( true );
  grains_gsize_s->object()->setEnabled( true );
  grains_gnumber_c->object()->setEnabled( true );
  grains_gnumber_s->object()->setEnabled( true );
  grains_magnitude->object()->setEnabled( true );
  gaussian_noise_activate->setEnabled( true );
  gaussian_blur_activate->setEnabled( true );
  grains_activate->setEnabled( true );
  
}


/*
 * Since QAbstractSpinbox::finishedEditing is a slot without a
 * parameter, we have to catch that signal in this slot to read the
 * value from the object and pass it onto another slot in the sp class
 * to actually change the value. Using the valueChanged signal would
 * avoid this issue, however causes annoying behavior where the focus
 * is lost, when updating the GUI after each digit change. Somehow
 * this does not affect the (integer) QSpinBox objects. Maybe I'll
 * implement this in a cleaner fashion in the future
 */
void GUI::set_parameter(){

  // get the sender
  QObject* sender = QObject::sender();

  if( sender == uc_size_control_a->object() ){
    // connect signal to right slot
    connect( this, &GUI::request_parameter_change, sp, &sp_qt::change_uc_scale_ab );
    emit request_parameter_change( uc_size_control_a->object()->value() );
    disconnect( this, &GUI::request_parameter_change, sp, &sp_qt::change_uc_scale_ab );
    
  } else if ( sender == uc_size_control_c->object() ){
    // connect signal to right slot
    connect( this, &GUI::request_parameter_change, sp, &sp_qt::change_uc_scale_c );
    emit request_parameter_change( uc_size_control_c->object()->value() );
    disconnect( this, &GUI::request_parameter_change, sp, &sp_qt::change_uc_scale_c );

  } else if ( sender == channel_prop_control->object() ){

    if( level_par_type->object()->currentIndex() == 0 ){ // channel proportion given
      connect( this, &GUI::request_parameter_change, sp, &sp_qt::change_vol_prop );
      emit request_parameter_change( channel_prop_control->object()->value() );
      disconnect( this, &GUI::request_parameter_change, sp, &sp_qt::change_vol_prop );      
    } else { // level set given
      connect( this, &GUI::request_parameter_change, sp, &sp_qt::change_lvl_set );
      emit request_parameter_change( channel_prop_control->object()->value() );
      disconnect( this, &GUI::request_parameter_change, sp, &sp_qt::change_lvl_set ); 
    }

  } else if ( sender == z_points_control->object() ){
    // connect signal to right slot
    connect( this, &GUI::request_parameter_change, sp, &sp_qt::change_z_points );
    emit request_parameter_change( z_points_control->object()->value() );
    disconnect( this, &GUI::request_parameter_change, sp, &sp_qt::change_z_points );
    
  } else if ( sender == x_points_control->object() ){
    // connect signal to right slot
    connect( this, &GUI::request_parameter_change, sp, &sp_qt::change_xy_points );
    emit request_parameter_change( x_points_control->object()->value() );
    disconnect( this, &GUI::request_parameter_change, sp, &sp_qt::change_xy_points );
    
  } else if ( sender == slice_thickness_control->object() ){
    // connect signal to right slot
    connect( this, &GUI::request_parameter_change, sp, &sp_qt::change_slice_thickness );
    emit request_parameter_change( slice_thickness_control->object()->value() );
    disconnect( this, &GUI::request_parameter_change, sp, &sp_qt::change_slice_thickness );
    
  } else if ( sender == slice_height_control->object() ){
    // connect signal to right slot
    connect( this, &GUI::request_parameter_change, sp, &sp_qt::change_slice_height );
    emit request_parameter_change( slice_height_control->object()->value() );
    disconnect( this, &GUI::request_parameter_change, sp, &sp_qt::change_slice_height );
    
  } else if ( sender == slice_width_control->object() ){
    // connect signal to right slot
    connect( this, &GUI::request_parameter_change, sp, &sp_qt::change_slice_width );
    emit request_parameter_change( slice_width_control->object()->value() );
    disconnect( this, &GUI::request_parameter_change, sp, &sp_qt::change_slice_width );
    
  } else if ( sender == slice_position_control->object() ){
    // connect signal to right slot
    connect( this, &GUI::request_parameter_change, sp, &sp_qt::change_slice_position );
    emit request_parameter_change( slice_position_control->object()->value() );
    disconnect( this, &GUI::request_parameter_change, sp, &sp_qt::change_slice_position );
    
  } 						
  
}



void GUI::do_something(){  
  //std::cout << "done something\n"; 

  if( sp_state == 0 ){
    std::cout << "ready" << std::endl;
    //emit call_compute_projection();
  } else {
    std::cout << "too busy" << std::endl;
  }

  
}

