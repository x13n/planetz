
void main(void)
{
	// upper right
	gl_Position    = vec4( 1 , 1 , 0 , 1 );
	gl_TexCoord[0] = vec4( 1 , 1 , 0 , 0 );
	EmitVertex();
	// upper left
	gl_Position    = vec4(-1 , 1 , 0 , 1 );
	gl_TexCoord[0] = vec4( 0 , 1 , 0 , 0 );
	EmitVertex();
	// lower right
	gl_Position    = vec4( 1 ,-1 , 0 , 1 );
	gl_TexCoord[0] = vec4( 1 , 0 , 0 , 0 );
	EmitVertex();
	// lower left
	gl_Position    = vec4(-1 ,-1 , 0 , 1 );
	gl_TexCoord[0] = vec4( 0 , 0 , 0 , 0 );
	EmitVertex();
	EndPrimitive();
}

