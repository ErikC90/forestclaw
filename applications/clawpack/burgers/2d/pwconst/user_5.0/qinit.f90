SUBROUTINE clawpack5_qinit(meqn,mbc,mx,my,xlower,ylower, &
     dx,dy,q,maux,aux)

  IMPLICIT NONE

  INTEGER meqn,mbc,mx,my,maux
  DOUBLE PRECISION xlower,ylower,dx,dy

  DOUBLE PRECISION q(meqn,1-mbc:mx+mbc, 1-mbc:my+mbc)
  DOUBLE PRECISION aux(maux,1-mbc:mx+mbc, 1-mbc:my+mbc)

  INTEGER i,j
  DOUBLE PRECISION xi,yj

  DO i = 1-mbc,mx+mbc
     xi = xlower + (i-0.5d0)*dx
     DO j = 1-mbc,my+mbc
        yj = ylower + (j-0.5d0)*dy
        IF (xi.GT.0.1d0 .AND. xi.LT.0.6d0 .AND. &
             yj.GT.0.1d0 .AND. yj.LT.0.6d0) THEN
           q(1,i,j) = 1.d0
        ELSE
           q(1,i,j) = 0.1d0
        ENDIF
     ENDDO
  ENDDO
  RETURN
END SUBROUTINE clawpack5_qinit
