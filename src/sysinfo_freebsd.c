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
