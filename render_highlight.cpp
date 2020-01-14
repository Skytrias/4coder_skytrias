// NOTE(Skytrias): Anything that highlights text or something else
const f32 ANIMATION_CURSOR_GROW1 = 55.0f;
const f32 ANIMATION_CURSOR_GROW2 = 45.0f;
const f32 ANIMATION_GROW_FACTOR = 5.0f;
static f32 ANIMATION_MACRO = 100.0f;
static f32 global_cursor_counter = 0.0f;
static u32 global_cursor_times = 0;
static Vec2_f32 global_smooth_cursor_position = {0};

// additional nord colors
static u32 FUNCTION_HIGHLIGHT_COLOR = 0xFF25B2BC;
static u32 MACRO_HIGHLIGHT_COLOR = 0xFFB877DB;
static u32 HACK_HIGHLIGHT_COLOR = 0xd08770FF;
static u32 STRUCT_HIGHLIGHT_COLOR = 0xFF99db76;

// NOTE(Skytrias): custom growth animation added to ryan squishy cursor
static void
st_render_cursor(Application_Links *app, View_ID view_id, b32 is_active_view,
				 Buffer_ID buffer, Text_Layout_ID text_layout_id,
				 f32 roundness, f32 outline_thickness, Frame_Info frame_info)
{
    b32 has_highlight_range = draw_highlight_range(app, view_id, buffer, text_layout_id, roundness);
    
	// TODO(Skytrias): optimize
    switch (global_cursor_times) {
        case 0: {
            if (global_cursor_counter < ANIMATION_CURSOR_GROW1) {
                global_cursor_counter += 1.0f;
            } else {
                global_cursor_times = 1;
            }
        } break;
        
        case 1: {
            if (global_cursor_counter > 0.0f) {
                global_cursor_counter -= 1.0f;
            } else {
                global_cursor_times = 2;
            }
        } break;
        
        case 2: {
            if (global_cursor_counter < ANIMATION_CURSOR_GROW2) {
                global_cursor_counter += 1.0f;
            } else {
                global_cursor_times = 3;
            }
        } break;
        
        case 3: {
            if (global_cursor_counter > 0.0f) {
                global_cursor_counter -= 1.0f;
            } else {
                global_cursor_times = 0;
            }
        } break;
    }
    
    if (!has_highlight_range) {
        i64 cursor_pos = view_get_cursor_pos(app, view_id);
        i64 mark_pos = view_get_mark_pos(app, view_id);
        
        if (is_active_view)
        {
            // NOTE(rjf): Draw cursor.
            {
                static Rect_f32 rect = {0};
                Rect_f32 target_rect = text_layout_character_on_screen(app, text_layout_id, cursor_pos);
                Rect_f32 last_rect = rect;
                
                // NOTE(Skytrias): counter
                float growth = (global_cursor_counter / (ANIMATION_CURSOR_GROW1 + ANIMATION_CURSOR_GROW2)) * ANIMATION_GROW_FACTOR;
                float x_change = target_rect.x0 - rect.x0 - growth / 2.0f;
                float y_change = target_rect.y0 - rect.y0 - growth / 2.0f;
                float cursor_size_x = (target_rect.x1 - target_rect.x0);
                float cursor_size_y = (target_rect.y1 - target_rect.y0) * (1 + fabsf(y_change) / 60.f);animate_in_n_milliseconds(app, 0);
                
                rect.x0 += x_change * frame_info.animation_dt * 14.f;
                rect.y0 += y_change * frame_info.animation_dt * 14.f;
                rect.x1 = (rect.x0 + cursor_size_x) + growth;
                rect.y1 = (rect.y0 + cursor_size_y) + growth;
                
				// no growth on code_peek
                global_smooth_cursor_position.x = rect.x0 + growth / 2.0f;
                global_smooth_cursor_position.y = rect.y0 + growth / 2.0f;
                
                if(target_rect.y0 > last_rect.y0)
                {
                    if(rect.y0 < last_rect.y0)
                    {
                        rect.y0 = last_rect.y0;
                    }
                }
                else
                {
                    if(rect.y1 > last_rect.y1)
                    {
                        rect.y1 = last_rect.y1;
                    }
                }
                
                FColor cursor_color = fcolor_id(defcolor_cursor);
                
                if(global_keyboard_macro_is_recording)
                {
                    cursor_color = fcolor_argb(0xffde40df);
                }
                
                // NOTE(rjf): Draw main cursor.
                {
                    draw_rectangle(app, rect, roundness, fcolor_resolve(cursor_color));
                }
            }
            
            paint_text_color_pos(app, text_layout_id, cursor_pos,
                                 fcolor_id(defcolor_at_cursor));
            draw_character_wire_frame(app, text_layout_id, mark_pos,
                                      roundness, outline_thickness,
                                      fcolor_id(defcolor_mark));
        }
        else
        {
            draw_character_wire_frame(app, text_layout_id, mark_pos,
                                      roundness, outline_thickness,
                                      fcolor_id(defcolor_mark));
            draw_character_wire_frame(app, text_layout_id, cursor_pos,
                                      roundness, outline_thickness,
                                      fcolor_id(defcolor_cursor));
        }
    }
}

