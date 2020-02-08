// timer 
struct Timer {
	f32 ticks; // addition of dt till range for 1 second add is reached
	i32 seconds; 
	i32 minutes;
	i32 hours;
};

global Timer global_timer = {};
global b32 global_timer_on = 0;
// the higher the longer the timer takes to increase, 2 works on 120 hz, 1 on 60 probably 
global f32 global_timer_speed = 0.5f;
// adjust these times to what you want
global i32 global_pomodoro_minutes = 25;
global i32 global_timer_goal = global_pomodoro_minutes;
global i32 global_short_break_minutes = 5;
global i32 global_long_break_minutes = 10;

// returns true if a "TIME" token exists in the line
function b32 st_line_timer_token_exists(Application_Links *app, Buffer_ID buffer, Token_Array *array, i64 line_start_number, i64 line_end_number) {
	if (array->tokens != 0){
		Token_Iterator_Array it = token_iterator_pos(buffer, array, line_end_number);
		
		for (;;) {
			Token *token = token_it_read(&it);
			
			if (token->pos < line_start_number) {
				return 0;
			}
			
			if (token->sub_kind == TokenCppKind_Time) {
				return 1;
			}
			
			if (!token_it_dec_non_whitespace(&it)) {
				return 0;
			}
		}
	}
	
	return 0;
}

CUSTOM_COMMAND_SIG(st_pause_timer)
CUSTOM_DOC("Pause timer") {
	if (!global_timer_on) {
		PlaySound(global_timer_start_sound);
	} else {
		PlaySound(global_timer_pause_sound);
	}
	
	global_timer_on = !global_timer_on;
}

// returns true if the timer hasnt counted up yet
function void st_timer_reset() {
	global_timer.hours = 0; 
	global_timer.minutes = 0;
	global_timer.seconds = 0;   
}

CUSTOM_COMMAND_SIG(st_stop_timer)
CUSTOM_DOC("Starts the timer") {
	PlaySound(global_timer_restart_sound);
	global_timer_on = 0;
	st_timer_reset();
}

CUSTOM_COMMAND_SIG(st_pomodoro_start)
CUSTOM_DOC("Start pomodoro timer, default to 25 minutes") {
	PlaySound(global_timer_start_sound);
	global_timer_on = 1;
	st_timer_reset();
	global_timer_goal = global_pomodoro_minutes;
}

CUSTOM_COMMAND_SIG(st_short_break_start)
CUSTOM_DOC("Start short break timer, default to 5 minutes") {
	PlaySound(global_timer_start_sound);
	global_timer_on = 1;
	st_timer_reset();
	global_timer_goal = global_short_break_minutes;
}

CUSTOM_COMMAND_SIG(st_long_break_start)
CUSTOM_DOC("Start long break timer, default to 10 minutes") {
	PlaySound(global_timer_start_sound);
	global_timer_on = 1;
	st_timer_reset();
	global_timer_goal = global_long_break_minutes;
}

function b32 st_timer_zero() {
	return (global_timer.hours == 0 && global_timer.minutes == 0 && global_timer.seconds == 0);
}

