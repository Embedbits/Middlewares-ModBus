/**
 * \author Mr.Nobody
 * \file Test_ModBus_SlaveTL.cpp
 * \ingroup UnitTests
 * \brief Unit tests of ModBus slave transport layer.
 */

/* ============================= INCLUDES =================================== */
#include "unity.h"                          /* Unity testing framework        */
#include "UtModBus_FakePhy.hpp"             /* Fake physical layer            */
#include "ModBus_Slave_TL.hpp"              /* Code under test                */
/* ========================= SYMBOLIC CONSTANTS ============================= */

/** Slave address used in tests */
static constexpr modBus_DeviceAddr_t UT_SLAVE_ADDR = 0x11u;

/** Task period used in tests in ms */
static constexpr modBus_Time_t UT_TASK_TIME = 10u;

/* ========================== LOCAL FUNCTIONS =============================== */

/** Runs task of transport layer */
static void RunTasks( ModBus::Slave::TransportLayer &tl, uint32_t count )
{
    for( uint32_t taskIndex = 0u; count > taskIndex; taskIndex++ )
    {
        tl.Task( UT_TASK_TIME );
    }
}


/** Activates transport layer and brings it into standby state */
static void StartSlave( ModBus::Slave::TransportLayer &tl )
{
    TEST_ASSERT_EQUAL( MODBUS_REQUEST_OK, tl.Set_SlaveAddr( UT_SLAVE_ADDR ) );

    tl.Activate();

    RunTasks( tl, 1u );

    TEST_ASSERT_EQUAL( MODBUS_SLAVE_TL_STANDBY, tl.Get_ModuleState() );
}


/** Receives request and runs tasks until it is pending for processing */
static void ReceiveRequest( UtModBus::FakePhy &phy, ModBus::Slave::TransportLayer &tl,
                            const uint8_t *frame, modBus_MsgLength_t frameLen, bool appendCrc = true )
{
    phy.ReceiveFrame( frame, frameLen, appendCrc );

    RunTasks( tl, 2u );
}

/* ======================== EXPORTED FUNCTIONS ============================== */

void setUp( void )
{
}


void tearDown( void )
{
}


void Ut_SlaveTL_StaysInInitUntilActivated( void )
{
    UtModBus::FakePhy             phy;
    ModBus::Slave::TransportLayer tl( phy );

    RunTasks( tl, 5u );

    TEST_ASSERT_EQUAL( MODBUS_SLAVE_TL_INIT, tl.Get_ModuleState() );
    TEST_ASSERT_EQUAL_UINT32( 0u, phy.initCnt );
}


/** Physical layer is initialized once on activation and reception is started */
void Ut_SlaveTL_ActivationInitializesPhyOnce( void )
{
    UtModBus::FakePhy             phy;
    ModBus::Slave::TransportLayer tl( phy );

    StartSlave( tl );
    RunTasks( tl, 10u );

    TEST_ASSERT_EQUAL_UINT32( 1u, phy.initCnt );
    TEST_ASSERT_EQUAL_UINT32( 1u, phy.rxStartCnt );
    TEST_ASSERT_EQUAL( MODBUS_SLAVE_TL_STANDBY, tl.Get_ModuleState() );
}


void Ut_SlaveTL_SetSlaveAddr_RejectsBroadcastAndReserved( void )
{
    UtModBus::FakePhy             phy;
    ModBus::Slave::TransportLayer tl( phy );

    TEST_ASSERT_EQUAL( MODBUS_REQUEST_ERROR, tl.Set_SlaveAddr( 0u ) );
    TEST_ASSERT_EQUAL( MODBUS_REQUEST_ERROR, tl.Set_SlaveAddr( 248u ) );
    TEST_ASSERT_EQUAL( MODBUS_REQUEST_OK,    tl.Set_SlaveAddr( 247u ) );
    TEST_ASSERT_EQUAL_UINT8( 247u, tl.Get_SlaveAddr() );
}


