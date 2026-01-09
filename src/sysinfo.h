#include <time.h>

time_t sys_boot_time(void);

struct cpu_info_t {
  unsigned long long u_mode, nice, s_mode, idle;
};

int sys_cpu_info(struct cpu_info_t *curr);

void sys_mem_info(void);
