/**
 * \author Mr.Nobody
 * \file ModBus_App_RES.cpp
 * \ingroup ModBus
 * \brief ModBus Read Exception Status (RES, 0x07) application layer handler
 *
 * # (0x07) Read Exception Status (Serial Line only) #
 *
 * This function code is used to read the contents of eight Exception Status
 * outputs in a remote device. The outputs are packed into one data byte, with
 * one bit per output. The status of the lowest output reference is contained
 * in the least significant bit of the byte. The contents of the eight
 * Exception Status outputs are device specific (provided by callback).
 *
 * | Request               | Size    | Value            |
 * |-----------------------|---------|------------------|
 * | Function code         | 1 Byte  | 0x07             |
 *
 * | Response              | Size    | Value            |
 * |-----------------------|---------|------------------|
 * | Function code         | 1 Byte  | 0x07             |
 * | Output Data           | 1 Byte  | 0x00 to 0xFF     |
 *
 * Exceptions: 0x03 (request length), 0x04 (callback failed or not assigned).
 *
 * Example: request `07`, response `07 6D`.
 */
/* ============================= INCLUDES =================================== */
#include "ModBus_App_RES.hpp"           /* Self include                       */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ============================== MACROS ==================================== */

/* ========================= EXPORTED VARIABLES ============================= */

/* ========================== LOCAL VARIABLES =============================== */

/* ======================== EXPORTED FUNCTIONS ============================== */

/**
 * \brief Read Exception Status handler constructor
 *
 * \param statusCallback [in]: Callback providing exception status outputs
 */
ModBus::AppLayer::RES::RES( modBus_ExceptionStatusCallback_t *statusCallback ) :
    statusCallback( statusCallback )
{
    return;
}


/**
 * \brief Processes Read Exception Status (RES) request
 *
 * \param msgBuffer [in,out]: Request, overwritten by response
 * \param msgLength [in,out]: Length of request / response without CRC
 *
 * \return Exception code or \ref MODBUS_ERROR_NO_ACTIVE_ERROR
 */
modBus_ErrorStatus_t ModBus::AppLayer::RES::SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength )
{
    modBus_ErrorStatus_t errorStatus     = MODBUS_ERROR_NO_ACTIVE_ERROR;
    uint8_t              exceptionStatus = 0u;

    if( ( nullptr == msgBuffer ) || ( requestLen != msgLength ) )
    {
        errorStatus = MODBUS_ERROR_ILLEGAL_DATA_VALUE;
    }
    else if( ( nullptr           == statusCallback                     ) ||
             ( MODBUS_REQUEST_OK != statusCallback( &exceptionStatus ) )    )
    {
        errorStatus = MODBUS_ERROR_SLAVE_DEVICE_FAILURE;
    }
    else
    {
        msgBuffer[ outputData_BuffPos ] = exceptionStatus;

        msgLength = responseLen;
    }

    return ( errorStatus );
}

/* ========================== LOCAL FUNCTIONS =============================== */

/* =============================== TASKS ==================================== */
