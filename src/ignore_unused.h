#ifndef IGNORE_UNUSED_H
#define IGNORE_UNUSED_H

#define IGNORE(x) do { \
	_Pragma("GCC diagnostic push");                            \
	_Pragma("GCC diagnostic ignored \"-Wunknown-pragmas\"");    \
	_Pragma("GCC diagnostic ignored \"-Wunused-result\"");       \
	x;                                                            \
	_Pragma("GCC diagnostic pop");                                 \
} while(0)

#endif
