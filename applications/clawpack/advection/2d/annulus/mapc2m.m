function [xp,yp,zp] = mapc2m(xc,yc)

global map isflat;

map = 'annulus';

R = 1;
r = 0.4;

isflat = true;
s = 0.0;
[xc1,yc1,~] = mapc2m_brick(xc,yc,s);
[xp,yp,zp] = mapc2m_annulus(xc1,yc1);

zp = zeros(size(xp));

end