CUSTOM_COMMAND_SIG(st_paste_timer)
CUSTOM_DOC("Places the timer stats onto the lind end of the cursor before the comma") {
	if (st_timer_zero()) {
		return;
	}
	
	View_ID view = get_active_view(app, Access_ReadWriteVisible);
	Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
	
	i64 pos = view_get_cursor_pos(app, view);
	i64 line_start_number = get_line_start_pos_from_pos(app, buffer, pos); 
	i64 line_end_number = get_line_end_pos_from_pos(app, buffer, pos); 
	
	Token_Array token_array = get_token_array_from_buffer(app, buffer);
	Scratch_Block scratch(app);
	
	b32 comma_token = st_end_token_match(app, buffer, &token_array, line_end_number, TokenCppKind_Comma);
	
	if (comma_token) {
		b32 time_token = st_line_timer_token_exists(app, buffer, &token_array, line_start_number, line_end_number);
		
		// NOTE(Skytrias): IF YOU CHANGE THE FORMAT OF THE TIME PASTED, YOU ALSO HAVE TO CHANGE THE READING VERSION OF IT
		// if no time token was found, add new time entirely
		if (!time_token) {
			String_Const_u8 result = result = push_u8_stringf(
															  scratch, 
															  "\tTime: %02d:%02d:%02d,", 
															  global_timer.hours, 
															  global_timer.minutes, 
															  global_timer.seconds
															  );
			buffer_replace_range(app, buffer, Ii64(line_end_number - 1, line_end_number), result);
		} else {
			// get the existing values back from the string line, add them to the current timer
			Range_i64 seconds_range = Ii64(line_end_number - 3, line_end_number - 1);
			String_Const_u8 seconds_string = push_buffer_range(app, scratch, buffer, seconds_range);
			i32 old_seconds = (i32) string_to_integer(seconds_string, 10);
			
			Range_i64 minutes_range = Ii64(line_end_number - 6, line_end_number - 4);
			String_Const_u8 minutes_string = push_buffer_range(app, scratch, buffer, minutes_range);
			i32 old_minutes = (i32) string_to_integer(minutes_string, 10);
			
			Range_i64 hours_range = Ii64(line_end_number - 9, line_end_number - 7);
			String_Const_u8 hours_string = push_buffer_range(app, scratch, buffer, hours_range);
			i32 old_hours = (i32) string_to_integer(hours_string, 10);
			
			i32 new_seconds = old_seconds;
			i32 new_minutes = old_minutes;
			i32 new_hours = old_hours;
			
			// add to the recent time if not on break
			new_seconds = global_timer.seconds + old_seconds;
				new_minutes = global_timer.minutes + old_minutes;
				new_hours = global_timer.hours + old_hours;
				if (new_seconds > 60) {
				new_seconds -= 60;
				new_minutes += 1;
				} 
				if (new_minutes > 60) {
				new_minutes -= 60;
				new_hours += 1;
				} 
			
			String_Const_u8 result = result = push_u8_stringf(
															  scratch, 
															  "\tTime: %02d:%02d:%02d,", 
															  new_hours,
															  new_minutes,
															  new_seconds
															  );
			
			PlaySound(global_timer_paste_sound);
			// if it does exist, redo the numbers
			buffer_replace_range(app, buffer, Ii64(line_end_number - result.size, line_end_number), result);
		}
		
		// reset timer and stop it, simple subtract if the timer wasnt finished yet
		if (global_timer.seconds == 0 &&
				(global_timer.minutes == global_pomodoro_minutes ||
			 global_timer.minutes == global_short_break_minutes ||
				 global_timer.minutes == global_long_break_minutes)) {
			global_timer_on = 0;
		} else {
			global_timer_goal -= global_timer.minutes;
		}
		
		st_timer_reset();
	}
}

CUSTOM_COMMAND_SIG(st_slow_timer)
CUSTOM_DOC("Decreases the speed the timer ticks at") {
	global_timer_speed *= 1.5; 
}

CUSTOM_COMMAND_SIG(st_fast_timer)
CUSTOM_DOC("Increases the speed the timer ticks at") {
	global_timer_speed *= 0.5;
}

// updates the timer to the newest dt
function void st_update_timer(Application_Links *app, View_ID view_id, Frame_Info frame_info) {
	if (!global_timer_on) {
		return;
	}
	
	Timer *timer = &global_timer;
	
	if (timer->minutes >= global_timer_goal) {
		global_timer_on = 0;
		PlaySound(global_timer_end_sound);
		return;
	}
	
	animate_in_n_milliseconds(app, 500);
	timer->ticks += frame_info.literal_dt * global_timer_speed;
	
	if (timer->ticks >= 1.0f) {
		if (timer->seconds < 60 - 1) {
			timer->seconds += 1;
			timer->ticks = 0.0f;
		} else {
				timer->seconds = 0;
			
			if (timer->minutes <= 60 - 1) {
				timer->minutes += 1;
				timer->ticks = 0.0f;
				} else {
				timer->minutes = 0; 
				
				if (timer->hours <= 60 - 1) {
					timer->hours += 1;
					timer->ticks = 0.0f;
				}
				}
		}
	}
}
