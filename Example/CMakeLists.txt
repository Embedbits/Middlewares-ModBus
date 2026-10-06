# Template version: 0.0.1
# Sets the version of CMakeLists.txt file.
# Never, under any circumstances, change this line. It is used by
# the module generation script to identify the template version.

#======================== Library configuration ===============================#
# Publicly included directories to provide headers include
set( ModBus_PublicIncludeDirs
    
)


# Privately included directories to provide headers include
set( ModBus_PrivateIncludeDirs
    ${CMAKE_CURRENT_SOURCE_DIR}
)


# Source files list for current library generation
set( ModBus_SourceFiles
    ModBus.cpp
    ModBus_Data.cpp
    ModBus_PhyUsart.cpp
    ModBus_Hal.c
)


# List of public header files provided by the current library
set( ModBus_PublicHeaders 
    ModBus_Types.h
    ModBus_Port.h
)


# List of dependent libraries that are publicly used by the current library
set( ModBus_PublicDependentLibs
    ModBusUser_Lib
)


# List of dependent libraries that are privately used by the current library
set( ModBus_PrivateDependentLibs
    
)


#========================== Library generation ================================#
# Create library
add_library(ModBus_Lib STATIC ${ModBus_SourceFiles})


# Include necessary libraries to the target
target_link_libraries( ModBus_Lib
    PUBLIC 
        ${ModBus_PublicDependentLibs} 
    PRIVATE 
        ${ModBus_PrivateDependentLibs}
)


# Set public headers directory path in build folder
set(PUBLIC_HEADERS_DIR ${CMAKE_CURRENT_BINARY_DIR}/PublicHeaders)

# Generates the public headers directory in build folder
file(MAKE_DIRECTORY ${PUBLIC_HEADERS_DIR})

# Process all public headers
foreach(PublicHeader IN LISTS ModBus_PublicHeaders)
    
    # Copy (or configure) only the public header there
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/${PublicHeader} 
        ${PUBLIC_HEADERS_DIR}/${PublicHeader}
        COPYONLY
    )
    
endforeach()


# Set include directories for the created target
target_include_directories( ModBus_Lib
    PUBLIC
        $<BUILD_INTERFACE:${PUBLIC_HEADERS_DIR}>
        $<INSTALL_INTERFACE:include>
        ${ModBus_PublicIncludeDirs}
    PRIVATE
        ${ModBus_PrivateIncludeDirs}
)


# Print status message
message(STATUS "ModBus_Lib library created")

#========================= Documentation configuration ========================#
if(DOXYGEN_AVAILABLE STREQUAL "ON")
    Doxygen_AddPath("${CMAKE_CURRENT_SOURCE_DIR}/")
endif()
