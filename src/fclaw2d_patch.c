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

#include <fclaw2d_patch.h>

#include <forestclaw2d.h>
#include <fclaw2d_defs.h>
#include <fclaw2d_global.h>
#include <fclaw2d_domain.h>
#include <fclaw2d_transform.h>

static fclaw2d_patch_vtable_t s_patch_vt;

static
fclaw2d_patch_vtable_t* patch_vt_init()
{
    s_patch_vt.is_set = 0;
    return &s_patch_vt;
}

static
fclaw2d_patch_vtable_t* patch_vt()
{
    return &s_patch_vt;
}

/* -----------------------------------------------------------
   Public interface to routines in this file
   ----------------------------------------------------------- */

fclaw2d_patch_vtable_t* fclaw2d_patch_vt()
{
    FCLAW_ASSERT(s_patch_vt.is_set != 0);
    return &s_patch_vt;
}

void fclaw2d_patch_vtable_initialize()
{
    fclaw2d_patch_vtable_t *patch_vt = patch_vt_init();
    patch_vt->is_set = 1;

    /* Function pointers are set to NULL by default so are not set here */
}

struct fclaw2d_patch_data
{
    fclaw2d_patch_relation_t face_neighbors[4];
    fclaw2d_patch_relation_t corner_neighbors[4];
    int corners[4];
    int block_corner_count[4];
    int on_coarsefine_interface;
    int has_finegrid_neighbors;
    int neighbors_set;

    void *user_patch; /* Start of attempt to "virtualize" the user patch. */
};

fclaw2d_patch_data_t*
fclaw2d_patch_get_user_data(fclaw2d_patch_t* patch)
{
    return (fclaw2d_patch_data_t *) patch->user;
}

void*
fclaw2d_patch_get_user_patch(fclaw2d_patch_t* patch)

{
    fclaw2d_patch_data_t *pdata = fclaw2d_patch_get_user_data(patch);
    FCLAW_ASSERT(pdata != NULL);
    return pdata->user_patch;
}


void fclaw2d_patch_set_face_type(fclaw2d_patch_t *patch,int iface,
                                 fclaw2d_patch_relation_t face_type)
{
    fclaw2d_patch_data_t *pdata = fclaw2d_patch_get_user_data(patch);
    pdata->face_neighbors[iface] = face_type;
}

void fclaw2d_patch_set_corner_type(fclaw2d_patch_t *patch,int icorner,
                                   fclaw2d_patch_relation_t corner_type)
{
    fclaw2d_patch_data_t *pdata = fclaw2d_patch_get_user_data(patch);
    pdata->corner_neighbors[icorner] = corner_type;
    pdata->corners[icorner] = 1;
}

void fclaw2d_patch_set_missing_corner(fclaw2d_patch_t *patch,int icorner)
{
    fclaw2d_patch_data_t *pdata = fclaw2d_patch_get_user_data(patch);
    pdata->corners[icorner] = 0;
}

fclaw2d_patch_relation_t fclaw2d_patch_get_face_type(fclaw2d_patch_t* patch,
                                                     int iface)
{
    fclaw2d_patch_data_t *pdata = fclaw2d_patch_get_user_data(patch);
    FCLAW_ASSERT(pdata->neighbors_set != 0);
    FCLAW_ASSERT(0 <= iface && iface < 4);
    return pdata->face_neighbors[iface];
}

fclaw2d_patch_relation_t fclaw2d_patch_get_corner_type(fclaw2d_patch_t* patch,
                                                       int icorner)
{
    fclaw2d_patch_data_t *pdata = fclaw2d_patch_get_user_data(patch);
    FCLAW_ASSERT(pdata->corners[icorner] != 0);
    FCLAW_ASSERT(pdata->neighbors_set != 0);
    return pdata->corner_neighbors[icorner];
}

int fclaw2d_patch_corner_is_missing(fclaw2d_patch_t* patch,
                                    int icorner)
{
    fclaw2d_patch_data_t *pdata = fclaw2d_patch_get_user_data(patch);
    return !pdata->corners[icorner];
}

