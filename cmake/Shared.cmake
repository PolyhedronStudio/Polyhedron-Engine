SET(HEADERS_SHARED

	# Interfaces
	${PROJECT_SOURCE_DIR}/inc/Shared/Interfaces/IClientGameExports.h
	${PROJECT_SOURCE_DIR}/inc/Shared/Interfaces/IClientGameImports.h

	# Math lib.
${PROJECT_SOURCE_DIR}/inc/Shared/Math/Color.h
${PROJECT_SOURCE_DIR}/inc/Shared/Math/Matrix3.h
${PROJECT_SOURCE_DIR}/inc/Shared/Math/Matrix4.h
${PROJECT_SOURCE_DIR}/inc/Shared/Math/Plane.h
${PROJECT_SOURCE_DIR}/inc/Shared/Math/Rectangle.h
${PROJECT_SOURCE_DIR}/inc/Shared/Math/Utilities.h
${PROJECT_SOURCE_DIR}/inc/Shared/Math/Vector2.h
${PROJECT_SOURCE_DIR}/inc/Shared/Math/Vector3.h
${PROJECT_SOURCE_DIR}/inc/Shared/Math/Vector4.h
${PROJECT_SOURCE_DIR}/inc/Shared/Math/Vector5.h
	
	# Other shared lib functionalities.
${PROJECT_SOURCE_DIR}/inc/Shared/CLGame.h
${PROJECT_SOURCE_DIR}/inc/Shared/CLTypes.h
${PROJECT_SOURCE_DIR}/inc/Shared/Collision.h
${PROJECT_SOURCE_DIR}/inc/Shared/Common.h
${PROJECT_SOURCE_DIR}/inc/Shared/Config.h
${PROJECT_SOURCE_DIR}/inc/Shared/Endian.h 
${PROJECT_SOURCE_DIR}/inc/Shared/SVGame.h 
${PROJECT_SOURCE_DIR}/inc/Shared/Keys.h 
${PROJECT_SOURCE_DIR}/inc/Shared/List.h 
${PROJECT_SOURCE_DIR}/inc/Shared/Math.h 
${PROJECT_SOURCE_DIR}/inc/Shared/Messaging.h 
${PROJECT_SOURCE_DIR}/inc/Shared/Platform.h
${PROJECT_SOURCE_DIR}/inc/Shared/Refresh.h 
${PROJECT_SOURCE_DIR}/inc/Shared/QString.h 
${PROJECT_SOURCE_DIR}/inc/Shared/Strings.h
${PROJECT_SOURCE_DIR}/inc/Shared/Shared.h 
${PROJECT_SOURCE_DIR}/inc/Shared/TickRate.h 
${PROJECT_SOURCE_DIR}/inc/Shared/UI.h 
)


SET(SRC_SHARED
	${PROJECT_SOURCE_DIR}/src/Shared/Math/Plane.cpp
	${PROJECT_SOURCE_DIR}/src/Shared/Math/Vector3.cpp

	${PROJECT_SOURCE_DIR}/src/Shared/Math.cpp
	${PROJECT_SOURCE_DIR}/src/Shared/Shared.cpp
)