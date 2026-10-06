/**
 * \author Mr.Nobody
 * \file ModBus_App_WMR.cpp
 * \ingroup ModBus
 * \brief ModBus Write Multiple Registers (WMR, 0x10) application layer handler
 *
 * # (0x10) Write Multiple Registers #
 *
 * This function code is used to write a block of contiguous registers (1 to
 * 123 registers) in a remote device. The requested written values are packed
 * as two bytes per register. The normal response returns the function code,
 * starting address, and quantity of registers written.
 *
 * | Request               | Size     | Value            |
 * |-----------------------|----------|------------------|
 * | Function code         | 1 Byte   | 0x10             |
 * | Starting Address      | 2 Bytes  | 0x0000 to 0xFFFF |
 * | Quantity of Registers | 2 Bytes  | 1 to 123         |
 * | Byte Count            | 1 Byte   | 2 x N            |
 * | Registers Value       | N x 2 B  |                  |
 *
 * | Response              | Size     | Value            |
 * |-----------------------|----------|------------------|
 * | Function code         | 1 Byte   | 0x10             |
 * | Starting Address      | 2 Bytes  | 0x0000 to 0xFFFF |
 * | Quantity of Registers | 2 Bytes  | 1 to 123         |
 *
 * Exceptions: 0x02 (address range), 0x03 (quantity, byte count, request
 * length), 0x04 (setter failed).
 *
 * Example - write registers 2 and 3: request `10 00 01 00 02 04 00 0A 01 02`,
 * response `10 00 01 00 02`.
 */
/* ============================= INCLUDES =================================== */
#include "ModBus_App_WMR.hpp"           /* Self include                       */
#include "ModBus_App_DataAccess.hpp"    /* Common data access                 */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ============================== MACROS ==================================== */

/* ========================= EXPORTED VARIABLES ============================= */

/* ========================== LOCAL VARIABLES =============================== */

/* ======================== EXPORTED FUNCTIONS ============================== */

/**
 * \brief Write Multiple Registers handler constructor
 *
 * \param holdingRegistersHandler [in]: Registers configuration array (indexed by address)
 * \param registersCount          [in]: Count of items in configuration array
 */
ModBus::AppLayer::WMR::WMR( const modBus_RegistersConfigStruct_t *holdingRegistersHandler, modBus_RegistersCount_t registersCount ) :
    registersHandler( holdingRegistersHandler ),
    supportedRegistersCount( registersCount )
{
    return;
}


/**
 * \brief Processes Write Multiple Registers (WMR) request
 *
 * \param msgBuffer [in,out]: Request, overwritten by response
 * \param msgLength [in,out]: Length of request / response without CRC
 *
 * \return Exception code or \ref MODBUS_ERROR_NO_ACTIVE_ERROR
 */
modBus_ErrorStatus_t ModBus::AppLayer::WMR::SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength )
{
    modBus_ErrorStatus_t errorStatus = MODBUS_ERROR_NO_ACTIVE_ERROR;

    if( ( nullptr == msgBuffer ) || ( registerValues_BuffPos > msgLength ) )
    {
        errorStatus = MODBUS_ERROR_ILLEGAL_DATA_VALUE;
    }
    else
    {
        const uint16_t startingAddr = Get_16b( msgBuffer, startingAddr_BuffPos );
        const uint16_t quantity     = Get_16b( msgBuffer, quantity_BuffPos );
        const uint8_t  byteCount    = msgBuffer[ byteCount_BuffPos ];

        if( ( 0u                                       == quantity  ) ||
            ( maxQuantity                              <  quantity  ) ||
            ( ( 2u * quantity )                        != byteCount ) ||
            ( ( registerValues_BuffPos + byteCount )   != msgLength )    )
        {
            errorStatus = MODBUS_ERROR_ILLEGAL_DATA_VALUE;
        }
        else
        {
            errorStatus = DataAccess::Check_RegistersRange( registersHandler, supportedRegistersCount,
                                                            startingAddr, quantity, MODBUS_DATA_ACCESS_WRITE );
        }

        for( uint16_t regIndex = 0u; ( MODBUS_ERROR_NO_ACTIVE_ERROR == errorStatus ) && ( quantity > regIndex ); regIndex++ )
        {
            const modBus_RegistersValue_t regValue = Get_16b( msgBuffer, registerValues_BuffPos + ( 2u * regIndex ) );

            errorStatus = DataAccess::Set_Register( registersHandler, (uint16_t)( startingAddr + regIndex ), regValue );
        }

        if( MODBUS_ERROR_NO_ACTIVE_ERROR == errorStatus )
        {
            /* Response contains function code, starting address and quantity (already in buffer) */
            msgLength = responseLen;
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
