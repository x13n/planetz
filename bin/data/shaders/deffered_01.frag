#version 130 

uniform sampler2D sph_pos;

varying in vec3 pos;
varying in float radius;
varying in float model;

void main()
{
	vec4 tex = texture2D( sph_pos , gl_TexCoord[0].st );

	gl_FragData[0].xyz = pos;
//        gl_FragData[0].xy += tex.xy;     // done by interpolation geom -> fragment
	gl_FragData[0].z+= tex.z * radius; // fragment position in modelviewprojection space
	gl_FragData[0].a = tex.a;	   // draw or not draw this fragment

	gl_FragData[1].xyz = tex.xyz;      // normal vector
	gl_FragData[1].w   = model;        // model id

	gl_FragData[2].xyz = vec3(.1,.1,.1); // ambient light?
}

