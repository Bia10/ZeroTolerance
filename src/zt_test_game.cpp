/**************************************************************************************************
 ** file: zt_testgame.cpp
 **
 ** This library is in the public domain.  Do with it what you will.
 **
 **************************************************************************************************
 **
 ** implementation notes:
 ** 
 **************************************************************************************************/

// headers ========================================================================================

#define ZT_TOOLS_IMPLEMENTATION
#define ZT_GAME_IMPLEMENTATION

//#define ZT_MEM_ARENA_LOG_DETAILS
#define ZT_GAME_NAME			"ZeroTolerance Test Game"
#define ZT_GAME_LOCAL_ONLY
#define ZT_GAME_FUNC_SETTINGS	game_settings
#define ZT_GAME_FUNC_INIT		game_init
#define ZT_GAME_FUNC_CLEANUP	game_cleanup
#define ZT_GAME_FUNC_LOOP		game_loop

#include "zt_tools.h"
#include "zt_game.h"


// types/enums/defines ============================================================================

// structs/classes ================================================================================

struct ztGame
{
	ztGameDetails *details;
	ztGameSettings *settings;


	ztMemoryArena* asset_arena;
	ztAssetManager asset_mgr;
};


// variables ======================================================================================

ztGame *g_game = nullptr;


// private functions ==============================================================================

ztInternal void _game_assetModified(ztAssetManager *asset_mgr, ztAssetID asset_id, void *user_data)
{
	zt_logDebug("Asset modified: %s", asset_mgr->asset_name[asset_id]);
}

// ------------------------------------------------------------------------------------------------

// functions ======================================================================================

bool game_settings(ztGameDetails* details, ztGameSettings* settings)
{
	zt_strMakePrintf(ini_file, ztFileMaxPath, "%s%csettings.cfg", details->user_path, ztFilePathSeparator);
	settings->memory = zt_iniFileGetValue(ini_file, "general", "app_memory", (i32)zt_megabytes(64));

	settings->native_w = settings->screen_w = zt_iniFileGetValue(ini_file, "general", "resolution_w", (i32)1920);
	settings->native_h = settings->screen_h = zt_iniFileGetValue(ini_file, "general", "resolution_h", (i32)1080);
	//settings->renderer = ztRenderer_OpenGL;
	settings->renderer = ztRenderer_DirectX;

	char cfg_renderer[128] = { 0 };
	zt_iniFileGetValue(ini_file, "general", "renderer", nullptr, cfg_renderer, sizeof(cfg_renderer));
	if(zt_striCmp(cfg_renderer, "opengl") == 0) settings->renderer = ztRenderer_OpenGL;
	if(zt_striCmp(cfg_renderer, "directx") == 0) settings->renderer = ztRenderer_DirectX;

	//settings->renderer_flags |= ztRendererFlags_Vsync;


	char test[1024 * 128];
	zt_getDirectorySubs(details->app_path, test, sizeof(test), true);
	zt_logDebug("directories:\n%s", test);

	zt_getDirectoryFiles(details->app_path, test, sizeof(test), true);
	zt_logDebug("files:\n%s", test);

	return true;
}

// ------------------------------------------------------------------------------------------------

bool game_init(ztGameDetails* game_details, ztGameSettings* game_settings)
{
	g_game = zt_mallocStruct(ztGame);
	*g_game = {};
	g_game->details = game_details;
	g_game->settings = game_settings;

	g_game->asset_arena = zt_memMakeArena(zt_megabytes(32), zt_memGetGlobalArena());
	zt_strMakePrintf(data_path, ztFileMaxPath, "%s%crun%cdata", game_details->user_path, ztFilePathSeparator, ztFilePathSeparator);

	if (!zt_assetManagerLoadDirectory(&g_game->asset_mgr, data_path, g_game->asset_arena)) {
		zt_logCritical("Unable to load game assets");
		return false;
	}

	char* assets[] = {
		"textures/debug.png",
		"textures/white.png",
		"shaders/shader_test.zts",
	};

	zt_fiz(zt_elementsOf(assets)) {
		ztAssetID asset_id = zt_assetLoad(&g_game->asset_mgr, assets[i]);
		if (asset_id != ztInvalidID) {
			zt_assetAddReloadCallback(&g_game->asset_mgr, asset_id, _game_assetModified, g_game);
		}
	}
	return true;
}

