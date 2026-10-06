/**
 * \author Mr.Nobody
 * \file Test_ModBus_AppCore.cpp
 * \ingroup UnitTests
 * \brief Unit tests of ModBus slave application core (complete slave stack).
 */

/* ============================= INCLUDES =================================== */
#include "unity.h"                          /* Unity testing framework        */
#include "UtModBus_Helpers.hpp"             /* Test helpers                   */
#include "ModBus_App_RHR.hpp"               /* Read Holding Registers         */
#include "ModBus_App_WSR.hpp"               /* Write Single Register          */
#include "ModBus_App_DIAG.hpp"              /* Diagnostics                    */
#include "ModBus_App_GCEC.hpp"              /* Get Comm Event Counter         */
/* ========================== LOCAL VARIABLES =============================== */

/** Emulated holding registers */
static uint16_t ut_Registers[ 4 ];

/** Count of setter calls */
static uint32_t ut_SetterCnt;

/* ========================== LOCAL FUNCTIONS =============================== */

template< int RegId >
static modBus_RequestState_t UtGetReg( modBus_RegistersValue_t *value )
{
    *value = ut_Registers[ RegId ];

    return ( MODBUS_REQUEST_OK );
}


template< int RegId >
static modBus_RequestState_t UtSetReg( modBus_RegistersValue_t *value )
{
    ut_Registers[ RegId ] = *value;
    ut_SetterCnt++;

    return ( MODBUS_REQUEST_OK );
}


static const modBus_RegistersConfigStruct_t ut_RegConfig[] =
{
    { 0u, MODBUS_SERVICE_SUPPORTED, UtGetReg< 0 >, UtSetReg< 0 > },
    { 1u, MODBUS_SERVICE_SUPPORTED, UtGetReg< 1 >, UtSetReg< 1 > },
    { 2u, MODBUS_SERVICE_SUPPORTED, UtGetReg< 2 >, UtSetReg< 2 > },
    { 3u, MODBUS_SERVICE_SUPPORTED, UtGetReg< 3 >, UtSetReg< 3 > },
};

/* ======================== EXPORTED FUNCTIONS ============================== */

void setUp( void )
{
    ut_Registers[ 0 ] = 0x1234u;
    ut_Registers[ 1 ] = 0x5678u;
    ut_Registers[ 2 ] = 0x9ABCu;
    ut_Registers[ 3 ] = 0xDEF0u;

    ut_SetterCnt = 0u;
}


void tearDown( void )
{
}


void Ut_AppCore_ReadHoldingRegisters_EndToEnd( void )
{
    ModBus::AppLayer::RHR     rhr( ut_RegConfig, 4u );
    ModBus::FunctionsHandler *handlers[] = { &rhr };
    UtModBus::SlaveStack      stack( handlers, 1u );
    uint8_t                   response[ UtModBus::BufferSize ];
    const uint8_t             request[]  = { UtModBus::SlaveAddr, 0x03, 0x00, 0x01, 0x00, 0x02 };
    const uint8_t             expected[] = { UtModBus::SlaveAddr, 0x03, 0x04, 0x56, 0x78, 0x9A, 0xBC };

    const modBus_MsgLength_t respLen = stack.Transact( request, sizeof( request ), response );

    TEST_ASSERT_EQUAL_UINT16( sizeof( expected ), respLen );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( expected, response, sizeof( expected ) );
}


/** Unsupported function code is answered by exception 0x01 (slave must not hang) */
void Ut_AppCore_UnsupportedFunction_IllegalFunctionException( void )
{
    ModBus::AppLayer::RHR     rhr( ut_RegConfig, 4u );
    ModBus::FunctionsHandler *handlers[] = { &rhr };
    UtModBus::SlaveStack      stack( handlers, 1u );
    uint8_t                   response[ UtModBus::BufferSize ];
    const uint8_t             request[]  = { UtModBus::SlaveAddr, 0x05, 0x00, 0x01, 0xFF, 0x00 };
    const uint8_t             expected[] = { UtModBus::SlaveAddr, 0x85, 0x01 };

    TEST_ASSERT_EQUAL_UINT16( sizeof( expected ), stack.Transact( request, sizeof( request ), response ) );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( expected, response, sizeof( expected ) );

    /* Next request is processed normally */
    const uint8_t request2[] = { UtModBus::SlaveAddr, 0x03, 0x00, 0x00, 0x00, 0x01 };

    TEST_ASSERT_EQUAL_UINT16( 5u, stack.Transact( request2, sizeof( request2 ), response ) );
}


void Ut_AppCore_HandlerError_ExceptionResponse( void )
{
    ModBus::AppLayer::RHR     rhr( ut_RegConfig, 4u );
    ModBus::FunctionsHandler *handlers[] = { &rhr };
    UtModBus::SlaveStack      stack( handlers, 1u );
    uint8_t                   response[ UtModBus::BufferSize ];
    const uint8_t             request[]  = { UtModBus::SlaveAddr, 0x03, 0x00, 0x03, 0x00, 0x02 };
    const uint8_t             expected[] = { UtModBus::SlaveAddr, 0x83, 0x02 };

    TEST_ASSERT_EQUAL_UINT16( sizeof( expected ), stack.Transact( request, sizeof( request ), response ) );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( expected, response, sizeof( expected ) );
}


