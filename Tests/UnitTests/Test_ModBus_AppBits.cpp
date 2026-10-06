/**
 * \author Mr.Nobody
 * \file Test_ModBus_AppBits.cpp
 * \ingroup UnitTests
 * \brief Unit tests of ModBus bit access handlers - Read Coils (0x01), Read
 *        Discrete Inputs (0x02), Write Single Coil (0x05), Write Multiple
 *        Coils (0x0F).
 */

/* ============================= INCLUDES =================================== */
#include "unity.h"                          /* Unity testing framework        */
#include "UtModBus_Helpers.hpp"             /* Test helpers                   */
#include "ModBus_App_RC.hpp"                /* Read Coils                     */
#include "ModBus_App_RDI.hpp"               /* Read Discrete Inputs           */
#include "ModBus_App_WSC.hpp"               /* Write Single Coil              */
#include "ModBus_App_WMC.hpp"               /* Write Multiple Coils           */
/* ========================= SYMBOLIC CONSTANTS ============================= */

/** Count of emulated coils */
static constexpr uint16_t UT_COILS_CNT = 40u;

/* ========================== LOCAL VARIABLES =============================== */

/** Emulated coils */
static modBus_CoilsRetValue_t ut_Coils[ UT_COILS_CNT ];

/** Address of coil with failing callback (-1 = none) */
static int32_t ut_FailingCoil;

/** Count of setter calls */
static uint32_t ut_SetterCnt;

/* ========================== LOCAL FUNCTIONS =============================== */

template< int CoilId >
static modBus_RequestState_t UtGetCoil( modBus_CoilsRetValue_t *value )
{
    *value = ut_Coils[ CoilId ];

    return ( ( CoilId == ut_FailingCoil ) ? MODBUS_REQUEST_ERROR : MODBUS_REQUEST_OK );
}


template< int CoilId >
static modBus_RequestState_t UtSetCoil( modBus_CoilsRetValue_t *value )
{
    ut_Coils[ CoilId ] = *value;
    ut_SetterCnt++;

    return ( ( CoilId == ut_FailingCoil ) ? MODBUS_REQUEST_ERROR : MODBUS_REQUEST_OK );
}

#define UT_COIL( id )       { id, MODBUS_SERVICE_SUPPORTED, UtGetCoil< id >, UtSetCoil< id > }
#define UT_COILS_8( id )    UT_COIL( id ), UT_COIL( id + 1 ), UT_COIL( id + 2 ), UT_COIL( id + 3 ), \
                            UT_COIL( id + 4 ), UT_COIL( id + 5 ), UT_COIL( id + 6 ), UT_COIL( id + 7 )

/** Coils configuration - coil 1 is not supported, coil 2 has no setter, coil 3 no getter */
static const modBus_CoilsConfigStruct_t ut_CoilConfig[ UT_COILS_CNT ] =
{
    UT_COIL( 0 ),
    { 1, MODBUS_SERVICE_NOT_SUPPORTED, UtGetCoil< 1 >, UtSetCoil< 1 > },
    { 2, MODBUS_SERVICE_SUPPORTED,     UtGetCoil< 2 >, nullptr        },
    { 3, MODBUS_SERVICE_SUPPORTED,     nullptr,        UtSetCoil< 3 > },
    UT_COIL( 4 ), UT_COIL( 5 ), UT_COIL( 6 ), UT_COIL( 7 ),
    UT_COILS_8( 8 ), UT_COILS_8( 16 ), UT_COILS_8( 24 ), UT_COILS_8( 32 )
};

/** Discrete inputs configuration (shares emulated values with coils) */
static const modBus_DiscreteIoConfigStruct_t ut_InputConfig[] =
{
    UT_COILS_8( 0 ), UT_COILS_8( 8 ), UT_COILS_8( 16 ), UT_COILS_8( 24 )
};


/** Sets emulated bits from byte (LSB = first bit) */
static void UtSetBits( uint16_t firstBit, uint8_t bitsCnt, uint8_t value )
{
    for( uint8_t bitIndex = 0u; bitsCnt > bitIndex; bitIndex++ )
    {
        ut_Coils[ firstBit + bitIndex ] = ( 0u != ( value & ( 1u << bitIndex ) ) ) ? MODBUS_COIL_ACTIVE : MODBUS_COIL_INACTIVE;
    }
}

/* ======================== EXPORTED FUNCTIONS ============================== */

void setUp( void )
{
    for( uint16_t coilIndex = 0u; UT_COILS_CNT > coilIndex; coilIndex++ )
    {
        ut_Coils[ coilIndex ] = MODBUS_COIL_INACTIVE;
    }

    ut_FailingCoil = -1;
    ut_SetterCnt   = 0u;
}


