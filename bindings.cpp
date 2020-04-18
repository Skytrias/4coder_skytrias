CUSTOM_COMMAND_SIG(st_exit_4coder)
CUSTOM_DOC("Attempts to close 4coder.")
{
	send_exit_signal(app);
}

CUSTOM_COMMAND_SIG(st_passive_buffer_on)
CUSTOM_DOC("Makes a buffer passive")
{
	View_ID view_id = get_active_view(app, Access_ReadVisible);
	view_set_passive(app, view_id, 1);
}

CUSTOM_COMMAND_SIG(st_passive_buffer_off)
CUSTOM_DOC("Makes a buffer non passive")
{
	View_ID view_id = get_active_view(app, Access_ReadVisible);
	view_set_passive(app, view_id, 0);
}

// NOTE(Skytrias): own preferences to keybindings
static void
st_set_bindings(Mapping *mapping, i64 global_id, i64 file_id, i64 code_id)
{
    MappingScope();
    SelectMapping(mapping);
	
    SelectMap(global_id);
    BindCore(default_startup, CoreCode_Startup);
    BindCore(default_try_exit, CoreCode_TryExit);
    Bind(keyboard_macro_start_recording , KeyCode_U, KeyCode_Control);
    Bind(keyboard_macro_finish_recording, KeyCode_U, KeyCode_Control, KeyCode_Alt);
    Bind(keyboard_macro_replay,           KeyCode_U, KeyCode_Alt);
    Bind(change_active_panel,           KeyCode_Comma, KeyCode_Control);
    Bind(change_active_panel_backwards, KeyCode_Comma, KeyCode_Control, KeyCode_Shift);
    Bind(interactive_new,               KeyCode_N, KeyCode_Control);
    Bind(interactive_open_or_new,       KeyCode_O, KeyCode_Control);
    Bind(open_in_other,                 KeyCode_O, KeyCode_Alt);
    Bind(interactive_kill_buffer,       KeyCode_K, KeyCode_Control);
    // NOTE(Skytrias): new
	Bind(interactive_switch_buffer,     KeyCode_I, KeyCode_Control);
    Bind(project_go_to_root_directory,  KeyCode_H, KeyCode_Control);
    Bind(save_all_dirty_buffers,        KeyCode_S, KeyCode_Control, KeyCode_Shift);
    Bind(change_to_build_panel,         KeyCode_Period, KeyCode_Alt);
    Bind(close_build_panel,             KeyCode_Comma, KeyCode_Alt);
    Bind(goto_next_jump,                KeyCode_N, KeyCode_Alt);
    Bind(goto_prev_jump,                KeyCode_N, KeyCode_Alt, KeyCode_Control);
    // NOTE(Skytrias): i never use this
	//Bind(build_in_build_panel,          KeyCode_M, KeyCode_Alt);
	Bind(cursor_mark_swap,            KeyCode_M, KeyCode_Alt);
    Bind(goto_first_jump,               KeyCode_M, KeyCode_Alt, KeyCode_Shift);
    Bind(toggle_filebar,                KeyCode_B, KeyCode_Alt);
    Bind(execute_any_cli,               KeyCode_Z, KeyCode_Alt);
    Bind(execute_previous_cli,          KeyCode_Z, KeyCode_Alt, KeyCode_Shift);
    Bind(command_lister,                KeyCode_X, KeyCode_Alt);
    Bind(project_command_lister,        KeyCode_X, KeyCode_Alt, KeyCode_Shift);
    // NOTE(Skytrias): new
    Bind(list_all_functions_current_buffer, KeyCode_Q, KeyCode_Control);
    Bind(list_all_functions_current_buffer_lister, KeyCode_Q, KeyCode_Control, KeyCode_Alt);
    Bind(project_fkey_command, KeyCode_F1);
    Bind(project_fkey_command, KeyCode_F2);
    Bind(project_fkey_command, KeyCode_F3);
    Bind(project_fkey_command, KeyCode_F4);
    Bind(project_fkey_command, KeyCode_F5);
    Bind(project_fkey_command, KeyCode_F6);
    Bind(project_fkey_command, KeyCode_F7);
    Bind(project_fkey_command, KeyCode_F8);
    Bind(project_fkey_command, KeyCode_F9);
    Bind(project_fkey_command, KeyCode_F10);
    Bind(project_fkey_command, KeyCode_F11);
    Bind(project_fkey_command, KeyCode_F12);
    Bind(project_fkey_command, KeyCode_F13);
    Bind(project_fkey_command, KeyCode_F14);
    Bind(project_fkey_command, KeyCode_F15);
    Bind(project_fkey_command, KeyCode_F16);
    Bind(st_exit_4coder,          KeyCode_F4, KeyCode_Alt);
    BindMouseWheel(mouse_wheel_scroll);
    BindMouseWheel(mouse_wheel_change_face_size, KeyCode_Control);
	
    SelectMap(file_id);
    ParentMap(global_id);
	
    // NOTE(Skytrias): used for auto snippet
	BindTextInput(st_write_text_input);
    BindMouse(click_set_cursor_and_mark, MouseCode_Left);
    BindMouseRelease(click_set_cursor, MouseCode_Left);
    BindCore(click_set_cursor_and_mark, CoreCode_ClickActivateView);
    BindMouseMove(click_set_cursor_if_lbutton);
    Bind(delete_char,            KeyCode_Delete);
    Bind(backspace_char,         KeyCode_Backspace);
    Bind(move_up,                KeyCode_Up);
    Bind(move_down,              KeyCode_Down);
    Bind(move_left,              KeyCode_Left);
    Bind(move_right,             KeyCode_Right);
    Bind(seek_end_of_line,       KeyCode_End);
    Bind(seek_beginning_of_line, KeyCode_Home);
	Bind(page_up,                KeyCode_PageUp);
    Bind(page_down,              KeyCode_PageDown);
    Bind(goto_beginning_of_file, KeyCode_PageUp, KeyCode_Control);
    Bind(goto_end_of_file,       KeyCode_PageDown, KeyCode_Control);
    Bind(move_up_to_blank_line_end,        KeyCode_Up, KeyCode_Control);
    Bind(move_down_to_blank_line_end,      KeyCode_Down, KeyCode_Control);
    Bind(move_left_whitespace_boundary,    KeyCode_Left, KeyCode_Control);
    Bind(move_right_whitespace_boundary,   KeyCode_Right, KeyCode_Control);
    Bind(move_line_up,                     KeyCode_Up, KeyCode_Alt);
    Bind(move_line_down,                   KeyCode_Down, KeyCode_Alt);
    Bind(backspace_alpha_numeric_boundary, KeyCode_Backspace, KeyCode_Control);
    Bind(delete_alpha_numeric_boundary,    KeyCode_Delete, KeyCode_Control);
    Bind(snipe_backward_whitespace_or_token_boundary, KeyCode_Backspace, KeyCode_Alt);
    Bind(snipe_forward_whitespace_or_token_boundary,  KeyCode_Delete, KeyCode_Alt);
    Bind(set_mark,                    KeyCode_Space, KeyCode_Control);
    Bind(replace_in_range,            KeyCode_A, KeyCode_Control);
    Bind(copy,                        KeyCode_C, KeyCode_Control);
    Bind(delete_range,                KeyCode_D, KeyCode_Control);
    Bind(delete_line,                 KeyCode_D, KeyCode_Control, KeyCode_Alt);
    Bind(center_view,                 KeyCode_E, KeyCode_Control);
    Bind(left_adjust_view,            KeyCode_E, KeyCode_Control, KeyCode_Shift);
    Bind(st_search,                      KeyCode_F, KeyCode_Control);
    Bind(list_all_locations,          KeyCode_F, KeyCode_Control, KeyCode_Alt);
    Bind(list_all_substring_locations_case_insensitive, KeyCode_F, KeyCode_Alt);
    Bind(st_goto_line,                   KeyCode_G, KeyCode_Control);
    Bind(list_all_locations_of_selection,  KeyCode_G, KeyCode_Control, KeyCode_Shift);
    Bind(snippet_lister,              KeyCode_J, KeyCode_Control);
    Bind(kill_buffer,                 KeyCode_K, KeyCode_Control, KeyCode_Shift);
    Bind(duplicate_line,              KeyCode_L, KeyCode_Control);
    Bind(cursor_mark_swap,            KeyCode_M, KeyCode_Control);
    Bind(reopen,                      KeyCode_O, KeyCode_Control, KeyCode_Shift);
    //Bind(query_replace,               KeyCode_Q, KeyCode_Control);
    //Bind(query_replace_identifier,    KeyCode_Q, KeyCode_Control, KeyCode_Shift);
    //Bind(query_replace_selection,     KeyCode_Q, KeyCode_Alt);
    Bind(st_reverse_search,              KeyCode_R, KeyCode_Control);
    Bind(save,                        KeyCode_S, KeyCode_Control);
    Bind(save_all_dirty_buffers,      KeyCode_S, KeyCode_Control, KeyCode_Shift);
    Bind(st_search_identifier,           KeyCode_T, KeyCode_Control);
    Bind(list_all_locations_of_identifier, KeyCode_T, KeyCode_Control, KeyCode_Alt);
    Bind(paste_and_indent,            KeyCode_V, KeyCode_Control);
    //Bind(paste_next_and_indent,       KeyCode_V, KeyCode_Control, KeyCode_Shift);
    Bind(cut,                         KeyCode_X, KeyCode_Control);
    Bind(redo,                        KeyCode_Y, KeyCode_Control);
    Bind(undo,                        KeyCode_Z, KeyCode_Control);
    Bind(view_buffer_other_panel,     KeyCode_1, KeyCode_Control);
    Bind(swap_panels,                 KeyCode_2, KeyCode_Control);
    Bind(if_read_only_goto_position,  KeyCode_Return);
    Bind(if_read_only_goto_position_same_panel, KeyCode_Return, KeyCode_Shift);
    Bind(view_jump_list_with_lister,  KeyCode_Period, KeyCode_Control, KeyCode_Shift);
	
    SelectMap(code_id);
    ParentMap(file_id);
	
	// NOTE(Skytrias): used for auto snippet
	BindTextInput(st_write_text_and_auto_indent);
    Bind(move_left_alpha_numeric_boundary,           KeyCode_Left, KeyCode_Control);
    Bind(move_right_alpha_numeric_boundary,          KeyCode_Right, KeyCode_Control);
    Bind(move_left_alpha_numeric_or_camel_boundary,  KeyCode_Left, KeyCode_Alt);
    Bind(move_right_alpha_numeric_or_camel_boundary, KeyCode_Right, KeyCode_Alt);
    Bind(comment_line_toggle,        KeyCode_Semicolon, KeyCode_Control);
    Bind(word_complete,              KeyCode_Tab);
    //Bind(word_complete_reverse,              KeyCode_Tab, KeyCode_Shift);
    //Bind(auto_indent_range,          KeyCode_Tab, KeyCode_Control);
    //Bind(auto_indent_line_at_cursor, KeyCode_Tab, KeyCode_Shift);
    //Bind(word_complete_drop_down,    KeyCode_Tab, KeyCode_Shift, KeyCode_Control);
	// NOTE(Skytrias): use hack instead of write_block
    Bind(write_hack,                KeyCode_R, KeyCode_Alt);
    Bind(write_todo,                 KeyCode_T, KeyCode_Alt);
    Bind(write_note,                 KeyCode_Y, KeyCode_Alt);
	Bind(list_all_locations,               KeyCode_Tab, KeyCode_Shift, KeyCode_Control);
    Bind(list_all_locations_of_type_definition,               KeyCode_D, KeyCode_Alt);
    Bind(list_all_locations_of_type_definition_of_identifier, KeyCode_T, KeyCode_Alt, KeyCode_Shift);
	
	// NOTE(Skytrias): german keyboard doesnt have these
	//Bind(select_surrounding_scope,   KeyCode_LeftBracket, KeyCode_Alt);
    //Bind(select_surrounding_scope_maximal, KeyCode_LeftBracket, KeyCode_Alt, KeyCode_Shift);
    //Bind(select_prev_scope_absolute, KeyCode_RightBracket, KeyCode_Alt);
    //Bind(select_prev_top_most_scope, KeyCode_RightBracket, KeyCode_Alt, KeyCode_Shift);
    //Bind(select_next_scope_absolute, KeyCode_Quote, KeyCode_Alt);
    //Bind(select_next_scope_after_current, KeyCode_Quote, KeyCode_Alt, KeyCode_Shift);
    //Bind(place_in_scope,             KeyCode_ForwardSlash, KeyCode_Alt);
    Bind(delete_current_scope,       KeyCode_Minus, KeyCode_Alt);
    //Bind(open_file_in_quotes,        KeyCode_1, KeyCode_Alt);
    //Bind(open_matching_file_cpp,     KeyCode_2, KeyCode_Alt);
	
	// NOTE(Skytrias): panel options
	Bind(open_panel_vsplit, KeyCode_V, KeyCode_Alt);
	Bind(open_panel_hsplit, KeyCode_H, KeyCode_Alt);
	Bind(close_panel, KeyCode_C, KeyCode_Alt);
	Bind(st_passive_buffer_on, KeyCode_P, KeyCode_Alt);
	Bind(st_passive_buffer_off, KeyCode_P, KeyCode_Shift, KeyCode_Alt);
}

