set( MIRALL_VERSION_MAJOR 3 )
set( MIRALL_VERSION_MINOR 4 )
set( MIRALL_VERSION_PATCH 2 )
set( MIRALL_VERSION_YEAR  2022 )
set( MIRALL_SOVERSION 0 )

# Minimum supported server version according to https://docs.nextcloud.com/server/latest/admin_manual/release_schedule.html
set(NEXTCLOUD_SERVER_VERSION_MIN_SUPPORTED_MAJOR 16)
set(NEXTCLOUD_SERVER_VERSION_MIN_SUPPORTED_MINOR 0)
set(NEXTCLOUD_SERVER_VERSION_MIN_SUPPORTED_PATCH 0)

if ( NOT DEFINED MIRALL_VERSION_SUFFIX )
    set( MIRALL_VERSION_SUFFIX "git") #e.g. beta1, beta2, rc1
endif( NOT DEFINED MIRALL_VERSION_SUFFIX )

if( NOT DEFINED MIRALL_VERSION_BUILD )
    set( MIRALL_VERSION_BUILD "0" ) # Integer ID. Generated by the build system
endif( NOT DEFINED MIRALL_VERSION_BUILD )
# Composite defines
# Used e.g. for libraries Keep at x.y.z.
set( MIRALL_VERSION "${MIRALL_VERSION_MAJOR}.${MIRALL_VERSION_MINOR}.${MIRALL_VERSION_PATCH}" )
# Version with Build ID. Used in the installer
set( MIRALL_VERSION_FULL ${MIRALL_VERSION} )
set( MIRALL_VERSION_STRING ${MIRALL_VERSION} )
set( MIRALL_VERSION_FULL "${MIRALL_VERSION_FULL}.${MIRALL_VERSION_BUILD}" )

set( MIRALL_VERSION_STRING "${MIRALL_VERSION}${MIRALL_VERSION_SUFFIX}" )

if( MIRALL_VERSION_BUILD )
    set( MIRALL_VERSION_STRING "${MIRALL_VERSION_STRING} (build ${MIRALL_VERSION_BUILD})" )
endif( MIRALL_VERSION_BUILD )
