// add these to your lexer and rebuild it, additionaly you can color highlight them in get_token_color_cpp or elsewhere
/*
sm_op("[x]", "TaskComplete");
sm_op("[_]", "TaskCanceled");
sm_op("[!]", "TaskImportant");
*/

// all colors used, you can also replace these with defcolors defined by your editor
global u32 NOTE_COLOR = 0x11FFFFFF;

// globals
global b32 global_debug_sidebar = 0;
global View_ID global_todo_view = -1;
global b32 global_todo_margin_open = 0;
global i32 global_todo_margin_digit_width = 5;

// TODO(Skytrias): dont use define
#define COUNTER_AMOUNT 5
global i64 indent_counters[COUNTER_AMOUNT] = { 0, 0, 0, 0, 0 };

// helpers for generic tokens

// returns true if the token or the non whitespace token before is the kind 
function b32 st_token_white_it_dec(Token_Iterator_Array *it, Token_Cpp_Kind kind) {
	Token *token = token_it_read(it);
	if (token->sub_kind == kind) {
		return 1;
	} else {
		if (!token_it_dec_non_whitespace(it)) {
			return 0;
		}
		token = token_it_read(it);
		
		if (token->sub_kind == kind) {
			return 1;
		}
	}
	
	return 0;
}

// returns true if the token or the next non whitespace token is the kind 
function b32 st_token_white_it_inc(Token_Iterator_Array *it, Token_Cpp_Kind kind) {
	Token *token = token_it_read(it);
	if (token->sub_kind == kind) {
		return 1;
	} else {
		if (!token_it_inc_non_whitespace(it)) {
			return 0;
		}
		token = token_it_read(it);
		
		if (token->sub_kind == kind) {
			return 1;
		}
	}
	
	return 0;
}

// gets the non whitespace token at the end of the line
function b32 st_end_token_match(Application_Links *app, Buffer_ID buffer, Token_Array *array, i64 end_line_number, Token_Cpp_Kind kind) {
	if (array->tokens != 0){
		Token_Iterator_Array it = token_iterator_pos(buffer, array, end_line_number);
		return st_token_white_it_dec(&it, kind);
	}
	
	return 0;
}

// gets the non whitespace token at the start of the line
function b32 st_start_token_match(Application_Links *app, Buffer_ID buffer, Token_Array *array, i64 start_line_number, Token_Cpp_Kind kind) {
	if (array->tokens != 0){
		Token_Iterator_Array it = token_iterator_pos(buffer, array, start_line_number);
		return st_token_white_it_inc(&it, kind);
	}
	
	return 0;
}

// helpers for important tokens

// gets the non whitespace token at the start of the line
function b32 st_line_task_complete(Application_Links *app, Buffer_ID buffer, Token_Array *array, i64 start_line_number) {
	if (array->tokens != 0){
		Token_Iterator_Array it = token_iterator_pos(buffer, array, start_line_number);
		return st_token_white_it_inc(&it, TokenCppKind_TaskComplete);
	}
	
	return 0;
}

// gets the non whitespace token at the start of the line
function b32 st_line_task_canceled(Application_Links *app, Buffer_ID buffer, Token_Array *array, i64 start_line_number) {
	if (array->tokens != 0){
		Token_Iterator_Array it = token_iterator_pos(buffer, array, start_line_number);
		return st_token_white_it_inc(&it, TokenCppKind_TaskCanceled);
	}
	
	return 0;
}

// gets the non whitespace token at the start of the line
function b32 st_line_task_important(Application_Links *app, Buffer_ID buffer, Token_Array *array, i64 start_line_number) {
	if (array->tokens != 0){
		Token_Iterator_Array it = token_iterator_pos(buffer, array, start_line_number);
		return st_token_white_it_inc(&it, TokenCppKind_TaskImportant);
	}
	
	return 0;
}

// NOTE(Skytrias): expensive!!! also doesnt work correctly, like if a counter is 10 and has 2 digits it doesnt account for that
/*
function Rect_f32_Pair layout_todo_number_margin_expensive(Application_Links *app, Buffer_ID buffer, Rect_f32 rect, f32 digit_advance){
    i64 max_tab = 0;
	i64 line_count = buffer_get_line_count(app, buffer);
    
	if (line_count != 1) {
		for (i32 i = 1; i < line_count; i++) {
			i64 line_start = get_line_start_pos(app, buffer, i);
			Indent_Info indent_info = get_indent_info_line_start(app, buffer, line_start, 1);
			max_tab = Max(indent_info.indent_pos + 1, max_tab);
		}
	}
	
	return(rect_split_left_right(rect, digit_advance * max_tab));
}
*/