void tearDown( void )
{
}

/* ------------------------------ Read Coils -------------------------------- */

/** Example from ModBus specification - read coils 20-38 */
void Ut_RC_SpecificationExample( void )
{
    ModBus::AppLayer::RC rc( ut_CoilConfig, UT_COILS_CNT );
    uint8_t              buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t   respLen   = 0u;
    const uint8_t        request[] = { UtModBus::SlaveAddr, 0x01, 0x00, 0x13, 0x00, 0x13 };
    const uint8_t        expected[] = { UtModBus::SlaveAddr, 0x01, 0x03, 0xCD, 0x6B, 0x05 };

    UtSetBits( 19u, 8u, 0xCDu );
    UtSetBits( 27u, 8u, 0x6Bu );
    UtSetBits( 35u, 3u, 0x05u );

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( rc, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_UINT16( sizeof( expected ), respLen );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( expected, buffer, sizeof( expected ) );
}


/** Exactly 8 coils fit into one byte */
void Ut_RC_EightCoils_OneByte( void )
{
    ModBus::AppLayer::RC rc( ut_CoilConfig, UT_COILS_CNT );
    uint8_t              buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t   respLen   = 0u;
    const uint8_t        request[] = { UtModBus::SlaveAddr, 0x01, 0x00, 0x08, 0x00, 0x08 };

    UtSetBits( 8u, 8u, 0x81u );

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( rc, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_UINT16( 4u, respLen );
    TEST_ASSERT_EQUAL_HEX8( 0x01u, buffer[ 2 ] );
    TEST_ASSERT_EQUAL_HEX8( 0x81u, buffer[ 3 ] );
}


void Ut_RC_QuantityOutOfRange_IllegalDataValue( void )
{
    ModBus::AppLayer::RC rc( ut_CoilConfig, UT_COILS_CNT );
    uint8_t              buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t   respLen   = 0u;
    const uint8_t        request0[]    = { UtModBus::SlaveAddr, 0x01, 0x00, 0x00, 0x00, 0x00 };
    const uint8_t        request2001[] = { UtModBus::SlaveAddr, 0x01, 0x00, 0x00, 0x07, 0xD1 };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE, UtModBus::Decode( rc, request0,    sizeof( request0 ),    buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE, UtModBus::Decode( rc, request2001, sizeof( request2001 ), buffer, respLen ) );
}


void Ut_RC_RequestLength_IllegalDataValue( void )
{
    ModBus::AppLayer::RC rc( ut_CoilConfig, UT_COILS_CNT );
    uint8_t              buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t   respLen   = 0u;
    const uint8_t        request[] = { UtModBus::SlaveAddr, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00 };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE, UtModBus::Decode( rc, request, sizeof( request ),      buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE, UtModBus::Decode( rc, request, sizeof( request ) - 2u, buffer, respLen ) );
}


void Ut_RC_AddressRange_IllegalDataAddress( void )
{
    ModBus::AppLayer::RC rc( ut_CoilConfig, UT_COILS_CNT );
    uint8_t              buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t   respLen   = 0u;
    const uint8_t        lastOk[]        = { UtModBus::SlaveAddr, 0x01, 0x00, 0x27, 0x00, 0x01 };
    const uint8_t        behindEnd[]     = { UtModBus::SlaveAddr, 0x01, 0x00, 0x27, 0x00, 0x02 };
    const uint8_t        notSupported[]  = { UtModBus::SlaveAddr, 0x01, 0x00, 0x00, 0x00, 0x02 };
    const uint8_t        noGetter[]      = { UtModBus::SlaveAddr, 0x01, 0x00, 0x03, 0x00, 0x01 };
    const uint8_t        addrOverflow[]  = { UtModBus::SlaveAddr, 0x01, 0xFF, 0xFF, 0x00, 0x02 };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR,      UtModBus::Decode( rc, lastOk,       sizeof( lastOk ),       buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_ADDRESS, UtModBus::Decode( rc, behindEnd,    sizeof( behindEnd ),    buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_ADDRESS, UtModBus::Decode( rc, notSupported, sizeof( notSupported ), buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_ADDRESS, UtModBus::Decode( rc, noGetter,     sizeof( noGetter ),     buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_ADDRESS, UtModBus::Decode( rc, addrOverflow, sizeof( addrOverflow ), buffer, respLen ) );
}


void Ut_RC_GetterFailure_SlaveDeviceFailure( void )
{
    ModBus::AppLayer::RC rc( ut_CoilConfig, UT_COILS_CNT );
    uint8_t              buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t   respLen   = 0u;
    const uint8_t        request[] = { UtModBus::SlaveAddr, 0x01, 0x00, 0x04, 0x00, 0x04 };

    ut_FailingCoil = 6;

    TEST_ASSERT_EQUAL( MODBUS_ERROR_SLAVE_DEVICE_FAILURE, UtModBus::Decode( rc, request, sizeof( request ), buffer, respLen ) );
}


void Ut_RC_NullConfiguration_IllegalDataAddress( void )
{
    ModBus::AppLayer::RC rc( nullptr, 10u );
    uint8_t              buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t   respLen   = 0u;
    const uint8_t        request[] = { UtModBus::SlaveAddr, 0x01, 0x00, 0x00, 0x00, 0x01 };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_ADDRESS, UtModBus::Decode( rc, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_FUNCTION_RC, rc.GetFunctionCode() );
    TEST_ASSERT_EQUAL( MODBUS_FUNCTION_EQUAL,     rc.GetFunctionStatus( 0x01u ) );
    TEST_ASSERT_EQUAL( MODBUS_FUNCTION_INCORRECT, rc.GetFunctionStatus( 0x02u ) );
}

/* ------------------------- Read Discrete Inputs --------------------------- */

/** Example from ModBus specification (shifted) - read 22 inputs */
void Ut_RDI_ReadInputs( void )
{
    ModBus::AppLayer::RDI rdi( ut_InputConfig, 32u );
    uint8_t               buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t    respLen   = 0u;
    const uint8_t         request[]  = { UtModBus::SlaveAddr, 0x02, 0x00, 0x04, 0x00, 0x16 };
    const uint8_t         expected[] = { UtModBus::SlaveAddr, 0x02, 0x03, 0xAC, 0xDB, 0x35 };

    UtSetBits(  4u, 8u, 0xACu );
    UtSetBits( 12u, 8u, 0xDBu );
    UtSetBits( 20u, 6u, 0x35u );

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( rdi, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_UINT16( sizeof( expected ), respLen );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( expected, buffer, sizeof( expected ) );
    TEST_ASSERT_EQUAL( MODBUS_FUNCTION_RDI, rdi.GetFunctionCode() );
}


void Ut_RDI_AddressRange_IllegalDataAddress( void )
{
    ModBus::AppLayer::RDI rdi( ut_InputConfig, 32u );
    uint8_t               buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t    respLen   = 0u;
    const uint8_t         request[] = { UtModBus::SlaveAddr, 0x02, 0x00, 0x1F, 0x00, 0x02 };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_ADDRESS, UtModBus::Decode( rdi, request, sizeof( request ), buffer, respLen ) );
}

/* --------------------------- Write Single Coil ---------------------------- */

void Ut_WSC_SetOnAndOff_Echo( void )
{
    ModBus::AppLayer::WSC wsc( ut_CoilConfig, UT_COILS_CNT );
    uint8_t               buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t    respLen = 0u;
    const uint8_t         on[]  = { UtModBus::SlaveAddr, 0x05, 0x00, 0x1D, 0xFF, 0x00 };
    const uint8_t         off[] = { UtModBus::SlaveAddr, 0x05, 0x00, 0x1D, 0x00, 0x00 };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( wsc, on, sizeof( on ), buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_COIL_ACTIVE, ut_Coils[ 29 ] );
    TEST_ASSERT_EQUAL_UINT16( sizeof( on ), respLen );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( on, buffer, sizeof( on ) );

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( wsc, off, sizeof( off ), buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_COIL_INACTIVE, ut_Coils[ 29 ] );
}


/** Value other than 0xFF00 / 0x0000 is rejected without coil change */
void Ut_WSC_InvalidValue_IllegalDataValue( void )
{
    ModBus::AppLayer::WSC wsc( ut_CoilConfig, UT_COILS_CNT );
    uint8_t               buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t    respLen   = 0u;
    const uint8_t         request[] = { UtModBus::SlaveAddr, 0x05, 0x00, 0x05, 0x00, 0x01 };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE, UtModBus::Decode( wsc, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_UINT32( 0u, ut_SetterCnt );
}


void Ut_WSC_Address_IllegalDataAddress( void )
{
    ModBus::AppLayer::WSC wsc( ut_CoilConfig, UT_COILS_CNT );
    uint8_t               buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t    respLen = 0u;
    const uint8_t         outOfRange[] = { UtModBus::SlaveAddr, 0x05, 0x00, 0x28, 0xFF, 0x00 };
    const uint8_t         noSetter[]   = { UtModBus::SlaveAddr, 0x05, 0x00, 0x02, 0xFF, 0x00 };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_ADDRESS, UtModBus::Decode( wsc, outOfRange, sizeof( outOfRange ), buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_ADDRESS, UtModBus::Decode( wsc, noSetter,   sizeof( noSetter ),   buffer, respLen ) );
}


void Ut_WSC_SetterFailure_SlaveDeviceFailure( void )
{
    ModBus::AppLayer::WSC wsc( ut_CoilConfig, UT_COILS_CNT );
    uint8_t               buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t    respLen   = 0u;
    const uint8_t         request[] = { UtModBus::SlaveAddr, 0x05, 0x00, 0x05, 0xFF, 0x00 };

    ut_FailingCoil = 5;

    TEST_ASSERT_EQUAL( MODBUS_ERROR_SLAVE_DEVICE_FAILURE, UtModBus::Decode( wsc, request, sizeof( request ), buffer, respLen ) );
}

/* ------------------------- Write Multiple Coils --------------------------- */

/** Example from ModBus specification - write 10 coils starting at coil 20 */
void Ut_WMC_SpecificationExample( void )
{
    ModBus::AppLayer::WMC wmc( ut_CoilConfig, UT_COILS_CNT );
    uint8_t               buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t    respLen    = 0u;
    const uint8_t         request[]  = { UtModBus::SlaveAddr, 0x0F, 0x00, 0x13, 0x00, 0x0A, 0x02, 0xCD, 0x01 };
    const uint8_t         expected[] = { UtModBus::SlaveAddr, 0x0F, 0x00, 0x13, 0x00, 0x0A };
    const modBus_CoilsRetValue_t expectedCoils[] =
    {
        MODBUS_COIL_ACTIVE,   MODBUS_COIL_INACTIVE, MODBUS_COIL_ACTIVE,   MODBUS_COIL_ACTIVE,
        MODBUS_COIL_INACTIVE, MODBUS_COIL_INACTIVE, MODBUS_COIL_ACTIVE,   MODBUS_COIL_ACTIVE,
        MODBUS_COIL_ACTIVE,   MODBUS_COIL_INACTIVE
    };

    ut_Coils[ 29 ] = MODBUS_COIL_ACTIVE;

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( wmc, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_UINT16( sizeof( expected ), respLen );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( expected, buffer, sizeof( expected ) );

    for( uint8_t coilIndex = 0u; 10u > coilIndex; coilIndex++ )
    {
        TEST_ASSERT_EQUAL( expectedCoils[ coilIndex ], ut_Coils[ 19u + coilIndex ] );
    }

    /* Coil behind the range is not changed */
    TEST_ASSERT_EQUAL( MODBUS_COIL_ACTIVE, ut_Coils[ 29 ] );
    TEST_ASSERT_EQUAL_UINT32( 10u, ut_SetterCnt );
}


void Ut_WMC_ByteCountMismatch_IllegalDataValue( void )
{
    ModBus::AppLayer::WMC wmc( ut_CoilConfig, UT_COILS_CNT );
    uint8_t               buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t    respLen = 0u;
    const uint8_t         wrongByteCount[] = { UtModBus::SlaveAddr, 0x0F, 0x00, 0x13, 0x00, 0x0A, 0x01, 0xCD };
    const uint8_t         missingData[]    = { UtModBus::SlaveAddr, 0x0F, 0x00, 0x13, 0x00, 0x0A, 0x02, 0xCD };
    const uint8_t         quantityZero[]   = { UtModBus::SlaveAddr, 0x0F, 0x00, 0x13, 0x00, 0x00, 0x00 };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE, UtModBus::Decode( wmc, wrongByteCount, sizeof( wrongByteCount ), buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE, UtModBus::Decode( wmc, missingData,    sizeof( missingData ),    buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE, UtModBus::Decode( wmc, quantityZero,   sizeof( quantityZero ),   buffer, respLen ) );
    TEST_ASSERT_EQUAL_UINT32( 0u, ut_SetterCnt );
}


/** Invalid address in the range - no coil is written */
void Ut_WMC_AddressError_NoSideEffects( void )
{
    ModBus::AppLayer::WMC wmc( ut_CoilConfig, UT_COILS_CNT );
    uint8_t               buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t    respLen   = 0u;
    const uint8_t         request[] = { UtModBus::SlaveAddr, 0x0F, 0x00, 0x24, 0x00, 0x05, 0x01, 0x1F };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_ADDRESS, UtModBus::Decode( wmc, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_UINT32( 0u, ut_SetterCnt );
}

