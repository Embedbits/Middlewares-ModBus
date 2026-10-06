/**
 * \author Mr.Nobody
 * \file ModBus_App_RFIFO.cpp
 * \ingroup ModBus
 * \brief ModBus Read FIFO Queue (RFIFO, 0x18) application layer handler
 *
 * # (0x18) Read FIFO Queue #
 *
 * This function code allows to read the contents of a First-In-First-Out
 * (FIFO) queue of registers in a remote device. The function returns a count
 * of the registers in the queue, followed by the queued data. Up to 31 queued
 * data registers can be read. The function reads the queue contents, but does
 * not clear them. If the queue count exceeds 31, an exception response is
 * returned with an error code of 03 (Illegal Data Value).
 *
 * | Request               | Size     | Value            |
 * |-----------------------|----------|------------------|
 * | Function code         | 1 Byte   | 0x18             |
 * | FIFO Pointer Address  | 2 Bytes  | 0x0000 to 0xFFFF |
 *
 * | Response              | Size     | Value            |
 * |-----------------------|----------|------------------|
 * | Function code         | 1 Byte   | 0x18             |
 * | Byte count            | 2 Bytes  | 2 + 2 x N        |
 * | FIFO Count            | 2 Bytes  | N <= 31          |
 * | FIFO Value Register   | N x 2 B  |                  |
 *
 * Exceptions: 0x02 (FIFO address not supported), 0x03 (FIFO count, request
 * length).
 *
 * Example - FIFO at 1246 with 2 values: request `18 04 DE`, response
 * `18 00 06 00 02 01 B8 12 84`.
 */
/* ============================= INCLUDES =================================== */
#include "ModBus_App_RFIFO.hpp"         /* Self include                       */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ============================== MACROS ==================================== */

/* ========================= EXPORTED VARIABLES ============================= */

/* ========================== LOCAL VARIABLES =============================== */

/* ======================== EXPORTED FUNCTIONS ============================== */

/**
 * \brief Read FIFO Queue handler constructor
 *
 * \param fifoCallback [in]: Callback providing content of FIFO queue
 */
ModBus::AppLayer::RFIFO::RFIFO( modBus_FifoCallback_t *fifoCallback ) :
    fifoCallback( fifoCallback )
{
    return;
}


/**
 * \brief Processes Read FIFO Queue (RFIFO) request
 *
 * \param msgBuffer [in,out]: Request, overwritten by response
 * \param msgLength [in,out]: Length of request / response without CRC
 *
 * \return Exception code or \ref MODBUS_ERROR_NO_ACTIVE_ERROR
 */
modBus_ErrorStatus_t ModBus::AppLayer::RFIFO::SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength )
{
    modBus_ErrorStatus_t errorStatus = MODBUS_ERROR_NO_ACTIVE_ERROR;

    if( ( nullptr == msgBuffer ) || ( requestLen != msgLength ) )
    {
        errorStatus = MODBUS_ERROR_ILLEGAL_DATA_VALUE;
    }
    else if( nullptr == fifoCallback )
    {
        errorStatus = MODBUS_ERROR_ILLEGAL_DATA_ADDRESS;
    }
    else
    {
        const uint16_t          fifoAddress = Get_16b( msgBuffer, fifoAddr_BuffPos );
        modBus_RegistersValue_t fifoValues[ MODBUS_RFIFO_MAX_COUNT ] = { 0u };
        uint16_t                fifoCount   = 0u;

        if( MODBUS_REQUEST_OK != fifoCallback( fifoAddress, fifoValues, &fifoCount ) )
        {
            errorStatus = MODBUS_ERROR_ILLEGAL_DATA_ADDRESS;
        }
        else if( MODBUS_RFIFO_MAX_COUNT < fifoCount )
        {
            errorStatus = MODBUS_ERROR_ILLEGAL_DATA_VALUE;
        }
        else
        {
            Set_16b( msgBuffer, byteCount_BuffPos, (uint16_t)( 2u + ( 2u * fifoCount ) ) );
            Set_16b( msgBuffer, fifoCount_BuffPos, fifoCount );

            for( uint16_t valueIndex = 0u; fifoCount > valueIndex; valueIndex++ )
            {
                Set_16b( msgBuffer, fifoValues_BuffPos + ( 2u * valueIndex ), fifoValues[ valueIndex ] );
            }

            msgLength = fifoValues_BuffPos + ( 2u * fifoCount );
        }
    }

    return ( errorStatus );
}

/* ========================== LOCAL FUNCTIONS =============================== */

/* =============================== TASKS ==================================== */
