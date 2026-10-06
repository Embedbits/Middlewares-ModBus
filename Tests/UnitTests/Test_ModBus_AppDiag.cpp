/**
 * \author Mr.Nobody
 * \file Test_ModBus_AppDiag.cpp
 * \ingroup UnitTests
 * \brief Unit tests of ModBus diagnostics and identification handlers - Read
 *        Exception Status (0x07), Diagnostics (0x08), Get Comm Event Counter
 *        (0x0B), Get Comm Event Log (0x0C), Report Slave ID (0x11), Read
 *        Device Identification (0x2B / 0x0E).
 */

/* ============================= INCLUDES =================================== */
#include "unity.h"                          /* Unity testing framework        */
#include "UtModBus_Helpers.hpp"             /* Test helpers                   */
#include "ModBus_App_RES.hpp"               /* Read Exception Status          */
#include "ModBus_App_DIAG.hpp"              /* Diagnostics                    */
#include "ModBus_App_GCEC.hpp"              /* Get Comm Event Counter         */
#include "ModBus_App_GCEL.hpp"              /* Get Comm Event Log             */
#include "ModBus_App_RSID.hpp"              /* Report Slave ID                */
#include "ModBus_App_RDID.hpp"              /* Read Device Identification     */
#include <string.h>                         /* String functions               */
/* ========================== LOCAL VARIABLES =============================== */

/** Emulated exception status */
static uint8_t ut_ExceptionStatus;

/** Exception status callback result */
static modBus_RequestState_t ut_ExceptionStatusResult;

/** Additional data of Report Slave ID */
static const uint8_t ut_AddData[] = { 'E', 'm', 'B', 'i' };

/** Length of private object 0x80 */
static uint8_t ut_PrivateObjLen;

/** Buffer of private object 0x80 */
static uint8_t ut_PrivateObj[ 255 ];

/* ========================== LOCAL FUNCTIONS =============================== */

static modBus_RequestState_t UtExceptionStatus( uint8_t *exceptionStatus )
{
    *exceptionStatus = ut_ExceptionStatus;

    return ( ut_ExceptionStatusResult );
}


static modBus_App_RunStatus_t UtRunStatus( void )
{
    return ( MODBUS_APP_RUN_ACTIVE );
}


static modBus_RequestState_t UtVendorName( const uint8_t **objData, uint8_t *objLength )
{
    static const char vendorName[] = "Company identification";

    *objData   = (const uint8_t *)vendorName;
    *objLength = (uint8_t)( sizeof( vendorName ) - 1u );

    return ( MODBUS_REQUEST_OK );
}


static modBus_RequestState_t UtProductCode( const uint8_t **objData, uint8_t *objLength )
{
    static const char productCode[] = "Product code XX";

    *objData   = (const uint8_t *)productCode;
    *objLength = (uint8_t)( sizeof( productCode ) - 1u );

    return ( MODBUS_REQUEST_OK );
}


static modBus_RequestState_t UtRevision( const uint8_t **objData, uint8_t *objLength )
{
    static const char revision[] = "V2.11";

    *objData   = (const uint8_t *)revision;
    *objLength = (uint8_t)( sizeof( revision ) - 1u );

    return ( MODBUS_REQUEST_OK );
}


static modBus_RequestState_t UtProductName( const uint8_t **objData, uint8_t *objLength )
{
    static const char productName[] = "PN";

    *objData   = (const uint8_t *)productName;
    *objLength = (uint8_t)( sizeof( productName ) - 1u );

    return ( MODBUS_REQUEST_OK );
}


static modBus_RequestState_t UtPrivateObject( const uint8_t **objData, uint8_t *objLength )
{
    *objData   = ut_PrivateObj;
    *objLength = ut_PrivateObjLen;

    return ( MODBUS_REQUEST_OK );
}


static modBus_RequestState_t UtFailingObject( const uint8_t **objData, uint8_t *objLength )
{
    (void)objData;
    (void)objLength;

    return ( MODBUS_REQUEST_ERROR );
}


