/**
 * \author Mr.Nobody
 * \file ModBus_App_RC.cpp
 * \ingroup ModBus
 * \brief ModBus Read Coils (RC, 0x01) application layer handler
 *
 * # (0x01) Read Coils #
 *
 * This function code is used to read from 1 to 2000 contiguous status of coils
 * in a remote device. The coils in the response message are packed as one coil
 * per bit of the data field. The LSB of the first data byte contains the output
 * addressed in the query. Unused bits of the last byte are padded with zeros.
 *
 * | Request               | Size    | Value            |
 * |-----------------------|---------|------------------|
 * | Function code         | 1 Byte  | 0x01             |
 * | Starting Address      | 2 Bytes | 0x0000 to 0xFFFF |
 * | Quantity of coils     | 2 Bytes | 1 to 2000        |
 *
 * | Response              | Size    | Value            |
 * |-----------------------|---------|------------------|
 * | Function code         | 1 Byte  | 0x01             |
 * | Byte count            | 1 Byte  | N = ceil(Q / 8)  |
 * | Coil Status           | N Bytes |                  |
 *
 * Exceptions: 0x02 (address range), 0x03 (quantity, request length),
 * 0x04 (getter failed).
 *
 * Example - read coils 20-38: request `01 00 13 00 13`,
 * response `01 03 CD 6B 05`.
 */
/* ============================= INCLUDES =================================== */
#include "ModBus_App_RC.hpp"            /* Self include                       */
#include "ModBus_App_DataAccess.hpp"    /* Common data access                 */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ============================== MACROS ==================================== */

/* ========================= EXPORTED VARIABLES ============================= */

/* ========================== LOCAL VARIABLES =============================== */

/* ======================== EXPORTED FUNCTIONS ============================== */

/**
 * \brief Read Coils handler constructor
 *
 * \param coilsHandler [in]: Coils configuration array (indexed by coil address)
 * \param coilsCount   [in]: Count of items in configuration array
 */
ModBus::AppLayer::RC::RC( const modBus_CoilsConfigStruct_t *coilsHandler, modBus_CoilsCount_t coilsCount ) :
    coilsHandler( coilsHandler ),
    supportedCoilsCount( coilsCount )
{
    return;
}


/**
 * \brief Processes Read Coils (RC) request
 *
 * \param msgBuffer [in,out]: Request, overwritten by response
 * \param msgLength [in,out]: Length of request / response without CRC
 *
 * \return Exception code or \ref MODBUS_ERROR_NO_ACTIVE_ERROR
 */
modBus_ErrorStatus_t ModBus::AppLayer::RC::SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength )
{
    return ( DataAccess::ReadBitsRequest( msgBuffer, msgLength, coilsHandler, supportedCoilsCount ) );
}

/* ========================== LOCAL FUNCTIONS =============================== */

/* =============================== TASKS ==================================== */