static f32
MinimumF32(f32 a, f32 b)
{
    return a < b ? a : b;
}

static f32
MaximumF32(f32 a, f32 b)
{
    return a > b ? a : b;
}

static void
Fleury4RenderBraceHighlight(Application_Links *app, Buffer_ID buffer, Text_Layout_ID text_layout_id,
                            i64 pos, ARGB_Color *colors, i32 color_count)
{
    Token_Array token_array = get_token_array_from_buffer(app, buffer);
    if (token_array.tokens != 0)
    {
        Token_Iterator_Array it = token_iterator_pos(0, &token_array, pos);
        Token *token = token_it_read(&it);
        if(token != 0 && token->kind == TokenBaseKind_ScopeOpen)
        {
            pos = token->pos + token->size;
        }
        else
        {
            
            if(token_it_dec_all(&it))
            {
                token = token_it_read(&it);
                
                if (token->kind == TokenBaseKind_ScopeClose &&
                    pos == token->pos + token->size)
                {
                    pos = token->pos;
                }
            }
        }
    }
    
    draw_enclosures(app, text_layout_id, buffer,
                    pos, FindNest_Scope,
                    RangeHighlightKind_CharacterHighlight,
                    0, 0, colors, color_count);
}

static void
Fleury4RenderCloseBraceAnnotation(Application_Links *app, Buffer_ID buffer, Text_Layout_ID text_layout_id,
                                  i64 pos)
{
    Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);
    Token_Array token_array = get_token_array_from_buffer(app, buffer);
    Face_ID face_id = get_face_id(app, buffer);
    
    if(token_array.tokens != 0)
    {
        Token_Iterator_Array it = token_iterator_pos(0, &token_array, pos);
        Token *token = token_it_read(&it);
        
        if(token != 0 && token->kind == TokenBaseKind_ScopeOpen)
        {
            pos = token->pos + token->size;
        }
		else if(token_it_dec_all(&it))
		{
			token = token_it_read(&it);
			if (token->kind == TokenBaseKind_ScopeClose &&
				pos == token->pos + token->size)
			{
				pos = token->pos;
			}
		}
	}
    
    Scratch_Block scratch(app);
    Range_i64_Array ranges = get_enclosure_ranges(app, scratch, buffer, pos, RangeHighlightKind_CharacterHighlight);
    
    for (i32 i = ranges.count - 1; i >= 0; i -= 1)
    {
        Range_i64 range = ranges.ranges[i];
        
        if(range.start >= visible_range.start)
        {
            continue;
        }
        
        Rect_f32 close_scope_rect = text_layout_character_on_screen(app, text_layout_id, range.end);
        Vec2_f32 close_scope_pos = { close_scope_rect.x0 + 12, close_scope_rect.y0 };
        
        // NOTE(rjf): Find token set before this scope begins.
		Token *start_token = 0;
		i64 token_count = 0;
		{
            Token_Iterator_Array it = token_iterator_pos(0, &token_array, range.start-1);
            int paren_nest = 0;
            
			for(;;)
            {
				Token *token = token_it_read(&it);
				if(!token_it_dec_non_whitespace(&it))
				{
					break;
                }
                
                if(token)
                {
                    token_count += 1;
                    
                    if(token->kind == TokenBaseKind_ParentheticalClose)
                    {
                        ++paren_nest;
                    }
                    else if(token->kind == TokenBaseKind_ParentheticalOpen)
                    {
                        --paren_nest;
                    }
                    else if(paren_nest == 0 &&
                            (token->kind == TokenBaseKind_ScopeClose ||
                             token->kind == TokenBaseKind_StatementClose))
                    {
                        break;
                    }
                    else if((token->kind == TokenBaseKind_Identifier || token->kind == TokenBaseKind_Keyword ||
                             token->kind == TokenBaseKind_Comment) &&
                            !paren_nest)
                    {
                        start_token = token;
                        break;
                    }
                    
                }
                else
                {
                    break;
                }
			}
            
		}
        
        // NOTE(rjf): Draw.
        if(start_token)
		{
            draw_string(app, face_id, string_u8_litexpr("← "), close_scope_pos, finalize_color(defcolor_comment, 0));
            close_scope_pos.x += 32;
            String_Const_u8 start_line = push_buffer_line(app, scratch, buffer,
                                                          get_line_number_from_pos(app, buffer, start_token->pos));
            
            u64 first_non_whitespace_offset = 0;
            for(u64 c = 0; c < start_line.size; ++c)
            {
                if(start_line.str[c] <= 32)
                {
                    ++first_non_whitespace_offset;
                }
                else
                {
                    break;
                }
            }
            start_line.str += first_non_whitespace_offset;
            start_line.size -= first_non_whitespace_offset;
            
            u32 color = finalize_color(defcolor_comment, 0);
            color &= 0x00ffffff;
            color |= 0x80000000;
            draw_string(app, face_id, start_line, close_scope_pos, color);
		}
    }
}

