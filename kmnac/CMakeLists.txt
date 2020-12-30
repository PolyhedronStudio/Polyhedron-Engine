cmake_minimum_required (VERSION 3.8)

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set( IS_64_BIT 1 )
endif ()

SET(SRC_BASEQ2
	game/acebot_ai.c
	game/acebot_cmds.c
	game/acebot_compress.c
	game/acebot_items.c
	game/acebot_movement.c
	game/acebot_nodes.c
	game/acebot_spawn.c
	game/g_ai.c
	game/g_camera.c
	game/g_chase.c
	game/g_cmds.c
	game/g_combat.c
	game/g_crane.c
	game/g_ctf.c
	game/g_fog.c
	game/g_func.c
	game/g_items.c
	game/g_jetpack.c
	game/g_lights.c
	game/g_lock.c
	game/g_main.c
	game/g_misc.c
	game/g_model.c
	game/g_monster.c
	game/g_moreai.c
	game/g_mtrain.c
	game/g_patchplayermodels.c
	game/g_pendulum.c
	game/g_phys.c
	game/g_ptrs.c
	game/g_reflect.c
	game/g_save.c
	game/g_spawn.c
	game/g_svcmds.c
	game/g_target.c
	game/g_thing.c
	game/g_tracktrain.c
	game/g_trigger.c
	game/g_turret.c
	game/g_utils.c
	game/g_vehicle.c
	game/g_weapon.c
	game/km_cvar.c
	game/m_actor.c
	game/m_actor_weap.c
	game/m_berserk.c
	game/m_boss2.c
	game/m_boss3.c
	game/m_boss31.c
	game/m_boss32.c
	game/m_brain.c
	game/m_chick.c
	game/m_flash.c
	game/m_flipper.c
	game/m_float.c
	game/m_flyer.c
	game/m_gladiator.c
	game/m_gunner.c
	game/m_hover.c
	game/m_infantry.c
	game/m_insane.c
	game/m_medic.c
	game/m_move.c
	game/m_mutant.c
	game/m_parasite.c
	game/m_soldier.c
	game/m_supertank.c
	game/m_tank.c
	game/p_chase.c
	game/p_client.c
	game/p_hud.c
	game/p_menu.c
	game/p_text.c
	game/p_trail.c
	game/p_view.c
	game/p_weapon.c
	game/q_shared.c
)

SET(HEADERS_BASEQ2
	game/acebot.h
	game/game.h
	game/g_ctf.h
	game/g_func_decs.h
	game/g_func_list.h
	game/g_local.h
	game/g_mmove_decs.h
	game/g_mmove_list.h
	game/lazarus.def
	game/laz_misc.h
	game/km_cvar.h
	game/g_ptrs.h
	game/m_actor.h
	game/m_berserk.h
	game/q_shared.h
	game/resource.h
	game/p_text.h
	game/p_menu.h
	game/m_tank.h
	game/pak.h
	game/m_supertank.h
	game/m_soldier.h
	game/m_parasite.h
	game/m_player.h
	game/m_rider.h
	game/m_mutant.h
	game/m_medic.h
	game/m_insane.h
	game/m_boss2.h
	game/m_infantry.h
	game/m_hover.h
	game/m_gunner.h
	game/m_gladiator.h
	game/m_flyer.h
	game/m_float.h
	game/m_boss31.h
	game/m_flipper.h
	game/m_chick.h
	game/m_brain.h
	game/m_boss32.h
)

# Embed version number
ADD_DEFINITIONS(-DVERSION_MAJOR=${Q2RTX_VERSION_MAJOR})
ADD_DEFINITIONS(-DVERSION_MINOR=${Q2RTX_VERSION_MINOR})
ADD_DEFINITIONS(-DVERSION_POINT=${Q2RTX_VERSION_POINT})
ADD_DEFINITIONS(-DVERSION_SHA=${Q2RTX_VERSION_SHA})
ADD_DEFINITIONS(-DVERSION_BRANCH=${Q2RTX_VERSION_BRANCH})

ADD_DEFINITIONS(-DHAVE_CONFIG_H=1 -DCURL_STATICLIB)

