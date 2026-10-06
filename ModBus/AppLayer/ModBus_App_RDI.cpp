/**
 * \author Mr.Nobody
 * \file ModBus_App_RDI.cpp
 * \ingroup ModBus
 * \brief ModBus Read Discrete Inputs (RDI, 0x02) application layer handler
 *
 * # (0x02) Read Discrete Inputs #
 *
 * This function code is used to read from 1 to 2000 contiguous status of
 * discrete inputs in a remote device. The discrete inputs in the response
 * message are packed as one input per bit of the data field. The LSB of the
 * first data byte contains the input addressed in the query. Unused bits of
 * the last byte are padded with zeros.
 *
 * | Request               | Size    | Value            |
 * |-----------------------|---------|------------------|
 * | Function code         | 1 Byte  | 0x02             |
 * | Starting Address      | 2 Bytes | 0x0000 to 0xFFFF |
 * | Quantity of Inputs    | 2 Bytes | 1 to 2000        |
 *
 * | Response              | Size    | Value            |
 * |-----------------------|---------|------------------|
 * | Function code         | 1 Byte  | 0x02             |
 * | Byte count            | 1 Byte  | N = ceil(Q / 8)  |
 * | Input Status          | N Bytes |                  |
 *
 * Exceptions: 0x02 (address range), 0x03 (quantity, request length),
 * 0x04 (getter failed).
 *
 * Example - read inputs 197-218: request `02 00 C4 00 16`,
 * response `02 03 AC DB 35`.
 */
/* ============================= INCLUDES =================================== */
#include "ModBus_App_RDI.hpp"           /* Self include                       */
#include "ModBus_App_DataAccess.hpp"    /* Common data access                 */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ============================== MACROS ==================================== */

/* ========================= EXPORTED VARIABLES ============================= */

/* ========================== LOCAL VARIABLES =============================== */

/* ======================== EXPORTED FUNCTIONS ============================== */

/**
 * \brief Read Discrete Inputs handler constructor
 *
 * \param discreteIoHandler [in]: Discrete inputs configuration array (indexed by address)
 * \param discreteIoCount   [in]: Count of items in configuration array
 */
ModBus::AppLayer::RDI::RDI( const modBus_DiscreteIoConfigStruct_t *discreteIoHandler,
                            modBus_DiscreteIoCount_t discreteIoCount ) :
    discreteIoHandler( discreteIoHandler ),
    supportedDiscreteIoCount( discreteIoCount )
{
    return;
}


/**
 * \brief Processes Read Discrete Inputs (RDI) request
 *
 * \param msgBuffer [in,out]: Request, overwritten by response
 * \param msgLength [in,out]: Length of request / response without CRC
 *
 * \return Exception code or \ref MODBUS_ERROR_NO_ACTIVE_ERROR
 */
modBus_ErrorStatus_t ModBus::AppLayer::RDI::SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength )
{
    return ( DataAccess::ReadBitsRequest( msgBuffer, msgLength, discreteIoHandler, supportedDiscreteIoCount ) );
}

/* ========================== LOCAL FUNCTIONS =============================== */

/* =============================== TASKS ==================================== */
