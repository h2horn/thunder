#include <avr/io.h>
#include "ethernet.h"
#include "enc28j60.h"
#include "config.h"
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stddef.h>
#include <avr/pgmspace.h> 
#include "dma.h"
#include "spi.h"
#define F_CPU 32000000UL
#include <util/delay.h>

// #define _DEBUG_

void write_light_data(USART_t *usart, char *buffer);

unsigned int packet_length;
char ethernetbuffer[MAX_FRAMELEN];
ETH_header_t *ETH_header = (ETH_header_t *)ethernetbuffer; 		//ETH_struc anlegen
ARP_header_t *ARP_header = (ARP_header_t *)&ethernetbuffer[ETH_HDR_LEN];
IP_header_t *IP_header = (IP_header_t *)&ethernetbuffer[ETH_HDR_LEN];
ICMP_header_t *ICMP_header = (ICMP_header_t *)&ethernetbuffer[ETH_HDR_LEN + IP_HDR_LEN];
UDP_header_t *UDP_header = (UDP_header_t *) &ethernetbuffer[ETH_HDR_LEN + IP_HDR_LEN];

// unsigned long PacketCounter; 
// unsigned long ByteCounter;
// ARP_table_t ARPtable[ MAX_ARPTABLE_ENTRYS ];

char mymac[6] = { ENC28J60_MAC0,ENC28J60_MAC1,ENC28J60_MAC2,ENC28J60_MAC3,ENC28J60_MAC4,ENC28J60_MAC5 };
// If DHCP fails these are the values that will be used as a fallback or when DHCP disable
long myIP     = MYIP;
// long Netmask    = NETMASK;
// long Gateway    = GATEWAY;
// long DNSserver  = DNSSERVER;

void ethernet_init() {
	enc28j60Init();

	// eth_int_init();

	// Rad Device Signature and set as MAC
	NVM_CMD = NVM_CMD_READ_CALIB_ROW_gc;
	mymac[0] = pgm_read_byte(offsetof(NVM_PROD_SIGNATURES_t, LOTNUM5));
	mymac[1] = pgm_read_byte(offsetof(NVM_PROD_SIGNATURES_t, WAFNUM));
	mymac[2] = pgm_read_byte(offsetof(NVM_PROD_SIGNATURES_t, COORDX0));
	mymac[3] = pgm_read_byte(offsetof(NVM_PROD_SIGNATURES_t, COORDX1));
	mymac[4] = pgm_read_byte(offsetof(NVM_PROD_SIGNATURES_t, COORDY0));
	mymac[5] = pgm_read_byte(offsetof(NVM_PROD_SIGNATURES_t, COORDY1));
	NVM_CMD = NVM_CMD_NO_OPERATION_gc;

	nicSetMacAddress(mymac);

	// Alle Packet lesen und ins leere laufen lassen damit ein definierter zustand herrscht
	char ethernetbuffer[ MAX_FRAMELEN ];
	while ( enc28j60PacketReceive( MAX_FRAMELEN, ethernetbuffer) != 0 ) {};

	for(int i = 0 ; i < MAX_FRAMELEN ; i++){	
		ethernetbuffer[i] = 0x00;
	}

	#ifdef _DEBUG_
		printf("Ethernet init!\n");
	#endif
}

void eth_int_init() {
	// ENC28J60_INT_PORT.DIRCLR = (1<<ENC28J60_INT_PIN);
	ENC28J60_INT_PORT.PIN1CTRL = PORT_ISC_FALLING_gc;
	ENC28J60_INT_PORT.INTCTRL = PORT_INT0LVL_HI_gc;
	ENC28J60_INT_PORT.INT0MASK = (1 << ENC28J60_INT_PIN);
}

int htons(int x) {
	asm volatile (
		"eor %A0, %B0"   "\n\t"
		"eor %B0, %A0"   "\n\t"
		"eor %A0, %B0"
		: "=r" (x)
		: "0"  (x)
	);
	return x;
	// return ((((uint16_t)(A) & 0xff00) >> 8) | (((uint16_t)(A) & 0x00ff) << 8));
}

/* -----------------------------------------------------------------------------------------------------------
Sendet ein Ethernetframe
------------------------------------------------------------------------------------------------------------*/
void sendEthernetframe()
{	
	// PacketCounter++;
	// ByteCounter = ByteCounter + packet_lenght;
	enc28j60PacketSend( packet_length, ethernetbuffer );
}


/*!\brief Ethernet-Layer
 * \param packet_lenght			Anzahl der Byte im Ethernetbuffer
 * \param ethernetbuffer		Zeiger auf den Ethernetbuffer an sich
 */
void eth() {
	enc28j60ReadBuffer(ETH_HDR_LEN, ethernetbuffer);
	// printf("new Paket! %x \n", htons( ETH_header->ETH_typefield ));
	switch ( htons( ETH_header->ETH_typefield ) ) // welcher type ist gesetzt 
	{
		case 0x0806:
						#ifdef _DEBUG_
							printf("-->> ARP\r\n");
						#endif
						arp();
						break;
		case 0x0800:		
						#ifdef _DEBUG_
							printf("-->> IP\r\n");										
						#endif
						ip();
						break;
	}
}

