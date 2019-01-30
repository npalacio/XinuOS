/* string.h */

extern	char	*strncpy(char *, const char *, int32);
extern	char	*strncat(char *, const char *, int32);
extern	int32	strncmp(const char *, const char *, int32);
extern	char	*strchr(const char *, int32);
extern	char	*strrchr(const char *, int32);
extern	char	*strstr(const char *, const char *);
extern	int32	strnlen(const char *, uint32);
extern	int	strlen(const char *);

extern char *strcpy(char *, const char *);
extern char *strcat(char *, const char *);
extern int strcmp(char *, char *);
extern char *strpbrk(const char *, const char *);
extern int strspn(const char *, const char *);
extern char *strtok(char *s1, const char *s2);

extern long strtol(const char *, char **, int);
extern unsigned long strtoul(const char *, char **, int);
extern unsigned long _Stoul(const char *, char **, int);
