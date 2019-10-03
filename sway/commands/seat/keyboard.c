#define _POSIX_C_SOURCE 200809L
#include <strings.h>
#include <time.h>
#include <xkbcommon/xkbcommon.h>
#include "sway/commands.h"
#include "sway/input/seat.h"
#include "sway/input/input-manager.h"

static const char expected_syntax[] =
	"Expected 'keyboard press|release [--keycode] <key>'";

/**
 * The last found keycode associated with the keysym
 * and the total count of matches.
 */
struct keycode_matches {
	xkb_keysym_t keysym;
	xkb_keycode_t keycode;
	int count;
};

/**
 * Iterate through keycodes in the keymap to find ones matching
 * the specified keysym.
 */
static void find_keycode(struct xkb_keymap *keymap,
		xkb_keycode_t keycode, void *data) {
	xkb_keysym_t keysym = xkb_state_key_get_one_sym(
			config->keysym_translation_state, keycode);

	if (keysym == XKB_KEY_NoSymbol) {
		return;
	}

	struct keycode_matches *matches = data;
	if (matches->keysym == keysym) {
		matches->keycode = keycode;
		matches->count++;
	}
}

/**
 * Return the keycode for the specified keysym.
 */
static struct keycode_matches get_keycode_for_keysym(xkb_keysym_t keysym) {
	struct keycode_matches matches = {
		.keysym = keysym,
		.keycode = XKB_KEYCODE_INVALID,
		.count = 0,
	};

	xkb_keymap_key_for_each(
			xkb_state_get_keymap(config->keysym_translation_state),
			find_keycode, &matches);
	return matches;
}

static struct cmd_results *press_or_release(struct sway_seat *seat,
		bool is_keycode, const char *action, const char *key) {
	int32_t state;
	if (strcasecmp(action, "press") == 0) {
		state = WLR_KEY_PRESSED;
	} else if (strcasecmp(action, "release") == 0) {
		state = WLR_KEY_RELEASED;
	} else {
		return cmd_results_new(CMD_INVALID, expected_syntax);
	}

	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	uint32_t time = now.tv_nsec / 1000;

	xkb_keycode_t keycode;
	if (is_keycode) {
		keycode = strtol(key, NULL, 10);
	} else {
		xkb_keysym_t keysym = xkb_keysym_from_name(key,
				XKB_KEYSYM_CASE_INSENSITIVE);

		if (keysym == XKB_KEY_NoSymbol) {
			return cmd_results_new(
					CMD_FAILURE, "Unable to convert key '%s' into keysym", key);
		}

		struct keycode_matches matches = get_keycode_for_keysym(keysym);
		if (matches.count != 1) {
			return cmd_results_new(CMD_FAILURE, "Unable to convert keysym %d "
					"into a single keycode (%d matches)", keysym, matches.count);
		}

		keycode = get_keycode_for_keysym(keysym).keycode - 8;
	}

	wlr_seat_set_keyboard(seat->wlr_seat, NULL);
	wlr_seat_keyboard_notify_key(seat->wlr_seat, time, keycode, state);

	return cmd_results_new(CMD_SUCCESS, NULL);
}

static struct cmd_results *handle_command(struct sway_seat *seat, int argc,
		char **argv) {
	if (strcasecmp(argv[0], "press") == 0
			|| strcasecmp(argv[0], "release") == 0) {
		if (argc == 2) {
			return press_or_release(seat, false, argv[0], argv[1]);
		} else if (argc == 3 && strcasecmp(argv[1], "--keycode") == 0) {
			return press_or_release(seat, true, argv[0], argv[2]);
		}
	}

	return cmd_results_new(CMD_INVALID, expected_syntax);
}

struct cmd_results *seat_cmd_keyboard(int argc, char **argv) {
	struct cmd_results *error = NULL;
	if ((error = checkarg(argc, "keyboard", EXPECTED_AT_LEAST, 2))) {
		return error;
	}

	struct seat_config *sc = config->handler_context.seat_config;
	if (!sc) {
		return cmd_results_new(CMD_FAILURE, "No seat defined");
	}

	if (config->reading || !config->active) {
		return cmd_results_new(CMD_DEFER, NULL);
	}

	if (strcmp(sc->name, "*") != 0) {
		struct sway_seat *seat = input_manager_get_seat(sc->name, false);
		if (!seat) {
			return cmd_results_new(
					CMD_FAILURE, "Seat %s does not exist", sc->name);
		}
		return handle_command(seat, argc, argv);
	} else {
		struct sway_seat *seat;
		wl_list_for_each(seat, &server.input->seats, link) {
			error = handle_command(seat, argc, argv);
			if ((error && error->status != CMD_SUCCESS)) {
				return error;
			}
		}
	}

	return cmd_results_new(CMD_SUCCESS, NULL);
}
