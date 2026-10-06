/**
 * \author Mr.Nobody
 * \file Test_ModBus_AppFile.cpp
 * \ingroup UnitTests
 * \brief Unit tests of ModBus file record access handlers - Read File Record
 *        (0x14), Write File Record (0x15).
 */

/* ============================= INCLUDES =================================== */
#include "unity.h"                          /* Unity testing framework        */
#include "UtModBus_Helpers.hpp"             /* Test helpers                   */
#include "ModBus_App_RFR.hpp"               /* Read File Record               */
#include "ModBus_App_WFR.hpp"               /* Write File Record              */
/* ========================= SYMBOLIC CONSTANTS ============================= */

/** Count of emulated files (file numbers 1 - 4) */
static constexpr uint16_t UT_FILES_CNT = 4u;

/** Count of emulated records in every file */
static constexpr uint16_t UT_RECORDS_CNT = 16u;

/* ========================== LOCAL VARIABLES =============================== */

/** Emulated files */
static uint16_t ut_Files[ UT_FILES_CNT ][ UT_RECORDS_CNT ];

/** Count of callback calls */
static uint32_t ut_CallbackCnt;

/* ========================== LOCAL FUNCTIONS =============================== */

static bool UtRecordsExist( uint16_t fileNumber, uint16_t recordNumber, uint16_t recordLength )
{
    return ( ( 0u != fileNumber ) && ( UT_FILES_CNT >= fileNumber ) &&
             ( UT_RECORDS_CNT >= ( (uint32_t)recordNumber + recordLength ) ) );
}


static modBus_RequestState_t UtReadRecords( uint16_t fileNumber, uint16_t recordNumber,
                                            modBus_RegistersValue_t *values, uint16_t recordLength )
{
    modBus_RequestState_t retState = MODBUS_REQUEST_ERROR;

    ut_CallbackCnt++;

    if( UtRecordsExist( fileNumber, recordNumber, recordLength ) )
    {
        for( uint16_t recordIndex = 0u; recordLength > recordIndex; recordIndex++ )
        {
            values[ recordIndex ] = ut_Files[ fileNumber - 1u ][ recordNumber + recordIndex ];
        }

        retState = MODBUS_REQUEST_OK;
    }
    else
    {
        /* Records do not exist */
    }

    return ( retState );
}


static modBus_RequestState_t UtWriteRecords( uint16_t fileNumber, uint16_t recordNumber,
                                             modBus_RegistersValue_t *values, uint16_t recordLength )
{
    modBus_RequestState_t retState = MODBUS_REQUEST_ERROR;

    ut_CallbackCnt++;

    if( UtRecordsExist( fileNumber, recordNumber, recordLength ) )
    {
        for( uint16_t recordIndex = 0u; recordLength > recordIndex; recordIndex++ )
        {
            ut_Files[ fileNumber - 1u ][ recordNumber + recordIndex ] = values[ recordIndex ];
        }

        retState = MODBUS_REQUEST_OK;
    }
    else
    {
        /* Records do not exist */
    }

    return ( retState );
}

/* ======================== EXPORTED FUNCTIONS ============================== */

void setUp( void )
{
    for( uint16_t fileIndex = 0u; UT_FILES_CNT > fileIndex; fileIndex++ )
    {
        for( uint16_t recordIndex = 0u; UT_RECORDS_CNT > recordIndex; recordIndex++ )
        {
            ut_Files[ fileIndex ][ recordIndex ] = (uint16_t)( ( fileIndex << 8u ) | recordIndex );
        }
    }

    ut_CallbackCnt = 0u;
}


void tearDown( void )
{
}

/* ---------------------------- Read File Record ---------------------------- */

