/**
 * \author Mr.Nobody
 * \file UtModBus_FakePhy.hpp
 * \ingroup UnitTests
 * \brief Fake physical layer and common helpers of ModBus unit tests.
 *
 * Fake records all calls of transport layer and allows to inject received
 * frames and to read transmitted frames.
 */

#ifndef MODBUS_UNITTESTS_UTMODBUS_FAKEPHY_HPP
#define MODBUS_UNITTESTS_UTMODBUS_FAKEPHY_HPP

/* ============================= INCLUDES =================================== */
#include "ModBus_Phy.hpp"                   /* Physical layer interface       */
#include "ModBus_Core_Types.h"              /* Module types definition        */
/* ============================= TYPEDEFS =================================== */

namespace UtModBus
{
    class FakePhy : public ModBus::PhysicalLayer
    {
    public:

        void              Set_Transmission_Start  ( const uint8_t *msgBuffer, modBus_MsgLength_t msgLen ) override;
        void              Set_Transmission_Stop   ( void ) override;
        modBus_TxStatus_t Get_Transmission_Status ( void ) override;

        void              Set_Reception_Start     ( void ) override;
        void              Set_Reception_Stop      ( void ) override;
        modBus_RxStatus_t Get_Reception_Status    ( void ) override;

        void              Set_ProcessingActive    ( void ) override;
        void              Set_ProcessingInactive  ( void ) override;

        void              Set_Baudrate            ( modBus_Baudrate_t baudrate ) override;
        void              Set_Parity              ( modBus_Parity_t parity )     override;
        void              Set_StopBits            ( modBus_StopBits_t stopbits ) override;
        void              PhyInit                 ( void )                       override;

        uint8_t *          Get_RxMessageBuff      ( void ) override;
        modBus_MsgLength_t Get_RxMessageLen       ( void ) override;

        /* ------------------------- Test helpers --------------------------- */

        /** Stores frame into reception buffer, optionally appends CRC and
         * signals finished reception */
        void ReceiveFrame( const uint8_t *frame, modBus_MsgLength_t frameLen, bool appendCrc = true );

        /** Reception buffer */
        uint8_t rxBuffer[ messageBufferMaxLen ] = { 0u };

        /** Length of received frame */
        modBus_MsgLength_t rxLen = 0u;

        /** Reception status reported to transport layer */
        modBus_RxStatus_t rxStatus = MODBUS_RX_NOT_RCVD;

        /** Copy of last transmitted frame */
        uint8_t txFrame[ messageBufferMaxLen ] = { 0u };

        /** Length of last transmitted frame */
        modBus_MsgLength_t txLen = 0u;

        /** Transmission status reported to transport layer */
        modBus_TxStatus_t txStatus = MODBUS_TX_ACTIVE;

        /** Transmission is reported as finished immediately */
        bool txAutoFinish = true;

        /* Call counters */
        uint32_t txStartCnt  = 0u;
        uint32_t txStopCnt   = 0u;
        uint32_t rxStartCnt  = 0u;
        uint32_t rxStopCnt   = 0u;
        uint32_t initCnt     = 0u;
        uint32_t procActCnt  = 0u;
        uint32_t procInacCnt = 0u;

        /** Bus configuration */
        modBus_BusConfig_t busConfig = { 0u, MODBUS_PARITY_NONE, MODBUS_STOPBIT_1 };
    };


    /** Calculates ModBus CRC (independent reference implementation) */
    uint16_t Crc16( const uint8_t *data, modBus_MsgLength_t len );
}

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ========================= EXPORTED MACROS ================================ */

/* ========================= EXPORTED VARIABLES ============================= */

/* ======================== EXPORTED FUNCTIONS ============================== */

#endif /* MODBUS_UNITTESTS_UTMODBUS_FAKEPHY_HPP */