void fclaw2d_patch_neighbors_set(fclaw2d_patch_t* patch)
{
    int iface, icorner;
    fclaw2d_patch_data_t *pdata = fclaw2d_patch_get_user_data(patch);
    FCLAW_ASSERT(pdata->neighbors_set == 0);

    pdata->has_finegrid_neighbors = 0;
    pdata->on_coarsefine_interface = 0;
    for (iface = 0; iface < 4; iface++)
    {
        fclaw2d_patch_relation_t nt;
        nt = pdata->face_neighbors[iface];
        if (nt == FCLAW2D_PATCH_HALFSIZE || (nt == FCLAW2D_PATCH_DOUBLESIZE))
        {
            pdata->on_coarsefine_interface = 1;
            if (nt == FCLAW2D_PATCH_HALFSIZE)
            {
                pdata->has_finegrid_neighbors = 1;
            }
        }
    }

    for (icorner = 0; icorner < 4; icorner++)
    {
        fclaw2d_patch_relation_t nt;
        int has_corner = pdata->corners[icorner];
        if (has_corner)
        {
            nt = pdata->corner_neighbors[icorner];
            if ((nt == FCLAW2D_PATCH_HALFSIZE) || (nt == FCLAW2D_PATCH_DOUBLESIZE))
            {
                pdata->on_coarsefine_interface = 1;
                if (nt == FCLAW2D_PATCH_HALFSIZE)
                {
                    pdata->has_finegrid_neighbors = 1;
                }
            }
        }
    }
    pdata->neighbors_set = 1;
}

void fclaw2d_patch_neighbors_reset(fclaw2d_patch_t* patch)
{
    fclaw2d_patch_data_t *pdata = fclaw2d_patch_get_user_data(patch);
    pdata->neighbors_set = 0;
}

int fclaw2d_patch_neighbor_type_set(fclaw2d_patch_t* patch)
{
    fclaw2d_patch_data_t *pdata = fclaw2d_patch_get_user_data(patch);
    return pdata->neighbors_set;
}


int fclaw2d_patch_has_finegrid_neighbors(fclaw2d_patch_t *patch)
{
    fclaw2d_patch_data_t *pdata = fclaw2d_patch_get_user_data(patch);
    return pdata->has_finegrid_neighbors;
}

int fclaw2d_patch_on_coarsefine_interface(fclaw2d_patch_t *patch)
{
    fclaw2d_patch_data_t *pdata = fclaw2d_patch_get_user_data(patch);
    return pdata->on_coarsefine_interface;
}


void fclaw2d_patch_get_info(fclaw2d_domain_t * domain,
                            fclaw2d_patch_t * this_patch,
                            int this_block_idx, int this_patch_idx,
                            int *global_num, int *level)

{
    
  fclaw2d_block_t *this_block = &domain->blocks[this_block_idx];

  *global_num = domain->global_num_patches_before +
        (this_block->num_patches_before + this_patch_idx);

  *level = this_patch->level;

}


int
fclaw2d_patch_on_parallel_boundary (const fclaw2d_patch_t * patch)
{
    return patch->flags & FCLAW2D_PATCH_ON_PARALLEL_BOUNDARY ? 1 : 0;
}

int* fclaw2d_patch_block_corner_count(fclaw2d_global_t* glob,
                                      fclaw2d_patch_t* this_patch)
{
    fclaw2d_patch_data_t *pdata = fclaw2d_patch_get_user_data(this_patch);
    return pdata->block_corner_count;
}

void fclaw2d_patch_set_block_corner_count(fclaw2d_global_t* glob,
                                          fclaw2d_patch_t* this_patch,
                                          int icorner, int block_corner_count)
{
    fclaw2d_patch_data_t *pdata = fclaw2d_patch_get_user_data(this_patch);
    pdata->block_corner_count[icorner] = block_corner_count;
}


