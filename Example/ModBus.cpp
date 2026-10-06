/**
 * \author Mr.Nobody
 * \file ModBus.cpp
 * \ingroup ModBus
 * \brief ModBus handler module - ModBus RTU slave integration.
 *
 * The module is created from example of ModBus middleware
 * (Middlewares/ThirdParty/ModBus/Example) at first project configuration and
 * it is never overwritten afterwards - adapt it to the application.
 *
 * Composition of the whole slave stack:
 * 1. Physical layer (USART over ModBus_Hal.h, implemented by application).
 * 2. Slave transport layer (framing, CRC, slave address).
 * 3. Communication diagnostics (counters, event log, Listen Only Mode).
 * 4. Function handlers configured by application data callbacks.
 * 5. Application core dispatching requests to handlers.
 *
 * All objects are static, no dynamic memory is used. Handlers which are not
 * needed by the application can be simply omitted from the handlers array.
 */

/* ============================= INCLUDES =================================== */
#include "ModBus.h"                         /* Self include                   */
#include "ModBus_Port.h"                    /* Own port file include          */
#include "ModBus_Types.h"                   /* Module types definitions       */
#include "ModBus_Data.hpp"                  /* Application data callbacks     */
#include "ModBus_PhyUsart.hpp"              /* USART physical layer           */
#include "ModBus_Slave_TL.hpp"              /* Slave transport layer          */
#include "ModBus_App.hpp"                   /* Application layer core         */
#include "ModBus_App_CommDiag.hpp"          /* Communication diagnostics      */
#include "ModBus_App_RC.hpp"                /* 0x01 Read Coils                */
#include "ModBus_App_RDI.hpp"               /* 0x02 Read Discrete Inputs      */
#include "ModBus_App_RHR.hpp"               /* 0x03 Read Holding Registers    */
#include "ModBus_App_RIR.hpp"               /* 0x04 Read Input Registers      */
#include "ModBus_App_WSC.hpp"               /* 0x05 Write Single Coil         */
#include "ModBus_App_WSR.hpp"               /* 0x06 Write Single Register     */
#include "ModBus_App_RES.hpp"               /* 0x07 Read Exception Status     */
#include "ModBus_App_DIAG.hpp"              /* 0x08 Diagnostics               */
#include "ModBus_App_GCEC.hpp"              /* 0x0B Get Comm Event Counter    */
#include "ModBus_App_GCEL.hpp"              /* 0x0C Get Comm Event Log        */
#include "ModBus_App_WMC.hpp"               /* 0x0F Write Multiple Coils      */
#include "ModBus_App_WMR.hpp"               /* 0x10 Write Multiple Registers  */
#include "ModBus_App_RSID.hpp"              /* 0x11 Report Slave ID           */
#include "ModBus_App_RFR.hpp"               /* 0x14 Read File Record          */
#include "ModBus_App_WFR.hpp"               /* 0x15 Write File Record         */
#include "ModBus_App_MWR.hpp"               /* 0x16 Mask Write Register       */
#include "ModBus_App_RWMR.hpp"              /* 0x17 Read/Write Multiple Regs  */
#include "ModBus_App_RFIFO.hpp"             /* 0x18 Read FIFO Queue           */
#include "ModBus_App_RDID.hpp"              /* 0x2B Read Device Identification*/
/* ========================= SYMBOLIC CONSTANTS ============================= */

/** Value of major version of SW module */
#define MODBUS_MAJOR_VERSION                ( 1u )

/** Value of minor version of SW module */
#define MODBUS_MINOR_VERSION                ( 0u )

/** Value of patch version of SW module */
#define MODBUS_PATCH_VERSION                ( 0u )

/** Count of items in array */
#define MODBUS_ITEMS_COUNT( array )         ( sizeof( array ) / sizeof( ( array )[ 0 ] ) )

/* ========================== LOCAL VARIABLES =============================== */

/* ------------------------- Data configuration ----------------------------- */

/** Holding registers, array index is register address */
static const modBus_RegistersConfigStruct_t modBus_HoldingRegisters[] =
{
    { 0u, MODBUS_SERVICE_SUPPORTED, ModBus_Data_Get_SetPoint,    ModBus_Data_Set_SetPoint    },
    { 1u, MODBUS_SERVICE_SUPPORTED, ModBus_Data_Get_Mode,        ModBus_Data_Set_Mode        },
    { 2u, MODBUS_SERVICE_SUPPORTED, ModBus_Data_Get_ConfigFlags, ModBus_Data_Set_ConfigFlags },
};

