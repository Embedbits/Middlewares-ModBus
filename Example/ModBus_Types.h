/**
 * \defgroup ModBus ModBus
 * \brief ModBus module
 */

/**
 * \author Mr.Nobody
 * \file ModBus_Types.h
 * \ingroup ModBus
 * \brief ModBus handler module global types definition
 *
 * This file contains the types definitions used across the module and are
 * available for other modules through Port file. Types of ModBus stack
 * (ThirdParty/ModBus) are provided by ModBus_Core_Types.h.
 *
 */

#ifndef MODBUS_MODBUS_TYPES_H
#define MODBUS_MODBUS_TYPES_H
/* ============================== INCLUDES ================================== */
#include "stdint.h"                         /* Standard types definition      */
#include "ModBus_Core_Types.h"              /* ModBus stack common types      */
/* ========================== SYMBOLIC CONSTANTS ============================ */

/* ========================== EXPORTED MACROS =============================== */

/* ============================== TYPEDEFS ================================== */

/** \brief Type signaling major version of SW module */
typedef uint8_t modBus_MajorVersion_t;


/** \brief Type signaling minor version of SW module */
typedef uint8_t modBus_MinorVersion_t;


/** \brief Type signaling patch version of SW module */
typedef uint8_t modBus_PatchVersion_t;


/** \brief Type signaling actual version of SW module */
typedef struct
{
    modBus_MajorVersion_t Major; /**< Major version */
    modBus_MinorVersion_t Minor; /**< Minor version */
    modBus_PatchVersion_t Patch; /**< Patch version */
}   modBus_ModuleVersion_t;

/* ========================== EXPORTED VARIABLES ============================ */

/* ========================= EXPORTED FUNCTIONS ============================= */

#endif /* MODBUS_MODBUS_TYPES_H */
