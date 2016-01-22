#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <time.h>
#include "ui.h"
#include "config.h"

enum {
	COL_TIME,
	COL_ID,
	COL_LEN,
	COL_DATA,
	NUM_COLS
};

#define FORWARD 0
#define REVERSE 1

struct ui {
	GtkWidget *window;
	GtkWidget *filter_box;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *scroll;

	GtkWidget *treeview;
	GtkListStore *treestore;
  
	GtkWidget *btn_quit;
	GtkWidget *btn_ignition;
	GtkWidget *btn_reverse;

	GtkWidget *label_status;
	GtkWidget *label_filter;
	GtkWidget *entry_filter;
	GtkWidget *btn_filter;

	GtkWidget *label_canid;
	GtkWidget *entry_canid;
	GtkWidget *btn_canid;

	GtkWidget *img_quit;
	GtkWidget *img_ignition;
	GtkWidget *img_forward;
	GtkWidget *img_reverse;
	GtkWidget *img_stop;

	int ignition;
	int direction;

	ui_callback_t *callbacks[UI_EVENT_MAX];
};

extern uint32_t malicious_canid;

static void _btn_canid_cb(GtkWidget *widget, gpointer data)
{
	struct ui *ui;
	FILE *fd;
	uint32_t id;
	
	ui = (struct ui*)data;

	if(!(fd = fopen(CONFIG_CFG_PATH, "w+"))) {
		perror("fopen");
		return;
	}

	id = strtoul(gtk_entry_get_text(GTK_ENTRY(ui->entry_canid)), NULL, 16);

	if(fwrite(&id, sizeof(id), 1, fd) < 0) {
		perror("fwrite");
	}
	fclose(fd);

	malicious_canid = id;

	return;
}

static void _btn_ignition_cb(GtkWidget *widget, gpointer data)
{
	struct ui *ui = (struct ui*)data;

	if(ui->callbacks[UI_EVENT_IGNITION]) {
		ui->callbacks[UI_EVENT_IGNITION](ui);
	}
  
	if(ui->ignition == 0) {
		GtkWidget *img;
    
		ui->ignition = 1;
		img = gtk_image_new_from_stock(GTK_STOCK_NO, GTK_ICON_SIZE_BUTTON);
    
		if(img) {
			gtk_button_set_image(GTK_BUTTON(ui->btn_ignition), img);
		} else {
			gtk_button_set_label(GTK_BUTTON(ui->btn_ignition), "Stop");
		}
	} else {
		GtkWidget *img;
    
		ui->ignition = 0;
		img = gtk_image_new_from_stock(GTK_STOCK_YES, GTK_ICON_SIZE_BUTTON);

		if(img) {
			gtk_button_set_image(GTK_BUTTON(ui->btn_ignition), img);
		} else {
			gtk_button_set_label(GTK_BUTTON(ui->btn_ignition), "Start");
		}
	}
  
	return;
}

static void _btn_reverse_cb(GtkWidget *widget, gpointer data)
{
	struct ui *ui = (struct ui*)data;

	if(ui->callbacks[UI_EVENT_REVERSE]) {
		ui->callbacks[UI_EVENT_REVERSE](ui);
	}

	if(ui->direction == FORWARD) {
		GtkWidget *img;

		img = gtk_image_new_from_stock(GTK_STOCK_MEDIA_FORWARD, GTK_ICON_SIZE_BUTTON);
		ui->direction = REVERSE;
		if(img) {
			gtk_button_set_image(GTK_BUTTON(ui->btn_reverse), img);
		} else {
			gtk_button_set_label(GTK_BUTTON(ui->btn_reverse), "D");
		}
	} else {
		GtkWidget *img;

		img = gtk_image_new_from_stock(GTK_STOCK_MEDIA_REWIND, GTK_ICON_SIZE_BUTTON);
		ui->direction = FORWARD;

		if(img) {
			gtk_button_set_image(GTK_BUTTON(ui->btn_reverse), img);
		} else {
			gtk_button_set_label(GTK_BUTTON(ui->btn_reverse), "R");
		}
	}
  
	return;
}

static void _btn_quit_cb(GtkWidget *widget, gpointer data)
{
	gtk_main_quit();
	return;
}

static void _window_cb(GtkWidget *widget, gpointer data)
{
	gtk_main_quit();
	return;
}

