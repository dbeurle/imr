meshSize  = 0.5;
x_origin  =  0;
y_origin  =  0;
factor = 1;
mesh_factor = 0.1;
//*********************************************************************************************

// create points
Point(2)  = {factor * 0   	        							,   factor * 0		    				,   0,  meshSize};
Point(3)  = {factor * 3.9	 			    			    	,   factor * 0					 	    ,   0,  mesh_factor*meshSize};
Point(4)  = {factor * 4	 			    			    	,   factor * 0.4					 	    ,   0,  mesh_factor*meshSize};

Point(5)  = {factor * 4						        			,   factor * 2					        ,   0,  mesh_factor*meshSize};
Point(6)  = {factor * 0						        			,   factor * 2					        ,   0,  meshSize};

Line(2) = {2, 3};
Line(3) = {3, 4};
Line(4) = {4, 5};
Line(5) = {5, 6};
Line(6) = {6, 2};

Line Loop(1) = {2:6};

Plane Surface(1) = {1};

Rotate {{0, 1, 0}, {factor * 4, 0, 0}, Pi} {
  Duplicata { Surface{1}; }
}

Recombine Surface {1,7};
Physical Surface(999) = {1,-7};
