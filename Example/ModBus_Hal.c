/**
 * \author Mr.Nobody
 * \file ModBus_Hal.c
 * \ingroup ModBus
 * \brief Hardware interface of ModBus physical layer - implementation skeleton.
 *
 * Connect the functions to the BSP (eg. USART MCAL with DMA, receiver timeout
 * and driver enable for RS-485). Until then the skeleton never receives any
 * frame, thus the slave stays silent.
 */

/* ============================== INCLUDES ================================== */
#include "ModBus_Hal.h"                     /* Self include                   */
/* ============================== TYPEDEFS ================================== */

/* ======================== FORWARD DECLARATIONS ============================ */

/* ========================== SYMBOLIC CONSTANTS ============================ */

/* =============================== MACROS =================================== */

/* ========================== EXPORTED VARIABLES ============================ */

/* =========================== LOCAL VARIABLES ============================== */

/* ========================= EXPORTED FUNCTIONS ============================= */

/**
 * \brief Initializes peripheral with requested bus configuration
 *
 * \param busConfig [in]: Baudrate, parity and stop bits
 */
void ModBus_Hal_Init( const modBus_BusConfig_t *busConfig )
{
    (void)busConfig;

    /* TODO: USART initialization (8 data bits, receiver timeout 3.5 characters, RS-485 DE) */
}


/**
 * \brief Starts transmission of buffer
 *
 * \param buffer [in]: Frame to be transmitted
 * \param length [in]: Count of bytes
 */
void ModBus_Hal_StartTransmit( const uint8_t *buffer, modBus_MsgLength_t length )
{
    (void)buffer;
    (void)length;

    /* TODO: start DMA / interrupt transmission */
}


/**
 * \brief Stops transmission
 */
void ModBus_Hal_StopTransmit( void )
{
    /* TODO: stop transmission */
}


/**
 * \brief Returns status of transmission
 *
 * \return \ref MODBUS_TX_FINISHED when the last byte was sent
 */
modBus_TxStatus_t ModBus_Hal_Get_TxStatus( void )
{
    /* TODO: transmission complete flag */
    return ( MODBUS_TX_FINISHED );
}


/**
 * \brief Starts reception into buffer
 *
 * \param buffer     [in]: Reception buffer
 * \param bufferSize [in]: Size of reception buffer
 */
void ModBus_Hal_StartReceive( uint8_t *buffer, modBus_MsgLength_t bufferSize )
{
    (void)buffer;
    (void)bufferSize;

    /* TODO: start DMA / interrupt reception, enable receiver timeout */
}


/**
 * \brief Stops reception
 */
void ModBus_Hal_StopReceive( void )
{
    /* TODO: stop reception */
}


/**
 * \brief Returns status of reception
 *
 * \param rcvdLength [out]: Count of received bytes
 *
 * \return \ref MODBUS_RX_READY when end of frame was detected
 */
modBus_RxStatus_t ModBus_Hal_Get_RxStatus( modBus_MsgLength_t *rcvdLength )
{
    /* TODO: receiver timeout flag and count of received bytes */
    *rcvdLength = 0u;

    return ( MODBUS_RX_NOT_RCVD );
}


/**
 * \brief Controls communication activity indication
 *
 * \param ledActive [in]: Non-zero value activates indication
 */
void ModBus_Hal_Set_ActivityLed( uint8_t ledActive )
{
    (void)ledActive;

    /* TODO: activity LED */
}

/* =========================== LOCAL FUNCTIONS ============================== */

/* =========================== INTERRUPT HANDLERS =========================== */

/* ================================ TASKS =================================== */
