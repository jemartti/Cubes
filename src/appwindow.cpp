#include "appwindow.hpp"
#include <iostream>


AppWindow::AppWindow()
{
	set_title("CS488 Assignment Two");

	// A utility class for constructing things that go into menus, which
	// we'll set up next.
	using Gtk::Menu_Helpers::MenuElem;
	using Gtk::Menu_Helpers::RadioMenuElem;
	using Gtk::RadioMenuItem;

	// Set up the application menu
	// The slot we use here just causes AppWindow::hide() on this,
	// which shuts down the application.
	m_menu_app.items().push_back( MenuElem("Reset", Gtk::AccelKey( "a" ),
	sigc::mem_fun( m_viewer, &Viewer::reset_view )) );
	m_menu_app.items().push_back( MenuElem("_Quit", Gtk::AccelKey( "q" ),
	sigc::mem_fun( *this, &AppWindow::hide )) );

	// Set up the mode menu
	Gtk::RadioButtonGroup m_modegroup;
	sigc::slot1<void, Viewer::Mode> mode_slot =
			sigc::mem_fun( m_viewer, &Viewer::set_mode );
	m_menu_mode.items().push_back( RadioMenuElem(m_modegroup,
			"View R_otate",
			Gtk::AccelKey( "o" ),
			sigc::bind( mode_slot, Viewer::VIEWROTATE )) );
	m_menu_mode.items().push_back( RadioMenuElem(m_modegroup,
			"View Tra_nslate",
			Gtk::AccelKey( "n" ),
			sigc::bind( mode_slot, Viewer::VIEWTRANSLATE )) );
	m_menu_mode.items().push_back( RadioMenuElem(m_modegroup,
			"View _Perspective",
			Gtk::AccelKey( "p" ),
			sigc::bind( mode_slot, Viewer::VIEWPERSPECTIVE )) );
	m_menu_mode.items().push_back( RadioMenuElem(m_modegroup,
			"Model _Rotate",
			Gtk::AccelKey( "r" ),
			sigc::bind( mode_slot, Viewer::MODELROTATE )) );
	m_menu_mode.items().push_back( RadioMenuElem(m_modegroup,
			"Model _Translate",
			Gtk::AccelKey( "t" ),
			sigc::bind( mode_slot, Viewer::MODELTRANSLATE )) );
	m_menu_mode.items().push_back( RadioMenuElem(m_modegroup,
			"Model _Scale",
			Gtk::AccelKey( "s" ),
			sigc::bind( mode_slot, Viewer::MODELSCALE )) );
	m_menu_mode.items().push_back( RadioMenuElem(m_modegroup,
			"_Viewport",
			Gtk::AccelKey( "v" ),
			sigc::bind( mode_slot, Viewer::VIEWPORT )) );

	// Set up the menu bar
	m_menubar.items().push_back(Gtk::Menu_Helpers::MenuElem
		  ("_Application", m_menu_app));
	m_menubar.items().push_back(Gtk::Menu_Helpers::MenuElem
		  ("_Mode", m_menu_mode));

	// Pack in our widgets

	// First add the vertical box as our single "top" widget
	add(m_vbox);

	// Put the menubar on the top, and make it as small as possible
	m_vbox.pack_start( m_menubar, Gtk::PACK_SHRINK );

	// Put the viewer below the menubar. pack_start "grows" the widget
	// by default, so it'll take up the rest of the window.
	m_viewer.set_size_request( 300, 300 );
	m_vbox.pack_start( m_viewer );

	// Add the information bar to the bottom of the screen
	m_infobar.set_size_request( 300, 30 );
	m_infobar.set_label( "" );
	m_viewer.set_infobar( &m_infobar );
	m_vbox.pack_start( m_infobar, Gtk::PACK_SHRINK );

	show_all();

	// Set MODELROTATE to default mode
	static_cast<RadioMenuItem*>( &m_menu_mode.items()[3] )->set_active();

	// Set the pointer to the window object so we can communicate back
	m_viewer.set_window(this);
}

void AppWindow::update_mode( int mode )
{
	using Gtk::RadioMenuItem;

	// Set mode to be the active radio button
	static_cast<RadioMenuItem*>( &m_menu_mode.items()[mode] )->set_active();
}
