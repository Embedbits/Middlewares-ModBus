/**
 * \author Mr.Nobody
 * \file ModBus_App_RFR.cpp
 * \ingroup ModBus
 * \brief ModBus Read File Record (RFR, 0x14) application layer handler
 *
 * # (0x14) Read File Record #
 *
 * This function code is used to perform a file record read. A file is an
 * organization of records, each file contains 10000 records addressed 0 to
 * 9999. Record lengths are provided in registers, data lengths in bytes. The
 * function can read multiple groups of references, each group is defined in
 * a separate 7 bytes sub-request. The whole response shall fit into 253 bytes
 * PDU.
 *
 * | Request                    | Size    | Value            |
 * |----------------------------|---------|------------------|
 * | Function code              | 1 Byte  | 0x14             |
 * | Byte Count                 | 1 Byte  | 0x07 to 0xF5     |
 * | Sub-Req. x, Reference Type | 1 Byte  | 6                |
 * | Sub-Req. x, File Number    | 2 Bytes | 0x0001 to 0xFFFF |
 * | Sub-Req. x, Record Number  | 2 Bytes | 0x0000 to 0x270F |
 * | Sub-Req. x, Record Length  | 2 Bytes | N                |
 * | Sub-Req. x+1, ...          |         |                  |
 *
 * | Response                   | Size    | Value            |
 * |----------------------------|---------|------------------|
 * | Function code              | 1 Byte  | 0x14             |
 * | Resp. data Length          | 1 Byte  | 0x07 to 0xF5     |
 * | Sub-Resp. x, File Resp. len| 1 Byte  | 1 + 2 x N        |
 * | Sub-Resp. x, Ref. Type     | 1 Byte  | 6                |
 * | Sub-Resp. x, Record Data   | N x 2 B |                  |
 * | Sub-Resp. x+1, ...         |         |                  |
 *
 * Exceptions: 0x02 (reference type, file / record range, callback failed),
 * 0x03 (byte count, request structure, response too long).
 *
 * Example - read 2 records from file 4 (record 1) and file 3 (record 9):
 * request `14 0E 06 00 04 00 01 00 02 06 00 03 00 09 00 02`,
 * response `14 0C 05 06 0D FE 00 20 05 06 33 CD 00 40`.
 */
/* ============================= INCLUDES =================================== */
#include "ModBus_App_RFR.hpp"           /* Self include                       */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ============================== MACROS ==================================== */

/* ========================= EXPORTED VARIABLES ============================= */

/* ========================== LOCAL VARIABLES =============================== */

/* ======================== EXPORTED FUNCTIONS ============================== */

/**
 * \brief Read File Record handler constructor
 *
 * \param readCallback [in]: Callback reading file records
 */
ModBus::AppLayer::RFR::RFR( modBus_FileRecordCallback_t *readCallback ) :
    readCallback( readCallback )
{
    return;
}


/**
 * \brief Processes Read File Record (RFR) request
 *
 * All sub-requests are parsed and checked before the response is written into
 * the buffer.
 *
 * \param msgBuffer [in,out]: Request, overwritten by response
 * \param msgLength [in,out]: Length of request / response without CRC
 *
 * \return Exception code or \ref MODBUS_ERROR_NO_ACTIVE_ERROR
 */
