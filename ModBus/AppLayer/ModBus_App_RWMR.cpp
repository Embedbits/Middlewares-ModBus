/**
 * \author Mr.Nobody
 * \file ModBus_App_RWMR.cpp
 * \ingroup ModBus
 * \brief ModBus Read/Write Multiple Registers (RWMR, 0x17) application layer
 *        handler
 *
 * # (0x17) Read/Write Multiple Registers #
 *
 * This function code performs a combination of one read operation and one
 * write operation in a single MODBUS transaction. The write operation is
 * performed before the read. The normal response contains the data from the
 * group of registers that were read.
 *
 * | Request                | Size     | Value            |
 * |------------------------|----------|------------------|
 * | Function code          | 1 Byte   | 0x17             |
 * | Read Starting Address  | 2 Bytes  | 0x0000 to 0xFFFF |
 * | Quantity to Read       | 2 Bytes  | 1 to 125         |
 * | Write Starting Address | 2 Bytes  | 0x0000 to 0xFFFF |
 * | Quantity to Write      | 2 Bytes  | 1 to 121         |
 * | Write Byte Count       | 1 Byte   | 2 x N (write)    |
 * | Write Registers Value  | N x 2 B  |                  |
 *
 * | Response               | Size     | Value            |
 * |------------------------|----------|------------------|
 * | Function code          | 1 Byte   | 0x17             |
 * | Byte count             | 1 Byte   | 2 x N (read)     |
 * | Read Registers value   | N x 2 B  |                  |
 *
 * Exceptions: 0x02 (address ranges), 0x03 (quantities, byte count, request
 * length), 0x04 (callback failed).
 *
 * Example - read 6 registers from 4, write 3 registers from 15: request
 * `17 00 03 00 06 00 0E 00 03 06 00 FF 00 FF 00 FF`, response
 * `17 0C 00 FE 0A CD 00 01 00 03 00 0D 00 FF`.
 */
/* ============================= INCLUDES =================================== */
#include "ModBus_App_RWMR.hpp"          /* Self include                       */
#include "ModBus_App_DataAccess.hpp"    /* Common data access                 */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ============================== MACROS ==================================== */

/* ========================= EXPORTED VARIABLES ============================= */

/* ========================== LOCAL VARIABLES =============================== */

/* ======================== EXPORTED FUNCTIONS ============================== */

/**
 * \brief Read/Write Multiple Registers handler constructor
 *
 * \param holdingRegistersHandler [in]: Registers configuration array (indexed by address)
 * \param registersCount          [in]: Count of items in configuration array
 */
ModBus::AppLayer::RWMR::RWMR( const modBus_RegistersConfigStruct_t *holdingRegistersHandler, modBus_RegistersCount_t registersCount ) :
    registersHandler( holdingRegistersHandler ),
    supportedRegistersCount( registersCount )
{
    return;
}


/**
 * \brief Processes Read/Write Multiple Registers (RWMR) request
 *
 * Write values are consumed before response is written into the buffer, thus
 * in-place processing is safe (response values start before write values).
 *
 * \param msgBuffer [in,out]: Request, overwritten by response
 * \param msgLength [in,out]: Length of request / response without CRC
 *
 * \return Exception code or \ref MODBUS_ERROR_NO_ACTIVE_ERROR
 */
modBus_ErrorStatus_t ModBus::AppLayer::RWMR::SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength )
{
    modBus_ErrorStatus_t errorStatus = MODBUS_ERROR_NO_ACTIVE_ERROR;

    if( ( nullptr == msgBuffer ) || ( writeValues_BuffPos > msgLength ) )
    {
        errorStatus = MODBUS_ERROR_ILLEGAL_DATA_VALUE;
    }
    else
    {
        const uint16_t readStartAddr  = Get_16b( msgBuffer, readStartAddr_BuffPos );
        const uint16_t readQuantity   = Get_16b( msgBuffer, readQuantity_BuffPos );
        const uint16_t writeStartAddr = Get_16b( msgBuffer, writeStartAddr_BuffPos );
        const uint16_t writeQuantity  = Get_16b( msgBuffer, writeQuantity_BuffPos );
        const uint8_t  writeByteCount = msgBuffer[ writeByteCount_BuffPos ];

        if( ( 0u                                       == readQuantity   ) ||
            ( maxReadQuantity                          <  readQuantity   ) ||
            ( 0u                                       == writeQuantity  ) ||
            ( maxWriteQuantity                         <  writeQuantity  ) ||
            ( ( 2u * writeQuantity )                   != writeByteCount ) ||
            ( ( writeValues_BuffPos + writeByteCount ) != msgLength      )    )
        {
            errorStatus = MODBUS_ERROR_ILLEGAL_DATA_VALUE;
        }
        else
        {
            errorStatus = DataAccess::Check_RegistersRange( registersHandler, supportedRegistersCount,
                                                            writeStartAddr, writeQuantity, MODBUS_DATA_ACCESS_WRITE );

            if( MODBUS_ERROR_NO_ACTIVE_ERROR == errorStatus )
            {
                errorStatus = DataAccess::Check_RegistersRange( registersHandler, supportedRegistersCount,
                                                                readStartAddr, readQuantity, MODBUS_DATA_ACCESS_READ );
            }
            else
            {
                /* Error is returned */
            }
        }

        /* Write operation is performed first */
        for( uint16_t regIndex = 0u; ( MODBUS_ERROR_NO_ACTIVE_ERROR == errorStatus ) && ( writeQuantity > regIndex ); regIndex++ )
        {
            const modBus_RegistersValue_t regValue = Get_16b( msgBuffer, writeValues_BuffPos + ( 2u * regIndex ) );

            errorStatus = DataAccess::Set_Register( registersHandler, (uint16_t)( writeStartAddr + regIndex ), regValue );
        }

        /* Read operation follows */
        for( uint16_t regIndex = 0u; ( MODBUS_ERROR_NO_ACTIVE_ERROR == errorStatus ) && ( readQuantity > regIndex ); regIndex++ )
        {
            modBus_RegistersValue_t regValue = 0u;

            errorStatus = DataAccess::Get_Register( registersHandler, (uint16_t)( readStartAddr + regIndex ), regValue );

            Set_16b( msgBuffer, readValues_BuffPos + ( 2u * regIndex ), regValue );
        }

        if( MODBUS_ERROR_NO_ACTIVE_ERROR == errorStatus )
        {
            msgBuffer[ readByteCount_BuffPos ] = (uint8_t)( 2u * readQuantity );

            msgLength = readValues_BuffPos + ( 2u * readQuantity );
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