/** Example from ModBus specification - two groups from files 4 and 3 */
void Ut_RFR_SpecificationExample( void )
{
    ModBus::AppLayer::RFR rfr( UtReadRecords );
    uint8_t               buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t    respLen   = 0u;
    const uint8_t         request[] = { UtModBus::SlaveAddr, 0x14, 0x0E,
                                        0x06, 0x00, 0x04, 0x00, 0x01, 0x00, 0x02,
                                        0x06, 0x00, 0x03, 0x00, 0x09, 0x00, 0x02 };
    const uint8_t         expected[] = { UtModBus::SlaveAddr, 0x14, 0x0C,
                                         0x05, 0x06, 0x0D, 0xFE, 0x00, 0x20,
                                         0x05, 0x06, 0x33, 0xCD, 0x00, 0x40 };

    ut_Files[ 3 ][ 1 ]  = 0x0DFEu;
    ut_Files[ 3 ][ 2 ]  = 0x0020u;
    ut_Files[ 2 ][ 9 ]  = 0x33CDu;
    ut_Files[ 2 ][ 10 ] = 0x0040u;

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( rfr, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_UINT16( sizeof( expected ), respLen );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( expected, buffer, sizeof( expected ) );
}


void Ut_RFR_InvalidStructure_IllegalDataValue( void )
{
    ModBus::AppLayer::RFR rfr( UtReadRecords );
    uint8_t               buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t    respLen = 0u;
    const uint8_t         notMultipleOf7[] = { UtModBus::SlaveAddr, 0x14, 0x08,
                                               0x06, 0x00, 0x04, 0x00, 0x01, 0x00, 0x02, 0x00 };
    const uint8_t         lengthMismatch[] = { UtModBus::SlaveAddr, 0x14, 0x0E,
                                               0x06, 0x00, 0x04, 0x00, 0x01, 0x00, 0x02 };
    const uint8_t         byteCountZero[]  = { UtModBus::SlaveAddr, 0x14, 0x00 };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE, UtModBus::Decode( rfr, notMultipleOf7, sizeof( notMultipleOf7 ), buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE, UtModBus::Decode( rfr, lengthMismatch, sizeof( lengthMismatch ), buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE, UtModBus::Decode( rfr, byteCountZero,  sizeof( byteCountZero ),  buffer, respLen ) );
    TEST_ASSERT_EQUAL_UINT32( 0u, ut_CallbackCnt );
}


void Ut_RFR_InvalidReference_IllegalDataAddress( void )
{
    ModBus::AppLayer::RFR rfr( UtReadRecords );
    uint8_t               buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t    respLen = 0u;
    const uint8_t         refType5[]     = { UtModBus::SlaveAddr, 0x14, 0x07, 0x05, 0x00, 0x04, 0x00, 0x01, 0x00, 0x02 };
    const uint8_t         file0[]        = { UtModBus::SlaveAddr, 0x14, 0x07, 0x06, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02 };
    const uint8_t         record10000[]  = { UtModBus::SlaveAddr, 0x14, 0x07, 0x06, 0x00, 0x01, 0x27, 0x0F, 0x00, 0x02 };
    const uint8_t         unknownFile[]  = { UtModBus::SlaveAddr, 0x14, 0x07, 0x06, 0x00, 0x09, 0x00, 0x01, 0x00, 0x02 };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_ADDRESS, UtModBus::Decode( rfr, refType5,    sizeof( refType5 ),    buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_ADDRESS, UtModBus::Decode( rfr, file0,       sizeof( file0 ),       buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_ADDRESS, UtModBus::Decode( rfr, record10000, sizeof( record10000 ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_UINT32( 0u, ut_CallbackCnt );

    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_ADDRESS, UtModBus::Decode( rfr, unknownFile, sizeof( unknownFile ), buffer, respLen ) );
}


/** Response longer than PDU is rejected */
void Ut_RFR_ResponseTooLong_IllegalDataValue( void )
{
    ModBus::AppLayer::RFR rfr( UtReadRecords );
    uint8_t               buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t    respLen   = 0u;
    /* 2 + (2 + 2 x 100) + (2 + 2 x 25) = 256 > 253 */
    const uint8_t         request[] = { UtModBus::SlaveAddr, 0x14, 0x0E,
                                        0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64,
                                        0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 0x19 };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE, UtModBus::Decode( rfr, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_UINT32( 0u, ut_CallbackCnt );
}

/* --------------------------- Write File Record ---------------------------- */

/** Example from ModBus specification - write 3 records into file 4 starting at record 7 */
void Ut_WFR_SpecificationExample( void )
{
    ModBus::AppLayer::WFR wfr( UtWriteRecords );
    uint8_t               buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t    respLen   = 0u;
    const uint8_t         request[] = { UtModBus::SlaveAddr, 0x15, 0x0D,
                                        0x06, 0x00, 0x04, 0x00, 0x07, 0x00, 0x03, 0x06, 0xAF, 0x04, 0xBE, 0x10, 0x0D };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( wfr, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_UINT16( sizeof( request ), respLen );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( request, buffer, sizeof( request ) );
    TEST_ASSERT_EQUAL_HEX16( 0x06AFu, ut_Files[ 3 ][ 7 ] );
    TEST_ASSERT_EQUAL_HEX16( 0x04BEu, ut_Files[ 3 ][ 8 ] );
    TEST_ASSERT_EQUAL_HEX16( 0x100Du, ut_Files[ 3 ][ 9 ] );
}


void Ut_WFR_TwoGroups( void )
{
    ModBus::AppLayer::WFR wfr( UtWriteRecords );
    uint8_t               buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t    respLen   = 0u;
    const uint8_t         request[] = { UtModBus::SlaveAddr, 0x15, 0x14,
                                        0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x11, 0x11,
                                        0x06, 0x00, 0x02, 0x00, 0x05, 0x00, 0x02, 0x22, 0x22, 0x33, 0x33 };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( wfr, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_UINT32( 2u, ut_CallbackCnt );
    TEST_ASSERT_EQUAL_HEX16( 0x1111u, ut_Files[ 0 ][ 0 ] );
    TEST_ASSERT_EQUAL_HEX16( 0x2222u, ut_Files[ 1 ][ 5 ] );
    TEST_ASSERT_EQUAL_HEX16( 0x3333u, ut_Files[ 1 ][ 6 ] );
}


/** Invalid second group - nothing is written */
void Ut_WFR_InvalidGroup_NoSideEffects( void )
{
    ModBus::AppLayer::WFR wfr( UtWriteRecords );
    uint8_t               buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t    respLen = 0u;
    const uint8_t         badRefType[] = { UtModBus::SlaveAddr, 0x15, 0x12,
                                           0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x11, 0x11,
                                           0x07, 0x00, 0x02, 0x00, 0x05, 0x00, 0x01, 0x22, 0x22 };
    const uint8_t         badLength[]  = { UtModBus::SlaveAddr, 0x15, 0x12,
                                           0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x11, 0x11,
                                           0x06, 0x00, 0x02, 0x00, 0x05, 0x00, 0x02, 0x22, 0x22 };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_ADDRESS, UtModBus::Decode( wfr, badRefType, sizeof( badRefType ), buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE,   UtModBus::Decode( wfr, badLength,  sizeof( badLength ),  buffer, respLen ) );
    TEST_ASSERT_EQUAL_UINT32( 0u, ut_CallbackCnt );
    TEST_ASSERT_EQUAL_HEX16( 0x0000u, ut_Files[ 0 ][ 0 ] );
}


void Ut_WFR_CallbackError_IllegalDataAddress( void )
{
    ModBus::AppLayer::WFR wfr( UtWriteRecords );
    ModBus::AppLayer::WFR wfrNoCallback( nullptr );
    uint8_t               buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t    respLen   = 0u;
    const uint8_t         request[] = { UtModBus::SlaveAddr, 0x15, 0x09,
                                        0x06, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x11, 0x11 };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_ADDRESS, UtModBus::Decode( wfr,           request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_ADDRESS, UtModBus::Decode( wfrNoCallback, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE,   UtModBus::Decode( wfr,           request, 3u,                buffer, respLen ) );
}

