#ifndef PSEUDO_TERMINAL_HPP
# define PSEUDO_TERMINAL_HPP

#include <sys/types.h>

class PseudoTerminal
{
public:
    static int loginTty(int fd);
    static int openPty(int *ptm, int *pts, const struct winsize *winp);

    static int grant(char *pts_name);
    static int unlock(int ptm);
    static int slaveNameReentrant(int ptm, char *pts_name, size_t size);
};

#endif // !PSEUDO_TERMINAL_HPP