/* -------------------------------------------------------
   Relies on ClawPatch
   To get the links to ClawPatch out, we need to set up
   several virtual functions so that "patch" knows how to
   interpolate, average, etc.  Right now, only ClawPatch knows
   how to do this.
   -------------------------------------------------------- */

void fclaw2d_patch_data_new(fclaw2d_global_t* glob,
                            fclaw2d_patch_t* this_patch)
{
    FCLAW_ASSERT(patch_vt()->patch_new != NULL);
    fclaw2d_domain_data_t *ddata = fclaw2d_domain_get_data(glob->domain);

    /* Initialize user data */
    fclaw2d_patch_data_t *pdata = FCLAW2D_ALLOC(fclaw2d_patch_data_t, 1);
    this_patch->user = (void *) pdata;

    /* create new user data */
    FCLAW_ASSERT(patch_vt()->patch_new != NULL);
    pdata->user_patch = patch_vt()->patch_new();
    ++ddata->count_set_patch; //this is now in cb_fclaw2d_regrid_repopulate 
    pdata->neighbors_set = 0;
}

void fclaw2d_patch_data_delete(fclaw2d_global_t *glob,
                               fclaw2d_patch_t *this_patch)
{
    FCLAW_ASSERT(patch_vt()->patch_delete != NULL);
    fclaw2d_patch_data_t *pdata = (fclaw2d_patch_data_t*) this_patch->user;

    if (pdata != NULL)
    {
        fclaw2d_domain_data_t *ddata = fclaw2d_domain_get_data(glob->domain);
        patch_vt()->patch_delete(pdata->user_patch);
        ++ddata->count_delete_patch;

        FCLAW2D_FREE(pdata);
        this_patch->user = NULL;
    }
}

void fclaw2d_patch_initialize(fclaw2d_global_t *glob,
                              fclaw2d_patch_t *this_patch,
                              int this_block_idx,
                              int this_patch_idx)
{
    FCLAW_ASSERT(patch_vt()->initialize != NULL);
    patch_vt()->initialize(glob,this_patch,this_block_idx,this_patch_idx);
}

void fclaw2d_patch_pack_local_ghost(fclaw2d_global_t *glob,
                                    fclaw2d_patch_t *this_patch,
                                    double *patch_data,
                                    int time_interp)
{
    FCLAW_ASSERT(patch_vt()->ghost_pack != NULL);
    patch_vt()->ghost_pack(glob,
                           this_patch,
                           patch_data,
                           time_interp);
}

void fclaw2d_patch_build_remote_ghost(fclaw2d_global_t *glob,
                                      fclaw2d_patch_t *this_patch,
                                      int blockno,
                                      int patchno,
                                      void *user)
{

    FCLAW_ASSERT(patch_vt()->build_ghost != NULL);
    patch_vt()->build_ghost(glob,this_patch,blockno,
                            patchno,(void*) user);
    if (patch_vt()->setup_ghost != NULL)
    {
        patch_vt()->setup_ghost(glob,this_patch,blockno,patchno);
    }
}

void fclaw2d_patch_unpack_remote_ghost(fclaw2d_global_t* glob,
                                       fclaw2d_patch_t* this_patch,
                                       int this_block_idx,
                                       int this_patch_idx,
                                       double *qdata, int time_interp)
{
    FCLAW_ASSERT(patch_vt()->ghost_unpack != NULL);
    patch_vt()->ghost_unpack(glob, this_patch, this_block_idx,
                             this_patch_idx, qdata, time_interp);
}

size_t fclaw2d_patch_ghost_packsize(fclaw2d_global_t* glob)
{
    FCLAW_ASSERT(patch_vt()->ghost_packsize != NULL);
    return patch_vt()->ghost_packsize(glob);
}

void fclaw2d_patch_alloc_local_ghost(fclaw2d_global_t* glob,
                                     fclaw2d_patch_t* this_patch,
                                     void** q)
{
    FCLAW_ASSERT(patch_vt()->local_ghost_alloc != NULL);
    patch_vt()->local_ghost_alloc(glob, this_patch, q);
}

