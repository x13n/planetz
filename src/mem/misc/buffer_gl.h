#ifndef BUFFER_GL_H
#define BUFFER_GL_H

#include "buffer.h"
#include "buffer_cu.hpp"

#include <GL/glew.h>

#include <cstdio>

#include <cuda_runtime_api.h>
#include <cuda_gl_interop.h>

#include "cuda/err.h"

namespace MEM
{
namespace MISC
{
	/** możliwe stany buffora */
	enum BUFFER_STATE {
		BUF_GL , /**< opengl */
		BUF_CU , /**< cuda   */
		BUF_H    /**< host   */
	};

	/** 
	 * @brief Buffor trzymający pamięć na karcie graficznej przy pomocy OpenGLa.
	 * API OpenGLa pozwala na mapowanie takich bufforów do pamięci hosta,
	 * czyli w tym przypadku CPU, natomiast API CUDA pozwala na mapowanie ich
	 * do pamięci CUDA. Są one więc najbardziej wszechstronnymi buforami karty graficznej.
	 */
	template<typename T>
	class BufferGl : public BufferBase<T> {
		// prevent from copying buffer. If so, smart counter is needed 
		// to unregister from cuda and delete from opengl
		BufferGl( const BufferGl<T>& );
	public:
		BufferGl( );
		BufferGl( const size_t num , const T*data = NULL );
		virtual ~BufferGl( );

		virtual void resize( const size_t num , const T*data = NULL );

		/** 
		 * @brief mapuje bufor. Możliwe mapowania to OpenGL, CUDA lub HOST
		 * 
		 * @param state nowy stan bufora
		 * 
		 * @return wskaźnik do pamięci na którą zmapowany został bufor.
		 */
		T*       map( enum BUFFER_STATE state );
		/** 
		 * @brief stała wersja funkcji mapującaej. Zwraca stały wskaźnik.
		 * 
		 * @param state nowy stan bufora.
		 * 
		 * @return stały wskaźnik do pamięci.
		 */
		const T* map( enum BUFFER_STATE state ) const;
		/** 
		 * @brief przywraca buffor do domyślego stanu. Po wywołaniu tej funckji
		 * wszystkie wskaźniki otrzymane przez wywołanie map stają się nieaktualne.
		 */
		void unmap() const;
		
		/** 
		 * @brief binduje buffor do użycia przez opengl jako GL_ARRAY_BUFFER
		 */
		void bind() const;
		/** 
		 * @brief ustawia pusty bufor jako GL_ARRAY_BUFFER
		 */
		void unbind() const;

		virtual T getAt(unsigned i) const;
		virtual void setAt( unsigned i, const T& val );

		/** 
		 * @brief Zwraca id bufora w API OpenGLa
		 * 
		 * @return id bufora.
		 */
		GLuint getId() const;
	protected:
		T* fucking_no_const_cast_workaround( enum BUFFER_STATE state ) const;

		void gl_resize( const size_t new_size , const T*data );

		GLuint glId; // opengl buffer id
		T*     cuPtr;// cuda gpu data pointer
		T*     hPtr; // host mapped opengl gpu data pointer

		enum BUFFER_STATE  state;

		// nasty const hacks
		T**   phPtr;
		enum BUFFER_STATE* const pstate;
	};
	
	//
	// Implementation BufferGl
	//
	template<typename T>
	BufferGl<T>::BufferGl( )
		: glId(0) , cuPtr(NULL) , hPtr(NULL) , state(BUF_GL) ,
		  phPtr(&hPtr) , pstate(&state)
	{
	}

	template<typename T>
	BufferGl<T>::BufferGl( const size_t num , const T*data )
		: glId(0) , cuPtr(NULL) , hPtr(NULL) , state(BUF_GL) ,
		  phPtr(&hPtr) , pstate(&state)
	{
		resize( num , data );
	}

	template<typename T>
	BufferGl<T>::~BufferGl()
	{
		if( glId ) {
			log_printf(DBG,"[DEL] Not empty GL buffer\n");
			unmap();
			cudaGLUnregisterBufferObject( glId );
			CUT_CHECK_ERROR("Unregistering buffer while deleting BufferGl");
			glDeleteBuffers( 1 , &glId );
		}
	}

	template<typename T>
	void BufferGl<T>::gl_resize( const size_t new_size , const T*data )
	{
		log_printf(DBG,"Resizing gl buffer from %d bytes to %d bytes\n",this->realsize,new_size);
		glBindBuffer(GL_ARRAY_BUFFER,glId);
		// FIXME: gl constants are hard-coded, is it good?
		glBufferData(GL_ARRAY_BUFFER,new_size,(GLvoid*)data,GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER,0);
	}