// width set via global, can be set via command
function Rect_f32_Pair st_layout_todo_number_margin(Application_Links *app, Rect_f32 rect, f32 digit_advance){
    return(rect_split_left_right(rect, digit_advance * global_todo_margin_digit_width));
}

function void st_draw_todo_numbers_margin(Application_Links *app, View_ID view_id, Buffer_ID buffer, 
										  Text_Layout_ID text_layout_id, Face_ID face_id, Rect_f32 margin) {
	Rect_f32 prev_clip = draw_set_clip(app, margin);
    draw_rectangle_fcolor(app, margin, 0.f, fcolor_id(defcolor_back));
	draw_rectangle_outline_fcolor(app, margin, 0.f, 2.0f, fcolor_id(defcolor_margin));
	
	// font info
	Face_Metrics face_metrics = get_face_metrics(app, face_id);
	f32 advance = face_metrics.normal_advance;
	f32 line_height = face_metrics.line_height;
	
	// buffer info
	i64 line_count = buffer_get_line_count(app, buffer);
    
	// save data
	i64 last_tab = 0;
	b32 inside_brace = 0;
	i64 brace_start = 0;
	i64 saved_counter = 0;
	
	Scratch_Block scratch(app);
	Token_Array token_array = get_token_array_from_buffer(app, buffer);
	
	// ranges
	Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);
	i64 view_line_start = get_line_number_from_pos(app, buffer, visible_range.min);
	i64 view_line_end = get_line_number_from_pos(app, buffer, visible_range.max);
	
    // start at 1, draw indent info for each line
	for (i32 i = 1; i < line_count; i++) {
		// skip blanks
		if (line_is_blank(app, buffer, i)) {
			// also reset all counters if not inside 
			if (!inside_brace) {
				for (i32 j = 0; j < COUNTER_AMOUNT; j+=1) {
					indent_counters[j] = 0;
				}
			}
			
			continue;
		}
		
		// line info
		i64 line_start = get_line_start_pos(app, buffer, i);
		i64 line_end = get_line_end_pos(app, buffer, i);
		Indent_Info indent_info = get_indent_info_line_number_and_start(app, buffer, i, line_start, 2);
		i64 tab = indent_info.indent_pos;
		
		// all tokens
		b32 brace_op_token = st_end_token_match(app, buffer, &token_array, line_end, TokenCppKind_BraceOp);
		b32 brace_cl_token = st_start_token_match(app, buffer, &token_array, line_start, TokenCppKind_BraceCl);
		b32 comma_token = st_end_token_match(app, buffer, &token_array, line_end, TokenCppKind_Comma);
		
		// continnue if no tokens were satisfied
		if (!brace_op_token && !brace_cl_token && !comma_token) {
			continue;
		}
		
		// if tab 0 do reset or save values of tokens
		if (tab == 0) {
			if (!inside_brace && brace_op_token) {
				inside_brace = 1;
				brace_start = i;
				
				// not necessary anymore, since you can just add an indent to all?
				if (st_start_token_match(app, buffer, &token_array, get_line_start_pos(app, buffer, i - 1), TokenCppKind_BraceCl)) {
					indent_counters[0] = saved_counter;
				}
			} 
			
			// if tab is 0 and token is cl, save the last indent_counters[0] and set inside_brace back
			if (brace_cl_token) {
				inside_brace = 0;
				saved_counter = indent_counters[0];
				
				// check if brace start and end is in range
				i64 start = brace_start - view_line_start - 1;
				i64 end = i - brace_start + 1;
				
				// TODO(Skytrias): somehow get this working to stop rendering rectangles at the right line positions
				// limits the outline to be drawn until you can't see them anymore, handtested by changing start and end
				//if (!(end < view_line_start || start > view_line_end)) {
				f32 offset = 0.0f;
				Rect_f32 rect = Rf32_xy_wh(
										   margin.x0, 
										   margin.y0 + start * line_height + offset, 
										   margin.x1 - 4.0f, 
										   end * line_height + offset
										   );
				
				draw_rectangle_outline_fcolor(app, rect, 0.f, 2.0f, fcolor_id(defcolor_pop2));
				draw_rectangle_fcolor(app, rect, 0.f, fcolor_change_alpha(fcolor_id(defcolor_pop2), 0.1f));
				//}
			}
		}
		
		// skip close braces lines
		if (brace_cl_token) {
			continue;
		}
		
		// reset all from the current tab if tabs have changed
		if (tab != last_tab) {
			for (i64 j = tab + 1; j < COUNTER_AMOUNT; j+=1) {
				indent_counters[j] = 0;
			}
		}
		last_tab = tab;
		
		// increase tab counter and push it to string 
		indent_counters[tab] += 1;
		
		// check if current line is in visible range of the buffer
		if (i > view_line_start && i < view_line_end) {
			String_Const_u8 result = push_u8_stringf(scratch, "%d", indent_counters[tab]);
			Range_f32 line_y = text_layout_line_on_screen(app, text_layout_id, i);
			// position has an x offset to make text readable on the left 
			Vec2_f32 pos = V2f32(margin.x0 + tab * advance + 3.0f, line_y.min);
			
			b32 task_complete_token = st_line_task_complete(app, buffer, &token_array, line_start);
			if (task_complete_token) {
				draw_string(app, face_id, result, pos, fcolor_id(defcolor_keyword));
			} else {
				draw_string(app, face_id, result, pos, fcolor_id(defcolor_pop1));
			}
		}
	}
	
	draw_set_clip(app, prev_clip);
}