// NOTE(Skytrias): my custom settings when creating a new rs file, probably will change a lot
BUFFER_HOOK_SIG(st_new_rust_file){
    Scratch_Block scratch(app);
	
	String_Const_u8 file_name = push_buffer_base_name(app, scratch, buffer_id);
    if (string_match(string_postfix(file_name, 3), string_u8_litexpr(".rs"))) {
		Buffer_Insertion insert = begin_buffer_insertion_at_buffered(app, buffer_id, 0, scratch, KB(16));
		insertf(&insert,
				"use crate::engine::App;\n"
				"use crate::scripts::*;\n"
				"use crate::helpers::*;\n"
				"\n");
		end_buffer_insertion(&insert);
		
		View_ID view = get_active_view(app, Access_ReadWriteVisible);
		jump_to_location(app, view, buffer_id, 100);
	}
	
	return(0);
}

BUFFER_HOOK_SIG(st_begin_buffer){
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
					string_match(ext, string_u8_litexpr("nim")) ||
					string_match(ext, string_u8_litexpr("rs")) ||
					string_match(ext, string_u8_litexpr("toml")) ||
					string_match(ext, string_u8_litexpr("vert")) ||
					string_match(ext, string_u8_litexpr("frag")) ||
					string_match(ext, string_u8_litexpr("ron")) ||
					string_match(ext, string_u8_litexpr("odin")) ||
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

function Rect_f32 st_buffer_region(Application_Links *app, View_ID view_id, Rect_f32 region){
    Buffer_ID buffer = view_get_buffer(app, view_id, Access_Always);
    Face_ID face_id = get_face_id(app, buffer);
    Face_Metrics metrics = get_face_metrics(app, face_id);
    f32 line_height = metrics.line_height;
    f32 digit_advance = metrics.decimal_digit_advance;
	
    // NOTE(allen): margins
    region = rect_inner(region, 3.f);
	
    // NOTE(allen): file bar
    b64 showing_file_bar = false;
    if (view_get_setting(app, view_id, ViewSetting_ShowFileBar, &showing_file_bar) &&
        showing_file_bar){
        Rect_f32_Pair pair = layout_file_bar_on_top(region, line_height);
        region = pair.max;
    }
	
    // NOTE(allen): query bars
    {
        Query_Bar *space[32];
        Query_Bar_Ptr_Array query_bars = {};
        query_bars.ptrs = space;
        if (get_active_query_bars(app, view_id, ArrayCount(space), &query_bars)){
            Rect_f32_Pair pair = layout_query_bar_on_top(region, line_height, query_bars.count);
            region = pair.max;
        }
    }
	
    // NOTE(allen): FPS hud
    if (show_fps_hud){
        Rect_f32_Pair pair = layout_fps_hud_on_bottom(region, line_height);
        region = pair.min;
    }
	
    // NOTE(allen): line numbers
    if (global_config.show_line_number_margins){
        Rect_f32_Pair pair = layout_line_number_margin(app, buffer, region, digit_advance);
        region = pair.max;
    }
	
    return(region);
}