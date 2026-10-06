/**
 * \author Mr.Nobody
 * \file ModBus_App_RSID.cpp
 * \ingroup ModBus
 * \brief ModBus Report Slave ID (RSID, 0x11) application layer handler
 *
 * # (0x11) Report Server ID (Serial Line only) #
 *
 * This function code is used to read the description of the type, the current
 * status, and other information specific to a remote device. The data
 * contents are device specific (see \ref modBus_App_RSID_t).
 *
 * | Request               | Size    | Value            |
 * |-----------------------|---------|------------------|
 * | Function code         | 1 Byte  | 0x11             |
 *
 * | Response              | Size    | Value            |
 * |-----------------------|---------|------------------|
 * | Function code         | 1 Byte  | 0x11             |
 * | Byte Count            | 1 Byte  | 2 + N            |
 * | Server ID             | 1 Byte  | device specific  |
 * | Run Indicator Status  | 1 Byte  | 0x00 = OFF, 0xFF = ON |
 * | Additional Data       | N Bytes | device specific  |
 *
 * Exceptions: 0x03 (request length), 0x04 (invalid configuration).
 */
/* ============================= INCLUDES =================================== */
#include "ModBus_App_RSID.hpp"          /* Self include                       */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ============================== MACROS ==================================== */

/* ========================= EXPORTED VARIABLES ============================= */

/* ========================== LOCAL VARIABLES =============================== */

/* ======================== EXPORTED FUNCTIONS ============================== */

/**
 * \brief Report Slave ID handler constructor
 *
 * \param slaveIdConfigStruct [in]: Function configuration (shall exist during
 *                                  whole life of handler)
 */
ModBus::AppLayer::RSID::RSID( const modBus_App_RSID_t &slaveIdConfigStruct ) :
    functionConfig( slaveIdConfigStruct )
{
    return;
}


/**
 * \brief Processes Report Slave ID (RSID) request
 *
 * \param msgBuffer [in,out]: Request, overwritten by response
 * \param msgLength [in,out]: Length of request / response without CRC
 *
 * \return Exception code or \ref MODBUS_ERROR_NO_ACTIVE_ERROR
 */
modBus_ErrorStatus_t ModBus::AppLayer::RSID::SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength )
{
    modBus_ErrorStatus_t errorStatus = MODBUS_ERROR_NO_ACTIVE_ERROR;

    if( ( nullptr == msgBuffer ) || ( requestLen != msgLength ) )
    {
        errorStatus = MODBUS_ERROR_ILLEGAL_DATA_VALUE;
    }
    else if( ( ( addData_BuffPos + functionConfig.AddData_Len ) > messageMaxLen ) ||
             ( ( 0u != functionConfig.AddData_Len ) && ( nullptr == functionConfig.AddData ) ) )
    {
        /* Invalid configuration */
        errorStatus = MODBUS_ERROR_SLAVE_DEVICE_FAILURE;
    }
    else
    {
        modBus_App_RunStatus_t runStatus = MODBUS_APP_RUN_INACTIVE;

        if( nullptr != functionConfig.RunStatusGetter )
        {
            runStatus = functionConfig.RunStatusGetter();
        }
        else
        {
            /* Run status is not provided */
        }

        msgBuffer[ byteCount_BuffPos ]          = (uint8_t)( functionConfig.AddData_Len + 2u );
        msgBuffer[ slaveId_BuffPos ]            = functionConfig.SlaveId;
        msgBuffer[ runIndicatorStatus_BuffPos ] = (uint8_t)runStatus;

        for( uint16_t addDataIndex = 0u; functionConfig.AddData_Len > addDataIndex; addDataIndex++ )
        {
            msgBuffer[ addData_BuffPos + addDataIndex ] = functionConfig.AddData[ addDataIndex ];
        }

        msgLength = addData_BuffPos + functionConfig.AddData_Len;
    }

    return ( errorStatus );
}

/* ========================== LOCAL FUNCTIONS =============================== */

/* =============================== TASKS ==================================== */
