#include <ctype.h>

#include "tlf.h"
#include "cluster.h"
#include "clusterinfo.h"
#include "globalvars.h"
#include "clear_display.h"
#include "tlf_panel.h"
#include "keystroke_names.h"
#include "nicebox.h"
#include "ui_utils.h"

/*
Screen structure:

    - 1 border line
    | ROWS_DX spots
    - 1 separator line
    | ROWS_NON_DX other non-spot cluster lines
    - 1 input line
    - 1 border line
    - 1 status line

LINES = 1(bord) + ROWS_DX + 1(sep) + ROWS_NON_DX + 1(inp) + 1(bord) + 1(stat)

*/

#define WIN_WIDTH   80
#define WIN_HEIGHT  (LINES - 1)         // keep bottom status row visible

#define LINE_LENGTH (WIN_WIDTH - 2)     // allow 2-char border

#define INPUT_LINE  (WIN_HEIGHT - 2)

#define ROWS_DX     8
#define ROWS_NON_DX (LINES - 1 - ROWS_DX - 1 - 1 - 1 -1)

#define DX_TOP_LINE     1
#define NON_DX_TOP_LINE (1 + ROWS_DX + 1)

#define WHITE_ON_BLACK  COLOR_PAIR(C_LOG)
#define BLACK_ON_WHITE  (COLOR_PAIR(C_LOG) | A_STANDOUT)
#define WHITE_ON_BLUE   (COLOR_PAIR(C_INPUT) | A_STANDOUT)

#define KEY_POLL_MS     10
#define REDRAW_MS       300

static WINDOW *win = NULL;
static PANEL *panel = NULL;

static char **dx_lines;
static char **non_dx_lines;

static int n_dx_lines;
static int n_non_dx_lines;

static void setup() {

    n_dx_lines = ROWS_DX;
    n_non_dx_lines = ROWS_NON_DX;

    // allocate lines of spaces for terminal lines
    dx_lines = g_malloc(n_dx_lines * sizeof(char *));
    for (int i = 0; i < n_dx_lines; i++) {
	dx_lines[i] = g_strdup(spaces(LINE_LENGTH));
    }

    non_dx_lines = g_malloc(n_non_dx_lines * sizeof(char *));
    for (int i = 0; i < n_non_dx_lines; i++) {
	non_dx_lines[i] = g_strdup(spaces(LINE_LENGTH));
    }

    win = newwin(WIN_HEIGHT, WIN_WIDTH, 0, 0);
    panel = new_panel(win);

    show_panel(panel);

    wattrset(win, WHITE_ON_BLACK);
    wclear(win);

    wnicebox(win, 0, 0, WIN_HEIGHT - 2, LINE_LENGTH,
	     "Cluster Terminal   (press Esc to exit)");

    wattrset(win, BLACK_ON_WHITE);
    mvwhline(win, 1 + ROWS_DX, 1, ACS_HLINE, LINE_LENGTH);  // separator
}

static void teardown() {

    hide_panel(panel);
    del_panel(panel);
    delwin(win);

    win = NULL;

    for (int i = 0; i < n_dx_lines; i++) {
	g_free(dx_lines[i]);
    }

    for (int i = 0; i < n_non_dx_lines; i++) {
	g_free(non_dx_lines[i]);
    }

    g_free(dx_lines);
    g_free(non_dx_lines);
}

void cluster_terminal_resize() {
    if (win == NULL) {
	return;
    }

    teardown();
    setup();
}

//
// move populated lines to the start of the array
// and fill remaining slots with blank lines.
// (array was filled in reverse order)
//
static void align_up(char **lines, int size, int total_size) {
    const int shift = total_size - size;
    if (shift > 0) {
	for (int i = shift; i < total_size; i++) {
	    strcpy(lines[i - shift], lines[i]);
	}
	for (int i = total_size - shift; i < total_size; i++) {
	    strcpy(lines[i], spaces(LINE_LENGTH));
	}
    }
}

//
// store spot line in terminal line array
//
static void store_line(char *array[], int *index, const char *src) {
    if (*index >= 0) {
	sprintf(array[*index], "%-*.*s", LINE_LENGTH, LINE_LENGTH, src);
	(*index)--;
    }
}

//
// retrieve the most recent lines from the cluster.
// populate local arrays in reverse order (starting from the end).
// left-align text and truncate to line_length characters during copying.
//
static void update_lines() {
    pthread_mutex_lock(&spot_ptr_mutex);

    int dxi = n_dx_lines - 1;
    int ndxi = n_non_dx_lines - 1;
    for (int i = nr_of_spots - 1; i >= 0; i--) {
	//if (g_str_has_prefix(spot_ptr[i], "DX de ")) {
	if (is_dx_spot(spot_ptr[i])) {
	    store_line(dx_lines, &dxi, spot_ptr[i]);
	} else {
	    store_line(non_dx_lines, &ndxi, spot_ptr[i]);
	}

	if (dxi < 0 && ndxi < 0) {
	    break;      // both arrays are full, exit the loop
	}
    }

    pthread_mutex_unlock(&spot_ptr_mutex);

    align_up(dx_lines, n_dx_lines - (dxi + 1), n_dx_lines);
    align_up(non_dx_lines, n_non_dx_lines - (ndxi + 1), n_non_dx_lines);
}

void cluster_terminal() {

    setup();

    int input_length = LINE_LENGTH - 2;     // 2 chars are used by the prompt

    char *input = g_malloc(input_length + 1);

    input[0] = 0;
    int pos = 0;

    int iterations = 0;
    int c = 0;              // force immediate redraw

    while (1) {
	iterations++;
	if (iterations > REDRAW_MS / KEY_POLL_MS || c >= 0) {

	    update_lines();

	    wattrset(win, WHITE_ON_BLACK);
	    for (int i = 0; i < n_dx_lines; i++) {
		mvwaddstr(win, DX_TOP_LINE + i, 1, dx_lines[i]);
	    }
	    for (int i = 0; i < n_non_dx_lines; i++) {
		mvwaddstr(win, NON_DX_TOP_LINE + i, 1, non_dx_lines[i]);
	    }

	    wattrset(win, WHITE_ON_BLUE);
	    mvwprintw(win, INPUT_LINE, 1, "> %-*.*s", input_length, input_length, input);
	    wmove(win, INPUT_LINE, pos + 3);

	    refreshp();
	    iterations = 0;
	}

	usleep(KEY_POLL_MS * 1000);
	c = key_poll();

	if (c == '\n' || c == KEY_ENTER) {
	    g_strstrip(input);
	    if (input[0] == 0) {
		strcpy(input, " ");     // send a space for an empty line
	    }
	    if (input[0] != ':') {
		send_to_cluster(input);
		input[0] = 0;           // clear input
		pos = 0;
	    } else {
		c = ESCAPE;             // exit if line starts with a ':'
	    }
	}

	if (c == ESCAPE) {
	    break;
	}

	switch (c) {
	    case DELETE:
	    case KEY_BACKSPACE:
		if (pos > 0) {
		    pos--;
		    input[pos] = 0;
		}
		break;
	    default:
		if (isprint(c) && pos < input_length) {
		    input[pos++] = c;
		    input[pos] = '\0';
		}
	}
    }

    g_free(input);

    teardown();

    clear_display();
}
