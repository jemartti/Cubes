#include "viewer.hpp"
#include "appwindow.hpp"
#include "draw.hpp"

#include <GL/gl.h>
#include <GL/glu.h>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>


Viewer::Viewer()
{
	Glib::RefPtr<Gdk::GL::Config> glconfig;

	// Ask for an OpenGL Setup with
	//  - red, green and blue component colour
	//  - a depth buffer to avoid things overlapping wrongly
	//  - double-buffered rendering to avoid tearing/flickering
	glconfig = Gdk::GL::Config::create( Gdk::GL::MODE_RGB   |
										Gdk::GL::MODE_DEPTH |
										Gdk::GL::MODE_DOUBLE );
	if (glconfig == 0)
	{
		// If we can't get this configuration, die
		std::cerr << "Unable to setup OpenGL Configuration!" << std::endl;
		abort();
	}

	// Accept the configuration
	set_gl_capability(glconfig);

	// Register the fact that we want to receive these events
	add_events( Gdk::BUTTON1_MOTION_MASK    |
				Gdk::BUTTON2_MOTION_MASK    |
				Gdk::BUTTON3_MOTION_MASK    |
				Gdk::BUTTON_PRESS_MASK      |
				Gdk::BUTTON_RELEASE_MASK    |
				Gdk::BUTTON_MOTION_MASK		|
				Gdk::POINTER_MOTION_MASK	|
				Gdk::VISIBILITY_NOTIFY_MASK );

	m_initflag = true;
	reset();
}

Viewer::~Viewer()
{
}

void Viewer::set_mode( Mode mode )
{
	m_mode = mode;
	update_mode( mode );
}

void Viewer::set_window( AppWindow* window )
{
	m_window = window;
}

void Viewer::invalidate()
{
	// Force a rerender
	Gtk::Allocation allocation = get_allocation();
	get_window()->invalidate_rect( allocation, false );
}

void Viewer::set_perspective( double fov,  double aspect,
                              double near, double far )
{
	Vector4D row1, row2, row3, row4;
	double   rfov, x1, x2;

	// Error check
	if      ( fov < 5   ) fov = 5;
	else if ( fov > 160 ) fov = 160;

	// Calculate the fov in degrees related to the viewport
	rfov = 1 / tan( fov * PI / 360 );
	x1   = (      far + near ) / ( far - near );
	x2   = ( -2 * far * near ) / ( far - near );

	// Implement perspective matrix as described in course notes
	row1         = Vector4D( rfov / aspect, 0,    0,  0  );
	row2         = Vector4D( 0,             rfov, 0,  0  );
	row3         = Vector4D( 0,             0,    x1, x2 );
	row4         = Vector4D( 0,             0,    1,  0  );
	m_projection = Matrix4x4( row1, row2, row3, row4 );
}

void Viewer::reset_view()
{
	reset();
}

void Viewer::set_infobar( Gtk::Label* infobar )
{
	m_infobar = infobar;
}

void Viewer::on_realize()
{
	// Do some OpenGL setup.
	// First, let the base class do whatever it needs to
	Gtk::GL::DrawingArea::on_realize();

	Glib::RefPtr<Gdk::GL::Drawable> gldrawable = get_gl_drawable();

	if ( !gldrawable )
	{
		return;
	}

	if ( !gldrawable->gl_begin(get_gl_context()) )
	{
		return;
	}

	gldrawable->gl_end();
}

bool Viewer::on_expose_event( GdkEventExpose* /*event*/ )
{
	Glib::RefPtr<Gdk::GL::Drawable> gldrawable = get_gl_drawable();

	if ( !gldrawable )
	{
		return false;
	}

	if ( !gldrawable->gl_begin(get_gl_context()) )
	{
		return false;
	}

	// Start drawing
	draw_init( get_width(), get_height() );

	// Transform the world gnomon
	for( int i = 0; i < 4; i += 1 )
	{
		m_gnomonTrans[i] = m_viewing * m_gnomon[i];
	}
	// Draw the world gnomon
	set_colour( Colour(0.1, 0.1, 1.0) );
	draw_line2D( m_gnomonTrans[0], m_gnomonTrans[1] );
	draw_line2D( m_gnomonTrans[0], m_gnomonTrans[2] );
	draw_line2D( m_gnomonTrans[0], m_gnomonTrans[3] );

	// Draw the modelling gnomon
	set_colour( Colour(0.1, 1.0, 0.1) );
	draw_modellingGnomon();

	// Draw the unit cube
	set_colour( Colour(0.1, 0.1, 0.1) );
	draw_unitCube();

	// Initialize the viewport
	if ( !m_viewflag )
	{
		m_viewport[0] = ( Point2D(get_width() * 0.05, get_height() * 0.05) );
		m_viewport[1] = ( Point2D(get_width() * 0.95, get_height() * 0.05) );
		m_viewport[2] = ( Point2D(get_width() * 0.95, get_height() * 0.95) );
		m_viewport[3] = ( Point2D(get_width() * 0.05, get_height() * 0.95) );
		m_viewflag    = true;
	}
	// Draw the viewport
	set_colour( Colour(0.1, 0.1, 0.1) );
	draw_line( m_viewport[0], m_viewport[1] );
	draw_line( m_viewport[1], m_viewport[2] );
	draw_line( m_viewport[2], m_viewport[3] );
	draw_line( m_viewport[3], m_viewport[0] );

	// Finish drawing
	draw_complete();

	// Update the information bar
	update_infobar();

	// Swap the contents of the front and back buffers so we see what we
	// just drew. This should only be done if double buffering is enabled.
	gldrawable->swap_buffers();

	gldrawable->gl_end();

	return true;
}