int ui_init(int argc, char **argv, struct ui **u)
{
	struct ui *ui;
	GtkCellRenderer *renderer;

	if(!(ui = malloc(sizeof(*ui)))) {
		return(-ENOMEM);
	}

	memset(ui, 0, sizeof(*ui));
  
	gtk_init(&argc, &argv);

	ui->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	ui->filter_box = gtk_hbox_new(FALSE, 0);
	ui->vbox = gtk_vbox_new(FALSE, 0);
	ui->hbox = gtk_hbox_new(TRUE, 0);
	ui->scroll = gtk_scrolled_window_new(NULL, NULL);
	ui->treeview = gtk_tree_view_new();
	ui->treestore = gtk_list_store_new(NUM_COLS, G_TYPE_STRING,
									   G_TYPE_STRING, G_TYPE_UINT,
									   G_TYPE_STRING);

	ui->label_canid = gtk_label_new("Trigger CAN ID");
	ui->entry_canid = gtk_entry_new();
	ui->btn_canid = gtk_button_new();
	
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(ui->treeview),
												-1, "Timestamp", renderer,
												"text", COL_TIME, NULL);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(ui->treeview), -1, "CAN id", renderer, "text", COL_ID, NULL);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(ui->treeview), -1, "Length", renderer, "text", COL_LEN, NULL);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(ui->treeview), -1, "Data", renderer, "text", COL_DATA, NULL);

	gtk_tree_view_set_model(GTK_TREE_VIEW(ui->treeview), GTK_TREE_MODEL(ui->treestore));
	gtk_tree_view_columns_autosize(GTK_TREE_VIEW(ui->treeview));
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(ui->treeview), TRUE);
  
	ui->btn_ignition = gtk_button_new();
	ui->btn_reverse = gtk_button_new();
	ui->btn_quit = gtk_button_new();
	ui->btn_filter = gtk_button_new();

	ui->label_status = gtk_label_new("");
	ui->label_filter = gtk_label_new("Filter");
	ui->entry_filter = gtk_entry_new();

	gtk_button_set_image(GTK_BUTTON(ui->btn_ignition),
						 gtk_image_new_from_stock(GTK_STOCK_YES,
												  GTK_ICON_SIZE_BUTTON));
	gtk_button_set_image(GTK_BUTTON(ui->btn_reverse),
						 gtk_image_new_from_stock(GTK_STOCK_MEDIA_REWIND,
												  GTK_ICON_SIZE_BUTTON));
	gtk_button_set_image(GTK_BUTTON(ui->btn_quit),
						 gtk_image_new_from_stock(GTK_STOCK_QUIT,
												  GTK_ICON_SIZE_BUTTON));
	gtk_button_set_image(GTK_BUTTON(ui->btn_filter),
						 gtk_image_new_from_stock(GTK_STOCK_FIND,
												  GTK_ICON_SIZE_BUTTON));
	gtk_button_set_image(GTK_BUTTON(ui->btn_canid),
						 gtk_image_new_from_stock(GTK_STOCK_APPLY,
												  GTK_ICON_SIZE_BUTTON));

	gtk_container_add(GTK_CONTAINER(ui->window), ui->vbox);
	gtk_box_pack_start(GTK_BOX(ui->vbox), ui->filter_box, FALSE, TRUE, 3);
	gtk_box_pack_start(GTK_BOX(ui->vbox), ui->scroll, TRUE, TRUE, 3);
	gtk_box_pack_start(GTK_BOX(ui->vbox), ui->hbox, FALSE, TRUE, 3);

	gtk_box_pack_start(GTK_BOX(ui->filter_box), ui->label_canid, FALSE, TRUE, 3);
	gtk_box_pack_start(GTK_BOX(ui->filter_box), ui->entry_canid, FALSE, TRUE, 3);
	gtk_box_pack_start(GTK_BOX(ui->filter_box), ui->btn_canid, FALSE, TRUE, 3);
	gtk_box_pack_start(GTK_BOX(ui->filter_box), ui->label_status, TRUE, TRUE, 10);
	/*
	gtk_box_pack_start(GTK_BOX(ui->filter_box), ui->label_filter, FALSE, TRUE, 3);
	gtk_box_pack_start(GTK_BOX(ui->filter_box), ui->entry_filter, FALSE, TRUE, 3);
	gtk_box_pack_start(GTK_BOX(ui->filter_box), ui->btn_filter, FALSE, TRUE, 3);
	*/
