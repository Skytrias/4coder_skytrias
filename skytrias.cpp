
// TOP

#if !defined(FCODER_DEFAULT_BINDINGS_CPP)
#define FCODER_DEFAULT_BINDINGS_CPP

#include <stdlib.h>

// NOTE(Skytrias): exports my snippets into the snippet lister, turn this off or include your own, additionally I'd recommend disabling the general snippets 
#define SNIPPET_EXPANSION "rust_snippets.inc"

// NOTE(Skytrias): audio via miniaudio
#define RAUDIO_STANDALONE
#define SUPPORT_FILEFORMAT_MP3
#include "external/raudio.h"
#include "external/raudio.c"

#include "4coder_default_include.cpp"

function void st_draw_cpp_token_colors(Application_Links *app, Text_Layout_ID text_layout_id, Token_Array *array);

global i32 global_snippet_count = 0;
global Sound global_timer_start_sound = {};
global Sound global_timer_restart_sound = {};
global Sound global_timer_end_sound = {};
global Sound global_timer_paste_sound = {};
global Sound global_timer_pause_sound = {};

#include "helpers.cpp"
#include "custom_search.cpp"
#include "auto_snippet.cpp"
#include "code_peek.cpp"
#include "todo_and_timer.cpp"
#include "render_highlight.cpp"
#include "bindings.cpp"

void
custom_layer_init(Application_Links *app){
	Thread_Context *tctx = get_thread_context(app);
	
	global_snippet_count = ArrayCount(default_snippets);
	
	// NOTE(allen): setup for default framework
	async_task_handler_init(app, &global_async_system);
	code_index_init();
	buffer_modified_set_init();
	Profile_Global_List *list = get_core_profile_list(app);
	ProfileThreadName(tctx, list, string_u8_litexpr("main"));
	initialize_managed_id_metadata(app);
	set_default_color_scheme(app);
	
	// NOTE(Skytrias): remove the old default hooks!
	st_set_all_default_hooks(app);
	
	mapping_init(tctx, &framework_mapping);
	st_set_bindings(&framework_mapping);
	
	InitAudioDevice();
    global_timer_start_sound = LoadSound("timer_start.mp3");
    global_timer_restart_sound = LoadSound("timer_restart.mp3");
    SetSoundVolume(global_timer_restart_sound, 0.3f);
    global_timer_end_sound = LoadSound("timer_end.mp3");
    SetSoundVolume(global_timer_end_sound, 0.4f);
	global_timer_paste_sound = LoadSound("timer_paste.mp3");
    SetSoundVolume(global_timer_paste_sound, 0.4f);
    global_timer_pause_sound = LoadSound("timer_pause.mp3");
}

#endif //FCODER_DEFAULT_BINDINGS