function void st_draw_todo_tasks(Application_Links *app, View_ID view_id, Buffer_ID buffer, 
								 Text_Layout_ID text_layout_id, Face_ID face_id) {
	Scratch_Block scratch(app);
	Token_Array token_array = get_token_array_from_buffer(app, buffer);
	
	// font info
	Face_Metrics face_metrics = get_face_metrics(app, face_id);
	f32 advance = face_metrics.normal_advance;
	f32 line_height = face_metrics.line_height;
	
	// buffer info
	i64 line_count = buffer_get_line_count(app, buffer);
    
	// view info
	Rect_f32 margin = text_layout_region(app, text_layout_id);
	Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);
	i64 view_line_start = get_line_number_from_pos(app, buffer, visible_range.min);
	i64 view_line_end = get_line_number_from_pos(app, buffer, visible_range.max);
	
	// count all tasks per line in brace range
	for (i32 i = 1; i < line_count; i++) {
		// line info
		i64 line_start = get_line_start_pos(app, buffer, i);
		i64 line_end = get_line_end_pos(app, buffer, i);
		Indent_Info indent_info = get_indent_info_line_number_and_start(app, buffer, i, line_start, 2);
		i64 tab = indent_info.indent_pos;
		
		// all tokens
		b32 brace_op_token = st_end_token_match(app, buffer, &token_array, line_end, TokenCppKind_BraceOp);
		
		// skip draw if not in range
		if (line_is_blank(app, buffer, i)) {
			continue;
		}
		
		// add all amounts found in a brace and add them up
		i32 task_complete_amount = 0;
		i32 task_canceled_amount = 0;
		i32 task_amount = 0;
		if (brace_op_token) {
			for (i32 j = i; j < line_count; j++) {
				// line info
				i64 other_line_start = get_line_start_pos(app, buffer, j);
				i64 other_line_end = get_line_end_pos(app, buffer, j);
				Indent_Info other_indent_info = get_indent_info_line_number_and_start(app, buffer, j, other_line_start, 2);
				i64 other_tab = other_indent_info.indent_pos;
				
				// skip blanks
				if (line_is_blank(app, buffer, j)) {
					continue;
				}
				
				b32 other_brace_cl_token = st_start_token_match(app, buffer, &token_array, other_line_start, TokenCppKind_BraceCl);
				
				// skip entire loop of close happened at same tab
				if (other_brace_cl_token && other_tab == tab) {
					break;
				}
				
				// canceled
				{
					b32 task_canceled_token = st_line_task_canceled(app, buffer, &token_array, other_line_start);
					
					// count the tasks that are completed
					if (other_tab - 1 == tab && task_canceled_token) {
						task_canceled_amount += 1;
					}
				}
				
				// complete
				{
					b32 task_complete_token = st_line_task_complete(app, buffer, &token_array, other_line_start);
					
					// count the tasks that are canceled
					if (other_tab - 1 == tab && task_complete_token) {
						task_complete_amount += 1;
					}
				}
				
				// general
				{
					b32 other_comma_token = st_end_token_match(app, buffer, &token_array, other_line_end, TokenCppKind_Comma);
					
					// if non token found
					if (!other_comma_token && !other_brace_cl_token) {
						continue;
					}
				}
				
				// if the other searched tab is 1 wider, add to task amount
				if (other_tab - 1 == tab) {
					task_amount += 1;
				}
			}
		}
		
		// skip draw if not in range
		if (i < view_line_start || i > view_line_end) {
			continue;
		}
		
		// draw info somewhere
		f32 offset = tab * 4 * advance - 3.0f; 
		f32 roundndess = 0.0f; 
		
		// draw rect for all tasks
		if (task_amount != 0) {
			Range_f32 line_y = text_layout_line_on_screen(app, text_layout_id, i);
			Rect_f32 rect = Rf32_xy_wh(
									   margin.x0 + offset,
									   line_y.min,
									   (line_end - line_start) * advance,
									   line_height
									   );
			
			draw_rectangle_fcolor(app, rect, roundndess, fcolor_change_alpha(fcolor_id(defcolor_pop1), 0.1f));
		}
		
		// draw rect for all completed tasks at the beginning
		if (task_complete_amount != 0) {
			Range_f32 line_y = text_layout_line_on_screen(app, text_layout_id, i);
			Rect_f32 rect = Rf32_xy_wh(
									   margin.x0 + offset,
									   line_y.min,
									   ((line_end - line_start) * advance) * ((f32) task_complete_amount / (f32) task_amount),
									   line_height
									   );
			
			draw_rectangle_fcolor(app, rect, roundndess, fcolor_change_alpha(fcolor_id(defcolor_pop1), 0.2f));
		}
		
		// draw rect for all canceled tasks offset by the completed rect tasks
		if (task_canceled_amount != 0) {
			Range_f32 line_y = text_layout_line_on_screen(app, text_layout_id, i);
			f32 complete_offset = ((line_end - line_start) * advance) * ((f32) task_complete_amount / (f32) task_amount);
			
			Rect_f32 rect = Rf32_xy_wh(
									   margin.x0 + offset + complete_offset,
									   line_y.min,
									   ((line_end - line_start) * advance) * ((f32) task_canceled_amount / (f32) task_amount),
									   line_height
									   );
			
			draw_rectangle_fcolor(app, rect, roundndess, fcolor_change_alpha(fcolor_id(defcolor_keyword), 0.2f));
		}
	}
}