/*
	gtk_box_pack_start(GTK_BOX(ui->hbox), ui->btn_ignition, FALSE, TRUE, 3);
	gtk_box_pack_start(GTK_BOX(ui->hbox), ui->btn_reverse, FALSE, TRUE, 3);
*/
	gtk_box_pack_start(GTK_BOX(ui->hbox), ui->btn_quit, FALSE, TRUE, 3);
	gtk_container_add(GTK_CONTAINER(ui->scroll), ui->treeview);

	gtk_entry_set_width_chars(GTK_ENTRY(ui->entry_canid), 3);
	gtk_entry_set_max_length(GTK_ENTRY(ui->entry_canid), 3);

	gtk_entry_set_width_chars(GTK_ENTRY(ui->entry_filter), 24);

	g_signal_connect(G_OBJECT(ui->btn_quit), "clicked", G_CALLBACK(_btn_quit_cb), ui);
	g_signal_connect(G_OBJECT(ui->btn_ignition), "clicked", G_CALLBACK(_btn_ignition_cb), ui);
	g_signal_connect(G_OBJECT(ui->btn_reverse), "clicked", G_CALLBACK(_btn_reverse_cb), ui);
	g_signal_connect(G_OBJECT(ui->window), "destroy", G_CALLBACK(_window_cb), ui);
	g_signal_connect(G_OBJECT(ui->btn_canid), "clicked", G_CALLBACK(_btn_canid_cb), ui);

	gtk_button_set_use_stock(GTK_BUTTON(ui->btn_ignition), TRUE);
	gtk_button_set_use_stock(GTK_BUTTON(ui->btn_reverse), TRUE);
	gtk_button_set_use_stock(GTK_BUTTON(ui->btn_quit), TRUE);

	gtk_window_set_default_size(GTK_WINDOW(ui->window), 640, 480);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(ui->scroll),
								   GTK_POLICY_AUTOMATIC,
								   GTK_POLICY_AUTOMATIC);
  
	*u = ui;

	return(0);
}

int ui_append_frame(struct ui *ui, uint32_t cid, int clen, char *cdata, time_t timestamp)
{
	GtkTreeIter iter;
	GtkTreePath *path;
	char str_id[12];
	char str_data[24];
	char str_ts[32];
	int i;

	memset(str_id, 0, sizeof(str_id));
	memset(str_data, 0, sizeof(str_data));
  
	strftime(str_ts, sizeof(str_ts), "%Y-%m-%d %H:%M:%S", localtime(&timestamp));
	snprintf(str_id, sizeof(str_id), "0x%03x", cid);
	for(i = 0; i < clen && i < (sizeof(str_data) / 2); i++) {
		snprintf(str_data + (i * 2), sizeof(str_data) - (i * 2), "%02x", (unsigned char)cdata[i]);
	}

	gtk_list_store_append(ui->treestore, &iter);
	gtk_list_store_set(ui->treestore, &iter,
					   COL_TIME, str_ts,
					   COL_ID, str_id,
					   COL_LEN, clen,
					   COL_DATA, str_data,
					   -1);

	path = gtk_tree_model_get_path(GTK_TREE_MODEL(ui->treestore), &iter);

	gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(ui->treeview),
								 path, NULL, FALSE, 0, 0);

	return(0);
}

int ui_add_idle(struct ui *ui, void (*func)(void*), void *data)
{
	/* first argument is actually not needed, but maybe it will be... */
  
	return(gtk_idle_add((GtkFunction)func, data));
}

int ui_add_input(struct ui *ui, int fd, GdkInputFunction func, void *data)
{
	return(gdk_input_add(fd, GDK_INPUT_READ, func, data));
}

void ui_set_canid(struct ui *ui, uint32_t canid)
{
	char buf[16];

	snprintf(buf, sizeof(buf), "%03x", canid);
	gtk_entry_set_text(GTK_ENTRY(ui->entry_canid), buf);
	return;
}

void ui_set_status(struct ui *ui, const char *msg, unsigned int color)
{
	char markup[64];

	snprintf(markup, sizeof(markup), "<span foreground=\"#%06X\">Status: %s</span>", color, msg);
	gtk_label_set_markup(GTK_LABEL(ui->label_status), markup);
	return;
}

void ui_draw(struct ui *ui)
{
	gtk_widget_show_all(ui->window);
	gtk_window_maximize(GTK_WINDOW(ui->window));
	gtk_main();

	return;
}
  
void ui_set_callback(struct ui *ui, ui_event_t event, ui_callback_t call)
{
	ui->callbacks[event] = call;
	return;
}
