/**
 * \author Mr.Nobody
 * \file ModBus_PhyUsart.hpp
 * \ingroup ModBus
 * \brief ModBus USART Physical Layer (PL) header file.
 */

#ifndef MODBUS_MODBUS_PHYUSART_HPP
#define MODBUS_MODBUS_PHYUSART_HPP
/* ============================= INCLUDES =================================== */
#include "ModBus_Phy.hpp"                   /* Physical layer interface       */
#include "ModBus_Core_Types.h"              /* Module C types definition      */
/* ============================= TYPEDEFS =================================== */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ========================= EXPORTED MACROS ================================ */

/* ========================= EXPORTED VARIABLES ============================= */

/* ======================== EXPORTED FUNCTIONS ============================== */

namespace ModBus
{
    class UsartPhysicalLayer : public PhysicalLayer
    {
    public:
        UsartPhysicalLayer( void ) = default;

        void              Set_Transmission_Start  ( const uint8_t *msgBuffer, modBus_MsgLength_t msgLen ) override;
        void              Set_Transmission_Stop   ( void )                       override;
        modBus_TxStatus_t Get_Transmission_Status ( void )                       override;

        void              Set_Reception_Start     ( void )                       override;
        void              Set_Reception_Stop      ( void )                       override;
        modBus_RxStatus_t Get_Reception_Status    ( void )                       override;

        void              Set_ProcessingActive    ( void )                       override;
        void              Set_ProcessingInactive  ( void )                       override;

        void              Set_Baudrate            ( modBus_Baudrate_t baudrate ) override;
        void              Set_Parity              ( modBus_Parity_t parity )     override;
        void              Set_StopBits            ( modBus_StopBits_t stopbits ) override;
        void              PhyInit                 ( void )                       override;

        uint8_t *          Get_RxMessageBuff      ( void )                       override;
        modBus_MsgLength_t Get_RxMessageLen       ( void )                       override;

    private:

        /** Buffer used for ModBus message reception (and slave response) */
        uint8_t messageRxBuffer[ messageBufferMaxLen ] = { 0u };

        /** Count of received bytes */
        modBus_MsgLength_t messageRxLen = 0u;

        /** Physical layer configuration, applied by PhyInit */
        modBus_BusConfig_t busConfiguration = { 19200u, MODBUS_PARITY_EVEN, MODBUS_STOPBIT_1 };
    };
} /* namespace ModBus */

#endif /* MODBUS_MODBUS_PHYUSART_HPP */
