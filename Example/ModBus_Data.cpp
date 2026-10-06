/**
 * \author Mr.Nobody
 * \file ModBus_Data.cpp
 * \ingroup ModBus
 * \brief Application data accessed through ModBus.
 *
 * In real application the callbacks read / write application modules (eg.
 * temperature controller). Here the data are simulated in RAM.
 */

/* ============================= INCLUDES =================================== */
#include "ModBus_Data.hpp"                  /* Self include                   */
/* ========================= SYMBOLIC CONSTANTS ============================= */

/** Highest supported operation mode */
static constexpr uint16_t MODBUS_DATA_MODE_MAX = 2u;

/** File number of user data file */
static constexpr uint16_t MODBUS_DATA_FILE_NUMBER = 1u;

/** Count of records in user data file */
static constexpr uint16_t MODBUS_DATA_FILE_RECORDS = 16u;

/** Count of events in event FIFO */
static constexpr uint16_t MODBUS_DATA_FIFO_SIZE = 8u;

/* ========================== LOCAL VARIABLES =============================== */

static uint16_t               modBus_Data_SetPoint    = 2500u;   /* 25.00 °C */
static uint16_t               modBus_Data_Mode        = 0u;
static uint16_t               modBus_Data_ConfigFlags = 0u;
static uint16_t               modBus_Data_Temperature = 2350u;   /* 23.50 °C */
static uint32_t               modBus_Data_UptimeMs    = 0u;
static modBus_CoilsRetValue_t modBus_Data_Relay       = MODBUS_COIL_INACTIVE;
static modBus_CoilsRetValue_t modBus_Data_Led         = MODBUS_COIL_INACTIVE;
static uint16_t               modBus_Data_File[ MODBUS_DATA_FILE_RECORDS ] = { 0u };
static uint16_t               modBus_Data_Fifo[ MODBUS_DATA_FIFO_SIZE ]    = { 0x0001u, 0x0002u };
static uint16_t               modBus_Data_FifoCount   = 2u;

static const char             modBus_Data_VendorName[]  = "Eifell";
static const char             modBus_Data_ProductCode[] = "EMBI-MB-EXAMPLE";
static const char             modBus_Data_Revision[]    = "V1.0";

/* ========================== LOCAL FUNCTIONS =============================== */

/** Returns constant string as RDID object */
static modBus_RequestState_t ModBus_Data_Get_String( const char *string, uint8_t length,
                                                       const uint8_t **objData, uint8_t *objLength )
{
    *objData   = (const uint8_t *)string;
    *objLength = length;

    return ( MODBUS_REQUEST_OK );
}

/* ======================== EXPORTED FUNCTIONS ============================== */

/* --------------------------- Holding registers ---------------------------- */

modBus_RequestState_t ModBus_Data_Get_SetPoint( modBus_RegistersValue_t *value )
{
    *value = modBus_Data_SetPoint;

    return ( MODBUS_REQUEST_OK );
}


modBus_RequestState_t ModBus_Data_Set_SetPoint( modBus_RegistersValue_t *value )
{
    modBus_Data_SetPoint = *value;

    return ( MODBUS_REQUEST_OK );
}


modBus_RequestState_t ModBus_Data_Get_Mode( modBus_RegistersValue_t *value )
{
    *value = modBus_Data_Mode;

    return ( MODBUS_REQUEST_OK );
}


/** Unsupported mode is rejected - master gets exception 0x04 */
modBus_RequestState_t ModBus_Data_Set_Mode( modBus_RegistersValue_t *value )
{
    modBus_RequestState_t retState = MODBUS_REQUEST_ERROR;

    if( MODBUS_DATA_MODE_MAX >= *value )
    {
        modBus_Data_Mode = *value;
        retState     = MODBUS_REQUEST_OK;
    }
    else
    {
        /* Unsupported mode */
    }

    return ( retState );
}


modBus_RequestState_t ModBus_Data_Get_ConfigFlags( modBus_RegistersValue_t *value )
{
    *value = modBus_Data_ConfigFlags;

    return ( MODBUS_REQUEST_OK );
}


modBus_RequestState_t ModBus_Data_Set_ConfigFlags( modBus_RegistersValue_t *value )
{
    modBus_Data_ConfigFlags = *value;

    return ( MODBUS_REQUEST_OK );
}

/* ---------------------------- Input registers ----------------------------- */

modBus_RequestState_t ModBus_Data_Get_Temperature( modBus_RegistersValue_t *value )
{
    *value = modBus_Data_Temperature;

    return ( MODBUS_REQUEST_OK );
}


modBus_RequestState_t ModBus_Data_Get_Uptime( modBus_RegistersValue_t *value )
{
    *value = (modBus_RegistersValue_t)( modBus_Data_UptimeMs / 1000u );

    return ( MODBUS_REQUEST_OK );
}

/* --------------------------------- Coils ---------------------------------- */

modBus_RequestState_t ModBus_Data_Get_Relay( modBus_CoilsRetValue_t *value )
{
    *value = modBus_Data_Relay;

    return ( MODBUS_REQUEST_OK );
}


modBus_RequestState_t ModBus_Data_Set_Relay( modBus_CoilsRetValue_t *value )
{
    modBus_Data_Relay = *value;

    return ( MODBUS_REQUEST_OK );
}


