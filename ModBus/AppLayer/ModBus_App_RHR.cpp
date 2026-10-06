/**
 * \author Mr.Nobody
 * \file ModBus_App_RHR.cpp
 * \ingroup ModBus
 * \brief ModBus Read Holding Registers (RHR, 0x03) application layer handler
 *
 * # (0x03) Read Holding Registers #
 *
 * This function code is used to read the contents of a contiguous block of
 * holding registers in a remote device. Registers are addressed starting at
 * zero. The register data in the response message are packed as two bytes per
 * register, high byte first.
 *
 * | Request               | Size     | Value            |
 * |-----------------------|----------|------------------|
 * | Function code         | 1 Byte   | 0x03             |
 * | Starting Address      | 2 Bytes  | 0x0000 to 0xFFFF |
 * | Quantity of Registers | 2 Bytes  | 1 to 125         |
 *
 * | Response              | Size     | Value            |
 * |-----------------------|----------|------------------|
 * | Function code         | 1 Byte   | 0x03             |
 * | Byte count            | 1 Byte   | 2 x N            |
 * | Register value        | N x 2 B  |                  |
 *
 * Exceptions: 0x02 (address range), 0x03 (quantity, request length),
 * 0x04 (getter failed).
 *
 * Example - read registers 108-110: request `03 00 6B 00 03`,
 * response `03 06 02 2B 00 00 00 64`.
 */
/* ============================= INCLUDES =================================== */
#include "ModBus_App_RHR.hpp"           /* Self include                       */
#include "ModBus_App_DataAccess.hpp"    /* Common data access                 */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ============================== MACROS ==================================== */

/* ========================= EXPORTED VARIABLES ============================= */

/* ========================== LOCAL VARIABLES =============================== */

/* ======================== EXPORTED FUNCTIONS ============================== */

/**
 * \brief Read Holding Registers handler constructor
 *
 * \param holdingRegistersHandler [in]: Registers configuration array (indexed by address)
 * \param registersCount          [in]: Count of items in configuration array
 */
ModBus::AppLayer::RHR::RHR( const modBus_RegistersConfigStruct_t *holdingRegistersHandler, modBus_RegistersCount_t registersCount ) :
    registersHandler( holdingRegistersHandler ),
    supportedRegistersCount( registersCount )
{
    return;
}


/**
 * \brief Processes Read Holding Registers (RHR) request
 *
 * \param msgBuffer [in,out]: Request, overwritten by response
 * \param msgLength [in,out]: Length of request / response without CRC
 *
 * \return Exception code or \ref MODBUS_ERROR_NO_ACTIVE_ERROR
 */
modBus_ErrorStatus_t ModBus::AppLayer::RHR::SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength )
{
    return ( DataAccess::ReadRegistersRequest( msgBuffer, msgLength, registersHandler, supportedRegistersCount ) );
}

/* ========================== LOCAL FUNCTIONS =============================== */

/* =============================== TASKS ==================================== */
