#ifndef APPWINDOW_HPP
#define APPWINDOW_HPP

#include <gtkmm.h>
#include "viewer.hpp"


class AppWindow : public Gtk::Window {
public:
	AppWindow();

	// Updates the mode menu radio buttons
	void update_mode   ( int mode         );

	// Updates the text in the information bar
	void update_infobar( std::string info );
  
protected:

private:
	// A "vertical box" which holds everything in our window
	Gtk::VBox    m_vbox;

	// The menubar, with all the menus at the top of the window
	Gtk::MenuBar m_menubar;
	// Each menu itself
	Gtk::Menu    m_menu_app;
	Gtk::Menu    m_menu_mode;

	// The information bar
	Gtk::Label   m_infobar;

	// The main OpenGL area
	Viewer       m_viewer;
};

#endif
