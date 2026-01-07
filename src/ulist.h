#ifndef _ULIST_H_
#define _ULIST_H_

#include <stdint.h> /* uint32_t etc */
#include <utmpx.h>  /* struct utmpx, USER_PROCESS */

#include "list.h" /* struct list_head, DECL_LIST_HEAD */
#include "var.h"  /* u32 typedefs */

#define LOGIN 1
#define LOGOUT -1

#ifdef HAVE_UT_NAME
#define ut_user ut_name
#endif

#define UT_NAMESIZE sizeof(((struct utmpx *)0)->ut_user)
#define UT_LINESIZE sizeof(((struct utmpx *)0)->ut_line)
#define UT_HOSTSIZE sizeof(((struct utmpx *)0)->ut_host)

struct user_t {
  struct list_head head;
  char name[UT_NAMESIZE + 1];
  char tty[UT_LINESIZE + 1];
  int pid;
  char parent[16];
  char host[UT_HOSTSIZE + 1];
  int line;
};

static DECL_LIST_HEAD(users_l);
static int toggle; /* if 0 show cmd line else show idle time 	*/
// static char *uhelp = "\001[F1]Help [F9]Menu [ENT]proc all[t]ree [i]dle/cmd "
//			" [c]md [d]etails [s]ysinfo";

#ifdef HAVE_PROCESS_SYSCTL
int get_login_pid(char *);
#endif

struct prot_t {
  char *s;
  short port;
  u32 nr;
};
#endif
