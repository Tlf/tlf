CLEANFILES = *~

EXTRA_DIST = callmaster cty.dat logcfg.dat arrlsections ea_sections paccmults \
			spdxmults ssamults

install-data-local:
	install -m 0644 callmaster $(pkgdatadir)
	install -m 0644 cty.dat $(pkgdatadir)
	install -m 0644 logcfg.dat $(pkgdatadir)
	install -m 0644 arrlsections $(pkgdatadir)
	install -m 0644 ea_sections $(pkgdatadir)
	install -m 0644 paccmults $(pkgdatadir)
	install -m 0644 spdxmults $(pkgdatadir)
	install -m 0644 ssamults $(pkgdatadir)

uninstall:
	rm -rf $(pkgdatadir)