static void
Fleury4RenderBraceLines(Application_Links *app, Buffer_ID buffer, View_ID view,
                        Text_Layout_ID text_layout_id, i64 pos)
{
    Face_ID face_id = get_face_id(app, buffer);
    Token_Array token_array = get_token_array_from_buffer(app, buffer);
    Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);
    
    if (token_array.tokens != 0)
    {
        Token_Iterator_Array it = token_iterator_pos(0, &token_array, pos);
        Token *token = token_it_read(&it);
        if(token != 0 && token->kind == TokenBaseKind_ScopeOpen)
        {
            pos = token->pos + token->size;
        }
        else
        {
            
            if(token_it_dec_all(&it))
            {
                token = token_it_read(&it);
                
                if (token->kind == TokenBaseKind_ScopeClose &&
                    pos == token->pos + token->size)
                {
                    pos = token->pos;
                }
            }
        }
    }
    
    Face_Metrics metrics = get_face_metrics(app, face_id);
    
    Scratch_Block scratch(app);
    Range_i64_Array ranges = get_enclosure_ranges(app, scratch, buffer, pos, RangeHighlightKind_CharacterHighlight);
    float x_position = view_get_screen_rect(app, view).x0 + 4 -
        view_get_buffer_scroll(app, view).position.pixel_shift.x;
    
    for (i32 i = ranges.count - 1; i >= 0; i -= 1)
    {
        Range_i64 range = ranges.ranges[i];
        
        Rect_f32 range_start_rect = text_layout_character_on_screen(app, text_layout_id, range.start);
        Rect_f32 range_end_rect = text_layout_character_on_screen(app, text_layout_id, range.end);
        
        float y_start = 0;
        float y_end = 10000;
        
        if(range.start >= visible_range.start)
        {
            y_start = range_start_rect.y0 + metrics.line_height;
        }
        if(range.end <= visible_range.end)
        {
            y_end = range_end_rect.y0;
        }
        
        Rect_f32 line_rect = {0};
        line_rect.x0 = x_position;
        line_rect.x1 = x_position+1;
        line_rect.y0 = y_start;
        line_rect.y1 = y_end;
        u32 color = finalize_color(defcolor_comment, 0);
        color &= 0x00ffffff;
        color |= 0x60000000;
        draw_rectangle(app, line_rect, 0.5f, color);
        
        x_position += metrics.space_advance * 4;
        
    }
}

static void
Fleury4RenderRangeHighlight(Application_Links *app, View_ID view_id, Text_Layout_ID text_layout_id,
                            Range_i64 range)
{
    Rect_f32 range_start_rect = text_layout_character_on_screen(app, text_layout_id, range.start);
    Rect_f32 range_end_rect = text_layout_character_on_screen(app, text_layout_id, range.end-1);
    Rect_f32 total_range_rect = {0};
    total_range_rect.x0 = MinimumF32(range_start_rect.x0, range_end_rect.x0);
    total_range_rect.y0 = MinimumF32(range_start_rect.y0, range_end_rect.y0);
    total_range_rect.x1 = MaximumF32(range_start_rect.x1, range_end_rect.x1);
    total_range_rect.y1 = MaximumF32(range_start_rect.y1, range_end_rect.y1);
    
    ARGB_Color background_color = fcolor_resolve(fcolor_id(defcolor_pop2));
    float background_color_r = (float)((background_color & 0x00ff0000) >> 16) / 255.f;
    float background_color_g = (float)((background_color & 0x0000ff00) >>  8) / 255.f;
    float background_color_b = (float)((background_color & 0x000000ff) >>  0) / 255.f;
    background_color_r += (1.f - background_color_r) * 0.5f;
    background_color_g += (1.f - background_color_g) * 0.5f;
    background_color_b += (1.f - background_color_b) * 0.5f;
    ARGB_Color highlight_color = (0x75000000 |
                                  (((u32)(background_color_r * 255.f)) << 16) |
								  (((u32)(background_color_g * 255.f)) <<  8) |
								  (((u32)(background_color_b * 255.f)) <<  0));
    draw_rectangle(app, total_range_rect, 4.f, highlight_color);
}


