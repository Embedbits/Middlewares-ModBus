/**
 * \author Mr.Nobody
 * \file ModBus_App_DataAccess.hpp
 * \ingroup ModBus
 * \brief ModBus application layer - common access to registers, coils and
 *        discrete inputs configuration (internal header of handlers).
 *
 * Rules applied for all data access functions:
 * - Configuration array is indexed by data address.
 * - Address outside of configuration array, not supported item or item without
 *   required callback (getter for read, setter for write) results in
 *   \ref MODBUS_ERROR_ILLEGAL_DATA_ADDRESS.
 * - Whole address range of request is checked before first callback is
 *   executed, thus request with invalid address has no side effects.
 * - Callback returning \ref MODBUS_REQUEST_ERROR results in
 *   \ref MODBUS_ERROR_SLAVE_DEVICE_FAILURE.
 */

#ifndef APPLAYER_MODBUS_APP_DATAACCESS_HPP
#define APPLAYER_MODBUS_APP_DATAACCESS_HPP

/* ============================= INCLUDES =================================== */
#include "ModBus_App_Types.hpp"         /* Module types definition            */
/* ============================= TYPEDEFS =================================== */

/** \brief Type of requested access to data item */
typedef enum
{
    MODBUS_DATA_ACCESS_READ = 0u, /**< Getter of item is required */
    MODBUS_DATA_ACCESS_WRITE      /**< Setter of item is required */
}   modBus_DataAccess_t;

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ========================= EXPORTED MACROS ================================ */

/* ========================= EXPORTED VARIABLES ============================= */

/* ======================== EXPORTED FUNCTIONS ============================== */

namespace ModBus
{
namespace AppLayer
{
namespace DataAccess
{
    /* ---------------------------- Registers ------------------------------- */

    modBus_ErrorStatus_t Check_RegistersRange( const modBus_RegistersConfigStruct_t *config,
                                               modBus_RegistersCount_t configCount,
                                               uint16_t startAddr,
                                               uint16_t quantity,
                                               modBus_DataAccess_t access );

    modBus_ErrorStatus_t Get_Register( const modBus_RegistersConfigStruct_t *config,
                                       uint16_t regAddr,
                                       modBus_RegistersValue_t &value );

    modBus_ErrorStatus_t Set_Register( const modBus_RegistersConfigStruct_t *config,
                                       uint16_t regAddr,
                                       modBus_RegistersValue_t value );

    modBus_ErrorStatus_t ReadRegistersRequest( modBus_Message_t * const msgBuffer,
                                               modBus_MsgLength_t &msgLength,
                                               const modBus_RegistersConfigStruct_t *config,
                                               modBus_RegistersCount_t configCount );

    /* ----------------------- Coils and discrete inputs -------------------- */

    /**
     * \brief Checks if all bits (coils / discrete inputs) in range are accessible
     *
     * \param config      [in]: Configuration array (indexed by address)
     * \param configCount [in]: Count of items in configuration array
     * \param startAddr   [in]: First address of range
     * \param quantity    [in]: Count of items in range
     * \param access      [in]: Required access type
     *
     * \return \ref MODBUS_ERROR_NO_ACTIVE_ERROR or \ref MODBUS_ERROR_ILLEGAL_DATA_ADDRESS
     */
    template< typename ConfigType >
    modBus_ErrorStatus_t Check_BitsRange( const ConfigType *config,
                                          uint16_t configCount,
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
            for( uint32_t itemAddr = startAddr; ( (uint32_t)startAddr + quantity ) > itemAddr; itemAddr++ )
            {
                modBus_CoilsCallback_t *callback = ( MODBUS_DATA_ACCESS_READ == access ) ?
                                                   config[ itemAddr ].ApplGetter :
                                                   config[ itemAddr ].ApplSetter;

                if( ( MODBUS_SERVICE_SUPPORTED != config[ itemAddr ].ServiceStatus ) ||
                    ( nullptr                  == callback                         )    )
                {
                    errorStatus = MODBUS_ERROR_ILLEGAL_DATA_ADDRESS;

                    break;
                }
                else
                {
                    /* Item is accessible */
                }
            }
        }

        return ( errorStatus );
    }