bool Viewer::on_configure_event( GdkEventConfigure* /*event*/ )
{
	Glib::RefPtr<Gdk::GL::Drawable> gldrawable = get_gl_drawable();

	if ( !gldrawable )
	{
		return false;
	}

	if ( !gldrawable->gl_begin(get_gl_context()) )
	{
		return false;
	}

	gldrawable->gl_end();

	return true;
}

bool Viewer::on_button_press_event( GdkEventButton* event )
{
	// Which button(s) pressed?
	switch (event->button)
	{
	case 1:
		m_button1 = true;
		break;
	case 2:
		m_button2 = true;
		break;
	case 3:
		m_button3 = true;
		break;
	default:
		break;
	}

	// Capture mouse position information
	m_ixpos = event->x;
	m_xpos  = event->x;
	m_txpos = event->x;
	if ( m_mode == VIEWPORT )
	{
		m_iypos = event->y;
		m_ypos  = event->y;
	}

	invalidate();
	return true;
}

bool Viewer::on_button_release_event( GdkEventButton* event )
{
	if ( m_mode == VIEWPORT && m_button1 )
	{
		m_xpos = event->x;
		m_ypos = event->y;

		// Error check
		if ( m_ixpos < 0 )           m_ixpos = 0;
		if ( m_ixpos > get_width() ) m_ixpos = get_width();
		if ( m_xpos  < 0 )           m_xpos  = 0;
		if ( m_xpos  > get_width() ) m_xpos  = get_width();
		if ( m_iypos < 0 )           m_iypos = 0;
		if ( m_iypos > get_width() ) m_iypos = get_width();
		if ( m_ypos  < 0 )           m_ypos  = 0;
		if ( m_ypos  > get_width() ) m_ypos  = get_width();

		// Process
		double x1, x2, y1, y2;
		x1 = std::min( m_ixpos, m_xpos );
		x2 = std::max( m_ixpos, m_xpos );
		y1 = std::min( m_iypos, m_ypos );
		y2 = std::max( m_iypos, m_ypos );

		// Update viewport
		m_viewport[0] = ( Point2D(x1, y1) );
		m_viewport[1] = ( Point2D(x2, y1) );
		m_viewport[2] = ( Point2D(x2, y2) );
		m_viewport[3] = ( Point2D(x1, y2) );

		invalidate();
	}

	// Which button(s) released?
	switch (event->button)
	{
	case 1:
		m_button1 = false;
		break;
	case 2:
		m_button2 = false;
		break;
	case 3:
		m_button3 = false;
		break;
	default:
		break;
	}

	return true;
}