// commands for placing todo tokens, bind these to some keys for easier usage

function void st_replace_todo_token(Application_Links *app, String_Const_u8 todo_string) {
	View_ID view = get_active_view(app, Access_ReadWriteVisible);
	Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
	
	i64 pos = view_get_cursor_pos(app, view);
	i64 line_start_number = get_line_start_pos_from_pos(app, buffer, pos); 
	i64 new_pos = get_pos_past_lead_whitespace(app, buffer, line_start_number);
	
	Token_Array token_array = get_token_array_from_buffer(app, buffer);
	b32 complete_token = st_line_task_complete(app, buffer, &token_array, line_start_number); 
	b32 canceled_token = st_line_task_canceled(app, buffer, &token_array, line_start_number); b32 important_token = st_line_task_important(app, buffer, &token_array, line_start_number);
	
	if (complete_token || canceled_token || important_token) {
		buffer_replace_range(app, buffer, Ii64(new_pos, new_pos + todo_string.size), todo_string);
	} else {
		buffer_replace_range(app, buffer, Ii64(new_pos), todo_string);
	}
}

CUSTOM_COMMAND_SIG(st_todo_insert_task_complete)
CUSTOM_DOC("Inserts '[x] ' at the line start of the current cursor position if it doesn't exit already") {
	st_replace_todo_token(app, string_u8_litexpr("[x] "));
}

CUSTOM_COMMAND_SIG(st_todo_insert_task_canceled)
CUSTOM_DOC("Inserts '[_] ' at the line start of the current cursor position if it doesn't exit already") {
	st_replace_todo_token(app, string_u8_litexpr("[_] "));
}

