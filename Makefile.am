ACLOCAL_AMFLAGS = -I macros

SUBDIRS = doc rules src share scripts test

CLEANFILES = *~

man_MANS = tlf.1

README: README.md
	grep -v "Build Status" $< > $@
