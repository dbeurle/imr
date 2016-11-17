Function CreateRectangle

  // create lines
  p1 = newp; Point(p1) = {x,      y,       0,  meshSize};
  p2 = newp; Point(p2) = {x+lx,   y,       0,  meshSize};
  p3 = newp; Point(p3) = {x+lx,   y+ly,    0,  meshSize};
  p4 = newp; Point(p4) = {x   ,   y+ly,    0,  meshSize};

  // create lines
  l1  = newreg; Line(l1)  = {p1,p2};
  l2  = newreg; Line(l2)  = {p2,p3};
  l3  = newreg; Line(l3)  = {p3,p4};
  l4  = newreg; Line(l4)  = {p4,p1};

  Transfinite Line {l1,l3} = 900+1;
  Transfinite Line {l2,l4} = 300+1;

  loop1 = newll; Line Loop(loop1) = {l1,l2,l3,l4};

  plane1 = news; Plane Surface(plane1) = {loop1};
  Transfinite Surface {plane1};

  Physical Surface("Domain")    = plane1;
//Physical Line("Boundary")     = {l1,l2,l3,l4};

Return


meshSize       = 10;
x  = 0;
y  = 0;
lx = 60;
ly = 10;
num_partitions = 3;

// left subdomain
partition = 1;
Call CreateRectangle;

//Mesh.Partitioner              =   1;            // 1= chaco, 2=metis
//Mesh.ColorCarousel            =   3;              // color by partition
//Mesh.MshFilePartitioned       =   1;              // Create physicals by partiion
//Mesh.NbPartitions             =   num_partitions; // number of partitions
//Mesh.IgnorePartitionBoundary  =   0; // 0 = no, 1 = yes
//Mesh.RecombineAll             =   0;
//Mesh.SurfaceFaces             =   1;
