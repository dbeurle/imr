

num_partitions  = 3;
mesh_size       = 0.25;


// geometry
x               = 0.0;
y               = 0.0;
z               = 0.0;

l_x             = 10.0;
l_y             = 2.0;

// points
p1 = newp; Point(p1) = {x,          y,          z,    mesh_size};
p2 = newp; Point(p2) = {x + l_x,    y,          z,    mesh_size};
p3 = newp; Point(p3) = {x + l_x,    y + l_y,    z,    mesh_size};
p4 = newp; Point(p4) = {x,          y + l_y,    z,    mesh_size};

// lines
l1 = newl; Line(l1) = {p1,p2};
l2 = newl; Line(l2) = {p2,p3};
l3 = newl; Line(l3) = {p3,p4};
l4 = newl; Line(l4) = {p4,p1};

// transfinite lines
Transfinite Line{l1, l3} = 5+1;
Transfinite Line{l2, l4} = 5 +1;

// line loop
ll = newll; Line Loop(ll) = {l1,l2,l3,l4};

// plane surface
ps = news;  Plane Surface(ps) = {ll};

// transfinite surface
Transfinite Surface {ps};

// physical surface
Physical Surface("domain") = {ps};

// left boundary
Physical Line("left_boundary") = l4;

Mesh.MshFilePartitioned = 0; // 0=no, 1=yes, 2=create physicals by partition
Mesh.RecombineAll       = 0; // 0=no, 1=yes
Mesh.Partitioner        = 1; // 1=chacho, 2=metis
Mesh.ColorCarousel      = 3; // 3=by partition
