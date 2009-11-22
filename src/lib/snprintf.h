extern int pg_vsnprintf(char *str, size_t count, const char *fmt, va_list args);
extern int pg_snprintf(char *str, size_t count, const char *fmt, ...);

#define vsnprintf pg_vsnprintf
#define snprintf pg_snprintf

