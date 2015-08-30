/*
 *  ircd-ratbox: A slightly useful ircd.
 *  unix.c: various unix type functions
 *
 *  Copyright (C) 1990 Jarkko Oikarinen and University of Oulu, Co Center
 *  Copyright (C) 2005-2012 ircd-ratbox development team
 *  Copyright (C) 2005 Aaron Sethman <androsyn@ratbox.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 *
 */
#include <libratbox_config.h>
#include <ratbox_lib.h>
#include <commio-int.h>

#include <sys/wait.h>

#if defined(HAVE_SPAWN_H) && defined(HAVE_POSIX_SPAWN)
#include <spawn.h>

#ifdef __APPLE__
#include <crt_externs.h>
#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#ifndef HAVE_DECL_ENVIRON
extern char **environ;
#endif
pid_t
rb_spawn_process(const char *path, const char **argv)
{
    pid_t pid;
    const void *arghack = argv;
    char **myenviron;
    int error;
    posix_spawnattr_t spattr;
    posix_spawnattr_init(&spattr);
#ifdef POSIX_SPAWN_USEVFORK
    posix_spawnattr_setflags(&spattr, POSIX_SPAWN_USEVFORK);
#endif
#ifdef __APPLE__
    myenviron = *_NSGetEnviron(); /* apple needs to go fuck themselves for this */
#else
    myenviron = environ;
#endif
    error = posix_spawn(&pid, path, NULL, &spattr, arghack, myenviron);
    posix_spawnattr_destroy(&spattr);
    if (error != 0) {
        errno = error;
        pid = -1;
    }
    return pid;
}
#else
pid_t
rb_spawn_process(const char *path, const char **argv)
{
    pid_t pid;
    if(!(pid = vfork())) {
        execv(path, (const void *)argv);	/* make gcc shut up */
        _exit(1);	/* if we're still here, we're screwed */
    }
    return (pid);
}
#endif

#ifndef HAVE_GETTIMEOFDAY
int
rb_gettimeofday(struct timeval *tv, void *tz)
{
    if(tv == NULL) {
        errno = EFAULT;
        return -1;
    }
    tv->tv_usec = 0;
    if(time(&tv->tv_sec) == -1)
        return -1;
    return 0;
}
#else
int
rb_gettimeofday(struct timeval *tv, void *tz)
{
    return (gettimeofday(tv, tz));
}
#endif

void
rb_sleep(unsigned int seconds, unsigned int useconds)
{
#ifdef HAVE_NANOSLEEP
    struct timespec tv;
    tv.tv_nsec = (useconds * 1000);
    tv.tv_sec = seconds;
    nanosleep(&tv, NULL);
#else
    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = useconds;
    select(0, NULL, NULL, NULL, &tv);
#endif
}

/* this is to keep some linkers from bitching about exporting a non-existant symbol..bleh */
char *
rb_strerror(int error)
{
    return strerror(error);
}

int
rb_ignore_errno(int error)
{
    switch (error) {
#ifdef EINPROGRESS
    case EINPROGRESS:
#endif
#if defined EWOULDBLOCK
    case EWOULDBLOCK:
#endif
#if defined(EAGAIN) && (EWOULDBLOCK != EAGAIN)
    case EAGAIN:
#endif
#ifdef EINTR
    case EINTR:
#endif
#ifdef ERESTART
    case ERESTART:
#endif
#ifdef ENOBUFS
    case ENOBUFS:
#endif
        return 1;
    default:
        break;
    }
    return 0;
}

int
rb_kill(pid_t pid, int sig)
{
    return kill(pid, sig);
}

int
rb_setenv(const char *name, const char *value, int overwrite)
{
    return setenv(name, value, overwrite);
}

pid_t
rb_waitpid(pid_t pid, int *status, int options)
{
    return waitpid(pid, status, options);
}

pid_t
rb_getpid(void)
{
    return getpid();
}

