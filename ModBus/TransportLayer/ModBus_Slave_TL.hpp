/**
 * \author Mr.Nobody
 * \file ModBus_Slave_TL.hpp
 * \ingroup ModBus
 * \brief ModBus Slave Transport Layer (TL) header file.
 */

#ifndef MODBUS_SLAVE_HPP
#define MODBUS_SLAVE_HPP
/* ============================= INCLUDES =================================== */
#include "ModBus_Core_Types.h"              /* Module types definition        */
#include "ModBus_Phy.hpp"                   /* Physical layer definition      */
#include "ModBus_Crc.hpp"                   /* CRC functionality              */
/* ============================= TYPEDEFS =================================== */

/** \brief Signal of new message status */
typedef enum
{
    MODBUS_APP_MSG_NO_NEW = 0u, /**< No new message received               */
    MODBUS_APP_MSG_NEW_PENDING  /**< New message is pending for processing */
}   modBus_App_MsgStatus_t;


typedef enum
{
    MODBUS_SLAVE_TL_INIT = 0u,  /**< Initialization state of transport layer */
    MODBUS_SLAVE_TL_STANDBY,    /**< StandBy state of transport layer        */
    MODBUS_SLAVE_TL_PROCESSING, /**< Processing message of transport layer   */
    MODBUS_SLAVE_TL_EXECUTING,  /**< Executing message of transport layer    */
    MODBUS_SLAVE_TL_SENDING,    /**< Sending response of transport layer     */
    MODBUS_SLAVE_TL_STATE_CNT   /**< Count of states in transport layer      */
}   modBus_App_TransportLayerState_t;

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ========================= EXPORTED MACROS ================================ */

/* ========================= EXPORTED VARIABLES ============================= */

/* ======================== EXPORTED FUNCTIONS ============================== */

namespace ModBus
{
namespace Slave
{
    class TransportLayer : private Crc
    {
    public:
        TransportLayer( PhysicalLayer &physicalLayerObj );
        ~TransportLayer( void );

        void Activate( void );
        void Deactivate( void );

        void Task( modBus_Time_t taskTime );

        modBus_RequestState_t Set_SlaveAddr( modBus_DeviceAddr_t slaveAddr );
        modBus_DeviceAddr_t   Get_SlaveAddr( void ) const;

        void Set_MessageReady( uint8_t * const msgBuffer, modBus_MsgLength_t messageLen );

        uint8_t *Get_RxMessagePtr( void );

        modBus_App_MsgStatus_t Get_NewMessageStatus( void ) const;
        modBus_MsgLength_t     Get_NewMessageLen( void ) const;

        modBus_StatsCounter_t  Get_StatCounter( void ) const;
        void                   Reset_StatCounter( void );

        modBus_App_TransportLayerState_t Get_ModuleState( void ) const;

        /** Lowest valid slave address (0 is broadcast address) */
        static constexpr modBus_DeviceAddr_t slaveAddrMin = 1u;

        /** Highest valid slave address (248 - 255 are reserved) */
        static constexpr modBus_DeviceAddr_t slaveAddrMax = 247u;

    private:

        /* ====================== SYMBOLIC CONSTANTS ======================== */

        /** Maximum standBy time in ms [0.001s]. Currently 30min. When no frame
         * is received within this time, the physical layer is re-initialized. */
        static constexpr modBus_Time_t modBus_MaxStandbyTime = ( 30u * 60u * 1000u );

        /** Maximal response length without CRC */
        static constexpr modBus_MsgLength_t responseMaxLen = MODBUS_FRAME_MAX_LEN - MODBUS_CRC_LEN;

        /* ========================= TYPEDEFS =============================== */

        /** Define the module specific states */
        typedef enum
        {
            STATE_INIT = 0u,       /**< Initialization state                 */
            STATE_STANDBY,         /**< Standby state                        */
            STATE_FRAME_CHECK,     /**< Received frame check state           */
            STATE_PROCESS_REQUEST, /**< Processing of received message state */
            STATE_TRANSMIT_MESSAGE /**< Transmit processed message state     */
        }   sm_States_t;


        /** Function pointer data-type to be used for the entry, execution and
         * checkLeave routines used by the state machine  */
        typedef void (TransportLayer::*sm_PtrToRoutine_t)( void );