bool Viewer::on_motion_notify_event( GdkEventMotion* event )
{
	if ( m_button1 || m_button2 || m_button3 )
	{
		m_txpos = m_xpos;
		m_xpos  = event->x;

		switch ( m_mode )
		{
		case VIEWROTATE:
			if ( m_button1 )
			{
				m_viewing = m_viewing *
						rotation( (m_txpos - m_xpos) / 100.0, 'y' ).invert();
			}
			if ( m_button2 )
			{
				m_viewing = m_viewing *
						rotation( (m_txpos - m_xpos) / 100.0, 'z' ).invert();
			}
			if ( m_button3 )
			{
				m_viewing = m_viewing *
						rotation( (m_txpos - m_xpos) / 100.0, 'x' ).invert();
			}
			break;
		case VIEWTRANSLATE:
			if ( m_button1 )
			{
				m_viewing = translation( Vector3D(( m_txpos - m_xpos ) /
						100.0, 0.0, 0.0) ).invert() * m_viewing;
			}
			if ( m_button2 )
			{
				m_viewing = translation( Vector3D(0.0, ( m_txpos - m_xpos ) /
						100.0, 0.0) ).invert() * m_viewing;
			}
			if ( m_button3 )
			{
				m_viewing = translation( Vector3D(0.0, 0.0,
						( m_txpos - m_xpos ) / 100.0) ).invert() * m_viewing;
			}
			break;
		case VIEWPERSPECTIVE:
			if ( m_button1 )
			{
				m_fov  -= ( m_txpos - m_xpos ) / 10.0;
				set_perspective( m_fov, 1, m_near, m_far );
			}
			if ( m_button2 )
			{
				m_near -= ( m_txpos - m_xpos ) / 10.0;
			}
			if ( m_button3 )
			{
				m_far  -= ( m_txpos - m_xpos ) / 10.0;
			}
			break;
		case MODELROTATE:
			if ( m_button1 )
			{
				m_modelling = m_modelling *
						rotation( (m_txpos - m_xpos) / 100.0, 'y' ).invert();
			}
			if ( m_button2 )
			{
				m_modelling = m_modelling *
						rotation( (m_txpos - m_xpos) / 100.0, 'z' ).invert();
			}
			if ( m_button3 )
			{
				m_modelling = m_modelling *
						rotation( (m_txpos - m_xpos) / 100.0, 'x' ).invert();
			}
			break;
		case MODELTRANSLATE:
			if ( m_button1 )
			{
				m_modelling = m_modelling * translation( Vector3D
						(( m_txpos - m_xpos ) / -100.0, 0.0, 0.0) );
			}
			if ( m_button2 )
			{
				m_modelling = m_modelling * translation( Vector3D
						(0.0, ( m_txpos - m_xpos ) / -100.0, 0.0) );
			}
			if ( m_button3 )
			{
				m_modelling = m_modelling * translation( Vector3D
						(0.0, 0.0, ( m_txpos - m_xpos ) / -100.0) );
			}
			break;
		case MODELSCALE:
			if ( m_button1 )
			{
				m_scaling = m_scaling * scaling( Vector3D(1.0 +
						( (m_txpos - m_xpos) / 100.0 ), 1.0, 1.0) ).invert();
			}
			if ( m_button2 )
			{
				m_scaling = m_scaling * scaling( Vector3D(1.0, 1.0 +
						( m_txpos - m_xpos ) / 100.0, 1.0) ).invert();
			}
			if ( m_button3 )
			{
				m_scaling = m_scaling * scaling( Vector3D(1.0, 1.0, 1.0 +
						( m_txpos - m_xpos ) / 100.0) ).invert();
			}
			break;
		default:
			break;
		}

		invalidate();
	}

	return true;
}

void Viewer::reset()
{
	if ( !m_initflag )
	{
		update_mode( MODELROTATE );
	}

	// Default mouse information
	m_button1 = false;
	m_button2 = false;
	m_button3 = false;
	m_ixpos   = 0.0;
	m_xpos    = 0.0;
	m_iypos   = 0.0;
	m_ypos    = 0.0;
	m_txpos   = 0.0;

	// Initialize viewport
	for ( int i = 0; i < 4; i += 1 )
	{
		m_viewport[i] = ( Point2D() );
	}

	// Default FOV of 30
	m_fov  = 30.0;

	// Set the default far and near plane values
	m_near = 2.0;
	m_far  = 20.0;

	// Initialize all the transformation matrices
	m_projection = Matrix4x4();
	m_modelling  = Matrix4x4();
	m_viewing    = Matrix4x4();
	m_scaling    = Matrix4x4();
	// Start off by pushing the cube back into the screen
	Vector4D row1, row2, row3, row4;
	row1      = Vector4D( 1,   0,  0, 0 );
	row2      = Vector4D( 0,   1,  0, 0 );
	row3      = Vector4D( 0,   0,  1, 8 );
	row4      = Vector4D( 0,   0,  0, 1 );
	m_viewing = Matrix4x4( row1, row2, row3, row4 );

	// Initialize the unit cubes
	m_unitCube[0] = ( Point3D( 1.0, -1.0, -1.0) );
	m_unitCube[1] = ( Point3D(-1.0, -1.0, -1.0) );
	m_unitCube[2] = ( Point3D(-1.0,  1.0, -1.0) );
	m_unitCube[3] = ( Point3D( 1.0,  1.0, -1.0) );
	m_unitCube[4] = ( Point3D(-1.0, -1.0,  1.0) );
	m_unitCube[5] = ( Point3D( 1.0, -1.0,  1.0) );
	m_unitCube[6] = ( Point3D( 1.0,  1.0,  1.0) );
	m_unitCube[7] = ( Point3D(-1.0,  1.0,  1.0) );
	for ( int i = 0; i < 8; i += 1 )
	{
		m_unitCubeTrans[i] = ( Point3D() );
	}

	// Initialize the gnomons
	m_gnomon[0] = ( Point3D(0.0, 0.0, 0.0) );
	m_gnomon[1] = ( Point3D(0.5, 0.0, 0.0) );
	m_gnomon[2] = ( Point3D(0.0, 0.5, 0.0) );
	m_gnomon[3] = ( Point3D(0.0, 0.0, 0.5) );
	for ( int i = 0; i < 4; i += 1 )
	{
		m_gnomonTrans[i] = ( Point3D() );
	}

	m_viewflag = false;
	m_initflag = false;

	// Initialize the perspective
	set_perspective( m_fov, 1, m_near, m_far );
}

