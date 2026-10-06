/**
 * \author Mr.Nobody
 * \file ModBus_App_WSR.cpp
 * \ingroup ModBus
 * \brief ModBus Write Single Register (WSR, 0x06) application layer handler
 *
 * # (0x06) Write Single Register #
 *
 * This function code is used to write a single holding register in a remote
 * device. Registers are addressed starting at zero. The normal response is an
 * echo of the request, returned after the register contents have been written.
 *
 * | Request / Response    | Size    | Value            |
 * |-----------------------|---------|------------------|
 * | Function code         | 1 Byte  | 0x06             |
 * | Register Address      | 2 Bytes | 0x0000 to 0xFFFF |
 * | Register Value        | 2 Bytes | 0x0000 to 0xFFFF |
 *
 * Exceptions: 0x02 (address), 0x03 (request length), 0x04 (setter failed).
 *
 * Example - write register 2 to 0x0003: request `06 00 01 00 03`,
 * response `06 00 01 00 03`.
 */
/* ============================= INCLUDES =================================== */
#include "ModBus_App_WSR.hpp"           /* Self include                       */
#include "ModBus_App_DataAccess.hpp"    /* Common data access                 */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ============================== MACROS ==================================== */

/* ========================= EXPORTED VARIABLES ============================= */

/* ========================== LOCAL VARIABLES =============================== */

/* ======================== EXPORTED FUNCTIONS ============================== */

/**
 * \brief Write Single Register handler constructor
 *
 * \param holdingRegistersHandler [in]: Registers configuration array (indexed by address)
 * \param registersCount          [in]: Count of items in configuration array
 */
ModBus::AppLayer::WSR::WSR( const modBus_RegistersConfigStruct_t *holdingRegistersHandler, modBus_RegistersCount_t registersCount ) :
    registersHandler( holdingRegistersHandler ),
    supportedRegistersCount( registersCount )
{
    return;
}


/**
 * \brief Processes Write Single Register (WSR) request
 *
 * \param msgBuffer [in,out]: Request, response is echo of the request
 * \param msgLength [in,out]: Length of request / response without CRC
 *
 * \return Exception code or \ref MODBUS_ERROR_NO_ACTIVE_ERROR
 */
modBus_ErrorStatus_t ModBus::AppLayer::WSR::SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength )
{
    modBus_ErrorStatus_t errorStatus = MODBUS_ERROR_NO_ACTIVE_ERROR;

    if( ( nullptr == msgBuffer ) || ( requestLen != msgLength ) )
    {
        errorStatus = MODBUS_ERROR_ILLEGAL_DATA_VALUE;
    }
    else
    {
        const uint16_t registerAddr  = Get_16b( msgBuffer, registerAddr_BuffPos );
        const uint16_t registerValue = Get_16b( msgBuffer, registerValue_BuffPos );

        errorStatus = DataAccess::Check_RegistersRange( registersHandler, supportedRegistersCount,
                                                        registerAddr, 1u, MODBUS_DATA_ACCESS_WRITE );

        if( MODBUS_ERROR_NO_ACTIVE_ERROR == errorStatus )
        {
            errorStatus = DataAccess::Set_Register( registersHandler, registerAddr, registerValue );

            /* Response is echo of request */
            msgLength = requestLen;
        }
        else
        {
            /* Error is returned */
        }
    }

    return ( errorStatus );
}

/* ========================== LOCAL FUNCTIONS =============================== */

/* =============================== TASKS ==================================== */
