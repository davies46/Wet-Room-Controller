#ifndef _TIMER4
#define _TIMER4 1

void timer4initMs(long ms, void (*handler)(void));
void timer4delayMs(long ms);
void timer4delayUs(long us);
void stopTimer4();

extern void (*timer4handler)(void);

#endif

