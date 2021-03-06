#ifndef _PLANET_ROW_H_
#define _PLANET_ROW_H_

#include "row.h"

namespace MEM
{
	/**
	 * @brief Klasa definiująca wiersz planety z bazy danych.
	 */
	class PlanetRow : public Row
	{
		public:
			PlanetRow();
			virtual ~PlanetRow();

			virtual uint8_t size() const;
			virtual void setCell( unsigned idx, const std::string &val );

		public:
			/**
			 * @brief Położenie x planety.
			 */
			float xcoord;
			/**
			 * @brief Położenie y planety.
			 */
			float ycoord;
			/**
			 * @brief Położenie z planety.
			 */
			float zcoord;
			/**
			 * @brief Promień planety
			 */
			float radius;
			/**
			 * @brief Masa planety.
			 */
			float mass;
			/**
			 * @brief Prędkość x planety.
			 */
			float xvel;
			/**
			 * @brief Prędkość y planety.
			 */
			float yvel;
			/**
			 * @brief Prędkość z planety.
			 */
			float zvel;
			/**
			 * @brief ID modelu planety.
			 */
			int model_id;

		protected:
			virtual std::string getTableName() const;
			virtual std::string getCellNames() const;
			virtual std::string getCellDefs() const;
			virtual std::string getCellValues() const;
	};
}

#endif // _PLANET_ROW_H_
