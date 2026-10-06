/**
 * \author Mr.Nobody
 * \file Test_ModBus_Crc.cpp
 * \ingroup UnitTests
 * \brief Unit tests of ModBus RTU CRC calculation.
 */

/* ============================= INCLUDES =================================== */
#include "unity.h"                          /* Unity testing framework        */
#include "ModBus_Crc.hpp"                   /* Code under test                */
/* ======================== EXPORTED FUNCTIONS ============================== */

void setUp( void )
{
}


void tearDown( void )
{
}


/** CRC-16/MODBUS check value of "123456789" is 0x4B37 */
void Ut_Crc_Get_StandardCheckValue( void )
{
    const ModBus::Crc crc;
    const uint8_t     data[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9' };

    TEST_ASSERT_EQUAL_HEX16( 0x4B37u, crc.Get_Crc( data, sizeof( data ) ) );
}


/** Known request "01 03 00 00 00 0A" has CRC bytes C5 CD */
void Ut_Crc_Get_KnownFrame( void )
{
    const ModBus::Crc crc;
    const uint8_t     data[] = { 0x01, 0x03, 0x00, 0x00, 0x00, 0x0A };

    TEST_ASSERT_EQUAL_HEX16( 0xCDC5u, crc.Get_Crc( data, sizeof( data ) ) );
}


void Ut_Crc_Get_EmptyBufferReturnsInitValue( void )
{
    const ModBus::Crc crc;
    const uint8_t     data[] = { 0x00 };

    TEST_ASSERT_EQUAL_HEX16( 0xFFFFu, crc.Get_Crc( data, 0u ) );
}


void Ut_Crc_Get_NullPointerReturnsZero( void )
{
    const ModBus::Crc crc;

    TEST_ASSERT_EQUAL_HEX16( 0u, crc.Get_Crc( nullptr, 5u ) );
}


/** Add_Crc appends CRC behind the data (low byte first) and keeps data intact */
void Ut_Crc_Add_AppendsCrcBehindData( void )
{
    const ModBus::Crc  crc;
    uint8_t            data[ 8 ] = { 0x01, 0x03, 0x00, 0x00, 0x00, 0x0A, 0xAA, 0xAA };
    modBus_MsgLength_t len       = 6u;

    crc.Add_Crc( data, &len );

    const uint8_t expected[] = { 0x01, 0x03, 0x00, 0x00, 0x00, 0x0A, 0xC5, 0xCD };

    TEST_ASSERT_EQUAL_UINT16( 8u, len );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( expected, data, sizeof( expected ) );
}


void Ut_Crc_Add_NullPointersIgnored( void )
{
    const ModBus::Crc  crc;
    uint8_t            data[ 4 ] = { 0x01, 0x02, 0x00, 0x00 };
    modBus_MsgLength_t len       = 2u;

    crc.Add_Crc( nullptr, &len );
    crc.Add_Crc( data, nullptr );

    TEST_ASSERT_EQUAL_UINT16( 2u, len );
}


void Ut_Crc_Check_ValidFrame( void )
{
    const ModBus::Crc crc;
    const uint8_t     frame[] = { 0x01, 0x03, 0x00, 0x00, 0x00, 0x0A, 0xC5, 0xCD };

    TEST_ASSERT_EQUAL( MODBUS_CRC_OK, crc.Check_Crc( frame, sizeof( frame ) ) );
}


void Ut_Crc_Check_CorruptedFrame( void )
{
    const ModBus::Crc crc;
    const uint8_t     frame[] = { 0x01, 0x03, 0x00, 0x01, 0x00, 0x0A, 0xC5, 0xCD };

    TEST_ASSERT_EQUAL( MODBUS_CRC_INCORRECT, crc.Check_Crc( frame, sizeof( frame ) ) );
}


void Ut_Crc_Check_SwappedCrcBytes( void )
{
    const ModBus::Crc crc;
    const uint8_t     frame[] = { 0x01, 0x03, 0x00, 0x00, 0x00, 0x0A, 0xCD, 0xC5 };

    TEST_ASSERT_EQUAL( MODBUS_CRC_INCORRECT, crc.Check_Crc( frame, sizeof( frame ) ) );
}


void Ut_Crc_Check_TooShortOrNull( void )
{
    const ModBus::Crc crc;
    const uint8_t     frame[] = { 0xFF, 0xFF };

    TEST_ASSERT_EQUAL( MODBUS_CRC_INCORRECT, crc.Check_Crc( frame, 2u ) );
    TEST_ASSERT_EQUAL( MODBUS_CRC_INCORRECT, crc.Check_Crc( frame, 0u ) );
    TEST_ASSERT_EQUAL( MODBUS_CRC_INCORRECT, crc.Check_Crc( nullptr, 8u ) );
}

