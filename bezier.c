#include <gtk/gtk.h>
#include <stdlib.h>
#include <math.h>

#include "bezier.h"




void bezier_draw_spline(cairo_t *c, point *p1, point *p2, point *p3, point *p4);




void bezier_draw_point(cairo_t* c, double x, double y, int point_state)
{
	cairo_arc(c, x, y, POINT_RADIUS, 0, 2*M_PI);

	if (point_state == 1) {
		cairo_set_source_rgb(c, 0, 0, 0);
	}
	else {
		cairo_set_source_rgb(c, 1, 1, 1);
	}

	cairo_fill_preserve(c);
	cairo_set_source_rgb(c, 0, 0, 0); 
	cairo_stroke(c);
}

void bezier_draw_points(bezier *bez, cairo_t *c)
{
	char buf[10];
	int index = 0;

	GList *iter = g_list_first(bez->points); // p0

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

			bezier_draw_spline(c, p1, p2, p3, p4);
		}
	}

	int point_index = 0;

	for (GList *iter = g_list_first(bez->points);
			iter != NULL;
			iter = g_list_next(iter),
			++point_index)
	{
		bezier_draw_point(c,
			((point*) iter->data)->x,
			((point*) iter->data)->y,
			point_index % 3);
#if 0
		cairo_move_to(c,
			((point*) iter->data)->x - 10,
			((point*) iter->data)->y - 5);

		sprintf(buf, "%d", index++);
		cairo_show_text(c, buf);

		cairo_fill_preserve (c);
		cairo_stroke (c);
#endif
	}
}

void bezier_init(bezier *bez)
{
	bez->points = NULL;
	bez->p0 = NULL; bez->p1 = NULL; bez->p2 = NULL;
}

void bezier_points_update(bezier *bez, double x, double y, int state)
{
	if (state == 1) {
		#define OVER_THE_POINT(x, y, px, py)    \
			((px) - POINT_RADIUS <= (x) && \
			(x) <= (px) + POINT_RADIUS && \
			(py) - POINT_RADIUS <= (y) && \
			(y) <= (py) + POINT_RADIUS)

		int index = 0;

		GList *iter, *iter_tmp;

		for (iter = g_list_first(bez->points);
				iter != NULL;
				iter = g_list_next(iter),
				++index)
		{
			if (OVER_THE_POINT(x, y,
				((point*) iter->data)->x,
				((point*) iter->data)->y))
			{
				switch (bez->point_state = index % 3) {
				case 0:
					bez->p0 = iter->data; iter = g_list_next(iter);
					bez->p1 = iter->data; iter = g_list_next(iter);
					bez->p2 = iter->data;
					goto LOOP_0;
				case 1:
					iter_tmp = iter;
					bez->p1 = iter->data; iter = g_list_previous(iter);
					bez->p0 = iter->data; iter = g_list_next(iter_tmp);
					bez->p2 = iter->data;
					goto LOOP_0;
				case 2:
					bez->p2 = iter->data; iter = g_list_previous(iter);
					bez->p1 = iter->data; iter = g_list_previous(iter);
					bez->p0 = iter->data;
					goto LOOP_0;
				}
			}
		}

		#undef OVER_THE_POINT

	LOOP_0:	if (bez->p0 == NULL) {
			point *pt;

			#define APPEND_THE_POINT(p) \
				pt = g_new(point, 1); \
				if (pt == NULL) { \
					g_print("no memmory\n"); \
					exit(1); \
				} \
				pt->x = x; pt->y = y; \
				bez->points = g_list_append(bez->points, pt); \
				p = pt;

			APPEND_THE_POINT(bez->p0);
			APPEND_THE_POINT(bez->p1);
			APPEND_THE_POINT(bez->p2);

			#undef APPEND_THE_POINT

			bez->point_state = 2;
		}

		bez->dp10.x = bez->p1->x - bez->p0->x;
		bez->dp10.y = bez->p1->y - bez->p0->y;

		bez->dp21.x = bez->p2->x - bez->p1->x;
		bez->dp21.y = bez->p2->y - bez->p1->y;
	}

	if (bez->p0 == NULL)
		return;

	if (bez->point_state == 0) {
		bez->p0->x = x;
		bez->p0->y = y;
		bez->p2->x = 2 * bez->p1->x - x;
		bez->p2->y = 2 * bez->p1->y - y;
	}
	else
	if (bez->point_state == 1) {
		bez->p1->x = x; bez->p1->y = y;

		bez->p0->x = bez->p1->x - bez->dp10.x;
		bez->p0->y = bez->p1->y - bez->dp10.y;

		bez->p2->x = bez->p1->x + bez->dp21.x;
		bez->p2->y = bez->p1->y + bez->dp21.y;
	}
	else
	if (bez->point_state == 2) {
		bez->p2->x = x;
		bez->p2->y = y;
		bez->p0->x = 2 * bez->p1->x - x;
		bez->p0->y = 2 * bez->p1->y - y;
	}

	if (state == 3) {
		bez->p0 = NULL;
		bez->p1 = NULL;
		bez->p2 = NULL;
	}
}

void bezier_draw_spline(cairo_t *c, point *p1, point *p2, point *p3, point *p4)
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