int
rb_recv_fd_buf(rb_fde_t *F, void *data, size_t datasize, rb_fde_t **xF, int nfds)
{
    struct msghdr msg;
    struct cmsghdr *cmsg;
    struct iovec iov[1];
    struct stat st;
    uint8_t stype = RB_FD_UNKNOWN;
    const char *desc;
    int fd, len, x, rfds;

    int control_len = CMSG_SPACE(sizeof(int) * nfds);

    iov[0].iov_base = data;
    iov[0].iov_len = datasize;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    msg.msg_flags = 0;
    cmsg = alloca(control_len);
    msg.msg_control = cmsg;
    msg.msg_controllen = control_len;

    if((len = recvmsg(rb_get_fd(F), &msg, 0)) <= 0)
        return len;

    if(msg.msg_controllen > 0 && msg.msg_control != NULL
       && (cmsg = CMSG_FIRSTHDR(&msg)) != NULL) {
        rfds = ((unsigned char *)cmsg + cmsg->cmsg_len - CMSG_DATA(cmsg)) / sizeof(int);

        for(x = 0; x < nfds && x < rfds; x++) {
            fd = ((int *)CMSG_DATA(cmsg))[x];
            stype = RB_FD_UNKNOWN;
            desc = "remote unknown";
            if(!fstat(fd, &st)) {
                if(S_ISSOCK(st.st_mode)) {
                    stype = RB_FD_SOCKET;
                    desc = "remote socket";
                } else if(S_ISFIFO(st.st_mode)) {
                    stype = RB_FD_PIPE;
                    desc = "remote pipe";
                } else if(S_ISREG(st.st_mode)) {
                    stype = RB_FD_FILE;
                    desc = "remote file";
                }
            }
            xF[x] = rb_open(fd, stype, desc);
        }
    } else
        *xF = NULL;
    return len;
}

int
rb_send_fd_buf(rb_fde_t *xF, rb_fde_t **F, int count, void *data, size_t datasize, __unused pid_t pid)
{
    int n;
    struct msghdr msg;
    struct cmsghdr *cmsg;
    struct iovec iov[1];
    char empty = '0';
    char *buf;

    memset(&msg, 0, sizeof(msg));
    if(datasize == 0) {
        iov[0].iov_base = &empty;
        iov[0].iov_len = 1;
    } else {
        iov[0].iov_base = data;
        iov[0].iov_len = datasize;
    }
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_flags = 0;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;

    if(count > 0) {
        int i;
        int len = CMSG_SPACE(sizeof(int) * count);
        buf = alloca(len);

        msg.msg_control = buf;
        msg.msg_controllen = len;
        cmsg = CMSG_FIRSTHDR(&msg);
        if(!cmsg)
            return 0;
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;
        cmsg->cmsg_len = CMSG_LEN(sizeof(int) * count);

        for(i = 0; i < count; i++) {
            ((int *)CMSG_DATA(cmsg))[i] = rb_get_fd(F[i]);
        }
        msg.msg_controllen = cmsg->cmsg_len;
    }
    n = sendmsg(rb_get_fd(xF), &msg, MSG_NOSIGNAL);
    return n;
}

static int
set_fd_flag(int fd, int flag)
{
    int flags = fcntl(fd, F_GETFD);
    if(flags & flag) /* Flag already set */
        return 0;
    return fcntl(fd, F_SETFD, flags | flag);
}

static int
unset_fd_flag(int fd, int flag)
{
    int flags = fcntl(fd, F_GETFD);
    if(!(flags & flag)) /* Flag already unset */
        return 0;
    return fcntl(fd, F_SETFD, flags & ~flag);
}

int
rb_set_inherit(rb_fde_t *F, int inherit)
{
    if(inherit)
        return unset_fd_flag(F->fd, FD_CLOEXEC);
    else
        return set_fd_flag(F->fd, FD_CLOEXEC);
}