// NOTE(Skytrias): custom token coloring
function FColor
st_get_token_color_cpp(Token token){
    Managed_ID color = defcolor_text_default;
    switch (token.kind){
        case TokenBaseKind_Preprocessor:
        {
            color = defcolor_preproc;
        }break;
        case TokenBaseKind_Keyword:
        {
            color = defcolor_keyword;
        }break;
        case TokenBaseKind_Comment:
        {
            color = defcolor_comment;
        }break;
        case TokenBaseKind_LiteralString:
        {
            color = defcolor_str_constant;
        }break;
        case TokenBaseKind_LiteralInteger:
        {
            color = defcolor_int_constant;
        }break;
        case TokenBaseKind_LiteralFloat:
        {
            color = defcolor_float_constant;
        }break;
        default:
        {
            switch (token.sub_kind){
                case TokenCppKind_And:
                case TokenCppKind_Or:
                case TokenCppKind_AndAnd:
                case TokenCppKind_OrOr:
				case TokenCppKind_ColonColon:
                case TokenCppKind_Colon:
                case TokenCppKind_Arrow:
				{
					color = defcolor_keyword;
				}break;
				
				case TokenCppKind_LiteralTrue:
                case TokenCppKind_LiteralFalse:
                {
                    color = defcolor_bool_constant;
                }break;
                
				case TokenCppKind_PPIncludeFile:
                {
                    color = defcolor_include;
                }break;
            }
        }break;
    }
    return(fcolor_id(color));
}

// NOTE(Skytrias): nothing customized, just here since token colors are customized
function void
st_draw_cpp_token_colors(Application_Links *app, Text_Layout_ID text_layout_id, Token_Array *array){
    Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);
    i64 first_index = token_index_from_pos(array, visible_range.first);
    Token_Iterator_Array it = token_iterator_index(0, array, first_index);
    for (;;){
        Token *token = token_it_read(&it);
        if (token->pos >= visible_range.one_past_last){
            break;
        }
        FColor color = st_get_token_color_cpp(*token);
        ARGB_Color argb = fcolor_resolve(color);
        paint_text_color(app, text_layout_id, Ii64_size(token->pos, token->size), argb);
        if (!token_it_inc_all(&it)){
            break;
        }
    }
}

// NOTE(Skytrias): paints all standard text leading to a '(' or '[' 
function void
st_paint_functions(Application_Links *app, Buffer_ID buffer, Text_Layout_ID text_layout_id) {
    i64 keyword_length = 0;
    i64 start_pos = 0;
    i64 end_pos = 0;
    
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
            
            // get pos at paren
            // NOTE(Skytrias): use token->sub_kind == TokenCppKind_ParenOp if only '(' should be used
			if (keyword_length != 0 && token->kind == TokenBaseKind_ParentheticalOpen) {
                end_pos = token->pos;
            }
            
            // search for default text, count up the size
            if (token->kind == TokenBaseKind_Identifier) {
                if (keyword_length == 0) {
                    start_pos = token->pos;
                }
                
                keyword_length += 1;
            } else {
                keyword_length = 0;
            }
            
            // color text 
            if (start_pos != 0 && end_pos != 0) {
                Range_i64 range = { 0 };
                range.start = start_pos;
                range.end = end_pos;
				
				// NOTE(Skytrias): use your own colorscheme her via fcolor_id(defcolor_*)
				// NOTE(Skytrias): or set the color you'd like to use globally like i do
                paint_text_color(app, text_layout_id, range, FUNCTION_HIGHLIGHT_COLOR);
                
                end_pos = 0;
                start_pos = 0;
            }
            
            if (!token_it_inc_all(&it)){
                break;
            }
        }
    }
}

