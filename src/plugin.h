#ifndef PLUGIN_H
#define PLUGIN_H

#include "tlf.h"

int plugin_init(const char *name);
void plugin_close();

bool plugin_has_setup();
void plugin_setup();

bool plugin_has_score();
int plugin_score(struct qso_t *qso);

bool plugin_has_check_exchange();
void plugin_check_exchange(struct qso_t *qso);

#endif
