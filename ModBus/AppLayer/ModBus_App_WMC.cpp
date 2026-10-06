/**
 * \author Mr.Nobody
 * \file ModBus_App_WMC.cpp
 * \ingroup ModBus
 * \brief ModBus Write Multiple Coils (WMC, 0x0F) application layer handler
 *
 * # (0x0F) Write Multiple Coils #
 *
 * This function code is used to force each coil in a sequence of coils to
 * either ON or OFF in a remote device. A logical '1' in a bit position of the
 * data field requests the corresponding output to be ON. The LSB of the first
 * data byte corresponds to the first output. The normal response returns the
 * function code, starting address, and quantity of coils forced.
 *
 * | Request               | Size    | Value            |
 * |-----------------------|---------|------------------|
 * | Function code         | 1 Byte  | 0x0F             |
 * | Starting Address      | 2 Bytes | 0x0000 to 0xFFFF |
 * | Quantity of Outputs   | 2 Bytes | 1 to 1968        |
 * | Byte Count            | 1 Byte  | N = ceil(Q / 8)  |
 * | Outputs Value         | N Bytes |                  |
 *
 * | Response              | Size    | Value            |
 * |-----------------------|---------|------------------|
 * | Function code         | 1 Byte  | 0x0F             |
 * | Starting Address      | 2 Bytes | 0x0000 to 0xFFFF |
 * | Quantity of Outputs   | 2 Bytes | 1 to 1968        |
 *
 * Exceptions: 0x02 (address range), 0x03 (quantity, byte count, request
 * length), 0x04 (setter failed).
 *
 * Example - write 10 coils starting at coil 20: request
 * `0F 00 13 00 0A 02 CD 01`, response `0F 00 13 00 0A`.
 */
/* ============================= INCLUDES =================================== */
#include "ModBus_App_WMC.hpp"           /* Self include                       */
#include "ModBus_App_DataAccess.hpp"    /* Common data access                 */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ============================== MACROS ==================================== */

/* ========================= EXPORTED VARIABLES ============================= */

/* ========================== LOCAL VARIABLES =============================== */

/* ======================== EXPORTED FUNCTIONS ============================== */

/**
 * \brief Write Multiple Coils handler constructor
 *
 * \param coilsHandler [in]: Coils configuration array (indexed by coil address)
 * \param coilsCount   [in]: Count of items in configuration array
 */
ModBus::AppLayer::WMC::WMC( const modBus_CoilsConfigStruct_t *coilsHandler, modBus_CoilsCount_t coilsCount ) :
    coilsHandler( coilsHandler ),
    supportedCoilsCount( coilsCount )
{
    return;
}


/**
 * \brief Processes Write Multiple Coils (WMC) request
 *
 * \param msgBuffer [in,out]: Request, overwritten by response
 * \param msgLength [in,out]: Length of request / response without CRC
 *
 * \return Exception code or \ref MODBUS_ERROR_NO_ACTIVE_ERROR
 */
modBus_ErrorStatus_t ModBus::AppLayer::WMC::SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength )
{
    modBus_ErrorStatus_t errorStatus = MODBUS_ERROR_NO_ACTIVE_ERROR;

    if( ( nullptr == msgBuffer ) || ( outputsValue_BuffPos > msgLength ) )
    {
        errorStatus = MODBUS_ERROR_ILLEGAL_DATA_VALUE;
    }
    else
    {
        const uint16_t startingAddr = Get_16b( msgBuffer, startingAddr_BuffPos );
        const uint16_t quantity     = Get_16b( msgBuffer, quantity_BuffPos );
        const uint8_t  byteCount    = msgBuffer[ byteCount_BuffPos ];

        if( ( 0u                                    == quantity                ) ||
            ( maxQuantity                           <  quantity                ) ||
            ( ( ( quantity + 7u ) / 8u )            != byteCount               ) ||
            ( ( outputsValue_BuffPos + byteCount )  != msgLength               )    )
        {
            errorStatus = MODBUS_ERROR_ILLEGAL_DATA_VALUE;
        }
        else
        {
            errorStatus = DataAccess::Check_BitsRange( coilsHandler, supportedCoilsCount, startingAddr, quantity, MODBUS_DATA_ACCESS_WRITE );
        }

        for( uint16_t coilIndex = 0u; ( MODBUS_ERROR_NO_ACTIVE_ERROR == errorStatus ) && ( quantity > coilIndex ); coilIndex++ )
        {
            const uint8_t                outputsByte = msgBuffer[ outputsValue_BuffPos + ( coilIndex / 8u ) ];
            const modBus_CoilsRetValue_t coilValue   = ( 0u != ( outputsByte & ( 1u << ( coilIndex % 8u ) ) ) ) ?
                                                       MODBUS_COIL_ACTIVE : MODBUS_COIL_INACTIVE;

            errorStatus = DataAccess::Set_Bit( coilsHandler, (uint16_t)( startingAddr + coilIndex ), coilValue );
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
