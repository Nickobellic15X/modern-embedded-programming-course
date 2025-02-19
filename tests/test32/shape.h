#ifndef SHAPE_H
#define SHAPE_H

#include <stdint.h>

/* Shape's attributes... */
typedef struct {
    struct ShapeVtable const *vptr; /* virtual pointer */

    int16_t x; /* x-coordinate of Shape's position */
    int16_t y; /* y-coordinate of Shape's position */
} Shape;


struct ShapeVtable {
    void (*draw)(Shape const * const me);
    uint32_t (*area)(Shape const * const me);
};

/* virtual calls (late binding) */
void Shape_draw_vcall(Shape const * const me);
uint32_t Shape_area_vcall(Shape const * const me);

/* virtual calls (late binding) in C99+ */
static inline void Shape_draw_vcall(Shape const * const me) {
    (*me->vptr->draw)(me);
}
static inline uint32_t Shape_area_vcall(Shape const * const me) {
    return (*me->vptr->area)(me);
}

#define SHAPE_DRAW_VCALL(me) (*(me)->vptr->draw)(me)
#define SHAPE_AREA_VCALL(me) (*(me)->vptr->area)(me)


/* Shape's operations... */
void Shape_ctor(Shape * const me, int16_t x0, int16_t y0);
void Shape_moveBy(Shape * const me, int16_t dx, int16_t dy);
uint16_t Shape_distanceFrom(Shape const * const me,
                            Shape const * other);

void Shape_draw(Shape const * const me);
uint32_t Shape_area(Shape const * const me);

//#define SHAPE_DRAW(me_) ((*(me_)->vptr->draw)((me_)))
//#define SHAPE_AREA(me_) ((*(me_)->vptr->area)((me_)))

void drawGraph(Shape const *graph[]);

#endif /* SHAPE_H */
