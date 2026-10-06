/**
 * \author Mr.Nobody
 * \file ModBus_App_RIR.cpp
 * \ingroup ModBus
 * \brief ModBus Read Input Registers (RIR, 0x04) application layer handler
 *
 * # (0x04) Read Input Registers #
 *
 * This function code is used to read the contents of a contiguous block of
 * input registers in a remote device. Registers are addressed starting at
 * zero. The register data in the response message are packed as two bytes per
 * register, high byte first.
 *
 * | Request               | Size     | Value            |
 * |-----------------------|----------|------------------|
 * | Function code         | 1 Byte   | 0x04             |
 * | Starting Address      | 2 Bytes  | 0x0000 to 0xFFFF |
 * | Quantity of Registers | 2 Bytes  | 1 to 125         |
 *
 * | Response              | Size     | Value            |
 * |-----------------------|----------|------------------|
 * | Function code         | 1 Byte   | 0x04             |
 * | Byte count            | 1 Byte   | 2 x N            |
 * | Register value        | N x 2 B  |                  |
 *
 * Exceptions: 0x02 (address range), 0x03 (quantity, request length),
 * 0x04 (getter failed).
 *
 * Example - read register 9: request `04 00 08 00 01`,
 * response `04 02 00 0A`.
 */
/* ============================= INCLUDES =================================== */
#include "ModBus_App_RIR.hpp"           /* Self include                       */
#include "ModBus_App_DataAccess.hpp"    /* Common data access                 */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ============================== MACROS ==================================== */

/* ========================= EXPORTED VARIABLES ============================= */

/* ========================== LOCAL VARIABLES =============================== */

/* ======================== EXPORTED FUNCTIONS ============================== */

/**
 * \brief Read Input Registers handler constructor
 *
 * \param inputRegistersHandler [in]: Registers configuration array (indexed by address)
 * \param registersCount          [in]: Count of items in configuration array
 */
ModBus::AppLayer::RIR::RIR( const modBus_RegistersConfigStruct_t *inputRegistersHandler, modBus_RegistersCount_t registersCount ) :
    registersHandler( inputRegistersHandler ),
    supportedRegistersCount( registersCount )
{
    return;
}


/**
 * \brief Processes Read Input Registers (RIR) request
 *
 * \param msgBuffer [in,out]: Request, overwritten by response
 * \param msgLength [in,out]: Length of request / response without CRC
 *
 * \return Exception code or \ref MODBUS_ERROR_NO_ACTIVE_ERROR
 */
modBus_ErrorStatus_t ModBus::AppLayer::RIR::SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength )
{
    return ( DataAccess::ReadRegistersRequest( msgBuffer, msgLength, registersHandler, supportedRegistersCount ) );
}

/* ========================== LOCAL FUNCTIONS =============================== */

/* =============================== TASKS ==================================== */
