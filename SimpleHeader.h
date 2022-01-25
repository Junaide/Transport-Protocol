#ifndef __SIMPLEHEADER_H
#define __SIMPLEHEADER_H

#include <cstdint>
#include <string>
// maximum size for the payload
#define DATA_SZ 512
// size of header
#define HEADER_SZ 12 // 12 bytes for the header


struct simplepacket {
  uint8_t header[HEADER_SZ];
  char data[DATA_SZ];  // payload
};

// class to be tested. Implements the demo header structure below.

// 0 1 2 3-7 8  15 16     23 24     31
// +--+-+---+-----+---------+--------+
// |Ty|T|Win| Seq |      Length      |
// |Pe|R|Dow| Num |                  |
// +--+-+---+-----+---------+--------+
// |        Timestamp (4 bytes)      |
// +---------------------------------+
// |        CRC1 (4 bytes)           |
// +---------------------------------+
// |                                 |
// |      Payload (max 512 Bytes)    |
// |                                 |
// +---------------------------------+
// |     CRC2 (4 bytes, optional)    |
// +---------------------------------+

// first byte contains TYPE, TR and WINDOW, second byte contains SEQ NUM, third and fourth byte contains LENGTH
// fifth, sixth, seventh, and eighth byte contain the TIMESTAMP, ninth, tenth, eleventh, and twevlth byte contain CRC1

class SimpleHeader {
private:
  struct simplepacket packet;

  // start index of type
  const int TY = 0;

  // start index of seq num
  const int SQ = 1;

  // start index of length
  const int LN = 2;

  // start index of timestamp
  const int TM = 4;

  // start index of CRC1
  const int CRC1 = 8;

  // start index of payload
  const int PL = 12;



public:
  // default constructor initializes the header to zero.
  SimpleHeader();

  // sets the value of the payload length
  // val = length; if val > DATA_SZ, the value set is DATA_SZ

  void setType( unsigned int type );
  unsigned int getType( ) const;

  void setTR( unsigned int tr );
  unsigned int getTR( ) const;

  void setWindow( unsigned int window );
  unsigned int getWindow( ) const;

  void setSequence( unsigned int num );
  unsigned int getSequence( ) const;

  void setLength( unsigned int len );
  unsigned int getLength( ) const;

  void setPayload( unsigned int payload );
  unsigned int getPayload() const;
  void setTimestamp( unsigned int time ); // YYYY/MM/DD/HH/MM/SS/TZ -------> Year/Month/Day/Hour/Minute/Second/Timezone XXXX XX XX XX XX XX XX (2 byte length)
  unsigned int getTimestamp( ) const;

  void setCRC1( unsigned int crc );
  unsigned int getCRC1( ) const;



  // returns the size of the packet, including headers and data
  // to be used with recvfrom() or sendto()
  unsigned int totalPacketSize() const {
    return getLength() + HEADER_SZ;
  }

  // returns pointer to the structure holding the thePacket, including the headers
  // To be used with recvfrom or sendto
  void * thePacket() {
    return &packet;
  }

  char * thePayload() {
    return packet.data;
  }
};

#endif
