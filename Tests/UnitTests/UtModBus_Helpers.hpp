/**
 * \author Mr.Nobody
 * \file UtModBus_Helpers.hpp
 * \ingroup UnitTests
 * \brief Helpers of ModBus application layer unit tests.
 */

#ifndef MODBUS_UNITTESTS_UTMODBUS_HELPERS_HPP
#define MODBUS_UNITTESTS_UTMODBUS_HELPERS_HPP

/* ============================= INCLUDES =================================== */
#include "unity.h"                          /* Unity testing framework        */
#include "UtModBus_FakePhy.hpp"             /* Fake physical layer            */
#include "ModBus_App.hpp"                   /* Application layer core         */
#include "ModBus_App_CommDiag.hpp"          /* Communication diagnostics      */
#include "ModBus_Slave_TL.hpp"              /* Slave transport layer          */
/* ========================= SYMBOLIC CONSTANTS ============================= */

namespace UtModBus
{
    /** Slave address used in tests */
    static constexpr modBus_DeviceAddr_t SlaveAddr = 0x11u;

    /** Size of message buffer used in tests */
    static constexpr modBus_MsgLength_t BufferSize = MODBUS_FRAME_MAX_LEN;

    /**
     * \brief Calls handler with request (address and PDU, without CRC)
     *
     * \param handler   [in]: Handler under test
     * \param request   [in]: Request
     * \param reqLen    [in]: Request length
     * \param buffer   [out]: Message buffer (BufferSize), contains response
     * \param respLen  [out]: Response length
     *
     * \return Error status returned by handler
     */
    static inline modBus_ErrorStatus_t Decode( ModBus::FunctionsHandler &handler,
                                               const uint8_t *request, modBus_MsgLength_t reqLen,
                                               uint8_t *buffer, modBus_MsgLength_t &respLen )
    {
        for( modBus_MsgLength_t byteIndex = 0u; BufferSize > byteIndex; byteIndex++ )
        {
            buffer[ byteIndex ] = ( reqLen > byteIndex ) ? request[ byteIndex ] : 0xEEu;
        }

        respLen = reqLen;

        return ( handler.SlaveDecodeMessage( buffer, respLen ) );
    }


    /** Complete slave stack - fake physical layer, transport layer, diagnostics and application core */
    class SlaveStack
    {
    public:

        SlaveStack( ModBus::FunctionsHandler * const handlers[], uint8_t handlersCnt, bool useDiagnostics = true ) :
            tl( phy ),
            diag( tl ),
            core( tl, handlers, handlersCnt, useDiagnostics ? &diag : nullptr )
        {
            TEST_ASSERT_EQUAL( MODBUS_REQUEST_OK, tl.Set_SlaveAddr( SlaveAddr ) );

            /* Init -> Standby */
            core.Task( taskTime );
        }

        /**
         * \brief Receives request and runs tasks until response is sent
         *
         * \param request  [in]: Request without CRC
         * \param reqLen   [in]: Request length
         * \param response [out]: Response without CRC (BufferSize)
         *
         * \return Length of response without CRC, 0 if no response was sent
         */
        modBus_MsgLength_t Transact( const uint8_t *request, modBus_MsgLength_t reqLen, uint8_t *response )
        {
            const uint32_t     txCntBefore = phy.txStartCnt;
            modBus_MsgLength_t respLen     = 0u;

            phy.ReceiveFrame( request, reqLen );

            for( uint8_t taskIndex = 0u; 5u > taskIndex; taskIndex++ )
            {
                core.Task( taskTime );
            }

            TEST_ASSERT_EQUAL( MODBUS_SLAVE_TL_STANDBY, tl.Get_ModuleState() );

            if( txCntBefore != phy.txStartCnt )
            {
                TEST_ASSERT_TRUE( MODBUS_FRAME_MIN_LEN <= phy.txLen );

                respLen = phy.txLen - MODBUS_CRC_LEN;

                const uint16_t crc = Crc16( phy.txFrame, respLen );

                TEST_ASSERT_EQUAL_HEX8( crc & 0xFFu, phy.txFrame[ respLen ] );
                TEST_ASSERT_EQUAL_HEX8( crc >> 8u,   phy.txFrame[ respLen + 1u ] );

                for( modBus_MsgLength_t byteIndex = 0u; respLen > byteIndex; byteIndex++ )
                {
                    response[ byteIndex ] = phy.txFrame[ byteIndex ];
                }
            }
            else
            {
                /* No response */
            }

            return ( respLen );
        }

        /** Task period in ms */
        static constexpr modBus_Time_t taskTime = 10u;

        FakePhy                            phy;
        ModBus::Slave::TransportLayer      tl;
        ModBus::AppLayer::CommDiagnostics  diag;
        ModBus::AppLayer::Slave::AppCore   core;
    };
}

#endif /* MODBUS_UNITTESTS_UTMODBUS_HELPERS_HPP */
