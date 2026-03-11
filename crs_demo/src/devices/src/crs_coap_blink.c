#include <zephyr/kernel.h>

struct blink_rsc_data {
	const struct gpio_dt_spec gpio;
	int state;
};

struct blink_rsc_ctx {
	struct blink_rsc_data *blink;
	int count;
};

static const struct json_obj_descr json_led_state_descr[] = {
	JSON_OBJ_DESCR_PRIM(struct json_led_state, led_id, JSON_TOK_NUMBER),
	JSON_OBJ_DESCR_PRIM(struct json_led_state, state, JSON_TOK_NUMBER),
};

static const struct json_obj_descr json_led_get_descr[] = {
	JSON_OBJ_DESCR_PRIM(struct json_led_get, device_id, JSON_TOK_STRING),
	JSON_OBJ_DESCR_OBJ_ARRAY(struct json_led_get, leds, JSON_MAX_LED, count,
				 json_led_state_descr, ARRAY_SIZE(json_led_state_descr)),
};

static int blink_handler_put(void *ctx, uint8_t *buf, int size)
{
	struct json_led_state led_data;
	struct blink_rsc_ctx *led_ctx = ctx;
	struct blink_rsc_data *led;
	int ret = -EINVAL;

	json_obj_parse(buf, size, json_led_state_descr, ARRAY_SIZE(json_led_state_descr),
		       &led_data);

	if (led_data.led_id >= led_ctx->count) {
		LOG_ERR("Invalid led id: %x", led_data.led_id);
		return -EINVAL;
	}
	led = &led_ctx->led[led_data.led_id];

	switch (led_data.state) {
	case LED_MSG_STATE_ON:
		ret = gpio_pin_set_dt(&led->gpio, 1);
		led->state = 1;
		break;
	case LED_MSG_STATE_OFF:
		ret = gpio_pin_set_dt(&led->gpio, 0);
		led->state = 0;
		break;
	case LED_MSG_STATE_TOGGLE:
		led->state = 1 - led->state;
		ret = gpio_pin_set_dt(&led->gpio, led->state);
		break;
	default:
		LOG_ERR("Set an unsupported LED state: %x", led_data.state);
	}

	return ret;
}

static int blink_handler_get(void *ctx, otMessage *msg, const otMessageInfo *msg_info)
{
	uint8_t buf[COAP_MAX_BUF_SIZE];
	struct led_rsc_ctx *led_ctx = ctx;

	struct json_led_get led_data = {
		.device_id = coap_device_id(),
	};

	for (int i = 0; i < led_ctx->count; i++) {
		led_data.leds[i].led_id = i;
		led_data.leds[i].state = led_ctx->led[i].state;
	}
	led_data.count = led_ctx->count;

	json_obj_encode_buf(json_led_get_descr, ARRAY_SIZE(json_led_get_descr), &led_data, buf,
			    COAP_MAX_BUF_SIZE);

	return coap_resp_send(msg, msg_info, buf, strlen(buf) + 1);
}

static void blink_handler(void *ctx, otMessage *msg, const otMessageInfo *msg_info)
{
	coap_req_handler(ctx, msg, msg_info, blink_handler_put, blink_handler_get);
}

static int blink_init(otCoapResource *rsc)
{
	struct blink_rsc_ctx *blink_ctx = rsc->mContext;
	int ret;

	LOG_INF("Initializing the LED");
	for (int i = 0; i < blink_ctx->count; i++) {
		struct blink_rsc_data *blink = &blink_ctx->led[i];

		if (!gpio_is_ready_dt(&blink->gpio)) {
			return -ENODEV;
		}

		ret = gpio_pin_configure_dt(&blink->gpio, GPIO_OUTPUT);
		if (ret) {
			LOG_ERR("Failed to configure the GPIO");
			return ret;
		}
	}

	return 0;
}

static struct blink_rsc_ctx blink_rsc_ctx = {
	.blink = blink_rsc_data,
	.count = ARRAY_SIZE(blink_rsc_data),
};

static otCoapResource blink_rsc = {
	.mUriPath = BLINK_URI,
	.mHandler = blink_handler,
	.mContext = &blink_rsc_ctx,
	.mNext = NULL,
};

void coap_blink_reg_rsc(void)
{
    otInstance *ot = openthread_get_default_instance();

	LOG_INF("Registering LED rsc");
	led_init(&blink_rsc);
	otCoapAddResource(ot, &blink_rsc);
}