/** Broadcast request is executed, but no response is sent */
void Ut_AppCore_Broadcast_ExecutedWithoutResponse( void )
{
    ModBus::AppLayer::WSR     wsr( ut_RegConfig, 4u );
    ModBus::FunctionsHandler *handlers[] = { &wsr };
    UtModBus::SlaveStack      stack( handlers, 1u );
    uint8_t                   response[ UtModBus::BufferSize ];
    const uint8_t             request[] = { MODBUS_BROADCAST_ADDR, 0x06, 0x00, 0x02, 0xAB, 0xCD };

    TEST_ASSERT_EQUAL_UINT16( 0u, stack.Transact( request, sizeof( request ), response ) );
    TEST_ASSERT_EQUAL_HEX16( 0xABCDu, ut_Registers[ 2 ] );
    TEST_ASSERT_EQUAL_UINT16( 1u, stack.diag.Get_SlaveNoResponseCount() );
}


/** Broadcast with error is not answered by exception */
void Ut_AppCore_BroadcastError_NoException( void )
{
    ModBus::AppLayer::WSR     wsr( ut_RegConfig, 4u );
    ModBus::FunctionsHandler *handlers[] = { &wsr };
    UtModBus::SlaveStack      stack( handlers, 1u );
    uint8_t                   response[ UtModBus::BufferSize ];
    const uint8_t             request[] = { MODBUS_BROADCAST_ADDR, 0x06, 0x00, 0x09, 0xAB, 0xCD };

    TEST_ASSERT_EQUAL_UINT16( 0u, stack.Transact( request, sizeof( request ), response ) );
    TEST_ASSERT_EQUAL_UINT16( 0u, stack.diag.Get_BusExceptionErrorCount() );
}


/** Null entries in handlers array are skipped */
void Ut_AppCore_NullHandlerEntries_Ignored( void )
{
    ModBus::AppLayer::RHR     rhr( ut_RegConfig, 4u );
    ModBus::FunctionsHandler *handlers[ 5 ] = { nullptr, &rhr, nullptr, nullptr, nullptr };
    UtModBus::SlaveStack      stack( handlers, 5u );
    uint8_t                   response[ UtModBus::BufferSize ];
    const uint8_t             request[]  = { UtModBus::SlaveAddr, 0x04, 0x00, 0x00, 0x00, 0x01 };
    const uint8_t             request2[] = { UtModBus::SlaveAddr, 0x03, 0x00, 0x00, 0x00, 0x01 };

    TEST_ASSERT_EQUAL_UINT16( 3u, stack.Transact( request, sizeof( request ), response ) );
    TEST_ASSERT_EQUAL_HEX8( 0x84u, response[ 1 ] );

    TEST_ASSERT_EQUAL_UINT16( 5u, stack.Transact( request2, sizeof( request2 ), response ) );
}


void Ut_AppCore_WithoutDiagnostics( void )
{
    ModBus::AppLayer::RHR     rhr( ut_RegConfig, 4u );
    ModBus::FunctionsHandler *handlers[] = { &rhr };
    UtModBus::SlaveStack      stack( handlers, 1u, false );
    uint8_t                   response[ UtModBus::BufferSize ];
    const uint8_t             request[]  = { UtModBus::SlaveAddr, 0x03, 0x00, 0x00, 0x00, 0x01 };
    const uint8_t             request2[] = { MODBUS_BROADCAST_ADDR, 0x03, 0x00, 0x00, 0x00, 0x01 };

    TEST_ASSERT_EQUAL_UINT16( 5u, stack.Transact( request, sizeof( request ), response ) );
    TEST_ASSERT_EQUAL_UINT16( 0u, stack.Transact( request2, sizeof( request2 ), response ) );
    TEST_ASSERT_EQUAL_UINT16( 0u, stack.diag.Get_SlaveMessageCount() );
}


/** Listen Only Mode - no responses, only Restart Communications is processed */
void Ut_AppCore_ListenOnlyMode( void )
{
    ModBus::AppLayer::WSR     wsr( ut_RegConfig, 4u );
    ModBus::FunctionsHandler *handlers[ 2 ] = { &wsr, nullptr };
    UtModBus::SlaveStack      stack( handlers, 2u );
    ModBus::AppLayer::DIAG    diag( stack.diag );
    uint8_t                   response[ UtModBus::BufferSize ];
    const uint8_t             forceListenOnly[] = { UtModBus::SlaveAddr, 0x08, 0x00, 0x04, 0x00, 0x00 };
    const uint8_t             restart[]         = { UtModBus::SlaveAddr, 0x08, 0x00, 0x01, 0x00, 0x00 };
    const uint8_t             write[]           = { UtModBus::SlaveAddr, 0x06, 0x00, 0x01, 0x11, 0x22 };

    /* Handler is registered after the stack, it needs its diagnostics */
    handlers[ 1 ] = &diag;

    /* Force Listen Only Mode - no response */
    TEST_ASSERT_EQUAL_UINT16( 0u, stack.Transact( forceListenOnly, sizeof( forceListenOnly ), response ) );
    TEST_ASSERT_TRUE( stack.diag.Get_ListenOnlyMode() );

    /* Requests are ignored - not executed, no response */
    TEST_ASSERT_EQUAL_UINT16( 0u, stack.Transact( write, sizeof( write ), response ) );
    TEST_ASSERT_EQUAL_UINT32( 0u, ut_SetterCnt );

    /* Restart leaves Listen Only Mode, but still without response */
    TEST_ASSERT_EQUAL_UINT16( 0u, stack.Transact( restart, sizeof( restart ), response ) );
    TEST_ASSERT_FALSE( stack.diag.Get_ListenOnlyMode() );

    /* Normal operation */
    TEST_ASSERT_EQUAL_UINT16( sizeof( write ), stack.Transact( write, sizeof( write ), response ) );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( write, response, sizeof( write ) );
    TEST_ASSERT_EQUAL_HEX16( 0x1122u, ut_Registers[ 1 ] );
}


