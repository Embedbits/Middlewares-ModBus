/**
 * \author Mr.Nobody
 * \file ModBus_App_WFR.cpp
 * \ingroup ModBus
 * \brief ModBus Write File Record (WFR, 0x15) application layer handler
 *
 * # (0x15) Write File Record #
 *
 * This function code is used to perform a file record write. Record lengths
 * are provided in registers, data lengths in bytes. The function can write
 * multiple groups of references, each sub-request contains 7 bytes header and
 * the record data. The normal response is an echo of the request.
 *
 * | Request / Response          | Size    | Value            |
 * |-----------------------------|---------|------------------|
 * | Function code               | 1 Byte  | 0x15             |
 * | Request data length         | 1 Byte  | 0x09 to 0xFB     |
 * | Sub-Req. x, Reference Type  | 1 Byte  | 6                |
 * | Sub-Req. x, File Number     | 2 Bytes | 0x0001 to 0xFFFF |
 * | Sub-Req. x, Record Number   | 2 Bytes | 0x0000 to 0x270F |
 * | Sub-Req. x, Record length   | 2 Bytes | N                |
 * | Sub-Req. x, Record data     | N x 2 B |                  |
 * | Sub-Req. x+1, ...           |         |                  |
 *
 * Exceptions: 0x02 (reference type, file / record range, callback failed),
 * 0x03 (data length, request structure).
 *
 * Example - write 3 records into file 4 starting at record 7: request
 * `15 0D 06 00 04 00 07 00 03 06 AF 04 BE 10 0D`, response is the same.
 */
/* ============================= INCLUDES =================================== */
#include "ModBus_App_WFR.hpp"           /* Self include                       */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ============================== MACROS ==================================== */

/* ========================= EXPORTED VARIABLES ============================= */

/* ========================== LOCAL VARIABLES =============================== */

/* ======================== EXPORTED FUNCTIONS ============================== */

/**
 * \brief Write File Record handler constructor
 *
 * \param writeCallback [in]: Callback writing file records
 */
ModBus::AppLayer::WFR::WFR( modBus_FileRecordCallback_t *writeCallback ) :
    writeCallback( writeCallback )
{
    return;
}


/**
 * \brief Processes Write File Record (WFR) request
 *
 * Whole request is checked before the first record is written.
 *
 * \param msgBuffer [in,out]: Request, response is echo of the request
 * \param msgLength [in,out]: Length of request / response without CRC
 *
 * \return Exception code or \ref MODBUS_ERROR_NO_ACTIVE_ERROR
 */
modBus_ErrorStatus_t ModBus::AppLayer::WFR::SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength )
{
    modBus_ErrorStatus_t errorStatus = CheckRequest( msgBuffer, msgLength );

    if( ( MODBUS_ERROR_NO_ACTIVE_ERROR == errorStatus   ) &&
        ( nullptr                      == writeCallback )    )
    {
        errorStatus = MODBUS_ERROR_ILLEGAL_DATA_ADDRESS;
    }
    else
    {
        /* Continue with processing */
    }

    if( MODBUS_ERROR_NO_ACTIVE_ERROR == errorStatus )
    {
        modBus_MsgLength_t      subPos = subRequests_BuffPos;
        modBus_RegistersValue_t recordValues[ recordLengthMax ];

        while( ( MODBUS_ERROR_NO_ACTIVE_ERROR == errorStatus ) && ( msgLength > subPos ) )
        {
            const uint16_t fileNumber   = Get_16b( msgBuffer, subPos + 1u );
            const uint16_t recordNumber = Get_16b( msgBuffer, subPos + 3u );
            const uint16_t recordLength = Get_16b( msgBuffer, subPos + 5u );

            for( uint16_t recordIndex = 0u; recordLength > recordIndex; recordIndex++ )
            {
                recordValues[ recordIndex ] = Get_16b( msgBuffer, subPos + subRequestHeaderLen + ( 2u * recordIndex ) );
            }

            if( MODBUS_REQUEST_OK != writeCallback( fileNumber, recordNumber, recordValues, recordLength ) )
            {
                errorStatus = MODBUS_ERROR_ILLEGAL_DATA_ADDRESS;
            }
            else
            {
                subPos = subPos + subRequestHeaderLen + ( 2u * recordLength );
            }
        }

        /* Response is echo of request, msgLength is not changed */
    }
    else
    {
        /* Error is returned */
    }

    return ( errorStatus );
}

/* ========================== LOCAL FUNCTIONS =============================== */

/**
 * \brief Checks structure and addresses of all sub-requests
 *
 * \param msgBuffer [in]: Request
 * \param msgLength [in]: Length of request without CRC
 *
 * \return Exception code or \ref MODBUS_ERROR_NO_ACTIVE_ERROR
 */
modBus_ErrorStatus_t ModBus::AppLayer::WFR::CheckRequest( const modBus_Message_t * const msgBuffer, modBus_MsgLength_t msgLength ) const
{
    modBus_ErrorStatus_t errorStatus = MODBUS_ERROR_NO_ACTIVE_ERROR;

    if( ( nullptr == msgBuffer ) || ( subRequests_BuffPos > msgLength ) )
    {
        errorStatus = MODBUS_ERROR_ILLEGAL_DATA_VALUE;
    }
    else
    {
        const uint8_t dataLength = msgBuffer[ dataLength_BuffPos ];

        if( ( dataLengthMin                        >  dataLength ) ||
            ( dataLengthMax                        <  dataLength ) ||
            ( ( subRequests_BuffPos + dataLength ) != msgLength  )    )
        {
            errorStatus = MODBUS_ERROR_ILLEGAL_DATA_VALUE;
        }
        else
        {
            modBus_MsgLength_t subPos = subRequests_BuffPos;

            while( ( MODBUS_ERROR_NO_ACTIVE_ERROR == errorStatus ) && ( msgLength > subPos ) )
            {
                if( ( subPos + subRequestHeaderLen ) > msgLength )
                {
                    /* Incomplete sub-request header */
                    errorStatus = MODBUS_ERROR_ILLEGAL_DATA_VALUE;
                }
                else
                {
                    const uint16_t fileNumber   = Get_16b( msgBuffer, subPos + 1u );
                    const uint16_t recordNumber = Get_16b( msgBuffer, subPos + 3u );
                    const uint16_t recordLength = Get_16b( msgBuffer, subPos + 5u );
                    const uint32_t subEndPos    = (uint32_t)subPos + subRequestHeaderLen + ( 2u * (uint32_t)recordLength );

                    if( subEndPos > msgLength )
                    {
                        /* Record data do not correspond with data length */
                        errorStatus = MODBUS_ERROR_ILLEGAL_DATA_VALUE;
                    }
                    else if( ( MODBUS_FILE_RECORD_REFERENCE_TYPE != msgBuffer[ subPos ] ) ||
                             ( 0u                                == fileNumber          ) ||
                             ( 0u                                == recordLength        ) ||
                             ( MODBUS_FILE_RECORD_NUMBER_MAX     <  ( (uint32_t)recordNumber + recordLength - 1u ) ) )
                    {
                        errorStatus = MODBUS_ERROR_ILLEGAL_DATA_ADDRESS;
                    }
                    else
                    {
                        subPos = (modBus_MsgLength_t)subEndPos;
                    }
                }
            }
        }
    }

    return ( errorStatus );
}

/* =============================== TASKS ==================================== */
