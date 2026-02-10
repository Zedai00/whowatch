/*

SPDX-License-Identifier: GPL-2.0+

Copyright 2001, 2006 Michal Suszycki <mt_suszycki@yahoo.com>
Copyright 2014, 2018 Paul Wise <pabs3@bonedaddy.net>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

/*
 * Builtin proc plugin and sys plugin.
 * Get process info (ppid, tpgid, name of executable and so on).
 * This is OS dependent: in Linux reading files from "/proc" is
 * needed, in FreeBSD and OpenBSD sysctl() is used (which
 * gives better performance)
 */
#include "config.h"
#include "pluglib.h"
#include "sysinfo.h"
#include "whowatch.h"
#include <err.h>
#include <stdio.h>

#ifdef __FreeBSD__
#include <sys/resource.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#if !defined(HZ) && defined(HAVE_SYSCONF) && defined(_SC_CLK_TCK)
#define HZ sysconf(_SC_CLK_TCK)
#endif

#if !defined(HZ) && defined(HAVE_ASM_PARAM_H)
#include <asm/param.h>
#endif

#if !defined(HZ) && (defined(__FreeBSD_kernel__) || defined(__FreeBSD__))
#define HZ 100
#endif

#ifndef HZ
#error Could not determine ticks per second
#endif

#if defined(HAVE_LIBKVM) && defined(HAVE_STDINT_H) && defined(HAVE_KVM_H)
#include <kvm.h>
// kvm_t *kd;
extern int can_use_kvm;
#endif

#define EXEC_FILE 128
#define elemof(x) (sizeof(x) / sizeof *(x))
#define endof(x) ((x) + elemof(x))

static inline void no_info(void) { println("Information unavailable.\n"); }

/*
 * Returns time the process
 * started in seconds after system boot.
 */

static void proc_starttime(int pid) {
  long i, sec;
  char *s;
  time_t btime = sys_boot_time();

  i = sys_start_time(pid);
  if (i == -1 || btime == -1) {
    no_info();
    return;
  }
  sec = btime + i;
  s = ctime(&sec);
  print("%s", s);
}

struct proc_detail_t {
  char *title; /* title of a particular information	*/
  void (*fn)(int pid);
  int t_lines; /* nr of line for a title		*/
  // char *name;  /* used only to read links		*/
};

struct proc_detail_t proc_details_t[] = {
    {"START: ", proc_starttime, 0},
    {"EXE: ", sys_proc_exe, 0},
    // {"ROOT: ", read_link, 0, "root"},
    {"ROOT: ", sys_proc_root, 0},
    // {"CWD: ", read_link, 0, "cwd"},
    {"CWD: ", sys_proc_cwd, 0},
    // {"\nSTATUS:\n", read_meminfo, 2, 0},
    {"\nSTATUS:\n", sys_proc_status, 2},
    // {"\nFILE DESCRIPTORS:\n", open_fds, 2, 0}};
    {"\nFILE DESCRIPTORS:\n", sys_proc_fds, 2}};

void eproc(void *p) {
  int i;
  int pid = !p ? 1 : *(u32 *)p;
  struct proc_detail_t *t;
  int size;
  plgn_out_start();
  //	print("PID %d\n", pid);newln();
  size = sizeof proc_details_t / sizeof(struct proc_detail_t);
  for (i = 0; i < size; i++) {
    t = &proc_details_t[i];
    title("%s", t->title);
    t->fn(pid);
  }
}

static inline void print_boot_time(void) {
  time_t btime = sys_boot_time();
  if (btime)
    print("%s", ctime(&btime));
  else
    no_info();
}

static inline float prcnt(unsigned long i, unsigned long v) {
  if (!v)
    return 0;
  return ((float)(i * 100)) / (float)v;
}

static struct cpu_info_t cur, prev, eff;

/*
 * Based on values taken by sys_cpu_info() calculate
 * and print CPU load (user, system, nice and idle).
 */
static void get_cpu_info() {
  char buf[64];
  unsigned long z;
  prev = cur;
  if (sys_cpu_info(&cur) == -1)
    no_info();

  eff.u_mode = cur.u_mode - prev.u_mode;
  eff.nice = cur.nice - prev.nice;
  eff.s_mode = cur.s_mode - prev.s_mode;
  eff.idle = cur.idle - prev.idle;

  z = eff.u_mode + eff.nice + eff.s_mode + eff.idle;
  snprintf(buf, sizeof buf, "%.1f%% user %.1f%% sys %.1f%% nice %.1f%% idle\n",
           prcnt(eff.u_mode, z), prcnt(eff.s_mode, z), prcnt(eff.nice, z),
           prcnt(eff.idle, z));
  print("%s", buf);
}

void print_or_no(long long val) {
  if (val == -1) {
    no_info();
  } else {
    println("%lld", val);
  }
}

void esys(void *unused) {
  long long c;
  plgn_out_start();

  title("BOOT TIME: ");
  print_boot_time();

  title("CPU: ");
  get_cpu_info();

  title("MEMORY:");
  newln();
  sys_mem_info();

  title("USED FILES: ");
  c = sys_open_files();
  print_or_no(c);

  title("\nUSED INODES: ");
  c = sys_open_inodes();
  print_or_no(c);

  title("\nMAX FILES: ");
  c = sys_max_files();
  print_or_no(c);

  title("\nSTAT:");
  newln();
  sys_stat_info();

  title("\nLOADED MODULES:");
  newln();
  sys_modules_info();

  title("\nFILESYSTEMS:");
  newln();
  sys_filesystems_info();

  title("\nPARTITIONS:");
  newln();
  sys_partitions_info();

  title("\nDEVICES:\n");
  sys_devices_info();
}