void fclaw2d_patch_free_local_ghost(fclaw2d_global_t* glob,
                                    void **q)
{
    FCLAW_ASSERT(patch_vt()->local_ghost_free != NULL);
    patch_vt()->local_ghost_free(glob, q);
}

void cb_fclaw2d_patch_partition_pack(fclaw2d_domain_t *domain,
                                     fclaw2d_patch_t *this_patch,
                                     int this_block_idx,
                                     int this_patch_idx,
                                     void *user)
{
    fclaw2d_global_iterate_t *g = (fclaw2d_global_iterate_t*) user;
    FCLAW_ASSERT(patch_vt()->partition_pack != NULL);
    patch_vt()->partition_pack(g->glob,
                               this_patch,
                               this_block_idx,
                               this_patch_idx,
                               g->user);
}



double fclaw2d_patch_single_step_update(fclaw2d_global_t *glob,
                                        fclaw2d_patch_t *this_patch,
                                        int this_block_idx,
                                        int this_patch_idx,
                                        double t,
                                        double dt)
{
    FCLAW_ASSERT(patch_vt()->single_step_update != NULL);

    double maxcfl = patch_vt()->single_step_update(glob,this_patch,this_block_idx,
                                                   this_patch_idx,t,dt);
    return maxcfl;
}


void fclaw2d_patch_partition_unpack(fclaw2d_global_t *glob,
                                    fclaw2d_domain_t *new_domain,
                                    fclaw2d_patch_t *this_patch,
                                    int this_block_idx,
                                    int this_patch_idx,
                                    void *user)
{
    fclaw2d_domain_data_t *ddata_old = fclaw2d_domain_get_data (glob->domain);
    fclaw2d_domain_data_t *ddata_new = fclaw2d_domain_get_data (new_domain);

    /* Create new data in 'user' pointer */
    fclaw2d_patch_data_new(glob,this_patch);
    /* Reason for the following two lines: the glob contains the old domain which is incremented in ddata_old 
       but we really want to increment the new domain. This will be fixed! */
    --ddata_old->count_set_patch;
    ++ddata_new->count_set_patch;

    fclaw2d_build_mode_t build_mode = FCLAW2D_BUILD_FOR_UPDATE;

    fclaw2d_patch_build(glob,this_patch,this_block_idx,
                        this_patch_idx,(void*) &build_mode);
    /* This copied q data from memory */
    FCLAW_ASSERT(patch_vt()->partition_pack != NULL);

    patch_vt()->partition_unpack(glob,
                                 new_domain,
                                 this_patch,
                                 this_block_idx,
                                 this_patch_idx,
                                 user);
}

size_t fclaw2d_patch_partition_packsize(fclaw2d_global_t* glob)
{
    FCLAW_ASSERT(patch_vt()->partition_packsize != NULL);
    return patch_vt()->partition_packsize(glob);
}

void fclaw2d_patch_build(fclaw2d_global_t *glob,
                         fclaw2d_patch_t *this_patch,
                         int blockno,
                         int patchno,
                         void *user)
{
    FCLAW_ASSERT(patch_vt()->build != NULL);
    patch_vt()->build(glob,
                      this_patch,
                      blockno,
                      patchno,
                      user);


    if (patch_vt()->setup != NULL)
    {
        /* The setup routine should check to see if this is a ghost patch and
           optimize accordingly.  For example, interior data is not generally
           needed (beyond what is needed for averaging) */
        patch_vt()->setup(glob,this_patch,blockno,patchno);
    }
}

