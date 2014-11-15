/***************************************************************************
 *            spi_core.c
 *
 *  Sun Jan 18 11:39:35 2009
 *  Copyright  2009  Dirk BroÃŸwick
 *  <sharandac@snafu.de>
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
 
#include <avr/io.h>

#include "spi.h"

/**
 * \brief Die Init fuer dir SPI-Schnittstelle. Es koennen verschiedene Geschwindigkeiten eingestellt werden.
 */
void SPI_init( int SPI_ID )
{
	// Set SPI baseaddress errechnen
	SPI_t * spi = (void *) ( 0x08c0 | ( SPI_ID << 8 ) );
	// Set PORT baseaddress errechnen
	PORT_t * port = (void *) ( 0x0600 | ( ( SPI_ID + 2 ) << 5 ) );

	/* MOSI and SCK as output. */
	port->OUT |= SPI_MOSI_bm | SPI_SCK_bm | SPI_SS_bm ;
	port->DIR |= SPI_MOSI_bm | SPI_SCK_bm | SPI_SS_bm ;
	/* MISO as Input. */
	port->DIR &= ~SPI_MISO_bm;

	spi->CTRL = SPI_CLK2X_bm | SPI_ENABLE_bm | SPI_MASTER_bm ;
	spi->CTRL = SPI_ENABLE_bm | SPI_MASTER_bm ;
}

/**
 * \brief Schreibt einen Wert auf den SPI-Bus. Gleichzeitig wird ein Wert von diesem im Takt eingelesen.
 * \warning	Auf den SPI-Bus sollte vorher per Chip-select ein Baustein ausgewaehlt werden. Dies geschied nicht in der SPI-Routine sondern
 * muss von der Aufrufenden Funktion gemacht werden.
 * \param	SPI_ID	Nummer des SPI_Bus der angesprochen werden soll.
 * \param 	Data	Der Wert der uebertragen werden soll.
 * \retval  Data	Der wert der gleichzeit empfangen wurde.
 */
char SPI_ReadWrite( int SPI_ID, char Data )
{
	// Set SPI baseaddress errechnen
	SPI_t * spi = (void *) ( 0x08c0 | ( SPI_ID << 8 ) );

	// daten senden
	spi->DATA = Data;
	// auf fertig warten
	while( !(spi->STATUS & SPI_IF_bm ) );
	// daten zurueckgeben
	return( spi->DATA );
}

/**
 * \brief Eine schnelle MEM->SPI Blocksende Routine mit optimierungen auf Speed.
 * \param	SPI_ID		Nummer des SPI_Bus der angesprochen werden soll.
 * \param	buffer		Zeiger auf den Puffer der gesendet werden soll.
 * \param	Datalenght	Anzahl der Bytes die gesedet werden soll.
 */
void SPI_WriteBlock( int SPI_ID, char * buffer, int Datalenght )
{
	// Set SPI baseaddress errechnen
	SPI_t * spi = (void *) ( 0x08c0 | ( SPI_ID << 8 ) );

	char * spi_status = (char *) &spi->STATUS ;
	char * spi_data = (char *) &spi->DATA ;
	
	char data;
	
	// ersten Wert senden
	*spi_data = *buffer++;
	Datalenght--;

	while( Datalenght-- )
	{
		// Wert schon mal in Register holen, schneller da der Wert jetzt in einem Register steht und nicht mehr aus dem RAM geholt werden muss
		// nachdem das senden des vorherigen Wertes fertig ist,
		data = *buffer++;
		// warten auf fertig
		while( !( *spi_status ) );
		// Wert aus Register senden
		*spi_data = data;
	}
	while( !( *spi_status ) );

	return;
}

/**
 * \brief Eine schnelle SPI->MEM Blockempfangroutine mit optimierungen auf Speed.
 * \warning Auf einigen Controller laufen die Optimierungen nicht richtig. Bitte teil des Sourcecode der dies verursacht ist auskommentiert.
 * \param	SPI_ID		Nummer des SPI_Bus der angesprochen werden soll.
 * \param	buffer		Zeiger auf den Puffer wohin die Daten geschrieben werden sollen.
 * \param	Datalenght	Anzahl der Bytes die empfangen werden sollen.
 */
void SPI_ReadBlock( int SPI_ID, char * buffer, int Datalenght )
{
	// Set SPI baseaddress errechnen
	SPI_t * spi = (void *) ( 0x08c0 | ( SPI_ID << 8 ) );
	
	while( Datalenght )
	{
		Datalenght--;
		// Dummy Wert senden
		spi->DATA = 0x00;
		// warten auf fertig
		while( !(spi->STATUS & SPI_IF_bm ) );
		// speichern
		*buffer++ = spi->DATA;
	}

	return;
}