modBus_RequestState_t ModBus_Data_Get_Led( modBus_CoilsRetValue_t *value )
{
    *value = modBus_Data_Led;

    return ( MODBUS_REQUEST_OK );
}


modBus_RequestState_t ModBus_Data_Set_Led( modBus_CoilsRetValue_t *value )
{
    modBus_Data_Led = *value;

    return ( MODBUS_REQUEST_OK );
}

/* ---------------------------- Discrete inputs ----------------------------- */

modBus_RequestState_t ModBus_Data_Get_Button( modBus_CoilsRetValue_t *value )
{
    *value = MODBUS_COIL_INACTIVE;

    return ( MODBUS_REQUEST_OK );
}


modBus_RequestState_t ModBus_Data_Get_DoorSwitch( modBus_CoilsRetValue_t *value )
{
    *value = MODBUS_COIL_ACTIVE;

    return ( MODBUS_REQUEST_OK );
}

/* ------------------- Exception status, FIFO, file records ----------------- */

/** Bit 0 - relay active, bit 1 - temperature above set-point */
modBus_RequestState_t ModBus_Data_Get_ExceptionStatus( uint8_t *exceptionStatus )
{
    *exceptionStatus = (uint8_t)( ( ( MODBUS_COIL_ACTIVE == modBus_Data_Relay ) ? 0x01u : 0x00u ) |
                                  ( ( modBus_Data_Temperature > modBus_Data_SetPoint ) ? 0x02u : 0x00u ) );

    return ( MODBUS_REQUEST_OK );
}


modBus_RequestState_t ModBus_Data_Get_EventFifo( uint16_t fifoAddress, modBus_RegistersValue_t *values, uint16_t *fifoCount )
{
    modBus_RequestState_t retState = MODBUS_REQUEST_ERROR;

    if( 0u == fifoAddress )
    {
        for( uint16_t valueIndex = 0u; modBus_Data_FifoCount > valueIndex; valueIndex++ )
        {
            values[ valueIndex ] = modBus_Data_Fifo[ valueIndex ];
        }

        *fifoCount = modBus_Data_FifoCount;
        retState   = MODBUS_REQUEST_OK;
    }
    else
    {
        /* Only FIFO at address 0 exists */
    }

    return ( retState );
}


modBus_RequestState_t ModBus_Data_Read_FileRecords( uint16_t fileNumber, uint16_t recordNumber,
                                                      modBus_RegistersValue_t *values, uint16_t recordLength )
{
    modBus_RequestState_t retState = MODBUS_REQUEST_ERROR;

    if( ( MODBUS_DATA_FILE_NUMBER  == fileNumber                                  ) &&
        ( MODBUS_DATA_FILE_RECORDS >= ( (uint32_t)recordNumber + recordLength ) )    )
    {
        for( uint16_t recordIndex = 0u; recordLength > recordIndex; recordIndex++ )
        {
            values[ recordIndex ] = modBus_Data_File[ recordNumber + recordIndex ];
        }

        retState = MODBUS_REQUEST_OK;
    }
    else
    {
        /* File or records do not exist */
    }

    return ( retState );
}


modBus_RequestState_t ModBus_Data_Write_FileRecords( uint16_t fileNumber, uint16_t recordNumber,
                                                       modBus_RegistersValue_t *values, uint16_t recordLength )
{
    modBus_RequestState_t retState = MODBUS_REQUEST_ERROR;

    if( ( MODBUS_DATA_FILE_NUMBER  == fileNumber                                  ) &&
        ( MODBUS_DATA_FILE_RECORDS >= ( (uint32_t)recordNumber + recordLength ) )    )
    {
        for( uint16_t recordIndex = 0u; recordLength > recordIndex; recordIndex++ )
        {
            modBus_Data_File[ recordNumber + recordIndex ] = values[ recordIndex ];
        }

        retState = MODBUS_REQUEST_OK;
    }
    else
    {
        /* File or records do not exist */
    }

    return ( retState );
}

/* ----------------------------- Identification ----------------------------- */

modBus_App_RunStatus_t ModBus_Data_Get_RunStatus( void )
{
    return ( MODBUS_APP_RUN_ACTIVE );
}


modBus_RequestState_t ModBus_Data_Get_VendorName( const uint8_t **objData, uint8_t *objLength )
{
    return ( ModBus_Data_Get_String( modBus_Data_VendorName, sizeof( modBus_Data_VendorName ) - 1u, objData, objLength ) );
}


modBus_RequestState_t ModBus_Data_Get_ProductCode( const uint8_t **objData, uint8_t *objLength )
{
    return ( ModBus_Data_Get_String( modBus_Data_ProductCode, sizeof( modBus_Data_ProductCode ) - 1u, objData, objLength ) );
}


modBus_RequestState_t ModBus_Data_Get_Revision( const uint8_t **objData, uint8_t *objLength )
{
    return ( ModBus_Data_Get_String( modBus_Data_Revision, sizeof( modBus_Data_Revision ) - 1u, objData, objLength ) );
}

/* =============================== TASKS ==================================== */

/**
 * \brief Simulation of application - uptime counter
 *
 * \param taskTime [in]: Time between two executions in ms [0.001s]
 */
void ModBus_Data_Task( modBus_Time_t taskTime )
{
    modBus_Data_UptimeMs = modBus_Data_UptimeMs + taskTime;
}
