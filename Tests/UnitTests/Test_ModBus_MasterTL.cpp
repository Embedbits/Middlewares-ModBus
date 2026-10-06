/**
 * \author Mr.Nobody
 * \file Test_ModBus_MasterTL.cpp
 * \ingroup UnitTests
 * \brief Unit tests of ModBus master transport layer.
 */

/* ============================= INCLUDES =================================== */
#include "unity.h"                          /* Unity testing framework        */
#include "UtModBus_FakePhy.hpp"             /* Fake physical layer            */
#include "ModBus_Master_TL.hpp"             /* Code under test                */
/* ========================= SYMBOLIC CONSTANTS ============================= */

/** Task period used in tests in ms */
static constexpr modBus_Time_t UT_TASK_TIME = 10u;

/* ========================== LOCAL FUNCTIONS =============================== */

/** Runs task of transport layer */
static void RunTasks( ModBus::Master::TransportLayer &tl, uint32_t count )
{
    for( uint32_t taskIndex = 0u; count > taskIndex; taskIndex++ )
    {
        tl.Task( UT_TASK_TIME );
    }
}


/** Request used in tests - read 1 holding register of slave 0x11 */
static void PrepareRequest( uint8_t *txBuffer, modBus_DeviceAddr_t slaveAddr )
{
    const uint8_t request[] = { slaveAddr, 0x03, 0x00, 0x00, 0x00, 0x01 };

    for( uint8_t byteIndex = 0u; sizeof( request ) > byteIndex; byteIndex++ )
    {
        txBuffer[ byteIndex ] = request[ byteIndex ];
    }
}

/* ======================== EXPORTED FUNCTIONS ============================== */

void setUp( void )
{
}


void tearDown( void )
{
}


/** Full transaction - request with CRC is sent, valid response is copied */
void Ut_MasterTL_Transaction_ResponseReceived( void )
{
    UtModBus::FakePhy              phy;
    ModBus::Master::TransportLayer tl( phy );
    uint8_t                        txBuffer[ 16 ] = { 0u };
    uint8_t                        rxBuffer[ ModBus::PhysicalLayer::messageBufferMaxLen ] = { 0u };
    const uint8_t                  response[] = { 0x11, 0x03, 0x02, 0x12, 0x34 };
    modBus_MsgLength_t             rxLen = 0u;

    PrepareRequest( txBuffer, 0x11u );

    TEST_ASSERT_EQUAL( MODBUS_MASTER_REQ_IDLE, tl.Get_RequestStatus() );
    TEST_ASSERT_EQUAL( MODBUS_REQUEST_OK, tl.StartTransmit( 6u, txBuffer, rxBuffer ) );
    TEST_ASSERT_EQUAL( MODBUS_MASTER_REQ_BUSY, tl.Get_RequestStatus() );

    /* Init -> Standby, Standby -> Transmit */
    RunTasks( tl, 2u );

    const uint16_t crc = UtModBus::Crc16( txBuffer, 6u );

    TEST_ASSERT_EQUAL_UINT32( 1u, phy.initCnt );
    TEST_ASSERT_EQUAL_UINT32( 1u, phy.txStartCnt );
    TEST_ASSERT_EQUAL_UINT16( 8u, phy.txLen );
    TEST_ASSERT_EQUAL_HEX8( crc & 0xFFu, phy.txFrame[ 6 ] );
    TEST_ASSERT_EQUAL_HEX8( crc >> 8u,   phy.txFrame[ 7 ] );

    /* Transmit -> Response wait */
    RunTasks( tl, 1u );
    TEST_ASSERT_EQUAL_UINT32( 1u, phy.rxStartCnt );

    phy.ReceiveFrame( response, sizeof( response ) );

    /* Response wait -> Frame check -> Standby */
    RunTasks( tl, 2u );

    TEST_ASSERT_EQUAL( MODBUS_MASTER_REQ_DONE, tl.Get_RequestStatus() );
    TEST_ASSERT_EQUAL( MODBUS_RX_READY, tl.Get_RxMessageStatus( &rxLen ) );
    TEST_ASSERT_EQUAL_UINT16( sizeof( response ), rxLen );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( response, rxBuffer, sizeof( response ) );
}


