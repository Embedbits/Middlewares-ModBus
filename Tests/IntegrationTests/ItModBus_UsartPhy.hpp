/**
 * \author Mr.Nobody
 * \file ItModBus_UsartPhy.hpp
 * \ingroup ModBus
 * \brief ModBus physical layer over USART MCAL used by integration tests.
 *
 * Physical layer of integration tests - ModBus stack runs over real USART
 * peripheral (USART MCAL, interrupt or DMA data handling). End of frame is
 * detected by USART receiver timeout (3.5 characters).
 *
 * Besides the physical layer function the object records communication for
 * the test evaluation:
 * - copy of the last received frame (raw bytes incl. CRC) and count of frames,
 * - copy of the last transmitted frame and count of frames,
 * - count of USART transfer errors (parity, framing, noise, overrun, DMA).
 *
 * USART callbacks have no context parameter, so every object is bound to one
 * port slot (\ref itModBus_PhyPort_t) with its own set of callbacks.
 */

#ifndef ITMODBUS_USARTPHY_HPP
#define ITMODBUS_USARTPHY_HPP
/* ============================= INCLUDES =================================== */
#include "ModBus_Phy.hpp"                   /* Physical layer interface       */
#include "ModBus_Core_Types.h"              /* Module C types definition      */
#include "Usart_Port.h"                     /* USART MCAL                     */
#include "Gpio_Port.h"                      /* RX pin pull-up                 */
/* ============================= TYPEDEFS =================================== */

/** \brief Port slot of physical layer object (selects USART callbacks) */
typedef enum
{
    IT_MODBUS_PHY_PORT_MASTER = 0u, /**< Physical layer of master */
    IT_MODBUS_PHY_PORT_SLAVE,       /**< Physical layer of slave  */
    IT_MODBUS_PHY_PORT_CNT          /**< Count of port slots      */
}   itModBus_PhyPort_t;

/** \brief Hardware resources of one USART port */
typedef struct
{
    usart_PeriphId_t     Bus;            /**< USART peripheral                        */
    usart_TxPin_t        TxPin;          /**< TX pin                                  */
    usart_RxPin_t        RxPin;          /**< RX pin                                  */
    gpio_PortId_t        RxGpioPort;     /**< GPIO port of RX pin (pull-up)           */
    gpio_PinId_t         RxGpioPin;      /**< GPIO pin of RX pin (pull-up)            */
    usart_DmaChannelId_t TxDmaChannel;   /**< GPDMA1 channel of transmission (DMA)    */
    usart_DmaChannelId_t RxDmaChannel;   /**< GPDMA1 channel of reception (DMA)       */
}   itModBus_PortConfig_t;

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ========================= EXPORTED MACROS ================================ */

/* ========================= EXPORTED VARIABLES ============================= */

/* ======================== EXPORTED FUNCTIONS ============================== */

namespace ItModBus
{
    class UsartPhy : public ModBus::PhysicalLayer
    {
    public:

        UsartPhy( itModBus_PhyPort_t port, const itModBus_PortConfig_t &portConfig );

        /* ------------------ ModBus::PhysicalLayer interface ---------------- */
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

        /* ----------------------- Test configuration ------------------------ */
        void Set_XferMode( usart_XferMode_t xferMode );

        /* ------------------------- Test evaluation ------------------------- */
        bool               Get_InitOk( void ) const;
        uint32_t           Get_RxFrameCnt( void ) const;
        const uint8_t     *Get_LastRxFrame( void ) const;
        modBus_MsgLength_t Get_LastRxFrameLen( void ) const;
        uint32_t           Get_TxFrameCnt( void ) const;
        const uint8_t     *Get_LastTxFrame( void ) const;
        modBus_MsgLength_t Get_LastTxFrameLen( void ) const;
        uint32_t           Get_ErrorCnt( void ) const;
        uint32_t           Get_TxStartErrorCnt( void ) const;
        usart_XferErrorId_t Get_LastErrorId( void ) const;
        void               Reset_Records( void );
        void               Clear_Errors( void );

        /* -------------------- USART events (callbacks) --------------------- */
        void On_TxComplete( void );
        void On_RxEnd( usart_RxDataCnt_t rxCnt );
        void On_Error( usart_XferErrorId_t errorId );

    private:

        /** Bits of one character - start, 8 data bits, parity and stop bits */
        uint32_t Get_CharacterBits( void ) const;

        /** Port slot (USART callbacks) */
        const itModBus_PhyPort_t phyPort;

        /** Hardware resources */
        const itModBus_PortConfig_t &hwConfig;

        /** Data handling configuration (copied by USART module) */
        usart_DataConfig_t dataConfig = {};

        /** Data transfer mode of both directions */
        usart_XferMode_t xferMode = USART_XFER_MODE_ISR;

        /** Bus configuration, applied by PhyInit */
        modBus_BusConfig_t busConfiguration = { 19200u, MODBUS_PARITY_EVEN, MODBUS_STOPBIT_1 };

        /** Reception buffer (USART), slave builds response in it */
        uint8_t messageRxBuffer[ messageBufferMaxLen ] = { 0u };

        /** Count of bytes of the received frame */
        modBus_MsgLength_t messageRxLen = 0u;

        /** End of frame detected by USART (callback context) */
        volatile bool rxFrameEnd = false;

        /** Count of bytes reported by end of frame (callback context) */
        volatile modBus_MsgLength_t rxFrameEndCnt = 0u;

        /** Received frame was reported to transport layer */
        bool rxFrameReported = false;

        /** Transmission finished (callback context) */
        volatile bool txFinished = true;

        /** Physical layer initialized successfully */
        bool initOk = false;

        /* ------------------------------ Records ------------------------------ */
        uint8_t            lastRxFrame[ messageBufferMaxLen ] = { 0u };
        modBus_MsgLength_t lastRxFrameLen = 0u;
        uint32_t           rxFrameCnt     = 0u;
        uint8_t            lastTxFrame[ messageBufferMaxLen ] = { 0u };
        modBus_MsgLength_t lastTxFrameLen = 0u;
        uint32_t           txFrameCnt     = 0u;
        volatile uint32_t  errorCnt       = 0u;
        volatile usart_XferErrorId_t lastErrorId = USART_XFER_ERROR_CNT;
        uint32_t           txStartErrCnt  = 0u;
    };
} /* namespace ItModBus */

#endif /* ITMODBUS_USARTPHY_HPP */
