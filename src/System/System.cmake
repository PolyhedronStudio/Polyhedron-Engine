################################################################################################
##  System Source and Header files.                                                           ##
################################################################################################
#function(GenerateSystemSourceLists)
	# Operating System specific sources:
	if (WIN32)
		# Win32 Specific Client Sources.
		list( APPEND SRC_OBJECT_SYSTEM
			"${SRC_SYSTEM_DIR}/Windows/Debug.cpp"
			"${SRC_SYSTEM_DIR}/Windows/Hunk.cpp"
			"${SRC_SYSTEM_DIR}/Windows/System.cpp"
		
			"${SRC_SYSTEM_DIR}/SDL2/Time.cpp"
			"${SRC_SYSTEM_DIR}/SDL2/Sound.cpp"
			"${SRC_SYSTEM_DIR}/SDL2/Video.cpp"

			"${SRC_SYSTEM_DIR}/Resources/polyhedron.rc"
		)
		# Win32 Specific Client Headers
		list( APPEND HEADERS_OBJECT_SYSTEM
			"${SRC_SYSTEM_DIR}/Windows/WGL.h"
			"${SRC_SYSTEM_DIR}/Windows/GLImp.h"
			"${SRC_SYSTEM_DIR}/Windows/WinClient.h"
			"${SRC_SYSTEM_DIR}/Windows/Threads/Threads.h"
		)
	else()
		# Unix Specific Client Sources.
		list( APPEND SRC_OBJECT_SYSTEM
			"${SRC_SYSTEM_DIR}/Unix/Hunk.cpp"
			"${SRC_SYSTEM_DIR}/Unix/System.cpp"
			"${SRC_SYSTEM_DIR}/Unix/TTY.cpp"
		
			"${SRC_SYSTEM_DIR}/SDL2/Time.cpp"
			"${SRC_SYSTEM_DIR}/SDL2/Sound.cpp"
			"${SRC_SYSTEM_DIR}/SDL2/Video.cpp"
		)
		# Unix Specific Client Headers
		#list( APPEND HEADERS_CLIENT "someheader.h")
	endif()
#endfunction()
#GenerateSystemSourceLists()