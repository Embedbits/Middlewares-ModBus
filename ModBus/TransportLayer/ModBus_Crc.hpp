/**
 * \author Mr.Nobody
 * \file ModBus_Crc.hpp
 * \ingroup ModBus
 * \brief ModBus CRC calculation functionality module header file.
 */

#ifndef MODBUS_CRC_HPP
#define MODBUS_CRC_HPP
/* ============================= INCLUDES =================================== */
#include "ModBus_Core_Types.h"              /* Module C types definition      */
/* ============================= TYPEDEFS =================================== */

/** \brief Enumeration signaling actual state of CRC check */
typedef enum
{
    MODBUS_CRC_INCORRECT = 0u,/**< CRC value is incorrect */
    MODBUS_CRC_OK             /**< CRC value corresponds  */
}   modBus_CrcState_t;

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ========================= EXPORTED MACROS ================================ */

/* ========================= EXPORTED VARIABLES ============================= */

/* ======================== EXPORTED FUNCTIONS ============================== */

namespace ModBus
{
    class Crc
    {
    public:
        Crc( void ) = default;

        uint16_t          Get_Crc  ( const uint8_t * const buff, modBus_MsgLength_t buffLength ) const;
        modBus_CrcState_t Check_Crc( const uint8_t * const buff, modBus_MsgLength_t buffLength ) const;
        void              Add_Crc  ( uint8_t * const buff, modBus_MsgLength_t * const buffLength ) const;

    private:

        /** Initial register value of CRC */
        static constexpr uint16_t CrcInitialValue = 0xFFFF;

        /** Polynomial used for CRC calculation (0x8005 reflected) */
        static constexpr uint16_t CrcPolynomial = 0xA001;

    };
}

#endif /* MODBUS_CRC_HPP */
