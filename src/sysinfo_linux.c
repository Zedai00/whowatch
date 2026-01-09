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
