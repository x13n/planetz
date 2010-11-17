#include "deffered_renderer.h"

#include <GL/glew.h>
#include <cmath>
#include <algorithm>

#include "gfx.h"

#include "constants.h"

using namespace GFX;

DeferRender::DeferRender( const MEM::MISC::GfxPlanetFactory * factory )
	: factory(factory)
{
}

DeferRender::~DeferRender()
{
	delete_textures();
}

void DeferRender::prepare()
{
	prPlanet.create(
		gfx->shmMgr.loadShader(GL_VERTEX_SHADER  ,
			DATA("shaders/deffered_01.vert")),
		gfx->shmMgr.loadShader(GL_FRAGMENT_SHADER,
			DATA("shaders/deffered_01.frag")),
		gfx->shmMgr.loadShader(GL_GEOMETRY_SHADER,
			DATA("shaders/deffered_01.geom")),
		GL_POINTS , GL_QUAD_STRIP );

	prLighting.create(
		gfx->shmMgr.loadShader(GL_VERTEX_SHADER  ,
			DATA("shaders/deffered_02.vert")),
		gfx->shmMgr.loadShader(GL_FRAGMENT_SHADER,
			DATA("shaders/deffered_02.frag")),
		gfx->shmMgr.loadShader(GL_GEOMETRY_SHADER,
			DATA("shaders/deffered_02.geom")),
		GL_POINTS , GL_QUAD_STRIP );

	radiusId = glGetAttribLocation( prPlanet.id() , "radius" );
	modelId  = glGetAttribLocation( prPlanet.id() , "model"  );

	TODO("Dynamically change screen size ratio in shader");
	ratioId = glGetUniformLocation( prPlanet.id() , "ratio" );

	prPlanet.use();
	glUniform1i( sphereTexId , 0 );
	glUniform1f( ratioId , (float)gfx->width()/(float)gfx->height() );
	Program::none();

	gbuffId[0] = glGetUniformLocation( prLighting.id() , "gbuff1" );
	gbuffId[1] = glGetUniformLocation( prLighting.id() , "gbuff2" );

	prLighting.use();
	glUniform1i( gbuffId[0] , 0 );
	glUniform1i( gbuffId[1] , 1 );
	Program::none();

	create_textures( gfx->width() , gfx->height() );
}

void DeferRender::create_textures( unsigned int w , unsigned int h )
{
	unsigned sphereSize = pow(2,floor(log(std::max(w,h))/log(2.0)));
	sphereTexId = glGetUniformLocation( prPlanet.id() , "sph_pos" );
	sphereTex = generate_sphere_texture( sphereSize ,sphereSize );

	for( int i=0 ;i<gbuffNum ; i++ )
		gbuffTex[i] = generate_render_target_texture( w , h );

	glGenTextures( 1, &depthTex );
	glBindTexture( GL_TEXTURE_2D, depthTex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0 );

	glGenFramebuffers( 1, &fboId );
	glBindFramebuffer( GL_FRAMEBUFFER , fboId );

	bufferlist[0] = GL_COLOR_ATTACHMENT0;
	bufferlist[1] = GL_COLOR_ATTACHMENT1;

	glFramebufferTexture2D( GL_FRAMEBUFFER , GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D , gbuffTex[0] , 0 );
	glFramebufferTexture2D( GL_FRAMEBUFFER , GL_COLOR_ATTACHMENT1 , GL_TEXTURE_2D , gbuffTex[1] , 0 );
	glFramebufferTexture2D( GL_FRAMEBUFFER , GL_DEPTH_ATTACHMENT  , GL_TEXTURE_2D , depthTex  , 0 );

	glBindFramebuffer( GL_FRAMEBUFFER , 0 );

}

void DeferRender::delete_textures()
{
	glDeleteTextures(1,&sphereTex);
	glDeleteTextures(gbuffNum,gbuffTex);
	glDeleteTextures(1,&depthTex );

	glDeleteFramebuffers( 1 , &fboId );
}

void DeferRender::resize( unsigned int width , unsigned int height )
{
	delete_textures();
	create_textures( width , height );
	prPlanet.use();
	glUniform1f( ratioId , (float)width/(float)height );
	Program::none();
}