void Ut_SlaveTL_AddressMatch_RequestPending( void )
{
    UtModBus::FakePhy             phy;
    ModBus::Slave::TransportLayer tl( phy );
    const uint8_t                 request[] = { UT_SLAVE_ADDR, 0x03, 0x00, 0x00, 0x00, 0x01 };

    StartSlave( tl );
    ReceiveRequest( phy, tl, request, sizeof( request ) );

    const modBus_StatsCounter_t stats = tl.Get_StatCounter();

    TEST_ASSERT_EQUAL( MODBUS_APP_MSG_NEW_PENDING, tl.Get_NewMessageStatus() );
    TEST_ASSERT_EQUAL_UINT16( sizeof( request ), tl.Get_NewMessageLen() );
    TEST_ASSERT_EQUAL_PTR( phy.rxBuffer, tl.Get_RxMessagePtr() );
    TEST_ASSERT_EQUAL_UINT32( 1u, stats.framesRcvdCnt );
    TEST_ASSERT_EQUAL_UINT32( 1u, stats.addrMatchedCnt );
    TEST_ASSERT_EQUAL_UINT32( 0u, stats.addrNotMatchedCnt );
    TEST_ASSERT_EQUAL_UINT32( 0u, stats.crcErrorCount );
}


/** Only own and broadcast address is accepted, all other addresses are ignored */
void Ut_SlaveTL_AllAddresses_OnlyOwnAndBroadcastAccepted( void )
{
    UtModBus::FakePhy             phy;
    ModBus::Slave::TransportLayer tl( phy );
    uint8_t                       request[] = { 0x00, 0x03, 0x00, 0x00, 0x00, 0x01 };

    StartSlave( tl );

    for( uint16_t addr = 0u; 256u > addr; addr++ )
    {
        request[ 0 ] = (uint8_t)addr;

        tl.Reset_StatCounter();
        ReceiveRequest( phy, tl, request, sizeof( request ) );

        const modBus_StatsCounter_t stats   = tl.Get_StatCounter();
        const bool                  matched = ( UT_SLAVE_ADDR == addr ) || ( 0u == addr );

        TEST_ASSERT_EQUAL_UINT32( matched ? 1u : 0u, stats.addrMatchedCnt );
        TEST_ASSERT_EQUAL_UINT32( matched ? 0u : 1u, stats.addrNotMatchedCnt );
        TEST_ASSERT_EQUAL( matched ? MODBUS_APP_MSG_NEW_PENDING : MODBUS_APP_MSG_NO_NEW, tl.Get_NewMessageStatus() );

        if( matched )
        {
            /* Finish processing without response */
            tl.Set_MessageReady( tl.Get_RxMessagePtr(), 0u );
            RunTasks( tl, 1u );
        }

        TEST_ASSERT_EQUAL( MODBUS_SLAVE_TL_STANDBY, tl.Get_ModuleState() );
    }
}


void Ut_SlaveTL_CrcError_FrameDropped( void )
{
    UtModBus::FakePhy             phy;
    ModBus::Slave::TransportLayer tl( phy );
    const uint8_t                 frame[] = { UT_SLAVE_ADDR, 0x03, 0x00, 0x00, 0x00, 0x01, 0x12, 0x34 };

    StartSlave( tl );
    ReceiveRequest( phy, tl, frame, sizeof( frame ), false );

    TEST_ASSERT_EQUAL( MODBUS_APP_MSG_NO_NEW, tl.Get_NewMessageStatus() );
    TEST_ASSERT_EQUAL_UINT32( 1u, tl.Get_StatCounter().crcErrorCount );
    TEST_ASSERT_EQUAL( MODBUS_SLAVE_TL_STANDBY, tl.Get_ModuleState() );
    TEST_ASSERT_EQUAL_UINT32( 0u, phy.txStartCnt );
}


/** Frame shorter than address + function code + CRC is invalid even with valid CRC */
void Ut_SlaveTL_TooShortFrame_Dropped( void )
{
    UtModBus::FakePhy             phy;
    ModBus::Slave::TransportLayer tl( phy );
    const uint8_t                 frame[] = { UT_SLAVE_ADDR };

    StartSlave( tl );
    ReceiveRequest( phy, tl, frame, sizeof( frame ) );

    TEST_ASSERT_EQUAL( MODBUS_APP_MSG_NO_NEW, tl.Get_NewMessageStatus() );
    TEST_ASSERT_EQUAL_UINT32( 1u, tl.Get_StatCounter().crcErrorCount );
}


