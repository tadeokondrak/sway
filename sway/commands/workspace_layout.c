#include <string.h>
#include <strings.h>
#include "sway/commands.h"

struct cmd_results *cmd_workspace_layout(int argc, char **argv) {
	struct cmd_results *error = NULL;
	if ((error = checkarg(argc, "workspace_layout", EXPECTED_AT_LEAST, 1))) {
		return error;
	}
	if ((error = checkarg(argc, "workspace_layout", EXPECTED_AT_MOST, 2))) {
		return error;
	}
	if (argc == 2) {
		if (strcasecmp(argv[1], "reverse") == 0) {
			config->default_fill_order = LFO_REVERSE;
		} else {
			goto usage;
		}
	} else {
		config->default_fill_order = LFO_DEFAULT;
	}
	if (strcasecmp(argv[0], "default") == 0) {
		config->default_layout = L_NONE;
	} else if (strcasecmp(argv[0], "stacking") == 0) {
		config->default_layout = L_STACKED;
	} else if (strcasecmp(argv[0], "tabbed") == 0) {
		config->default_layout = L_TABBED;
	} else {
		goto usage;
	}
	return cmd_results_new(CMD_SUCCESS, NULL);
usage:
	return cmd_results_new(CMD_INVALID,
		"Expected 'workspace_layout <default|stacking|tabbed> [reverse]'");
}
