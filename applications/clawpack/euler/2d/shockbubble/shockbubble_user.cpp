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

#include "shockbubble_user.h"

#include <fclaw2d_include_all.h>

#include <fclaw2d_clawpatch.h>

#include <fc2d_clawpack46.h>
#include <fc2d_clawpack46_options.h>

#include <fc2d_clawpack5.h>
#include <fc2d_clawpack5_options.h>

#include "../rp/euler_user_fort.h"

void shockbubble_link_solvers(fclaw2d_global_t *glob)
{
    const user_options_t* user = shockbubble_get_options(glob);
    fclaw2d_vtable_t *fclaw_vt = fclaw2d_vt();
    fclaw2d_clawpatch_vtable_t *clawpatch_vt = fclaw2d_clawpatch_vt();

    fclaw_vt->problem_setup = &shockbubble_problem_setup;

    if (user->claw_version == 4)
    {
        fc2d_clawpack46_vtable_t *clawpack46_vt = fc2d_clawpack46_vt();
        fc2d_clawpack46_options_t *clawopt = fc2d_clawpack46_get_options(glob);

        clawpack46_vt->qinit  = &CLAWPACK46_QINIT;
        clawpack46_vt->setaux = &CLAWPACK46_SETAUX;
        clawpack46_vt->bc2    = &CLAWPACK46_BC2;   /* Special  BCs at left edge */
        clawpack46_vt->src2   = &CLAWPACK46_SRC2;  /* To simulate axis-symmetric */

        switch (clawopt->mwaves)
        {
        case 4:
            /* Requires meqn=4 */
            clawpack46_vt->rpn2   = &CLAWPACK46_RPN2_EULER4;  /* No tracer */
            clawpack46_vt->rpt2   = &CLAWPACK46_RPT2_EULER4;
            break;
        case 5:
            /* Requires meqn=5 */
            clawpack46_vt->rpn2   = &CLAWPACK46_RPN2_EULER5;  /* Includes a tracer */
            clawpack46_vt->rpt2   = &CLAWPACK46_RPT2_EULER5;
            break;
        default:
            SC_ABORT_NOT_REACHED ();
        }

        /* Use divided differences to tag grids */
        clawpatch_vt->fort_tag4refinement = &CLAWPACK46_TAG4REFINEMENT;
        clawpatch_vt->fort_tag4coarsening = &CLAWPACK46_TAG4COARSENING;
    }
    else if (user->claw_version == 5)
    {
        fc2d_clawpack5_vtable_t *clawpack5_vt = fc2d_clawpack5_vt();
        fclaw2d_clawpatch_vtable_t *clawpatch_vt = fclaw2d_clawpatch_vt();
        fc2d_clawpack5_options_t *clawopt = fc2d_clawpack5_get_options(glob);

        clawpack5_vt->qinit  = &CLAWPACK5_QINIT;
        clawpack5_vt->setaux = &CLAWPACK5_SETAUX;
        clawpack5_vt->bc2    = &CLAWPACK5_BC2;   /* Special  BCs at left edge */
        clawpack5_vt->src2   = &CLAWPACK5_SRC2;  /* To simulate axis-symmetric */


        switch (clawopt->mwaves)
        {
        case 4:
            /* Requires meqn=4 */
            clawpack5_vt->rpn2   = &CLAWPACK5_RPN2_EULER4;  /* no tracer */
            clawpack5_vt->rpt2   = &CLAWPACK5_RPT2_EULER4;
            break;
        case 5:
            /* Requires meqn=5 */
            clawpack5_vt->rpn2   = &CLAWPACK5_RPN2_EULER5;  /* Includes a tracer */
            clawpack5_vt->rpt2   = &CLAWPACK5_RPT2_EULER5;
            break;
        default:
            SC_ABORT_NOT_REACHED ();
        }

        /* Use divided differences to tag grids */
        clawpatch_vt->fort_tag4refinement = &CLAWPACK5_TAG4REFINEMENT;
        clawpatch_vt->fort_tag4coarsening = &CLAWPACK5_TAG4COARSENING;
    }
}

void shockbubble_problem_setup(fclaw2d_global_t* glob)
{
    const user_options_t* user = shockbubble_get_options(glob);

    SHOCKBUBBLE_SETPROB(&user->gamma, &user->x0, &user->y0, &user->r0,
                        &user->rhoin, &user->pinf, &user->idisc);
}