// NOTE(Skytrias): paints all text leading up to a '!' in some color you like, nice for rust macros  
function void
st_paint_rust_macros(Application_Links *app, Buffer_ID buffer, Text_Layout_ID text_layout_id) {
    i64 keyword_length = 0;
    i64 start_pos = 0;
    i64 end_pos = 0;
    
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
            
            // get pos at paren
            if (keyword_length != 0 && token->sub_kind == TokenCppKind_Not) {
                end_pos = token->pos + 1;
            }
            
            // search for default text, count up the size
            if (token->kind == TokenBaseKind_Identifier) {
                if (keyword_length == 0) {
                    start_pos = token->pos;
                }
                
                keyword_length += 1;
            } else {
                keyword_length = 0;
            }
            
            // color text 
            if (start_pos != 0 && end_pos != 0) {
                Range_i64 range = { 0 };
                range.start = start_pos;
                range.end = end_pos;
				
				// NOTE(Skytrias): use your own colorscheme her via fcolor_id(defcolor_*)
				// NOTE(Skytrias): or set the color you'd like to use globally like i do
                paint_text_color(app, text_layout_id, range, MACRO_HIGHLIGHT_COLOR);
                
                end_pos = 0;
                start_pos = 0;
            }
            
            if (!token_it_inc_all(&it)){
                break;
            }
        }
    }
}

// NOTE(Skytrias): not used! "can" show you dotted '.' places 
function void
st_paint_rust_indent(Application_Links *app, Buffer_ID buffer, Text_Layout_ID text_layout_id) {
    i64 start_pos = 0;
    i64 end_pos = 0;
	i64 keyword_length = 0;
	b32 text_found = 0;
	
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
            
			// get pos at paren
            if (text_found && token->sub_kind == TokenCppKind_ParenOp) {
                end_pos = token->pos;
            }
			
            // search for default text, count up the size
            if (token->kind == TokenBaseKind_Identifier || 
				token->sub_kind == TokenCppKind_Dot
				) {
                if (keyword_length == 0) {
					if (token->sub_kind == TokenCppKind_Dot) {
						i64 pos = token->pos;
						i64 line_num = get_line_number_from_pos(app, buffer, pos);
						i64 line_start_pos = get_line_start_pos(app, buffer, line_num);
						
						// HACK(Skytrias): very bad, takes distance from pos to line start and sees if bytes are "close enough"
						i64 sub_byte_range = pos - line_start_pos;
						
						if (sub_byte_range < 8) {
							start_pos = token->pos;
							keyword_length += 1;
						}
					}
				} else {
					if (token->kind == TokenBaseKind_Identifier) {
						keyword_length += 1;
						text_found = 1;
					}
				}
            } else {
                keyword_length = 0;
				text_found = 0;
			}
            
            // color text 
            if (start_pos != 0 && end_pos != 0) {
                Range_i64 range = { 0 };
                range.start = start_pos;
                range.end = end_pos;
				
				// NOTE(Skytrias): use your own colorscheme her via fcolor_id(defcolor_*)
				// NOTE(Skytrias): or set the color you'd like to use globally like i do
                paint_text_color(app, text_layout_id, range, MACRO_HIGHLIGHT_COLOR);
                
                end_pos = 0;
                start_pos = 0;
				text_found = 0;
			}
            
            if (!token_it_inc_all(&it)){
                break;
            }
        }
    }
}

// NOTE(Skytrias): paints all text leading up to a '\'' in some color you like, nice for rust macros  
function void
st_paint_rust_characters(Application_Links *app, Buffer_ID buffer, Text_Layout_ID text_layout_id) {
	b32 character_found = 0;
    
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
            
			if (character_found) {
				if (token->kind == TokenBaseKind_Identifier) {
					Range_i64 range = { 0 };
					range.start = token->pos - 1;
					range.end = token->pos + 1;
					paint_text_color(app, text_layout_id, range, fcolor_resolve(fcolor_id(defcolor_keyword)));
					character_found = 0;
				} else {
					character_found = 0;
				}
			} else {
				if (token->sub_kind == TokenCppKind_LiteralCharacter ||
					token->sub_kind == TokenCppKind_LiteralCharacterWide ||
					token->sub_kind == TokenCppKind_LiteralCharacterUTF8 ||
					token->sub_kind == TokenCppKind_LiteralCharacterUTF16 ||
					token->sub_kind == TokenCppKind_LiteralCharacterUTF32) {
					character_found = 1;
				}
			}
			
            if (!token_it_inc_all(&it)){
                break;
            }
        }
    }
}