/*!\brief Die ARP-Funktion wenn ein ARP-Packet eintrifft
 * \param packet_lenght			Anzahl der Byte im Ethernetbuffer
 * \param ethernetbuffer		Zeiger auf den Ethernetbuffer an sich
 */
void arp() {
	uint8_t i;
	enc28j60ReadBuffer(packet_length - ETH_HDR_LEN, &ethernetbuffer[ETH_HDR_LEN]);
	switch ( htons( ARP_header->ARP_opcode ) ) {
		
		case 0x0001:	// ARP Request
						#ifdef _DEBUG_
							printf("-->> ARP Request\n");
							// printf("-->> ARP IP %lu \n", ARP_header->ARP_destIP);
							// printf("-->> ARP myIP %lu \n", myIP);
							// printf("-->> ARP MAC %x \n", ARP_header->ARP_sourceMac[3]);
							printf("-->> ARP HW %x \n", ARP_header->ARP_HWType);
						#endif
						if ( ARP_header->ARP_destIP != myIP ) return ;
						// wenn ja fang mal an die antwort zusammen zu basteln
						ARP_header->ARP_opcode = htons( 0x0002 );
						// mac und ip des senders in ziel kopieren
						for ( i = 0; i < 6; i++ )ARP_header->ARP_destMac[i] = ARP_header->ARP_sourceMac[i]; // MAC und IP umkopieren
						// meine mac und ip als absender einsetzen
						for ( i = 0; i < 6; i++ )ARP_header->ARP_sourceMac[i] = mymac[i]; // MAC einsetzen

						// IP einsetzen
						ARP_header->ARP_destIP = ARP_header->ARP_sourceIP;
						ARP_header->ARP_sourceIP = myIP;

						// sourceMAC in destMAC eintragen und meine MAC in sourceMAC kopieren
						for( i = 0 ; i < 6 ; i++){	
								ETH_header->ETH_destMac[i] = ETH_header->ETH_sourceMac[i];	
								ETH_header->ETH_sourceMac[i] = mymac[i]; }
						sendEthernetframe();
						break;
		case 0x0002:	// ARP Response
						#ifdef _DEBUG_
							printf("-->> ARP Answer\n");
						#endif
						// for( i = 0 ; i < MAX_ARPTABLE_ENTRYS ; i++ )
						// {
						// 	if( ARPtable[ i ].IP == ARP_header->ARP_sourceIP )
						// 	{
						// 		ARPtable[i].IP = ARP_header->ARP_sourceIP;
						// 		ARPtable[i].ttl = DEFAULT_ARP_TTL;
						// 		for ( a = 0 ; a < 6 ; a++ ) ARPtable[i].MAC[a] = ARP_header->ARP_sourceMac[a];
						// 	}
						// }
						break;
	}
}


/*!\brief Die IP-Funktion wenn ein IP-Packet eintrifft
 * \param packet_lenght			Anzahl der Byte im Ethernetbuffer
 * \param ethernetbuffer		Zeiger auf den Ethernetbuffer an sich
 */
void ip() {
	enc28j60ReadBuffer(IP_HDR_LEN, &ethernetbuffer[ETH_HDR_LEN]);
										// checke mal ob dat Ã¼berhaupt fÃ¼r uns ist
	// if ( IP_packet->IP_DestinationIP != myIP || IP_packet->IP_DestinationIP != 0xffffffff ) return;

//		if ( ( IP_packet->IP_Flags_Fragmentoffset & htons( FRAGMENTOFFSET_bm ) ) == 0 )
//		{
		switch ( IP_header->IP_protocol )
		{
			case 0x01:		// ICMP
							#ifdef _DEBUG_
								printf("-->> IP ICMP\n");
							#endif
							// printf("-->> IP IP %lu \n", IP_header->IP_destIP);
							// printf("-->> IP myIP %lu \n", myIP);
							if ( IP_header->IP_destIP != myIP ) return;
							icmp();
							break;
			case 0x06:		// TCP
							#ifdef _DEBUG_
								printf("-->> IP TCP\n");
							#endif	
							// if ( IP_header->IP_destIP != myIP ) return;
							// tcp( packet_length , buffer );
							break;
			case 0x11:		// UDP
							#ifdef _DEBUG_
								printf("-->> IP UDP\n");
							#endif	
							udp();
							break;
		}
//		}
}

/*!\brief Die ICMP-Funktion wenn ein ICMP-Packet eintrifft
 * \param packet_lenght			Anzahl der Byte im Ethernetbuffer
 * \param ethernetbuffer		Zeiger auf den Ethernetbuffer an sich
 */
