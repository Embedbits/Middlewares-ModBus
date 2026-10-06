# ModBus RTU stack library (ModBusCore_Lib). Added by module root CMakeLists.txt.

# Include current directory for provide headers include
set( ModBusCore_IncludeDirs
    ${CMAKE_CURRENT_SOURCE_DIR}/TransportLayer
    ${CMAKE_CURRENT_SOURCE_DIR}/AppLayer
    ${CMAKE_CURRENT_SOURCE_DIR}/PhysicalLayer
)


# Set source files for library generation
# All used function files shall be added here.
set( ModBusCore_LibSrcs
    TransportLayer/ModBus_Master_TL.cpp
    TransportLayer/ModBus_Slave_TL.cpp
    TransportLayer/ModBus_Crc.cpp

    AppLayer/ModBus_App.cpp
    AppLayer/ModBus_App_CommDiag.cpp
    AppLayer/ModBus_App_DataAccess.cpp

    AppLayer/ModBus_App_RC.cpp
    AppLayer/ModBus_App_RDI.cpp
    AppLayer/ModBus_App_RHR.cpp
    AppLayer/ModBus_App_RIR.cpp
    AppLayer/ModBus_App_WSC.cpp
    AppLayer/ModBus_App_WSR.cpp
    AppLayer/ModBus_App_RES.cpp
    AppLayer/ModBus_App_DIAG.cpp
    AppLayer/ModBus_App_GCEC.cpp
    AppLayer/ModBus_App_GCEL.cpp
    AppLayer/ModBus_App_WMC.cpp
    AppLayer/ModBus_App_WMR.cpp
    AppLayer/ModBus_App_RSID.cpp
    AppLayer/ModBus_App_RFR.cpp
    AppLayer/ModBus_App_WFR.cpp
    AppLayer/ModBus_App_MWR.cpp
    AppLayer/ModBus_App_RWMR.cpp
    AppLayer/ModBus_App_RFIFO.cpp
    AppLayer/ModBus_App_RDID.cpp
)


# Set public header files for the library
# All header files used by other libraries shall be added here.
set( ModBusCore_PublicHeaders
    AppLayer/ModBus_Core_Types.h
    AppLayer/ModBus_App_Types.hpp
    AppLayer/ModBus_App.hpp
    PhysicalLayer/ModBus_Phy.hpp
    TransportLayer/ModBus_Slave_TL.hpp
    TransportLayer/ModBus_Master_TL.hpp
)


# Set dependent libraries
# All libraries used by current target shall be added here.
set( ModBusCore_DependentLibs
    #REQUIRED_EXAMPLE_Lib
)


# Create library
add_library(ModBusCore_Lib STATIC ${ModBusCore_LibSrcs})


# Specify the public headers for the library
set_target_properties( ModBusCore_Lib PROPERTIES PUBLIC_HEADER "${ModBusCore_PublicHeaders}" )


# Include necessary libraries
target_link_libraries( ModBusCore_Lib
    PRIVATE
    ${ModBusCore_DependentLibs}
)


target_include_directories( ModBusCore_Lib
    PUBLIC
    ${ModBusCore_IncludeDirs}
)


message(STATUS "ModBusCore_Lib library created")
