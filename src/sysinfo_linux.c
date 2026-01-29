#include "pluglib.h"
#include "sysinfo.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

time_t sys_boot_time(void) {
  char buf[128];
  static time_t boot_time;
  static int init_boot_time;
  FILE *f;

  if (init_boot_time)
    return boot_time;

  f = fopen("/proc/stat", "r");
  if (!f)
    return 0;

  while (fgets(buf, sizeof buf, f)) {
    if (strncmp(buf, "btime ", 6) == 0) {
      if (sscanf(buf + 6, "%ld", &boot_time) == 1)
        init_boot_time = 1;
      break;
    }
  }

  fclose(f);
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

void sys_stat_info() { read_proc_file("/proc/stat", "cpu", "intr"); }

void sys_modules_info() { read_proc_file("/proc/modules", NULL, NULL); }

void sys_filesystems_info() { read_proc_file("/proc/filesystems", NULL, NULL); }

void sys_partitions_info() { read_proc_file("/proc/partitions", NULL, NULL); }

void sys_devices_info() { read_proc_file("/proc/devices", NULL, NULL); }

#define START_TIME_FIELD 22

long sys_start_time(int pid) {

  char path[64];
  char buf[4096];
  FILE *f;
  char *p;
  long long start_ticks;
  long hz;

  snprintf(path, sizeof(path), "/proc/%d/stat", pid);
  f = fopen(path, "r");
  if (!f) {
    return -1;
  }

  if (!fgets(buf, sizeof(buf), f)) {
    fclose(f);
    return -1;
  }
  fclose(f);

  /*
   * Format:
   * pid (comm) state ppid ... starttime
   * comm can contain spaces → must skip () properly
   */

  p = strchr(buf, ')');
  if (!p)
    return -1;
  p++; // past (comm)

  /* starttime is field 22 → count fields after comm */
  int field = 3; /* state is field 3 */
  while (*p && field < START_TIME_FIELD) {
    if (*p == ' ') {
      field++;
    }
    p++;
  }

  if (field != START_TIME_FIELD) {
    return -1;
  }

  if (sscanf(p, "%lld", &start_ticks) != 1) {
    return -1;
  }

  hz = sysconf(_SC_CLK_TCK);
  if (hz < 0)
    return -1;

  return (time_t)(start_ticks / hz);
}

static inline char *_read_link(const char *path) {
  static char buf[128];
  bzero(buf, sizeof buf);
  if (readlink(path, buf, sizeof buf) == -1)
    return 0;
  return buf;
}

static void read_link(int pid, char *name) {
  char *v;
  char pbuf[32];
  snprintf(pbuf, sizeof pbuf, "/proc/%d/%s", pid, name);
  v = _read_link(pbuf);
  if (!v) {
    no_info();
    return;
  }
  println(v);
  newln();
}

void sys_proc_exe(int pid) { read_link(pid, "exe"); }

void sys_proc_root(int pid) { read_link(pid, "root"); }

void sys_proc_cwd(int pid) { no_info(); }

void sys_proc_status(int pid) { no_info(); }

void sys_proc_fds(int pid) { no_info(); }
