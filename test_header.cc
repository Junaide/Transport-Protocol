#include "SimpleHeader.h"
#include "gtest/gtest.h"  // google test framework
#include <bitset>
class HeaderTest : public testing::Test
{
protected:
    SimpleHeader *h_;

    void SetUp( ) override
    {
        h_ = new SimpleHeader;  // create a new class before each test to start fresh
    }

    void TearDown( ) override
    {
        delete h_;
    }
};

//TEST_F( HeaderTest, setPayloadLength )
//{
//    h_->setPayloadLength( 0x1234 );
//    struct simplepacket *ptr = static_cast< struct simplepacket * > ( h_->thePacket( ) );
//    ASSERT_TRUE( ptr->header [ 2 ] == 0x12 );
//    ASSERT_TRUE( ptr->header [ 3 ] == 0x34 );
//}
//
//TEST_F( HeaderTest, getHeader )
//{
//    struct simplepacket *ptr = static_cast< struct simplepacket * > ( h_->thePacket( ) );
//    ptr->header [ 2 ] = 0x56;
//    ptr->header [ 3 ] = 0xa2;
//    ASSERT_TRUE( h_->getPayloadLength( ) == 0x56a2 );
//}


TEST_F( HeaderTest, setType )
{

    h_->setType( 0x3 );
    struct simplepacket *ptr = static_cast< struct simplepacket * > ( h_->thePacket( ) );
    EXPECT_EQ( ( ptr->header [ 0 ] >> 6 ), 0x3 );

}

TEST_F( HeaderTest, getType )
{

    h_->setType( 0x3 );
    h_->setTR( 0x0 );
    h_->setWindow( 15 );

    struct simplepacket *ptr = static_cast< struct simplepacket * > ( h_->thePacket( ) );
    std::cout << "Type: " << std::bitset<8>( h_->getType( ) ) << std::endl;
    std::cout << "Byte: " << std::bitset<8>( ptr->header [ 0 ] ) << std::endl;
    EXPECT_EQ( h_->getType(), 0x3 );

}

TEST_F( HeaderTest, setTR )
{

    h_->setTR( 0x1 );
    struct simplepacket *ptr = static_cast< struct simplepacket * > ( h_->thePacket( ) );
   //std::cout << std::bitset<8>( ptr->header [ 0 ] ) << std::endl;
    EXPECT_EQ( ( ptr->header [ 0 ] >> 5 ), 0x1);

}

TEST_F( HeaderTest, getTR )
{
    h_->setType( 0x3 );
    h_->setTR( 0x1 );
    h_->setWindow( 22 );
    struct simplepacket *ptr = static_cast< struct simplepacket * > ( h_->thePacket( ) );
    std::cout << "TR: " << std::bitset<8>( h_->getTR( ) ) << std::endl;
    std::cout << "Byte: " << std::bitset<8>( ptr->header [ 0 ] ) << std::endl;
    EXPECT_EQ( h_->getTR(), 0x1 );

}

TEST_F( HeaderTest, setWindow )
{

    h_->setWindow( 0x20 );
    struct simplepacket *ptr = static_cast< struct simplepacket * > ( h_->thePacket( ) );
    EXPECT_EQ( ptr->header [ 0 ], 0x0 );

    h_->setWindow( 0xB ); // 01011
    EXPECT_EQ( ptr->header [ 0 ], 0xB );


}

TEST_F( HeaderTest, getWindow )
{


    h_->setType( 0x3 );
    h_->setTR( 0x1 );
    h_->setWindow( 0xA );




    struct simplepacket *ptr = static_cast< struct simplepacket * > ( h_->thePacket( ) );

    std::cout << "Window: " << std::bitset<8>( h_->getWindow( ) ) << std::endl;
    std::cout << "Byte: " << std::bitset<8>( ptr->header[0] ) << std::endl;
    EXPECT_EQ( h_->getWindow( ), 0xA );


}

TEST_F( HeaderTest, setSequence )
{
    h_->setSequence( 255 );
    struct simplepacket *ptr = static_cast< struct simplepacket * > ( h_->thePacket( ) );
    EXPECT_EQ( ptr->header [ 1 ], 255 );

    h_->setWindow( 256 );
    EXPECT_EQ( ptr->header [ 0 ], 0 );

}

TEST_F( HeaderTest, getSequence )
{
    h_->setSequence( 255 );
    struct simplepacket *ptr = static_cast< struct simplepacket * > ( h_->thePacket( ) );
    std::cout << "Sequence Number: " << std::bitset<8>( h_->getSequence( ) ) << std::endl;
    std::cout << "Byte: " << std::bitset<8>( ptr->header [ 1 ] ) << std::endl;
    EXPECT_EQ( h_->getSequence( ), 255 );

}