/** Basic device identification only */
static const modBus_App_RDID_t ut_BasicObjects[] =
{
    { MODBUS_RDID_ID_BASIC_VENDOR_NAME,  UtVendorName  },
    { MODBUS_RDID_ID_BASIC_PRODUCT_CODE, UtProductCode },
    { MODBUS_RDID_ID_BASIC_SW_VERSION,   UtRevision    },
};


/** Extended device identification (not ordered on purpose) */
static const modBus_App_RDID_t ut_ExtendedObjects[] =
{
    { 0x80,                                UtPrivateObject },
    { MODBUS_RDID_ID_BASIC_VENDOR_NAME,    UtVendorName    },
    { MODBUS_RDID_ID_BASIC_PRODUCT_CODE,   UtProductCode   },
    { MODBUS_RDID_ID_BASIC_SW_VERSION,     UtRevision      },
    { MODBUS_RDID_ID_REGULAR_PRODUCT_NAME, UtProductName   },
    { 0x81,                                UtFailingObject },
};


/** Diagnostics test fixture - transport layer providing bus counters */
struct UtDiagFixture
{
    UtModBus::FakePhy                 phy;
    ModBus::Slave::TransportLayer     tl{ phy };
    ModBus::AppLayer::CommDiagnostics diag{ tl };
};

/* ======================== EXPORTED FUNCTIONS ============================== */

void setUp( void )
{
    ut_ExceptionStatus       = 0x6Du;
    ut_ExceptionStatusResult = MODBUS_REQUEST_OK;
    ut_PrivateObjLen         = 4u;

    memset( ut_PrivateObj, 0xA5, sizeof( ut_PrivateObj ) );
}


void tearDown( void )
{
}

/* ------------------------- Read Exception Status -------------------------- */

