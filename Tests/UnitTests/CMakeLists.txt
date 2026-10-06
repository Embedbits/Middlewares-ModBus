#======================== Unit tests configuration ============================#
# Unit tests of ModBus middleware. Registered in module CMakeLists.txt by
# UnitTesting_AddPath() and created by UnitTesting_Generate() in "UnitTest"
# build (see EmBi_Platform/CMake/UnitTesting/README.md).
#
# Test files are C++ (module is C++), Unity test runner is compiled as C++
# too (see UnitTesting_Add_Test).

get_filename_component(MODBUS_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)

set(MODBUS_DIR         "${MODBUS_ROOT_DIR}/ModBus")
set(MODBUS_EXAMPLE_DIR "${MODBUS_ROOT_DIR}/Example")

set(MODBUS_UT_INCLUDE_DIRS
    ${MODBUS_DIR}/AppLayer
    ${MODBUS_DIR}/TransportLayer
    ${MODBUS_DIR}/PhysicalLayer
)

set(MODBUS_UT_TL_SOURCES
    ${MODBUS_DIR}/TransportLayer/ModBus_Crc.cpp
    ${MODBUS_DIR}/TransportLayer/ModBus_Slave_TL.cpp
    ${MODBUS_DIR}/TransportLayer/ModBus_Master_TL.cpp
)

set(MODBUS_UT_APP_SOURCES
    ${MODBUS_UT_TL_SOURCES}
    ${MODBUS_DIR}/AppLayer/ModBus_App.cpp
    ${MODBUS_DIR}/AppLayer/ModBus_App_CommDiag.cpp
    ${MODBUS_DIR}/AppLayer/ModBus_App_DataAccess.cpp
)

set(MODBUS_UT_HELPER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/UtModBus_FakePhy.cpp
)

UnitTesting_Add_Test(
    NAME            ModBus_Crc
    TEST_SOURCE     Test_ModBus_Crc.cpp
    UUT_SOURCES     ${MODBUS_DIR}/TransportLayer/ModBus_Crc.cpp
    INCLUDE_DIRS    ${MODBUS_UT_INCLUDE_DIRS}
)

UnitTesting_Add_Test(
    NAME            ModBus_SlaveTL
    TEST_SOURCE     Test_ModBus_SlaveTL.cpp
    UUT_SOURCES     ${MODBUS_UT_TL_SOURCES}
    SOURCES         ${MODBUS_UT_HELPER_SOURCES}
    INCLUDE_DIRS    ${MODBUS_UT_INCLUDE_DIRS}
)

UnitTesting_Add_Test(
    NAME            ModBus_MasterTL
    TEST_SOURCE     Test_ModBus_MasterTL.cpp
    UUT_SOURCES     ${MODBUS_UT_TL_SOURCES}
    SOURCES         ${MODBUS_UT_HELPER_SOURCES}
    INCLUDE_DIRS    ${MODBUS_UT_INCLUDE_DIRS}
)

UnitTesting_Add_Test(
    NAME            ModBus_AppCore
    TEST_SOURCE     Test_ModBus_AppCore.cpp
    UUT_SOURCES     ${MODBUS_UT_APP_SOURCES}
                    ${MODBUS_DIR}/AppLayer/ModBus_App_RHR.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_WSR.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_DIAG.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_GCEC.cpp
    SOURCES         ${MODBUS_UT_HELPER_SOURCES}
    INCLUDE_DIRS    ${MODBUS_UT_INCLUDE_DIRS}
)

UnitTesting_Add_Test(
    NAME            ModBus_AppBits
    TEST_SOURCE     Test_ModBus_AppBits.cpp
    UUT_SOURCES     ${MODBUS_UT_APP_SOURCES}
                    ${MODBUS_DIR}/AppLayer/ModBus_App_RC.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_RDI.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_WSC.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_WMC.cpp
    SOURCES         ${MODBUS_UT_HELPER_SOURCES}
    INCLUDE_DIRS    ${MODBUS_UT_INCLUDE_DIRS}
)

UnitTesting_Add_Test(
    NAME            ModBus_AppRegisters
    TEST_SOURCE     Test_ModBus_AppRegisters.cpp
    UUT_SOURCES     ${MODBUS_UT_APP_SOURCES}
                    ${MODBUS_DIR}/AppLayer/ModBus_App_RHR.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_RIR.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_WSR.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_WMR.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_MWR.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_RWMR.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_RFIFO.cpp
    SOURCES         ${MODBUS_UT_HELPER_SOURCES}
    INCLUDE_DIRS    ${MODBUS_UT_INCLUDE_DIRS}
)

UnitTesting_Add_Test(
    NAME            ModBus_AppFile
    TEST_SOURCE     Test_ModBus_AppFile.cpp
    UUT_SOURCES     ${MODBUS_UT_APP_SOURCES}
                    ${MODBUS_DIR}/AppLayer/ModBus_App_RFR.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_WFR.cpp
    SOURCES         ${MODBUS_UT_HELPER_SOURCES}
    INCLUDE_DIRS    ${MODBUS_UT_INCLUDE_DIRS}
)

UnitTesting_Add_Test(
    NAME            ModBus_AppDiag
    TEST_SOURCE     Test_ModBus_AppDiag.cpp
    UUT_SOURCES     ${MODBUS_UT_APP_SOURCES}
                    ${MODBUS_DIR}/AppLayer/ModBus_App_RES.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_DIAG.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_GCEC.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_GCEL.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_RSID.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_RDID.cpp
    SOURCES         ${MODBUS_UT_HELPER_SOURCES}
    INCLUDE_DIRS    ${MODBUS_UT_INCLUDE_DIRS}
)

# Example (project handler module template) - built with fake HAL instead of
# ModBus_Hal.c, complete slave stack is exercised through ModBus_Port.h
UnitTesting_Add_Test(
    NAME            ModBus_Example
    TEST_SOURCE     Test_ModBus_Example.cpp
    UUT_SOURCES     ${MODBUS_EXAMPLE_DIR}/ModBus.cpp
                    ${MODBUS_EXAMPLE_DIR}/ModBus_Data.cpp
                    ${MODBUS_EXAMPLE_DIR}/ModBus_PhyUsart.cpp
    SOURCES         ${MODBUS_UT_APP_SOURCES}
                    ${MODBUS_DIR}/AppLayer/ModBus_App_RC.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_RDI.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_RHR.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_RIR.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_WSC.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_WSR.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_RES.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_DIAG.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_GCEC.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_GCEL.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_WMC.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_WMR.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_RSID.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_RFR.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_WFR.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_MWR.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_RWMR.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_RFIFO.cpp
                    ${MODBUS_DIR}/AppLayer/ModBus_App_RDID.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/UtModBus_FakePhy.cpp
    INCLUDE_DIRS    ${MODBUS_EXAMPLE_DIR}
                    ${MODBUS_UT_INCLUDE_DIRS}
)
