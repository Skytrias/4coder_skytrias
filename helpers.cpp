// returns true if the given line_number contains two "//" at the line start
// TODO(Skytrias): look through tokens, find comment in line, kinda bad, look string match instead?
function b32 st_has_line_comment(Application_Links *app, Buffer_ID buffer, i64 line_number) {
	Token_Array array = get_token_array_from_buffer(app, buffer);
	if (array.tokens != 0){
		i64 line_start = get_line_start_pos(app, buffer, line_number);
		Token_Iterator_Array it = token_iterator_pos(buffer, &array, line_start);
		
		b32 comment_found = false;
		
		Token *token = token_it_read(&it);
		if (token->sub_kind == TokenCppKind_LineComment) {
			token = token_it_read(&it);
			if (token->sub_kind == TokenCppKind_LineComment) {
				comment_found = true;
			}
		} else {
			token_it_inc_non_whitespace(&it);
			token = token_it_read(&it);
			
			if (token->sub_kind == TokenCppKind_LineComment) {
				token = token_it_read(&it);
				
				if (token->sub_kind == TokenCppKind_LineComment) {
					comment_found = true;
				}
			}
		}
		
		if (comment_found) {
			return true;
		}
	}
	
	return false;
}

// returns true if the given line_number contains two "//" in a line where the pos is at
function b32 st_has_line_comment_pos(Application_Links *app, Buffer_ID buffer, i64 pos) {
	i64 current_line_number = get_line_number_from_pos(app, buffer, pos);
	return st_has_line_comment(app, buffer, current_line_number);
}
