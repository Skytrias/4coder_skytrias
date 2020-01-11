
// TOP

#if !defined(FCODER_DEFAULT_BINDINGS_CPP)
#define FCODER_DEFAULT_BINDINGS_CPP

#include <stdlib.h>

// NOTE(Skytrias): exports my snippets into the snippet lister, turn this off or include your own, additionally I'd recommend disabling the general snippets 
#define SNIPPET_EXPANSION "rust_snippets.inc"

#include "4coder_default_include.cpp"

function void skytrias_draw_cpp_token_colors(Application_Links *app, Text_Layout_ID text_layout_id, Token_Array *array);

global i32 global_snippet_count = 0;
#include "auto_snippet.cpp"
#include "code_peek.cpp"
#include "render_highlight.cpp"
#include "bindings.cpp"

BUFFER_HOOK_SIG(skytrias_begin_buffer){
    ProfileScope(app, "begin buffer");
	
    Scratch_Block scratch(app);
	
    b32 treat_as_code = false;
    String_Const_u8 file_name = push_buffer_file_name(app, scratch, buffer_id);
    if (file_name.size > 0){
        String_Const_u8_Array extensions = global_config.code_exts;
        String_Const_u8 ext = string_file_extension(file_name);
        
		for (i32 i = 0; i < extensions.count; ++i){
            if (string_match(ext, extensions.strings[i])){
                if (string_match(ext, string_u8_litexpr("cpp")) || 
                    string_match(ext, string_u8_litexpr("h")) ||
                    string_match(ext, string_u8_litexpr("c")) ||
                    string_match(ext, string_u8_litexpr("hpp")) ||
                    string_match(ext, string_u8_litexpr("rs")) ||
                    string_match(ext, string_u8_litexpr("toml")) ||
					string_match(ext, string_u8_litexpr("cc"))){
                    treat_as_code = true;
				}
				
                break;
            }
        }
    }
	
    Command_Map_ID map_id = (treat_as_code)?(mapid_code):(mapid_file);
    Managed_Scope scope = buffer_get_managed_scope(app, buffer_id);
    Command_Map_ID *map_id_ptr = scope_attachment(app, scope, buffer_map_id, Command_Map_ID);
    *map_id_ptr = map_id;
	
    Line_Ending_Kind setting = guess_line_ending_kind_from_buffer(app, buffer_id);
    Line_Ending_Kind *eol_setting = scope_attachment(app, scope, buffer_eol_setting, Line_Ending_Kind);
    *eol_setting = setting;
	
    // NOTE(allen): Decide buffer settings
    b32 wrap_lines = true;
    b32 use_virtual_whitespace = false;
    b32 use_lexer = false;
    if (treat_as_code){
        wrap_lines = global_config.enable_code_wrapping;
        use_virtual_whitespace = global_config.enable_virtual_whitespace;
        use_lexer = true;
    }
	
    String_Const_u8 buffer_name = push_buffer_base_name(app, scratch, buffer_id);
    if (string_match(buffer_name, string_u8_litexpr("*compilation*"))){
        wrap_lines = false;
    }
	
    if (use_lexer){
		ProfileBlock(app, "begin buffer kick off lexer");
		Async_Task *lex_task_ptr = scope_attachment(app, scope, buffer_lex_task, Async_Task);
		*lex_task_ptr = async_task_no_dep(&global_async_system, do_full_lex_async, make_data_struct(&buffer_id));
	}
	
    {
        b32 *wrap_lines_ptr = scope_attachment(app, scope, buffer_wrap_lines, b32);
        *wrap_lines_ptr = wrap_lines;
    }
	
    if (use_virtual_whitespace){
        if (use_lexer){
            buffer_set_layout(app, buffer_id, layout_virt_indent_index_generic);
        }
        else{
            buffer_set_layout(app, buffer_id, layout_virt_indent_literal_generic);
        }
    }
    else{
        buffer_set_layout(app, buffer_id, layout_generic);
    }
	
    // no meaning for return
    return(0);
}


internal void
skytrias_set_all_default_hooks(Application_Links *app){
    set_custom_hook(app, HookID_BufferViewerUpdate, default_view_adjust);
	
    set_custom_hook(app, HookID_ViewEventHandler, default_view_input_handler);
    set_custom_hook(app, HookID_Tick, default_tick);
    
	// NOTE(Skytrias): new
	set_custom_hook(app, HookID_RenderCaller, skytrias_render_caller);
#if 0
    set_custom_hook(app, HookID_DeltaRule, original_delta);
    set_custom_hook_memory_size(app, HookID_DeltaRule,
                                delta_ctx_size(original_delta_memory_size));
#else
    set_custom_hook(app, HookID_DeltaRule, fixed_time_cubic_delta);
    set_custom_hook_memory_size(app, HookID_DeltaRule,
                                delta_ctx_size(fixed_time_cubic_delta_memory_size));
#endif
    set_custom_hook(app, HookID_BufferNameResolver, default_buffer_name_resolution);
	
	// NOTE(Skytrias): new
    set_custom_hook(app, HookID_BeginBuffer, skytrias_begin_buffer);
    set_custom_hook(app, HookID_EndBuffer, end_buffer_close_jump_list);
    set_custom_hook(app, HookID_NewFile, default_new_file);
    set_custom_hook(app, HookID_SaveFile, default_file_save);
    set_custom_hook(app, HookID_BufferEditRange, default_buffer_edit_range);
    set_custom_hook(app, HookID_BufferRegion, default_buffer_region);
	
    set_custom_hook(app, HookID_Layout, layout_unwrapped);
    //set_custom_hook(app, HookID_Layout, layout_wrap_anywhere);
    //set_custom_hook(app, HookID_Layout, layout_wrap_whitespace);
    //set_custom_hook(app, HookID_Layout, layout_virt_indent_unwrapped);
    //set_custom_hook(app, HookID_Layout, layout_unwrapped_small_blank_lines);
}

// BOTTOM

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
    skytrias_set_all_default_hooks(app);
	
	mapping_init(tctx, &framework_mapping);
    skytrias_set_bindings(&framework_mapping);
}

#endif //FCODER_DEFAULT_BINDINGS