/** Minimal frame (address + function code + CRC) is accepted */
void Ut_SlaveTL_MinimalFrame_Accepted( void )
{
    UtModBus::FakePhy             phy;
    ModBus::Slave::TransportLayer tl( phy );
    const uint8_t                 frame[] = { UT_SLAVE_ADDR, 0x11 };

    StartSlave( tl );
    ReceiveRequest( phy, tl, frame, sizeof( frame ) );

    TEST_ASSERT_EQUAL( MODBUS_APP_MSG_NEW_PENDING, tl.Get_NewMessageStatus() );
    TEST_ASSERT_EQUAL_UINT16( 2u, tl.Get_NewMessageLen() );
}


/** Response gets CRC appended and is transmitted, then reception restarts */
void Ut_SlaveTL_Response_TransmittedWithCrc( void )
{
    UtModBus::FakePhy             phy;
    ModBus::Slave::TransportLayer tl( phy );
    const uint8_t                 request[] = { 0x01, 0x03, 0x00, 0x00, 0x00, 0x0A };

    TEST_ASSERT_EQUAL( MODBUS_REQUEST_OK, tl.Set_SlaveAddr( 0x01u ) );
    tl.Activate();
    RunTasks( tl, 1u );

    ReceiveRequest( phy, tl, request, sizeof( request ) );

    uint8_t *buffer = tl.Get_RxMessagePtr();

    tl.Set_MessageReady( buffer, sizeof( request ) );

    RunTasks( tl, 1u );

    TEST_ASSERT_EQUAL( MODBUS_SLAVE_TL_SENDING, tl.Get_ModuleState() );

    const uint8_t expected[] = { 0x01, 0x03, 0x00, 0x00, 0x00, 0x0A, 0xC5, 0xCD };

    TEST_ASSERT_EQUAL_UINT32( 1u, phy.txStartCnt );
    TEST_ASSERT_EQUAL_UINT16( sizeof( expected ), phy.txLen );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( expected, phy.txFrame, sizeof( expected ) );

    RunTasks( tl, 1u );

    TEST_ASSERT_EQUAL( MODBUS_SLAVE_TL_STANDBY, tl.Get_ModuleState() );
    TEST_ASSERT_EQUAL_UINT32( 1u, phy.txStopCnt );
    TEST_ASSERT_EQUAL_UINT32( 2u, phy.rxStartCnt );
}


/** Slave waits in sending state until physical layer finishes transmission */
void Ut_SlaveTL_Response_WaitsForTransmissionEnd( void )
{
    UtModBus::FakePhy             phy;
    ModBus::Slave::TransportLayer tl( phy );
    const uint8_t                 request[] = { UT_SLAVE_ADDR, 0x03, 0x00, 0x00, 0x00, 0x01 };

    phy.txAutoFinish = false;

    StartSlave( tl );
    ReceiveRequest( phy, tl, request, sizeof( request ) );
    tl.Set_MessageReady( tl.Get_RxMessagePtr(), sizeof( request ) );

    RunTasks( tl, 10u );

    TEST_ASSERT_EQUAL( MODBUS_SLAVE_TL_SENDING, tl.Get_ModuleState() );

    phy.txStatus = MODBUS_TX_FINISHED;
    RunTasks( tl, 1u );

    TEST_ASSERT_EQUAL( MODBUS_SLAVE_TL_STANDBY, tl.Get_ModuleState() );
}


/** Zero response length means no response (broadcast, listen only mode) */
void Ut_SlaveTL_ZeroLengthResponse_NotTransmitted( void )
{
    UtModBus::FakePhy             phy;
    ModBus::Slave::TransportLayer tl( phy );
    const uint8_t                 request[] = { 0x00, 0x06, 0x00, 0x00, 0x00, 0x01 };

    StartSlave( tl );
    ReceiveRequest( phy, tl, request, sizeof( request ) );
    tl.Set_MessageReady( tl.Get_RxMessagePtr(), 0u );
    RunTasks( tl, 1u );

    TEST_ASSERT_EQUAL( MODBUS_SLAVE_TL_STANDBY, tl.Get_ModuleState() );
    TEST_ASSERT_EQUAL_UINT32( 0u, phy.txStartCnt );
}