    /**
     * \brief Reads value of one bit item. Range shall be checked before.
     *
     * \param config  [in]: Configuration array (indexed by address)
     * \param bitAddr [in]: Address of item
     * \param value  [out]: Value of item
     *
     * \return \ref MODBUS_ERROR_NO_ACTIVE_ERROR or \ref MODBUS_ERROR_SLAVE_DEVICE_FAILURE
     */
    template< typename ConfigType >
    modBus_ErrorStatus_t Get_Bit( const ConfigType *config, uint16_t bitAddr, modBus_CoilsRetValue_t &value )
    {
        modBus_ErrorStatus_t errorStatus = MODBUS_ERROR_SLAVE_DEVICE_FAILURE;

        value = MODBUS_COIL_INACTIVE;

        if( MODBUS_REQUEST_OK == config[ bitAddr ].ApplGetter( &value ) )
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
     * \brief Writes value of one bit item. Range shall be checked before.
     *
     * \param config  [in]: Configuration array (indexed by address)
     * \param bitAddr [in]: Address of item
     * \param value   [in]: Requested value of item
     *
     * \return \ref MODBUS_ERROR_NO_ACTIVE_ERROR or \ref MODBUS_ERROR_SLAVE_DEVICE_FAILURE
     */
    template< typename ConfigType >
    modBus_ErrorStatus_t Set_Bit( const ConfigType *config, uint16_t bitAddr, modBus_CoilsRetValue_t value )
    {
        modBus_ErrorStatus_t errorStatus = MODBUS_ERROR_SLAVE_DEVICE_FAILURE;

        if( MODBUS_REQUEST_OK == config[ bitAddr ].ApplSetter( &value ) )
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
     * \brief Processes read bits request (Read Coils / Read Discrete Inputs)
     *
     * Request : | Addr | FC | Start Hi | Start Lo | Quantity Hi | Quantity Lo |
     * Response: | Addr | FC | Byte count N | Bits (N bytes, LSB of first byte = first item) |
     *
     * \param msgBuffer   [in/out]: Request, overwritten by response
     * \param msgLength   [in/out]: Length of request / response without CRC
     * \param config          [in]: Configuration array (indexed by address)
     * \param configCount     [in]: Count of items in configuration array
     *
     * \return Exception code or \ref MODBUS_ERROR_NO_ACTIVE_ERROR
     */
    template< typename ConfigType >
    modBus_ErrorStatus_t ReadBitsRequest( modBus_Message_t * const msgBuffer,
                                          modBus_MsgLength_t &msgLength,
                                          const ConfigType *config,
                                          uint16_t configCount )
    {
        /** Expected request length (address, FC, start address, quantity) */
        constexpr modBus_MsgLength_t requestLen    = 6u;
        /** Maximal count of bits in one request */
        constexpr uint16_t           maxQuantity   = 0x07D0u;
        constexpr modBus_MsgLength_t startAddrPos  = 2u;
        constexpr modBus_MsgLength_t quantityPos   = 4u;
        constexpr modBus_MsgLength_t byteCountPos  = 2u;
        constexpr modBus_MsgLength_t dataStartPos  = 3u;

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
                errorStatus = Check_BitsRange( config, configCount, startAddr, quantity, MODBUS_DATA_ACCESS_READ );
            }

            if( MODBUS_ERROR_NO_ACTIVE_ERROR == errorStatus )
            {
                const uint8_t byteCount = (uint8_t)( ( quantity + 7u ) / 8u );

                for( uint16_t byteIndex = 0u; byteCount > byteIndex; byteIndex++ )
                {
                    msgBuffer[ dataStartPos + byteIndex ] = 0u;
                }

                for( uint16_t bitIndex = 0u; quantity > bitIndex; bitIndex++ )
                {
                    modBus_CoilsRetValue_t bitValue = MODBUS_COIL_INACTIVE;

                    errorStatus = Get_Bit( config, (uint16_t)( startAddr + bitIndex ), bitValue );

                    if( MODBUS_ERROR_NO_ACTIVE_ERROR != errorStatus )
                    {
                        break;
                    }
                    else if( MODBUS_COIL_INACTIVE != bitValue )
                    {
                        msgBuffer[ dataStartPos + ( bitIndex / 8u ) ] |= (uint8_t)( 1u << ( bitIndex % 8u ) );
                    }
                    else
                    {
                        /* Bit is cleared already */
                    }
                }

                msgBuffer[ byteCountPos ] = byteCount;

                msgLength = dataStartPos + byteCount;
            }
            else
            {
                /* Error is returned */
            }
        }

        return ( errorStatus );
    }

} /* namespace DataAccess */
} /* namespace AppLayer */
} /* namespace ModBus */

#endif /* APPLAYER_MODBUS_APP_DATAACCESS_HPP */
