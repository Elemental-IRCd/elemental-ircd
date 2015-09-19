/*
 *  ircd-ratbox: A slightly useful ircd.
 *  windows.c: various unix type functions (win32 shims)
 *
 *  Copyright (C) 1990 Jarkko Oikarinen and University of Oulu, Co Center
 *  Copyright (C) 1996-2002 Hybrid Development Team
 *  Copyright (C) 2001 Adrian Chadd <adrian@creative.net.au>
 *  Copyright (C) 2005-2006 Aaron Sethman <androsyn@ratbox.org>
 *  Copyright (C) 2002-2012 ircd-ratbox development team
 *  Copyright (C) 2015 elemental-ircd development team
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

/*
 * having gettimeofday is nice...
 */

typedef union {
    unsigned __int64 ft_i64;
    FILETIME ft_val;
} FT_t;

#ifdef __GNUC__
#define Const64(x) x##LL
#else
#define Const64(x) x##i64
#endif
/* Number of 100 nanosecond units from 1/1/1601 to 1/1/1970 */
#define EPOCH_BIAS  Const64(116444736000000000)

pid_t
rb_getpid()
{
    return GetCurrentProcessId();
}


int
rb_gettimeofday(struct timeval *tp, void *not_used)
{
    FT_t ft;

    /* this returns time in 100-nanosecond units  (i.e. tens of usecs) */
    GetSystemTimeAsFileTime(&ft.ft_val);

    /* seconds since epoch */
    tp->tv_sec = (long)((ft.ft_i64 - EPOCH_BIAS) / Const64(10000000));

    /* microseconds remaining */
    tp->tv_usec = (long)((ft.ft_i64 / Const64(10)) % Const64(1000000));

    return 0;
}


pid_t
rb_spawn_process(const char *path, const char **argv)
{
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    char cmd[MAX_PATH];
    memset(&pi, 0, sizeof(pi));
    memset(&si, 0, sizeof(si));
    rb_strlcpy(cmd, path, sizeof(cmd));
    if(CreateProcess(cmd, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi) == FALSE)
        return -1;

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return (pi.dwProcessId);
}

pid_t
rb_waitpid(pid_t pid, int *status, int flags)
{
    DWORD timeout = (flags & WNOHANG) ? 0 : INFINITE;
    HANDLE hProcess;
    DWORD waitcode;

    hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, pid);
    if(!hProcess) {
        errno = ECHILD;
        return -1;
    }

    waitcode = WaitForSingleObject(hProcess, timeout);
    if(waitcode == WAIT_TIMEOUT) {
        CloseHandle(hProcess);
        return 0;
    }

    if(waitcode == WAIT_OBJECT_0) {
        if(GetExitCodeProcess(hProcess, &waitcode)) {
            *status = (int)((waitcode & 0xff) << 8);
            CloseHandle(hProcess);
            return pid;
        }
    }
    CloseHandle(hProcess);

    return -1;
}

int
rb_setenv(const char *name, const char *value, int overwrite)
{
    char *buf;
    int len;
    if(!overwrite) {
        if((buf = getenv(name)) != NULL) {
            if(strlen(buf) > 0) {
                return 0;
            }
        }
    }
    if(name == NULL || value == NULL)
        return -1;
    len = strlen(name) + strlen(value) + 5;
    buf = rb_malloc(len);
    snprintf(buf, len, "%s=%s", name, value);
    len = putenv(buf);
    rb_free(buf);
    return (len);
}

int
rb_kill(pid_t pid, int sig)
{
    HANDLE hProcess;
    int ret = -1;
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, pid);

    if(hProcess) {
        switch (sig) {
        case 0:
            ret = 0;
            break;

        default:
            if(TerminateProcess(hProcess, sig))
                ret = 0;
            break;
        }
        CloseHandle(hProcess);
    } else
        errno = EINVAL;

    return ret;


}

/*
 * packet format is
 uint32_t magic
 uint8_t protocol count
 WSAPROTOCOL_INFO * count
 size_t datasize
 data

 */

static int
make_wsaprotocol_info(pid_t process, rb_fde_t *F, WSAPROTOCOL_INFO * inf)
{
    WSAPROTOCOL_INFO info;
    if(!WSADuplicateSocket((SOCKET) rb_get_fd(F), process, &info)) {
        memcpy(inf, &info, sizeof(WSAPROTOCOL_INFO));
        return 1;
    }
    return 0;
}

