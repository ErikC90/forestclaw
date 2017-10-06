/* Pillow grid surface.  Matches p4est_connectivity_new_pillow (). */

#include <fclaw2d_map.h>

#ifdef __cplusplus
extern "C"
{
#if 0
}
#endif
#endif

static int
fclaw2d_map_query_twisted_torus (fclaw2d_map_context_t * cont, int query_identifier)
{
    switch (query_identifier)
    {
    case FCLAW2D_MAP_QUERY_IS_USED:
        return 1;
        /* no break necessary after return statement */
    case FCLAW2D_MAP_QUERY_IS_SCALEDSHIFT:
        return 0;
    case FCLAW2D_MAP_QUERY_IS_AFFINE:
        return 0;
    case FCLAW2D_MAP_QUERY_IS_NONLINEAR:
        return 1;
    case FCLAW2D_MAP_QUERY_IS_CART:
        return 0;
    case FCLAW2D_MAP_QUERY_IS_GRAPH:
        return 0;
    case FCLAW2D_MAP_QUERY_IS_PLANAR:
        return 0;
    case FCLAW2D_MAP_QUERY_IS_ALIGNED:
        return 0;
    case FCLAW2D_MAP_QUERY_IS_FLAT:
        return 0;
    case FCLAW2D_MAP_QUERY_IS_DISK:
        return 0;
    case FCLAW2D_MAP_QUERY_IS_SPHERE:
        return 0;
    case FCLAW2D_MAP_QUERY_IS_PILLOWDISK:
        return 0;
    case FCLAW2D_MAP_QUERY_IS_SQUAREDDISK:
        return 0;
    case FCLAW2D_MAP_QUERY_IS_PILLOWSPHERE:
        return 0;
    case FCLAW2D_MAP_QUERY_IS_CUBEDSPHERE:
        return 0;
    case FCLAW2D_MAP_QUERY_IS_BRICK:
        return 0;
    default:
        printf("\n");
        printf("fclaw2d_map_query_twisted_torus (fclaw2d_map_twisted_torus.c) : Query id not "\
               "identified;  Maybe the query is not up to date?\nSee "  \
               "fclaw2d_map_torus.c.\n");
        printf("Requested query id : %d\n",query_identifier);
        SC_ABORT_NOT_REACHED ();
    }
    return 0;
}


static void
fclaw2d_map_c2m_twisted_torus (fclaw2d_map_context_t * cont, int blockno,
                               double xc, double yc,
                               double *xp, double *yp, double *zp)
{
    double xc1,yc1,zc1;

    /* Scale's brick mapping to [0,1]x[0,1] */
    /* fclaw2d_map_context_t *brick_map = (fclaw2d_map_context_t*) cont->user_data; */
    FCLAW2D_MAP_BRICK2C(&cont,&blockno,&xc,&yc,&xc1,&yc1,&zc1);

    /* blockno is ignored in the current torus mapping;  it just assumes
       a single "logical" block in [0,1]x[0,1] */
    double alpha = cont->user_double[0];
    MAPC2M_TWISTED_TORUS(&blockno,&xc1,&yc1,xp,yp,zp,&alpha);

    rotate_map(cont,xp,yp,zp);
}

fclaw2d_map_context_t *
    fclaw2d_map_new_twisted_torus (fclaw2d_map_context_t* brick,
                                   const double scale[],
                                   const double shift[],
                                   const double rotate[],
                                   const double alpha)
{
    fclaw2d_map_context_t *cont;

    cont = FCLAW_ALLOC_ZERO (fclaw2d_map_context_t, 1);
    cont->query = fclaw2d_map_query_twisted_torus;
    cont->mapc2m = fclaw2d_map_c2m_twisted_torus;

    cont->user_double[0] = alpha;

    set_scale(cont,scale);
    set_shift(cont,shift);
    set_rotate(cont,rotate);

    cont->brick = brick;

    return cont;
}

#ifdef __cplusplus
#if 0
{
#endif
}
#endif
