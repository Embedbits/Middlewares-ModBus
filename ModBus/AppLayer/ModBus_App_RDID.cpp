/**
 * \author Mr.Nobody
 * \file ModBus_App_RDID.cpp
 * \ingroup ModBus
 * \brief ModBus Read Device Identification (RDID, 0x2B / MEI 0x0E) application
 *        layer handler
 *
 * # (0x2B / 0x0E) Read Device Identification #
 *
 * This function code allows reading the identification and additional
 * information relative to the physical and functional description of a
 * remote device. Objects are divided into three categories - basic (0x00 -
 * 0x02, mandatory), regular (0x03 - 0x7F) and extended (0x80 - 0xFF).
 *
 * Stream access (Read Device ID code 1, 2, 3) returns all objects of the
 * category (and lower categories) starting from requested Object Id. If the
 * objects do not fit into one response, More Follows = 0xFF and Next Object Id
 * is set - master continues with next request starting at that object. If
 * requested Object Id does not exist, transfer restarts at object 0.
 * Individual access (code 4) returns one object, unknown object results in
 * exception 0x02.
 *
 * | Request               | Size    | Value            |
 * |-----------------------|---------|------------------|
 * | Function code         | 1 Byte  | 0x2B             |
 * | MEI Type              | 1 Byte  | 0x0E             |
 * | Read Device ID code   | 1 Byte  | 01 / 02 / 03 / 04|
 * | Object Id             | 1 Byte  | 0x00 to 0xFF     |
 *
 * | Response              | Size    | Value            |
 * |-----------------------|---------|------------------|
 * | Function code         | 1 Byte  | 0x2B             |
 * | MEI Type              | 1 Byte  | 0x0E             |
 * | Read Device ID code   | 1 Byte  | 01 / 02 / 03 / 04|
 * | Conformity level      | 1 Byte  | 0x01-0x03, 0x81-0x83 |
 * | More Follows          | 1 Byte  | 0x00 / 0xFF      |
 * | Next Object Id        | 1 Byte  | Object ID number |
 * | Number of objects     | 1 Byte  |                  |
 * | List of: Object ID, Object length, Object Value | |  |
 *
 * Exceptions: 0x01 (MEI type), 0x02 (individual access to unknown object),
 * 0x03 (Read Device ID code, request length), 0x04 (callback failed, object
 * does not fit into response).
 *
 * Example: request `2B 0E 01 00`, response
 * `2B 0E 01 01 00 00 03 00 16 "Company identification" 01 0D "Product code XX" 02 05 "V2.11"`.
 */
/* ============================= INCLUDES =================================== */
#include "ModBus_App_RDID.hpp"          /* Self include                       */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ============================== MACROS ==================================== */

/* ========================= EXPORTED VARIABLES ============================= */

/* ========================== LOCAL VARIABLES =============================== */

/* ======================== EXPORTED FUNCTIONS ============================== */

/**
 * \brief Read Device Identification handler constructor
 *
 * \param functionConfigStruct [in]: Objects configuration array. Mandatory
 *                                   basic objects 0x00 - 0x02 shall be present.
 * \param objCount             [in]: Count of objects in configuration array
 */
ModBus::AppLayer::RDID::RDID( const modBus_App_RDID_t *functionConfigStruct, modBus_RDID_ObjCnt_t objCount ) :
    functionConfig( functionConfigStruct ),
    objCount( objCount )
{
    if( nullptr == functionConfig )
    {
        this->objCount = 0u;
    }
    else
    {
        /* Configuration assigned */
    }
}


/**
 * \brief Returns conformity level derived from configured objects
 *
 * Both stream and individual access are supported. Category is given by the
 * highest configured object.
 *
 * \return Conformity level
 */