IF(WIN32)
	ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
ENDIF()

ADD_LIBRARY(gamex86 SHARED ${SRC_BASEQ2} ${HEADERS_BASEQ2} ${SRC_SHARED})

SOURCE_GROUP("baseq2\\sources" FILES ${SRC_BASEQ2})
SOURCE_GROUP("baseq2\\headers" FILES ${HEADERS_BASEQ2})

TARGET_INCLUDE_DIRECTORIES(gamex86 PRIVATE ../inc)

# specify both LIBRARY and RUNTIME because one works only on Windows and another works only on Linux

SET_TARGET_PROPERTIES(gamex86
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/baseq2"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/baseq2"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/baseq2"
    LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/baseq2"
    LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/baseq2"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/baseq2"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/baseq2"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/baseq2"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/baseq2"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/baseq2"
    PREFIX ""
    DEBUG_POSTFIX ""
)

IF(IS_64_BIT)
	SET_TARGET_PROPERTIES(gamex86
		PROPERTIES
		LIBRARY_OUTPUT_NAME "gamex86_64"
		RUNTIME_OUTPUT_NAME "gamex86_64"
	)
ENDIF()

IF(CONFIG_LINUX_PACKAGING_SUPPORT)
    # Put the real game binary in /usr/share so we can have a wrapper in /usr/bin
    INSTALL(TARGETS client DESTINATION share/quake2rtx/bin COMPONENT shareware)
    INSTALL(TARGETS server DESTINATION games COMPONENT shareware)

    # Compress NVIDIA custom content
    INSTALL(CODE "set(SOURCE \"${CMAKE_SOURCE_DIR}\")"
        SCRIPT "${CMAKE_SOURCE_DIR}/setup/package_media.cmake"
        SCRIPT "${CMAKE_SOURCE_DIR}/setup/package_shaders.cmake"
        COMPONENT shareware)
    INSTALL(TARGETS gamex86 DESTINATION share/quake2rtx/baseq2 COMPONENT shareware)

    # Package data files, including the shareware pak0 for demo levels
    set (SHAREWARE_DATA_FILES_LIST
        "${CMAKE_SOURCE_DIR}/baseq2/q2rtx_media.pkz"
        "${CMAKE_SOURCE_DIR}/baseq2/shaders.pkz"
        "${CMAKE_SOURCE_DIR}/baseq2/blue_noise.pkz"
        "${CMAKE_SOURCE_DIR}/baseq2/shareware/pak0.pak"
    )
    INSTALL(FILES
        ${SHAREWARE_DATA_FILES_LIST}
        DESTINATION share/quake2rtx/baseq2
        COMPONENT shareware)

    # Package shareware player model
    INSTALL(DIRECTORY
        "${CMAKE_SOURCE_DIR}/baseq2/shareware/players"
        DESTINATION share/quake2rtx/baseq2
        COMPONENT shareware)

    # Package the legal and info files
    INSTALL(FILES
        "${CMAKE_SOURCE_DIR}/license.txt"
        "${CMAKE_SOURCE_DIR}/notice.txt"
        "${CMAKE_SOURCE_DIR}/readme.md"
        "${CMAKE_SOURCE_DIR}/changelog.md"
        DESTINATION share/quake2rtx/
        COMPONENT shareware)

    # Package the icon/desktop file
    INSTALL(FILES
        "${CMAKE_SOURCE_DIR}/setup/q2rtx.desktop"
        DESTINATION share/applications/
        COMPONENT shareware)
    INSTALL(FILES
        "${CMAKE_SOURCE_DIR}/setup/q2rtx.png"
        DESTINATION share/pixmaps/
        COMPONENT shareware)

    # Install Client Wrapper
    INSTALL(FILES
        "${CMAKE_SOURCE_DIR}/setup/q2rtx.sh"
        DESTINATION games/
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        COMPONENT shareware
	RENAME q2rtx)

    # Install PAK file finder
    INSTALL(FILES
        "${CMAKE_SOURCE_DIR}/setup/find-retail-paks.sh"
        DESTINATION share/quake2rtx/bin
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        COMPONENT shareware)
ENDIF()
