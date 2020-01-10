
// TOP

#if !defined(FCODER_DEFAULT_BINDINGS_CPP)
#define FCODER_DEFAULT_BINDINGS_CPP

#include <stdlib.h>

// NOTE(Skytrias): exports my snippets into the snippet lister, turn this off or include your own, additionally I'd recommend disabling the general snippets 
#define SNIPPET_EXPANSION "rust_snippets.inc"

#include "4coder_default_include.cpp"
#include "code_peek.cpp"
#include "render_highlight.cpp"
#include "bindings.cpp"

void
custom_layer_init(Application_Links *app){
    Thread_Context *tctx = get_thread_context(app);
    
    // NOTE(allen): setup for default framework
    async_task_handler_init(app, &global_async_system);
    code_index_init();
    buffer_modified_set_init();
    Profile_Global_List *list = get_core_profile_list(app);
    ProfileThreadName(tctx, list, string_u8_litexpr("main"));
    initialize_managed_id_metadata(app);
    set_default_color_scheme(app);
    
    // NOTE(allen): default hooks and command maps
    set_all_default_hooks(app);
	// NOTE(Skytrias): you have to disable the calls in default_hooks.cpp i think
    set_custom_hook(app, HookID_RenderCaller, skytrias_render_caller);
    
	mapping_init(tctx, &framework_mapping);
    skytrias_set_bindings(&framework_mapping);
}

#endif //FCODER_DEFAULT_BINDINGS
