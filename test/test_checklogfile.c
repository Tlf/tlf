#include "test.h"
#include "stdio.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../src/tlf.h"

#include "../src/checklogfile.h"

// OBJECT ../src/checklogfile.o

// dummy
void shownr(char *msg, int x) {
}

extern char logfile[];

/* setup / teardown */
void append_tofile (char *file, char * msg) {
    int fd;

    fd = open(file, O_WRONLY | O_APPEND );
    (void)write (fd, msg, strlen(msg));
    close(fd);
}

#define LOGLINE " 20CW  18-Jan-14 16:04 0111  N0CALL         599  599  33            W        3  14025.0\n"
#define COMMENT "; Node A, 110 : comment                                                                \n"

void append_line (int fd, char *msg) {
    (void)write (fd, msg, strlen(msg));
}

void write_log(char * file) {
    int fd;

    fd = creat(file, 0644);
    for (int i = 0; i < 10; i++) {
	if (i == 8)
	    append_line(fd, COMMENT);
	else
	    append_line(fd, LOGLINE);
    }
    close(fd);
}

int setup_default(void **state) {

    strcpy (logfile, "test.log");
    write_log(logfile);

    return 0;
}

int teardown_default(void **state) {
    remove(logfile);

    return 0;
}

/* verify that all log lines have correct length */
int check_log(char *file) {
    FILE *fp;
    char buffer[LOGLINELEN+5];
    int result = 0;

    fp = fopen(file, "r");

    while (fgets(buffer, LOGLINELEN + 5, fp)) {
	if (strlen(buffer) != LOGLINELEN) {
	    result = 1;
	    break;
	}
    }

    fclose(fp);
    return result;
}

/* verify check_log helper can detect errors */
void test_filelength_ok(void **state) {
    assert_int_equal(check_log(logfile), 0);
}

void test_file_to_Long(void **state) {
    append_tofile(logfile, " ");
    assert_int_equal(check_log(logfile), 1);
}

/* checklogfile() shall check and fix logfile so that it conforms to the
 * following:
/* 1. each logfile should have lines with only LOGLINELEN as length */
/* 2. each line should end with an \n (future request) */
void test_file_length_ok(void **state) {
    checklogfile();
    assert_int_equal(check_log(logfile), 0);
}

void test_file_length_short(void **state) {
    append_tofile(logfile, " ");
    checklogfile();
    assert_int_equal(check_log(logfile), 0);
}

void test_file_length_long(void **state) {
    for (int i = 0; i < LOGLINELEN + 5; i++) {
	append_tofile(logfile, " ");
    }
    checklogfile();
    assert_int_equal(check_log(logfile), 0);
}

