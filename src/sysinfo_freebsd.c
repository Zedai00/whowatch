#include "sysinfo.h"
#include <sys/resource.h>
#include <sys/sysctl.h>
#include <sys/time.h>
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