modBus_ErrorStatus_t ModBus::AppLayer::RFR::SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength )
{
    modBus_ErrorStatus_t errorStatus = MODBUS_ERROR_NO_ACTIVE_ERROR;
    subRequest_t         subRequests[ subRequestsMax ];
    uint8_t              subRequestsCnt = 0u;

    if( ( nullptr == msgBuffer ) || ( subRequests_BuffPos > msgLength ) )
    {
        errorStatus = MODBUS_ERROR_ILLEGAL_DATA_VALUE;
    }
    else
    {
        const uint8_t byteCount = msgBuffer[ byteCount_BuffPos ];

        if( ( byteCountMin                          >  byteCount ) ||
            ( byteCountMax                          <  byteCount ) ||
            ( 0u                                    != ( byteCount % subRequestLen ) ) ||
            ( ( subRequests_BuffPos + byteCount )   != msgLength )    )
        {
            errorStatus = MODBUS_ERROR_ILLEGAL_DATA_VALUE;
        }
        else
        {
            /* Response contains function code and response data length */
            uint32_t responsePduLen = 2u;

            subRequestsCnt = byteCount / subRequestLen;

            for( uint8_t subIndex = 0u; subRequestsCnt > subIndex; subIndex++ )
            {
                const modBus_MsgLength_t subPos = subRequests_BuffPos + ( subIndex * subRequestLen );

                subRequests[ subIndex ].FileNumber   = Get_16b( msgBuffer, subPos + 1u );
                subRequests[ subIndex ].RecordNumber = Get_16b( msgBuffer, subPos + 3u );
                subRequests[ subIndex ].RecordLength = Get_16b( msgBuffer, subPos + 5u );

                /* Sub-response contains length, reference type and data */
                responsePduLen = responsePduLen + 2u + ( 2u * subRequests[ subIndex ].RecordLength );

                if( ( MODBUS_FILE_RECORD_REFERENCE_TYPE != msgBuffer[ subPos ]                        ) ||
                    ( 0u                                == subRequests[ subIndex ].FileNumber         ) ||
                    ( 0u                                == subRequests[ subIndex ].RecordLength       ) ||
                    ( MODBUS_FILE_RECORD_NUMBER_MAX     <  ( (uint32_t)subRequests[ subIndex ].RecordNumber +
                                                             subRequests[ subIndex ].RecordLength - 1u ) )    )
                {
                    errorStatus = MODBUS_ERROR_ILLEGAL_DATA_ADDRESS;

                    break;
                }
                else if( pduMaxLen < responsePduLen )
                {
                    errorStatus = MODBUS_ERROR_ILLEGAL_DATA_VALUE;

                    break;
                }
                else
                {
                    /* Sub-request is valid */
                }
            }
        }
    }

    if( ( MODBUS_ERROR_NO_ACTIVE_ERROR == errorStatus ) &&
        ( nullptr                      == readCallback )    )
    {
        errorStatus = MODBUS_ERROR_ILLEGAL_DATA_ADDRESS;
    }
    else
    {
        /* Continue with processing */
    }

    if( MODBUS_ERROR_NO_ACTIVE_ERROR == errorStatus )
    {
        modBus_MsgLength_t      buffPos = subRequests_BuffPos;
        modBus_RegistersValue_t recordValues[ pduMaxLen / 2u ];

        for( uint8_t subIndex = 0u; subRequestsCnt > subIndex; subIndex++ )
        {
            const uint16_t recordLength = subRequests[ subIndex ].RecordLength;

            if( MODBUS_REQUEST_OK != readCallback( subRequests[ subIndex ].FileNumber,
                                                   subRequests[ subIndex ].RecordNumber,
                                                   recordValues,
                                                   recordLength ) )
            {
                errorStatus = MODBUS_ERROR_ILLEGAL_DATA_ADDRESS;

                break;
            }
            else
            {
                msgBuffer[ buffPos      ] = (uint8_t)( 1u + ( 2u * recordLength ) );
                msgBuffer[ buffPos + 1u ] = MODBUS_FILE_RECORD_REFERENCE_TYPE;

                buffPos = buffPos + 2u;

                for( uint16_t recordIndex = 0u; recordLength > recordIndex; recordIndex++ )
                {
                    Set_16b( msgBuffer, buffPos, recordValues[ recordIndex ] );

                    buffPos = buffPos + 2u;
                }
            }
        }

        if( MODBUS_ERROR_NO_ACTIVE_ERROR == errorStatus )
        {
            msgBuffer[ byteCount_BuffPos ] = (uint8_t)( buffPos - subRequests_BuffPos );

            msgLength = buffPos;
        }
        else
        {
            /* Error is returned */
        }
    }
    else
    {
        /* Error is returned */
    }

    return ( errorStatus );
}

/* ========================== LOCAL FUNCTIONS =============================== */

/* =============================== TASKS ==================================== */
