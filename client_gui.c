#include "client_gui.h"

/* Initialize a chat program GUI for a client */
struct GUI *initialize_gui(int argc, char **argv)
{
	//Allocate space for a new GUI object
	struct GUI *gui = (struct GUI *)malloc(sizeof(struct GUI));
	GtkWidget *label;
	
	//Initialize GTK
	gtk_init(&argc, &argv);
	
	//Create the main window
	gui->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(gui->window), "Logan's Chat Program");
	gtk_window_set_default_size(GTK_WINDOW(gui->window), WINDOW_WIDTH, WINDOW_HEIGHT);
	
	//Create the GTK grid box and add it to the main window
	gui->grid = gtk_grid_new();
	gtk_container_add(GTK_CONTAINER(gui->window), gui->grid);
	
	//Create the menu bar and add it to the GUI
	gui->menu_bar = create_menu_bar();
	gtk_grid_attach(gui->grid, gui->menu_bar, 0, 0, 10, 1);
	
	//Create the chat tabs notebook to show the chat 
	gui->chat_tabs_notebook = gtk_notebook_new();
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(gui->chat_tabs_notebook), GTK_POS_TOP);
	
	//Create the main chat window and add it to a new notebook page
	label = gtk_label_new("Main");
	gui->chat_box = create_chat_box();
	gtk_notebook_append_page(GTK_NOTEBOOK(gui->chat_tabs_notebook), chat, label);
	
	//Set the destroy conditions for the window
	g_signal_connect_swapped(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
	
	//Make the window visible and start the GUI
	gtk_widget_show_all(window);
	gtk_main();
}

/* Create a menu bar for a GUI */
GtkWidget *create_menu_bar(void)
{
	GtkWidget *menu_bar;
	
	//Submenu to allow a user to connect to a new server
	GtkWidget *chat_menu;
	GtkWidget *server_connect;
	GtkWidget *server_disconnect;
	
	//Create the menu bar
	menu_bar = gtk_menu_bar_new();
	
	//Create the chat menu and its items
	chat_menu = gtk_menu_new();
	server_connect = gtk_menu_item_new_with_label("Connect");
	server_disconnect = gtk_menu_item_new_with_label("Disconnect");
	
	//TODO: Create functions for "connect" and "disconnect" and set up gtk_signal_connect for the buttons
	
	//Add the chat menu items to the chat menu
	gtk_menu_shell_append(GTK_MENU_SHELL(chat_menu), server_connect);
	gtk_menu_shell_append(GTK_MENU_SHELL(chat_menu), server_disconnect);
	
	//Add the chat menu to the menu bar and return the completed menu bar
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), chat_menu);
	return menu_bar;
}

/* Create a chat box for a GUI */
GtkWidget *create_chat_box(void)
{
	GtkWidget *chat_box;

	GtkWidget *chat_log;
	GtkWidget *scroll_window;
	GtkWidget *input_box;
	GtkWidget *send_button;
	
	//Create the main chat window
	chat_box = gtk_grid_new();
	
	//Create the chat log view and set its properties
	chat_log = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(chat_log), FALSE);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(chat_log), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(chat_log), GTK_WRAP_WORD_CHAR);
	
	//Create the scroll window and add the chat_log to it
	scroll_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scroll_window), chat_log);
	
	//Create the text input box and send button
	input_box = gtk_entry_new();
	send_button = gtk_button_new_with_label("Send");
	
	//TODO: gtk_signal_connect(GTK_OBJECT(send_button), "clicked", GTK_SIGNAL_FUNC(<pointer to send function>), (gpointer) data <that should be the text in the box>)
	
	//Add the components to the chat_box
	gtk_grid_attach(chat_box, scroll_window, 0, 0, 10, 8);
	gtk_grid_attach(chat_box, input_box, 0, 8, 8, 2);
	gtk_grid_attach(chat_box, send_button, 8, 8, 2, 2);
	
	//Return the chat_box
	return chat_box;
}
