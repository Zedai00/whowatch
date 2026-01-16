#include "pluglib.h"
#include "sysinfo.h"
#include <stdint.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <sys/resource.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>

time_t sys_boot_time() {
  static time_t boot_time;
  struct timeval tv;
  size_t len = sizeof(tv);
  int mib[2];
  mib[0] = CTL_KERN;
  mib[1] = KERN_BOOTTIME;
  sysctl(mib, 2, &tv, &len, NULL, 0);
  boot_time = tv.tv_sec;
  return boot_time;
}

int sys_cpu_info(struct cpu_info_t *cur_cpu_info) {
  long cp_time[CPUSTATES];
  size_t len = sizeof(cp_time);

  if (sysctlbyname("kern.cp_time", cp_time, &len, NULL, 0) == -1)
    return -1;

  cur_cpu_info->u_mode = cp_time[CP_USER];
  cur_cpu_info->nice = cp_time[CP_NICE];
  cur_cpu_info->s_mode = cp_time[CP_SYS];
  cur_cpu_info->idle = cp_time[CP_IDLE];

  return 0;
}

void sys_mem_info(void) {
  uint64_t pagesize, physmem;
  u_int v_free, v_active, v_inactive, v_cache, swap_total, swap_used;
  size_t len;

  len = sizeof(pagesize);
  sysctlbyname("hw.pagesize", &pagesize, &len, NULL, 0);

  len = sizeof(physmem);
  sysctlbyname("hw.physmem", &physmem, &len, NULL, 0);

  len = sizeof(pagesize);
  sysctlbyname("hw.pagesize", &pagesize, &len, NULL, 0);

  len = sizeof(u_int);
  sysctlbyname("vm.stats.vm.v_free_count", &v_free, &len, NULL, 0);
  sysctlbyname("vm.stats.vm.v_active_count", &v_active, &len, NULL, 0);
  sysctlbyname("vm.stats.vm.v_inactive_count", &v_inactive, &len, NULL, 0);
  sysctlbyname("vm.stats.vm.v_cache_count", &v_cache, &len, NULL, 0);
  sysctlbyname("vm.swap_total", &swap_total, &len, NULL, 0);
  sysctlbyname("vm.swap_reserved", &swap_used, &len, NULL, 0);

  println("MemTotal: %llu kB\n", physmem / 1024);
  println("MemFree: %llu kB\n", (uint64_t)v_free * pagesize / 1024);
  println("MemAvailable: %llu kB\n",
          (uint64_t)(v_free + v_inactive + v_cache) * pagesize / 1024);
  println("Active: %llu kB\n", (uint64_t)v_active * pagesize / 1024);
  println("Inactive: %llu kB\n", (uint64_t)v_inactive * pagesize / 1024);
  println("Cached: %llu kB\n", (uint64_t)v_cache * pagesize / 1024);
  println("SwapTotal: %llu kB\n", (uint64_t)swap_total / 1024);
  println("SwapUsed: %llu kB\n", (uint64_t)swap_used / 1024);
  println("SwapFree: %llu kB\n", (uint64_t)(swap_total - swap_used) / 1024);
}

long long sys_open_files() {
  u_int openfiles;
  size_t len;
  len = sizeof(openfiles);
  if (sysctlbyname("kern.openfiles", &openfiles, &len, NULL, 0) == -1) {
    return -1;
  } else {
    return openfiles;
  }
}

long long sys_open_inodes() {
  struct statfs s;
  if (statfs("/", &s) == -1) {
    return -1;
  }
  return s.f_files - s.f_ffree;
}
