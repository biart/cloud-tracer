find_package (PackageHandleStandardArgs)


# Find GLFW includes.
find_path (GLFW3_INCLUDE_DIR
    NAMES
        glfw/glfw3.h
    HINTS
        "${GLFW3_LOCATION}/include"
        "$ENV{GLFW3_LOCATION}/include"
)


set (GLFW3_GLFW_LIBRARY_LOCATION_HINTS
    "${GLFW_LOCATION}/lib"
    "$ENV{GLFW3_LOCATION}/lib"
)


# Find GLFW library.
set (GLFW3_GLFW_LIBRARY_LOCATION_HINTS
    "${GLFW3_LOCATION}/lib"
    "$ENV{GLFW3_LOCATION}/lib"
)

if (MSVC)
    if (MSVC12)
        set (GLFW3_GLFW_LIBRARY_LOCATION_HINTS
            ${GLFW3_GLFW_LIBRARY_LOCATION_HINTS}
            "${GLFW3_LOCATION}/lib-vc2012"
            "$ENV{GLFW3_LOCATION}/lib-vc2012"
        )
    elseif (MSVC13)
        set (GLFW3_GLFW_LIBRARY_LOCATION_HINTS
            ${GLFW3_GLFW_LIBRARY_LOCATION_HINTS}
            "${GLFW3_LOCATION}/lib-vc2013"
            "$ENV{GLFW3_LOCATION}/lib-vc2013"
        )
    elseif (MSVC15)
        set (GLFW3_GLFW_LIBRARY_LOCATION_HINTS
            ${GLFW3_GLFW_LIBRARY_LOCATION_HINTS}
            "${GLFW3_LOCATION}/lib-vc2015"
            "$ENV{GLFW3_LOCATION}/lib-vc2015"
        )
    else ()
        set (GLFW3_GLFW_LIBRARY_LOCATION_HINTS
            ${GLFW3_GLFW_LIBRARY_LOCATION_HINTS}
            "${GLFW3_LOCATION}/lib-vc2015"
            "$ENV{GLFW3_LOCATION}/lib-vc2015"
        )
    endif ()
elseif (MINGW)
    set (GLFW3_GLFW_LIBRARY_LOCATION_HINTS
        ${GLFW3_GLFW_LIBRARY_LOCATION_HINTS}
        "${GLFW3_LOCATION}/lib-mingw-w64"
        "$ENV{GLFW3_LOCATION}/lib-mingw-w64"
    )
endif ()

find_library (GLFW3_GLFW_LIBRARY
	NAMES
		glfw3
	HINTS
		${GLFW3_GLFW_LIBRARY_LOCATION_HINTS}
)


# Handle the QUIETLY and REQUIRED arguments and set GLFW3_FOUND.
find_package_handle_standard_args (GLFW3 DEFAULT_MSG GLFW3_INCLUDE_DIR GLFW3_GLFW_LIBRARY)


# Set the output variables.
if (GLFW3_FOUND)
    set (GLFW3_INCLUDE_DIRS ${GLFW3_INCLUDE_DIR})
    set (GLFW3_LIBRARIES ${GLFW3_GLFW_LIBRARY})
else ()
    set (GLFW3_INCLUDE_DIRS)
    set (GLFW3_LIBRARIES)
endif ()
