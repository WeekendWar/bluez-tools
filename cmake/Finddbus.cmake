if(WIN32 OR APPLE OR NOT UNIX)
	if(NOT dbus_FIND_QUIETLY)
		message(STATUS "Platform not supported by dbus - skipping search")
	endif()
else()
	set(DBUS_ROOT_DIR "${DBUS_ROOT_DIR}"
		CACHE
		PATH "Directory to search")

	if(CMAKE_SIZEOF_VOID_P MATCHES "8")
		set(_LIBSUFFIXES lib64 lib)
	else()
		set(_LIBSUFFIXES lib)
	endif()

	find_library(DBUS_LIBRARY
		# NAMES dbus-1.0 dbus-1 dbus
		NAMES dbus-1
		HINTS "${DBUS_ROOT_DIR}"
		PATH_SUFFIXES "${_LIBSUFFIXES}"
	)

	# Might want to look close to the library first for the includes.
	get_filename_component(_libdir "${DBUS_LIBRARY}" PATH)

	find_path(DBUS_INCLUDE_DIR
		NAMES dbus/dbus.h
		HINTS "${_libdir}/.."
		PATHS "${DBUS_ROOT_DIR}"
		PATH_SUFFIXES include/dbus-1.0
	)

	# dbus-related libraries also use a separate config header, which is in lib dir
	find_path(DBUSCONFIG_INCLUDE_DIR
		NAMES dbus/dbus-arch-deps.h
		HINTS "${_libdir}/.."
		PATHS ${DBUS_ROOT_DIR} /usr/lib/x86_64-linux-gnu/dbus-1.0/include/
	  PATH_SUFFIXES dbus-1.0/include/ "${_LIBSUFFIXES}" 
	)

endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(dbus
	DEFAULT_MSG
	DBUS_LIBRARY
	DBUS_INCLUDE_DIR
	DBUSCONFIG_INCLUDE_DIR)

message(STATUS "libblepp DBUSCONFIG_INCLUDE_DIR set to ${DBUSCONFIG_INCLUDE_DIR}")

if(DBUS_FOUND)
  # Legacy
  set(DBUS_LIBRARIES "${DBUS_LIBRARY}")
	set(DBUS_INCLUDE_DIRS "${DBUS_INCLUDE_DIR}" "${DBUSCONFIG_INCLUDE_DIR}")
	mark_as_advanced(DBUS_ROOT_DIR)

  # Modern
  add_library(dbus INTERFACE IMPORTED)
  set_target_properties(dbus PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${DBUS_INCLUDE_DIRS}")
endif()

mark_as_advanced(DBUS_INCLUDE_DIR DBUSCONFIG_INCLUDE_DIR
DBUS_LIBRARY)
   