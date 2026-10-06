/**
 * \author Mr.Nobody
 * \file ModBus_Port.h
 * \ingroup ModBus
 * \brief ModBus handler module public functionality
 *
 * This file contains all available public functionality, any other files shall
 * not used outside of the module.
 *
 * Usage:
 * \code
 * ModBus_Init();
 *
 * while( 1 )
 * {
 *     ModBus_Task();    // called every MODBUS_TASK_PERIOD_MS (ModBus.h)
 * }
 * \endcode
 */

#ifndef MODBUS_MODBUS_PORT_H
#define MODBUS_MODBUS_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================== INCLUDES ================================== */
#include "ModBus_Types.h"                   /* Module types definition        */
/* ============================== TYPEDEFS ================================== */

/* ========================== SYMBOLIC CONSTANTS ============================ */

/* ========================== EXPORTED MACROS =============================== */

/* ========================== EXPORTED VARIABLES ============================ */

/* ========================= EXPORTED FUNCTIONS ============================= */

modBus_ModuleVersion_t      ModBus_Get_ModuleVersion           ( void );

void                        ModBus_Init                        ( void );
void                        ModBus_Deinit                      ( void );
void                        ModBus_Task                        ( void );

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_MODBUS_PORT_H */
