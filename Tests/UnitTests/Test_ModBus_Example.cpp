/**
 * \author Mr.Nobody
 * \file Test_ModBus_Example.cpp
 * \ingroup UnitTests
 * \brief Unit tests of ModBus example (project handler module template).
 *
 * The example is built with fake hardware interface (ModBus_Hal.h) instead of
 * ModBus_Hal.c. Requests are processed by complete slave stack through module
 * port functions ModBus_Init() / ModBus_Task().
 */

/* ============================= INCLUDES =================================== */
#include "unity.h"                          /* Unity testing framework        */
#include "UtModBus_FakePhy.hpp"             /* CRC reference implementation   */
#include "ModBus.h"                         /* Module configuration           */
#include "ModBus_Port.h"                    /* Module under test              */
#include "ModBus_Hal.h"                     /* Faked hardware interface       */
#include <string.h>                         /* Memory functions               */
/* ========================= SYMBOLIC CONSTANTS ============================= */

/** Count of tasks processing one request (request, response, standby) */
static constexpr uint8_t UT_TASKS_PER_REQUEST = 6u;

/** Size of frame buffers */
static constexpr modBus_MsgLength_t UT_BUFFER_SIZE = MODBUS_FRAME_MAX_LEN;

/* ========================== LOCAL VARIABLES =============================== */

/** Reception buffer provided by physical layer */
static uint8_t           *ut_RxBuffer;

/** Length of received frame */
static modBus_MsgLength_t ut_RxLen;

/** Frame is received */
static bool               ut_RxReady;

/** Last transmitted frame */
static uint8_t            ut_TxFrame[ UT_BUFFER_SIZE ];

/** Length of last transmitted frame */
static modBus_MsgLength_t ut_TxLen;

/** Count of HAL initializations */
static uint32_t           ut_InitCnt;

/** Last bus configuration */
static modBus_BusConfig_t ut_BusConfig;

/* ======================== FAKE HARDWARE INTERFACE ========================= */

extern "C"
{

void ModBus_Hal_Init( const modBus_BusConfig_t *busConfig )
{
    ut_BusConfig = *busConfig;
    ut_InitCnt++;
}


void ModBus_Hal_StartTransmit( const uint8_t *buffer, modBus_MsgLength_t length )
{
    memcpy( ut_TxFrame, buffer, length );
    ut_TxLen = length;
}


void ModBus_Hal_StopTransmit( void )
{
}


modBus_TxStatus_t ModBus_Hal_Get_TxStatus( void )
{
    return ( MODBUS_TX_FINISHED );
}


void ModBus_Hal_StartReceive( uint8_t *buffer, modBus_MsgLength_t bufferSize )
{
    (void)bufferSize;

    ut_RxBuffer = buffer;
    ut_RxReady  = false;
}


void ModBus_Hal_StopReceive( void )
{
}


modBus_RxStatus_t ModBus_Hal_Get_RxStatus( modBus_MsgLength_t *rcvdLength )
{
    *rcvdLength = ut_RxLen;

    return ( ut_RxReady ? MODBUS_RX_READY : MODBUS_RX_NOT_RCVD );
}


void ModBus_Hal_Set_ActivityLed( uint8_t ledActive )
{
    (void)ledActive;
}

} /* extern "C" */

/* ========================== LOCAL FUNCTIONS =============================== */

/** Runs module task */
static void UtRunTasks( uint8_t count )
{
    for( uint8_t taskIndex = 0u; count > taskIndex; taskIndex++ )
    {
        ModBus_Task();
    }
}


/**
 * \brief Sends request (without CRC) and returns response (without CRC)
 *
 * \return Length of response, 0 if no response was sent
 */
