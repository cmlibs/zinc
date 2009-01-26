x = -pi:2:pi;
y = -pi:2:pi;
[X,Y] = meshgrid(x,y);
Z = cos(X).*cos(Y);

numnodes = length(x)*length(y);
fid = fopen('grid.exnode', 'w');
fprintf(fid, "Group name: grid\n")
fprintf(fid, "#Fields=2\n")
fprintf(fid, "1) coordinates, coordinate, rectangular cartesian, #Components=2\n")
fprintf(fid, " x. Value index= 1, #Derivatives= 0\n")
fprintf(fid, " y. Value index= 2, #Derivatives= 0\n")
fprintf(fid, "2) general, field, rectangular cartesian, #Components=1\n")
fprintf(fid, " value.  Value index= 3, #Derivatives= 0\n")

for p = 1: numnodes;
 fprintf(fid, "Node: %d\n", p);
 fprintf(fid, "%2.1f %2.1f\n", X(p), Y(p));
 fprintf(fid, "%2.1f\n", Z(p))
end;
fclose(fid);


numelems = (length(x)*(length(y)-1)) + (length(y)*(length(x)-1));
fid = fopen('grid.exelem', 'w');
fprintf(fid, "Group name: grid\n")
fprintf(fid, "Shape. Dimension=1\n");

for q = 1: numelems;
fprintf(fid, "Element: 0 0   %d\n", q);
end;

numfaces = (length(x)-1)*(length(y)-1);

fprintf(fid, "Shape. Dimension=2\n")
fprintf(fid, "#Scale factor sets= 0\n");
fprintf(fid, "#Nodes= 4\n#Fields= 2\n")
fprintf(fid, "%d) coordinates, coordinate, rectangular cartesian, #Components=2\n",1)
fprintf(fid, "x.  l.Lagrange*l.Lagrange, no modify, standard node based.\n   #Nodes= 4\n")
Nodes = 4;
for a = 1:Nodes;
fprintf(fid, "    %d.   #Values= 1\n       Value indices:  1\n       Scale factor indices: %d\n", a, a)
end;

fprintf(fid, "y.  l.Lagrange*l.Lagrange, no modify, standard node based.\n   #Nodes= 4\n")
for b = 1:Nodes;
fprintf(fid, "    %d.   #Values= 1\n       Value indices:  1\n       Scale factor indices: %d\n", b, b)
end;
%c = 1:numfaces;

fprintf(fid, "2) general, field, rectangular cartesian, #Components=1\n")
fprintf(fid, "value.  l.Lagrange*l.Lagrange, no modify, standard node based.\n   #Nodes= 4\n")
for g = 1:Nodes;
fprintf(fid, "    %d.   #Values= 1\n       Value indices:  1\n       Scale factor indices: %d\n", g, g)
end;

fprintf(fid, " Element:     %d 0 0\n", 1)
fprintf(fid, "   Faces:\n     0  0   4\n     0  0   5\n     0  0   1\n     0  0   8\n")
fprintf(fid, "   Nodes:\n  1 2 5 6\n")
%fprintf(fid, "   Scale factors:\n       0.10000E+01   0.10000E+01   0.10000E+01   0.10000E+01\n")

fprintf(fid, " Element:     %d 0 0\n", 2)
fprintf(fid, "   Faces:\n     0  0   5\n     0  0   6\n     0  0   2\n     0  0   9\n")
fprintf(fid, "   Nodes:\n  2 3 6 7\n")
%fprintf(fid, "   Scale factors:\n       0.10000E+01   0.10000E+01   0.10000E+01   0.10000E+01\n")

fprintf(fid, " Element:     %d 0 0\n", 3)
fprintf(fid, "   Faces:\n     0  0   6\n     0  0   7\n     0  0   3\n     0  0   10\n")
fprintf(fid, "   Nodes:\n  3 4 7 8\n")
%fprintf(fid, "   Scale factors:\n       0.10000E+01   0.10000E+01   0.10000E+01   0.10000E+01\n")

fprintf(fid, " Element:     %d 0 0\n", 4)
fprintf(fid, "   Faces:\n     0  0   11\n     0  0   12\n     0  0   8\n     0  0   15\n")
fprintf(fid, "   Nodes:\n  5 6 9 10\n")
%fprintf(fid, "   Scale factors:\n       0.10000E+01   0.10000E+01   0.10000E+01   0.10000E+01\n")

fprintf(fid, " Element:     %d 0 0\n", 5)
fprintf(fid, "   Faces:\n     0  0   12\n     0  0   13\n     0  0   9\n     0  0   16\n")
fprintf(fid, "   Nodes:\n  6 7 10 11\n")
%fprintf(fid, "   Scale factors:\n       0.10000E+01   0.10000E+01   0.10000E+01   0.10000E+01\n")

fprintf(fid, " Element:     %d 0 0\n", 6)
fprintf(fid, "   Faces:\n     0  0   13\n     0  0   14\n     0  0   10\n     0  0   17\n")
fprintf(fid, "   Nodes:\n  7 8 11 12\n")
%fprintf(fid, "   Scale factors:\n       0.10000E+01   0.10000E+01   0.10000E+01   0.10000E+01\n")

fprintf(fid, " Element:     %d 0 0\n", 7)
fprintf(fid, "   Faces:\n     0  0   18\n     0  0   19\n     0  0   15\n     0  0   22\n")
fprintf(fid, "   Nodes:\n  9 10 13 14\n")
%fprintf(fid, "   Scale factors:\n       0.10000E+01   0.10000E+01   0.10000E+01   0.10000E+01\n")

fprintf(fid, " Element:     %d 0 0\n", 8)
fprintf(fid, "   Faces:\n     0  0   19\n     0  0   20\n     0  0   16\n     0  0   23\n")
fprintf(fid, "   Nodes:\n  10 11 14 15\n")
%fprintf(fid, "   Scale factors:\n       0.10000E+01   0.10000E+01   0.10000E+01   0.10000E+01\n")

fprintf(fid, " Element:     %d 0 0\n", 9)
fprintf(fid, "   Faces:\n     0  0   20\n     0  0   21\n     0  0   17\n     0  0   24\n")
fprintf(fid, "   Nodes:\n  11 12 15 16\n")
%fprintf(fid, "   Scale factors:\n       0.10000E+01   0.10000E+01   0.10000E+01   0.10000E+01\n")

fclose(fid)

