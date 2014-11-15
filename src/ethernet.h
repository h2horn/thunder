#ifndef _ETHERNET_H
#define _ETHERNET_H

#define ETH_HDR_LEN		14
#define IP_HDR_LEN		20
#define UDP_HDR_LEN		8

// #define MAX_ARPTABLE_ENTRYS 2
// #define DEFAULT_ARP_TTL	30

// Convert dot-notation IP address into 32-bit word. Example: IPDOT(192l,168l,1l,1l)
#define IPDOT( d, c, b, a ) ((a<<24)|(b<<16)|(c<<8)|(d))

void ethernet_init(void);
void eth_int_init(void);
void check_ethernet(void);
void sendEthernetframe(void);
void eth(void); 
void ip(void);
void arp(void);
void icmp(void);
void udp(void);
int htons(int x);

//----------------------------------------------------------------------------
//Aufbau eines Ethernetheader
//
//
//
//
typedef struct ETH_header_struct	{
	unsigned char ETH_destMac[6];	//MAC Zieladresse 6 Byte
	unsigned char ETH_sourceMac[6];	//MAC Quelladresse 6 Byte
	unsigned int ETH_typefield;		//Nutzlast 0x0800=IP Datagramm;0x0806 = ARP
} ETH_header_t;

//----------------------------------------------------------------------------
//Aufbau eines ARP Header
//	
//	2 BYTE Hardware Typ				|	2 BYTE Protokoll Typ	
//	1 BYTE Länge Hardwareadresse	|	1 BYTE Länge Protokolladresse
//	2 BYTE Operation
//	6 BYTE MAC Adresse Absender		|	4 BYTE IP Adresse Absender
//	6 BYTE MAC Adresse Empfänger	|	4 BYTE IP Adresse Empfänger	
typedef struct ARP_header_struct	{
		unsigned int ARP_HWType;		//Hardware Typ enthält den Code für Ethernet oder andere Link Layer
		unsigned int ARP_PRType;		//Protokoll Typ enthält den Code für IP o. anderes Übertragungsprotokoll
		unsigned char ARP_HWLen;		//Länge der Hardwareadresse enthält 6 für 6 Byte lange MAC Adressen
		unsigned char ARP_PRLen;		//Länge der Protocolladresse enthält 4 für 4 Byte lange IP Adressen
		unsigned int ARP_opcode;		//Enthält Code der signalisiert ob es sich um eine Anfrage o. Antwort handelt
		unsigned char ARP_sourceMac[6];	//Enthält die MAC Adresse des Anfragenden  
		unsigned long ARP_sourceIP;    //Enthält die IP Adresse des Absenders
		unsigned char ARP_destMac[6];	//MAC Adresse Ziel, ist in diesem Fall 6 * 00,da die Adresse erst noch herausgefunden werden soll (ARP Request)
		unsigned long ARP_destIP;    //IP Adresse enthält die IP Adresse zu der die Kommunikation aufgebaut werden soll 
} ARP_header_t;

//----------------------------------------------------------------------------
//Aufbau eines IP Datagramms (B=BIT)
//	
//4B Version	|4B Headergr.	|8B Tos	|16B Gesamtlänge in Bytes	
//16B Identifikation			|3B Schalter	|13B Fragmentierungsposition
//8B Time to Live	|8B Protokoll	|16B Header Prüfsumme 
//32B IP Quelladresse
//32B IB Zieladresse
typedef struct IP_header_struct {
	unsigned char	IP_Vers_Len;	//4 BIT Die Versionsnummer von IP, 
									//meistens also 4 + 4Bit Headergröße 	
	unsigned char	IP_Tos;			//Type of Service
	unsigned int	IP_Pktlen;		//16 Bit Komplette Läng des IP Datagrams in Bytes
	unsigned int	IP_Id;			//ID des Packet für Fragmentierung oder 
									//Reassemblierung
	unsigned int	IP_Frag_Offset;	//wird benutzt um ein fragmentiertes 
									//IP Packet wieder korrekt zusammenzusetzen
	unsigned char	IP_ttl;			//8 Bit Time to Live die lebenszeit eines Paket
	unsigned char	IP_protocol;		//Zeigt das höherschichtige Protokoll an 
									//(TCP, UDP, ICMP)
	unsigned int	IP_Hdr_Cksum;	//Checksumme des IP Headers
	unsigned long	IP_srcIP;		//32 Bit IP Quelladresse
	unsigned long	IP_destIP;		//32 Bit IP Zieladresse
} IP_header_t;

//----------------------------------------------------------------------------
//Aufbau einer ICMP Nachricht
//	
//8 BIT Typ	|8 BIT Code	|16 BIT Prüfsumme	
//Je nach Typ			|Nachricht unterschiedlich
//Testdaten
//
//8 BIT Typ = 0 Echo Reply oder 8 Echo request	
typedef struct ICMP_header_struct {
		unsigned char     	ICMP_Type;		//8 bit typ identifiziert Aufgabe der Nachricht 
											//0 = Echo Reply 8 = Echo Request
		unsigned char     	ICMP_Code;		//8 Bit Code enthält Detailangaben zu bestimmten Typen
		unsigned int     	ICMP_Cksum;		//16 Bit Prüfsumme enthält die CRC Prüfsumme
		unsigned int     	ICMP_Id;		//2 byte Identifizierung
		unsigned int     	ICMP_SeqNum;	//Sequenznummer
} ICMP_header_t;

//----------------------------------------------------------------------------
//UDP Header Layout
//
//
typedef struct UDP_header_struct {
	unsigned int 	UDP_SrcPort;	//der Quellport für die Verbindung (Socket)
	unsigned int 	UDP_DestPort;	//der Zielport für die Verbindung (Socket)
	unsigned int 	UDP_Hdrlen;
	unsigned int	UDP_Chksum;	//Enthält eine Prüfsumme über Header und Datenbereich
} UDP_header_t;

typedef struct ARP_table_struct {
	long IP;
	char MAC[6];
	unsigned char ttl;
} ARP_table_t;

#endif // _ETHERNET_H
