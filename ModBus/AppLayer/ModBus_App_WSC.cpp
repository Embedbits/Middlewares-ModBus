/**
 * \author Mr.Nobody
 * \file ModBus_App_WSC.cpp
 * \ingroup ModBus
 * \brief ModBus Write Single Coil (WSC, 0x05) application layer handler
 *
 * # (0x05) Write Single Coil #
 *
 * This function code is used to write a single output to either ON or OFF in
 * a remote device. A value of 0xFF00 requests the output to be ON, a value of
 * 0x0000 requests it to be OFF. All other values are illegal and do not affect
 * the output. The normal response is an echo of the request, returned after
 * the coil state has been written.
 *
 * | Request / Response    | Size    | Value            |
 * |-----------------------|---------|------------------|
 * | Function code         | 1 Byte  | 0x05             |
 * | Output Address        | 2 Bytes | 0x0000 to 0xFFFF |
 * | Output Value          | 2 Bytes | 0x0000 or 0xFF00 |
 *
 * Exceptions: 0x02 (address), 0x03 (value, request length),
 * 0x04 (setter failed).
 *
 * Example - set coil 173 ON: request `05 00 AC FF 00`, response `05 00 AC FF 00`.
 */
/* ============================= INCLUDES =================================== */
#include "ModBus_App_WSC.hpp"           /* Self include                       */
#include "ModBus_App_DataAccess.hpp"    /* Common data access                 */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ============================== MACROS ==================================== */

/* ========================= EXPORTED VARIABLES ============================= */

/* ========================== LOCAL VARIABLES =============================== */

/* ======================== EXPORTED FUNCTIONS ============================== */

/**
 * \brief Write Single Coil handler constructor
 *
 * \param coilsHandler [in]: Coils configuration array (indexed by coil address)
 * \param coilsCount   [in]: Count of items in configuration array
 */
ModBus::AppLayer::WSC::WSC( const modBus_CoilsConfigStruct_t *coilsHandler, modBus_CoilsCount_t coilsCount ) :
    coilsHandler( coilsHandler ),
    supportedCoilsCount( coilsCount )
{
    return;
}


/**
 * \brief Processes Write Single Coil (WSC) request
 *
 * \param msgBuffer [in,out]: Request, response is echo of the request
 * \param msgLength [in,out]: Length of request / response without CRC
 *
 * \return Exception code or \ref MODBUS_ERROR_NO_ACTIVE_ERROR
 */
modBus_ErrorStatus_t ModBus::AppLayer::WSC::SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength )
{
    modBus_ErrorStatus_t errorStatus = MODBUS_ERROR_NO_ACTIVE_ERROR;

    if( ( nullptr == msgBuffer ) || ( requestLen != msgLength ) )
    {
        errorStatus = MODBUS_ERROR_ILLEGAL_DATA_VALUE;
    }
    else
    {
        const uint16_t outputAddr  = Get_16b( msgBuffer, outputAddr_BuffPos );
        const uint16_t outputValue = Get_16b( msgBuffer, outputValue_BuffPos );

        if( ( coilOnValue != outputValue ) && ( coilOffValue != outputValue ) )
        {
            errorStatus = MODBUS_ERROR_ILLEGAL_DATA_VALUE;
        }
        else
        {
            errorStatus = DataAccess::Check_BitsRange( coilsHandler, supportedCoilsCount, outputAddr, 1u, MODBUS_DATA_ACCESS_WRITE );
        }

        if( MODBUS_ERROR_NO_ACTIVE_ERROR == errorStatus )
        {
            const modBus_CoilsRetValue_t coilValue = ( coilOnValue == outputValue ) ? MODBUS_COIL_ACTIVE : MODBUS_COIL_INACTIVE;

            errorStatus = DataAccess::Set_Bit( coilsHandler, outputAddr, coilValue );

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
