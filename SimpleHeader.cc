#include "SimpleHeader.h"
#include <iostream>
#include <string>
#include <bitset>

bool inRange( unsigned low, unsigned high, unsigned x )
{
	return  ( ( x - low ) <= ( high - low ) );
}

SimpleHeader::SimpleHeader() {

	for ( int i = 0; i < 13; i++ ) // 12 bytes + 512 bytes for payload + 4 bytes for CRC32
	{
		packet.header [ i ] = 0;
	}
}




void SimpleHeader::setType( unsigned int type ) // accepts either 1, 2 or 3
{

	packet.header [ TY ] &= 0x3f;
	packet.header [ TY ] |= ( type << 6 );


}

unsigned int SimpleHeader::getType( ) const
{

	return packet.header [ TY ] >> 6;
}

void SimpleHeader::setTR( unsigned int tr) // accepts 0, or 1
{
	packet.header [ TY ] |= ( tr << 5 );
}

unsigned int SimpleHeader::getTR( ) const
{
	return packet.header [ TY ] >> 5 & 0x1;
}

void SimpleHeader::setWindow( unsigned int window ) // accepts 0 up to 31
{
	if ( inRange( 0, 31, window ) )
	{
		packet.header [ TY ] |= window; // Add but leave other bits untouched
	}
	else
	{
		packet.header [ TY ] = 0; // set to zero by default
	}


}
unsigned int SimpleHeader::getWindow( ) const
{


	return packet.header[TY] & 0x1F;
}

void SimpleHeader::setSequence( unsigned int num )
{
	if ( inRange( 0, 255, num ) )
	{
		packet.header [ SQ ] = num;
	}
	else
	{
		packet.header [ SQ ] = 0; // set to zero by default
	}
}

unsigned int SimpleHeader::getSequence( ) const
{
	return packet.header [ SQ ];
}


void SimpleHeader::setLength( unsigned int len )
{
	if ( inRange( 0, 512, len ) )
	{
		packet.header [ LN ] = ( len >> 8 );
		packet.header [ LN + 1 ] = ( len & 255 );
	}
	else
	{
		packet.header [ LN ] = 0;
		packet.header [ LN + 1 ] = 0; // set to 0 by default
	}

}

unsigned int SimpleHeader::getLength( ) const
{
	unsigned int count = 0;
	for(int i = 0; i<=512; i++)
	{
		if(this->packet.data[i] == '\0')
		break;
		count++;
	}
	return count;
}

void SimpleHeader::setTimestamp( unsigned int time )
{
	if ( inRange( 0, 4294967295, time ) )
	{
		packet.header [ TM ] = ( time >> 24 );
		packet.header [ TM + 1 ] = ( time >> 16 );
		packet.header [ TM + 2 ] = ( time >> 8 );
		packet.header [ TM + 3 ] = ( time & 255 );
	}
}

unsigned int SimpleHeader::getTimestamp( ) const
{
	return packet.header [ TM + 3 ] | ( packet.header [ TM + 2 ] << 8 ) | (packet.header[TM + 1] << 16) | (packet.header[TM] << 24);
}

void SimpleHeader::setCRC1( unsigned int crc )
{
	if ( inRange( 0, 4294967295, crc ) )
	{
		packet.header [ CRC1 ] = ( crc >> 24 );
		packet.header [ CRC1 + 1 ] = ( crc >> 16 );
		packet.header [ CRC1 + 2 ] = ( crc >> 8 );
		packet.header [ CRC1 + 3 ] = ( crc & 255 );
	}
}



void SimpleHeader::setPayload(unsigned int payload) {

	packet.header[PL] = (payload >> 4096);
	for(int i = 1; i <= 524; i++)
	{
		packet.header[PL + i] = (payload >> (4096 - i*8));
	}


}

unsigned int SimpleHeader::getPayload() const {

	return packet.header[524];
}

unsigned int SimpleHeader::getCRC1( ) const
{
	return packet.header [ CRC1 + 3 ] | ( packet.header [ CRC1 + 2 ] << 8 ) | ( packet.header [ CRC1 + 1 ] << 16 ) | ( packet.header [ CRC1 ] << 24 );
}
