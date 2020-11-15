
// TOP

#if !defined(FCODER_DEFAULT_BINDINGS_CPP)
#define FCODER_DEFAULT_BINDINGS_CPP

//#include <stdlib.h>

// NOTE(Skytrias): exports my snippets into the snippet lister, turn this off or include your own, additionally I'd recommend disabling the general snippets
//#define SNIPPET_EXPANSION "snippets.inc"

#include "4coder_default_include.cpp"

function void st_draw_cpp_token_colors(Application_Links *app, Text_Layout_ID text_layout_id, Token_Array *array);

global i32 global_snippet_count = 0;
#include "generated/managed_id_metadata.cpp"

#include "helpers.cpp"
#include "custom_search.cpp"
#include "auto_snippet.cpp"
#include "render_highlight.cpp"
#include "preview_goto.cpp"
#include "bindings.cpp"

void
custom_layer_init(Application_Links *app){
	Thread_Context *tctx = get_thread_context(app);
	global_snippet_count = ArrayCount(default_snippets);
	
    // NOTE(allen): setup for default framework
    default_framework_init(app);
    
	// NOTE(Skytrias): remove the old default hooks!
	set_all_default_hooks(app);
	set_custom_hook(app, HookID_RenderCaller, st_render_caller);
	set_custom_hook(app, HookID_BeginBuffer, st_begin_buffer);
	set_custom_hook(app, HookID_BufferRegion, st_buffer_region);
	
	mapping_init(tctx, &framework_mapping);
	st_set_bindings(&framework_mapping, mapid_global, mapid_file, mapid_code);
	//setup_default_mapping(&framework_mapping, mapid_global, mapid_file, mapid_code);
}

#endif //FCODER_DEFAULT_BINDINGS