/** Input registers (read only - setter not assigned) */
static const modBus_RegistersConfigStruct_t modBus_InputRegisters[] =
{
    { 0u, MODBUS_SERVICE_SUPPORTED, ModBus_Data_Get_Temperature, nullptr },
    { 1u, MODBUS_SERVICE_SUPPORTED, ModBus_Data_Get_Uptime,      nullptr },
};

/** Coils */
static const modBus_CoilsConfigStruct_t modBus_Coils[] =
{
    { 0u, MODBUS_SERVICE_SUPPORTED, ModBus_Data_Get_Relay, ModBus_Data_Set_Relay },
    { 1u, MODBUS_SERVICE_SUPPORTED, ModBus_Data_Get_Led,   ModBus_Data_Set_Led   },
};

/** Discrete inputs */
static const modBus_DiscreteIoConfigStruct_t modBus_DiscreteInputs[] =
{
    { 0u, MODBUS_SERVICE_SUPPORTED, ModBus_Data_Get_Button,     nullptr },
    { 1u, MODBUS_SERVICE_SUPPORTED, ModBus_Data_Get_DoorSwitch, nullptr },
};

/** Report Slave ID configuration */
static const uint8_t           modBus_SlaveIdData[] = { 'E', 'M', 'B', 'I', '-', 'M', 'B' };

static const modBus_App_RSID_t modBus_SlaveIdConfig =
{
    0x42u,                          /* SlaveId         */
    ModBus_Data_Get_RunStatus,    /* RunStatusGetter */
    sizeof( modBus_SlaveIdData ),   /* AddData_Len     */
    modBus_SlaveIdData              /* AddData         */
};

/** Device identification objects (basic category is mandatory) */
static const modBus_App_RDID_t modBus_DeviceIdObjects[] =
{
    { MODBUS_RDID_ID_BASIC_VENDOR_NAME,  ModBus_Data_Get_VendorName  },
    { MODBUS_RDID_ID_BASIC_PRODUCT_CODE, ModBus_Data_Get_ProductCode },
    { MODBUS_RDID_ID_BASIC_SW_VERSION,   ModBus_Data_Get_Revision    },
};

/* ----------------------------- Stack objects ------------------------------ */

static ModBus::UsartPhysicalLayer          modBus_PhysicalLayer;

static ModBus::Slave::TransportLayer       modBus_TransportLayer( modBus_PhysicalLayer );

static ModBus::AppLayer::CommDiagnostics   modBus_Diagnostics( modBus_TransportLayer );

static ModBus::AppLayer::RC    modBus_Rc   ( modBus_Coils,            MODBUS_ITEMS_COUNT( modBus_Coils ) );
static ModBus::AppLayer::RDI   modBus_Rdi  ( modBus_DiscreteInputs,   MODBUS_ITEMS_COUNT( modBus_DiscreteInputs ) );
static ModBus::AppLayer::RHR   modBus_Rhr  ( modBus_HoldingRegisters, MODBUS_ITEMS_COUNT( modBus_HoldingRegisters ) );
static ModBus::AppLayer::RIR   modBus_Rir  ( modBus_InputRegisters,   MODBUS_ITEMS_COUNT( modBus_InputRegisters ) );
static ModBus::AppLayer::WSC   modBus_Wsc  ( modBus_Coils,            MODBUS_ITEMS_COUNT( modBus_Coils ) );
static ModBus::AppLayer::WSR   modBus_Wsr  ( modBus_HoldingRegisters, MODBUS_ITEMS_COUNT( modBus_HoldingRegisters ) );
static ModBus::AppLayer::RES   modBus_Res  ( ModBus_Data_Get_ExceptionStatus );
static ModBus::AppLayer::DIAG  modBus_Diag ( modBus_Diagnostics );
static ModBus::AppLayer::GCEC  modBus_Gcec ( modBus_Diagnostics );
static ModBus::AppLayer::GCEL  modBus_Gcel ( modBus_Diagnostics );
static ModBus::AppLayer::WMC   modBus_Wmc  ( modBus_Coils,            MODBUS_ITEMS_COUNT( modBus_Coils ) );
static ModBus::AppLayer::WMR   modBus_Wmr  ( modBus_HoldingRegisters, MODBUS_ITEMS_COUNT( modBus_HoldingRegisters ) );
static ModBus::AppLayer::RSID  modBus_Rsid ( modBus_SlaveIdConfig );
static ModBus::AppLayer::RFR   modBus_Rfr  ( ModBus_Data_Read_FileRecords );
static ModBus::AppLayer::WFR   modBus_Wfr  ( ModBus_Data_Write_FileRecords );
static ModBus::AppLayer::MWR   modBus_Mwr  ( modBus_HoldingRegisters, MODBUS_ITEMS_COUNT( modBus_HoldingRegisters ) );
static ModBus::AppLayer::RWMR  modBus_Rwmr ( modBus_HoldingRegisters, MODBUS_ITEMS_COUNT( modBus_HoldingRegisters ) );
static ModBus::AppLayer::RFIFO modBus_Rfifo( ModBus_Data_Get_EventFifo );
static ModBus::AppLayer::RDID  modBus_Rdid ( modBus_DeviceIdObjects,  MODBUS_ITEMS_COUNT( modBus_DeviceIdObjects ) );

