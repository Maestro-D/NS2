#include <linux/limits.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <grp.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include "PseudoTerminal.hpp"

int PseudoTerminal::loginTty(int fd)
{
  char *fdname = NULL;
  int newfd;

  if (setsid() == -1)
  {
    std::cerr << "Error " << errno << ": setsid() failure" << std::endl;
    return (-1);
  }
  if (ioctl(fd, TIOCSCTTY, (char *)NULL) == -1)
  {
    std::cerr << "Error " << errno << ": ioctl() failure" << std::endl;
    return (-1);
  }

  if ((fdname = ttyname(fd)))
  {
    if (fd != 0)
      (void) close(0);
    if (fd != 1)
      (void) close(1);
    if (fd != 2)
      (void) close(2);
    newfd = open(fdname, O_RDWR);
    (void) close(newfd);
  }

  (void) dup2(fd, 0);
  (void) dup2(fd, 1);
  (void) dup2(fd, 2);
  if (fd > 2)
    (void)close(fd);

  return (0);
}

int PseudoTerminal::openPty(int *ptm, int *pts, const struct winsize *winp)
{
  char pts_name[PATH_MAX];

  memset(pts_name, 0, PATH_MAX);
  if (((*ptm) = open("/dev/ptmx", O_RDWR)) == -1)
  {
      std::cerr << "Error " << errno << ": open() failure" << std::endl;
      return (-1);
  }
  if (slaveNameReentrant((*ptm), pts_name, sizeof(pts_name)) == -1)
  {
      return (-1);
  }
  if (grant(pts_name) == -1)
  {
      std::cerr << "Error: my_grantpt() failure" << std::endl;
      return (-1);
  }
  if (unlock(*ptm) == -1)
  {
      std::cerr << "Error: my_unlockpt() failure" << std::endl;
      return (-1);
  }
  if (((*pts) = open(pts_name, O_RDWR)) == -1)
  {
      std::cerr << "Error " << errno << ": open() failure" << std::endl;
      return (-1);
  }
  if (ioctl(*ptm, TIOCSWINSZ, winp) == -1)
  {
      std::cerr << "Error " << errno << ": ioctl() failure" << std::endl;
      return (-1);
  }
  return (0);
}

int PseudoTerminal::grant(char *pts_name)
{
  struct group *grp = NULL;
  uid_t uid = getuid();

  if (!(grp = getgrnam("tty")))
  {
      std::cerr << "Error " << errno << ": getgrnam() failure" << std::endl;
      return (-1);
  }
  if ((chmod(pts_name, 00620)) == -1)
  {
      std::cerr << "Error " << errno << ": chmod() failure" << std::endl;
      return (-1);
  }
  if ((chown(pts_name, uid, grp->gr_gid)) == -1)
  {
      std::cerr << "Error " << errno << ": chown() failure" << std::endl;
      return (-1);
  }
  return (0);
}

int PseudoTerminal::unlock(int ptm)
{
  int unlock = 0;

  if (ioctl(ptm, TIOCSPTLCK, &unlock))
  {
    std::cerr << "Error " << errno << ": ioctl() failure" << std::endl;
    return (-1);
  }
  return (0);
}

int PseudoTerminal::slaveNameReentrant(int ptm, char *pts_name, size_t size)
{
  unsigned int pts_no = 0;
  int end = 0;

  if (ioctl(ptm, TIOCGPTN, &pts_no))
    {
      std::cerr << "Error " << errno << ": ioctl() failure" << std::endl;
      return (-1);
    }
  end = snprintf(pts_name, size, "/dev/pts/%u", pts_no);
  pts_name[end] = '\0';
  return (0);
}
