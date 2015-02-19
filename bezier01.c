#include <gtk/gtk.h>

#include "bezier.h"




struct window_main_ {
	GtkWidget* window_main;
	GtkWidget* drawing_area;
	bezier bez;
};

typedef struct window_main_ window_main;




static gboolean drawing_area_button_press_event(GtkWidget *widget,
               GdkEventButton *event, window_main *wm);
static gboolean drawing_area_button_release_event(GtkWidget *widget,
               GdkEventButton *event, window_main *wm);
static gboolean drawing_area_motion_notify_event(GtkWidget *widget,
               GdkEventButton *event,
               window_main *wm);
static gboolean expose_event(GtkWidget* widget, GdkEventExpose* event, window_main *wm);
void window_main_init(window_main *wm);




window_main wm;




void create(window_main *wm)
{
	wm->window_main = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	wm->drawing_area = gtk_drawing_area_new();

	gtk_widget_add_events(GTK_WIDGET(wm->drawing_area),
			GDK_BUTTON_PRESS_MASK |
			GDK_BUTTON_RELEASE_MASK |
			GDK_POINTER_MOTION_MASK);

	gtk_container_add(GTK_CONTAINER(wm->window_main), wm->drawing_area);

	g_signal_connect(GTK_WIDGET(wm->drawing_area), "button-press-event", G_CALLBACK(drawing_area_button_press_event), wm);
	g_signal_connect(GTK_WIDGET(wm->drawing_area), "button-release-event", G_CALLBACK(drawing_area_button_release_event), wm);
	g_signal_connect(GTK_WIDGET(wm->drawing_area), "motion-notify-event", G_CALLBACK(drawing_area_motion_notify_event), wm);
	g_signal_connect(wm->drawing_area, "expose-event", G_CALLBACK(expose_event), wm);
	g_signal_connect_swapped(wm->window_main, "destroy", G_CALLBACK(gtk_main_quit), NULL);

	gtk_window_resize(GTK_WINDOW(wm->window_main), 1000, 800);
	gtk_widget_show_all(wm->window_main);
}

static gboolean drawing_area_button_press_event(GtkWidget *widget,
               GdkEventButton *event, window_main *wm)
{
	bezier_points_update(&wm->bez, event->x, event->y, 1);
	gtk_widget_queue_draw(wm->drawing_area);
}

static gboolean drawing_area_button_release_event(GtkWidget *widget,
               GdkEventButton *event, window_main *wm)
{
	bezier_points_update(&wm->bez, event->x, event->y, 3);
	gtk_widget_queue_draw(wm->drawing_area);
}

static gboolean drawing_area_motion_notify_event(GtkWidget *widget,
               GdkEventButton *event,
               window_main *wm)
{
	bezier_points_update(&wm->bez, event->x, event->y, 2);
	gtk_widget_queue_draw(wm->drawing_area);
}

static gboolean expose_event(GtkWidget* widget, GdkEventExpose* event, window_main *wm)
{
	cairo_t* c;
	GtkAllocation allocation;
	
	
	gtk_widget_get_allocation(wm->drawing_area, &allocation);
	c = gdk_cairo_create(wm->drawing_area->window);

	int offset = 15;
	
	
	cairo_set_line_width(c, 1);

	allocation.x = offset;
	allocation.y = offset;
	allocation.width -= (offset * 2);
	allocation.height -= (offset * 2);

	cairo_rectangle(c, allocation.x, allocation.y, allocation.width, allocation.height);

	cairo_set_source_rgb(c, 1, 1, 1);
	cairo_fill_preserve(c);
	cairo_set_source_rgb(c, 0, 0, 0);
	cairo_stroke(c);

	allocation.x += offset;
	allocation.y += offset;
	allocation.width -= (offset * 2);
	allocation.height -= (offset * 2);

	bezier_draw_points(&wm->bez, c);

	cairo_destroy(c);
}

void window_main_init(window_main *wm)
{
	wm->window_main = NULL;
	wm->drawing_area = NULL;

	bezier_init(&wm->bez);
}

int main(int argc, char *argv[])
{
	gtk_init(&argc, &argv);

	window_main_init(&wm);

	create(&wm);

	gtk_main();

	return 0;
}
