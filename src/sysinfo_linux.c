#include "pluglib.h"
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

static inline void no_info(void) { println("Information unavailable.\n"); }

static void read_proc_file(char *name, char *start, char *end) {
  char buf[128];
  int ok = 0;
  int slen, elen;
  FILE *f;
  slen = elen = 0;
  if (start)
    slen = strlen(start);
  if (end)
    elen = strlen(end);
  f = fopen(name, "r");
  if (!f) {
    no_info();
    return;
  }
  if (!start)
    ok = 1;
  while (fgets(buf, sizeof buf, f)) {
    if (!ok && !strncmp(buf, start, slen))
      ok = 1;
    if (end && !strncmp(buf, end, elen))
      goto END;
    if (!ok)
      continue;
    println(buf);
    //		newln();
  }
END:
  if (!ok)
    no_info();
  fclose(f);
}

long long read_file_pos(char *name, int pos) {
  FILE *f;
  long long val;

  f = fopen(name, "r");
  if (!f)
    return -1;

  for (int i = 0; i <= pos; i++) {
    if (fscanf(f, "%lld", &val) != 1) {
      fclose(f);
      return -1;
    }
  }

  fclose(f);
  return val;
}

void sys_mem_info(void) { read_proc_file("/proc/meminfo", "MemTotal:", NULL); }

long long sys_open_files() {
  long long allocated = read_file_pos("/proc/sys/fs/file-nr", 0);
  if (allocated == -1)
    return -1;

  long long unused = read_file_pos("/proc/sys/fs/file-nr", 1);
  if (unused == -1)
    return -1;

  return allocated - unused;
}

long long sys_open_inodes() {
  long long alloc = read_file_pos("/proc/sys/fs/inode-nr", 0);
  if (alloc == -1)
    return -1;

  long long unused = read_file_pos("/proc/sys/fs/inode-nr", 1);
  if (unused == -1)
    return -1;

  return alloc - unused;
}

long long sys_max_files() {
  long long maxfiles = read_file_pos("/proc/sys/fs/file-max", 0);
  if (maxfiles == -1)
    return -1;
  return maxfiles;
}