void Ut_RES_ReadStatus( void )
{
    ModBus::AppLayer::RES res( UtExceptionStatus );
    uint8_t               buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t    respLen    = 0u;
    const uint8_t         request[]  = { UtModBus::SlaveAddr, 0x07 };
    const uint8_t         expected[] = { UtModBus::SlaveAddr, 0x07, 0x6D };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( res, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_UINT16( sizeof( expected ), respLen );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( expected, buffer, sizeof( expected ) );
}


void Ut_RES_Errors( void )
{
    ModBus::AppLayer::RES res( UtExceptionStatus );
    ModBus::AppLayer::RES resNoCallback( nullptr );
    uint8_t               buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t    respLen   = 0u;
    const uint8_t         request[] = { UtModBus::SlaveAddr, 0x07, 0x00 };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE,   UtModBus::Decode( res,           request, 3u, buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_SLAVE_DEVICE_FAILURE, UtModBus::Decode( resNoCallback, request, 2u, buffer, respLen ) );

    ut_ExceptionStatusResult = MODBUS_REQUEST_ERROR;
    TEST_ASSERT_EQUAL( MODBUS_ERROR_SLAVE_DEVICE_FAILURE, UtModBus::Decode( res,           request, 2u, buffer, respLen ) );
}

/* ------------------------------ Diagnostics ------------------------------- */

void Ut_DIAG_ReturnQueryData_Echo( void )
{
    UtDiagFixture          fx;
    ModBus::AppLayer::DIAG diag( fx.diag );
    uint8_t                buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t     respLen   = 0u;
    const uint8_t          request[] = { UtModBus::SlaveAddr, 0x08, 0x00, 0x00, 0xA5, 0x37, 0x12, 0x34 };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( diag, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_UINT16( sizeof( request ), respLen );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( request, buffer, sizeof( request ) );
}


void Ut_DIAG_ReturnDiagnosticRegister( void )
{
    UtDiagFixture          fx;
    ModBus::AppLayer::DIAG diag( fx.diag );
    uint8_t                buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t     respLen    = 0u;
    const uint8_t          request[]  = { UtModBus::SlaveAddr, 0x08, 0x00, 0x02, 0x00, 0x00 };
    const uint8_t          expected[] = { UtModBus::SlaveAddr, 0x08, 0x00, 0x02, 0xBE, 0xEF };

    fx.diag.Set_DiagnosticRegister( 0xBEEFu );

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( diag, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( expected, buffer, sizeof( expected ) );
}


/** Clear counters clears all counters, event counter and diagnostic register */
void Ut_DIAG_ClearCounters( void )
{
    UtDiagFixture          fx;
    ModBus::AppLayer::DIAG diag( fx.diag );
    uint8_t                buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t     respLen   = 0u;
    const uint8_t          request[] = { UtModBus::SlaveAddr, 0x08, 0x00, 0x0A, 0x00, 0x00 };

    fx.diag.Set_DiagnosticRegister( 0xBEEFu );
    fx.diag.Set_RequestReceived( UtModBus::SlaveAddr );
    fx.diag.Set_RequestProcessed( 0x03u, MODBUS_ERROR_NO_ACTIVE_ERROR, true );
    fx.diag.Set_RequestReceived( UtModBus::SlaveAddr );
    fx.diag.Set_RequestProcessed( 0x03u, MODBUS_ERROR_ILLEGAL_DATA_ADDRESS, true );

    TEST_ASSERT_EQUAL_UINT16( 1u, fx.diag.Get_EventCounter() );
    TEST_ASSERT_EQUAL_UINT16( 1u, fx.diag.Get_BusExceptionErrorCount() );

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( diag, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( request, buffer, sizeof( request ) );

    TEST_ASSERT_EQUAL_UINT16( 0u, fx.diag.Get_EventCounter() );
    TEST_ASSERT_EQUAL_UINT16( 0u, fx.diag.Get_BusExceptionErrorCount() );
    TEST_ASSERT_EQUAL_UINT16( 0u, fx.diag.Get_SlaveMessageCount() );
    TEST_ASSERT_EQUAL_HEX16( 0u, fx.diag.Get_DiagnosticRegister() );
}


/** Every counter sub-function returns corresponding counter */
void Ut_DIAG_ReturnCounters( void )
{
    UtDiagFixture          fx;
    ModBus::AppLayer::DIAG diag( fx.diag );
    uint8_t                buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t     respLen = 0u;

    /* 2 slave messages: exception NAK sent, busy sent; 1 broadcast without response */
    fx.diag.Set_RequestReceived( UtModBus::SlaveAddr );
    fx.diag.Set_RequestProcessed( 0x03u, MODBUS_ERROR_NEGATIVE_ACKNOWLEDGE, true );
    fx.diag.Set_RequestReceived( UtModBus::SlaveAddr );
    fx.diag.Set_RequestProcessed( 0x03u, MODBUS_ERROR_SLAVE_DEVICE_BUSY, true );
    fx.diag.Set_RequestReceived( MODBUS_BROADCAST_ADDR );
    fx.diag.Set_RequestProcessed( 0x06u, MODBUS_ERROR_NO_ACTIVE_ERROR, false );

    const struct
    {
        uint8_t  subFunction;
        uint16_t expected;
    }   counters[] =
    {
        { 0x0Bu, 0u }, { 0x0Cu, 0u }, { 0x0Du, 2u }, { 0x0Eu, 3u },
        { 0x0Fu, 1u }, { 0x10u, 1u }, { 0x11u, 1u }, { 0x12u, 0u },
    };

    for( uint8_t counterIndex = 0u; ( sizeof( counters ) / sizeof( counters[ 0 ] ) ) > counterIndex; counterIndex++ )
    {
        const uint8_t request[] = { UtModBus::SlaveAddr, 0x08, 0x00, counters[ counterIndex ].subFunction, 0x00, 0x00 };

        TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( diag, request, sizeof( request ), buffer, respLen ) );
        TEST_ASSERT_EQUAL_UINT16( 6u, respLen );
        TEST_ASSERT_EQUAL_HEX8( counters[ counterIndex ].subFunction, buffer[ 3 ] );
        TEST_ASSERT_EQUAL_HEX16( counters[ counterIndex ].expected, ( buffer[ 4 ] << 8u ) | buffer[ 5 ] );
    }
}


void Ut_DIAG_ForceListenOnlyAndRestart( void )
{
    UtDiagFixture          fx;
    ModBus::AppLayer::DIAG diag( fx.diag );
    uint8_t                buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t     respLen = 0u;
    const uint8_t          listenOnly[]   = { UtModBus::SlaveAddr, 0x08, 0x00, 0x04, 0x00, 0x00 };
    const uint8_t          restartKeep[]  = { UtModBus::SlaveAddr, 0x08, 0x00, 0x01, 0x00, 0x00 };
    const uint8_t          restartClear[] = { UtModBus::SlaveAddr, 0x08, 0x00, 0x01, 0xFF, 0x00 };
    uint8_t                events[ MODBUS_COMM_EVENT_LOG_SIZE ];

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( diag, listenOnly, sizeof( listenOnly ), buffer, respLen ) );
    TEST_ASSERT_TRUE( fx.diag.Get_ListenOnlyMode() );

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( diag, restartKeep, sizeof( restartKeep ), buffer, respLen ) );
    TEST_ASSERT_FALSE( fx.diag.Get_ListenOnlyMode() );

    /* Event log keeps listen only event, restart event is the most recent */
    TEST_ASSERT_EQUAL_UINT8( 2u, fx.diag.Get_EventLog( events, sizeof( events ) ) );
    TEST_ASSERT_EQUAL_HEX8( MODBUS_COMM_EVENT_COMM_RESTART,        events[ 0 ] );
    TEST_ASSERT_EQUAL_HEX8( MODBUS_COMM_EVENT_LISTEN_ONLY_ENTERED, events[ 1 ] );

    /* Restart with FF00 clears event log */
    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( diag, restartClear, sizeof( restartClear ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_UINT8( 1u, fx.diag.Get_EventLog( events, sizeof( events ) ) );
    TEST_ASSERT_EQUAL_HEX8( MODBUS_COMM_EVENT_COMM_RESTART, events[ 0 ] );
}


void Ut_DIAG_Errors( void )
{
    UtDiagFixture          fx;
    ModBus::AppLayer::DIAG diag( fx.diag );
    uint8_t                buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t     respLen = 0u;
    const uint8_t          asciiDelimiter[] = { UtModBus::SlaveAddr, 0x08, 0x00, 0x03, 0x0A, 0x00 };
    const uint8_t          unknownSub[]     = { UtModBus::SlaveAddr, 0x08, 0x00, 0x13, 0x00, 0x00 };
    const uint8_t          nonZeroData[]    = { UtModBus::SlaveAddr, 0x08, 0x00, 0x0B, 0x00, 0x01 };
    const uint8_t          badRestart[]     = { UtModBus::SlaveAddr, 0x08, 0x00, 0x01, 0x12, 0x34 };
    const uint8_t          tooLong[]        = { UtModBus::SlaveAddr, 0x08, 0x00, 0x0B, 0x00, 0x00, 0x00, 0x00 };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_FUNCTION,   UtModBus::Decode( diag, asciiDelimiter, sizeof( asciiDelimiter ), buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_FUNCTION,   UtModBus::Decode( diag, unknownSub,     sizeof( unknownSub ),     buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE, UtModBus::Decode( diag, nonZeroData,    sizeof( nonZeroData ),    buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE, UtModBus::Decode( diag, badRestart,     sizeof( badRestart ),     buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE, UtModBus::Decode( diag, tooLong,        sizeof( tooLong ),        buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE, UtModBus::Decode( diag, tooLong,        3u,                       buffer, respLen ) );
}

/* ------------------------ Get Comm Event Counter -------------------------- */

void Ut_GCEC_ReturnsEventCounter( void )
{
    UtDiagFixture          fx;
    ModBus::AppLayer::GCEC gcec( fx.diag );
    uint8_t                buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t     respLen    = 0u;
    const uint8_t          request[]  = { UtModBus::SlaveAddr, 0x0B };
    const uint8_t          expected[] = { UtModBus::SlaveAddr, 0x0B, 0x00, 0x00, 0x00, 0x02 };

    /* Two successful requests, exception and fetch commands are not counted */
    fx.diag.Set_RequestProcessed( 0x03u, MODBUS_ERROR_NO_ACTIVE_ERROR,      true );
    fx.diag.Set_RequestProcessed( 0x06u, MODBUS_ERROR_NO_ACTIVE_ERROR,      true );
    fx.diag.Set_RequestProcessed( 0x03u, MODBUS_ERROR_ILLEGAL_DATA_ADDRESS, true );
    fx.diag.Set_RequestProcessed( 0x0Bu, MODBUS_ERROR_NO_ACTIVE_ERROR,      true );
    fx.diag.Set_RequestProcessed( 0x0Cu, MODBUS_ERROR_NO_ACTIVE_ERROR,      true );

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( gcec, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_UINT16( sizeof( expected ), respLen );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( expected, buffer, sizeof( expected ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE, UtModBus::Decode( gcec, request, 3u, buffer, respLen ) );
}

/* -------------------------- Get Comm Event Log ---------------------------- */

void Ut_GCEL_ReturnsEventLog( void )
{
    UtDiagFixture          fx;
    ModBus::AppLayer::GCEL gcel( fx.diag );
    uint8_t                buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t     respLen    = 0u;
    const uint8_t          request[]  = { UtModBus::SlaveAddr, 0x0C };
    const uint8_t          expected[] = { UtModBus::SlaveAddr, 0x0C, 0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x40, 0x80 };

    fx.diag.Set_RequestReceived( UtModBus::SlaveAddr );
    fx.diag.Set_RequestProcessed( 0x03u, MODBUS_ERROR_NO_ACTIVE_ERROR, true );

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( gcel, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_UINT16( sizeof( expected ), respLen );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( expected, buffer, sizeof( expected ) );
}


/** Event log holds at most 64 events, the oldest are dropped */
void Ut_GCEL_LogLimitedTo64Events( void )
{
    UtDiagFixture          fx;
    ModBus::AppLayer::GCEL gcel( fx.diag );
    uint8_t                buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t     respLen   = 0u;
    const uint8_t          request[] = { UtModBus::SlaveAddr, 0x0C };

    for( uint8_t eventIndex = 0u; 40u > eventIndex; eventIndex++ )
    {
        fx.diag.Set_RequestReceived( UtModBus::SlaveAddr );
        fx.diag.Set_RequestProcessed( 0x03u, MODBUS_ERROR_NO_ACTIVE_ERROR, true );
    }

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( gcel, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_UINT16( 9u + 64u, respLen );
    TEST_ASSERT_EQUAL_HEX8( 6u + 64u, buffer[ 2 ] );
    TEST_ASSERT_EQUAL_HEX8( 40u, buffer[ 6 ] );
}

/* ---------------------------- Report Slave ID ----------------------------- */

void Ut_RSID_Report( void )
{
    modBus_App_RSID_t      config;
    uint8_t                buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t     respLen    = 0u;
    const uint8_t          request[]  = { UtModBus::SlaveAddr, 0x11 };
    const uint8_t          expected[] = { UtModBus::SlaveAddr, 0x11, 0x06, 0x42, 0xFF, 'E', 'm', 'B', 'i' };

    config.SlaveId         = 0x42u;
    config.RunStatusGetter = UtRunStatus;
    config.AddData_Len     = sizeof( ut_AddData );
    config.AddData         = ut_AddData;

    ModBus::AppLayer::RSID rsid( config );

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( rsid, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_UINT16( sizeof( expected ), respLen );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( expected, buffer, sizeof( expected ) );
}


void Ut_RSID_InvalidConfiguration( void )
{
    modBus_App_RSID_t  config;
    uint8_t            buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t respLen   = 0u;
    const uint8_t      request[] = { UtModBus::SlaveAddr, 0x11 };

    config.AddData_Len = 5u;
    config.AddData     = nullptr;

    ModBus::AppLayer::RSID rsid( config );

    TEST_ASSERT_EQUAL( MODBUS_ERROR_SLAVE_DEVICE_FAILURE, UtModBus::Decode( rsid, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE,   UtModBus::Decode( rsid, request, 3u,                buffer, respLen ) );

    /* No run status getter and no additional data */
    config.AddData_Len = 0u;

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( rsid, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_UINT16( 5u, respLen );
    TEST_ASSERT_EQUAL_HEX8( 0x00u, buffer[ 4 ] );
}

/* ---------------------- Read Device Identification ------------------------ */

/** Example from ModBus specification - basic identification, stream access */
void Ut_RDID_BasicStream_SpecificationExample( void )
{
    ModBus::AppLayer::RDID rdid( ut_BasicObjects, 3u );
    uint8_t                buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t     respLen   = 0u;
    const uint8_t          request[] = { UtModBus::SlaveAddr, 0x2B, 0x0E, 0x01, 0x00 };
    const uint8_t          header[]  = { UtModBus::SlaveAddr, 0x2B, 0x0E, 0x01, 0x81, 0x00, 0x00, 0x03,
                                         0x00, 0x16 };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( rdid, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( header, buffer, sizeof( header ) );
    TEST_ASSERT_EQUAL_MEMORY( "Company identification", &buffer[ 10 ], 22u );
    TEST_ASSERT_EQUAL_HEX8( 0x01u, buffer[ 32 ] );
    TEST_ASSERT_EQUAL_HEX8( 0x0Fu, buffer[ 33 ] );
    TEST_ASSERT_EQUAL_MEMORY( "Product code XX", &buffer[ 34 ], 15u );
    TEST_ASSERT_EQUAL_HEX8( 0x02u, buffer[ 49 ] );
    TEST_ASSERT_EQUAL_HEX8( 0x05u, buffer[ 50 ] );
    TEST_ASSERT_EQUAL_MEMORY( "V2.11", &buffer[ 51 ], 5u );
    TEST_ASSERT_EQUAL_UINT16( 56u, respLen );
}


/** Regular stream returns basic and regular objects, conformity is derived from objects */
void Ut_RDID_RegularStream_ConformityLevel( void )
{
    ModBus::AppLayer::RDID rdid( ut_ExtendedObjects, 5u );
    uint8_t                buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t     respLen   = 0u;
    const uint8_t          request[] = { UtModBus::SlaveAddr, 0x2B, 0x0E, 0x02, 0x00 };

    TEST_ASSERT_EQUAL( MODBUS_APP_RDID_CONFORMITY_EXTENDED_BOTH, rdid.Get_ConformityLevel() );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( rdid, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_HEX8( 0x83u, buffer[ 4 ] );
    TEST_ASSERT_EQUAL_HEX8( 0x00u, buffer[ 5 ] );
    TEST_ASSERT_EQUAL_HEX8( 4u,    buffer[ 7 ] );

    /* Last object is product name */
    TEST_ASSERT_EQUAL_HEX8( MODBUS_RDID_ID_REGULAR_PRODUCT_NAME, buffer[ respLen - 4u ] );
    TEST_ASSERT_EQUAL_HEX8( 2u, buffer[ respLen - 3u ] );
}


/** Objects not fitting into response are transferred by next request (More Follows) */
void Ut_RDID_ExtendedStream_MoreFollows( void )
{
    ModBus::AppLayer::RDID rdid( ut_ExtendedObjects, 5u );
    uint8_t                buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t     respLen = 0u;
    const uint8_t          request[]     = { UtModBus::SlaveAddr, 0x2B, 0x0E, 0x03, 0x00 };
    const uint8_t          nextRequest[] = { UtModBus::SlaveAddr, 0x2B, 0x0E, 0x03, 0x80 };

    ut_PrivateObjLen = 200u;

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( rdid, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_HEX8( 0xFFu, buffer[ 5 ] );
    TEST_ASSERT_EQUAL_HEX8( 0x80u, buffer[ 6 ] );
    TEST_ASSERT_EQUAL_HEX8( 4u,    buffer[ 7 ] );

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( rdid, nextRequest, sizeof( nextRequest ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_HEX8( 0x00u, buffer[ 5 ] );
    TEST_ASSERT_EQUAL_HEX8( 1u,    buffer[ 7 ] );
    TEST_ASSERT_EQUAL_HEX8( 0x80u, buffer[ 8 ] );
    TEST_ASSERT_EQUAL_HEX8( 200u,  buffer[ 9 ] );
    TEST_ASSERT_EQUAL_UINT16( 10u + 200u, respLen );
}


/** Unknown object in stream access restarts the transfer at object 0 */
void Ut_RDID_Stream_UnknownObjectRestarts( void )
{
    ModBus::AppLayer::RDID rdid( ut_BasicObjects, 3u );
    uint8_t                buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t     respLen   = 0u;
    const uint8_t          request[] = { UtModBus::SlaveAddr, 0x2B, 0x0E, 0x01, 0x05 };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( rdid, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_HEX8( 3u,    buffer[ 7 ] );
    TEST_ASSERT_EQUAL_HEX8( 0x00u, buffer[ 8 ] );
}


/** Stream access starting at requested object */
void Ut_RDID_Stream_StartAtRequestedObject( void )
{
    ModBus::AppLayer::RDID rdid( ut_BasicObjects, 3u );
    uint8_t                buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t     respLen   = 0u;
    const uint8_t          request[] = { UtModBus::SlaveAddr, 0x2B, 0x0E, 0x01, 0x02 };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( rdid, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_HEX8( 1u,    buffer[ 7 ] );
    TEST_ASSERT_EQUAL_HEX8( 0x02u, buffer[ 8 ] );
}


void Ut_RDID_IndividualAccess( void )
{
    ModBus::AppLayer::RDID rdid( ut_ExtendedObjects, 6u );
    uint8_t                buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t     respLen = 0u;
    const uint8_t          request[]   = { UtModBus::SlaveAddr, 0x2B, 0x0E, 0x04, 0x04 };
    const uint8_t          expected[]  = { UtModBus::SlaveAddr, 0x2B, 0x0E, 0x04, 0x83, 0x00, 0x00, 0x01, 0x04, 0x02, 'P', 'N' };
    const uint8_t          unknown[]   = { UtModBus::SlaveAddr, 0x2B, 0x0E, 0x04, 0x05 };
    const uint8_t          failing[]   = { UtModBus::SlaveAddr, 0x2B, 0x0E, 0x04, 0x81 };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( rdid, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_UINT16( sizeof( expected ), respLen );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( expected, buffer, sizeof( expected ) );

    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_ADDRESS, UtModBus::Decode( rdid, unknown, sizeof( unknown ), buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_SLAVE_DEVICE_FAILURE, UtModBus::Decode( rdid, failing, sizeof( failing ), buffer, respLen ) );
}


void Ut_RDID_Errors( void )
{
    ModBus::AppLayer::RDID rdid( ut_BasicObjects, 3u );
    uint8_t                buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t     respLen = 0u;
    const uint8_t          badMei[]  = { UtModBus::SlaveAddr, 0x2B, 0x0D, 0x01, 0x00 };
    const uint8_t          badCode[] = { UtModBus::SlaveAddr, 0x2B, 0x0E, 0x05, 0x00 };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_FUNCTION,   UtModBus::Decode( rdid, badMei,  sizeof( badMei ),  buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE, UtModBus::Decode( rdid, badCode, sizeof( badCode ), buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE, UtModBus::Decode( rdid, badCode, 4u,                buffer, respLen ) );
}


/** Object longer than whole response can not be transferred */
void Ut_RDID_ObjectTooLong_SlaveDeviceFailure( void )
{
    ModBus::AppLayer::RDID rdid( ut_ExtendedObjects, 5u );
    uint8_t                buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t     respLen   = 0u;
    const uint8_t          request[] = { UtModBus::SlaveAddr, 0x2B, 0x0E, 0x04, 0x80 };

    ut_PrivateObjLen = 250u;

    TEST_ASSERT_EQUAL( MODBUS_ERROR_SLAVE_DEVICE_FAILURE, UtModBus::Decode( rdid, request, sizeof( request ), buffer, respLen ) );
}

