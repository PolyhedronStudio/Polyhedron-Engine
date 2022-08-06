################################################################################################
##  Shared Source and Header files.                                                           ##
################################################################################################
function(GenerateCommonSourceLists)
LIST( APPEND SRC_COMMON
	"${SRC_COMMON_DIR}/Bsp.cpp"
	"${SRC_COMMON_DIR}/Cmd.cpp"
	"${SRC_COMMON_DIR}/CollisionModel.cpp"
	"${SRC_COMMON_DIR}/Common.cpp"
	"${SRC_COMMON_DIR}/CVar.cpp"
	"${SRC_COMMON_DIR}/EntitySkeleton.cpp"
	"${SRC_COMMON_DIR}/Error.cpp"
	"${SRC_COMMON_DIR}/Field.cpp"
	"${SRC_COMMON_DIR}/Fifo.cpp"
	"${SRC_COMMON_DIR}/Files.cpp"
	"${SRC_COMMON_DIR}/Huffman.cpp"
	"${SRC_COMMON_DIR}/MDFour.cpp"
	"${SRC_COMMON_DIR}/Messaging.cpp"
	"${SRC_COMMON_DIR}/Prompt.cpp"
	"${SRC_COMMON_DIR}/SizeBuffer.cpp"
	"${SRC_COMMON_DIR}/SkeletalModelData.cpp"
	"${SRC_COMMON_DIR}/TemporaryBoneCache.cpp"
	#"${SRC_COMMON_DIR}/Tests.cpp"
	"${SRC_COMMON_DIR}/Utilities.cpp"
	"${SRC_COMMON_DIR}/Zone.cpp"

	"${SRC_COMMON_DIR}/Models/Iqm.cpp"

	"${SRC_COMMON_DIR}/Hashes/Crc32.cpp"

	"${SRC_COMMON_DIR}/Messaging/MessageReadWrite.cpp"
	"${SRC_COMMON_DIR}/Messaging/ParseDeltaClientMoveCommand.cpp"
	"${SRC_COMMON_DIR}/Messaging/ParseDeltaEntityState.cpp"
	"${SRC_COMMON_DIR}/Messaging/ParseDeltaPlayerState.cpp"
	"${SRC_COMMON_DIR}/Messaging/WriteDeltaClientMoveCommand.cpp"
	"${SRC_COMMON_DIR}/Messaging/WriteDeltaEntityState.cpp"
	"${SRC_COMMON_DIR}/Messaging/WriteDeltaPlayerState.cpp"

	"${SRC_COMMON_DIR}/Net/NetChan.cpp"
	"${SRC_COMMON_DIR}/Net/Net.cpp"
)
LIST( APPEND HEADERS_COMMON
	"${SRC_COMMON_DIR}/Bsp.h"
	"${SRC_COMMON_DIR}/Cmd.h"
	"${SRC_COMMON_DIR}/CollisionModel.h"
	"${SRC_COMMON_DIR}/Common.h"
	"${SRC_COMMON_DIR}/CVar.h"
	"${SRC_COMMON_DIR}/EntitySkeleton.h"
	"${SRC_COMMON_DIR}/Error.h"
	"${SRC_COMMON_DIR}/Field.h"
	"${SRC_COMMON_DIR}/Fifo.h"
	"${SRC_COMMON_DIR}/Files.h"
	"${SRC_COMMON_DIR}/HalfFloat.h"
	"${SRC_COMMON_DIR}/Huffman.h"
	"${SRC_COMMON_DIR}/MDFour.h"
	"${SRC_COMMON_DIR}/Messaging.h"
	"${SRC_COMMON_DIR}/PlayerMove.h"
	"${SRC_COMMON_DIR}/Prompt.h"
	"${SRC_COMMON_DIR}/Protocol.h"
	"${SRC_COMMON_DIR}/SizeBuffer.h"
	"${SRC_COMMON_DIR}/SkeletalModelData.h"
	"${SRC_COMMON_DIR}/TemporaryBoneCache.h"
	#"${SRC_COMMON_DIR}/Tests.h"
	"${SRC_COMMON_DIR}/Utilities.h"
	"${SRC_COMMON_DIR}/Zone.h"

	"${SRC_COMMON_DIR}/Net/INetNToP.h"
	"${SRC_COMMON_DIR}/Net/INetPToN.h"
	"${SRC_COMMON_DIR}/Net/Win.h"
	
	"${SRC_COMMON_DIR}/Hashes/Crc32.h"
)
endfunction()
GenerateCommonSourceLists()