################################################################################################
##  Shared Source and Header files.                                                           ##
################################################################################################
function(GenerateSharedSourceLists)
	LIST( APPEND SRC_SHARED
		"${SRC_SHARED_DIR}/Math/Plane.cpp"
		"${SRC_SHARED_DIR}/Math/Vector3.cpp"

		"${SRC_SHARED_DIR}/Math.cpp"
		"${SRC_SHARED_DIR}/Shared.cpp"
	)
	LIST( APPEND HEADERS_SHARED
		# Formats.
		"${SRC_SHARED_DIR}/Formats/Bsp.h" 
		"${SRC_SHARED_DIR}/Formats/Iqm.h" 
		"${SRC_SHARED_DIR}/Formats/Md2.h" 
		"${SRC_SHARED_DIR}/Formats/Md3.h" 
		"${SRC_SHARED_DIR}/Formats/Pak.h" 
		"${SRC_SHARED_DIR}/Formats/Pcx.h" 
		"${SRC_SHARED_DIR}/Formats/Sp2.h" 
		"${SRC_SHARED_DIR}/Formats/Wal.h" 

		# Interfaces
		"${SRC_SHARED_DIR}/Interfaces/IClientGameExports.h"
		"${SRC_SHARED_DIR}/Interfaces/IClientGameImports.h"
		"${SRC_SHARED_DIR}/Interfaces/IServerGameExports.h"
		"${SRC_SHARED_DIR}/Interfaces/IServerGameImports.h"

		# Math lib.
		"${SRC_SHARED_DIR}/Math/Color.h"
		"${SRC_SHARED_DIR}/Math/DualQuaternion.h"
		"${SRC_SHARED_DIR}/Math/Matrix3x3.h"
		"${SRC_SHARED_DIR}/Math/Matrix4x4.h"
		"${SRC_SHARED_DIR}/Math/Plane.h"
		"${SRC_SHARED_DIR}/Math/Rectangle.h"
		"${SRC_SHARED_DIR}/Math/Quaternion.h"
		"${SRC_SHARED_DIR}/Math/Utilities.h"
		"${SRC_SHARED_DIR}/Math/Vector2.h"
		"${SRC_SHARED_DIR}/Math/Vector3.h"
		"${SRC_SHARED_DIR}/Math/Vector4.h"
		"${SRC_SHARED_DIR}/Math/Vector5.h"
	
		# Other shared lib functionalities.
		"${SRC_SHARED_DIR}/CLGame.h"
		"${SRC_SHARED_DIR}/CLTypes.h"
		"${SRC_SHARED_DIR}/CollisionModel.h"
		"${SRC_SHARED_DIR}/Common.h"
		"${SRC_SHARED_DIR}/Config.h"
		"${SRC_SHARED_DIR}/Endian.h" 
		"${SRC_SHARED_DIR}/Entities.h" 
		"${SRC_SHARED_DIR}/EntitySkeleton.h" 
		"${SRC_SHARED_DIR}/Keys.h" 
		"${SRC_SHARED_DIR}/KeyValue.h" 
		"${SRC_SHARED_DIR}/List.h" 
		"${SRC_SHARED_DIR}/Math.h" 
		"${SRC_SHARED_DIR}/Messaging.h" 
		"${SRC_SHARED_DIR}/Platform.h"
		"${SRC_SHARED_DIR}/PlayerMove.h"
		"${SRC_SHARED_DIR}/QString.h"
		"${SRC_SHARED_DIR}/Refresh.h" 
		"${SRC_SHARED_DIR}/Shared.h" 
		"${SRC_SHARED_DIR}/SkeletalModelData.h" 
		"${SRC_SHARED_DIR}/Sound.h" 
		"${SRC_SHARED_DIR}/Strings.h"
		"${SRC_SHARED_DIR}/SVGame.h"
		"${SRC_SHARED_DIR}/TickRate.h"
		"${SRC_SHARED_DIR}/UI.h"
	)
endfunction()
GenerateSharedSourceLists()