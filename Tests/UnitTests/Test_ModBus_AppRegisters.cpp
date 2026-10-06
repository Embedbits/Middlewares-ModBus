/**
 * \author Mr.Nobody
 * \file Test_ModBus_AppRegisters.cpp
 * \ingroup UnitTests
 * \brief Unit tests of ModBus 16 bit access handlers - Read Holding Registers
 *        (0x03), Read Input Registers (0x04), Write Single Register (0x06),
 *        Write Multiple Registers (0x10), Mask Write Register (0x16),
 *        Read/Write Multiple Registers (0x17), Read FIFO Queue (0x18).
 */

/* ============================= INCLUDES =================================== */
#include "unity.h"                          /* Unity testing framework        */
#include "UtModBus_Helpers.hpp"             /* Test helpers                   */
#include "ModBus_App_RHR.hpp"               /* Read Holding Registers         */
#include "ModBus_App_RIR.hpp"               /* Read Input Registers           */
#include "ModBus_App_WSR.hpp"               /* Write Single Register          */
#include "ModBus_App_WMR.hpp"               /* Write Multiple Registers       */
#include "ModBus_App_MWR.hpp"               /* Mask Write Register            */
#include "ModBus_App_RWMR.hpp"              /* Read/Write Multiple Registers  */
#include "ModBus_App_RFIFO.hpp"             /* Read FIFO Queue                */
/* ========================= SYMBOLIC CONSTANTS ============================= */

/** Count of emulated registers */
static constexpr uint16_t UT_REGS_CNT = 32u;

/* ========================== LOCAL VARIABLES =============================== */

/** Emulated registers */
static uint16_t ut_Regs[ UT_REGS_CNT ];

/** Address of register with failing callback (-1 = none) */
static int32_t ut_FailingReg;

/** Count of setter calls */
static uint32_t ut_SetterCnt;

/** Count of values in emulated FIFO */
static uint16_t ut_FifoCount;

/* ========================== LOCAL FUNCTIONS =============================== */

template< int RegId >
static modBus_RequestState_t UtGetReg( modBus_RegistersValue_t *value )
{
    *value = ut_Regs[ RegId ];

    return ( ( RegId == ut_FailingReg ) ? MODBUS_REQUEST_ERROR : MODBUS_REQUEST_OK );
}


template< int RegId >
static modBus_RequestState_t UtSetReg( modBus_RegistersValue_t *value )
{
    ut_Regs[ RegId ] = *value;
    ut_SetterCnt++;

    return ( ( RegId == ut_FailingReg ) ? MODBUS_REQUEST_ERROR : MODBUS_REQUEST_OK );
}

#define UT_REG( id )        { id, MODBUS_SERVICE_SUPPORTED, UtGetReg< id >, UtSetReg< id > }
#define UT_REGS_8( id )     UT_REG( id ), UT_REG( id + 1 ), UT_REG( id + 2 ), UT_REG( id + 3 ), \
                            UT_REG( id + 4 ), UT_REG( id + 5 ), UT_REG( id + 6 ), UT_REG( id + 7 )

/** Holding registers - register 30 is read only, register 31 write only */
static const modBus_RegistersConfigStruct_t ut_RegConfig[ UT_REGS_CNT ] =
{
    UT_REGS_8( 0 ), UT_REGS_8( 8 ), UT_REGS_8( 16 ),
    UT_REG( 24 ), UT_REG( 25 ), UT_REG( 26 ), UT_REG( 27 ), UT_REG( 28 ), UT_REG( 29 ),
    { 30, MODBUS_SERVICE_SUPPORTED, UtGetReg< 30 >, nullptr         },
    { 31, MODBUS_SERVICE_SUPPORTED, nullptr,        UtSetReg< 31 >  }
};


/** FIFO callback - FIFO at address 0x04DE contains ut_FifoCount values 0x01B8, 0x1284, ... */
static modBus_RequestState_t UtFifoCallback( uint16_t fifoAddress, modBus_RegistersValue_t *values, uint16_t *fifoCount )
{
    modBus_RequestState_t retState = MODBUS_REQUEST_ERROR;

    if( 0x04DEu == fifoAddress )
    {
        for( uint16_t valueIndex = 0u; ( ut_FifoCount > valueIndex ) && ( MODBUS_RFIFO_MAX_COUNT > valueIndex ); valueIndex++ )
        {
            values[ valueIndex ] = ( 0u == valueIndex ) ? 0x01B8u : (uint16_t)( 0x1284u + valueIndex - 1u );
        }

        *fifoCount = ut_FifoCount;
        retState   = MODBUS_REQUEST_OK;
    }
    else
    {
        /* Unknown FIFO */
    }

    return ( retState );
}

