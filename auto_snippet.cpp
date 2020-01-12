static u32 SNIPPET_HIGHLIGHT_COLOR = 0x3325B2BC;

// highlights the word youre writing  
global b32 global_snippet_word_highlight_on = 1; // NOTE(Skytrias): DISABLE this if you dont want words to be highlighted, this just helps a lot to see bugs appear, 
global Range_i64 global_snippet_cursor_range = {};
global b32 global_snippet_cursor_set = 0;
// if the start of the word you write doesnt match the current one, stop auto snippet
global i64 global_snippet_start_line = 0;

function void
st_write_text(Application_Links *app, String_Const_u8 insert){
    ProfileScope(app, "write character");
    if (insert.str != 0 && insert.size > 0){
        View_ID view = get_active_view(app, Access_ReadWriteVisible);
        if_view_has_highlighted_range_delete_range(app, view);
        
        Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
        
        i64 pos = view_get_cursor_pos(app, view);
        pos = view_get_character_legal_pos_from_pos(app, view, pos);
        
		// NOTE(Skytrias): save position of character written for snippet automation 
		{
			char c = insert.str[0];
			
			// these characters will break up the range setting
			if (c == ' ' ||
				c == '_' ||
				c == ':' ||
				c == ';' ||
				c == '\t' ||
				c == '\n') {
				global_snippet_cursor_set = 0;
			} else {
				// if nothing has been set, set it and its first position
				if (!global_snippet_cursor_set) {
					global_snippet_cursor_set = 1;
					global_snippet_cursor_range.start = pos;
					global_snippet_start_line = get_line_number_from_pos(app, buffer, pos);
				} else {
					// reset cursor range when any break happens
					if (c == ',' ||
						c == '.') {
						global_snippet_cursor_range.start = pos;
					}
				}
			}
		}
		
        // NOTE(allen): consecutive inserts merge logic
        History_Record_Index first_index = buffer_history_get_current_state_index(app, buffer);
        b32 do_merge = false;
        if (insert.str[0] != '\n'){
            Record_Info record = get_single_record(app, buffer, first_index);
            if (record.error == RecordError_NoError && record.kind == RecordKind_Single){
                String_Const_u8 string = record.single_string_forward;
                i32 last_end = (i32)(record.single_first + string.size);
                if (last_end == pos && string.size > 0){
                    char c = string.str[string.size - 1];
                    if (c != '\n'){
                        if (character_is_whitespace(insert.str[0]) &&
                            character_is_whitespace(c)){
                            do_merge = true;
                        }
                        else if (character_is_alpha_numeric(insert.str[0]) && character_is_alpha_numeric(c)){
                            do_merge = true;
                        }
                    }
                }
            }
        }
        
        // NOTE(allen): perform the edit
        b32 edit_success = buffer_replace_range(app, buffer, Ii64(pos), insert);
        
        // NOTE(allen): finish merging records if necessary
        if (do_merge){
            History_Record_Index last_index = buffer_history_get_current_state_index(app, buffer);
            buffer_history_merge_record_range(app, buffer, first_index, last_index, RecordMergeFlag_StateInRange_MoveStateForward);
        }
        
        // NOTE(allen): finish updating the cursor
        if (edit_success){
            view_set_cursor_and_preferred_x(app, view, seek_pos(pos + insert.size));
        }
    }
}

// NOTE(Skytrias): the custom command must also go to bindings.cpp
CUSTOM_COMMAND_SIG(st_write_text_input)
CUSTOM_DOC("Inserts whatever text was used to trigger this command.")
{
    User_Input in = get_current_input(app);
    String_Const_u8 insert = to_writable(&in);
    st_write_text(app, insert);
}

