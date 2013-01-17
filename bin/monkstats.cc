#include "types.h"
#include "user.h"
#include "kstats.hh"

#include <fcntl.h>

int
xread(int fd, const void *buf, size_t n)
{
  size_t pos = 0;
  while (pos < n) {
    int r = read(fd, (char*)buf + pos, n - pos);
    if (r < 0)
      die("read failed");
    if (r == 0)
      break;
    pos += r;
  }
  return pos;
}

static void
read_kstats(kstats *out)
{
  int fd = open("/dev/kstats", O_RDONLY);
  if (fd < 0)
    die("Couldn't open /dev/kstats");
  int r = xread(fd, out, sizeof *out);
  if (r != sizeof *out)
    die("Short read from /dev/kstats");
  close(fd);
}

int
main(int ac, const char *av[])
{
  struct kstats kstats_before, kstats_after;

  read_kstats(&kstats_before);

  int pid = fork(0);
  if (pid < 0)
    die("monkstats: fork failed");

  if (pid == 0) {
    exec(av[1], av + 1);
    die("monkstats: exec failed");
  }

  wait(-1);

  read_kstats(&kstats_after);

  struct kstats kstats = kstats_after - kstats_before;

  // XXX Assumes uint64_t.  Use to_stream instead.
  // XXX Would be nice if this knew what fields were relative to other
  // fields and could divide them for you.
#define X(type, name) printf("%lu " #name "\n", kstats.name);
  KSTATS_ALL(X);
#undef X
  printf("\n");
}