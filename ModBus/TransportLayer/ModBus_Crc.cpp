/**
 * \author Mr.Nobody
 * \file ModBus_Crc.cpp
 * \ingroup ModBus
 * \brief ModBus RTU CRC-16 calculation functionality.
 *
 * CRC is appended to the frame as low byte first, high byte second.
 */

/* ============================= INCLUDES =================================== */
#include "ModBus_Crc.hpp"                   /* Self include                   */
#include "ModBus_Core_Types.h"              /* Module C types definition      */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ============================== MACROS ==================================== */

/* ========================= EXPORTED VARIABLES ============================= */

/* ========================== LOCAL VARIABLES =============================== */

/* ======================== EXPORTED FUNCTIONS ============================== */

/**
 * \brief Calculates CRC value of message for ModBus RTU
 *
 * \param buff       [in]: Pointer to message buffer
 * \param buffLength [in]: Count of bytes used for calculation
 *
 * \return Value of CRC calculated from message buffer (low byte is transmitted
 *         first). Value 0 is returned in case of null pointer.
 */
uint16_t ModBus::Crc::Get_Crc( const uint8_t * const buff, modBus_MsgLength_t buffLength ) const
{
    uint16_t crcRegister = CrcInitialValue;

    if( nullptr != buff )
    {
        for( modBus_MsgLength_t buffIndex = 0u; buffLength > buffIndex; buffIndex++ )
        {
            /* XOR byte into least sig. byte of crc */
            crcRegister ^= ( uint16_t ) buff[ buffIndex ];

            /* Loop over each bit */
            for( uint8_t bitIndex = 8u; 0u != bitIndex; bitIndex-- )
            {
                /* Check if the LSB is set */
                if( 0u != ( crcRegister & 0x0001u ) )
                {
                    /* Shift right and XOR POLYNOMIAL */
                    crcRegister >>= 1u;
                    crcRegister ^= CrcPolynomial;
                }
                else /* Else LSB is not set */
                {
                    /* Just shift right */
                    crcRegister >>= 1u;
                }
            }
        }
    }
    else
    {
        crcRegister = 0u;
    }

    return crcRegister;
}


/**
 * \brief Checks if the CRC in received message is correct
 *
 * \param buff       [in]: Pointer to message buffer
 * \param buffLength [in]: Length of received message in bytes (including CRC)
 *
 * \return Validity state of CRC in received message. Message shorter than CRC
 *         itself or null pointer is reported as incorrect.
 */
modBus_CrcState_t ModBus::Crc::Check_Crc( const uint8_t * const buff, modBus_MsgLength_t buffLength ) const
{
    modBus_CrcState_t retState = MODBUS_CRC_INCORRECT;

    if( ( nullptr        != buff       ) &&
        ( MODBUS_CRC_LEN <  buffLength )    )
    {
        /* Calculate CRC value from received message. Last two bytes are ignored, they contain CRC value */
        const uint16_t calcCrcValue = Get_Crc( buff, buffLength - MODBUS_CRC_LEN );
        const uint16_t rcvdCrcValue = ModBus_Get_16bFromTwo8b( buff[ buffLength - 1u ], buff[ buffLength - 2u ] );

        if( calcCrcValue == rcvdCrcValue )
        {
            retState = MODBUS_CRC_OK;
        }
        else
        {
            retState = MODBUS_CRC_INCORRECT;
        }
    }
    else
    {
        /* Invalid message */
        retState = MODBUS_CRC_INCORRECT;
    }

    return ( retState );
}


/**
 * \brief Appends CRC value to the message
 *
 * CRC is calculated from first buffLength bytes and stored behind them. The
 * buffer shall have space for two more bytes.
 *
 * \param buff       [in/out]: Pointer to message buffer
 * \param buffLength [in/out]: Length of message without CRC. Increased by CRC length.
 */
void ModBus::Crc::Add_Crc( uint8_t * const buff, modBus_MsgLength_t * const buffLength ) const
{
    if( ( nullptr != buff       ) &&
        ( nullptr != buffLength )    )
    {
        const uint16_t calcCrcValue = Get_Crc( buff, *buffLength );

        buff[ *buffLength      ] = ModBus_Get_Lower8bFrom16b( calcCrcValue );
        buff[ *buffLength + 1u ] = ModBus_Get_Upper8bFrom16b( calcCrcValue );

        *buffLength = *buffLength + MODBUS_CRC_LEN;
    }
    else
    {
        /* Null pointer has been assigned */
    }
}

/* ========================== LOCAL FUNCTIONS =============================== */

/* =============================== TASKS ==================================== */
