#ifndef __PROFILE_TIMER_H__
#define __PROFILE_TIMER_H__

#include <asm/io.h>
#include <linux/types.h>

extern void *prof_timers_base;
extern bool prof_timers_enabled;

#define PROF_TIMERS_BASE   0x50000000
#define PROF_TIMERS_LENGTH 0x1000

#define PROF_CTRL       0x0000
#define PROF_TAG_IN     0x0018
#define PROF_CTRL_START 1

void prof_timers_control(bool enable);

#define PROF_TAG(x) if (prof_timers_enabled) {iowrite32(x, prof_timers_base + PROF_TAG_IN); iowrite32(PROF_CTRL_START, prof_timers_base + PROF_CTRL);}

#endif /* __PROFILE_TIMER_H__ */
