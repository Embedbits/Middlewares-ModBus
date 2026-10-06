/**
 * \author Mr.Nobody
 * \file ModBus_App_DataAccess.cpp
 * \ingroup ModBus
 * \brief ModBus application layer - common access to registers configuration.
 *
 */
/* ============================= INCLUDES =================================== */
#include "ModBus_App_DataAccess.hpp"    /* Self include                       */
#include "ModBus_App_Types.hpp"         /* Module types definition            */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ============================== MACROS ==================================== */

/* ========================= EXPORTED VARIABLES ============================= */

/* ========================== LOCAL VARIABLES =============================== */

/* ======================== EXPORTED FUNCTIONS ============================== */

/**
 * \brief Checks if all registers in range are accessible
 *
 * \param config      [in]: Configuration array (indexed by address)
 * \param configCount [in]: Count of items in configuration array
 * \param startAddr   [in]: First address of range
 * \param quantity    [in]: Count of registers in range
 * \param access      [in]: Required access type
 *
 * \return \ref MODBUS_ERROR_NO_ACTIVE_ERROR or \ref MODBUS_ERROR_ILLEGAL_DATA_ADDRESS
 */
modBus_ErrorStatus_t ModBus::AppLayer::DataAccess::Check_RegistersRange( const modBus_RegistersConfigStruct_t *config,
                                                                         modBus_RegistersCount_t configCount,
                                                                         uint16_t startAddr,
                                                                         uint16_t quantity,
                                                                         modBus_DataAccess_t access )
{
    modBus_ErrorStatus_t errorStatus = MODBUS_ERROR_NO_ACTIVE_ERROR;

    if( ( nullptr == config                                                     ) ||
        ( (uint32_t)configCount < ( (uint32_t)startAddr + (uint32_t)quantity ) )    )
    {
        errorStatus = MODBUS_ERROR_ILLEGAL_DATA_ADDRESS;
    }
    else
    {
        for( uint32_t regAddr = startAddr; ( (uint32_t)startAddr + quantity ) > regAddr; regAddr++ )
        {
            modBus_RegistersCallback_t *callback = ( MODBUS_DATA_ACCESS_READ == access ) ?
                                                   config[ regAddr ].ApplGetter :
                                                   config[ regAddr ].ApplSetter;

            if( ( MODBUS_SERVICE_SUPPORTED != config[ regAddr ].ServiceStatus ) ||
                ( nullptr                  == callback                        )    )
            {
                errorStatus = MODBUS_ERROR_ILLEGAL_DATA_ADDRESS;

                break;
            }
            else
            {
                /* Register is accessible */
            }
        }
    }

    return ( errorStatus );
}


/**
 * \brief Reads value of one register. Range shall be checked before.
 *
 * \param config  [in]: Configuration array (indexed by address)
 * \param regAddr [in]: Address of register
 * \param value  [out]: Value of register
 *
 * \return \ref MODBUS_ERROR_NO_ACTIVE_ERROR or \ref MODBUS_ERROR_SLAVE_DEVICE_FAILURE
 */
modBus_ErrorStatus_t ModBus::AppLayer::DataAccess::Get_Register( const modBus_RegistersConfigStruct_t *config,
                                                                 uint16_t regAddr,
                                                                 modBus_RegistersValue_t &value )
{
    modBus_ErrorStatus_t errorStatus = MODBUS_ERROR_SLAVE_DEVICE_FAILURE;

    value = 0u;

    if( MODBUS_REQUEST_OK == config[ regAddr ].ApplGetter( &value ) )
    {
        errorStatus = MODBUS_ERROR_NO_ACTIVE_ERROR;
    }
    else
    {
        /* Application failed to provide the value */
    }

    return ( errorStatus );
}


/**
 * \brief Writes value of one register. Range shall be checked before.
 *
 * \param config  [in]: Configuration array (indexed by address)
 * \param regAddr [in]: Address of register
 * \param value   [in]: Requested value of register
 *
 * \return \ref MODBUS_ERROR_NO_ACTIVE_ERROR or \ref MODBUS_ERROR_SLAVE_DEVICE_FAILURE
 */
