#include "sysinfo.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

time_t sys_boot_time(void) {
  char buf[32];
  static time_t boot_time;
  FILE *f;

  if (boot_time)
    return boot_time;
  if (!(f = fopen("/proc/stat", "r")))
    return boot_time;
  while (fgets(buf, sizeof buf, f))
    if (!strncmp(buf, "btime ", 6))
      goto FOUND;
  fclose(f);
  return boot_time;
FOUND:
  fclose(f);
  sscanf(buf + 5, "%ld", &boot_time);
  return boot_time;
}

int sys_cpu_info(struct cpu_info_t *curr) {
  FILE *f;
  char buf[256];

  f = fopen("/proc/stat", "r");
  if (!f) {
    perror("fopen /proc/stat");
    return -1;
  }

  while (fgets(buf, sizeof buf, f)) {
    if (!strncmp(buf, "cpu ", 4)) {
      int n = sscanf(buf + 5, "%llu %llu %llu %llu", &curr->u_mode, &curr->nice,
                     &curr->s_mode, &curr->idle);
      fclose(f);
      return (n == 4) ? 0 : -1;
    }
  }

  fclose(f);
  return -1;
}
