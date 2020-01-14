// code peek
static f32 global_code_peek_height = 400.0f; // NOTE(Skytrias): height of code peek
static b32 global_code_peek_open = 0;
static int global_code_peek_match_count = 0;
String_Match global_code_peek_matches[16] = {0};
static int global_code_peek_selected_index = -1;
static f32 global_code_peek_open_transition = 0.f;
static Range_i64 global_code_peek_token_range;
global f32 global_code_peek_slide = 0.0f;

// ryan
static void Fleury4OpenCodePeek(Application_Links *app, String_Const_u8 base_needle, String_Match_Flag must_have_flags, String_Match_Flag must_not_have_flags, Buffer_ID current_buffer);
static void Fleury4CloseCodePeek(void);
static void Fleury4NextCodePeek(void);
static void st_backwards_code_peek();
static void Fleury4CodePeekGo(Application_Links *app);

static ARGB_Color
ARGBFromID(Managed_ID id)
{
    return fcolor_resolve(fcolor_id(id));
}

static String_Const_u8_Array
Fleury4MakeTypeSearchList(Application_Links *app, Arena *arena, String_Const_u8 base_needle)
{
    String_Const_u8_Array result = {0};
    if(base_needle.size > 0)
    {
        result.count = 9;
        result.vals = push_array(arena, String_Const_u8, result.count);
        i32 i = 0;
        result.vals[i++] = (push_u8_stringf(arena, "struct %.*s{"  , string_expand(base_needle)));
        result.vals[i++] = (push_u8_stringf(arena, "struct %.*s\n{", string_expand(base_needle)));
        result.vals[i++] = (push_u8_stringf(arena, "struct %.*s {" , string_expand(base_needle)));
        result.vals[i++] = (push_u8_stringf(arena, "union %.*s{"   , string_expand(base_needle)));
        result.vals[i++] = (push_u8_stringf(arena, "union %.*s\n{" , string_expand(base_needle)));
        result.vals[i++] = (push_u8_stringf(arena, "union %.*s {"  , string_expand(base_needle)));
        result.vals[i++] = (push_u8_stringf(arena, "enum %.*s{"    , string_expand(base_needle)));
        result.vals[i++] = (push_u8_stringf(arena, "enum %.*s\n{"  , string_expand(base_needle)));
        result.vals[i++] = (push_u8_stringf(arena, "enum %.*s {"   , string_expand(base_needle)));
        Assert(i == result.count);
    }
    return(result);
}


CUSTOM_COMMAND_SIG(st_backwards_code_peek)
CUSTOM_DOC("Goes back in code peek")
{
	View_ID view = get_active_view(app, Access_ReadWriteVisible);
    i64 pos = view_get_cursor_pos(app, view);
	
	if(global_code_peek_open && pos >= global_code_peek_token_range.start &&
       pos <= global_code_peek_token_range.end)
    {
		st_backwards_code_peek();
	}
}

CUSTOM_COMMAND_SIG(fleury_code_peek)
CUSTOM_DOC("Opens code peek.")
{
    View_ID view = get_active_view(app, Access_ReadWriteVisible);
    i64 pos = view_get_cursor_pos(app, view);
    if(global_code_peek_open && pos >= global_code_peek_token_range.start &&
       pos <= global_code_peek_token_range.end)
    {
        Fleury4NextCodePeek();
    }
    else
    {
        Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
        Scratch_Block scratch(app);
        Range_i64 range = enclose_pos_alpha_numeric_underscore(app, buffer, pos);
        global_code_peek_token_range = range;
        String_Const_u8 base_needle = push_token_or_word_under_active_cursor(app, scratch);
        Fleury4OpenCodePeek(app, base_needle, StringMatch_CaseSensitive, StringMatch_LeftSideSloppy | StringMatch_RightSideSloppy, buffer);
    }
}

CUSTOM_COMMAND_SIG(fleury_close_code_peek)
CUSTOM_DOC("Closes code peek.")
{
    if(global_code_peek_open)
    {
        Fleury4CloseCodePeek();
    }
    else
    {
        leave_current_input_unhandled(app);
    }
}

CUSTOM_COMMAND_SIG(fleury_code_peek_go)
CUSTOM_DOC("Goes to the active code peek.")
{
    Fleury4CodePeekGo(app);
}


static void
Fleury4OpenCodePeek(Application_Links *app, String_Const_u8 base_needle,
                    String_Match_Flag must_have_flags, String_Match_Flag must_not_have_flags, Buffer_ID current_buffer_id)
{
    global_code_peek_match_count = 0;
    global_code_peek_slide = 1.0f;
    global_code_peek_open_transition = 0.f;
    
    Scratch_Block scratch(app);
    String_Const_u8_Array type_array = Fleury4MakeTypeSearchList(app, scratch, base_needle);
    String_Match_List matches = find_all_matches_all_buffers(app, scratch, type_array, must_have_flags, must_not_have_flags);
    
    matches = find_all_matches_all_buffers(app, scratch, base_needle, must_have_flags, must_not_have_flags);
    string_match_list_filter_remove_buffer_predicate(app, &matches, buffer_has_name_with_star);
    
	// NOTE(Skytrias): exclude current view buffer from all results
	string_match_list_filter_remove_buffer(&matches, current_buffer_id);
	
    if(global_code_peek_match_count == 0)
    {
        for(String_Match *match = matches.first; match; match = match->next)
        {
			global_code_peek_matches[global_code_peek_match_count++] = *match;
            if(global_code_peek_match_count >= sizeof(global_code_peek_matches)/sizeof(global_code_peek_matches[0]))
            {
                break;
            }
        }
    }
    
    if(global_code_peek_match_count > 0)
    {
        global_code_peek_selected_index = 0;
        global_code_peek_open = 1;
    }
    else
    {
        global_code_peek_selected_index = -1;
        global_code_peek_open = 0;
    }
}

