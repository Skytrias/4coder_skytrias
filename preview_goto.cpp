// NOTE(Skytrias): changes the goto_line function to move to the line while previewing

internal b32
st_query_user_general(Application_Links *app, Query_Bar *bar, View_ID view, b32 force_number){
    // NOTE(allen|a3.4.4): It will not cause an *error* if we continue on after failing to.
    // start a query bar, but it will be unusual behavior from the point of view of the
    // user, if this command starts intercepting input even though no prompt is shown.
    // This will only happen if you have a lot of bars open already or if the current view
    // doesn't support query bars.
    if (start_query_bar(app, bar, 0) == 0){
        return(false);
    }
    
    b32 success = true;
    
    for (;;){
        // NOTE(allen|a3.4.4): This call will block until the user does one of the input
        // types specified in the flags.  The first set of flags are inputs you'd like to
        // intercept that you don't want to abort on.  The second set are inputs that
        // you'd like to cause the command to abort.  If an event satisfies both flags, it
        // is treated as an abort.
        User_Input in = get_next_input(app, EventPropertyGroup_AnyKeyboardEvent,
                                       EventProperty_Escape|EventProperty_MouseButton);
        
        // NOTE(allen|a3.4.4): The responsible thing to do on abort is to end the command
        // without waiting on get_next_input again.
        if (in.abort){
            success = false;
            break;
        }
        
        Scratch_Block scratch(app);
        b32 good_insert = false;
        String_Const_u8 insert_string = to_writable(&in);
        if (insert_string.str != 0 && insert_string.size > 0){
            insert_string = string_replace(scratch, insert_string,
                                           string_u8_litexpr("\n"),
                                           string_u8_litexpr(""));
            insert_string = string_replace(scratch, insert_string,
                                           string_u8_litexpr("\t"),
                                           string_u8_litexpr(""));
            if (force_number){
                if (string_is_integer(insert_string, 10)){
                    good_insert = true;
                }
            }
            else{
                good_insert = true;
            }
        }
        
        // NOTE(allen|a3.4.4): All we have to do to update the query bar is edit our
        // local Query_Bar struct!  This is handy because it means our Query_Bar
        // is always correct for typical use without extra work updating the bar.
        if (in.event.kind == InputEventKind_KeyStroke &&
            (in.event.key.code == KeyCode_Return || in.event.key.code == KeyCode_Tab)){
            break;
        }
        else if (in.event.kind == InputEventKind_KeyStroke &&
                 in.event.key.code == KeyCode_Backspace){
            bar->string = backspace_utf8(bar->string);
        }
        else if (good_insert){
            String_u8 string = Su8(bar->string.str, bar->string.size, bar->string_capacity);
            string_append(&string, insert_string);
            bar->string = string.string;
			
			i32 line_number = (i32) string_to_integer(bar->string, 10);
			view_set_cursor_and_preferred_x(app, view, seek_line_col(line_number, 0));
			center_view(app);
		}
        else{
            leave_current_input_unhandled(app);
        }
    }
    
    return(success);
}

CUSTOM_COMMAND_SIG(st_goto_line)
CUSTOM_DOC("Queries the user for a number, and jumps the cursor to the corresponding line.")
{
    Query_Bar_Group group(app);
    u8 string_space[256];
    Query_Bar bar = {};
    bar.prompt = string_u8_litexpr("Goto Line: ");
    bar.string = SCu8(string_space, (u64)0);
    bar.string_capacity = sizeof(string_space);
	
	View_ID view = get_active_view(app, Access_ReadVisible);
	Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
	i64 cursor_pos = view_get_cursor_pos(app, view);
	i64 start_line_number = get_line_number_from_pos(app, buffer, cursor_pos);
	
	// if query stops in between, go back to the starting line number 
	if (!st_query_user_general(app, &bar, view, true)) {
        view_set_cursor_and_preferred_x(app, view, seek_line_col(start_line_number, 0));
		center_view(app);
	}
}