void Viewer::draw_unitCube()
{
	// Apply transformations to unit cube
	for( int i = 0; i < 8; i += 1 )
	{
		m_unitCubeTrans[i] = m_viewing   *
							 m_modelling *
							 m_scaling   *
							 m_unitCube[i];
	}

	// Draw front face of cube in white, and the rest in a dark grey
	set_colour( Colour(1, 1, 1) );
	draw_line2D( m_unitCubeTrans[0], m_unitCubeTrans[1] );
	draw_line2D( m_unitCubeTrans[0], m_unitCubeTrans[3] );
	set_colour( Colour(0.1, 0.1, 0.1) );
	draw_line2D( m_unitCubeTrans[0], m_unitCubeTrans[5] );
	set_colour( Colour(1, 1, 1) );
	draw_line2D( m_unitCubeTrans[1], m_unitCubeTrans[2] );
	set_colour( Colour(0.1, 0.1, 0.1) );
	draw_line2D( m_unitCubeTrans[1], m_unitCubeTrans[4] );
	set_colour( Colour(1, 1, 1) );
	draw_line2D( m_unitCubeTrans[2], m_unitCubeTrans[3] );
	set_colour( Colour(0.1, 0.1, 0.1) );
	draw_line2D( m_unitCubeTrans[2], m_unitCubeTrans[7] );
	draw_line2D( m_unitCubeTrans[3], m_unitCubeTrans[6] );
	draw_line2D( m_unitCubeTrans[4], m_unitCubeTrans[5] );
	draw_line2D( m_unitCubeTrans[4], m_unitCubeTrans[7] );
	draw_line2D( m_unitCubeTrans[5], m_unitCubeTrans[6] );
	draw_line2D( m_unitCubeTrans[6], m_unitCubeTrans[7] );
	set_colour( Colour(0.1, 0.1, 0.1) );
}

void Viewer::draw_modellingGnomon()
{
	// Apply transformation to the modelling gnomon
	for( int i = 0; i < 4; i += 1 )
	{
		m_gnomonTrans[i] = m_viewing * m_modelling * m_gnomon[i];
	}

	// Draw the modelling gnomon
	draw_line2D( m_gnomonTrans[0], m_gnomonTrans[1] );
	draw_line2D( m_gnomonTrans[0], m_gnomonTrans[2] );
	draw_line2D( m_gnomonTrans[0], m_gnomonTrans[3] );
}

