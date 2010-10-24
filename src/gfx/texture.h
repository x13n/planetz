/**
 * @file texture.h
 * @author Jakub Kotur 
 * @version 0.1
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * Ladowanie oraz bindowanie tekstur z formatow obslugiwanych przez
 * devila - czyli calkiem sporo.
 */

#ifndef __TEXTURE_H__

#define __TEXTURE_H__

#include <GL/gl.h>
#include <string>
#include <map>

using std::string;

namespace Gfx {
	
/**
 * Klasa odpowiedzialan za ładownie tekstur z plików.
 * Korzysta z bibliotedki devil.
 * Zawiera optymalizację polegającą na agregowaniu już załadownych
 * tekstur, i zwracaniu ich gdy były już ładowane.
 */
class Texture {
	Texture( string _path , GLuint _tex ) : path(_path) , tex(_tex) {}
	virtual ~Texture();
public:
	void bind();

	static Texture* LoadTexture( const string& str );
	static Texture* LoadTexture( const char* str );
private:
	string path;
	GLuint tex;

	static std::map<string,Texture*> loaded_textures;
};

}

#endif /* __TEXTURE_H__ */
