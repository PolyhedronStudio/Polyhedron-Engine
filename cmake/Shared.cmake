SET(HEADERS_SHARED

	# Interfaces
	${CMAKE_SOURCE_DIR}/inc/Shared/Interfaces/IClientGameExports.h
	${CMAKE_SOURCE_DIR}/inc/Shared/Interfaces/IClientGameImports.h

	# Math lib.
${CMAKE_SOURCE_DIR}/inc/Shared/Math/Color.h
${CMAKE_SOURCE_DIR}/inc/Shared/Math/Matrix3.h
${CMAKE_SOURCE_DIR}/inc/Shared/Math/Matrix4.h
${CMAKE_SOURCE_DIR}/inc/Shared/Math/Plane.h
${CMAKE_SOURCE_DIR}/inc/Shared/Math/Rectangle.h
${CMAKE_SOURCE_DIR}/inc/Shared/Math/Utilities.h
${CMAKE_SOURCE_DIR}/inc/Shared/Math/Vector2.h
${CMAKE_SOURCE_DIR}/inc/Shared/Math/Vector3.h
${CMAKE_SOURCE_DIR}/inc/Shared/Math/Vector4.h
${CMAKE_SOURCE_DIR}/inc/Shared/Math/Vector5.h
	
	# Other shared lib functionalities.
${CMAKE_SOURCE_DIR}/inc/Shared/CLGame.h
${CMAKE_SOURCE_DIR}/inc/Shared/CLTypes.h
${CMAKE_SOURCE_DIR}/inc/Shared/Collision.h
${CMAKE_SOURCE_DIR}/inc/Shared/Common.h
${CMAKE_SOURCE_DIR}/inc/Shared/Config.h
${CMAKE_SOURCE_DIR}/inc/Shared/Endian.h 
${CMAKE_SOURCE_DIR}/inc/Shared/SVGame.h 
${CMAKE_SOURCE_DIR}/inc/Shared/Keys.h 
${CMAKE_SOURCE_DIR}/inc/Shared/List.h 
${CMAKE_SOURCE_DIR}/inc/Shared/Math.h 
${CMAKE_SOURCE_DIR}/inc/Shared/Messaging.h 
${CMAKE_SOURCE_DIR}/inc/Shared/Platform.h
${CMAKE_SOURCE_DIR}/inc/Shared/Refresh.h 
${CMAKE_SOURCE_DIR}/inc/Shared/QString.h 
${CMAKE_SOURCE_DIR}/inc/Shared/Strings.h
${CMAKE_SOURCE_DIR}/inc/Shared/Shared.h 
${CMAKE_SOURCE_DIR}/inc/Shared/TickRate.h 
${CMAKE_SOURCE_DIR}/inc/Shared/UI.h 
)


SET(SRC_SHARED
	${CMAKE_SOURCE_DIR}/src/Shared/Math/Plane.cpp
	${CMAKE_SOURCE_DIR}/src/Shared/Math/Vector3.cpp

	${CMAKE_SOURCE_DIR}/src/Shared/Math.cpp
	${CMAKE_SOURCE_DIR}/src/Shared/Shared.cpp
)