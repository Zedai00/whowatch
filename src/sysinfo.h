#include <time.h>

time_t sys_boot_time(void);

struct cpu_info_t {
  unsigned long long u_mode, nice, s_mode, idle;
};

int sys_cpu_info(struct cpu_info_t *curr);

void sys_mem_info(void);

long long sys_open_files(void);

long long sys_open_inodes(void);

long long sys_max_files(void);

void sys_stat_info(void);

void sys_modules_info(void);

void sys_filesystems_info(void);

void sys_partitions_info(void);

void sys_devices_info(void);