/** Response longer than frame (without CRC) is not transmitted */
void Ut_SlaveTL_TooLongResponse_NotTransmitted( void )
{
    UtModBus::FakePhy             phy;
    ModBus::Slave::TransportLayer tl( phy );
    const uint8_t                 request[] = { UT_SLAVE_ADDR, 0x03, 0x00, 0x00, 0x00, 0x01 };

    StartSlave( tl );
    ReceiveRequest( phy, tl, request, sizeof( request ) );
    tl.Set_MessageReady( tl.Get_RxMessagePtr(), 255u );
    RunTasks( tl, 1u );

    TEST_ASSERT_EQUAL( MODBUS_SLAVE_TL_STANDBY, tl.Get_ModuleState() );
    TEST_ASSERT_EQUAL_UINT32( 0u, phy.txStartCnt );
}


void Ut_SlaveTL_SetMessageReady_IgnoredWithoutPendingRequest( void )
{
    UtModBus::FakePhy             phy;
    ModBus::Slave::TransportLayer tl( phy );
    uint8_t                       buffer[ 8 ] = { 0u };

    StartSlave( tl );
    tl.Set_MessageReady( buffer, 4u );
    RunTasks( tl, 3u );

    TEST_ASSERT_EQUAL( MODBUS_SLAVE_TL_STANDBY, tl.Get_ModuleState() );
    TEST_ASSERT_EQUAL_UINT32( 0u, phy.txStartCnt );
}


void Ut_SlaveTL_Deactivate_ReturnsToInit( void )
{
    UtModBus::FakePhy             phy;
    ModBus::Slave::TransportLayer tl( phy );

    StartSlave( tl );
    tl.Deactivate();
    RunTasks( tl, 2u );

    TEST_ASSERT_EQUAL( MODBUS_SLAVE_TL_INIT, tl.Get_ModuleState() );
    TEST_ASSERT_EQUAL_UINT32( 1u, phy.rxStopCnt );
}


/** Physical layer is re-initialized after 30 minutes without any frame */
void Ut_SlaveTL_StandbyTimeout_ReinitializesPhy( void )
{
    UtModBus::FakePhy             phy;
    ModBus::Slave::TransportLayer tl( phy );
    const modBus_Time_t           taskTime  = 1000u;
    const uint32_t                tasksIn30 = ( 30u * 60u * 1000u ) / taskTime;

    StartSlave( tl );

    for( uint32_t taskIndex = 0u; ( tasksIn30 - 2u ) > taskIndex; taskIndex++ )
    {
        tl.Task( taskTime );
    }

    TEST_ASSERT_EQUAL_UINT32( 1u, phy.initCnt );

    for( uint32_t taskIndex = 0u; 4u > taskIndex; taskIndex++ )
    {
        tl.Task( taskTime );
    }

    TEST_ASSERT_EQUAL_UINT32( 2u, phy.initCnt );
    TEST_ASSERT_EQUAL( MODBUS_SLAVE_TL_STANDBY, tl.Get_ModuleState() );
}


void Ut_SlaveTL_ResetStatCounter( void )
{
    UtModBus::FakePhy             phy;
    ModBus::Slave::TransportLayer tl( phy );
    const uint8_t                 frame[] = { 0x55, 0x03, 0x00, 0x00, 0x00, 0x01 };

    StartSlave( tl );
    ReceiveRequest( phy, tl, frame, sizeof( frame ) );
    tl.Reset_StatCounter();

    const modBus_StatsCounter_t stats = tl.Get_StatCounter();

    TEST_ASSERT_EQUAL_UINT32( 0u, stats.framesRcvdCnt );
    TEST_ASSERT_EQUAL_UINT32( 0u, stats.addrNotMatchedCnt );
}

