/* setjmp.h - setjmp, longjmp, jmp_buf */

/* Type definition for jmp_buf */
typedef int32 jmp_buf[6];

/* Function prototypes */

extern	int32	setjmp(jmp_buf env);
extern	void	longjmp(jmp_buf env, int32 val);
