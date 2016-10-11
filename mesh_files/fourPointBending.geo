meshSize       	= 4;
notchHeight    	= 10;
notchWidth     	= 2;
y_origin = 0;
x_origin = 0;
y_end = 100;
//*********************************************************************************************
// create points
Point(1)  = {x_origin       	        ,   y_origin    ,   0,  meshSize};
Point(2)  = {x_origin + 25              ,   y_origin    ,   0,  meshSize};
Point(3)  = {x_origin + 175             ,   y_origin    ,   0,  meshSize};
Point(4)  = {x_origin + 250             ,   y_origin    ,   0,  meshSize};

Point(11)  = {x_origin       	         ,   y_origin + 10    ,   0,  meshSize};
Point(12)  = {x_origin + 25              ,   y_origin + 10    ,   0,  meshSize};
Point(13)  = {x_origin + 175             ,   y_origin + 10    ,   0,  meshSize};
Point(14)  = {x_origin + 250             ,   y_origin + 10    ,   0,  meshSize};

Point(21)  = {x_origin       	         ,   y_end    ,   0,  meshSize};
Point(22)  = {x_origin + 25              ,   y_end    ,   0,  meshSize};
Point(23)  = {x_origin + 175             ,   y_end    ,   0,  meshSize};
Point(24)  = {x_origin + 250             ,   y_end    ,   0,  meshSize};


// create lines
Line(1)  	= {1,2};
Line(2)  	= {2,3};
Line(3)  	= {3,4};

Line(11)  	= {11,12};
Line(12)   	= {12,13};
Line(13)   	= {13,14};

Line(21)  	= {21,22};
Line(22)   	= {22,23};
Line(23)   	= {23,24};

Line(31) = {1,11};
Line(32) = {2,12};
Line(33) = {3,13};
Line(34) = {4,14};

Line(41) = {11,21};
Line(42) = {12,22};
Line(43) = {13,23};
Line(44) = {14,24};

Line Loop(1) = {1,32,-11,-31};
Line Loop(2) = {2,33,-12,-32};
Line Loop(3) = {3,34,-13,-33};

Line Loop(11) = {11,42,-21,-41};
Line Loop(12) = {12,43,-22,-42};
Line Loop(13) = {13,44,-23,-43};

Plane Surface(1) = {1};
Plane Surface(2) = {2};
Plane Surface(3) = {3};
Plane Surface(11) = {11};
Plane Surface(12) = {12};
Plane Surface(13) = {13};
Recombine Surface {1,2,3,11,12,13};
Transfinite Surface {1,2,3,11,12,13};
Physical Surface(999) = {1,2,3,11,12,13};
