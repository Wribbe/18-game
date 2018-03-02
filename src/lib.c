#include "lib.h"

/* Information output functions.
 * ----------------------------- */

void
error(const char * fmt, ...) {
  va_list ap;
  fprintf(stderr, "[!]: ");
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}

void
info(const char * fmt, ...) {
  va_list ap;
  fprintf(stdout, "[-]: ");
  va_start(ap, fmt);
  vfprintf(stdout, fmt, ap);
  va_end(ap);
}
