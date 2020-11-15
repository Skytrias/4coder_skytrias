static u32 SNIPPET_HIGHLIGHT_COLOR = 0x3325B2BC;

// highlights the word youre writing
global b32 global_snippet_word_highlight_on = 1; // NOTE(Skytrias): DISABLE this if you dont want words to be highlighted, this just helps a lot to see bugs appear,
global Range_i64 global_snippet_cursor_range = {};
global b32 global_snippet_cursor_set = 0;
// if the start of the word you write doesnt match the current one, stop auto snippet
global i64 global_snippet_start_line = 0;
// check to see if the pos has changed, if not dont do string comparison
global i64 global_snippet_last_end_pos = 0;

function void
st_write_text(Application_Links *app, String_Const_u8 insert){
    ProfileScope(app, "write character");
    if (insert.str != 0 && insert.size > 0){
        View_ID view = get_active_view(app, Access_ReadWriteVisible);
        if_view_has_highlighted_range_delete_range(app, view);
        
        Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
        
        i64 pos = view_get_cursor_pos(app, view);
        pos = view_get_character_legal_pos_from_pos(app, view, pos);
		
        // NOTE(Skytrias): insert more characters automatically, pos set to pos + 1 if one_left
        b32 one_left = 0;
        {
            char c = insert.str[0];
            if (c == '\"') {
                insert = string_u8_litexpr("\"\"");
                one_left = 1;
            }
            
            if (c == '[') {
                insert = string_u8_litexpr("[]");
                one_left = 1;
            }
            
            if (c == '(') {
                insert = string_u8_litexpr("()");
                one_left = 1;
            }
            
            if (c == '\'') {
                insert = string_u8_litexpr("\'\'");
                one_left = 1;
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
		
        b32 edit_success = buffer_replace_range(app, buffer, Ii64(pos), insert);
        
		// NOTE(allen): finish merging records if necessary
        if (do_merge){
            History_Record_Index last_index = buffer_history_get_current_state_index(app, buffer);
            buffer_history_merge_record_range(app, buffer, first_index, last_index, RecordMergeFlag_StateInRange_MoveStateForward);
        }
        
        // NOTE(allen): finish updating the cursor
        if (edit_success){
            view_set_cursor_and_preferred_x(app, view, seek_pos(one_left ? pos + one_left : pos + insert.size));
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