/* ======================== EXPORTED FUNCTIONS ============================== */

void setUp( void )
{
    for( uint16_t regIndex = 0u; UT_REGS_CNT > regIndex; regIndex++ )
    {
        ut_Regs[ regIndex ] = (uint16_t)( 0x1000u + regIndex );
    }

    ut_FailingReg = -1;
    ut_SetterCnt  = 0u;
    ut_FifoCount  = 2u;
}


void tearDown( void )
{
}

/* ------------------------ Read Holding Registers -------------------------- */

void Ut_RHR_ReadRegisters( void )
{
    ModBus::AppLayer::RHR rhr( ut_RegConfig, UT_REGS_CNT );
    uint8_t               buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t    respLen    = 0u;
    const uint8_t         request[]  = { UtModBus::SlaveAddr, 0x03, 0x00, 0x01, 0x00, 0x03 };
    const uint8_t         expected[] = { UtModBus::SlaveAddr, 0x03, 0x06, 0x02, 0x2B, 0x00, 0x00, 0x00, 0x64 };

    ut_Regs[ 1 ] = 0x022Bu;
    ut_Regs[ 2 ] = 0x0000u;
    ut_Regs[ 3 ] = 0x0064u;

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( rhr, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_UINT16( sizeof( expected ), respLen );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( expected, buffer, sizeof( expected ) );
}


/** Range ending at the last register is valid, one more register is not */
void Ut_RHR_AddressRange( void )
{
    ModBus::AppLayer::RHR rhr( ut_RegConfig, UT_REGS_CNT );
    uint8_t               buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t    respLen = 0u;
    const uint8_t         lastOk[]    = { UtModBus::SlaveAddr, 0x03, 0x00, 0x1C, 0x00, 0x03 };
    const uint8_t         behindEnd[] = { UtModBus::SlaveAddr, 0x03, 0x00, 0x1D, 0x00, 0x03 };
    const uint8_t         startOut[]  = { UtModBus::SlaveAddr, 0x03, 0x00, 0x20, 0x00, 0x01 };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR,      UtModBus::Decode( rhr, lastOk,    sizeof( lastOk ),    buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_ADDRESS, UtModBus::Decode( rhr, behindEnd, sizeof( behindEnd ), buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_ADDRESS, UtModBus::Decode( rhr, startOut,  sizeof( startOut ),  buffer, respLen ) );
}


void Ut_RHR_Quantity_IllegalDataValue( void )
{
    ModBus::AppLayer::RHR rhr( ut_RegConfig, UT_REGS_CNT );
    uint8_t               buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t    respLen = 0u;
    const uint8_t         qty0[]   = { UtModBus::SlaveAddr, 0x03, 0x00, 0x00, 0x00, 0x00 };
    const uint8_t         qty126[] = { UtModBus::SlaveAddr, 0x03, 0x00, 0x00, 0x00, 0x7E };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE, UtModBus::Decode( rhr, qty0,   sizeof( qty0 ),   buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE, UtModBus::Decode( rhr, qty126, sizeof( qty126 ), buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE, UtModBus::Decode( rhr, qty0,   4u,               buffer, respLen ) );
}


void Ut_RHR_GetterFailureOrWriteOnly( void )
{
    ModBus::AppLayer::RHR rhr( ut_RegConfig, UT_REGS_CNT );
    uint8_t               buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t    respLen = 0u;
    const uint8_t         request[]   = { UtModBus::SlaveAddr, 0x03, 0x00, 0x00, 0x00, 0x04 };
    const uint8_t         writeOnly[] = { UtModBus::SlaveAddr, 0x03, 0x00, 0x1F, 0x00, 0x01 };

    ut_FailingReg = 2;

    TEST_ASSERT_EQUAL( MODBUS_ERROR_SLAVE_DEVICE_FAILURE, UtModBus::Decode( rhr, request,   sizeof( request ),   buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_ADDRESS, UtModBus::Decode( rhr, writeOnly, sizeof( writeOnly ), buffer, respLen ) );
}

/* ------------------------- Read Input Registers --------------------------- */

void Ut_RIR_ReadRegister( void )
{
    ModBus::AppLayer::RIR rir( ut_RegConfig, UT_REGS_CNT );
    uint8_t               buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t    respLen    = 0u;
    const uint8_t         request[]  = { UtModBus::SlaveAddr, 0x04, 0x00, 0x08, 0x00, 0x01 };
    const uint8_t         expected[] = { UtModBus::SlaveAddr, 0x04, 0x02, 0x00, 0x0A };

    ut_Regs[ 8 ] = 0x000Au;

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( rir, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_UINT16( sizeof( expected ), respLen );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( expected, buffer, sizeof( expected ) );
    TEST_ASSERT_EQUAL( MODBUS_FUNCTION_RIR, rir.GetFunctionCode() );
}

/* ------------------------- Write Single Register -------------------------- */

void Ut_WSR_WriteRegister_Echo( void )
{
    ModBus::AppLayer::WSR wsr( ut_RegConfig, UT_REGS_CNT );
    uint8_t               buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t    respLen   = 0u;
    const uint8_t         request[] = { UtModBus::SlaveAddr, 0x06, 0x00, 0x01, 0x00, 0x03 };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( wsr, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_HEX16( 0x0003u, ut_Regs[ 1 ] );
    TEST_ASSERT_EQUAL_UINT16( sizeof( request ), respLen );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( request, buffer, sizeof( request ) );
}


/** Address equal to count of registers is outside of configuration (no out of bounds access) */
void Ut_WSR_AddressBehindLastRegister_IllegalDataAddress( void )
{
    ModBus::AppLayer::WSR wsr( ut_RegConfig, UT_REGS_CNT );
    uint8_t               buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t    respLen = 0u;
    const uint8_t         behindEnd[] = { UtModBus::SlaveAddr, 0x06, 0x00, 0x20, 0x00, 0x03 };
    const uint8_t         readOnly[]  = { UtModBus::SlaveAddr, 0x06, 0x00, 0x1E, 0x00, 0x03 };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_ADDRESS, UtModBus::Decode( wsr, behindEnd, sizeof( behindEnd ), buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_ADDRESS, UtModBus::Decode( wsr, readOnly,  sizeof( readOnly ),  buffer, respLen ) );
    TEST_ASSERT_EQUAL_UINT32( 0u, ut_SetterCnt );
}


void Ut_WSR_LengthAndSetterFailure( void )
{
    ModBus::AppLayer::WSR wsr( ut_RegConfig, UT_REGS_CNT );
    uint8_t               buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t    respLen   = 0u;
    const uint8_t         request[] = { UtModBus::SlaveAddr, 0x06, 0x00, 0x05, 0x00, 0x03 };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE, UtModBus::Decode( wsr, request, 5u, buffer, respLen ) );

    ut_FailingReg = 5;

    TEST_ASSERT_EQUAL( MODBUS_ERROR_SLAVE_DEVICE_FAILURE, UtModBus::Decode( wsr, request, sizeof( request ), buffer, respLen ) );
}

/* ------------------------ Write Multiple Registers ------------------------ */

void Ut_WMR_WriteRegisters( void )
{
    ModBus::AppLayer::WMR wmr( ut_RegConfig, UT_REGS_CNT );
    uint8_t               buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t    respLen    = 0u;
    const uint8_t         request[]  = { UtModBus::SlaveAddr, 0x10, 0x00, 0x01, 0x00, 0x02, 0x04, 0x00, 0x0A, 0x01, 0x02 };
    const uint8_t         expected[] = { UtModBus::SlaveAddr, 0x10, 0x00, 0x01, 0x00, 0x02 };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( wmr, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_UINT16( sizeof( expected ), respLen );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( expected, buffer, sizeof( expected ) );
    TEST_ASSERT_EQUAL_HEX16( 0x000Au, ut_Regs[ 1 ] );
    TEST_ASSERT_EQUAL_HEX16( 0x0102u, ut_Regs[ 2 ] );
    TEST_ASSERT_EQUAL_HEX16( 0x1003u, ut_Regs[ 3 ] );
}


void Ut_WMR_InvalidStructure_IllegalDataValue( void )
{
    ModBus::AppLayer::WMR wmr( ut_RegConfig, UT_REGS_CNT );
    uint8_t               buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t    respLen = 0u;
    const uint8_t         wrongByteCount[] = { UtModBus::SlaveAddr, 0x10, 0x00, 0x01, 0x00, 0x02, 0x03, 0x00, 0x0A, 0x01 };
    const uint8_t         missingData[]    = { UtModBus::SlaveAddr, 0x10, 0x00, 0x01, 0x00, 0x02, 0x04, 0x00, 0x0A, 0x01 };
    const uint8_t         qty124[]         = { UtModBus::SlaveAddr, 0x10, 0x00, 0x00, 0x00, 0x7C, 0xF8 };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE, UtModBus::Decode( wmr, wrongByteCount, sizeof( wrongByteCount ), buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE, UtModBus::Decode( wmr, missingData,    sizeof( missingData ),    buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE, UtModBus::Decode( wmr, qty124,         sizeof( qty124 ),         buffer, respLen ) );
    TEST_ASSERT_EQUAL_UINT32( 0u, ut_SetterCnt );
}


/** Range containing read only register - nothing is written */
void Ut_WMR_AddressError_NoSideEffects( void )
{
    ModBus::AppLayer::WMR wmr( ut_RegConfig, UT_REGS_CNT );
    uint8_t               buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t    respLen   = 0u;
    const uint8_t         request[] = { UtModBus::SlaveAddr, 0x10, 0x00, 0x1D, 0x00, 0x02, 0x04, 0x00, 0x01, 0x00, 0x02 };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_ADDRESS, UtModBus::Decode( wmr, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_UINT32( 0u, ut_SetterCnt );
}

/* --------------------------- Mask Write Register -------------------------- */

/** Example from ModBus specification - 0x12 AND 0xF2 OR 0x25 = 0x17 */
void Ut_MWR_SpecificationExample( void )
{
    ModBus::AppLayer::MWR mwr( ut_RegConfig, UT_REGS_CNT );
    uint8_t               buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t    respLen   = 0u;
    const uint8_t         request[] = { UtModBus::SlaveAddr, 0x16, 0x00, 0x04, 0x00, 0xF2, 0x00, 0x25 };

    ut_Regs[ 4 ] = 0x0012u;

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( mwr, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_HEX16( 0x0017u, ut_Regs[ 4 ] );
    TEST_ASSERT_EQUAL_UINT16( sizeof( request ), respLen );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( request, buffer, sizeof( request ) );
}


/** Register has to be readable and writable */
void Ut_MWR_ReadOnlyOrWriteOnly_IllegalDataAddress( void )
{
    ModBus::AppLayer::MWR mwr( ut_RegConfig, UT_REGS_CNT );
    uint8_t               buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t    respLen = 0u;
    const uint8_t         readOnly[]  = { UtModBus::SlaveAddr, 0x16, 0x00, 0x1E, 0x00, 0xF2, 0x00, 0x25 };
    const uint8_t         writeOnly[] = { UtModBus::SlaveAddr, 0x16, 0x00, 0x1F, 0x00, 0xF2, 0x00, 0x25 };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_ADDRESS, UtModBus::Decode( mwr, readOnly,  sizeof( readOnly ),  buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_ADDRESS, UtModBus::Decode( mwr, writeOnly, sizeof( writeOnly ), buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE,   UtModBus::Decode( mwr, readOnly,  6u,                  buffer, respLen ) );
}

/* --------------------- Read/Write Multiple Registers ---------------------- */

/** Example from ModBus specification - read 6 registers from 3, write 3 registers from 14 */
void Ut_RWMR_SpecificationExample( void )
{
    ModBus::AppLayer::RWMR rwmr( ut_RegConfig, UT_REGS_CNT );
    uint8_t                buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t     respLen   = 0u;
    const uint8_t          request[] = { UtModBus::SlaveAddr, 0x17, 0x00, 0x03, 0x00, 0x06, 0x00, 0x0E, 0x00, 0x03,
                                         0x06, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF };
    const uint8_t          expected[] = { UtModBus::SlaveAddr, 0x17, 0x0C, 0x00, 0xFE, 0x0A, 0xCD, 0x00, 0x01,
                                          0x00, 0x03, 0x00, 0x0D, 0x00, 0xFF };

    ut_Regs[ 3 ] = 0x00FEu;
    ut_Regs[ 4 ] = 0x0ACDu;
    ut_Regs[ 5 ] = 0x0001u;
    ut_Regs[ 6 ] = 0x0003u;
    ut_Regs[ 7 ] = 0x000Du;
    ut_Regs[ 8 ] = 0x00FFu;

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( rwmr, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_UINT16( sizeof( expected ), respLen );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( expected, buffer, sizeof( expected ) );
    TEST_ASSERT_EQUAL_HEX16( 0x00FFu, ut_Regs[ 14 ] );
    TEST_ASSERT_EQUAL_HEX16( 0x00FFu, ut_Regs[ 15 ] );
    TEST_ASSERT_EQUAL_HEX16( 0x00FFu, ut_Regs[ 16 ] );
}


/** Write operation is performed before read */
void Ut_RWMR_WriteBeforeRead( void )
{
    ModBus::AppLayer::RWMR rwmr( ut_RegConfig, UT_REGS_CNT );
    uint8_t                buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t     respLen   = 0u;
    const uint8_t          request[] = { UtModBus::SlaveAddr, 0x17, 0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0x00, 0x01,
                                         0x02, 0xBE, 0xEF };
    const uint8_t          expected[] = { UtModBus::SlaveAddr, 0x17, 0x04, 0x10, 0x00, 0xBE, 0xEF };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( rwmr, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( expected, buffer, sizeof( expected ) );
}


void Ut_RWMR_Errors( void )
{
    ModBus::AppLayer::RWMR rwmr( ut_RegConfig, UT_REGS_CNT );
    uint8_t                buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t     respLen = 0u;
    const uint8_t          wrongByteCount[] = { UtModBus::SlaveAddr, 0x17, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01,
                                                0x04, 0xBE, 0xEF };
    const uint8_t          readQty0[]       = { UtModBus::SlaveAddr, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01,
                                                0x02, 0xBE, 0xEF };
    const uint8_t          readOutOfRange[] = { UtModBus::SlaveAddr, 0x17, 0x00, 0x1F, 0x00, 0x02, 0x00, 0x01, 0x00, 0x01,
                                                0x02, 0xBE, 0xEF };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE,   UtModBus::Decode( rwmr, wrongByteCount, sizeof( wrongByteCount ), buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE,   UtModBus::Decode( rwmr, readQty0,       sizeof( readQty0 ),       buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_ADDRESS, UtModBus::Decode( rwmr, readOutOfRange, sizeof( readOutOfRange ), buffer, respLen ) );

    /* Nothing written in case of read range error */
    TEST_ASSERT_EQUAL_UINT32( 0u, ut_SetterCnt );
}

/* ---------------------------- Read FIFO Queue ----------------------------- */

/** Example from ModBus specification - FIFO at 1246 with 2 values */
void Ut_RFIFO_SpecificationExample( void )
{
    ModBus::AppLayer::RFIFO rfifo( UtFifoCallback );
    uint8_t                 buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t      respLen    = 0u;
    const uint8_t           request[]  = { UtModBus::SlaveAddr, 0x18, 0x04, 0xDE };
    const uint8_t           expected[] = { UtModBus::SlaveAddr, 0x18, 0x00, 0x06, 0x00, 0x02, 0x01, 0xB8, 0x12, 0x84 };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( rfifo, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_UINT16( sizeof( expected ), respLen );
    TEST_ASSERT_EQUAL_HEX8_ARRAY( expected, buffer, sizeof( expected ) );
}


void Ut_RFIFO_EmptyAndFullQueue( void )
{
    ModBus::AppLayer::RFIFO rfifo( UtFifoCallback );
    uint8_t                 buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t      respLen   = 0u;
    const uint8_t           request[] = { UtModBus::SlaveAddr, 0x18, 0x04, 0xDE };

    ut_FifoCount = 0u;
    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( rfifo, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_UINT16( 6u, respLen );
    TEST_ASSERT_EQUAL_HEX8( 0x02u, buffer[ 3 ] );

    ut_FifoCount = 31u;
    TEST_ASSERT_EQUAL( MODBUS_ERROR_NO_ACTIVE_ERROR, UtModBus::Decode( rfifo, request, sizeof( request ), buffer, respLen ) );
    TEST_ASSERT_EQUAL_UINT16( 6u + 62u, respLen );
    TEST_ASSERT_EQUAL_HEX8( 64u, buffer[ 3 ] );
}


void Ut_RFIFO_Errors( void )
{
    ModBus::AppLayer::RFIFO rfifo( UtFifoCallback );
    ModBus::AppLayer::RFIFO rfifoNoCallback( nullptr );
    uint8_t                 buffer[ UtModBus::BufferSize ];
    modBus_MsgLength_t      respLen = 0u;
    const uint8_t           request[]     = { UtModBus::SlaveAddr, 0x18, 0x04, 0xDE };
    const uint8_t           unknownFifo[] = { UtModBus::SlaveAddr, 0x18, 0x00, 0x01 };

    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_ADDRESS, UtModBus::Decode( rfifo,           unknownFifo, sizeof( unknownFifo ), buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_ADDRESS, UtModBus::Decode( rfifoNoCallback, request,     sizeof( request ),     buffer, respLen ) );
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE,   UtModBus::Decode( rfifo,           request,     3u,                    buffer, respLen ) );

    ut_FifoCount = 32u;
    TEST_ASSERT_EQUAL( MODBUS_ERROR_ILLEGAL_DATA_VALUE,   UtModBus::Decode( rfifo,           request,     sizeof( request ),     buffer, respLen ) );
}

