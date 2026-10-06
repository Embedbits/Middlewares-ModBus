/**
 * \author Mr.Nobody
 * \file ModBus.h
 * \ingroup ModBus
 * \brief ModBus handler module common functionality header file.
 *
 * This file contains the configuration and common functionality used
 * internally by the module.
 *
 */

#ifndef MODBUS_MODBUS_H
#define MODBUS_MODBUS_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================= INCLUDES =================================== */
#include "ModBus_Types.h"                   /* Module types definitions       */
/* ============================= TYPEDEFS =================================== */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/** Period of ModBus_Task() execution in ms [0.001s] */
#define MODBUS_TASK_PERIOD_MS                   ( 1u )

/** Slave address of the device [1-247] */
#define MODBUS_SLAVE_ADDRESS                    ( 0x01u )

/** Bus baudrate in bit/s */
#define MODBUS_BUS_BAUDRATE                     ( 19200u )

/** Bus parity (ModBus RTU default is even parity) */
#define MODBUS_BUS_PARITY                       ( MODBUS_PARITY_EVEN )

/** Bus stop bits */
#define MODBUS_BUS_STOPBITS                     ( MODBUS_STOPBIT_1 )

/* ========================= EXPORTED MACROS ================================ */

/* ========================= EXPORTED VARIABLES ============================= */

/* ======================== EXPORTED FUNCTIONS ============================== */

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_MODBUS_H */