static modBus_MsgLength_t UtTransact( const uint8_t *request, modBus_MsgLength_t reqLen, uint8_t *response )
{
    TEST_ASSERT_NOT_NULL( ut_RxBuffer );

    memcpy( ut_RxBuffer, request, reqLen );

    const uint16_t crc = UtModBus::Crc16( request, reqLen );

    ut_RxBuffer[ reqLen      ] = (uint8_t)( crc & 0xFFu );
    ut_RxBuffer[ reqLen + 1u ] = (uint8_t)( crc >> 8u );

    ut_RxLen   = reqLen + MODBUS_CRC_LEN;
    ut_RxReady = true;
    ut_TxLen   = 0u;

    UtRunTasks( UT_TASKS_PER_REQUEST );

    modBus_MsgLength_t respLen = 0u;

    if( 0u != ut_TxLen )
    {
        respLen = ut_TxLen - MODBUS_CRC_LEN;

        TEST_ASSERT_EQUAL_HEX16( UtModBus::Crc16( ut_TxFrame, respLen ),
                                 ModBus_Get_16bFromTwo8b( ut_TxFrame[ respLen + 1u ], ut_TxFrame[ respLen ] ) );

        memcpy( response, ut_TxFrame, respLen );
    }
    else
    {
        /* No response */
    }

    return ( respLen );
}

/* ======================== EXPORTED FUNCTIONS ============================== */

void setUp( void )
{
    ut_RxBuffer = nullptr;
    ut_RxLen    = 0u;
    ut_RxReady  = false;
    ut_TxLen    = 0u;
    ut_InitCnt  = 0u;

    ModBus_Init();

    /* Init -> Standby, physical layer initialized */
    UtRunTasks( 1u );
}


void tearDown( void )
{
}


void Ut_Example_Init_ConfiguresBus( void )
{
    TEST_ASSERT_EQUAL_UINT32( 1u, ut_InitCnt );
    TEST_ASSERT_EQUAL_UINT32( MODBUS_BUS_BAUDRATE, ut_BusConfig.Baudrate );
    TEST_ASSERT_EQUAL( MODBUS_BUS_PARITY,   ut_BusConfig.Parity );
    TEST_ASSERT_EQUAL( MODBUS_BUS_STOPBITS, ut_BusConfig.StopBits );
}


void Ut_Example_ModuleVersion( void )
{
    const modBus_ModuleVersion_t version = ModBus_Get_ModuleVersion();

    TEST_ASSERT_EQUAL_UINT8( 1u, version.Major );
}


void Ut_Example_ReadHoldingRegisters( void )
{
    uint8_t       response[ UT_BUFFER_SIZE ];
    const uint8_t request[]  = { MODBUS_SLAVE_ADDRESS, 0x03, 0x00, 0x00, 0x00, 0x03 };
    const uint8_t expected[] = { MODBUS_SLAVE_ADDRESS, 0x03, 0x06, 0x09, 0xC4, 0x00, 0x00, 0x00, 0x00 };

    TEST_ASSERT_EQUAL_UINT16( sizeof( expected ), UtTransact( request, sizeof( request ), response ) );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( expected, response, sizeof( expected ) );
}


/** Write of set-point is visible by following read */
void Ut_Example_WriteAndReadSetPoint( void )
{
    uint8_t       response[ UT_BUFFER_SIZE ];
    const uint8_t write[]    = { MODBUS_SLAVE_ADDRESS, 0x06, 0x00, 0x00, 0x0A, 0x28 };
    const uint8_t read[]     = { MODBUS_SLAVE_ADDRESS, 0x03, 0x00, 0x00, 0x00, 0x01 };
    const uint8_t expected[] = { MODBUS_SLAVE_ADDRESS, 0x03, 0x02, 0x0A, 0x28 };

    TEST_ASSERT_EQUAL_UINT16( sizeof( write ), UtTransact( write, sizeof( write ), response ) );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( write, response, sizeof( write ) );

    TEST_ASSERT_EQUAL_UINT16( sizeof( expected ), UtTransact( read, sizeof( read ), response ) );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( expected, response, sizeof( expected ) );
}


/** Unsupported operation mode is rejected by application (exception 0x04) */
void Ut_Example_InvalidMode_SlaveDeviceFailure( void )
{
    uint8_t       response[ UT_BUFFER_SIZE ];
    const uint8_t request[]  = { MODBUS_SLAVE_ADDRESS, 0x06, 0x00, 0x01, 0x00, 0x05 };
    const uint8_t expected[] = { MODBUS_SLAVE_ADDRESS, 0x86, 0x04 };

    TEST_ASSERT_EQUAL_UINT16( sizeof( expected ), UtTransact( request, sizeof( request ), response ) );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( expected, response, sizeof( expected ) );
}


