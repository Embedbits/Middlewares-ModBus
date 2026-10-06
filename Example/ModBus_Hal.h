/**
 * \author Mr.Nobody
 * \file ModBus_Hal.h
 * \ingroup ModBus
 * \brief Hardware interface used by USART physical layer (ModBus_PhyUsart.cpp).
 *
 * Functions are implemented by the application (BSP), typically over USART
 * with DMA:
 * - Reception uses USART receiver timeout (or timer) to detect end of frame
 *   (silent interval of 3.5 characters).
 * - Transmission controls RS-485 driver enable (DE) pin, eg. by USART driver
 *   enable feature.
 */

#ifndef MODBUS_MODBUS_HAL_H
#define MODBUS_MODBUS_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================= INCLUDES =================================== */
#include "ModBus_Core_Types.h"              /* Module types definition        */
/* ============================= TYPEDEFS =================================== */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ========================= EXPORTED MACROS ================================ */

/* ========================= EXPORTED VARIABLES ============================= */

/* ======================== EXPORTED FUNCTIONS ============================== */

/** Initializes peripheral with requested bus configuration */
void                    ModBus_Hal_Init                 ( const modBus_BusConfig_t *busConfig );

/** Starts transmission of buffer (buffer stays valid until transmission stop) */
void                    ModBus_Hal_StartTransmit        ( const uint8_t *buffer, modBus_MsgLength_t length );
/** Stops transmission */
void                    ModBus_Hal_StopTransmit         ( void );
/** Returns MODBUS_TX_FINISHED when the last byte (incl. stop bits) was sent */
modBus_TxStatus_t       ModBus_Hal_Get_TxStatus         ( void );

/** Starts reception into buffer of bufferSize bytes */
void                    ModBus_Hal_StartReceive         ( uint8_t *buffer, modBus_MsgLength_t bufferSize );
/** Stops reception */
void                    ModBus_Hal_StopReceive          ( void );
/** Returns MODBUS_RX_READY when end of frame was detected, rcvdLength is count of received bytes */
modBus_RxStatus_t       ModBus_Hal_Get_RxStatus         ( modBus_MsgLength_t *rcvdLength );

/** Controls communication activity indication (eg. LED) */
void                    ModBus_Hal_Set_ActivityLed      ( uint8_t ledActive );

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_MODBUS_HAL_H */
