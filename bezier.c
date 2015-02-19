#include <gtk/gtk.h>
#include <math.h>
#include <stdlib.h>




#define POINT_RADIUS    4




struct point_ {
	double x;
	double y;
};

typedef struct point_ point;

struct window_main_ {
	GtkWidget* window_main;
	GtkWidget* drawing_area;
	GList *points;
	point *p0;
	point *p1;
	point *p2;
	point dp10, dp21;
	int point_state;
};

typedef struct window_main_ window_main;




void draw_points(window_main *wm, cairo_t *c);
gboolean drawing_area_button_press_event(GtkWidget *widget,
               GdkEventButton *event, window_main *wm);
gboolean drawing_area_button_release_event(GtkWidget *widget,
               GdkEventButton *event, window_main *wm);
gboolean drawing_area_motion_notify_event(GtkWidget *widget,
               GdkEventButton *event,
               window_main *wm);
void points_update(window_main *wm, double x, double y, int state);
static gboolean expose_event(GtkWidget* widget, GdkEventExpose* event, window_main *wm);
void spline_bezier_draw(cairo_t *c, point *p1, point *p2, point *p3, point *p4);
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

void draw_point(cairo_t* c, double x, double y)
{
	cairo_arc(c, x, y, POINT_RADIUS, 0, 2*M_PI);
	cairo_set_source_rgb(c, 1, 1, 1); 
	cairo_fill_preserve(c);
	cairo_set_source_rgb(c, 0, 0, 0);
	cairo_stroke(c);
}

void draw_points(window_main *wm, cairo_t *c)
{
	char buf[10];
	int index = 0;

	GList *iter = g_list_first(wm->points); // p0

	if (iter != NULL) {
		iter = g_list_next(iter);

		for (;;) {
			point *p1, *p2, *p3, *p4;

			p1 = iter->data; iter = g_list_next(iter);
			p2 = iter->data; iter = g_list_next(iter);

			if (iter == NULL)
				break;

			p3 = iter->data; iter = g_list_next(iter);
			p4 = iter->data;

			spline_bezier_draw(c, p1, p2, p3, p4);
		}
	}

	for (GList *iter = g_list_first(wm->points);
			iter != NULL;
			iter = g_list_next(iter))
	{
		draw_point(c,
			((point*) iter->data)->x,
			((point*) iter->data)->y);

		cairo_move_to(c,
			((point*) iter->data)->x - 10,
			((point*) iter->data)->y - 5);

		sprintf(buf, "%d", index++);
		cairo_show_text(c, buf);

		cairo_fill_preserve (c);
		cairo_stroke (c);
	}
}

gboolean drawing_area_button_press_event(GtkWidget *widget,
               GdkEventButton *event, window_main *wm)
{
	points_update(wm, event->x, event->y, 1);
}

gboolean drawing_area_button_release_event(GtkWidget *widget,
               GdkEventButton *event, window_main *wm)
{
	points_update(wm, event->x, event->y, 3);
}

gboolean drawing_area_motion_notify_event(GtkWidget *widget,
               GdkEventButton *event,
               window_main *wm)
{
	points_update(wm, event->x, event->y, 2);
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

	draw_points(wm, c);

	cairo_destroy(c);
}