modBus_App_RDID_ConformityLevel_t ModBus::AppLayer::RDID::Get_ConformityLevel( void ) const
{
    modBus_App_RDID_ConformityLevel_t conformityLevel = MODBUS_APP_RDID_CONFORMITY_BASIC_BOTH;

    for( modBus_RDID_ObjCnt_t objIndex = 0u; objCount > objIndex; objIndex++ )
    {
        if( regularObjIdMax < functionConfig[ objIndex ].ObjId )
        {
            conformityLevel = MODBUS_APP_RDID_CONFORMITY_EXTENDED_BOTH;
        }
        else if( ( basicObjIdMax                         <  functionConfig[ objIndex ].ObjId ) &&
                 ( MODBUS_APP_RDID_CONFORMITY_BASIC_BOTH == conformityLevel                  )    )
        {
            conformityLevel = MODBUS_APP_RDID_CONFORMITY_REGULAR_BOTH;
        }
        else
        {
            /* Level is not increased */
        }
    }

    return ( conformityLevel );
}


/**
 * \brief Processes Read Device Identification (RDID) request
 *
 * \param msgBuffer [in,out]: Request, overwritten by response
 * \param msgLength [in,out]: Length of request / response without CRC
 *
 * \return Exception code or \ref MODBUS_ERROR_NO_ACTIVE_ERROR
 */
modBus_ErrorStatus_t ModBus::AppLayer::RDID::SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength )
{
    modBus_ErrorStatus_t errorStatus = MODBUS_ERROR_NO_ACTIVE_ERROR;

    if( ( nullptr == msgBuffer ) || ( requestLen != msgLength ) )
    {
        errorStatus = MODBUS_ERROR_ILLEGAL_DATA_VALUE;
    }
    else if( meiTypeReadDeviceId != msgBuffer[ meiType_BuffPos ] )
    {
        /* Other MEI types are not supported */
        errorStatus = MODBUS_ERROR_ILLEGAL_FUNCTION;
    }
    else
    {
        const uint8_t             readDevIdCode  = msgBuffer[ readDeviceIdCode_BuffPos ];
        const modBus_RDID_ObjId_t requestedObjId = msgBuffer[ objectId_BuffPos ];
        modBus_MsgLength_t        buffPos        = firstObject_BuffPos;
        uint8_t                   objectsCnt     = 0u;
        bool                      objectFits     = true;

        msgBuffer[ conformityLevel_BuffPos ] = (uint8_t)Get_ConformityLevel();
        msgBuffer[ moreFollows_BuffPos ]     = 0u;
        msgBuffer[ nextObjectId_BuffPos ]    = 0u;

        if( MODBUS_APP_RDID_SPECIFIC_DEVICE_ID == readDevIdCode )
        {
            /* Individual access */
            const modBus_App_RDID_t *object = FindObject( requestedObjId );

            if( nullptr == object )
            {
                errorStatus = MODBUS_ERROR_ILLEGAL_DATA_ADDRESS;
            }
            else
            {
                errorStatus = AppendObject( msgBuffer, buffPos, *object, objectFits );

                if( ( MODBUS_ERROR_NO_ACTIVE_ERROR == errorStatus ) && ( false == objectFits ) )
                {
                    errorStatus = MODBUS_ERROR_SLAVE_DEVICE_FAILURE;
                }
                else
                {
                    objectsCnt = 1u;
                }
            }
        }
        else if( ( MODBUS_APP_RDID_BASIC_DEVICE_ID    <= readDevIdCode ) &&
                 ( MODBUS_APP_RDID_EXTENDED_DEVICE_ID >= readDevIdCode )    )
        {
            /* Stream access */
            const modBus_RDID_ObjId_t lastObjId = ( MODBUS_APP_RDID_BASIC_DEVICE_ID   == readDevIdCode ) ? basicObjIdMax   :
                                                  ( MODBUS_APP_RDID_REGULAR_DEVICE_ID == readDevIdCode ) ? regularObjIdMax :
                                                                                                           extendedObjIdMax;
            uint16_t startObjId = requestedObjId;

            if( ( lastObjId < requestedObjId ) || ( nullptr == FindObject( requestedObjId ) ) )
            {
                /* Unknown object, restart at the beginning */
                startObjId = MODBUS_RDID_ID_BASIC_VENDOR_NAME;
            }
            else
            {
                /* Continue with requested object */
            }

            for( uint16_t objId = startObjId; ( lastObjId >= objId ) && ( MODBUS_ERROR_NO_ACTIVE_ERROR == errorStatus ); objId++ )
            {
                const modBus_App_RDID_t *object = FindObject( objId );

                if( nullptr != object )
                {
                    errorStatus = AppendObject( msgBuffer, buffPos, *object, objectFits );

                    if( MODBUS_ERROR_NO_ACTIVE_ERROR != errorStatus )
                    {
                        /* Error is returned */
                    }
                    else if( true == objectFits )
                    {
                        objectsCnt++;
                    }
                    else if( 0u == objectsCnt )
                    {
                        /* Object is longer than whole response */
                        errorStatus = MODBUS_ERROR_SLAVE_DEVICE_FAILURE;
                    }
                    else
                    {
                        /* Object is transferred by next request */
                        msgBuffer[ moreFollows_BuffPos ]  = moreFollowsActive;
                        msgBuffer[ nextObjectId_BuffPos ] = (uint8_t)objId;

                        break;
                    }
                }
                else
                {
                    /* Object is not configured */
                }
            }
        }
        else
        {
            errorStatus = MODBUS_ERROR_ILLEGAL_DATA_VALUE;
        }

        if( MODBUS_ERROR_NO_ACTIVE_ERROR == errorStatus )
        {
            msgBuffer[ objectsCount_BuffPos ] = objectsCnt;

            msgLength = buffPos;
        }
        else
        {
            /* Error is returned */
        }
    }

    return ( errorStatus );
}

