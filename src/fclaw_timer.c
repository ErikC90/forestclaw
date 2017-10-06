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

#include <fclaw_timer.h>
#include <fclaw2d_global.h>

#include <fclaw2d_partition.h>
#include <sc_statistics.h>
#include <fclaw2d_domain.h>

#define FCLAW2D_STATS_SET(stats,glob,NAME) do {                               \
    SC_CHECK_ABORT (!(glob)->timers[FCLAW2D_TIMER_ ## NAME].running,          \
                    "Timer " #NAME " still running in fclaw2d_domain_finalize");              \
    sc_stats_set1 ((stats) + FCLAW2D_TIMER_ ## NAME,                           \
                   (glob)->timers[FCLAW2D_TIMER_ ## NAME].cumulative, #NAME); \
} while (0)

/* -----------------------------------------------------------------
   Work with timers
   ----------------------------------------------------------------- */

double
fclaw2d_timer_wtime (void)
{
    return sc_MPI_Wtime ();
}

void
fclaw2d_timer_init (fclaw2d_timer_t *timer)
{
    memset (timer, 0, sizeof (fclaw2d_timer_t));
}

void
fclaw2d_timer_start (fclaw2d_timer_t *timer)
{
    if (!timer->running) {
        timer->started = fclaw2d_timer_wtime ();
        timer->stopped = 0.;
        timer->running = 1;
    }
    else
    {
        SC_ABORT_NOT_REACHED ();
    }
}

void
fclaw2d_timer_stop (fclaw2d_timer_t *timer)
{
    if (timer->running) {
        timer->stopped = fclaw2d_timer_wtime ();
        timer->cumulative += timer->stopped - timer->started;
        timer->running = 0;
    }
    else
    {
        SC_ABORT_NOT_REACHED ();
    }
}

void
fclaw2d_timer_report(fclaw2d_global_t *glob)
{
    sc_statinfo_t stats[FCLAW2D_TIMER_COUNT];

    fclaw2d_timer_stop (&glob->timers[FCLAW2D_TIMER_WALLTIME]);

    FCLAW2D_STATS_SET (stats, glob, INIT);
    FCLAW2D_STATS_SET (stats, glob, OUTPUT);
    FCLAW2D_STATS_SET (stats, glob, DIAGNOSTICS);
    FCLAW2D_STATS_SET (stats, glob, REGRID);
    FCLAW2D_STATS_SET (stats, glob, ADVANCE);
    FCLAW2D_STATS_SET (stats, glob, GHOSTFILL);
    FCLAW2D_STATS_SET (stats, glob, ADAPT_COMM);
    FCLAW2D_STATS_SET (stats, glob, PARTITION_COMM);
    FCLAW2D_STATS_SET (stats, glob, GHOSTPATCH_COMM);
    FCLAW2D_STATS_SET (stats, glob, DIAGNOSTICS_COMM);
    FCLAW2D_STATS_SET (stats, glob, CFL_COMM);
    FCLAW2D_STATS_SET (stats, glob, WALLTIME);
    FCLAW2D_STATS_SET (stats, glob, REGRID_BUILD);
    FCLAW2D_STATS_SET (stats, glob, REGRID_TAGGING);
    FCLAW2D_STATS_SET (stats, glob, PARTITION);
    FCLAW2D_STATS_SET (stats, glob, PARTITION_BUILD);
    FCLAW2D_STATS_SET (stats, glob, GHOSTPATCH_BUILD);
    FCLAW2D_STATS_SET (stats, glob, GHOSTFILL_COPY);
    FCLAW2D_STATS_SET (stats, glob, GHOSTFILL_AVERAGE);
    FCLAW2D_STATS_SET (stats, glob, GHOSTFILL_INTERP);
    FCLAW2D_STATS_SET (stats, glob, GHOSTFILL_PHYSBC);
    FCLAW2D_STATS_SET (stats, glob, GHOSTFILL_STEP1);
    FCLAW2D_STATS_SET (stats, glob, GHOSTFILL_STEP2);
    FCLAW2D_STATS_SET (stats, glob, GHOSTFILL_STEP3);
    FCLAW2D_STATS_SET (stats, glob, NEIGHBOR_SEARCH);
    FCLAW2D_STATS_SET (stats, glob, EXTRA1);
    FCLAW2D_STATS_SET (stats, glob, EXTRA2);
    FCLAW2D_STATS_SET (stats, glob, EXTRA3);
    FCLAW2D_STATS_SET (stats, glob, EXTRA4);

    int d = glob->count_grids_per_proc;
    glob->count_grids_per_proc = (d > 0) ? d : 1;   /* To avoid division by zero */
    d = glob->count_amr_advance;
    glob->count_amr_advance = (d > 0) ? d : 1;   /* To avoid division by zero */

    double gpp = glob->count_grids_per_proc/       glob->count_amr_advance;
    double glb = glob->count_grids_local_boundary/ glob->count_amr_advance;
    double grb = glob->count_grids_remote_boundary/glob->count_amr_advance;
    double gint = gpp - glb;

    /* compute arithmetic mean of total advance steps per processor */
    sc_stats_set1 (&stats[FCLAW2D_TIMER_ADVANCE_STEPS_COUNTER],
                   glob->count_single_step,"ADVANCE_STEPS_COUNTER");

    /* Compute the inverse harmonic mean of total advance steps per processor.  */
    int c = glob->count_single_step;
    glob->count_single_step = (c > 0) ? c : 1;   /* To avoid division by 0 */
    sc_stats_set1 (&stats[FCLAW2D_TIMER_ADVANCE_STEPS_INV_HMEAN],
                   1.0/glob->count_single_step,"ADVANCE_STEPS_INV_HMEAN");

    /* Compute the arithmetic mean of grids per processor */
    sc_stats_set1 (&stats[FCLAW2D_TIMER_GRIDS_PER_PROC],gpp,"GRIDS_PER_PROC");

    /* Compute the inverse harmonic mean of grids per processor  */
    gpp = (gpp == 0) ? 1 : gpp; 
    sc_stats_set1 (&stats[FCLAW2D_TIMER_GRIDS_PER_PROC_INV_HMEAN],1.0/gpp,
                   "GRIDS_PER_PROC_INV_HMEAN");

    /* Compute the arithmetic mean of grids per processor */
    sc_stats_set1 (&stats[FCLAW2D_TIMER_GRIDS_LOCAL_BOUNDARY],glb,
                   "GRIDS_LOCAL_BOUNDARY");

    /* Compute the arithmetic mean of grids per processor */
    sc_stats_set1 (&stats[FCLAW2D_TIMER_GRIDS_INTERIOR],gint,
                   "GRIDS_INTERIOR");

    /* Ratio of grids on the local boundary to grids per proc */
    glb = glb == 0 ? 1 : glb; /* This might happen in the 1-proc case. */
    sc_stats_set1 (&stats[FCLAW2D_TIMER_GRIDS_LOCAL_BOUNDARY_RATIO],gint/glb,
                    "GRIDS_LOCAL_BOUNDARY_RATIO");

    /* Compute the arithmetic mean of grids per processor */
    sc_stats_set1 (&stats[FCLAW2D_TIMER_GRIDS_REMOTE_BOUNDARY],grb,
                   "GRIDS_REMOTE_BOUNDARY");

    /* Compute the arithmetic mean of grids per processor */
    /* We might have zero processors on the remote boundary */
    grb = grb == 0 ? 1 : grb;
    sc_stats_set1 (&stats[FCLAW2D_TIMER_GRIDS_REMOTE_BOUNDARY_RATIO],gint/grb,
                   "GRIDS_REMOTE_BOUNDARY_RATIO");

    sc_stats_set1 (&stats[FCLAW2D_TIMER_UNACCOUNTED],
                   glob->timers[FCLAW2D_TIMER_WALLTIME].cumulative -
                   (glob->timers[FCLAW2D_TIMER_INIT].cumulative +
                    glob->timers[FCLAW2D_TIMER_REGRID].cumulative +
                    glob->timers[FCLAW2D_TIMER_OUTPUT].cumulative +
                    glob->timers[FCLAW2D_TIMER_DIAGNOSTICS].cumulative +
                    glob->timers[FCLAW2D_TIMER_ADVANCE].cumulative +
                    glob->timers[FCLAW2D_TIMER_GHOSTFILL].cumulative +
                    glob->timers[FCLAW2D_TIMER_ADAPT_COMM].cumulative +
                    glob->timers[FCLAW2D_TIMER_GHOSTPATCH_COMM].cumulative +
                    glob->timers[FCLAW2D_TIMER_PARTITION_COMM].cumulative +
                    glob->timers[FCLAW2D_TIMER_DIAGNOSTICS_COMM].cumulative +
                    glob->timers[FCLAW2D_TIMER_CFL_COMM].cumulative),
                   "UNACCOUNTED");

    sc_stats_set1 (&stats[FCLAW2D_TIMER_COMM],
                   glob->timers[FCLAW2D_TIMER_ADAPT_COMM].cumulative +
                   glob->timers[FCLAW2D_TIMER_GHOSTPATCH_COMM].cumulative +
                   glob->timers[FCLAW2D_TIMER_PARTITION_COMM].cumulative +
                   glob->timers[FCLAW2D_TIMER_DIAGNOSTICS_COMM].cumulative +
                   glob->timers[FCLAW2D_TIMER_CFL_COMM].cumulative,
                   "FCLAW2D_TIMER_COMM");

    /* Just subtracting FCLAW2D_TIMER_COMM here doesn't work ... */
    sc_stats_set1 (&stats[FCLAW2D_TIMER_LOCAL],
                   glob->timers[FCLAW2D_TIMER_WALLTIME].cumulative -
                   (glob->timers[FCLAW2D_TIMER_ADAPT_COMM].cumulative +
                   glob->timers[FCLAW2D_TIMER_GHOSTPATCH_COMM].cumulative +
                   glob->timers[FCLAW2D_TIMER_PARTITION_COMM].cumulative +
                   glob->timers[FCLAW2D_TIMER_DIAGNOSTICS_COMM].cumulative +
                    glob->timers[FCLAW2D_TIMER_CFL_COMM].cumulative),
                   "FCLAW2D_TIMER_LOCAL");


    sc_stats_compute (glob->mpicomm, FCLAW2D_TIMER_COUNT, stats);

    sc_stats_print (sc_package_id, SC_LP_ESSENTIAL, FCLAW2D_TIMER_COUNT,
                    stats, 1, 0);

    SC_GLOBAL_ESSENTIALF ("Procs %d advance %d %g exchange %d %g "
                          "regrid %d %d %g\n", glob->mpisize,
                          glob->count_amr_advance,
                          stats[FCLAW2D_TIMER_ADVANCE].average,
                          glob->count_ghost_exchange,
                          stats[FCLAW2D_TIMER_GHOSTFILL].average,
                          glob->count_amr_regrid,
                          glob->count_amr_new_domain,
                          stats[FCLAW2D_TIMER_REGRID].average);

    SC_GLOBAL_ESSENTIALF ("Max/P %d advance %d %g exchange %d %g "
                          "regrid %d %d %g\n", glob->mpisize,
                          glob->count_amr_advance,
                          stats[FCLAW2D_TIMER_ADVANCE].max,
                          glob->count_ghost_exchange,
                          stats[FCLAW2D_TIMER_GHOSTFILL].max,
                          glob->count_amr_regrid,
                          glob->count_amr_new_domain,
                          stats[FCLAW2D_TIMER_REGRID].max);

#if 0
    /* Find out process rank */
    /* TODO : Fix this so that it doesn't interfere with output printed above. */
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Barrier(MPI_COMM_WORLD);

    /* Write out individual processor timers */
    printf("%12s time on proc %d : %12.4f\n","ADVANCE",
           domain->mpirank,glob->timers[FCLAW2D_TIMER_ADVANCE].cumulative);
    printf("%12s time on proc %d : %12.4f\n","GHOSTCOMM",
           domain->mpirank,glob->timers[FCLAW2D_TIMER_GHOSTCOMM].cumulative);
    printf("%12s time on proc %d : %12.4f\n","EXCHANGE",
           domain->mpirank,glob->timers[FCLAW2D_TIMER_EXCHANGE].cumulative);
    printf("%12s time on proc %d : %12.4f\n","REGRID",
           domain->mpirank,glob->timers[FCLAW2D_TIMER_REGRID].cumulative);
    printf("\n");
#endif


}
