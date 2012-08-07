#ifndef CS488_VIEWER_HPP
#define CS488_VIEWER_HPP

#include <gtkmm.h>
#include <gtkglmm.h>
#include <math.h>
#include <vector>
#include "algebra.hpp"
#include "a2.hpp"

// Define a default value for Pi
#define PI 4*atan(1)


class AppWindow;

// The "main" OpenGL widget
class Viewer : public Gtk::GL::DrawingArea {
public:
	// Mode enum
	enum Mode {
		VIEWROTATE,
		VIEWTRANSLATE,
		VIEWPERSPECTIVE,
		MODELROTATE,
		MODELTRANSLATE,
		MODELSCALE,
		VIEWPORT
	};

	Viewer();
	virtual ~Viewer();

	// Setter functions
	void set_mode   ( Mode        mode    );
	void set_window ( AppWindow*  window  );
	void set_infobar( Gtk::Label* infobar );

	// A useful function that forces this widget to rerender. If you
	// want to render a new frame, do not call on_expose_event
	// directly. Instead call this, which will cause an on_expose_event
	// call when the time is right.
	void invalidate();

	// Set the parameters of the current perspective projection using
	// the semantics of gluPerspective().
	void set_perspective( double fov, double aspect,
					      double near, double far );

	// Restore all the transforms and perspective parameters to their
	// original state. Set the viewport to its initial size.
	void reset_view();

protected:
	// Events we implement
	// Note that we could use gtkmm's "signals and slots" mechanism
	// instead, but for many classes there's a convenient member
	// function one just needs to define that'll be called with the
	// event.

	// Called when GL is first initialized
	virtual void on_realize             ();
	// Called when our window needs to be redrawn
	virtual bool on_expose_event        ( GdkEventExpose*    event );
	// Called when the window is resized
	virtual bool on_configure_event     ( GdkEventConfigure* event );
	// Called when a mouse button is pressed
	virtual bool on_button_press_event  ( GdkEventButton*    event );
	// Called when a mouse button is released
	virtual bool on_button_release_event( GdkEventButton*    event );
	// Called when the mouse moves
	virtual bool on_motion_notify_event ( GdkEventMotion*    event );

private:
	// Set/reset the application state
	void    reset               ();

	// Used to draw the unit cube
	void    draw_unitCube       ();

	// Used to draw the modelling gnomon
	void    draw_modellingGnomon();

	// Used to draw a 3D line in the 2D window
	void    draw_line2D         ( Point3D left, Point3D right );

	// Applies projective transform to a point
	Point3D project             ( Point3D point               );

	// Normalizes a point to the viewing window
	Point2D normalize           ( Point3D point               );

	// Updates the application mode
	void    update_mode         ( Mode mode                   );

	// Updates the information bar
	void    update_infobar      ();

	// The AppWindow object
	AppWindow*  m_window;

	// Current application mode
	Mode        m_mode;

	// Mouse information
	bool        m_button1, m_button2, m_button3;
	double      m_ixpos,   m_iypos;
	double      m_xpos,    m_ypos;
	double      m_txpos;

	// Viewport corners
	Point2D     m_viewport[4];

	// FOV value, default 30
	double      m_fov;

	// Clipping planes
	double      m_near;
	double      m_far;

	// Transformation matrices
	Matrix4x4   m_projection;
	Matrix4x4   m_modelling;
	Matrix4x4   m_viewing;
	Matrix4x4   m_scaling;

	// Stores the unit cube and the transformed unit cube
	Point3D     m_unitCube[8];
	Point3D     m_unitCubeTrans[8];

	// Stores gnomons
	Point3D     m_gnomon[4];
	Point3D     m_gnomonTrans[4];

	// Flags for initializing and resetting state
	bool        m_initflag;
	bool        m_viewflag;

	// Pointer to the infobar
	Gtk::Label* m_infobar;
};

#endif
