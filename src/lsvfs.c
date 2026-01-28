#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/sysctl.h>

static const char *fmt_flags(int flags);

int main() {
  struct xvfsconf *xvfsp;
  size_t buf;
  sysctlbyname("vfs.conflist", NULL, &buf, NULL, 0);
  xvfsp = malloc(buf);
  sysctlbyname("vfs.conflist", xvfsp, &buf, NULL, 0);

  printf("%-16.16s  %s\n", "Filesystem", "Flags");
  printf("---------------- ---------------\n");
  size_t cnt = buf / sizeof(struct xvfsconf);
  for (int i = 0; i < cnt; i++) {

    printf("%-16.16s %s\n", xvfsp[i].vfc_name, fmt_flags(xvfsp[i].vfc_flags));
  }
  free(xvfsp);
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
