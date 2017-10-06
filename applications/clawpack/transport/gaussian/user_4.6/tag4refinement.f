      subroutine clawpack46_tag4refinement(mx,my,mbc,
     &      meqn, xlower,ylower,dx,dy,blockno,
     &      q, tag_threshold, init_flag,tag_patch)
      implicit none

      integer mx,my, mbc, meqn, tag_patch, init_flag
      integer blockno
      double precision xlower, ylower, dx, dy
      double precision tag_threshold
      double precision q(1-mbc:mx+mbc,1-mbc:my+mbc,meqn)

      integer i,j, mq
      double precision qmin, qmax, dq1, dq2,dq

      tag_patch = 0

c     # We check only internal cells, because there is a problem with the
c     # corner values on the cubed sphere. Possible bad values in (unused)
c     # corners at block corners are triggering refinement where three blocks meet.
      mq = 1
      do j = 1,my
         do i = 1,mx
            dq1 = abs(q(i+1,j,mq) - q(i-1,j,mq))
            dq2 = abs(q(i,j+1,mq) - q(i,j-1,mq))
            dq = max(dq1,dq2)
            if (dq .gt. tag_threshold) then
               tag_patch = 1
               return
            endif
         enddo
      enddo

      end