void icmp() {
	enc28j60ReadBuffer(packet_length - ETH_HDR_LEN - IP_HDR_LEN, &ethernetbuffer[ETH_HDR_LEN + IP_HDR_LEN]);
	switch (ICMP_header->ICMP_Type) {
		
		case 0x00:		// ICMP Echo Reply
						#ifdef _DEBUG_
							printf("-->> ICMP Echo Reply \n");
						#endif
						break;
		case 0x08:		// ICMP Echo Request
						#ifdef _DEBUG_
							printf("-->> ICMP Echo Request \n");
							// printf("-->> IP Vers %u \n", IP_header->IP_Vers_Len);
							// printf("-->> IP Tos %u \n", IP_header->IP_Tos);
							// printf("-->> IP dIP %lu \n", IP_header->IP_destIP);
							// printf("-->> IP sIP %lu \n", IP_header->IP_srcIP);
							// printf("-->> IP myIP %lu \n", myIP);
						#endif

						ICMP_header->ICMP_Type = 0x00;

						// IP_header->IP_Vers_Len = 33;
						IP_header->IP_destIP = IP_header->IP_srcIP ;
						IP_header->IP_srcIP = myIP ; // IP einsetzen

						for(uint8_t i = 0 ; i < 6 ; i++){	
								ETH_header->ETH_destMac[i] = ETH_header->ETH_sourceMac[i];	
								ETH_header->ETH_sourceMac[i] = mymac[i]; 
						}
						sendEthernetframe();
						break;
	}
}

/*!\brief Die UDP-Funktion wenn ein UDP-Packet eintrifft
 * \param packet_lenght			Anzahl der Byte im Ethernetbuffer
 * \param ethernetbuffer		Zeiger auf den Ethernetbuffer an sich
 */
void udp() {
	// enc28j60ReadBuffer(UDP_HDR_LEN, &ethernetbuffer[ETH_HDR_LEN + IP_HDR_LEN]);
	enc28j60ReadBuffer(packet_length - ETH_HDR_LEN - IP_HDR_LEN, &ethernetbuffer[ETH_HDR_LEN + IP_HDR_LEN]);
	#ifdef _DEBUG_
		printf("UDP Port %u \n", htons(UDP_header->UDP_DestPort));
		// printf("UDP IP %u.%u.%u.%u \n", (uint8_t)IP_header->IP_srcIP & 0xFF, (uint8_t)(IP_header->IP_srcIP >> 8) & 0xFF, (uint8_t)(IP_header->IP_srcIP >> 16) & 0xFF, (uint8_t)(IP_header->IP_srcIP >> 24) & 0xFF);
		// printf("UDP MAC %i:%i:%i:%i:%i:%i \n", ETH_header->ETH_destMac[0], ETH_header->ETH_destMac[1], ETH_header->ETH_destMac[2], ETH_header->ETH_destMac[3], ETH_header->ETH_destMac[4], ETH_header->ETH_destMac[5]);
		// printf("--> UDP Data %s \n", &buffer[ETH_HDR_LEN + IP_HDR_LEN + UDP_HDR_LEN]);
	#endif

	switch (htons(UDP_header->UDP_DestPort)) {
		case 50000:	// LED1
					//PORTA.OUTTGL = 0xFF;

					write_light_data(&USARTE0, &ethernetbuffer[ETH_HDR_LEN + IP_HDR_LEN + UDP_HDR_LEN]);

					// assert CS
					//ENC28J60_CONTROL_PORT.OUTCLR = ( 1<<ENC28J60_CONTROL_CS );
					// // issue read command
					//SPI_ReadWrite( SPIBUS, ENC28J60_READ_BUF_MEM );

					//dma_start(&DMA.CH0);

					//_delay_ms(2000);


					break;
		case 50001:	// LED2
					write_light_data(&USARTD1, &ethernetbuffer[ETH_HDR_LEN + IP_HDR_LEN + UDP_HDR_LEN]);
					break;
		case 50002:	// LED3
					write_light_data(&USARTD0, &ethernetbuffer[ETH_HDR_LEN + IP_HDR_LEN + UDP_HDR_LEN]);
					break;
		case 50003:	// LED4
					write_light_data(&USARTC0, &ethernetbuffer[ETH_HDR_LEN + IP_HDR_LEN + UDP_HDR_LEN]);
					break;
		// case 55000:	// Control
		// 			break;
		// case 67:	// DHCP
		// 			break;
		default: 	break;
	}

}

void check_ethernet() {
	packet_length = enc28j60PacketInfoReceive();

	if (packet_length == 0)
		return;

	// enc28j60ReadBuffer(packet_length, ethernetbuffer);
	eth();
	enc28j60PacketFinished();
}

// ISR (PORTB_INT0_vect) {
// 	new_eth++;

// 	// #ifdef _DEBUG_
// 		// puts("new ETH Packet!\n");
// 	// #endif

// }

void write_light_data(USART_t *usart, char *buffer) {

	for (int i = 0; i < 484; ++i)
	{
		usart->DATA = buffer[i] | 0x80;
		while(!(usart->STATUS & USART_DREIF_bm));
	}

	_delay_ms(1);
	// ((160 + 63) / 64) * 3
	for(uint16_t n = 1; n > 0; n--) {
		usart->DATA = 0x00;
		while(!(usart->STATUS & USART_DREIF_bm));
	}
}
