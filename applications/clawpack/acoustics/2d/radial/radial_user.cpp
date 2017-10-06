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

#include "radial_user.h"

#include <fclaw2d_include_all.h>

#include <fclaw2d_clawpatch.h>

#include <fc2d_clawpack46.h>
#include <fc2d_clawpack5.h>

#include "../rp/acoustics_user_fort.h"

void radial_link_solvers(fclaw2d_global_t *glob)
{
    fclaw2d_vtable_t *vt = fclaw2d_vt();

    vt->problem_setup = &radial_problem_setup;  /* Version-independent */

    const user_options_t* user = radial_get_options(glob);
    if (user->claw_version == 4)
    {
        fc2d_clawpack46_vtable_t *clawpack46_vt = fc2d_clawpack46_vt();

        clawpack46_vt->qinit     = &CLAWPACK46_QINIT;

        if (user->example == 0)
        {
            clawpack46_vt->rpn2      = &CLAWPACK46_RPN2;
            clawpack46_vt->rpt2      = &CLAWPACK46_RPT2;
        }
        else if (user->example == 1)
        {
            fclaw_global_essentialf("Not implemented for ClawPack 4.6\n");
            exit(0);
        }
    }
    else if (user->claw_version == 5)
    {
        fc2d_clawpack5_vtable_t    *clawpack5_vt = fc2d_clawpack5_vt();

        clawpack5_vt->qinit     = &CLAWPACK5_QINIT;

        if (user->example == 0)
        {
            clawpack5_vt->rpn2 = &CLAWPACK5_RPN2;
            clawpack5_vt->rpt2 = &CLAWPACK5_RPT2;
        }
        else if (user->example == 1)
        {
            fclaw2d_patch_vtable_t  *patch_vt = fclaw2d_patch_vt();
            fclaw2d_clawpatch_vtable_t *clawpatch_vt = fclaw2d_clawpatch_vt();
            
            patch_vt->setup = &radial_patch_setup;

            clawpack5_vt->rpn2  = &CLAWPACK5_RPN2_MANIFOLD;
            clawpack5_vt->rpt2  = &CLAWPACK5_RPT2_MANIFOLD;

            /* Avoid tagging block corners in 5 patch example*/
            clawpatch_vt->fort_tag4refinement = &CLAWPACK5_TAG4REFINEMENT;
            clawpatch_vt->fort_tag4coarsening = &CLAWPACK5_TAG4COARSENING;
        }
    }
}

void radial_problem_setup(fclaw2d_global_t* glob)
{
    user_options_t* user = radial_get_options(glob);

    /* rho, bulk are inputs; cc and zz are outputs.  Results are
       stored in a common block */
    RADIAL_SETPROB(&user->rho, &user->bulk, &user->cc, &user->zz);
}

void radial_patch_setup(fclaw2d_global_t *glob,
                        fclaw2d_patch_t *this_patch,
                        int this_block_idx,
                        int this_patch_idx)
{
    int mx,my,mbc,maux;
    double xlower,ylower,dx,dy;
    double *aux,*xd,*yd,*zd,*area;
    double *xp,*yp,*zp;
    double *xnormals,*ynormals,*xtangents,*ytangents;
    double *surfnormals,*edgelengths,*curvature;

    if (fclaw2d_patch_is_ghost(this_patch))
    {
        /* Mapped info is needed only for an update */
        return;
    }

    fclaw2d_clawpatch_grid_data(glob,this_patch,&mx,&my,&mbc,
                                &xlower,&ylower,&dx,&dy);

    fclaw2d_clawpatch_metric_data(glob,this_patch,&xp,&yp,&zp,
                                  &xd,&yd,&zd,&area);

    fclaw2d_clawpatch_metric_data2(glob,this_patch,
                                   &xnormals,&ynormals,
                                   &xtangents,&ytangents,
                                   &surfnormals,&edgelengths,
                                   &curvature);
    fclaw2d_clawpatch_aux_data(glob,this_patch,&aux,&maux);

    RADIAL_SETAUX_MANIFOLD(&mbc,&mx,&my,&xlower,&ylower,
                          &dx,&dy,&maux,aux,
                          xnormals,ynormals,edgelengths,area);
}