TEST_F( HeaderTest, setLength )
{
    h_->setLength( 10 );
    struct simplepacket *ptr = static_cast< struct simplepacket * > ( h_->thePacket( ) );
    EXPECT_EQ( ptr->header [ 2 ], 0x00 );

    h_->setLength( 512 );
    EXPECT_EQ( ptr->header [ 2 ], 0x2 );
    EXPECT_EQ( ptr->header [ 3 ], 0x0 );

}

TEST_F( HeaderTest, getLength )
{
    struct simplepacket *ptr = static_cast< struct simplepacket * > ( h_->thePacket( ) );
    ptr->data[0] = 0x01;
    ptr->data[1] = 0xF1;
    h_->setLength(127);
    std::cout << "Length: " << std::bitset<10>( h_->getLength( ) ) << std::endl;
    std::cout << "Byte(s): " << std::bitset<8>( ptr->header [ 2 ] ) << " " << std::bitset<8>( ptr->header[ 3 ] ) << std::endl;
    EXPECT_EQ( h_->getLength( ), 2 );


}

TEST_F( HeaderTest, setTimestamp )
{
    h_->setTimestamp( 0xFFFF );
    struct simplepacket *ptr = static_cast< struct simplepacket * > ( h_->thePacket( ) );
    EXPECT_EQ( ptr->header [ 4 ], 0x0 );
    EXPECT_EQ( ptr->header [ 5 ], 0x0 );
    EXPECT_EQ( ptr->header [ 6 ], 0xFF );
    EXPECT_EQ( ptr->header [ 7 ], 0xFF );

}

TEST_F( HeaderTest, setPayload)
{
    h_->setPayload( 0xFFFF );
    struct simplepacket *ptr = static_cast< struct simplepacket * > ( h_->thePacket( ) );
    EXPECT_EQ( ptr->header [523], 0xFF);
    EXPECT_EQ( ptr->header [ 524 ], 0xFF );

}

TEST_F( HeaderTest, getPayload)
{
    h_->setPayload( 0xFBFA );
    struct simplepacket *ptr = static_cast< struct simplepacket * > ( h_->thePacket( ) );
    std::cout << "Byte(s): " << std::bitset<8>( ptr->header [ 523 ] ) << std::endl;
    EXPECT_EQ( h_->getPayload( ), 0xFA );

}

TEST_F( HeaderTest, getTimestamp )
{
    h_->setTimestamp( 0xFCAF );
    struct simplepacket *ptr = static_cast< struct simplepacket * > ( h_->thePacket( ) );
    std::cout << "Time: " << std::bitset<32>( h_->getTimestamp( ) ) << std::endl;
    std::cout << "Byte(s): " << std::bitset<8>( ptr->header [ 4 ] ) << " " << std::bitset<8>( ptr->header [ 5 ] ) << " " << std::bitset<8>( ptr->header [ 6 ] ) << " " << std::bitset<8>( ptr->header [ 7 ] ) << std::endl;
    EXPECT_EQ( h_->getTimestamp( ), 0xFCAF );


}

TEST_F( HeaderTest, setCRC1 )
{
    h_->setCRC1( 0xFFFF );
    struct simplepacket *ptr = static_cast< struct simplepacket * > ( h_->thePacket( ) );
    EXPECT_EQ( ptr->header [ 8 ], 0x0 );
    EXPECT_EQ( ptr->header [ 9 ], 0x0 );
    EXPECT_EQ( ptr->header [ 10 ], 0xFF );
    EXPECT_EQ( ptr->header [ 11 ], 0xFF );

}

TEST_F( HeaderTest, getCRC1 )
{
    h_->setCRC1( 0xFAFAFAFA );
    struct simplepacket *ptr = static_cast< struct simplepacket * > ( h_->thePacket( ) );
    std::cout << "CRC1: " << std::bitset<32>( h_->getCRC1( ) ) << std::endl;
    std::cout << "Byte(s): " << std::bitset<8>( ptr->header [ 8 ] ) << " " << std::bitset<8>( ptr->header [ 9 ] ) << " " << std::bitset<8>( ptr->header [ 10 ] ) << " " << std::bitset<8>( ptr->header [ 11 ] ) << std::endl;
    EXPECT_EQ( h_->getCRC1( ), 0xFAFAFAFA );


}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
