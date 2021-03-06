#ifndef __MEMORY_MANAGER_H__

#define __MEMORY_MANAGER_H__

#include <GL/glew.h>

#include <string>

#include "misc/materials_manager.h"
#include "misc/textures_manager.h"
#include "misc/gfx_planet_factory.h"
#include "misc/phx_planet_factory.h"
#include "misc/planet_params.h"

/**
 * @brief Przestrzeń nazw dla obiektów odpowiedzialnych za zarządzanie pamięcią.
 */
namespace MEM {

	class MemMgr {
	public:
		MemMgr( );
		virtual ~MemMgr();

		void init();

		MISC::GfxPlanetFactory* getGfxMem();
		MISC::PhxPlanetFactory* getPhxMem();
		
		GLuint loadMaterials( const MISC::Materials& materials );
		GLuint loadTextures( const MISC::Textures& ctex );

		MISC::CpuPlanetHolder *getPlanets();
		void setPlanets( MISC::CpuPlanetHolder * );

		/**
		 * @brief Tworzy nową planetę.
		 *
		 * @param params Parametry tworzonej planety.
		 *
		 * @returns id nowej planety.
		 */
		unsigned createPlanet( MISC::PlanetParams params );

		/**
		 * @brief Usuwa planetę.
		 *
		 * @param id Indeks planety do usunięcia. 
		 */
		void removePlanet( unsigned id );

		void dropPlanets();

	private:
		GLuint matTexId;
		GLuint texTexId;

		MISC::PlanetHolder holder;

		MISC::GfxPlanetFactory gpf;
		MISC::PhxPlanetFactory ppf;
	};

}


#endif /* __MEMORY_MANAGER_H__ */

