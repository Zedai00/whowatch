#include "pluglib.h"
#include "sysinfo.h"
#include <devinfo.h>
#include <kvm.h>
#include <libgeom.h>
#include <machine/param.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/linker.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <sys/queue.h>
#include <sys/resource.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/user.h>
#include <time.h>

static inline void no_info(void) { println("Information unavailable.\n"); }

time_t sys_boot_time() {
  static time_t boot_time;
  static int init_boot_time;

  if (init_boot_time)
    return boot_time;

  struct timeval tv;
  size_t len = sizeof(tv);
  int mib[2];
  mib[0] = CTL_KERN;
  mib[1] = KERN_BOOTTIME;
  if (sysctl(mib, 2, &tv, &len, NULL, 0) == -1) {
    boot_time = -1;
    return boot_time;
  }
  boot_time = tv.tv_sec;
  init_boot_time = 1;
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
  if (sysctlbyname("kern.openfiles", &openfiles, &len, NULL, 0) == -1)
    return -1;
  return openfiles;
}

long long sys_open_inodes() {
  struct statfs s;
  if (statfs("/", &s) == -1) {
    return -1;
  }
  return s.f_files - s.f_ffree;
}

long long sys_max_files() {
  u_int maxfiles;
  size_t len;
  len = sizeof(len);
  if (sysctlbyname("kern.maxfiles", &maxfiles, &len, NULL, 0) == -1) {
    return -1;
  }
  return maxfiles;
}

void sys_stat_info() {
  long cp_time[CPUSTATES];
  size_t len = sizeof(cp_time);
  if (sysctlbyname("kern.cp_time", cp_time, &len, NULL, 0) == -1) {
    no_info();
    return;
  }
  print("cpu ");
  for (int i = 0; i < CPUSTATES; i++) {
    print("%ld ", cp_time[i]);
  }
  newln();

  int ncpu;
  len = sizeof(ncpu);
  if (sysctlbyname("hw.ncpu", &ncpu, &len, NULL, 0) == -1) {
    no_info();
    return;
  }

  long cp_times[ncpu * CPUSTATES];
  len = sizeof(cp_times);
  if (sysctlbyname("kern.cp_times", cp_times, &len, NULL, 0) == -1) {
    no_info();
  }
  for (int cpu = 0; cpu < ncpu; cpu++) {
    print("cpu%d ", cpu);
    long *times = &cp_times[cpu * CPUSTATES];
    for (int t = 0; t < CPUSTATES; t++) {
      print("%ld ", times[t]);
    }
    newln();
  }
  return;
}

void sys_modules_info() {
  int id = 0;
  struct kld_file_stat stat;

  // 0 indicates end of modules list
  while ((id = kldnext(id)) != 0) {
    if (id == -1) {
      no_info();
      return;
    }

    memset(&stat, 0, sizeof(stat));
    stat.version = sizeof(stat);

    if (kldstat(id, &stat) == -1) {
      no_info();
      return;
    }

    println("%s %zu %d - Live %p", stat.name, stat.size, stat.refs,
            stat.address);
    newln();
  }
}

static struct flaglist {
  int flag;
  const char str[32]; /* must be longer than the longest one. */
} fl[] = {
    {
        .flag = VFCF_STATIC,
        .str = "static",
    },
    {
        .flag = VFCF_NETWORK,
        .str = "network",
    },
    {
        .flag = VFCF_READONLY,
        .str = "read-only",
    },
    {
        .flag = VFCF_SYNTHETIC,
        .str = "synthetic",
    },
    {
        .flag = VFCF_LOOPBACK,
        .str = "loopback",
    },
    {
        .flag = VFCF_UNICODE,
        .str = "unicode",
    },
    {
        .flag = VFCF_JAIL,
        .str = "jail",
    },
    {
        .flag = VFCF_DELEGADMIN,
        .str = "delegated-administration",
    },
};

static const char *fmt_flags(int flags) {
  static char buf[sizeof(struct flaglist) * sizeof(fl)];

  buf[0] = '\0';
  for (size_t i = 0; i < (int)nitems(fl); i++) {
    if ((flags & fl[i].flag) != 0) {
      strlcat(buf, fl[i].str, sizeof(buf));
      strlcat(buf, ", ", sizeof(buf));
    }
  }

  /* Zap the trailing comma + space. */
  if (buf[0] != '\0')
    buf[strlen(buf) - 2] = '\0';
  return (buf);
}