static void st_paint_tokens(Application_Links *app, Buffer_ID buffer, Text_Layout_ID text_layout_id)
{
    Scratch_Block scratch(app);
    FColor col = {0};
	
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
            // TODO(stefan): hack
            b32 valid = true;
            if(token->sub_kind == TokenCppKind_Dot){
                token_it_inc_all(&it);
                Token *peek = token_it_read(&it);
                String_Const_u8 token_as_string = push_token_lexeme(app, scratch, buffer, token);
                if(peek->kind == TokenBaseKind_Identifier &&
                   peek->sub_kind == TokenCppKind_Identifier){
                    //token_it_dec_all(&it);
                    valid = false;
                }
            }
			
            if (token->kind == TokenBaseKind_Identifier &&
                token->sub_kind == TokenCppKind_Identifier &&
                valid){
				
                String_Const_u8 token_as_string = push_token_lexeme(app, scratch, buffer, token);
				
                for(Buffer_ID buf = get_buffer_next(app, 0, Access_Always);
                    buf != 0;
                    buf = get_buffer_next(app, buf, Access_Always))
                {
					Code_Index_File *file = code_index_get_file(buf);
					if(file != 0){
						for(i32 i = 0; i < file->note_array.count; i += 1){
							Code_Index_Note *note = file->note_array.ptrs[i];
							//b32 found =false;
							switch(note->note_kind){
								case CodeIndexNote_Type:{
									if(string_match(note->text, token_as_string, StringMatch_Exact)){
										Range_i64 range = { 0 };
										range.start= token->pos;
										range.end= token->pos+token->size;
                                        //paint_text_color(app, text_layout_id, range, ARGBFromID(defcolor_keyword));
                                        paint_text_color(app, text_layout_id, range, STRUCT_HIGHLIGHT_COLOR);
										break;
									}
								}break;
							}
						}
					}
                }
            }
            if (!token_it_inc_all(&it)){
                break;
			}
		}
	}
}

static void
st_render_buffer(Application_Links *app, View_ID view_id, Face_ID face_id,
				 Buffer_ID buffer, Text_Layout_ID text_layout_id,
				 Rect_f32 rect, Frame_Info frame_info){
    ProfileScope(app, "[Skytrias] render buffer");
    
    View_ID active_view = get_active_view(app, Access_Always);
    b32 is_active_view = (active_view == view_id);
    Rect_f32 prev_clip = draw_set_clip(app, rect);
    
    // NOTE(allen): Token colorizing
    Token_Array token_array = get_token_array_from_buffer(app, buffer);
    if (token_array.tokens != 0){
        st_draw_cpp_token_colors(app, text_layout_id, &token_array);
        
        // NOTE(allen): Scan for TODOs and NOTEs
        if (global_config.use_comment_keyword){
            Comment_Highlight_Pair pairs[] = {
                {string_u8_litexpr("NOTE"), finalize_color(defcolor_comment_pop, 0)},
                {string_u8_litexpr("TODO"), finalize_color(defcolor_comment_pop, 1)},
                {string_u8_litexpr("HACK"), HACK_HIGHLIGHT_COLOR},
            };
            draw_comment_highlights(app, buffer, text_layout_id,
                                    &token_array, pairs, ArrayCount(pairs));
        }
    }
    else{
        Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);
        paint_text_color_fcolor(app, text_layout_id, visible_range, fcolor_id(defcolor_text_default));
    }
    
    i64 cursor_pos = view_correct_cursor(app, view_id);
    view_correct_mark(app, view_id);
    
    // NOTE(allen): Scope highlight
    if (global_config.use_scope_highlight){
        Color_Array colors = finalize_color_array(defcolor_back_cycle);
        draw_scope_highlight(app, buffer, text_layout_id, cursor_pos, colors.vals, colors.count);
    }
	
    // NOTE(rjf): Brace highlight
    {
        ARGB_Color colors[] =
        {
            0xff8ffff2,
        };
        
        Fleury4RenderBraceHighlight(app, buffer, text_layout_id, cursor_pos,
                                    colors, sizeof(colors)/sizeof(colors[0]));
    }
    
    if (global_config.use_error_highlight || global_config.use_jump_highlight){
        // NOTE(allen): Error highlight
        String_Const_u8 name = string_u8_litexpr("*compilation*");
        Buffer_ID compilation_buffer = get_buffer_by_name(app, name, Access_Always);
        if (global_config.use_error_highlight){
            draw_jump_highlights(app, buffer, text_layout_id, compilation_buffer,
                                 fcolor_id(defcolor_highlight_junk));
        }
        
        // NOTE(allen): Search highlight
        if (global_config.use_jump_highlight){
            Buffer_ID jump_buffer = get_locked_jump_buffer(app);
            if (jump_buffer != compilation_buffer){
                draw_jump_highlights(app, buffer, text_layout_id, jump_buffer,
                                     fcolor_id(defcolor_highlight_white));
            }
        }
    }
    
    // NOTE(allen): Color parens
    if (global_config.use_paren_helper){
        Color_Array colors = finalize_color_array(defcolor_text_cycle);
        draw_paren_highlight(app, buffer, text_layout_id, cursor_pos, colors.vals, colors.count);
    }
	
    // NOTE(Skytrias): word highlight before braces ()
    st_paint_functions(app, buffer, text_layout_id);
    st_paint_rust_macros(app, buffer, text_layout_id);
    st_paint_rust_characters(app, buffer, text_layout_id);
	//st_paint_rust_indent(app, buffer, text_layout_id);
    st_paint_tokens(app, buffer, text_layout_id);
	
	if (is_active_view) {
		st_auto_snippet(app, view_id, buffer, face_id, text_layout_id);
	}
	
    // NOTE(allen): Line highlight
    if (global_config.highlight_line_at_cursor && is_active_view){
        i64 line_number = get_line_number_from_pos(app, buffer, cursor_pos);
        draw_line_highlight(app, text_layout_id, line_number,
                            fcolor_id(defcolor_highlight_cursor_line));
    }
    
    // NOTE(allen): Cursor shape
    Face_Metrics metrics = get_face_metrics(app, face_id);
    f32 cursor_roundness = (metrics.normal_advance*0.5f)*0.9f;
    f32 mark_thickness = 2.f;
    
    // NOTE(allen): Cursor
    switch (fcoder_mode){
        case FCoderMode_Original:
        {
            st_render_cursor(app, view_id, is_active_view, buffer, text_layout_id, cursor_roundness, mark_thickness, frame_info);
        }break;
        case FCoderMode_NotepadLike:
        {
            draw_notepad_style_cursor_highlight(app, view_id, buffer, text_layout_id, cursor_roundness);
        }break;
    }
    
    // NOTE(allen): put the actual text on the actual screen
    draw_text_layout_default(app, text_layout_id);
    
    // NOTE(rjf): Brace annotations
    {
        Fleury4RenderCloseBraceAnnotation(app, buffer, text_layout_id, cursor_pos);
    }
    
    // NOTE(rjf): Brace lines
    {
        Fleury4RenderBraceLines(app, buffer, view_id, text_layout_id, cursor_pos);
    }
    
    // NOTE(rjf): Draw code peek
    if(global_code_peek_open && is_active_view)
    {
        Fleury4RenderRangeHighlight(app, view_id, text_layout_id, global_code_peek_token_range);
        st_render_code_peek(app, active_view, face_id, buffer, frame_info);
    }
    
    draw_set_clip(app, prev_clip);
}