static rb_fde_t *
make_fde_from_wsaprotocol_info(void *data)
{
    WSAPROTOCOL_INFO *info = data;
    SOCKET t;
    t = WSASocket(FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO, info, 0, 0);
    if(t == INVALID_SOCKET)
        return NULL;

    return rb_open(t, RB_FD_SOCKET, "remote_socket");
}

static uint8_t fd_buf[16384];
#define MAGIC_CONTROL 0xFF0ACAFE

int
rb_send_fd_buf(rb_fde_t *xF, rb_fde_t **F, int count, void *data, size_t datasize, pid_t pid)
{
    size_t bufsize =
        sizeof(uint32_t) + sizeof(uint8_t) + (sizeof(WSAPROTOCOL_INFO) * (size_t)count) +
        sizeof(size_t) + datasize;
    int i;
    uint32_t magic = MAGIC_CONTROL;
    void *ptr;
    if(count > 4) {
        errno = EINVAL;
        return -1;
    }
    if(bufsize > sizeof(fd_buf)) {
        errno = EINVAL;
        return -1;
    }
    memset(fd_buf, 0, sizeof(fd_buf));

    ptr = fd_buf;
    memcpy(ptr, &magic, sizeof(magic));
    ptr = (void *)((uintptr_t)ptr + (uintptr_t)sizeof(magic));
    *((uint8_t *)ptr) = count;
    ptr = (void *)((uintptr_t)ptr + (uintptr_t)sizeof(uint8_t));

    for(i = 0; i < count; i++) {
        make_wsaprotocol_info(pid, F[i], (WSAPROTOCOL_INFO *) ptr);
        ptr = (void *)((uintptr_t)ptr + (uintptr_t)sizeof(WSAPROTOCOL_INFO));
    }
    memcpy(ptr, &datasize, sizeof(size_t));
    ptr = (void *)((uintptr_t)ptr + (uintptr_t)sizeof(size_t));
    memcpy(ptr, data, datasize);
    return rb_write(xF, fd_buf, bufsize);
}

#ifdef MYMIN
#undef MYMIN
#endif
#define MYMIN(a, b)  ((a) < (b) ? (a) : (b))

int
rb_recv_fd_buf(rb_fde_t *F, void *data, size_t datasize, rb_fde_t **xF, int nfds)
{
    size_t minsize = sizeof(uint32_t) + sizeof(uint8_t) + sizeof(size_t);
    size_t datalen;
    ssize_t retlen;
    uint32_t magic;
    uint8_t count;
    unsigned int i;
    void *ptr;
    ssize_t ret;
    memset(fd_buf, 0, sizeof(fd_buf));	/* some paranoia here... */
    ret = rb_read(F, fd_buf, sizeof(fd_buf));
    if(ret <= 0) {
        return ret;
    }
    if(ret < (ssize_t) minsize) {
        errno = EINVAL;
        return -1;
    }
    ptr = fd_buf;
    memcpy(&magic, ptr, sizeof(uint32_t));
    if(magic != MAGIC_CONTROL) {
        errno = EAGAIN;
        return -1;
    }
    ptr = (void *)((uintptr_t)ptr + (uintptr_t)sizeof(uint32_t));
    memcpy(&count, ptr, sizeof(uint8_t));
    ptr = (void *)((uintptr_t)ptr + (uintptr_t)sizeof(uint8_t));
    for(i = 0; i < count && i < (unsigned int)nfds; i++) {
        rb_fde_t *tF = make_fde_from_wsaprotocol_info(ptr);
        if(tF == NULL)
            return -1;
        xF[i] = tF;
        ptr = (void *)((uintptr_t)ptr + (uintptr_t)sizeof(WSAPROTOCOL_INFO));
    }
    memcpy(&datalen, ptr, sizeof(size_t));
    ptr = (void *)((uintptr_t)ptr + (uintptr_t)sizeof(size_t));
    retlen = MYMIN(datalen, datasize);
    memcpy(data, ptr, datalen);
    return retlen;
}

void
rb_sleep(unsigned int seconds, unsigned int useconds)
{
    DWORD msec = seconds * 1000;;
    Sleep(msec);
}

#ifdef strerror
#undef strerror
#endif