// ------------------------------------------------------------------------------------------------

void game_cleanup()
{
	zt_assetManagerFree(&g_game->asset_mgr);
	zt_memDumpArena(g_game->asset_arena, "asset memory");
	zt_memFreeArena(g_game->asset_arena);
	zt_free(g_game);
}

// ------------------------------------------------------------------------------------------------

bool game_loop(r32 dt)
{
	ztInputKeys* input = zt_inputKeysAccessState();
	
	// color cycle the background
	{
		const ztVec4 colors[] = {
			ztVec4(1, 0, 0, 1),
			ztVec4(0, 1, 0, 1),
			ztVec4(0, 0, 1, 1),
			ztVec4(1, 1, 0, 1),
			ztVec4(1, 0, 1, 1),
			ztVec4(0, 1, 1, 1),
		};

		static i32 color_idx = 0;
		static r32 color_time = 0;

		color_time += dt;
		if (color_time > 1) {
			color_time -= 1;
			color_idx += 1;
		}

		ztVec4 color = ztVec4::lerp(colors[color_idx % zt_elementsOf(colors)], colors[(color_idx + 1) % zt_elementsOf(colors)], color_time);
		zt_rendererClear(color);
	}

	{
#if 0
		ztAssetManager asset_mgr;
		zt_assetManagerLoadDirectory(&asset_mgr, data_dir);
		zt_assetManagerLoadPackedFile(&asset_mgr, pack_file);

		ztAssetID asset_id = zt_assetLoadFromFile(&asset_mgr, file_name);
		ztAssetID asset_id2 = zt_assetLoadFromData(&asset_mgr, data, data_size);

		i32 bin_size = zt_assetSize(&asset_mgr, asset_id);
		void* bin_data = zt_memAlloc(bin_size);
		if(!zt_assetLoad(&asset_mgr, asset_id, bin_data, bin_size))
			return false;

		zt

		zt_rlPushShader(g_game->renderList, g_game->shader_test); {
			zt_rlPushTexture(g_game->renderList, g_game->texture_test); {
				zt_rlPushQuad(-1, -1, 1, 1, ztVec4::one);
			}
			zt_rlPopTexture(g_game->renderList);
		}
		zt_rlPopShader(g_game->renderList);

		zt_rlFlush(g_game->renderList);
#endif
	}

	// test changing renderers
	{
		if (input[ztInputKeys_R].justPressed() && input[ztInputKeys_Control].pressed()) {
			zt_rendererRequestChange(g_game->settings->renderer == ztRenderer_OpenGL ? ztRenderer_DirectX : ztRenderer_OpenGL);
		}
	}

	// test changing windows mode
	{
		static bool fullscreen = false;

		if (input[ztInputKeys_F].justPressed() && input[ztInputKeys_Control].pressed()) {
			if(!fullscreen) {
				zt_rendererRequestFullscreen();
			}
			else {
				zt_rendererRequestWindowed();
			}
			fullscreen = !fullscreen;
		}
	}

	// calculate frame time
	{
		static r32 dt_accum = 0;
		static int frame = 0;

		dt_accum += dt;
		frame += 1;
		if (dt_accum > 1) {
			// show frame time in microseconds.  to hit 60 fps, each frame must take less than 16,666 microseconds (or 16.666 ms)
			zt_logDebug("average frame rate: %.6f us/f (%.2f fps)", (dt_accum / frame) * 1000.f, 1.f / (dt_accum / frame));
			dt_accum = 0;
			frame = 0;
		}
	}

	zt_assetManagerCheckForChanges(&g_game->asset_mgr);
	return !input[ztInputKeys_Escape].justPressed();
}

// ------------------------------------------------------------------------------------------------