void Ut_MasterTL_ResponseTimeout( void )
{
    UtModBus::FakePhy              phy;
    ModBus::Master::TransportLayer tl( phy );
    uint8_t                        txBuffer[ 16 ] = { 0u };
    uint8_t                        rxBuffer[ ModBus::PhysicalLayer::messageBufferMaxLen ] = { 0u };

    PrepareRequest( txBuffer, 0x11u );
    tl.Set_ResponseTimeout( 100u );

    TEST_ASSERT_EQUAL( MODBUS_REQUEST_OK, tl.StartTransmit( 6u, txBuffer, rxBuffer ) );

    RunTasks( tl, 3u + 9u );
    TEST_ASSERT_EQUAL( MODBUS_MASTER_REQ_BUSY, tl.Get_RequestStatus() );

    RunTasks( tl, 2u );
    TEST_ASSERT_EQUAL( MODBUS_MASTER_REQ_TIMEOUT, tl.Get_RequestStatus() );
    TEST_ASSERT_EQUAL( MODBUS_RX_NOT_RCVD, tl.Get_RxMessageStatus( nullptr ) );
    TEST_ASSERT_EQUAL_UINT32( 1u, phy.rxStopCnt );
}


void Ut_MasterTL_ResponseCrcError( void )
{
    UtModBus::FakePhy              phy;
    ModBus::Master::TransportLayer tl( phy );
    uint8_t                        txBuffer[ 16 ] = { 0u };
    uint8_t                        rxBuffer[ ModBus::PhysicalLayer::messageBufferMaxLen ] = { 0u };
    const uint8_t                  response[] = { 0x11, 0x03, 0x02, 0x12, 0x34, 0x00, 0x00 };

    PrepareRequest( txBuffer, 0x11u );
    TEST_ASSERT_EQUAL( MODBUS_REQUEST_OK, tl.StartTransmit( 6u, txBuffer, rxBuffer ) );
    RunTasks( tl, 3u );

    phy.ReceiveFrame( response, sizeof( response ), false );
    RunTasks( tl, 2u );

    TEST_ASSERT_EQUAL( MODBUS_MASTER_REQ_CRC_ERROR, tl.Get_RequestStatus() );
    TEST_ASSERT_EQUAL( MODBUS_RX_NOT_RCVD, tl.Get_RxMessageStatus( nullptr ) );
}


void Ut_MasterTL_ResponseFromOtherSlave( void )
{
    UtModBus::FakePhy              phy;
    ModBus::Master::TransportLayer tl( phy );
    uint8_t                        txBuffer[ 16 ] = { 0u };
    uint8_t                        rxBuffer[ ModBus::PhysicalLayer::messageBufferMaxLen ] = { 0u };
    const uint8_t                  response[] = { 0x12, 0x03, 0x02, 0x12, 0x34 };

    PrepareRequest( txBuffer, 0x11u );
    TEST_ASSERT_EQUAL( MODBUS_REQUEST_OK, tl.StartTransmit( 6u, txBuffer, rxBuffer ) );
    RunTasks( tl, 3u );

    phy.ReceiveFrame( response, sizeof( response ) );
    RunTasks( tl, 2u );

    TEST_ASSERT_EQUAL( MODBUS_MASTER_REQ_ADDR_ERROR, tl.Get_RequestStatus() );
}


/** Exception response is valid response for transport layer */
void Ut_MasterTL_ExceptionResponse_Delivered( void )
{
    UtModBus::FakePhy              phy;
    ModBus::Master::TransportLayer tl( phy );
    uint8_t                        txBuffer[ 16 ] = { 0u };
    uint8_t                        rxBuffer[ ModBus::PhysicalLayer::messageBufferMaxLen ] = { 0u };
    const uint8_t                  response[] = { 0x11, 0x83, 0x02 };
    modBus_MsgLength_t             rxLen = 0u;

    PrepareRequest( txBuffer, 0x11u );
    TEST_ASSERT_EQUAL( MODBUS_REQUEST_OK, tl.StartTransmit( 6u, txBuffer, rxBuffer ) );
    RunTasks( tl, 3u );

    phy.ReceiveFrame( response, sizeof( response ) );
    RunTasks( tl, 2u );

    TEST_ASSERT_EQUAL( MODBUS_MASTER_REQ_DONE, tl.Get_RequestStatus() );
    TEST_ASSERT_EQUAL( MODBUS_RX_READY, tl.Get_RxMessageStatus( &rxLen ) );
    TEST_ASSERT_EQUAL_UINT16( 3u, rxLen );
    TEST_ASSERT_EQUAL_HEX8( 0x83u, rxBuffer[ 1 ] );
}