void sys_filesystems_info() {
  struct xvfsconf *xvfsp;
  size_t buf;
  if (sysctlbyname("vfs.conflist", NULL, &buf, NULL, 0) == -1) {
    no_info();
    return;
  }
  xvfsp = malloc(buf);
  if (sysctlbyname("vfs.conflist", xvfsp, &buf, NULL, 0) == -1) {
    no_info();
    free(xvfsp);
    return;
  }

  println("%-20.20s  %s\n", "Filesystem", "Flags");
  println("-------------------- -------------------\n");
  size_t cnt = buf / sizeof(struct xvfsconf);
  for (int i = 0; i < cnt; i++) {

    println("%-20.20s %s\n", xvfsp[i].vfc_name, fmt_flags(xvfsp[i].vfc_flags));
  }
  free(xvfsp);
}

void sys_partitions_info() {
  struct gmesh mesh;
  struct gclass *classp;
  struct ggeom *geomp;
  struct gprovider *providerp;
  if (geom_gettree(&mesh) == -1) {
    no_info();
    return;
  };

  println("%-20.20s %s\n", "Name", "Size");
  println("-------------------- -------------------\n");
  LIST_FOREACH(classp, &mesh.lg_class, lg_class) {
    if (strcmp(classp->lg_name, "DISK") != 0 &&
        strcmp(classp->lg_name, "PART") != 0) {
      continue;
    }
    LIST_FOREACH(geomp, &classp->lg_geom, lg_geom) {
      LIST_FOREACH(providerp, &geomp->lg_provider, lg_provider) {
        println("%-20.20s %lld\n", providerp->lg_name,
                (unsigned long long)providerp->lg_mediasize);
      }
    }
  }
  geom_deletetree(&mesh);
}

static int print_device(struct devinfo_dev *dev, void *arg) {
  char *name = dev->dd_name[0] ? dev->dd_name : "unknown";
  char *driver = dev->dd_drivername ? dev->dd_drivername : "unknown";
  if (dev->dd_name[0] == '\0' || dev->dd_state < DS_ATTACHED)
    return devinfo_foreach_device_child(dev, print_device, NULL);
  println("%-20.20s %-20.20s\n", name, driver);
  return (devinfo_foreach_device_child(dev, print_device, NULL));
}

void sys_devices_info() {
  struct devinfo_dev *root;
  int rv;
  if ((rv = devinfo_init()) != 0) {
    no_info();
    return;
  }

  if ((root = devinfo_handle_to_device(DEVINFO_ROOT_DEVICE)) == NULL) {
    no_info();
    return;
  }

  println("%-20.20s %-20.20s\n", "NAME", "DRIVER");
  println("-------------------- -------------------\n");
  devinfo_foreach_device_child(root, print_device, NULL);
}

time_t sys_start_time(int pid) {
  struct kinfo_proc kp;
  int mib[4];
  size_t len = sizeof(kp);
  mib[0] = CTL_KERN;
  mib[1] = KERN_PROC;
  mib[2] = KERN_PROC_PID;
  mib[3] = pid;

  if (sysctl(mib, 4, &kp, &len, NULL, 0) == -1) {
    return -1;
  }

  if (len == 0)
    return -1;

  return kp.ki_start.tv_sec;
}

void sys_proc_exe(int pid) {
  int mib[4];
  size_t len = sizeof(mib);
  if (sysctlnametomib("kern.proc.pathname", mib, &len) == -1) {
    no_info();
    return;
  }

  mib[3] = pid;
  char path[MAXPATHLEN];
  len = sizeof(path);

  if (sysctl(mib, 4, path, &len, NULL, 0) == -1) {
    no_info();
    return;
  }
  print("%s\n", path);
}

void sys_proc_root(int pid) { no_info(); }

void sys_proc_cwd(int pid) { no_info(); }

void sys_proc_status(int pid) { no_info(); }

void sys_proc_fds(int pid) { no_info(); }
