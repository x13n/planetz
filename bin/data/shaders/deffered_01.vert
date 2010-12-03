#version 130 

uniform sampler1D materialsTex;

in float radius;
in int   model;

out float radiuses;
out vec4  maters1;
out vec4  maters2;

void main()
{	
	radiuses = radius;
	gl_Position = gl_ModelViewMatrix * gl_Vertex;

	maters1 = texelFetch( materialsTex , model   , 0 );
	maters2 = texelFetch( materialsTex , model+1 , 0 );
}

