/*
Copyright (c) 2012 Carsten Burstedde, Donna Calhoun
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef FCLAW2D_CLAWPATCH_H
#define FCLAW2D_CLAWPATCH_H

#include <forestclaw2d.h>       /* Need patch callback def */

#include <fclaw2d_clawpatch_fort.h>

#ifdef __cplusplus
extern "C"
{
#if 0
}                               /* need this because indent is dumb */
#endif
#endif

typedef struct fclaw2d_clawpatch_vtable fclaw2d_clawpatch_vtable_t;

/* ---------------------------- Virtual table ------------------------------------ */

/* members of this structure provide the only access to above functions */

void fclaw2d_clawpatch_vtable_initialize();

fclaw2d_clawpatch_vtable_t* fclaw2d_clawpatch_vt();

struct fclaw2d_clawpatch_vtable
{
    /* ghost filling functions */
    clawpatch_fort_copy_face_t             fort_copy_face;
    clawpatch_fort_average_face_t          fort_average_face;
    clawpatch_fort_interpolate_face_t      fort_interpolate_face;
    clawpatch_fort_copy_corner_t           fort_copy_corner;
    clawpatch_fort_average_corner_t        fort_average_corner;
    clawpatch_fort_interpolate_corner_t    fort_interpolate_corner;

    /* regridding functions */
    clawpatch_fort_tag4refinement_t        fort_tag4refinement;
    clawpatch_fort_tag4coarsening_t        fort_tag4coarsening;
    clawpatch_fort_average2coarse_t        fort_average2coarse;
    clawpatch_fort_interpolate2fine_t      fort_interpolate2fine;

    /* output functions (ascii) */
    clawpatch_fort_header_ascii_t          fort_header_ascii;

    fclaw2d_patch_callback_t               cb_output_ascii;    
    clawpatch_fort_output_ascii_t          fort_output_ascii;

    /* Time interpolation functions */
    clawpatch_fort_timeinterp_t            fort_timeinterp;

    /* ghost patch functions */
    clawpatch_fort_local_ghost_pack_t      fort_local_ghost_pack;
    clawpatch_fort_local_ghost_pack_aux_t  fort_local_ghost_pack_aux;

    /* diagnostic functions */
    clawpatch_fort_error_t                 fort_compute_patch_error;
    clawpatch_fort_conscheck_t             fort_conservation_check;
    clawpatch_fort_norm_t                  fort_compute_error_norm;
    clawpatch_fort_area_t                  fort_compute_patch_area;

    int is_set;
};


/* -------------------------------- time stepping ----------------------------------- */

/* Called in step2 (clawpack 4.6 and clawpack 5.0) */
void fclaw2d_clawpatch_save_current_step(struct fclaw2d_global* glob,
                                         struct fclaw2d_patch* this_patch);


/* ------------------------------- Misc access functions ------------------------------ */

void fclaw2d_clawpatch_grid_data(struct fclaw2d_global* glob,
                                 struct fclaw2d_patch* this_patch,
                                 int* mx, int* my, int* mbc,
                                 double* xlower, double* ylower,
                                 double* dx, double* dy);


void fclaw2d_clawpatch_metric_data(struct fclaw2d_global* glob,
                                   struct fclaw2d_patch* this_patch,
                                   double **xp, double **yp, double **zp,
                                   double **xd, double **yd, double **zd,
                                   double **area);

void fclaw2d_clawpatch_metric_data2(struct fclaw2d_global* glob,
                                    struct fclaw2d_patch* this_patch,
                                    double **xnormals, double **ynormals,
                                    double **xtangents, double **ytangents,
                                    double **surfnormals, double ** edgelengths,
                                    double **curvature);

double* fclaw2d_clawpatch_get_area(struct fclaw2d_global* glob,
                                   struct fclaw2d_patch* this_patch);

void fclaw2d_clawpatch_soln_data(struct fclaw2d_global* glob,
                                 struct fclaw2d_patch* this_patch,
                                 double **q, int* meqn);

void fclaw2d_clawpatch_aux_data(struct fclaw2d_global *glob,
                                struct fclaw2d_patch *this_patch,
                                double **aux, int* maux);

double* fclaw2d_clawpatch_get_q(struct fclaw2d_global* glob,
                                struct fclaw2d_patch* this_patch);


double* fclaw2d_clawpatch_get_error(struct fclaw2d_global* glob,
                                    struct fclaw2d_patch* this_patch);

size_t fclaw2d_clawpatch_size(struct fclaw2d_global *glob);

void fclaw2d_clawpatch_timesync_data(struct fclaw2d_global* glob,
                                     struct fclaw2d_patch* this_patch,
                                     int time_interp,
                                     double **q, int* meqn);

double* fclaw2d_clawpatch_get_q_timesync(struct fclaw2d_global* glob,
                                         struct fclaw2d_patch* this_patch,
                                         int time_interp);


#ifdef __cplusplus
#if 0
{                               /* need this because indent is dumb */
#endif
}
#endif

#endif /* !FCLAW2D_CLAWPATCH_H */
