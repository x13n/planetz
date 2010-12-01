#version 130 
#extension GL_EXT_geometry_shader4 : enable

in float radiuses[];
in vec4  maters1[];
in vec4  maters2[];

out vec3 pos;
out mat3 rot;
out float radius;
out vec4 mater1;
out vec4 mater2;

out float phi;
out float lambda;

void main()
{
	radius = radiuses[0];
	mater1 = maters1 [0];
	mater2 = maters2 [0];
	pos    = gl_PositionIn[0].xyz;

	float lenx = length( gl_PositionIn[0].xz );
	float cosx = gl_PositionIn[0].z / lenx;
	float sinx = gl_PositionIn[0].x / lenx;

	lambda = asin( sinx );

	mat3 rotx  = mat3( cosx ,  0   ,-sinx ,
	                    0   ,  1   ,  0   ,
			   sinx ,  0   , cosx );

	float leny = length( gl_PositionIn[0].yz );
	float cosy = gl_PositionIn[0].z / leny;
	float siny = gl_PositionIn[0].y / leny;

	phi    = asin( siny );

	mat3 roty  = mat3(  1   ,  0   ,  0   ,
	                    0   , cosy ,-siny ,
			    0   , siny , cosy );

//	mat3 rotyp = mat3(  1   ,  0   ,  0   ,
//	                    0   , cosy , siny ,
//			    0   ,-siny , cosy );

	vec4 u = vec4( roty * vec3(0,radius,0) , 0 );
	vec4 r = vec4( rotx * vec3(radius,0,0) , 0 );

//	rot = rotx * rotyp;

	// upper right
	gl_Position    = gl_PositionIn[0] + u + r;
	gl_Position    = gl_ProjectionMatrix * gl_Position;
	gl_TexCoord[0] = vec4( 1 , 1 , 0 , 0 );
	EmitVertex();
	// upper left
	gl_Position    = gl_PositionIn[0] + u - r;
	gl_Position    = gl_ProjectionMatrix * gl_Position;
	gl_TexCoord[0] = vec4( 0 , 1 , 0 , 0 );
	EmitVertex();
	// lower right
	gl_Position    = gl_PositionIn[0] - u + r;
	gl_Position    = gl_ProjectionMatrix * gl_Position;
	gl_TexCoord[0] = vec4( 1 , 0 , 0 , 0 );
	EmitVertex();
	// lower left
	gl_Position    = gl_PositionIn[0] - u - r;
	gl_Position    = gl_ProjectionMatrix * gl_Position;
	gl_TexCoord[0] = vec4( 0 , 0 , 0 , 0 );
	EmitVertex();
	EndPrimitive();
}