CUSTOM_COMMAND_SIG(st_todo_insert_task_important)
CUSTOM_DOC("Inserts '[!] ' at the line start of the current cursor position if it doesn't exit already") {
	st_replace_todo_token(app, string_u8_litexpr("[!] "));
}

CUSTOM_COMMAND_SIG(st_todo_remove_task)
CUSTOM_DOC("Removes a Task if it exists at the start of the line") {
	View_ID view = get_active_view(app, Access_ReadWriteVisible);
	Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
	
	i64 pos = view_get_cursor_pos(app, view);
	i64 line_start_number = get_line_start_pos_from_pos(app, buffer, pos); 
	i64 new_pos = get_pos_past_lead_whitespace(app, buffer, line_start_number);
	
	Token_Array token_array = get_token_array_from_buffer(app, buffer);
	b32 complete_token = st_line_task_complete(app, buffer, &token_array, line_start_number); 
	b32 canceled_token = st_line_task_canceled(app, buffer, &token_array, line_start_number); b32 important_token = st_line_task_important(app, buffer, &token_array, line_start_number);
	
	if (complete_token || canceled_token || important_token) {
		// NOTE(Skytrias): dependent on the token size of "[*] "
		buffer_replace_range(app, buffer, Ii64(new_pos, new_pos + 4), string_u8_litexpr(""));
	} 
}

// TODO(Skytrias): find a better way to do this, i dont like to thought of counting all digits in the margin bar just to place it correctly

// TODO(Skytrias): digit_count_from_integer

CUSTOM_COMMAND_SIG(st_todo_margin_width_3)
CUSTOM_DOC("Sets the todo margin digit width to 3") {
	global_todo_margin_digit_width = 3;
}

CUSTOM_COMMAND_SIG(st_todo_margin_width_5)
CUSTOM_DOC("Sets the todo margin digit width to 5") {
	global_todo_margin_digit_width = 5;
}

CUSTOM_COMMAND_SIG(st_todo_margin_width_7)
CUSTOM_DOC("Sets the todo margin digit width to 7") {
	global_todo_margin_digit_width = 7;
}

CUSTOM_COMMAND_SIG(st_todo_margin_width_10)
CUSTOM_DOC("Sets the todo margin digit width to 10") {
	global_todo_margin_digit_width = 10;
}

CUSTOM_COMMAND_SIG(st_todo_toggle)
CUSTOM_DOC("Toggles the todo highlighting for the active view on/off") {
	global_todo_margin_open = !global_todo_margin_open;
	
	if (global_todo_margin_open) {
		global_todo_view = get_active_view(app, Access_ReadWriteVisible);
	} else {
		global_todo_view = -1;
	}
}

CUSTOM_COMMAND_SIG(st_todo_set_buffer)
CUSTOM_DOC("Toggles the todo highlighting for the active view on/off") {
	global_todo_margin_open = 1;
	global_todo_view = get_active_view(app, Access_ReadWriteVisible);
}

CUSTOM_UI_COMMAND_SIG(st_interactive_open_or_new)
CUSTOM_DOC("Interactively open a file out of the file system and turns on todo highlight on 'todo'")
{
    for (;;){
        Scratch_Block scratch(app);
        View_ID view = get_this_ctx_view(app, Access_Always);
        File_Name_Result result = get_file_name_from_user(app, scratch, "Open:",
                                                          view);
        if (result.canceled) break;
        
        String_Const_u8 file_name = result.file_name_activated;
        if (file_name.size == 0){
            file_name = result.file_name_in_text_field;
        }
        if (file_name.size == 0) break;
        
        String_Const_u8 path = result.path_in_text_field;
        String_Const_u8 full_file_name =
            push_u8_stringf(scratch, "%.*s/%.*s",
                            string_expand(path), string_expand(file_name));
        
        if (result.is_folder){
            set_hot_directory(app, full_file_name);
            continue;
        }
        
        Buffer_ID buffer = create_buffer(app, full_file_name, 0);
        if (buffer != 0){
            view_set_buffer(app, view, buffer, 0);
			
			// NOTE(Skytrias): add todo turn on automatically on "todo.*" file_name 
			if (string_match(string_file_without_extension(file_name), string_u8_litexpr("todo"))) {
				st_todo_set_buffer(app);
			}
		}
        break;
    }
}

