#ifndef PLUGIN_H
#define PLUGIN_H

#include "tlf.h"

int plugin_init(const char *name);

bool plugin_has_setup();
void plugin_setup();

bool plugin_has_score();
int plugin_score(struct qso_t *qso);

bool plugin_has_is_dupe();
bool plugin_is_dupe();

bool plugin_has_is_multi();
bool plugin_is_multi(int band, const char *call, int mode);

bool plugin_has_nr_of_mults();
int plugin_nr_of_mults(int band);

bool plugin_has_get_multi();
char *plugin_get_multi(struct qso_t *qso);

#endif