// default file_bar draw call with macro recording highlighted in red
static void
st_draw_file_bar(Application_Links *app, View_ID view_id, Buffer_ID buffer, Face_ID face_id, Rect_f32 bar, f32 delta){
    Scratch_Block scratch(app);
    
    // NOTE(Skytrias): when recording, highlight file bar
    if (global_keyboard_macro_is_recording) {
        draw_rectangle_fcolor(app, bar, 0.f, fcolor_blend(fcolor_id(defcolor_bar), 0.5f, f_red, 0.5f));
    } else {
        draw_rectangle_fcolor(app, bar, 0.f, fcolor_id(defcolor_bar));
    }
    
    FColor base_color = fcolor_id(defcolor_base);
    FColor pop2_color = fcolor_id(defcolor_pop2);
    
    i64 cursor_position = view_get_cursor_pos(app, view_id);
    Buffer_Cursor cursor = view_compute_cursor(app, view_id, seek_pos(cursor_position));
    
    Fancy_Line list = {};
    String_Const_u8 unique_name = push_buffer_unique_name(app, scratch, buffer);
    push_fancy_string(scratch, &list, base_color, unique_name);
    push_fancy_stringf(scratch, &list, base_color, " - Row: %3.lld Col: %3.lld -", cursor.line, cursor.col);
    
    Managed_Scope scope = buffer_get_managed_scope(app, buffer);
    Line_Ending_Kind *eol_setting = scope_attachment(app, scope, buffer_eol_setting,
                                                     Line_Ending_Kind);
    switch (*eol_setting){
        case LineEndingKind_Binary:
        {
            push_fancy_string(scratch, &list, base_color, string_u8_litexpr(" bin"));
        }break;
        
        case LineEndingKind_LF:
        {
            push_fancy_string(scratch, &list, base_color, string_u8_litexpr(" lf"));
        }break;
        
        case LineEndingKind_CRLF:
        {
            push_fancy_string(scratch, &list, base_color, string_u8_litexpr(" crlf"));
        }break;
    }
    
    {
        Dirty_State dirty = buffer_get_dirty_state(app, buffer);
        u8 space[3];
        String_u8 str = Su8(space, 0, 3);
        if (dirty != 0){
            string_append(&str, string_u8_litexpr(" "));
        }
        if (HasFlag(dirty, DirtyState_UnsavedChanges)){
            string_append(&str, string_u8_litexpr("*"));
        }
        if (HasFlag(dirty, DirtyState_UnloadedChanges)){
            string_append(&str, string_u8_litexpr("!"));
        }
        push_fancy_string(scratch, &list, pop2_color, str.string);
    }
    
    // NOTE(Skytrias): push the string REC to the file bar
    if (global_keyboard_macro_is_recording) {
        push_fancy_string(scratch, &list, base_color, string_u8_litexpr(" REC"));
    }
    
    Vec2_f32 p = bar.p0 + V2f32(2.f, 2.f);
    draw_fancy_line(app, face_id, fcolor_zero(), &list, p);
}

