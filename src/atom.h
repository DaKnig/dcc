#ifndef ATOM_H_
#define ATOM_H_

#include <stddef.h>

const char *atom_fromstr(const char *str);
const char *atom_fromstrwlen(const char *str, size_t len);

#endif