void Viewer::draw_line2D ( Point3D left, Point3D right )
{
	// Flag set to determine whether we draw this line
	// Gets set to false if it lies outside the clipping area
	bool draw = true;

	// Clip to the near plane using algorithm described in course notes
	double clipNL = ( left  - Point3D(0.0, 0.0, m_near) ).dot(
			Vector3D(0.0, 0.0, 1.0) );
	double clipNR = ( right - Point3D(0.0, 0.0, m_near) ).dot(
			Vector3D(0.0, 0.0, 1.0) );
	if ( clipNL < 0.0 && clipNR < 0.0 )
	{
		draw = false;
	}
	else
	{
		if ( clipNL < 0.0 || clipNR < 0.0 )
		{
			double t = clipNL / ( clipNL - clipNR );
			if ( clipNL < 0.0 )
			{
				left = left + t * ( right - left );
			}
			else
			{
				right = left + t * ( right - left );
			}
		}
	}

	// Clip to the far plane using algorithm described in course notes
	if ( draw )
	{
		double clipFL = ( left  - Point3D(0.0, 0.0, m_far) ).dot(
				Vector3D(0.0, 0.0, -1.0) );
		double clipFR = ( right - Point3D(0.0, 0.0, m_far) ).dot(
				Vector3D(0.0, 0.0, -1.0) );
		if ( clipFL < 0.0 && clipFR < 0.0 )
		{
			draw = false;
		}
		else
		{
			if ( clipFL < 0.0 || clipFR < 0.0 )
			{
				double t = clipFL / ( clipFL - clipFR );
				if ( clipFL < 0.0 )
				{
					left = left + t * ( right - left );
				}
				else
				{
					right = left + t * ( right - left );
				}
			}
		}
	}

	// Now transform and clip to the viewport
	if( draw )
	{
		// First we project, then normalize each of the points
		Point2D nleft  = normalize( project(left)  );
		Point2D nright = normalize( project(right) );

		// Next, we clip to the viewing cube (the viewport) using algorithm
		// described in course notes
		for ( int i = 0; i < 4; i++ )
		{
			Point2D normal;
			double clipVL, clipVR;
			switch( i )
			{
			case 0:
				clipVL = nleft[1]  - m_viewport[0][1];
				clipVR = nright[1] - m_viewport[0][1];
				break;
			case 1:
				clipVL = -1.0 * ( nleft[0]  - m_viewport[1][0] );
				clipVR = -1.0 * ( nright[0] - m_viewport[1][0] );
				break;
			case 2:
				clipVL = -1.0 * ( nleft[1]  - m_viewport[2][1] );
				clipVR = -1.0 * ( nright[1] - m_viewport[2][1] );
				break;
			case 3:
				clipVL = nleft[0]  - m_viewport[3][0];
				clipVR = nright[0] - m_viewport[3][0];
				break;
			default:
				clipVL = -1.0;
				clipVR = -1.0;
				break;
			}

			if ( clipVL < 0.0 && clipVR < 0.0 )
			{
				draw = false;
			}
			else
			{
				if ( clipVL < 0.0 || clipVR < 0.0 )
				{
					double t = clipVL / ( clipVL - clipVR );
					if ( clipVL < 0.0 )
					{
						nleft[0]  = nleft[0] + t * ( nright[0] - nleft[0] );
						nleft[1]  = nleft[1] + t * ( nright[1] - nleft[1] );
					}
					else
					{
						nright[0] = nleft[0] + t * ( nright[0] - nleft[0] );
						nright[1] = nleft[1] + t * ( nright[1] - nleft[1] );
					}
				}
			}
		}

		// Finally, draw the line
		if ( draw )
		{
			draw_line( nleft, nright );
		}
	}
}

Point3D Viewer::project( Point3D point )
{
	// Project the point into the viewing plane using algorithm described in
	// course notes
	Point3D point_p = m_projection * point;

	return Point3D( point_p[0] / point[2],
					point_p[1] / point[2],
			        point_p[2] / point[2] );
}

Point2D Viewer::normalize( Point3D point )
{
	// Normalize the point to the coordinates used in the viewing window using
	// algorithm described in course notes
	double x, y, z;
	x = ( point[0] + 1.0 ) / 2;
	y = ( point[1] + 1.0 ) / 2;
	z =   point[2];

	return Point2D( (point[0] / z + 1.5) *
			        (m_viewport[2][0] - m_viewport[0][0]) /
			         3 + m_viewport[0][0],
					(point[1] / z + 1.5) *
					(m_viewport[2][1] - m_viewport[0][1]) /
					 3 + m_viewport[0][1] );
}

void Viewer::update_mode( Mode mode )
{
	// Update both the mode and the radio button, if required
	if ( m_mode != mode )
	{
		m_mode = mode;
		m_window->update_mode( (int)mode );
	}

	invalidate();
}

void Viewer::update_infobar( )
{
	std::stringstream infoss;
	infoss << "Mode: ";

	switch( m_mode )
	{
	case VIEWROTATE:
		infoss << "View Rotate";
		break;
	case VIEWTRANSLATE:
		infoss << "View Translate";
		break;
	case VIEWPERSPECTIVE:
		infoss << "View Perspective";
		break;
	case MODELROTATE:
		infoss << "Model Rotate";
		break;
	case MODELTRANSLATE:
		infoss << "Model Translate";
		break;
	case MODELSCALE:
		infoss << "Model Scale";
		break;
	case VIEWPORT:
		infoss << "Viewport";
		break;
	default:
		infoss << "";
		break;
	}

	infoss << ", Near: " << m_near;
	infoss << ", Far: "  << m_far;
	infoss << std::endl;

	m_infobar->set_label( infoss.str() );
}