void Ut_Example_CoilsAndExceptionStatus( void )
{
    uint8_t       response[ UT_BUFFER_SIZE ];
    const uint8_t relayOn[]    = { MODBUS_SLAVE_ADDRESS, 0x05, 0x00, 0x00, 0xFF, 0x00 };
    const uint8_t readCoils[]  = { MODBUS_SLAVE_ADDRESS, 0x01, 0x00, 0x00, 0x00, 0x02 };
    const uint8_t readStatus[] = { MODBUS_SLAVE_ADDRESS, 0x07 };
    const uint8_t coils[]      = { MODBUS_SLAVE_ADDRESS, 0x01, 0x01, 0x01 };
    const uint8_t status[]     = { MODBUS_SLAVE_ADDRESS, 0x07, 0x01 };

    TEST_ASSERT_EQUAL_UINT16( sizeof( relayOn ), UtTransact( relayOn, sizeof( relayOn ), response ) );

    TEST_ASSERT_EQUAL_UINT16( sizeof( coils ), UtTransact( readCoils, sizeof( readCoils ), response ) );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( coils, response, sizeof( coils ) );

    TEST_ASSERT_EQUAL_UINT16( sizeof( status ), UtTransact( readStatus, sizeof( readStatus ), response ) );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( status, response, sizeof( status ) );
}


void Ut_Example_DeviceIdentification( void )
{
    uint8_t       response[ UT_BUFFER_SIZE ];
    const uint8_t request[] = { MODBUS_SLAVE_ADDRESS, 0x2B, 0x0E, 0x01, 0x00 };
    const uint8_t header[]  = { MODBUS_SLAVE_ADDRESS, 0x2B, 0x0E, 0x01, 0x81, 0x00, 0x00, 0x03 };

    TEST_ASSERT_TRUE( sizeof( header ) < UtTransact( request, sizeof( request ), response ) );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( header, response, sizeof( header ) );
}


void Ut_Example_UnsupportedFunctionAndBroadcast( void )
{
    uint8_t       response[ UT_BUFFER_SIZE ];
    const uint8_t unknown[]   = { MODBUS_SLAVE_ADDRESS, 0x41 };
    const uint8_t broadcast[] = { MODBUS_BROADCAST_ADDR, 0x06, 0x00, 0x00, 0x01, 0x00 };
    const uint8_t read[]      = { MODBUS_SLAVE_ADDRESS, 0x03, 0x00, 0x00, 0x00, 0x01 };
    const uint8_t expected[]  = { MODBUS_SLAVE_ADDRESS, 0x03, 0x02, 0x01, 0x00 };

    TEST_ASSERT_EQUAL_UINT16( 3u, UtTransact( unknown, sizeof( unknown ), response ) );
    TEST_ASSERT_EQUAL_HEX8( 0xC1u, response[ 1 ] );
    TEST_ASSERT_EQUAL_HEX8( 0x01u, response[ 2 ] );

    TEST_ASSERT_EQUAL_UINT16( 0u, UtTransact( broadcast, sizeof( broadcast ), response ) );

    TEST_ASSERT_EQUAL_UINT16( sizeof( expected ), UtTransact( read, sizeof( read ), response ) );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( expected, response, sizeof( expected ) );
}


/** Deinitialized module does not respond, initialization restores communication */
void Ut_Example_DeinitStopsCommunication( void )
{
    uint8_t       response[ UT_BUFFER_SIZE ];
    const uint8_t read[] = { MODBUS_SLAVE_ADDRESS, 0x03, 0x00, 0x00, 0x00, 0x01 };

    ModBus_Deinit();
    UtRunTasks( 2u );

    ut_RxReady = true;
    ut_RxLen   = 8u;
    UtRunTasks( UT_TASKS_PER_REQUEST );
    TEST_ASSERT_EQUAL_UINT16( 0u, ut_TxLen );

    ModBus_Init();
    UtRunTasks( 1u );

    TEST_ASSERT_EQUAL_UINT32( 2u, ut_InitCnt );
    TEST_ASSERT_EQUAL_UINT16( 5u, UtTransact( read, sizeof( read ), response ) );
}
