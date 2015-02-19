#ifndef BEZIER_H
#define BEZIER_H

#include <glib.h>




#define POINT_RADIUS    2




struct point_ {
	double x;
	double y;
};

typedef struct point_ point;

struct bezier_ {
	GList *points;
	point *p0;
	point *p1;
	point *p2;
	point dp10, dp21;
	int point_state;
};

typedef struct bezier_ bezier;




void bezier_draw_points(bezier *bez, cairo_t *c);
void bezier_init(bezier *bez);
void bezier_points_update(bezier *bez, double x, double y, int state);




#endif /* BEZIER_H */