/** Restart Communications in normal mode is answered by echo */
void Ut_AppCore_RestartInNormalMode_Echo( void )
{
    ModBus::FunctionsHandler *handlers[ 1 ] = { nullptr };
    UtModBus::SlaveStack      stack( handlers, 1u );
    ModBus::AppLayer::DIAG    diag( stack.diag );
    uint8_t                   response[ UtModBus::BufferSize ];
    const uint8_t             restart[] = { UtModBus::SlaveAddr, 0x08, 0x00, 0x01, 0xFF, 0x00 };

    handlers[ 0 ] = &diag;

    TEST_ASSERT_EQUAL_UINT16( sizeof( restart ), stack.Transact( restart, sizeof( restart ), response ) );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( restart, response, sizeof( restart ) );
}


/** Diagnostic counters are updated by application core */
void Ut_AppCore_DiagnosticCounters( void )
{
    ModBus::AppLayer::RHR     rhr( ut_RegConfig, 4u );
    ModBus::FunctionsHandler *handlers[] = { &rhr };
    UtModBus::SlaveStack      stack( handlers, 1u );
    uint8_t                   response[ UtModBus::BufferSize ];
    const uint8_t             readOk[]     = { UtModBus::SlaveAddr, 0x03, 0x00, 0x00, 0x00, 0x01 };
    const uint8_t             readErr[]    = { UtModBus::SlaveAddr, 0x03, 0x00, 0x00, 0x00, 0x00 };
    const uint8_t             broadcast[]  = { MODBUS_BROADCAST_ADDR, 0x03, 0x00, 0x00, 0x00, 0x01 };
    const uint8_t             otherSlave[] = { 0x22, 0x03, 0x00, 0x00, 0x00, 0x01 };

    (void)stack.Transact( readOk,     sizeof( readOk ),     response );
    (void)stack.Transact( readOk,     sizeof( readOk ),     response );
    (void)stack.Transact( readErr,    sizeof( readErr ),    response );
    (void)stack.Transact( broadcast,  sizeof( broadcast ),  response );
    (void)stack.Transact( otherSlave, sizeof( otherSlave ), response );

    /* Frame with CRC error */
    stack.phy.ReceiveFrame( readOk, sizeof( readOk ), false );
    for( uint8_t taskIndex = 0u; 5u > taskIndex; taskIndex++ )
    {
        stack.core.Task( UtModBus::SlaveStack::taskTime );
    }

    TEST_ASSERT_EQUAL_UINT16( 6u, stack.diag.Get_BusMessageCount() );
    TEST_ASSERT_EQUAL_UINT16( 1u, stack.diag.Get_BusCommErrorCount() );
    TEST_ASSERT_EQUAL_UINT16( 1u, stack.diag.Get_BusExceptionErrorCount() );
    TEST_ASSERT_EQUAL_UINT16( 4u, stack.diag.Get_SlaveMessageCount() );
    TEST_ASSERT_EQUAL_UINT16( 1u, stack.diag.Get_SlaveNoResponseCount() );
    TEST_ASSERT_EQUAL_UINT16( 3u, stack.diag.Get_EventCounter() );

    /* Event log - the most recent first */
    uint8_t       events[ MODBUS_COMM_EVENT_LOG_SIZE ];
    const uint8_t expectedEvents[] =
    {
        MODBUS_COMM_EVENT_RCV | MODBUS_COMM_EVENT_RCV_COMM_ERROR,       /* CRC error         */
        MODBUS_COMM_EVENT_RCV | MODBUS_COMM_EVENT_RCV_BROADCAST,        /* Broadcast request */
        MODBUS_COMM_EVENT_SEND | MODBUS_COMM_EVENT_SEND_READ_EXCEPTION, /* Exception 03 sent */
        MODBUS_COMM_EVENT_RCV,                                          /* Request received  */
        MODBUS_COMM_EVENT_SEND,                                         /* Normal response   */
    };

    TEST_ASSERT_EQUAL_UINT8( 8u, stack.diag.Get_EventLog( events, sizeof( events ) ) );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( expectedEvents, events, sizeof( expectedEvents ) );
}