CUSTOM_COMMAND_SIG(st_write_text_and_auto_indent)
CUSTOM_DOC("Inserts text and auto-indents the line on which the cursor sits if any of the text contains 'layout punctuation' such as ;:{}()[]# and new lines.")
{
    User_Input in = get_current_input(app);
    String_Const_u8 insert = to_writable(&in);
    if (insert.str != 0 && insert.size > 0){
        b32 do_auto_indent = false;
        for (u64 i = 0; !do_auto_indent && i < insert.size; i += 1){
            switch (insert.str[i]){
                case ';': case ':':
                case '{': case '}':
                case '(': case ')':
                case '[': case ']':
                case '#':
                case '\n': case '\t':
                {
                    do_auto_indent = true;
                }break;
            }
        }
        if (do_auto_indent){
            View_ID view = get_active_view(app, Access_ReadWriteVisible);
            Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
            Range_i64 pos = {};
            pos.min = view_get_cursor_pos(app, view);
            st_write_text_input(app);
            pos.max= view_get_cursor_pos(app, view);
            auto_indent_buffer(app, buffer, pos, 0);
            move_past_lead_whitespace(app, view, buffer);
        }
        else{
            st_write_text_input(app);
        }
    }
}

// loop to set the cursor end of the word you're typing, cancels the write at certian characters
function void st_auto_snippet(Application_Links *app, View_ID view_id, Buffer_ID buffer, Face_ID face_id, Text_Layout_ID text_layout_id) {
	// dont allow snippet autocomplete when no existasdasda
	if (global_snippet_count < 0) {
		global_snippet_cursor_set = false;
		return;
	}
	
	if (global_snippet_cursor_set) {
		i64 cursor_pos = view_get_cursor_pos(app, view_id);
		
		// find out if there is a comment at the start of the line
		i64 current_line_number = get_line_number_from_pos(app, buffer, cursor_pos);
		
		// additional check for line numbers, is helpful if you move after writing
		if (current_line_number != global_snippet_start_line) {
			global_snippet_cursor_set = false;
			return;
		}
		
		// NOTE(Skytrias): forced to do this or you could wrap all base_commands disable global_snippet_cursor_set each time it doesnt write
		global_snippet_cursor_range.end = cursor_pos;
		
		// stop if snippet set when word is bigger than 10 characters or goes minus!
		i64 diff = global_snippet_cursor_range.end - global_snippet_cursor_range.start;
		if (diff < 0 || diff > 10) {
			global_snippet_cursor_set = false;
			return;
		}
		
		Scratch_Block scratch(app);
		
		if (st_has_line_comment(app, buffer, current_line_number)) {
			global_snippet_cursor_set = false;
		}
		
		// visual help
		if (global_snippet_word_highlight_on) {
			// Simple rect 
			Rect_f32 character_rect = text_layout_character_on_screen(app, text_layout_id, global_snippet_cursor_range.start);
			Face_Metrics face_metrics = get_face_metrics(app, face_id);
			f32 x = character_rect.x0;
			f32 y = character_rect.y0;
			f32 w = character_rect.x0 + (global_snippet_cursor_range.end - global_snippet_cursor_range.start) * face_metrics.space_advance;
			f32 h = character_rect.y1;
			Rect_f32 rect = { x, y, w, h };
			draw_rectangle(app, rect, 4.0f, SNIPPET_HIGHLIGHT_COLOR);
		}
		
		String_Const_u8 result = string_u8_empty;
		
		// TODO(Skytrias): simplify?
		i64 length = range_size(global_snippet_cursor_range);
		if (length > 0){
			Temp_Memory restore_point = begin_temp(scratch);
			u8 *memory = push_array(scratch, u8, length);
			if (buffer_read_range(app, buffer, global_snippet_cursor_range, memory)){
				result = SCu8(memory, length);
			} else{
				end_temp(restore_point);
			}
		}
		
		if (result.size > 0) {
			// remove any whitespace 
			result = string_skip_whitespace(result);
			
			// loop through snippet names and match with result 
			Snippet *snippet = default_snippets;
			for (i32 i = 0; i < global_snippet_count; i += 1, snippet += 1){
				if (string_match(result, SCu8(snippet->name))){
					write_snippet(app, view_id, buffer, cursor_pos, snippet);
					buffer_replace_range(app, buffer, global_snippet_cursor_range, string_u8_empty);
					global_snippet_cursor_set = false;
					break;
				}
			}
		}
	}		
}		
