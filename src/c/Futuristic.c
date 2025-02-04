#include <pebble.h>

static Window* s_window;
static TextLayer* s_text_layer;
static Layer* s_canvas_layer;

char* hrBuf;
char* minBuf;

#define HR_VERT_STEP 12
#define MIN_VERT_STEP 24

#define BUF_SIZE 4

static void prv_select_click_handler(ClickRecognizerRef recognizer, void* context)
{
	text_layer_set_text(s_text_layer, "Select");
}

static void prv_up_click_handler(ClickRecognizerRef recognizer, void* context)
{
	text_layer_set_text(s_text_layer, "Up");
}

static void prv_down_click_handler(ClickRecognizerRef recognizer, void* context)
{
	text_layer_set_text(s_text_layer, "Down");
}

static void prv_click_config_provider(void* context) {
	window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click_handler);
	window_single_click_subscribe(BUTTON_ID_UP, prv_up_click_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, prv_down_click_handler);
}

static void tick_handler(struct tm* tick_time, TimeUnits units_changed)
{
	layer_mark_dirty(s_canvas_layer);
}

static void update_proc(Layer* layer, GContext* ctx)
{
	graphics_context_set_text_color(ctx, GColorBlack);

	//HR/MIN titles
	GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
	graphics_draw_text(ctx, "HR", font, GRect(20,
		0, 30, 30), GTextOverflowModeFill, GTextAlignmentCenter, NULL);

	graphics_draw_text(ctx, "MIN", font, GRect(90,
		0, 30, 30), GTextOverflowModeFill, GTextAlignmentCenter, NULL);


	time_t temp = time(NULL);
	struct tm* tick_time = localtime(&temp);

	//Left side of face: Hours, max 12 bars
	//Right side: Minutes, max 6 bars


	//Hours
	GPoint origin = GPoint(20, 160);
	GPoint end = GPoint(50, 160);

	int hr = tick_time->tm_hour >= 13 ? tick_time->tm_hour - 12 : tick_time->tm_hour;

	char tmpBuf[BUF_SIZE];

	int drawnY = origin.y;

	for (int h = 0; h < 12; h++)
	{
		if(h <= hr)
		{
			graphics_draw_line(ctx, origin, end);
			drawnY -= HR_VERT_STEP;
		}

		//Place numbers alongside bar
		snprintf(tmpBuf, BUF_SIZE, "%i", h);
		graphics_draw_text(ctx, tmpBuf, font, GRect(2,
			origin.y - 10, 12, 12), GTextOverflowModeFill, GTextAlignmentCenter, NULL);

		origin.y -= HR_VERT_STEP;
		end.y -= HR_VERT_STEP;
	}

	//Place preceeding 0 if required
	char tmphr[4];
	snprintf(tmphr, 4, "%i", hr);
	if (hr < 10)
	{
		tmphr[1] = tmphr[0];
		tmphr[0] = '0';
	}

	snprintf(hrBuf, BUF_SIZE, "%s", tmphr);
	graphics_draw_text(ctx, hrBuf, font, GRect(origin.x,
		drawnY - HR_VERT_STEP, 30, 30), GTextOverflowModeFill, GTextAlignmentCenter, NULL);

	//Minutes
	origin = GPoint(90, 160);
	end = GPoint(120, 160);

	drawnY = origin.y;
	for (int m = 0; m < 6; m++)
	{
		if(m * 10 <= tick_time->tm_min)
		{
			graphics_draw_line(ctx, origin, end);
			drawnY -= MIN_VERT_STEP;
		}

		//Place numbers alongside bar
		snprintf(tmpBuf, BUF_SIZE, "%i", m);
		graphics_draw_text(ctx, tmpBuf, font, GRect(130,
			origin.y - 10, 12, 12), GTextOverflowModeFill, GTextAlignmentCenter, NULL);

		origin.y -= MIN_VERT_STEP;
		end.y -= MIN_VERT_STEP;
	}

	char tmpMins[4];
	snprintf(tmpMins, 4, "%i", tick_time->tm_min);
	if (tick_time->tm_min < 10)
	{
		tmpMins[1] = tmpMins[0];
		tmpMins[0] = '0';
	}

	snprintf(minBuf, BUF_SIZE, "%s", tmpMins);
	graphics_draw_text(ctx, minBuf, font, GRect(origin.x,
		drawnY, 30, 30), GTextOverflowModeFill, GTextAlignmentCenter, NULL);

	//Centre line
	origin = GPoint(70, 180);
	end = GPoint(70, 15);
	graphics_draw_line(ctx, origin, end);
}

static void prv_window_load(Window* window)
{
	Layer* window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	s_text_layer = text_layer_create(GRect(0, 0, bounds.size.w, 20));
	text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(s_text_layer));

	s_canvas_layer = layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));

	layer_set_update_proc(s_canvas_layer, update_proc);
	layer_add_child(window_layer, s_canvas_layer);
}

static void prv_window_unload(Window* window)
{
	text_layer_destroy(s_text_layer);
	layer_destroy(s_canvas_layer);

	if (hrBuf)
	{
		free(hrBuf);
	}

	if (minBuf) 
	{
		free(minBuf);
	}
}

static void prv_init(void) 
{
	hrBuf = malloc(sizeof(char) * BUF_SIZE);
	minBuf = malloc(sizeof(char) * BUF_SIZE);


	s_window = window_create();
	window_set_click_config_provider(s_window, prv_click_config_provider);
	window_set_window_handlers(s_window, (WindowHandlers) {
		.load = prv_window_load,
			.unload = prv_window_unload,
	});

	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

	const bool animated = true;
	window_stack_push(s_window, animated);
}

static void prv_deinit(void) 
{
	window_destroy(s_window);
}

int main(void) {
	prv_init();

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

	app_event_loop();
	prv_deinit();
}