	template<typename T>
	void BufferGl<T>::setAt( unsigned i, const T& val )
	{
		T* ptr = map( BUF_CU );
		cudaMemcpy( &ptr[i], &val, sizeof(T), cudaMemcpyHostToDevice );
		unmap();
	}

	template<typename T>
	T BufferGl<T>::getAt( unsigned i ) const
	{
		const T* ptr = map( BUF_CU );
		T retval;
		cudaMemcpy( &retval, &ptr[i], sizeof(T), cudaMemcpyDeviceToHost );
		unmap();
		return retval;
	}

	template<typename T>
	void BufferGl<T>::resize( const size_t num , const T*data )
	{
		// FIXME: assert if unmapped or unmap?
		ASSERT_MSG( state == BUF_GL , "Buffer not in gl mode\n" );

		size_t new_size = num*sizeof(T);

		if( new_size <= this->realsize ) {
			this->length = num;
			this->size = new_size;
			return;
		}

		if( !glId ) {
			glGenBuffers(1,&glId);

			gl_resize( new_size , data );

			cudaGLRegisterBufferObject( glId );
			CUT_CHECK_ERROR("Registering buffer while resizing BufferGl");
		} else {
			BufferCu<T> tmp;
			bool save = NULL == data && this->realsize > 0;
			if( save )
			{ //FIXME: @qba - da się to zrobić lepiej/bardziej openglowo?
				tmp.resize( this->realsize );
				cudaMemcpy( tmp.d_data(), map( BUF_CU ), tmp.getSize(), cudaMemcpyDeviceToDevice );
				unmap();
			}
			cudaGLUnregisterBufferObject( glId );
			CUT_CHECK_ERROR("Unregistering buffer while resizing old BufferGl");

			gl_resize( new_size , data );

			cudaGLRegisterBufferObject( glId );
			CUT_CHECK_ERROR("Registering buffer while resizing old BufferGl");
			if( save )
			{
				cudaMemcpy( map( BUF_CU ), tmp.d_data(), tmp.getSize(), cudaMemcpyDeviceToDevice );
				unmap();
			}
		}

		this->length = num;
		this->size =this-> realsize = new_size;
	}

	template<typename T>
	GLuint BufferGl<T>::getId() const
	{
		ASSERT( state == BUF_GL );
		return glId;
	}

	template<typename T>
	void BufferGl<T>::bind() const
	{
		ASSERT( state == BUF_GL );
		glBindBuffer( GL_ARRAY_BUFFER , glId );
		// FIXME: another state?
//                state = BUF_BIND;
	}

	template<typename T>
	void BufferGl<T>::unbind() const
	{
		glBindBuffer( GL_ARRAY_BUFFER , 0 );
	}

	template<typename T>
	T* BufferGl<T>::fucking_no_const_cast_workaround( enum BUFFER_STATE new_state ) const
	{
		ASSERT_MSG( this->size > 0 , "Cannot map empty buffer:<" );

		if( state == new_state ) return *pstate==BUF_CU?cuPtr:(state==BUF_H?hPtr:NULL);

		unmap();

		*pstate = new_state;

		if( state == BUF_GL ) return NULL;
		if( state == BUF_CU ) {
			cudaGLMapBufferObject( (void**)&cuPtr , glId );
			CUT_CHECK_ERROR("Mapping BufferGl to cuda\n");
			return cuPtr;
		}
		if( state == BUF_H  ) {
			glBindBuffer( GL_ARRAY_BUFFER , glId );
			*phPtr = (T*)glMapBuffer( GL_ARRAY_BUFFER , GL_READ_WRITE );
			glBindBuffer( GL_ARRAY_BUFFER , 0 );
			return hPtr;
		}
		ASSERT(false); // should never reach this code
		return NULL;
	}

	template<typename T>
	const T* BufferGl<T>::map( enum BUFFER_STATE new_state ) const
	{
		return fucking_no_const_cast_workaround( new_state );
	}

	template<typename T> 
	T* BufferGl<T>::map( enum BUFFER_STATE new_state ) 
	{
		/* fucking const_cast seg faults while casting invalid pointers.
		 * unfortunetely all gpu pointers are invalid for cpu!!
		 */
//                return const_cast<T*>(map(new_state)); 
		return fucking_no_const_cast_workaround( new_state );
	}

	template<typename T>
	void BufferGl<T>::unmap() const
	{
		ASSERT( this->size > 0 );

		
		if( state == BUF_CU ) {
			cudaGLUnmapBufferObject( glId );
			CUT_CHECK_ERROR("Unmapping cuda buffer to BufferGl");
		}
		if( state == BUF_H  ) {
			glBindBuffer( GL_ARRAY_BUFFER , glId );
			glUnmapBuffer( GL_ARRAY_BUFFER );
			glBindBuffer( GL_ARRAY_BUFFER , 0 );
		}

		*pstate = BUF_GL;
	}


}
}

#endif // BUFFER_GL_H