static void
Fleury4CloseCodePeek(void)
{
    global_code_peek_open = 0;
}

static void
Fleury4NextCodePeek(void)
{
    if(++global_code_peek_selected_index >= global_code_peek_match_count)
    {
        global_code_peek_selected_index = 0;
    }
    
    if(global_code_peek_selected_index >= global_code_peek_match_count)
    {
        global_code_peek_selected_index = -1;
        global_code_peek_open = 0;
    }
}

// NOTE(Skytrias): goes backwards in the code peek indexes list
static void st_backwards_code_peek(void) {
    if (--global_code_peek_selected_index < 0) {
        global_code_peek_selected_index = global_code_peek_match_count;
	}
}

static void
Fleury4CodePeekGo(Application_Links *app)
{
    if(global_code_peek_selected_index >= 0 && global_code_peek_selected_index < global_code_peek_match_count &&
       global_code_peek_match_count > 0)
    {
        View_ID view = get_active_view(app, Access_Always);
        String_Match *match = &global_code_peek_matches[global_code_peek_selected_index];
        view = get_next_view_looped_primary_panels(app, view, Access_Always);
        view_set_buffer(app, view, match->buffer, 0);
        i64 line_number = get_line_number_from_pos(app, match->buffer, match->range.start);
        Buffer_Scroll scroll = view_get_buffer_scroll(app, view);
        scroll.position.line_number = scroll.target.line_number = line_number;
        view_set_buffer_scroll(app, view, scroll, SetBufferScroll_SnapCursorIntoView);
        Fleury4CloseCodePeek();
    }
}

static void
st_render_code_peek(Application_Links *app, View_ID view_id, Face_ID face_id, Buffer_ID buffer,
					Frame_Info frame_info)
{
    if(global_code_peek_open &&
       global_code_peek_selected_index >= 0 &&
       global_code_peek_selected_index < global_code_peek_match_count)
    {
        String_Match *match = &global_code_peek_matches[global_code_peek_selected_index];
        
        global_code_peek_open_transition += (1.f - global_code_peek_open_transition) * frame_info.animation_dt * 14.f;
        if(fabs(global_code_peek_open_transition - 1.f) > 0.005f)
        {
            animate_in_n_milliseconds(app, 0);
        }
        
		// NOTE(Skytrias): write code_peek into the bottom of the panel, easier to read imo
		// similar to file_bar draw
		Rect_f32 whole_rect = view_get_screen_rect(app, view_id);
		Rect_f32 inner_rect = rect_inner(whole_rect, 3.f); // cut of the outline from 4coder default
		Rect_f32_Pair bottom_rect = rect_split_top_bottom_neg(inner_rect, global_code_peek_height);
		// animaate y0 to come from the bottom to its position
		
		// NOTE(Skytrias): starts at 1 and goes to 0
		f32 speed = 2.5f;
		if (global_code_peek_slide > 0.0f) {
			global_code_peek_slide -= frame_info.animation_dt * speed;
		} else {
			global_code_peek_slide = 0.0f;
		}
		
		bottom_rect.max.y0 += bottom_rect.max.y1 * Max(0.0f, global_code_peek_slide);
		draw_rectangle(app, bottom_rect.max, 0.0f, fcolor_resolve(fcolor_id(defcolor_back)));
		draw_rectangle_outline(app, bottom_rect.max, 0.0f, 3.0f, fcolor_resolve(fcolor_id(defcolor_pop2)));
		Rect_f32 rect = bottom_rect.max;
        
		// NOTE(Skytrias): custom layout and draw of panel
        if(rect.y1 - rect.y0 > 60.f)
        {
			rect.x0 += 10;
			rect.y0 += 10;
			rect.x1 -= 10;
			rect.y1 -= 10;
            
            Scratch_Block scratch(app);
			Fancy_Line list = {};
			FColor base_color = fcolor_id(defcolor_base);
            
			// file name of buffer
			String_Const_u8 unique_name = push_buffer_unique_name(app, scratch, match->buffer);
			push_fancy_string(scratch, &list, base_color, unique_name);
            
			//push_fancy_string(scratch, &list, base_color, string_u8_litexpr(" testing"));
			push_fancy_stringf(scratch, &list, base_color, " - Match: %d of %d", global_code_peek_selected_index + 1,
                               global_code_peek_match_count);
            
			draw_fancy_line(app, face_id, fcolor_zero(), &list, rect.p0);
            
			rect.y0 += 25;
            
			Buffer_Point buffer_point =
			{
				get_line_number_from_pos(app, match->buffer, match->range.start),
				0,
			};
			Text_Layout_ID text_layout_id = text_layout_create(app, match->buffer, rect, buffer_point);
            
			Rect_f32 prev_clip = draw_set_clip(app, rect);
			{
				Token_Array token_array = get_token_array_from_buffer(app, match->buffer);
				if(token_array.tokens != 0)
				{
                    st_draw_cpp_token_colors(app, text_layout_id, &token_array);
				}
				else
				{
					Range_i64 visible_range = match->range;
					paint_text_color_fcolor(app, text_layout_id, visible_range, fcolor_id(defcolor_text_default));
				}
                
				draw_text_layout_default(app, text_layout_id);
			}
			draw_set_clip(app, prev_clip);
            text_layout_free(app, text_layout_id);
        }
    }
    else
    {
        global_code_peek_open_transition = 0.f;
    }
}