void fclaw2d_patch_build_from_fine(fclaw2d_global_t *glob,
                                   fclaw2d_patch_t *fine_patches,
                                   fclaw2d_patch_t *coarse_patch,
                                   int blockno,
                                   int coarse_patchno,
                                   int fine0_patchno,
                                   fclaw2d_build_mode_t build_mode)
{
    FCLAW_ASSERT(patch_vt()->build_from_fine != NULL);
    patch_vt()->build_from_fine(glob,
                                fine_patches,
                                coarse_patch,
                                blockno,
                                coarse_patchno,
                                fine0_patchno,
                                build_mode);

    if (patch_vt()->setup != NULL && build_mode == FCLAW2D_BUILD_FOR_UPDATE)
    {
        /* We might want to distinguish between new fine grid patches, and
           new coarse grid patches.  In the latter case, we might just average
           aux array info, for example, rather than recreate it from scratch.
           Something like a general "build from fine" routine might be needed */
        patch_vt()->setup(glob,coarse_patch,blockno,coarse_patchno);
    }
}

void fclaw2d_patch_restore_step(fclaw2d_global_t* glob,
                                fclaw2d_patch_t* this_patch)
{
    FCLAW_ASSERT(patch_vt()->restore_step != NULL);

    patch_vt()->restore_step(glob, this_patch);
}

/* This is called from libraries routines (clawpack4.6, clawpack5, etc) */
void fclaw2d_patch_save_step(fclaw2d_global_t* glob,
                             fclaw2d_patch_t* this_patch)
{
    FCLAW_ASSERT(patch_vt()->save_step != NULL);
    patch_vt()->save_step(glob, this_patch);
}

void fclaw2d_patch_interpolate_face(fclaw2d_global_t* glob,
                                    fclaw2d_patch_t *coarse_patch,
                                    fclaw2d_patch_t *fine_patch,
                                    int idir,
                                    int iside,
                                    int RefineFactor,
                                    int refratio,
                                    int time_interp,
                                    int igrid,
                                    fclaw2d_transform_data_t* transform_data)
{
    FCLAW_ASSERT(patch_vt()->interpolate_face != NULL);
    patch_vt()->interpolate_face(glob,coarse_patch,fine_patch,idir,
                                 iside,RefineFactor,refratio,
                                 time_interp,igrid,transform_data);
}

void fclaw2d_patch_average_face(fclaw2d_global_t* glob,
                                fclaw2d_patch_t *coarse_patch,
                                fclaw2d_patch_t *fine_patch,
                                int idir,
                                int iface_coarse,
                                int RefineFactor,
                                int refratio,
                                int time_interp,
                                int igrid,
                                fclaw2d_transform_data_t* transform_data)
{
    FCLAW_ASSERT(patch_vt()->average_face != NULL);

    patch_vt()->average_face(glob,coarse_patch,fine_patch,idir,
                          iface_coarse,RefineFactor,
                          refratio,time_interp,igrid,
                          transform_data);
}

void fclaw2d_patch_copy_face(fclaw2d_global_t* glob,
                             fclaw2d_patch_t *this_patch,
                             fclaw2d_patch_t *neighbor_patch,
                             int iface,
                             int time_interp,
                             fclaw2d_transform_data_t *transform_data)
{
    FCLAW_ASSERT(patch_vt()->copy_face != NULL);

    patch_vt()->copy_face(glob,this_patch,neighbor_patch,iface,
                       time_interp,transform_data);
}

void fclaw2d_patch_copy_corner(fclaw2d_global_t* glob,
                               fclaw2d_patch_t *this_patch,
                               fclaw2d_patch_t *corner_patch,
                               int icorner,
                               int time_interp,
                               fclaw2d_transform_data_t *transform_data)
{
    FCLAW_ASSERT(patch_vt()->copy_corner != NULL);

    patch_vt()->copy_corner(glob,this_patch,corner_patch,
                            icorner,time_interp,transform_data);
}

void fclaw2d_patch_average_corner(fclaw2d_global_t* glob,
                                  fclaw2d_patch_t *coarse_patch,
                                  fclaw2d_patch_t *fine_patch,
                                  int coarse_corner,
                                  int refratio,
                                  int time_interp,
                                  fclaw2d_transform_data_t* transform_data)
{
    FCLAW_ASSERT(patch_vt()->average_corner != NULL);
    patch_vt()->average_corner(glob,coarse_patch,fine_patch,coarse_corner,
                               refratio,time_interp,transform_data);
}


