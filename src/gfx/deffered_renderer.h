#ifndef __DEFFERED_RENDERER_H__

#define __DEFFERED_RENDERER_H__

#include "gfx.h"
#include "shader.h"
#include "drawable.h"
#include "mem/misc/buffer.h"
#include "mem/misc/gfx_planet_factory.h"

namespace GFX
{
class DeferRender : public Drawable {
	enum OPT {
		LIGHTING    = 1 << 0,
		LIGHT_PLANES= 1 << 1,
		TEXTURES    = 1 << 2,
	};
public:
	DeferRender( const MEM::MISC::GfxPlanetFactory * factory );
	virtual ~DeferRender();

	virtual void draw() const;

	virtual void prepare();

	virtual void resize(
			unsigned int width ,
			unsigned int height );

	virtual void update_configuration();
	
	void setMaterials( GLuint );
	void setTextures ( GLuint );

	void on_camera_angle_changed( float*m );
private:
	void create_textures( unsigned int w , unsigned int h );
	void delete_textures();

	//
	// Shaders
	//
	Program prPlanet , prLighting , prLightsBase , prAtmosphere;

	//
	// Vertex data
	//
	GLint radiusId;

	GLint modelId ;
	GLint texIdId ;
	GLint modelLId ;
	GLint emissiveLId;

	//
	// Sphere normals (deprecated)
	//
	GLuint generate_sphere_texture( int w , int h );

	GLint sphereTexId;
	GLuint sphereTex;
	
	//
	// Materials
	//
	GLint materialsTexId;
	GLuint materialsTex;
	GLuint matLId;

	//
	// MTR
	//
	GLuint generate_render_target_texture( int w , int h );

	GLuint fboId[3];
	GLuint depthTex;

	static const GLsizei gbuffNum = 4;

	GLint  gbuffId   [gbuffNum*2];
	GLuint gbuffTex  [gbuffNum  ];
	GLenum bufferlist[gbuffNum  ];

	GLuint screenTex;

	//
	// Texturing
	// 
	GLuint generate_angles_texture( int w , int h );
	GLuint generate_normals_texture( int w , int h );

	GLuint anglesTex;
	GLuint normalsTex;
	GLuint texturesTex;

	GLint anglesTexId;
	GLint normalsTexId;
	GLint textureTexId;

	GLint anglesId;

	const MEM::MISC::GfxPlanetFactory * const factory;

	//
	// Atmospehere
	//
	GLuint generate_atmosphere_texture( int w , int h );

	GLint atmId;
	GLint radiusAId;
	GLuint atmTex;

	//
	// Glow
	//
	void generate_glow_planes( MEM::MISC::BufferGl<float>& buf , int num , int size );

//        MEM::MISC::BufferGl<float> planes;

	static const int glow_size = 128;

	Texture*tmptex;

	//
	// options switches
	//
	unsigned flags;

	GLint ifplanesId;
	GLint iftexturesId;
	GLint ifnormalsId;
	GLint brightness;
};

} // GFX


#endif /* __DEFFERED_RENDERER_H__ */

