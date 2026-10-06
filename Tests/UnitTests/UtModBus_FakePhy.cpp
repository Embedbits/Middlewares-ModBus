/**
 * \author Mr.Nobody
 * \file UtModBus_FakePhy.cpp
 * \ingroup UnitTests
 * \brief Fake physical layer and common helpers of ModBus unit tests.
 */

/* ============================= INCLUDES =================================== */
#include "UtModBus_FakePhy.hpp"             /* Self include                   */
/* ======================== EXPORTED FUNCTIONS ============================== */

void UtModBus::FakePhy::Set_Transmission_Start( const uint8_t *msgBuffer, modBus_MsgLength_t msgLen )
{
    txStartCnt++;
    txLen = msgLen;

    for( modBus_MsgLength_t byteIndex = 0u; ( msgLen > byteIndex ) && ( messageBufferMaxLen > byteIndex ); byteIndex++ )
    {
        txFrame[ byteIndex ] = msgBuffer[ byteIndex ];
    }

    txStatus = txAutoFinish ? MODBUS_TX_FINISHED : MODBUS_TX_ACTIVE;
}


void UtModBus::FakePhy::Set_Transmission_Stop( void )
{
    txStopCnt++;
}


modBus_TxStatus_t UtModBus::FakePhy::Get_Transmission_Status( void )
{
    return ( txStatus );
}


void UtModBus::FakePhy::Set_Reception_Start( void )
{
    rxStartCnt++;
    rxStatus = MODBUS_RX_NOT_RCVD;
}


void UtModBus::FakePhy::Set_Reception_Stop( void )
{
    rxStopCnt++;
}


modBus_RxStatus_t UtModBus::FakePhy::Get_Reception_Status( void )
{
    return ( rxStatus );
}


void UtModBus::FakePhy::Set_ProcessingActive( void )
{
    procActCnt++;
}


void UtModBus::FakePhy::Set_ProcessingInactive( void )
{
    procInacCnt++;
}


void UtModBus::FakePhy::Set_Baudrate( modBus_Baudrate_t baudrate )
{
    busConfig.Baudrate = baudrate;
}


void UtModBus::FakePhy::Set_Parity( modBus_Parity_t parity )
{
    busConfig.Parity = parity;
}


void UtModBus::FakePhy::Set_StopBits( modBus_StopBits_t stopbits )
{
    busConfig.StopBits = stopbits;
}


void UtModBus::FakePhy::PhyInit( void )
{
    initCnt++;
}


uint8_t *UtModBus::FakePhy::Get_RxMessageBuff( void )
{
    return ( rxBuffer );
}


modBus_MsgLength_t UtModBus::FakePhy::Get_RxMessageLen( void )
{
    return ( rxLen );
}


void UtModBus::FakePhy::ReceiveFrame( const uint8_t *frame, modBus_MsgLength_t frameLen, bool appendCrc )
{
    for( modBus_MsgLength_t byteIndex = 0u; frameLen > byteIndex; byteIndex++ )
    {
        rxBuffer[ byteIndex ] = frame[ byteIndex ];
    }

    rxLen = frameLen;

    if( appendCrc )
    {
        const uint16_t crc = Crc16( frame, frameLen );

        rxBuffer[ rxLen      ] = (uint8_t)( crc & 0xFFu );
        rxBuffer[ rxLen + 1u ] = (uint8_t)( crc >> 8u );

        rxLen = rxLen + 2u;
    }

    rxStatus = MODBUS_RX_READY;
}


uint16_t UtModBus::Crc16( const uint8_t *data, modBus_MsgLength_t len )
{
    uint16_t crc = 0xFFFFu;

    for( modBus_MsgLength_t byteIndex = 0u; len > byteIndex; byteIndex++ )
    {
        crc ^= data[ byteIndex ];

        for( uint8_t bitIndex = 0u; 8u > bitIndex; bitIndex++ )
        {
            crc = ( 0u != ( crc & 1u ) ) ? (uint16_t)( ( crc >> 1u ) ^ 0xA001u ) : (uint16_t)( crc >> 1u );
        }
    }

    return ( crc );
}
