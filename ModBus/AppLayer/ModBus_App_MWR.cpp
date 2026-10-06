/**
 * \author Mr.Nobody
 * \file ModBus_App_MWR.cpp
 * \ingroup ModBus
 * \brief ModBus Mask Write Register (MWR, 0x16) application layer handler
 *
 * # (0x16) Mask Write Register #
 *
 * This function code is used to modify the contents of a specified holding
 * register using a combination of an AND mask, an OR mask, and the register's
 * current contents. The function can be used to set or clear individual bits
 * in the register:
 *
 *     Result = (Current Contents AND And_Mask) OR (Or_Mask AND (NOT And_Mask))
 *
 * Register shall have both getter and setter. The normal response is an echo
 * of the request, returned after the register has been written.
 *
 * | Request / Response    | Size    | Value            |
 * |-----------------------|---------|------------------|
 * | Function code         | 1 Byte  | 0x16             |
 * | Reference Address     | 2 Bytes | 0x0000 to 0xFFFF |
 * | And_Mask              | 2 Bytes | 0x0000 to 0xFFFF |
 * | Or_Mask               | 2 Bytes | 0x0000 to 0xFFFF |
 *
 * Exceptions: 0x02 (address), 0x03 (request length), 0x04 (callback failed).
 *
 * Example - register 4 contains 0x12: request `16 00 04 00 F2 00 25`,
 * register value is 0x17, response `16 00 04 00 F2 00 25`.
 */
/* ============================= INCLUDES =================================== */
#include "ModBus_App_MWR.hpp"           /* Self include                       */
#include "ModBus_App_DataAccess.hpp"    /* Common data access                 */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ============================== MACROS ==================================== */

/* ========================= EXPORTED VARIABLES ============================= */

/* ========================== LOCAL VARIABLES =============================== */

/* ======================== EXPORTED FUNCTIONS ============================== */

/**
 * \brief Mask Write Register handler constructor
 *
 * \param holdingRegistersHandler [in]: Registers configuration array (indexed by address)
 * \param registersCount          [in]: Count of items in configuration array
 */
ModBus::AppLayer::MWR::MWR( const modBus_RegistersConfigStruct_t *holdingRegistersHandler, modBus_RegistersCount_t registersCount ) :
    registersHandler( holdingRegistersHandler ),
    supportedRegistersCount( registersCount )
{
    return;
}


/**
 * \brief Processes Mask Write Register (MWR) request
 *
 * \param msgBuffer [in,out]: Request, response is echo of the request
 * \param msgLength [in,out]: Length of request / response without CRC
 *
 * \return Exception code or \ref MODBUS_ERROR_NO_ACTIVE_ERROR
 */
modBus_ErrorStatus_t ModBus::AppLayer::MWR::SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength )
{
    modBus_ErrorStatus_t errorStatus = MODBUS_ERROR_NO_ACTIVE_ERROR;

    if( ( nullptr == msgBuffer ) || ( requestLen != msgLength ) )
    {
        errorStatus = MODBUS_ERROR_ILLEGAL_DATA_VALUE;
    }
    else
    {
        const uint16_t          referenceAddr = Get_16b( msgBuffer, referenceAddr_BuffPos );
        const uint16_t          andMask       = Get_16b( msgBuffer, andMask_BuffPos );
        const uint16_t          orMask        = Get_16b( msgBuffer, orMask_BuffPos );
        modBus_RegistersValue_t regValue      = 0u;

        errorStatus = DataAccess::Check_RegistersRange( registersHandler, supportedRegistersCount,
                                                        referenceAddr, 1u, MODBUS_DATA_ACCESS_READ );

        if( MODBUS_ERROR_NO_ACTIVE_ERROR == errorStatus )
        {
            errorStatus = DataAccess::Check_RegistersRange( registersHandler, supportedRegistersCount,
                                                            referenceAddr, 1u, MODBUS_DATA_ACCESS_WRITE );
        }
        else
        {
            /* Error is returned */
        }

        if( MODBUS_ERROR_NO_ACTIVE_ERROR == errorStatus )
        {
            errorStatus = DataAccess::Get_Register( registersHandler, referenceAddr, regValue );
        }
        else
        {
            /* Error is returned */
        }

        if( MODBUS_ERROR_NO_ACTIVE_ERROR == errorStatus )
        {
            regValue = (modBus_RegistersValue_t)( ( regValue & andMask ) | ( orMask & (uint16_t)( ~andMask ) ) );

            errorStatus = DataAccess::Set_Register( registersHandler, referenceAddr, regValue );

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
