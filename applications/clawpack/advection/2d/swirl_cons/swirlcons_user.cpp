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

#include "swirlcons_user.h"

#include <fclaw2d_include_all.h>

#include <fc2d_clawpack46.h>
#include <fc2d_clawpack46_options.h>
#include <clawpack46_user_fort.h>    /* Headers for user defined fortran files */

#include <fclaw2d_clawpatch.h>
#include <fclaw2d_clawpatch_fort.h>  /* headers for tag2refinement, tag4coarsening  */

#include "../all/advection_user_fort.h"  

void swirlcons_link_solvers(fclaw2d_global_t *glob)
{
    fclaw2d_vtable_t                     *vt = fclaw2d_vt();
    fclaw2d_patch_vtable_t         *patch_vt = fclaw2d_patch_vt();
    fclaw2d_clawpatch_vtable_t *clawpatch_vt = fclaw2d_clawpatch_vt();
    fc2d_clawpack46_vtable_t  *clawpack46_vt = fc2d_clawpack46_vt();
    fc2d_clawpack46_options_t *clawopt = fc2d_clawpack46_get_options(glob);
    const user_options_t* user = swirlcons_get_options(glob);

    /* ForestClaw functions */
    vt->problem_setup = &swirlcons_problem_setup;  /* Version-independent */

    /* ClawPatch specific functions */
    clawpatch_vt->fort_tag4coarsening = &TAG4COARSENING;
    clawpatch_vt->fort_tag4refinement = &TAG4REFINEMENT;

    /* Patch specific functions */
    patch_vt->setup   = &swirlcons_patch_setup_manifold;


    clawopt->use_fwaves = 0;
    switch(user->mapping)
    {
        case 0:
            clawpack46_vt->fort_rpt2      = &RPT2CONS;
            clawpack46_vt->fort_rpn2_cons = &RPN2_CONS_UPDATE;
            switch(user->rp_solver)
            {
                case 1:  /* QS */
                    clawpack46_vt->fort_rpn2      = &RPN2CONS_QS;
                case 2:  /* WD */
                    clawpack46_vt->fort_rpn2      = &RPN2CONS_WD; 
                case 3:  /* EC */
                    clawpack46_vt->fort_rpn2      = &RPN2CONS_EC;                 
                case 4:  /* FW */
                    clawopt->use_fwaves = 1;
                    clawpack46_vt->fort_rpn2      = RPN2CONS_FW;
            }

            break;

        case 1: /* Cart */
        case 2: /* Fivepatch */
        case 3: /* bilinear */
            clawpack46_vt->fort_rpt2      = &RPT2CONS_MANIFOLD;      
            clawpack46_vt->fort_rpn2_cons = &RPN2_CONS_UPDATE_MANIFOLD;
            switch(user->rp_solver)
            {
                case 1:  /* QS */
                    clawpack46_vt->fort_rpn2      = &RPN2CONS_QS_MANIFOLD; 
                case 2:  /* WD */
                    clawpack46_vt->fort_rpn2      = &RPN2CONS_WD; 
                case 3:  /* EC */
                    clawpack46_vt->fort_rpn2      = &RPN2CONS_EC;                 
                case 4:  /* FW */
                    clawopt->use_fwaves = 1;
                    clawpack46_vt->fort_rpn2      = RPN2CONS_FW_MANIFOLD; 
            }


            break;
    }

    clawpack46_vt->fort_qinit     = CLAWPACK46_QINIT;
    clawpack46_vt->fort_setaux    = CLAWPACK46_SETAUX;
    clawpack46_vt->fort_rpt2      = RPT2CONS;

#if 0    
    switch(user->rp_solver)
    {
        case 1:
            if (user->mapping == 0)
            {
                clawpack46_vt->fort_rpn2      = &RPN2CONS_QS; 
            }
            else
            {
                clawpack46_vt->fort_rpn2      = &RPN2CONS_QS_MANIFOLD; 
            }
            break; 

        case 2:
            clawpack46_vt->fort_rpn2      = &RPN2CONS_WD; 
            break; 

        case 3:
            if (user->mapping == 0)
            {
                clawpack46_vt->fort_rpn2      = &RPN2CONS_EC;                 
            }
            else
            {
                clawpack46_vt->fort_rpn2      = &RPN2CONS_EC_MANIFOLD;                                 
            }
            break;

        case 4:
            clawopt->use_fwaves = 1;
            if (user_mapping == 0)
            {
                clawpack46_vt->fort_rpn2      = RPN2CONS_FW; 

            }
            break;
    }
#endif    
 }

void swirlcons_problem_setup(fclaw2d_global_t* glob)
{
    const user_options_t* user = swirlcons_get_options(glob);

    int ex = user->example;
    SWIRL_SETPROB(&ex);
}


void swirlcons_patch_setup_manifold(fclaw2d_global_t *glob,
                                    fclaw2d_patch_t *this_patch,
                                    int this_block_idx,
                                    int this_patch_idx)
{
    const user_options_t* user = swirlcons_get_options(glob);

    int mx,my,mbc,maux;
    double xlower,ylower,dx,dy;
    double *aux,*edgelengths,*area, *curvature;
    double *xp, *yp, *zp, *xd, *yd, *zd;
    double *xnormals,*ynormals,*xtangents,*ytangents,*surfnormals;

    fclaw2d_clawpatch_grid_data(glob,this_patch,&mx,&my,&mbc,
                                &xlower,&ylower,&dx,&dy);

    fclaw2d_clawpatch_metric_data(glob,this_patch,&xp,&yp,&zp,
                                  &xd,&yd,&zd,&area);

    fclaw2d_clawpatch_metric_scalar(glob, this_patch,&area,&edgelengths,
                                    &curvature);

    fclaw2d_clawpatch_metric_vector(glob,this_patch,
                                    &xnormals, &ynormals,
                                    &xtangents, &ytangents,
                                    &surfnormals);

    fclaw2d_clawpatch_aux_data(glob,this_patch,&aux,&maux);

    if (user->mapping == 0)
    {
        int maxmx, maxmy;
        maxmx = mx;
        maxmy = my;
        CLAWPACK46_SETAUX(&maxmx, &maxmy, &mbc,&mx,&my,&xlower,&ylower,
                          &dx,&dy,&maux,aux);
    }
    else 
    {
        CLAWPACK46_SETAUX_MANIFOLD(&mbc,&mx,&my,&xlower,&ylower,
                                   &dx,&dy,&maux,aux,&this_block_idx,
                                   xp,yp,zp,area,
                                   edgelengths,xnormals,ynormals,surfnormals);
    }
}






