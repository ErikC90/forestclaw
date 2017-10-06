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

#ifndef TRANSPORT_USER_H
#define TRANSPORT_USER_H

#include <fclaw2d_include_all.h>
#include <fclaw_base.h>  /* for MPI */

#include "../../advection/2d/all/advection_user_fort.h"


#ifdef __cplusplus
extern "C"
{
#if 0
}
#endif
#endif

struct fclaw_options;
struct user_options;
struct fclaw2d_patch;
struct fclaw2d_domain;

/* 1 = cubed sphere domain; 2 - pillowgrid */
struct fclaw2d_domain* create_domain(sc_MPI_Comm mpicomm, 
                                struct fclaw_options* fclaw_opt,
                                struct user_options* user_opt);

fclaw2d_map_context_t * fclaw2d_map_new_cubedsphere (const double scale[],
                                                     const double shift[],
                                                     const double rotate[]);

fclaw2d_map_context_t * fclaw2d_map_new_pillowsphere (const double scale[],
                                                      const double shfit[],
                                                      const double rotate[]);


void transport_patch_setup(struct fclaw2d_global *glob,
                           struct fclaw2d_patch *this_patch,
                           int this_block_idx,
                           int this_patch_idx);

double transport_update(struct fclaw2d_global *glob,
                        struct fclaw2d_patch *this_patch,
                        int this_block_idx,
                        int this_patch_idx,
                        double t,
                        double dt);

void transport_b4step2(struct fclaw2d_global *glob,
                       struct fclaw2d_patch *this_patch,
                       int this_block_idx,
                       int this_patch_idx,
                       double t,
                       double dt);

void transport_link_solvers(struct fclaw2d_global *glob);


#define TRANSPORT_SETPROB FCLAW_F77_FUNC(transport_setprob,TRANSPORT_SETPROB)
void TRANSPORT_SETPROB(const double* kappa, const double* tfinal);


#define USER46_B4STEP2_MANIFOLD FCLAW_F77_FUNC(user46_b4step2_manifold,USER46_B4STEP2_MANIFOLD)
void USER46_B4STEP2_MANIFOLD(const int* mx, const int* my, const int* mbc,
                             const double* dx, const double* dy,
                             const double* t, const int* maux, double aux[],
                             const int* blockno,
                             double xd[], double yd[], double zd[]);

#define USER5_B4STEP2_MANIFOLD FCLAW_F77_FUNC(user5_b4step2_manifold,USER5_B4STEP2_MANIFOLD)
void USER5_B4STEP2_MANIFOLD(const int* mx, const int* my, const int* mbc,
                            const double* dx, const double* dy,
                            const double* t, const int* maux, double aux[],
                            const int* blockno,
                            double xd[], double yd[], double zd[]);

#ifdef __cplusplus
#if 0
{
#endif
}
#endif

#endif
