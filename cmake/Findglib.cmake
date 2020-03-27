if(WIN32 OR APPLE OR NOT UNIX)
	if(NOT glib_FIND_QUIETLY)
		message(STATUS "Platform not supported by glib - skipping search")
	endif()
else()
	set(GLIB_ROOT_DIR "${GLIB_ROOT_DIR}"
		CACHE
		PATH "Directory to search")

	if(CMAKE_SIZEOF_VOID_P MATCHES "8")
		set(_LIBSUFFIXES lib64 lib)
	else()
		set(_LIBSUFFIXES lib)
	endif()

	find_library(GLIB_LIBRARY
		NAMES glib-2.0 glib
		HINTS "${GLIB_ROOT_DIR}"
		PATH_SUFFIXES "${_LIBSUFFIXES}"
	)

	# Might want to look close to the library first for the includes.
	get_filename_component(_libdir "${GLIB_LIBRARY}" PATH)

	find_path(GLIB_INCLUDE_DIR
		NAMES glib.h
		HINTS "${_libdir}/.."
		PATHS "${GLIB_ROOT_DIR}"
		PATH_SUFFIXES glib-2.0/
	)

	# glib-related libraries also use a separate config header, which is in lib dir
	find_path(GLIBCONFIG_INCLUDE_DIR
		NAMES glibconfig.h
		HINTS "${_libdir}/.." 
		PATHS ${GLIB_ROOT_DIR} /usr/lib/x86_64-linux-gnu/glib-2.0/include/
	  PATH_SUFFIXES glib-2.0/include/ "${_LIBSUFFIXES}" 
	)

endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(glib
	DEFAULT_MSG
	GLIB_LIBRARY
	GLIB_INCLUDE_DIR
	GLIBCONFIG_INCLUDE_DIR)

if(GLIB_FOUND)
  # Legacy
  set(GLIB_LIBRARIES "${GLIB_LIBRARY}")
	set(GLIB_INCLUDE_DIRS "${GLIB_INCLUDE_DIR}" "${GLIBCONFIG_INCLUDE_DIR}")
	mark_as_advanced(GLIB_ROOT_DIR)

  # Modern
  add_library(glib INTERFACE IMPORTED)
  set_target_properties(glib PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${GLIB_INCLUDE_DIRS}")
endif()

mark_as_advanced(GLIB_INCLUDE_DIR GLIBCONFIG_INCLUDE_DIR
GLIB_LIBRARY)
   