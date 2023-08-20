#include "test.h"
#include <stdlib.h>
#include <sys/wait.h>

GString *errors;

int setup_default(void **state) {

    errors = g_string_new("");

    return 0;
}

int teardown_default(void **state) {

    g_string_free(errors, TRUE);

    return 0;
}

static void append_error(const gchar *path, const gchar *result) {
    if (errors->len > 0) {
	g_string_append(errors, ", ");
    }
    g_string_append_printf(errors, "%s(%s)", path, result);
}

static void check_rules(const gchar *dirname, gchar *path) {
    printf("++ %s\n", dirname);

    gchar *cmd = g_strdup_printf("cd %s; ../run_tlf.py", path);
    printf(">>> %s\n", cmd);
    int rc = WEXITSTATUS(system(cmd));
    g_free(cmd);

    if (rc == 1) {
	append_error(dirname, "FAIL");
    } else if (rc == 2) {
	append_error(dirname, "TIMEOUT");
    }
}

#define TEST_RULES_DIR  TOP_SRCDIR "/test/rules"

void test_rules(void **state) {

    GError *error;
    const gchar *dirname;

    GDir *dir = g_dir_open(TEST_RULES_DIR, 0, &error);

    while ((dirname = g_dir_read_name(dir))) {
	gchar *path = g_build_filename(TEST_RULES_DIR, dirname, (gchar *)NULL);
	if (g_file_test(path, G_FILE_TEST_IS_DIR)) {
	    check_rules(dirname, path);
	}
	g_free(path);
    }

    g_dir_close(dir);

    assert_string_equal(errors->str, "");
}

