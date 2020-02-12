function void
st_seek_string_insensitive_forward(Application_Links *app, Buffer_ID buffer, i64 pos, i64 end, String_Const_u8 needle, i64 *result){
    if (end == 0){
        end = (i32)buffer_get_size(app, buffer);
    }
	
	String_Match match = buffer_seek_string(app, buffer, needle, Scan_Forward, pos);
	if (match.range.first < end && match.buffer == buffer){
        if (st_has_line_comment_pos(app, buffer, match.range.first)) {
			st_seek_string_insensitive_forward(app, buffer, match.range.end + 1, 0, needle, result);
			return;
		}
		
		*result = match.range.first;
    }
    else{
        *result = buffer_get_size(app, buffer);
    }
}

function void
st_seek_string_insensitive_backward(Application_Links *app, Buffer_ID buffer, i64 pos, i64 min, String_Const_u8 needle, i64 *result){
    String_Match match = buffer_seek_string(app, buffer, needle, Scan_Backward, pos);
    if (match.range.first >= min && match.buffer == buffer){
        if (st_has_line_comment_pos(app, buffer, match.range.first)) {
			st_seek_string_insensitive_backward(app, buffer, match.range.first - 1, 0, needle, result);
			return;
		}
		
		*result = match.range.first;
    }
    else{
        *result = -1;
    }
}

// NOTE(Skytrias): manual center with a position that can be set yourself
function void 
st_center_view(Application_Links *app, View_ID view, i64 new_pos) {
	Rect_f32 region = view_get_buffer_region(app, view);
	Buffer_Cursor cursor = view_compute_cursor(app, view, seek_pos(new_pos));
	f32 view_height = rect_height(region);
	Buffer_Scroll scroll = view_get_buffer_scroll(app, view);
	scroll.target.line_number = cursor.line;
	scroll.target.pixel_shift.y = -view_height*0.5f;
	view_set_buffer_scroll(app, view, scroll, SetBufferScroll_SnapCursorIntoView);
}

