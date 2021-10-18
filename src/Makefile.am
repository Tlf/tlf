AM_CPPFLAGS = @GLIB_CFLAGS@ -DPACKAGE_DATA_DIR=\"$(pkgdatadir)\"
AM_CFLAGS = @AM_CFLAGS@ @PTHREAD_CFLAGS@ @HAMLIB_CFLAGS@ @CURSES_CFLAGS@

bin_PROGRAMS = tlf

tlf_SOURCES = \
	addcall.c addmult.c addpfx.c addspot.c audio.c autocq.c \
	background_process.c bandmap.c bands.c \
	cabrillo_utils.c calledit.c callinput.c changefreq.c changepars.c \
	change_rst.c checklogfile.c checkqtclogfile.c \
	cleanup.c clear_display.c clusterinfo.c \
        cqww_simulator.c cw_utils.c \
	dxcc.c deleteqso.c \
	edit_last.c editlog.c err_utils.c \
	fldigixmlrpc.c freq_display.c focm.c \
	genqtclist.c \
	get_time.c getctydata.c getexchange.c getmessages.c getpx.c \
	gettxinfo.c getwwv.c grabspot.c \
	hamlib_keyer.c \
	initial_exchange.c \
	keyer.c \
	lancode.c last10.c listmessages.c log_to_disk.c log_utils.c \
	logit.c logview.c locator2longlat.c \
	main.c makelogline.c messagechange.c muf.c \
	nicebox.c note.c netkeyer.c\
	paccdx.c parse_logcfg.c prevqso.c printcall.c \
	qrb.c qsonr_to_str.c qtc_log.c qtcwin.c qtcutil.c readcabrillo.c \
	readcalls.c readqtccalls.c readctydata.c recall_exchange.c rules.c \
	rtty.c \
	score.c scroll_log.c  searchcallarray.c searchlog.c sendbuf.c \
	sendqrg.c sendspcall.c set_tone.c setcontest.c \
	show_help.c showinfo.c showpxmap.c \
	showscore.c showzones.c sockserv.c speedupndown.c   \
	stoptx.c store_qso.c sunup.c splitscreen.c startmsg.c\
	trx_memory.c time_update.c ui_utils.c utils.c \
	write_keyer.c writecabrillo.c \
	zone_nr.c

tlf_LDADD = @LIBM_LIB@ @PTHREAD_LIBS@ @GLIB_LIBS@ @PANEL_LIBS@ @CURSES_LIBS@ \
	    @HAMLIB_LIBS@ @LIBXMLRPC_LIB@ @LIBXMLRPC_CLIENT_LIB@ \
	    @LIBXMLRPC_UTIL_LIB@

noinst_HEADERS = \
	addcall.h addmult.h addpfx.h addspot.h audio.h autocq.h \
	background_process.h bandmap.h bands.h \
	cabrillo_utils.h calledit.h callinput.h changefreq.h changepars.h \
	change_rst.h checklogfile.h checkqtclogfile.h \
	cleanup.h clear_display.h clusterinfo.h \
	cqww_simulator.h cw_utils.h \
	dxcc.h deleteqso.h \
	edit_last.h editlog.h  err_utils.h \
	fldigixmlrpc.h freq_display.h focm.h \
	genqtclist.h \
	get_time.h  getctydata.h getexchange.h getmessages.h getpx.h \
	gettxinfo.h getwwv.h globalvars.h grabspot.h \
	hamlib_keyer.h \
	ignore_unused.h initial_exchange.h \
	keyer.h keystroke_names.h \
	lancode.h last10.h listmessages.h log_utils.h \
	log_to_disk.h logit.h logview.h locator2longlat.h \
	makelogline.h messagechange.h muf.h \
	nicebox.h note.h netkeyer.h\
	paccdx.h parse_logcfg.h prevqso.h printcall.h \
	qrb.h qsonr_to_str.h qtc_log.h qtcvars.h qtcwin.h qtcutil.h \
	readcalls.h readqtccalls.h readctydata.h recall_exchange.h \
	rules.h readcabrillo.h rtty.h \
	score.h scroll_log.h searchcallarray.h searchlog.h sendbuf.h \
	sendqrg.h sendspcall.h set_tone.h setcontest.h \
	show_help.h showinfo.h showpxmap.h showscore.h \
	showzones.h sockserv.h speedupndown.h  \
	splitscreen.h startmsg.h stoptx.h store_qso.h sunup.h \
	time_update.h tlf.h tlf_curses.h tlf_panel.h trx_memory.h \
	ui_utils.h utils.h \
	write_keyer.h writecabrillo.h \
	zone_nr.h
