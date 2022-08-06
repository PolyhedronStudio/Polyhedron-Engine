################################################################################################
##  Server Source and Header files.                                                           ##
################################################################################################
function(GenerateServerSourceLists)
	LIST( APPEND SRC_SERVER
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Server/Commands.cpp"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Server/Entities.cpp"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Server/SVGame.cpp"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Server/Init.cpp"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Server/Main.cpp"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Server/Models.cpp"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Server/Send.cpp"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Server/User.cpp"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Server/World.cpp"

		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Server/Save.cpp"
	)
	LIST( APPEND HEADERS_SERVER
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Server/Server.h"
	)
endfunction()
GenerateServerSourceLists()