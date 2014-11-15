/***************************************************************************
 *            spi_core.h
 *
 *  Sun Jan 18 11:40:26 2009
 *  Copyright  2009  Dirk BroÃŸwick
 *  <sharandac@snafu.de>
///	\ingroup hardware
 ****************************************************************************/

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */
 
#ifndef _SPI_H
	#define _SPI_H

	#define SPI_SS_bm             0x10 /*!< \brief Bit mask for the SS pin. */
	#define SPI_MOSI_bm           0x20 /*!< \brief Bit mask for the MOSI pin. */
	#define SPI_MISO_bm           0x40 /*!< \brief Bit mask for the MISO pin. */
	#define SPI_SCK_bm            0x80 /*!< \brief Bit mask for the SCK pin. */

	void SPI_init( int SPI_ID );
	char SPI_ReadWrite( int SPI_ID, char Data );
	void SPI_WriteBlock( int SPI_ID, char * buffer, int Datalenght );
	void SPI_ReadBlock( int SPI_ID, char * buffer, int Datalenght );

	typedef void pSPI_INIT ( void );
	typedef char pSPI_READWRITE ( char Data );
	typedef void pSPI_WRITEBLOCK ( char * buffer, int Datalenght );
	typedef void pSPI_READBLOCK ( char * buffer, int Datalenght );
	typedef void pSPI_PRESCALER ( char prescaler );

	typedef struct {
		pSPI_INIT			* INIT;
		pSPI_READWRITE		* ReadWrite;
		pSPI_WRITEBLOCK		* WriteBlock;
		pSPI_READBLOCK		* ReadBlock;
		pSPI_PRESCALER		* Prescaler;
	} const SPI_BUS ;		

#endif /* SPI_H */

