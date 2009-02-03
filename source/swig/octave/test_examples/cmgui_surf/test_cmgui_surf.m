x = -pi:2:pi;
y = -pi:2:pi;
[X,Y] = meshgrid(x,y);
Z = cos(X).*cos(Y);
cmgui_surf(x,y,X, Y, Z);

