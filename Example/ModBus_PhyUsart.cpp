/**
 * \author Mr.Nobody
 * \file ModBus_PhyUsart.cpp
 * \ingroup ModBus
 * \brief ModBus USART Physical Layer (PL) implemented over ModBus_Hal.h
 *
 * Default configuration is 19200 Bd, even parity, 1 stop bit (ModBus RTU
 * default). Configuration changed by setters is applied by next PhyInit.
 */

/* ============================= INCLUDES =================================== */
#include "ModBus_PhyUsart.hpp"              /* Self include                   */
#include "ModBus_Core_Types.h"              /* Module C types definition      */
#include "ModBus_Hal.h"                     /* Hardware interface             */
/* ======================== EXPORTED FUNCTIONS ============================== */

void ModBus::UsartPhysicalLayer::Set_Transmission_Start( const uint8_t *msgBuffer, modBus_MsgLength_t msgLen )
{
    ModBus_Hal_StartTransmit( msgBuffer, msgLen );
}


void ModBus::UsartPhysicalLayer::Set_Transmission_Stop( void )
{
    ModBus_Hal_StopTransmit();
}


modBus_TxStatus_t ModBus::UsartPhysicalLayer::Get_Transmission_Status( void )
{
    return ( ModBus_Hal_Get_TxStatus() );
}


void ModBus::UsartPhysicalLayer::Set_Reception_Start( void )
{
    messageRxLen = 0u;

    ModBus_Hal_StartReceive( messageRxBuffer, messageBufferMaxLen );
}


void ModBus::UsartPhysicalLayer::Set_Reception_Stop( void )
{
    ModBus_Hal_StopReceive();
}


modBus_RxStatus_t ModBus::UsartPhysicalLayer::Get_Reception_Status( void )
{
    modBus_MsgLength_t rcvdLength = 0u;
    modBus_RxStatus_t  rxStatus   = ModBus_Hal_Get_RxStatus( &rcvdLength );

    if( MODBUS_RX_READY == rxStatus )
    {
        messageRxLen = ( messageBufferMaxLen < rcvdLength ) ? messageBufferMaxLen : rcvdLength;
    }
    else
    {
        /* Frame is not complete */
    }

    return ( rxStatus );
}


void ModBus::UsartPhysicalLayer::Set_ProcessingActive( void )
{
    ModBus_Hal_Set_ActivityLed( 1u );
}


void ModBus::UsartPhysicalLayer::Set_ProcessingInactive( void )
{
    ModBus_Hal_Set_ActivityLed( 0u );
}


void ModBus::UsartPhysicalLayer::Set_Baudrate( modBus_Baudrate_t baudrate )
{
    busConfiguration.Baudrate = baudrate;
}


void ModBus::UsartPhysicalLayer::Set_Parity( modBus_Parity_t parity )
{
    busConfiguration.Parity = parity;
}


void ModBus::UsartPhysicalLayer::Set_StopBits( modBus_StopBits_t stopbits )
{
    busConfiguration.StopBits = stopbits;
}


void ModBus::UsartPhysicalLayer::PhyInit( void )
{
    ModBus_Hal_Init( &busConfiguration );
}


uint8_t *ModBus::UsartPhysicalLayer::Get_RxMessageBuff( void )
{
    return ( messageRxBuffer );
}


modBus_MsgLength_t ModBus::UsartPhysicalLayer::Get_RxMessageLen( void )
{
    return ( messageRxLen );
}
