#ifndef CLIENT_GUI_H
#define CLIENT_GUI_H

#include <gtk/gtk.h>
#include <glib.h>

#define WINDOW_WIDTH	800
#define WINDOW_HEIGHT	600

struct GUI
{
	GtkWidget *window;
	GtkWidget *grid;
	GtkWidget *menu_bar;
	GtkWidget *chat_tabs_notebook;
	GtkWidget *chat_box_main;
};

/* Initialize a chat program GUI for a client */
GtkWidget *initialize_gui(int argc, char **argv);

/* Create a chat box for a GUI */
GtkWidget *create_chat_box(void);

/* Create a menu bar for a GUI */
GtkWidget *create_menu_bar(void);

#endif