/* ========================== LOCAL FUNCTIONS =============================== */

/**
 * \brief Finds object configuration
 *
 * \param objId [in]: Object identification
 *
 * \return Object configuration or null pointer if object is not configured
 */
const modBus_App_RDID_t *ModBus::AppLayer::RDID::FindObject( uint16_t objId ) const
{
    const modBus_App_RDID_t *object = nullptr;

    for( modBus_RDID_ObjCnt_t objIndex = 0u; objCount > objIndex; objIndex++ )
    {
        if( ( objId   == functionConfig[ objIndex ].ObjId       ) &&
            ( nullptr != functionConfig[ objIndex ].ObjCallback )    )
        {
            object = &functionConfig[ objIndex ];

            break;
        }
        else
        {
            /* Continue with search */
        }
    }

    return ( object );
}


/**
 * \brief Appends object (ID, length, value) into response
 *
 * \param msgBuffer [in,out]: Response buffer
 * \param buffPos   [in,out]: Position of object in buffer, moved behind object
 * \param object        [in]: Object configuration
 * \param objectFits   [out]: False if object does not fit into response (not appended)
 *
 * \return \ref MODBUS_ERROR_NO_ACTIVE_ERROR or \ref MODBUS_ERROR_SLAVE_DEVICE_FAILURE
 */
modBus_ErrorStatus_t ModBus::AppLayer::RDID::AppendObject( modBus_Message_t * const msgBuffer,
                                                           modBus_MsgLength_t &buffPos,
                                                           const modBus_App_RDID_t &object,
                                                           bool &objectFits ) const
{
    modBus_ErrorStatus_t errorStatus = MODBUS_ERROR_NO_ACTIVE_ERROR;
    const uint8_t       *objData     = nullptr;
    uint8_t              objLength   = 0u;

    objectFits = false;

    if( ( MODBUS_REQUEST_OK != object.ObjCallback( &objData, &objLength ) ) ||
        ( ( nullptr == objData ) && ( 0u != objLength ) )                      )
    {
        errorStatus = MODBUS_ERROR_SLAVE_DEVICE_FAILURE;
    }
    else if( ( buffPos + 2u + objLength ) <= messageMaxLen )
    {
        msgBuffer[ buffPos      ] = object.ObjId;
        msgBuffer[ buffPos + 1u ] = objLength;

        for( uint8_t dataIndex = 0u; objLength > dataIndex; dataIndex++ )
        {
            msgBuffer[ buffPos + 2u + dataIndex ] = objData[ dataIndex ];
        }

        buffPos = buffPos + 2u + objLength;

        objectFits = true;
    }
    else
    {
        /* Object does not fit into response */
    }

    return ( errorStatus );
}

/* =============================== TASKS ==================================== */
