/**
 * \author Mr.Nobody
 * \file ModBus_Phy.hpp
 * \ingroup ModBus
 * \brief ModBus Physical Layer (PL) interface.
 *
 * Physical layer is implemented by user (see Example/ModBus_PhyUsart.cpp) and
 * it is used by transport layer (slave or master). Transport layer works only
 * by polling - the physical layer does not need any callback into the module.
 *
 * Expected behavior of implementation:
 * - Reception is started by \ref Set_Reception_Start. Received bytes are stored
 *   into own buffer (\ref Get_RxMessageBuff). End of frame is detected by the
 *   physical layer (eg. USART receiver timeout of 3.5 characters). After that
 *   \ref Get_Reception_Status returns \ref MODBUS_RX_READY and
 *   \ref Get_RxMessageLen returns count of received bytes (including CRC).
 * - Transmission of buffer is started by \ref Set_Transmission_Start. After the
 *   last byte is physically sent \ref Get_Transmission_Status returns
 *   \ref MODBUS_TX_FINISHED.
 */

#ifndef MODBUS_PHY_HPP
#define MODBUS_PHY_HPP
/* ============================= INCLUDES =================================== */
#include "ModBus_Core_Types.h"              /* Module C types definition      */
/* ============================= TYPEDEFS =================================== */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ========================= EXPORTED MACROS ================================ */

/* ========================= EXPORTED VARIABLES ============================= */

/* ======================== EXPORTED FUNCTIONS ============================== */


namespace ModBus
{
    class PhysicalLayer
    {
    public:

        virtual ~PhysicalLayer() = default;

        /**
         * \brief Starts transmission of frame.
         *
         * \param msgBuffer [in]: Frame to be transmitted (including CRC). Buffer
         *                        stays valid until transmission is stopped.
         * \param msgLen    [in]: Count of bytes in frame
         */
        virtual void              Set_Transmission_Start  ( const uint8_t *msgBuffer,
                                                            modBus_MsgLength_t msgLen )  = 0;
        /** \brief Stops (finishes) transmission */
        virtual void              Set_Transmission_Stop   ( void )                       = 0;
        /** \brief Returns status of transmission started by \ref Set_Transmission_Start */
        virtual modBus_TxStatus_t Get_Transmission_Status ( void )                       = 0;

        /** \brief Starts reception of new frame into reception buffer */
        virtual void              Set_Reception_Start     ( void )                       = 0;
        /** \brief Stops reception */
        virtual void              Set_Reception_Stop      ( void )                       = 0;
        /** \brief Returns \ref MODBUS_RX_READY if complete frame has been received */
        virtual modBus_RxStatus_t Get_Reception_Status    ( void )                       = 0;

        /** \brief Signals start of frame processing (eg. activity LED on) */
        virtual void              Set_ProcessingActive    ( void )                       = 0;
        /** \brief Signals end of frame processing (eg. activity LED off) */
        virtual void              Set_ProcessingInactive  ( void )                       = 0;

        /** \brief Configures baudrate. Applied by next \ref PhyInit */
        virtual void              Set_Baudrate            ( modBus_Baudrate_t baudrate ) = 0;
        /** \brief Configures parity. Applied by next \ref PhyInit */
        virtual void              Set_Parity              ( modBus_Parity_t parity )     = 0;
        /** \brief Configures stop bits. Applied by next \ref PhyInit */
        virtual void              Set_StopBits            ( modBus_StopBits_t stopbits ) = 0;
        /** \brief (Re)initializes peripheral with actual configuration */
        virtual void              PhyInit                 ( void )                       = 0;

        /**
         * \brief Returns pointer to reception message buffer
         *
         * Buffer size shall be at least \ref messageBufferMaxLen bytes. Slave
         * builds response directly in this buffer.
         *
         * \return Pointer to reception message buffer
         */
        virtual uint8_t * Get_RxMessageBuff( void ) = 0;

        /** \brief Returns count of bytes of received frame (including CRC) */
        virtual modBus_MsgLength_t Get_RxMessageLen( void ) = 0;

        /** Maximal length (count of bytes) of ModBus frame */
        static constexpr modBus_MsgLength_t messageBufferMaxLen = MODBUS_FRAME_MAX_LEN;

    private:

    };
}

#endif /* MODBUS_PHY_HPP */
