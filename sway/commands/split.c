#include <string.h>
#include <strings.h>
#include "sway/commands.h"
#include "sway/tree/arrange.h"
#include "sway/tree/container.h"
#include "sway/tree/view.h"
#include "sway/tree/workspace.h"
#include "sway/input/input-manager.h"
#include "sway/input/seat.h"
#include "log.h"

static struct cmd_results *do_split(int layout, int fill_order) {
	struct sway_container *con = config->handler_context.container;
	struct sway_workspace *ws = config->handler_context.workspace;
	if (con) {
		if (container_is_scratchpad_hidden_or_child(con) &&
				con->pending.fullscreen_mode != FULLSCREEN_GLOBAL) {
			return cmd_results_new(CMD_FAILURE,
					"Cannot split a hidden scratchpad container");
		}
		if (fill_order == -1)
			fill_order = con->pending.fill_order;
		container_split(con, layout, fill_order);
	} else {
		if (fill_order == -1)
			fill_order = ws->fill_order;
		workspace_split(ws, layout, fill_order);
	}

	if (root->fullscreen_global) {
		arrange_root();
	} else {
		arrange_workspace(ws);
	}

	return cmd_results_new(CMD_SUCCESS, NULL);
}

struct cmd_results *cmd_split(int argc, char **argv) {
	struct cmd_results *error = NULL;
	if ((error = checkarg(argc, "split", EXPECTED_EQUAL_TO, 1))) {
		return error;
	}
	if (!root->outputs->length) {
		return cmd_results_new(CMD_INVALID,
				"Can't run this command while there's no outputs connected.");
	}
	if (strcasecmp(argv[0], "v") == 0 || strcasecmp(argv[0], "vertical") == 0) {
		return do_split(L_VERT, -1);
	} else if (strcasecmp(argv[0], "h") == 0 ||
			strcasecmp(argv[0], "horizontal") == 0) {
		return do_split(L_HORIZ, -1);
	} else if (strcasecmp(argv[0], "t") == 0 ||
			strcasecmp(argv[0], "toggle") == 0) {
		struct sway_container *focused = config->handler_context.container;

		if (focused && container_parent_layout(focused) == L_VERT) {
			return do_split(L_HORIZ, -1);
		} else {
			return do_split(L_VERT, -1);
		}
	} else if (strcasecmp(argv[0], "l") == 0||
			strcasecmp(argv[0], "left") == 0) {
		do_split(L_HORIZ, LFO_REVERSE);
	} else if (strcasecmp(argv[0], "r") == 0 ||
			strcasecmp(argv[0], "right") == 0) {
		do_split(L_HORIZ, LFO_DEFAULT);
	} else if (strcasecmp(argv[0], "u") == 0 ||
			strcasecmp(argv[0], "up") == 0) {
		do_split(L_VERT, LFO_REVERSE);
	} else if (strcasecmp(argv[0], "d") == 0 ||
			strcasecmp(argv[0], "down") == 0) {
		do_split(L_VERT, LFO_DEFAULT);
	} else {
		return cmd_results_new(CMD_FAILURE,
			"Invalid split command (expected one of "
			"vertical|v|horizontal|h|toggle|t"
			"|left|l|right|r|up|u|down|d).");
	}
	return cmd_results_new(CMD_SUCCESS, NULL);
}

struct cmd_results *cmd_splitv(int argc, char **argv) {
	struct cmd_results *error = NULL;
	if ((error = checkarg(argc, "splitv", EXPECTED_EQUAL_TO, 0))) {
		return error;
	}
	return do_split(L_VERT, -1);
}

struct cmd_results *cmd_splith(int argc, char **argv) {
	struct cmd_results *error = NULL;
	if ((error = checkarg(argc, "splith", EXPECTED_EQUAL_TO, 0))) {
		return error;
	}
	return do_split(L_HORIZ, -1);
}

struct cmd_results *cmd_splitt(int argc, char **argv) {
	struct cmd_results *error = NULL;
	if ((error = checkarg(argc, "splitt", EXPECTED_EQUAL_TO, 0))) {
		return error;
	}

	struct sway_container *con = config->handler_context.container;

	if (con && container_parent_layout(con) == L_VERT) {
		return do_split(L_HORIZ, -1);
	} else {
		return do_split(L_VERT, -1);
	}
}