/** Broadcast - no reception, done after turnaround delay */
void Ut_MasterTL_Broadcast_DoneAfterTurnaround( void )
{
    UtModBus::FakePhy              phy;
    ModBus::Master::TransportLayer tl( phy );
    uint8_t                        txBuffer[ 16 ] = { 0u };
    uint8_t                        rxBuffer[ ModBus::PhysicalLayer::messageBufferMaxLen ] = { 0u };

    PrepareRequest( txBuffer, MODBUS_BROADCAST_ADDR );
    tl.Set_TurnaroundDelay( 50u );

    TEST_ASSERT_EQUAL( MODBUS_REQUEST_OK, tl.StartTransmit( 6u, txBuffer, rxBuffer ) );

    RunTasks( tl, 3u + 4u );
    TEST_ASSERT_EQUAL( MODBUS_MASTER_REQ_BUSY, tl.Get_RequestStatus() );

    RunTasks( tl, 1u );
    TEST_ASSERT_EQUAL( MODBUS_MASTER_REQ_DONE, tl.Get_RequestStatus() );
    TEST_ASSERT_EQUAL_UINT32( 0u, phy.rxStartCnt );
    TEST_ASSERT_EQUAL( MODBUS_RX_NOT_RCVD, tl.Get_RxMessageStatus( nullptr ) );
}


void Ut_MasterTL_TransmissionTimeout( void )
{
    UtModBus::FakePhy              phy;
    ModBus::Master::TransportLayer tl( phy );
    uint8_t                        txBuffer[ 16 ] = { 0u };
    uint8_t                        rxBuffer[ ModBus::PhysicalLayer::messageBufferMaxLen ] = { 0u };

    phy.txAutoFinish = false;

    PrepareRequest( txBuffer, 0x11u );
    TEST_ASSERT_EQUAL( MODBUS_REQUEST_OK, tl.StartTransmit( 6u, txBuffer, rxBuffer ) );

    RunTasks( tl, 2u + ( ModBus::Master::TransportLayer::modBus_TransmissionTimeout_ms / UT_TASK_TIME ) );

    TEST_ASSERT_EQUAL( MODBUS_MASTER_REQ_TX_ERROR, tl.Get_RequestStatus() );
    TEST_ASSERT_EQUAL_UINT32( 1u, phy.txStopCnt );
}


void Ut_MasterTL_StartTransmit_RejectedWhenBusyOrInvalid( void )
{
    UtModBus::FakePhy              phy;
    ModBus::Master::TransportLayer tl( phy );
    uint8_t                        txBuffer[ 16 ] = { 0u };
    uint8_t                        rxBuffer[ ModBus::PhysicalLayer::messageBufferMaxLen ] = { 0u };

    PrepareRequest( txBuffer, 0x11u );

    TEST_ASSERT_EQUAL( MODBUS_REQUEST_ERROR, tl.StartTransmit( 6u, nullptr, rxBuffer ) );
    TEST_ASSERT_EQUAL( MODBUS_REQUEST_ERROR, tl.StartTransmit( 6u, txBuffer, nullptr ) );
    TEST_ASSERT_EQUAL( MODBUS_REQUEST_ERROR, tl.StartTransmit( 1u, txBuffer, rxBuffer ) );
    TEST_ASSERT_EQUAL( MODBUS_REQUEST_ERROR, tl.StartTransmit( 255u, txBuffer, rxBuffer ) );

    TEST_ASSERT_EQUAL( MODBUS_REQUEST_OK,    tl.StartTransmit( 6u, txBuffer, rxBuffer ) );
    TEST_ASSERT_EQUAL( MODBUS_REQUEST_ERROR, tl.StartTransmit( 6u, txBuffer, rxBuffer ) );
}


/** New request can be started after previous one has finished */
void Ut_MasterTL_SecondRequestAfterTimeout( void )
{
    UtModBus::FakePhy              phy;
    ModBus::Master::TransportLayer tl( phy );
    uint8_t                        txBuffer[ 16 ] = { 0u };
    uint8_t                        rxBuffer[ ModBus::PhysicalLayer::messageBufferMaxLen ] = { 0u };

    tl.Set_ResponseTimeout( 20u );

    PrepareRequest( txBuffer, 0x11u );
    TEST_ASSERT_EQUAL( MODBUS_REQUEST_OK, tl.StartTransmit( 6u, txBuffer, rxBuffer ) );
    RunTasks( tl, 10u );
    TEST_ASSERT_EQUAL( MODBUS_MASTER_REQ_TIMEOUT, tl.Get_RequestStatus() );

    PrepareRequest( txBuffer, 0x11u );
    TEST_ASSERT_EQUAL( MODBUS_REQUEST_OK, tl.StartTransmit( 6u, txBuffer, rxBuffer ) );
    RunTasks( tl, 1u );

    TEST_ASSERT_EQUAL_UINT32( 2u, phy.txStartCnt );
    TEST_ASSERT_EQUAL_UINT16( 8u, phy.txLen );
}