// draw the line / scope highlight for important tasks
function void st_draw_todo_important_tasks(Application_Links *app, View_ID view_id, Buffer_ID buffer, Text_Layout_ID text_layout_id, Face_ID face_id) {
	Token_Array token_array = get_token_array_from_buffer(app, buffer);
	
	// buffer info
	i64 line_count = buffer_get_line_count(app, buffer);
    
	// ranges
	Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);
	i64 view_line_start = get_line_number_from_pos(app, buffer, visible_range.min);
	i64 view_line_end = get_line_number_from_pos(app, buffer, visible_range.max);
	
	// count all tasks per line in brace range
	for (i32 i = 1; i < line_count; i++) {
		if (line_is_blank(app, buffer, i)) {
			continue;
		}
		
		// line info
		i64 line_start = get_line_start_pos(app, buffer, i);
		i64 line_end = get_line_end_pos(app, buffer, i);
		Indent_Info indent_info = get_indent_info_line_number_and_start(app, buffer, i, line_start, 2);
		i64 tab = indent_info.indent_pos;
		
		
		// all tokens
		b32 brace_op_token = st_end_token_match(app, buffer, &token_array, line_end, TokenCppKind_BraceOp);
		b32 important_task_token = st_start_token_match(app, buffer, &token_array, line_start, TokenCppKind_TaskImportant);
		b32 comma_token = st_end_token_match(app, buffer, &token_array, line_end, TokenCppKind_Comma);
		
		// if task token and comma is there, highlight single line
		if (important_task_token && comma_token) {
			// skip line draw if not in range
			if (i < view_line_start || i > view_line_end) {
				continue;
			}
			
			Range_f32 y1 = text_layout_line_on_screen(app, text_layout_id, i);
			Range_f32 y = range_union(y1, y1);
			if (range_size(y) > 0.f){
				Rect_f32 region = text_layout_region(app, text_layout_id);
				draw_rectangle_fcolor(app, Rf32(rect_range_x(region), y), 0.f, fcolor_change_alpha(fcolor_id(defcolor_keyword), 0.1f));
			}
			
			continue;
		}
		
		// search for open and close brace on the same indent
		if (brace_op_token && important_task_token) {
			for (i32 j = i; j < line_count; j++) {
				// skip draw if not in range
				if (j < view_line_start || i > view_line_end) {
					break;
				}
				
				// line info
				i64 other_line_start = get_line_start_pos(app, buffer, j);
				Indent_Info other_indent_info = get_indent_info_line_number_and_start(app, buffer, j, other_line_start, 2);
				i64 other_tab = other_indent_info.indent_pos;
				
				if (tab == other_tab) {
					if (st_start_token_match(app, buffer, &token_array, other_line_start, TokenCppKind_BraceCl)) {
						
						Range_f32 y1 = text_layout_line_on_screen(app, text_layout_id, i);
						Range_f32 y2 = text_layout_line_on_screen(app, text_layout_id, j);
						Range_f32 y = range_union(y1, y2);
						if (range_size(y) > 0.f){
							Rect_f32 region = text_layout_region(app, text_layout_id);
							draw_rectangle_fcolor(app, Rf32(rect_range_x(region), y), 0.f, fcolor_change_alpha(fcolor_id(defcolor_keyword), 0.1f));
						}
						
						break;
					}
				}
			}
		}
	}
}

// draws note token line highlight
function void st_draw_todo_note(Application_Links *app, View_ID view_id, Buffer_ID buffer, Text_Layout_ID text_layout_id) {
	
    Token_Array array = get_token_array_from_buffer(app, buffer);
    if (array.tokens != 0){
        Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);
        i64 first_index = token_index_from_pos(&array, visible_range.first);
        Token_Iterator_Array it = token_iterator_index(0, &array, first_index);
		
        for (;;){
            Token *token = token_it_read(&it);
            if (token->pos >= visible_range.one_past_last){
                break;
            }
            
			// only draw line highlight once
			if (token->sub_kind == TokenCppKind_Note) {
				i64 line_number = get_line_number_from_pos(app, buffer, token->pos);
				draw_line_highlight(app, text_layout_id, line_number, NOTE_COLOR);
			}
			
			if (!token_it_inc_all(&it)){
				break;
			}
		}
	}
}