// NOTE(Skytrias): pretty much just custom scroll speed and render_buffer
function void
st_render_caller(Application_Links *app, Frame_Info frame_info, View_ID view_id){
    ProfileScope(app, "skytrias render caller");
    View_ID active_view = get_active_view(app, Access_Always);
    b32 is_active_view = (active_view == view_id);
    
    Rect_f32 region = draw_background_and_margin(app, view_id, is_active_view);
    Rect_f32 prev_clip = draw_set_clip(app, region);
    
    Buffer_ID buffer = view_get_buffer(app, view_id, Access_Always);
    Face_ID face_id = get_face_id(app, buffer);
    Face_Metrics face_metrics = get_face_metrics(app, face_id);
    f32 line_height = face_metrics.line_height;
    f32 digit_advance = face_metrics.decimal_digit_advance;
    
    // NOTE(allen): file bar
    b64 showing_file_bar = false;
    if (view_get_setting(app, view_id, ViewSetting_ShowFileBar, &showing_file_bar) && showing_file_bar){
        Rect_f32_Pair pair = layout_file_bar_on_top(region, line_height);
        st_draw_file_bar(app, view_id, buffer, face_id, pair.min, frame_info.animation_dt);
        region = pair.max;
    }
    
    Buffer_Scroll scroll = view_get_buffer_scroll(app, view_id);
    
    // NOTE(Skytrias): faster scrolling like alpha
    Buffer_Point_Delta_Result delta = delta_apply(app, view_id,
                                                  frame_info.animation_dt * 1.125f, scroll);
    if (!block_match_struct(&scroll.position, &delta.point)){
        block_copy_struct(&scroll.position, &delta.point);
        view_set_buffer_scroll(app, view_id, scroll, SetBufferScroll_NoCursorChange);
    }
    if (delta.still_animating){
        animate_in_n_milliseconds(app, 0);
    }
    
    // NOTE(allen): query bars
    {
        Query_Bar *space[32];
        Query_Bar_Ptr_Array query_bars = {};
        query_bars.ptrs = space;
        if (get_active_query_bars(app, view_id, ArrayCount(space), &query_bars)){
            for (i32 i = 0; i < query_bars.count; i += 1){
                Rect_f32_Pair pair = layout_query_bar_on_top(region, line_height, 1);
                draw_query_bar(app, query_bars.ptrs[i], face_id, pair.min);
                region = pair.max;
            }
        }
    }
    
    // NOTE(allen): FPS hud
    if (show_fps_hud){
        Rect_f32_Pair pair = layout_fps_hud_on_bottom(region, line_height);
        draw_fps_hud(app, frame_info, face_id, pair.max);
        region = pair.min;
        animate_in_n_milliseconds(app, 1000);
    }
    
    // NOTE(allen): layout line numbers
    Rect_f32 line_number_rect = {};
    if (global_config.show_line_number_margins){
        Rect_f32_Pair pair = layout_line_number_margin(app, buffer, region, digit_advance);
        line_number_rect = pair.min;
        region = pair.max;
    }
    
    // NOTE(allen): begin buffer render
    Buffer_Point buffer_point = scroll.position;
    Text_Layout_ID text_layout_id = text_layout_create(app, buffer, region, buffer_point);
    
    // NOTE(allen): draw line numbers
    if (global_config.show_line_number_margins){
        draw_line_number_margin(app, view_id, buffer, face_id, text_layout_id, line_number_rect);
    }
    
    // NOTE(allen): draw the buffer
    st_render_buffer(app, view_id, face_id, buffer, text_layout_id, region, frame_info);
    
    text_layout_free(app, text_layout_id);
    draw_set_clip(app, prev_clip);
}