modBus_ErrorStatus_t ModBus::AppLayer::DataAccess::Set_Register( const modBus_RegistersConfigStruct_t *config,
                                                                 uint16_t regAddr,
                                                                 modBus_RegistersValue_t value )
{
    modBus_ErrorStatus_t errorStatus = MODBUS_ERROR_SLAVE_DEVICE_FAILURE;

    if( MODBUS_REQUEST_OK == config[ regAddr ].ApplSetter( &value ) )
    {
        errorStatus = MODBUS_ERROR_NO_ACTIVE_ERROR;
    }
    else
    {
        /* Application failed to store the value */
    }

    return ( errorStatus );
}


/**
 * \brief Processes read registers request (Read Holding / Input Registers)
 *
 * Request : | Addr | FC | Start Hi | Start Lo | Quantity Hi | Quantity Lo |
 * Response: | Addr | FC | Byte count (2 x N) | Register values (N x 2 bytes) |
 *
 * \param msgBuffer   [in/out]: Request, overwritten by response
 * \param msgLength   [in/out]: Length of request / response without CRC
 * \param config          [in]: Configuration array (indexed by address)
 * \param configCount     [in]: Count of items in configuration array
 *
 * \return Exception code or \ref MODBUS_ERROR_NO_ACTIVE_ERROR
 */
modBus_ErrorStatus_t ModBus::AppLayer::DataAccess::ReadRegistersRequest( modBus_Message_t * const msgBuffer,
                                                                         modBus_MsgLength_t &msgLength,
                                                                         const modBus_RegistersConfigStruct_t *config,
                                                                         modBus_RegistersCount_t configCount )
{
    /** Expected request length (address, FC, start address, quantity) */
    constexpr modBus_MsgLength_t requestLen   = 6u;
    /** Maximal count of registers in one request */
    constexpr uint16_t           maxQuantity  = 0x007Du;
    constexpr modBus_MsgLength_t startAddrPos = 2u;
    constexpr modBus_MsgLength_t quantityPos  = 4u;
    constexpr modBus_MsgLength_t byteCountPos = 2u;
    constexpr modBus_MsgLength_t dataStartPos = 3u;

    modBus_ErrorStatus_t errorStatus = MODBUS_ERROR_NO_ACTIVE_ERROR;

    if( ( nullptr == msgBuffer ) || ( requestLen != msgLength ) )
    {
        errorStatus = MODBUS_ERROR_ILLEGAL_DATA_VALUE;
    }
    else
    {
        const uint16_t startAddr = ModBus_Get_16bFromTwo8b( msgBuffer[ startAddrPos ], msgBuffer[ startAddrPos + 1u ] );
        const uint16_t quantity  = ModBus_Get_16bFromTwo8b( msgBuffer[ quantityPos  ], msgBuffer[ quantityPos  + 1u ] );

        if( ( 0u == quantity ) || ( maxQuantity < quantity ) )
        {
            errorStatus = MODBUS_ERROR_ILLEGAL_DATA_VALUE;
        }
        else
        {
            errorStatus = Check_RegistersRange( config, configCount, startAddr, quantity, MODBUS_DATA_ACCESS_READ );
        }

        for( uint16_t regIndex = 0u; ( MODBUS_ERROR_NO_ACTIVE_ERROR == errorStatus ) && ( quantity > regIndex ); regIndex++ )
        {
            modBus_RegistersValue_t regValue = 0u;

            errorStatus = Get_Register( config, (uint16_t)( startAddr + regIndex ), regValue );

            msgBuffer[ dataStartPos + ( 2u * regIndex )      ] = ModBus_Get_Upper8bFrom16b( regValue );
            msgBuffer[ dataStartPos + ( 2u * regIndex ) + 1u ] = ModBus_Get_Lower8bFrom16b( regValue );
        }

        if( MODBUS_ERROR_NO_ACTIVE_ERROR == errorStatus )
        {
            msgBuffer[ byteCountPos ] = (uint8_t)( 2u * quantity );

            msgLength = dataStartPos + ( 2u * quantity );
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