GLuint DeferRender::generate_sphere_texture( int w , int h )
{
	float*sphere = new float[ w * h * 4 ];

	int w4 = w*4;
	float w2 = (float)w/2.0f;
	float h2 = (float)h/2.0f;

	for( int wi=0 ; wi<w4 ; wi+=4 )
		for( int hi=0 ; hi<h ; hi++ )
		{
			float x = ((float)wi/4.0f - w2)/(float)w2;
			float y = ((float)hi - h2)/(float)h2;
			float a = 1 <= x*x + y*y ? 0.0 : 1.0 ;
			float z =-(!a?0.0f:std::sqrt( 1 - x*x - y*y ));

			int i = wi + hi*w4;

			sphere[ i     ] = x;
			sphere[ i + 1 ] = y;
			sphere[ i + 2 ] = z;
			sphere[ i + 3 ] = a;
		}

	GLuint texId;
	glGenTextures( 1 , &texId );
	glBindTexture(GL_TEXTURE_2D, texId );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S    , GL_CLAMP  );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA16F,w,h,0,GL_RGBA,GL_FLOAT,sphere);
	delete[]sphere;
	return texId;
}

GLuint DeferRender::generate_render_target_texture( int w , int h )
{
	GLuint texId;
	glGenTextures( 1 , &texId );
	glBindTexture(GL_TEXTURE_2D, texId );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S    , GL_CLAMP  );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA16F,w,h,0,GL_RGBA,GL_FLOAT,NULL);
	return texId;
}

void DeferRender::draw() const
{
	glAlphaFunc( GL_GREATER, 0.1 );
	glEnable( GL_ALPHA_TEST );

	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableVertexAttribArray( radiusId );
	glEnableVertexAttribArray( modelId  );

	factory->getPositions().bind();
	glVertexPointer( 3 , GL_FLOAT , 0 , NULL );
	factory->getPositions().unbind();

	factory->getRadiuses().bind();
	glVertexAttribPointer( radiusId , 1, GL_FLOAT, GL_FALSE, 0, NULL );
	factory->getRadiuses().unbind();

	factory->getModels().bind();
	glVertexAttribPointer( modelId  , 1, GL_FLOAT , GL_FALSE, 0, NULL );
	factory->getModels().unbind();

	prPlanet.use();
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D , sphereTex );

	glBindFramebuffer( GL_FRAMEBUFFER , fboId );

	glDrawBuffers( gbuffNum , bufferlist );

	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDrawArrays( GL_POINTS , 0 , factory->getPositions().getLen() );

	Program::none();
	glBindFramebuffer( GL_FRAMEBUFFER , 0 );
	glDisableVertexAttribArray( radiusId );
	glDisableVertexAttribArray( modelId  );

	glEnable( GL_BLEND );
	glBlendFunc( GL_ONE , GL_ONE );

	glClear( GL_DEPTH_BUFFER_BIT ); 

	prLighting.use();
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D , gbuffTex[0] );

	glActiveTexture( GL_TEXTURE1 );
	glBindTexture( GL_TEXTURE_2D , gbuffTex[1] );

//        log_printf(DBG,"cze\n");

	glBegin(GL_POINTS);
	  glVertex3f(0, 4,0);
	  glVertex3f(0,-4,0);
	  glVertex3f(-1,-4,0);
	  glVertex3f(1,-4,0);
	  glVertex3f(1,-4,10);
	  glVertex3f(10,10,10);
	  glVertex3f(-10,10,10);
	  glVertex3f(-10,15,10);
	  glVertex3f(100,75,100);
	  glVertex3f(1000,1000,1000);
	glEnd();
//        factory->getPositions().bind();
//        glVertexPointer  ( 3 , GL_FLOAT , 0 , NULL );
//        factory->getPositions().unbind();

//        log_printf(DBG,"cze\n");

//        glDrawArrays( GL_POINTS , 0 , 1 ); 

//        log_printf(DBG,"cze\n");

	glBindTexture( GL_TEXTURE_2D , 0 );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D , 0 );
	Program::none();

	glDisable( GL_BLEND );
	glDisable( GL_ALPHA_TEST );

	glDisableClientState( GL_VERTEX_ARRAY );
}

