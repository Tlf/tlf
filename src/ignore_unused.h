#ifndef IGNORE_UNUSED_H
#define IGNORE_UNUSED_H

#define IGNORE(x) do { \
	if (x);         \
		(void)0; \
} while(0)

#endif