static const char *
_rb_strerror(int error)
{
    switch (error) {
    case 0:
        return "Success";
    case WSAEINTR:
        return "Interrupted system call";
    case WSAEBADF:
        return "Bad file number";
    case WSAEACCES:
        return "Permission denied";
    case WSAEFAULT:
        return "Bad address";
    case WSAEINVAL:
        return "Invalid argument";
    case WSAEMFILE:
        return "Too many open sockets";
    case WSAEWOULDBLOCK:
        return "Operation would block";
    case WSAEINPROGRESS:
        return "Operation now in progress";
    case WSAEALREADY:
        return "Operation already in progress";
    case WSAENOTSOCK:
        return "Socket operation on non-socket";
    case WSAEDESTADDRREQ:
        return "Destination address required";
    case WSAEMSGSIZE:
        return "Message too long";
    case WSAEPROTOTYPE:
        return "Protocol wrong type for socket";
    case WSAENOPROTOOPT:
        return "Bad protocol option";
    case WSAEPROTONOSUPPORT:
        return "Protocol not supported";
    case WSAESOCKTNOSUPPORT:
        return "Socket type not supported";
    case WSAEOPNOTSUPP:
        return "Operation not supported on socket";
    case WSAEPFNOSUPPORT:
        return "Protocol family not supported";
    case WSAEAFNOSUPPORT:
        return "Address family not supported";
    case WSAEADDRINUSE:
        return "Address already in use";
    case WSAEADDRNOTAVAIL:
        return "Can't assign requested address";
    case WSAENETDOWN:
        return "Network is down";
    case WSAENETUNREACH:
        return "Network is unreachable";
    case WSAENETRESET:
        return "Net connection reset";
    case WSAECONNABORTED:
        return "Software caused connection abort";
    case WSAECONNRESET:
        return "Connection reset by peer";
    case WSAENOBUFS:
        return "No buffer space available";
    case WSAEISCONN:
        return "Socket is already connected";
    case WSAENOTCONN:
        return "Socket is not connected";
    case WSAESHUTDOWN:
        return "Can't send after socket shutdown";
    case WSAETOOMANYREFS:
        return "Too many references, can't splice";
    case WSAETIMEDOUT:
        return "Connection timed out";
    case WSAECONNREFUSED:
        return "Connection refused";
    case WSAELOOP:
        return "Too many levels of symbolic links";
    case WSAENAMETOOLONG:
        return "File name too long";
    case WSAEHOSTDOWN:
        return "Host is down";
    case WSAEHOSTUNREACH:
        return "No route to host";
    case WSAENOTEMPTY:
        return "Directory not empty";
    case WSAEPROCLIM:
        return "Too many processes";
    case WSAEUSERS:
        return "Too many users";
    case WSAEDQUOT:
        return "Disc quota exceeded";
    case WSAESTALE:
        return "Stale NFS file handle";
    case WSAEREMOTE:
        return "Too many levels of remote in path";
    case WSASYSNOTREADY:
        return "Network system is unavailable";
    case WSAVERNOTSUPPORTED:
        return "Winsock version out of range";
    case WSANOTINITIALISED:
        return "WSAStartup not yet called";
    case WSAEDISCON:
        return "Graceful shutdown in progress";
    case WSAHOST_NOT_FOUND:
        return "Host not found";
    case WSANO_DATA:
        return "No host data of that type was found";
    default:
        return strerror(error);
    }
};

char *
rb_strerror(int error)
{
    static char buf[128];
    rb_strlcpy(buf, _rb_strerror(error), sizeof(buf));
    return buf;
}

/* TODO: Translate winsock errnos'
 *
 * WSAENOBUFS      -> ENOBUFS
 * WSAEINPROGRESS  -> EINPROGRESS
 * WSAEWOULDBLOCK  -> EWOULDBLOCK
 * WSAEMSGSIZE     -> EMSGSIZE
 * WSAEALREADY     -> EALREADY
 * WSAEISCONN      -> EISCONN
 * WSAEADDRINUSE   -> EADDRINUSE
 * WSAEAFNOSUPPORT -> EAFNOSUPPORT
 */

int
rb_ignore_errno(int error)
{
    switch (error) {
#ifdef WSAEINPROGRESS
    case WSAEINPROGRESS:
#endif
#if defined WSAEWOULDBLOCK
    case WSAEWOULDBLOCK:
#endif
#if defined(WSAEAGAIN) && (WSAEWOULDBLOCK != WSAEAGAIN)
    case WSAEAGAIN:
#endif
#ifdef WSAEINTR
    case WSAEINTR:
#endif
#ifdef WSAERESTART
    case WSAERESTART:
#endif
#ifdef WSAENOBUFS
    case WSAENOBUFS:
#endif
        return 1;
    default:
        break;
    }
    return 0;
}

int
rb_set_inherit(rb_fde_t *F, int inherit)
{
    return SetHandleInformation((HANDLE) F->fd, HANDLE_FLAG_INHERIT, !!inherit);
}