        /** Structure data type that integrates the function pointers to entry,
         *  execution and checkLeave routines of one state. */
        typedef struct
        {
            sm_PtrToRoutine_t entry;      /**< Entry part of state       */
            sm_PtrToRoutine_t execute;    /**< Execute part of state     */
            sm_PtrToRoutine_t checkLeave; /**< Check leave part of state */
            sm_PtrToRoutine_t leave;      /**< Check leave part of state */
        }   sm_Routines_t;


        /** Structure data type that links the function pointers of entry,
         *  execute and checkLeave routines to its specific state */
        typedef struct
        {
            sm_States_t   state;   /**< State ID (name)                    */
            sm_Routines_t routine; /**< Routines list for current state ID */
        }   sm_StateLinkedRoutines_t;


        /** \brief Enumeration used to signal frame status */
        typedef enum
        {
            FRAME_INACTIVE = 0u,      /**< Frame not received                               */
            FRAME_RECEIVED,           /**< Frame received and ready for processing          */
            FRAME_READY_FOR_PROCESS,  /**< Frame is checked and is ready for processing     */
            FRAME_READY_FOR_TRANSMIT, /**< Frame is processed and is ready for transmitting */
            FRAME_TRANSMITTED,        /**< Frame was transmitted                            */
            FRAME_CRC_ERROR,          /**< Frame received with CRC error                    */
            FRAME_ADDR_ERROR          /**< Received address does not correspond             */
        }   frameStatus_t;


        typedef enum
        {
            RUN_INACTIVE = 0u, /**< Module is in inactive state */
            RUN_ACTIVE         /**< Module is in active state   */
        }   runStatus_t;


        /** \brief Structure used for handling received frames */
        typedef struct
        {
            /** Frame processing status */
            frameStatus_t      FrameStatus;

            /** Frame length in bytes */
            modBus_MsgLength_t FrameLength;

            /** Pointer of received frame */
            uint8_t            *FrameData;

        }   frameData_t;


        /* ==================== FORWARD DECLARATIONS ======================== */

        void StateHandler( void );

        void State_Init_Entry( void );
        void State_Init_Execute( void );
        void State_Init_CheckLeave( void );
        void State_Init_Leave( void );

        void State_Standby_Entry( void );
        void State_Standby_Execute( void );
        void State_Standby_CheckLeave( void );
        void State_Standby_Leave( void );

        void State_FrameCheck_Entry( void );
        void State_FrameCheck_Execute( void );
        void State_FrameCheck_CheckLeave( void );
        void State_FrameCheck_Leave( void );

        void State_ProcessRequest_Entry( void );
        void State_ProcessRequest_Execute( void );
        void State_ProcessRequest_CheckLeave( void );
        void State_ProcessRequest_Leave( void );

        void State_TransmitMessage_Entry( void );
        void State_TransmitMessage_Execute( void );
        void State_TransmitMessage_CheckLeave( void );
        void State_TransmitMessage_Leave( void );

        /* =========================== MACROS =============================== */

        /* ======================= LOCAL VARIABLES ========================== */

        /** holds the current state of the state machine */
        sm_States_t sm_ActualState = STATE_INIT;

        /** holds the desired next state */
        sm_States_t sm_NewState = STATE_INIT;

        /** Table of received data */
        frameData_t frameDataStruct = { FRAME_INACTIVE, 0u, nullptr };

        /** Actual value of device address */
        modBus_DeviceAddr_t deviceAddress = slaveAddrMin;

        /** Holds value of state counter in ms [0.001s] */
        modBus_Time_t stateCounter = 0u;

        /** Value of cyclic execution of task handler in ms [0.001s] */
        modBus_Time_t handlerTaskTime = 0u;

        /** Structure with communication statistic counters */
        modBus_StatsCounter_t statsCounter = { 0u, 0u, 0u, 0u };

        /** Actual state of transport layer */
        modBus_App_TransportLayerState_t moduleState = MODBUS_SLAVE_TL_INIT;

        /** Object of physical layer handler */
        PhysicalLayer &physicalLayer;

        /** Module run status */
        runStatus_t runStatus = RUN_INACTIVE;
    };
} /* namespace Slave */
} /* namespace ModBus */

#endif /* MODBUS_SLAVE_HPP */