void points_update(window_main *wm, double x, double y, int state)
{
	if (state == 1) {
		#define OVER_THE_POINT(x, y, px, py)    \
			((px) - POINT_RADIUS <= (x) && \
			(x) <= (px) + POINT_RADIUS && \
			(py) - POINT_RADIUS <= (y) && \
			(y) <= (py) + POINT_RADIUS)

		int index = 0;

		GList *iter, *iter_tmp;

		for (iter = g_list_first(wm->points);
				iter != NULL;
				iter = g_list_next(iter),
				++index)
		{
			if (OVER_THE_POINT(x, y,
				((point*) iter->data)->x,
				((point*) iter->data)->y))
			{
				switch (wm->point_state = index % 3) {
				case 0:
					wm->p0 = iter->data; iter = g_list_next(iter);
					wm->p1 = iter->data; iter = g_list_next(iter);
					wm->p2 = iter->data;
					goto LOOP_0;
				case 1:
					iter_tmp = iter;
					wm->p1 = iter->data; iter = g_list_previous(iter);
					wm->p0 = iter->data; iter = g_list_next(iter_tmp);
					wm->p2 = iter->data;
					goto LOOP_0;
				case 2:
					wm->p2 = iter->data; iter = g_list_previous(iter);
					wm->p1 = iter->data; iter = g_list_previous(iter);
					wm->p0 = iter->data;
					goto LOOP_0;
				}
			}
		}

		#undef OVER_THE_POINT

	LOOP_0:	if (wm->p0 == NULL) {
			point *pt;

			#define APPEND_THE_POINT(p) \
				pt = g_new(point, 1); \
				if (pt == NULL) { \
					g_print("no memmory\n"); \
					exit(1); \
				} \
				pt->x = x; pt->y = y; \
				wm->points = g_list_append(wm->points, pt); \
				p = pt;

			APPEND_THE_POINT(wm->p0);
			APPEND_THE_POINT(wm->p1);
			APPEND_THE_POINT(wm->p2);

			#undef APPEND_THE_POINT

			wm->point_state = 2;
		}

		wm->dp10.x = wm->p1->x - wm->p0->x;
		wm->dp10.y = wm->p1->y - wm->p0->y;

		wm->dp21.x = wm->p2->x - wm->p1->x;
		wm->dp21.y = wm->p2->y - wm->p1->y;
	}

	if (wm->p0 == NULL)
		return;

	if (wm->point_state == 0) {
		wm->p0->x = x;
		wm->p0->y = y;
		wm->p2->x = 2 * wm->p1->x - x;
		wm->p2->y = 2 * wm->p1->y - y;
	}
	else
	if (wm->point_state == 1) {
		wm->p1->x = x; wm->p1->y = y;

		wm->p0->x = wm->p1->x - wm->dp10.x;
		wm->p0->y = wm->p1->y - wm->dp10.y;

		wm->p2->x = wm->p1->x + wm->dp21.x;
		wm->p2->y = wm->p1->y + wm->dp21.y;
	}
	else
	if (wm->point_state == 2) {
		wm->p2->x = x;
		wm->p2->y = y;
		wm->p0->x = 2 * wm->p1->x - x;
		wm->p0->y = 2 * wm->p1->y - y;
	}

	if (state == 3) {
		wm->p0 = NULL;
		wm->p1 = NULL;
		wm->p2 = NULL;
	}

	gtk_widget_queue_draw(wm->drawing_area);
}

void spline_bezier_draw(cairo_t *c, point *p1, point *p2, point *p3, point *p4)
{
	#define BEZIER(p1, p2, p3, p4, t) \
		              (1 - t)*(1 - t)*(1 - t) * p1 \
		+ 3 * t     * (1 - t)*(1 - t)         * p2 \
		+ 3 * t*t   * (1 - t)                 * p3 \
		+     t*t*t                           * p4;

	double y = BEZIER(p1->y, p2->y, p3->y, p4->y, 0);
	double x = BEZIER(p1->x, p2->x, p3->x, p4->x, 0);

	cairo_move_to(c, x, y);

	for (int tt = 0; tt <= 100; ++tt) {
		double t = 0.01 * tt;

		y = BEZIER(p1->y, p2->y, p3->y, p4->y, t);
		x = BEZIER(p1->x, p2->x, p3->x, p4->x, t);

		cairo_line_to(c, x, y);
	}

	cairo_stroke(c);

	#undef BEZIER
}

void window_main_init(window_main *wm)
{
	wm->window_main = NULL;
	wm->drawing_area = NULL;
	wm->points = NULL;

	wm->p0 = NULL; wm->p1 = NULL; wm->p2 = NULL;
}

int main(int argc, char *argv[])
{
	gtk_init(&argc, &argv);

	create(&wm);

	gtk_main();

	return 0;
}
