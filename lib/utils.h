#ifndef UTILS_H
#define UTILS_H

#define DIE_IF(__cond, __msg, ...) if (__cond) { fprintf(stderr, "[X] " __msg "\n", ## __VA_ARGS__); exit(EXIT_FAILURE); }
#define ERR(__msg, ...) do { fprintf(stderr, "[X] " __msg "\n", ## __VA_ARGS__); } while (0)

#define VALIDATE_PTR_OR_RETURN(__ptr, __retval) if (0 == (__ptr)) { ERR(# __ptr "is NULL!"); return (__retval); }

#endif