// NOTE(Skytrias): search doesnt include any result in a comment line
function void
st_isearch(Application_Links *app, Scan_Direction start_scan, i64 first_pos,
		   String_Const_u8 query_init){
    View_ID view = get_active_view(app, Access_ReadVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
    if (!buffer_exists(app, buffer)){
        return;
    }
	
    i64 buffer_size = buffer_get_size(app, buffer);
	
    Query_Bar_Group group(app);
    Query_Bar bar = {};
    if (start_query_bar(app, &bar, 0) == 0){
        return;
    }
	
    Scan_Direction scan = start_scan;
    i64 pos = first_pos;
	
    u8 bar_string_space[256];
    bar.string = SCu8(bar_string_space, query_init.size);
    block_copy(bar.string.str, query_init.str, query_init.size);
	
    String_Const_u8 isearch_str = string_u8_litexpr("I-Search: ");
    String_Const_u8 rsearch_str = string_u8_litexpr("Reverse-I-Search: ");
	
    u64 match_size = bar.string.size;
	
    User_Input in = {};
    for (;;){
        switch (scan){
            case Scan_Forward:
            {
                bar.prompt = isearch_str;
            }break;
            case Scan_Backward:
            {
                bar.prompt = rsearch_str;
            }break;
        }
        isearch__update_highlight(app, view, Ii64_size(pos, match_size));
		
        in = get_next_input(app, EventPropertyGroup_AnyKeyboardEvent,
                            EventProperty_Escape|EventProperty_ViewActivation);
        if (in.abort){
            break;
        }
		
		String_Const_u8 string = to_writable(&in);
		
        b32 string_change = false;
        if (match_key_code(&in, KeyCode_Return) ||
            match_key_code(&in, KeyCode_Tab)){
            Input_Modifier_Set *mods = &in.event.key.modifiers;
            if (has_modifier(mods, KeyCode_Control)){
                bar.string.size = cstring_length(previous_isearch_query);
                block_copy(bar.string.str, previous_isearch_query, bar.string.size);
            }
            else{
                u64 size = bar.string.size;
                size = clamp_top(size, sizeof(previous_isearch_query) - 1);
                block_copy(previous_isearch_query, bar.string.str, size);
                previous_isearch_query[size] = 0;
                break;
            }
        }
        else if (string.str != 0 && string.size > 0){
            String_u8 bar_string = Su8(bar.string, sizeof(bar_string_space));
            string_append(&bar_string, string);
            bar.string = bar_string.string;
            string_change = true;
        }
        else if (match_key_code(&in, KeyCode_Backspace)){
            if (is_unmodified_key(&in.event)){
                u64 old_bar_string_size = bar.string.size;
                bar.string = backspace_utf8(bar.string);
                string_change = (bar.string.size < old_bar_string_size);
            }
            else if (has_modifier(&in.event.key.modifiers, KeyCode_Control)){
                if (bar.string.size > 0){
                    string_change = true;
                    bar.string.size = 0;
                }
            }
        }
		
        // TODO(allen): how to detect if the input corresponds to
        // a search or rsearch command, a scroll wheel command?
		
        b32 do_scan_action = false;
        b32 do_scroll_wheel = false;
        Scan_Direction change_scan = scan;
        if (!string_change){
            if (match_key_code(&in, KeyCode_PageDown) ||
                match_key_code(&in, KeyCode_Down)){
                change_scan = Scan_Forward;
                do_scan_action = true;
            }
            if (match_key_code(&in, KeyCode_PageUp) ||
                match_key_code(&in, KeyCode_Up)){
                change_scan = Scan_Backward;
                do_scan_action = true;
            }
			
#if 0
            if (in.command == mouse_wheel_scroll){
                do_scroll_wheel = true;
            }
#endif
        }
		
        if (string_change){
            switch (scan){
                case Scan_Forward:
                {
                    i64 new_pos = 0;
                    st_seek_string_insensitive_forward(app, buffer, pos - 1, 0, bar.string, &new_pos);
                    if (new_pos != -1 && new_pos < buffer_size){
                        st_center_view(app, view, new_pos);
						pos = new_pos;
                        match_size = bar.string.size;
                    }
                }break;
				
                case Scan_Backward:
                {
                    i64 new_pos = 0;
                    st_seek_string_insensitive_backward(app, buffer, pos + 1, 0, bar.string, &new_pos);
                    if (new_pos != -1 && new_pos >= 0){
                        st_center_view(app, view, new_pos);
						pos = new_pos;
                        match_size = bar.string.size;
                    }
                }break;
            }
        }
        else if (do_scan_action){
            scan = change_scan;
            switch (scan){
                case Scan_Forward:
                {
                    i64 new_pos = 0;
                    st_seek_string_insensitive_forward(app, buffer, pos, 0, bar.string, &new_pos);
                    if (new_pos != -1 && new_pos < buffer_size){
                        st_center_view(app, view, new_pos);
						pos = new_pos;
                        match_size = bar.string.size;
                    }
                }break;
				
                case Scan_Backward:
                {
                    i64 new_pos = 0;
                    st_seek_string_insensitive_backward(app, buffer, pos, 0, bar.string, &new_pos);
                    if (new_pos != -1 && new_pos >= 0){
                        st_center_view(app, view, new_pos);
						pos = new_pos;
                        match_size = bar.string.size;
                    }
                }break;
            }
        }
        else if (do_scroll_wheel){
            mouse_wheel_scroll(app);
        }
        else{
            leave_current_input_unhandled(app);
        }
    }
	
    view_disable_highlight_range(app, view);
	
    if (in.abort){
        u64 size = bar.string.size;
        size = clamp_top(size, sizeof(previous_isearch_query) - 1);
        block_copy(previous_isearch_query, bar.string.str, size);
        previous_isearch_query[size] = 0;
        view_set_cursor_and_preferred_x(app, view, seek_pos(first_pos));
    }
}

function void
st_isearch(Application_Links *app, Scan_Direction start_scan, String_Const_u8 query_init){
    View_ID view = get_active_view(app, Access_ReadVisible);
    i64 pos = view_get_cursor_pos(app, view);;
    st_isearch(app, start_scan, pos, query_init);
}

function void
st_isearch(Application_Links *app, Scan_Direction start_scan){
    View_ID view = get_active_view(app, Access_ReadVisible);
    i64 pos = view_get_cursor_pos(app, view);;
    st_isearch(app, start_scan, pos, SCu8());
}

function void
st_isearch_identifier(Application_Links *app, Scan_Direction scan){
    View_ID view = get_active_view(app, Access_ReadVisible);
    Buffer_ID buffer_id = view_get_buffer(app, view, Access_ReadVisible);
    i64 pos = view_get_cursor_pos(app, view);
    Scratch_Block scratch(app);
    Range_i64 range = enclose_pos_alpha_numeric_underscore(app, buffer_id, pos);
    String_Const_u8 query = push_buffer_range(app, scratch, buffer_id, range);
    st_isearch(app, scan, range.first, query);
}

CUSTOM_COMMAND_SIG(st_search)
CUSTOM_DOC("Begins an incremental search down through the current buffer for a user specified string.")
{
    st_isearch(app, Scan_Forward);
}

CUSTOM_COMMAND_SIG(st_reverse_search)
CUSTOM_DOC("Begins an incremental search up through the current buffer for a user specified string.")
{
    st_isearch(app, Scan_Backward);
}

CUSTOM_COMMAND_SIG(st_search_identifier)
CUSTOM_DOC("Begins an incremental search down through the current buffer for the word or token under the cursor.")
{
    st_isearch_identifier(app, Scan_Forward);
}