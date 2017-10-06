SUBROUTINE clawpack5_qinit(meqn,mbc,mx,my,xlower,ylower, &
     dx,dy,q,maux,aux)
  IMPLICIT NONE

  INTEGER meqn, mbc, mx, my, maux
  DOUBLE PRECISION xlower, ylower, dx, dy
  DOUBLE PRECISION q(meqn,1-mbc:mx+mbc, 1-mbc:my+mbc)
  DOUBLE PRECISION aux(maux,1-mbc:mx+mbc, 1-mbc:my+mbc)

  DOUBLE PRECISION xlow, ylow, w, xc,yc, q0
  INTEGER i,j

  INTEGER blockno, fc2d_clawpack46_get_block

  INTEGER example
  COMMON /comm_example/ example

  blockno = fc2d_clawpack46_get_block()

  DO j = 1-mbc,my+mbc
     DO i = 1-mbc,mx+mbc
        IF (example .EQ. 0) THEN
           !! # Discontinuous solution
           xlow = xlower + (i-1)*dx
           ylow = ylower + (j-1)*dy
           CALL cellave2(blockno,xlow,ylow,dx,dy,w)
           q(1,i,j) = w
        ELSEIF (example .EQ. 1) THEN
           !! # Smooth solution for computing the error
           xc = xlower + (i-0.5)*dx
           yc = ylower + (j-0.5)*dy
           q(1,i,j) = q0(blockno,xc,yc)
        ENDIF
     ENDDO
  ENDDO

  RETURN
END SUBROUTINE clawpack5_qinit