void fclaw2d_patch_interpolate_corner(fclaw2d_global_t* glob,
                                      fclaw2d_patch_t* coarse_patch,
                                      fclaw2d_patch_t* fine_patch,
                                      int coarse_corner,
                                      int refratio,
                                      int time_interp,
                                      fclaw2d_transform_data_t* transform_data)
{
    FCLAW_ASSERT(patch_vt()->interpolate_corner != NULL);
    patch_vt()->interpolate_corner(glob,coarse_patch,fine_patch,
                                coarse_corner,refratio,time_interp,
                                transform_data);
}

int fclaw2d_patch_tag4refinement(fclaw2d_global_t *glob,
                                      fclaw2d_patch_t *this_patch,
                                      int blockno, int patchno,
                                      int initflag)
{
    FCLAW_ASSERT(patch_vt()->tag4refinement != NULL);

    return patch_vt()->tag4refinement(glob,this_patch,blockno,
                                      patchno, initflag);
}

int fclaw2d_patch_tag4coarsening(fclaw2d_global_t *glob,
                                      fclaw2d_patch_t *fine_patches,
                                      int blockno,
                                      int patchno)
{
    FCLAW_ASSERT(patch_vt()->tag4coarsening != NULL);

    return patch_vt()->tag4coarsening(glob,fine_patches,
                                      blockno, patchno);
}

void fclaw2d_patch_interpolate2fine(fclaw2d_global_t* glob,
                                    fclaw2d_patch_t* coarse_patch,
                                    fclaw2d_patch_t* fine_patches,
                                    int this_blockno, int coarse_patchno,
                                    int fine0_patchno)
{
    FCLAW_ASSERT(patch_vt()->interpolate2fine != NULL);

    patch_vt()->interpolate2fine(glob,coarse_patch,fine_patches,
                                 this_blockno,coarse_patchno,
                                 fine0_patchno);
}

void fclaw2d_patch_average2coarse(fclaw2d_global_t *glob,
                                  fclaw2d_patch_t *fine_patches,
                                  fclaw2d_patch_t *coarse_patch,
                                  int blockno, int fine0_patchno,
                                  int coarse_patchno)

{
    FCLAW_ASSERT(patch_vt()->average2coarse != NULL);

    patch_vt()->average2coarse(glob,fine_patches,coarse_patch,
                               blockno,fine0_patchno,coarse_patchno);
}


void fclaw2d_patch_setup_timeinterp(fclaw2d_global_t *glob,
                                    fclaw2d_patch_t *this_patch,
                                    double alpha)
{
    fclaw2d_patch_vtable_t *vt = patch_vt();
    FCLAW_ASSERT(vt->setup_timeinterp != NULL);

    vt->setup_timeinterp(glob,this_patch,alpha);
}


#if 0
void fclaw2d_patch_write_file(fclaw2d_global_t *glob,
                              fclaw2d_patch_t *this_patch,
                              int this_block_idx,
                              int this_patch_idx,
                              int iframe,int patch_num,
                              int level)
{
    FCLAW_ASSERT(patch_vt()->write_file != NULL);

    patch_vt()->write_file(glob,this_patch,this_block_idx,
                           this_patch_idx,iframe,patch_num,level);
}

void fclaw2d_patch_write_header(fclaw2d_global_t* glob,
                                int iframe)
{
    FCLAW_ASSERT(patch_vt()->write_header != NULL);
    patch_vt()->write_header(glob,iframe);
}
#endif
  

void fclaw2d_patch_physical_bc(fclaw2d_global_t *glob,
                               fclaw2d_patch_t *this_patch,
                               int this_block_idx,
                               int this_patch_idx,
                               double t,
                               double dt,
                               int *intersects_bc,
                               int time_interp)
{
    FCLAW_ASSERT(patch_vt()->physical_bc != NULL);
    patch_vt()->physical_bc(glob,this_patch,this_block_idx,this_patch_idx,
                            t,dt,intersects_bc,time_interp);
}