/** Supported functions. Function code not handled here is answered by exception 0x01. */
static ModBus::FunctionsHandler * const modBus_FunctionHandlers[] =
{
    &modBus_Rc,   &modBus_Rdi,  &modBus_Rhr,  &modBus_Rir,  &modBus_Wsc,
    &modBus_Wsr,  &modBus_Res,  &modBus_Diag, &modBus_Gcec, &modBus_Gcel,
    &modBus_Wmc,  &modBus_Wmr,  &modBus_Rsid, &modBus_Rfr,  &modBus_Wfr,
    &modBus_Mwr,  &modBus_Rwmr, &modBus_Rfifo, &modBus_Rdid
};

/** Application layer core - activates transport layer */
static ModBus::AppLayer::Slave::AppCore    modBus_AppCore( modBus_TransportLayer,
                                                           modBus_FunctionHandlers,
                                                           MODBUS_ITEMS_COUNT( modBus_FunctionHandlers ),
                                                           &modBus_Diagnostics );

/* ======================== EXPORTED FUNCTIONS ============================== */

/**
 * \brief Returns module SW version
 *
 * \return Module SW version
 */
modBus_ModuleVersion_t ModBus_Get_ModuleVersion( void )
{
    modBus_ModuleVersion_t retVersion;

    retVersion.Major = MODBUS_MAJOR_VERSION;
    retVersion.Minor = MODBUS_MINOR_VERSION;
    retVersion.Patch = MODBUS_PATCH_VERSION;

    return ( retVersion );
}


/**
 * \brief Initializes module ModBus
 *
 * Configures slave address and bus configuration (ModBus.h). Configuration is
 * applied when the transport layer initializes physical layer (first task).
 * Transport layer is activated by application core constructor.
 */
void ModBus_Init( void )
{
    modBus_PhysicalLayer.Set_Baudrate( MODBUS_BUS_BAUDRATE );
    modBus_PhysicalLayer.Set_Parity( MODBUS_BUS_PARITY );
    modBus_PhysicalLayer.Set_StopBits( MODBUS_BUS_STOPBITS );

    (void)modBus_TransportLayer.Set_SlaveAddr( MODBUS_SLAVE_ADDRESS );

    modBus_TransportLayer.Activate();
}


/**
 * \brief Deinitializes module ModBus
 *
 * Communication is stopped, transport layer returns to initialization state
 * in the next task.
 */
void ModBus_Deinit( void )
{
    modBus_TransportLayer.Deactivate();
}

/* =============================== TASKS ==================================== */

/**
 * \brief Main task of module ModBus
 *
 * Shall be called every \ref MODBUS_TASK_PERIOD_MS.
 */
void ModBus_Task( void )
{
    ModBus_Data_Task( MODBUS_TASK_PERIOD_MS );

    modBus_AppCore.Task( MODBUS_TASK_PERIOD_MS );
}
