
mesh_size       = 0.1;

x               = 0.0;
y               = 0.0;
z               = 0.0;

l_x             = 1.0;
l_y             = 1.0;

p1 = newp; Point(p1) = {x,          y,          z,    mesh_size};
p2 = newp; Point(p2) = {x + l_x,    y,          z,    mesh_size};
p3 = newp; Point(p3) = {x + l_x,    y + l_y,    z,    mesh_size};
p4 = newp; Point(p4) = {x,          y + l_y,    z,    mesh_size};

l1 = newl; Line(l1) = {p1,p2};
l2 = newl; Line(l2) = {p2,p3};
l3 = newl; Line(l3) = {p3,p4};
l4 = newl; Line(l4) = {p4,p1};

ll = newll; Line Loop(ll) = {l1,l2,l3,l4};

ps = news;  Plane Surface(ps) = {ll};

Transfinite Surface {ps};

Physical Surface("domain") = {ps};

Physical Line("left_boundary") = l4;