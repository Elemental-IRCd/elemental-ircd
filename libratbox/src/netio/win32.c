/*
 *  ircd-ratbox: A slightly useful ircd.
 *  win32.c: select() compatible network routines.
 *
 *  Copyright (C) 1990 Jarkko Oikarinen and University of Oulu, Co Center
 *  Copyright (C) 1996-2002 Hybrid Development Team
 *  Copyright (C) 2001 Adrian Chadd <adrian@creative.net.au>
 *  Copyright (C) 2005-2006 Aaron Sethman <androsyn@ratbox.org>
 *  Copyright (C) 2002-2012 ircd-ratbox development team
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

#ifdef WITH_WIN32_SELECT

#define WM_SOCKET WM_USER

static HWND hwnd;

static LRESULT CALLBACK
rb_process_events(HWND nhwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
    rb_fde_t *F;
    PF *hdl;
    void *data;
    switch (umsg) {
    case WM_SOCKET: {
        F = rb_find_fd(wparam);

        if(F != NULL && IsFDOpen(F)) {
            switch (WSAGETSELECTEVENT(lparam)) {
            case FD_ACCEPT:
            case FD_CLOSE:
            case FD_READ: {
                if((hdl = F->read_handler) != NULL) {
                    F->read_handler = NULL;
                    data = F->read_data;
                    F->read_data = NULL;
                    hdl(F, data);
                }
                break;
            }

            case FD_CONNECT:
            case FD_WRITE: {
                if((hdl = F->write_handler) != NULL) {
                    F->write_handler = NULL;
                    data = F->write_data;
                    F->write_data = NULL;
                    hdl(F, data);
                }
            }
            }

        }
        return 0;
    }
    case WM_DESTROY: {
        PostQuitMessage(0);
        return 0;
    }

    default:
        return DefWindowProc(nhwnd, umsg, wparam, lparam);
    }
    return 0;
}

int
rb_init_netio_win32(void)
{
    /* this muchly sucks, but i'm too lazy to do overlapped i/o, maybe someday... -androsyn */
    WNDCLASS wc;
    static const char *classname = "ircd-ratbox-class";

    wc.style = 0;
    wc.lpfnWndProc = (WNDPROC) rb_process_events;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hIcon = NULL;
    wc.hCursor = NULL;
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = classname;
    wc.hInstance = GetModuleHandle(NULL);

    if(!RegisterClass(&wc))
        rb_lib_die("cannot register window class");

    hwnd = CreateWindow(classname, classname, WS_POPUP, CW_USEDEFAULT,
                        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                        (HWND) NULL, (HMENU) NULL, wc.hInstance, NULL);
    if(!hwnd)
        rb_lib_die("could not create window");
    return 0;
}

void
rb_setselect_win32(rb_fde_t *F, unsigned int type, PF * handler, void *client_data)
{
    int old_flags = F->pflags;

    lrb_assert(IsFDOpen(F));

    /* Update the list, even though we're not using it .. */
    if(type & RB_SELECT_READ) {
        if(handler != NULL)
            F->pflags |= FD_CLOSE | FD_READ | FD_ACCEPT;
        else
            F->pflags &= ~(FD_CLOSE | FD_READ | FD_ACCEPT);
        F->read_handler = handler;
        F->read_data = client_data;
    }

    if(type & RB_SELECT_WRITE) {
        if(handler != NULL)
            F->pflags |= FD_WRITE | FD_CONNECT;
        else
            F->pflags &= ~(FD_WRITE | FD_CONNECT);
        F->write_handler = handler;
        F->write_data = client_data;
    }

    if(old_flags == 0 && F->pflags == 0)
        return;

    if(F->pflags != old_flags) {
        WSAAsyncSelect(F->fd, hwnd, WM_SOCKET, F->pflags);
    }

}

static int has_set_timer = 0;

int
rb_select_win32(long delay)
{
    MSG msg;
    if(has_set_timer == 0) {
        /* XXX should probably have this handle all the events
         * instead of busy looping
         */
        SetTimer(hwnd, 0, delay, NULL);
        has_set_timer = 1;
    }

    if(GetMessage(&msg, NULL, 0, 0) == FALSE) {
        rb_lib_die("GetMessage failed..byebye");
    }
    rb_set_time();

    DispatchMessage(&msg);
    return RB_OK;
}

int
rb_setup_fd_win32(rb_fde_t *F)
{
    if(F == NULL)
        return 0;

    switch (F->type) {
    case RB_FD_SOCKET: {
        u_long nonb = 1;
        if(ioctlsocket((SOCKET) F->fd, FIONBIO, &nonb) == -1) {
            rb_get_errno();
            return 0;
        }
        return 1;
    }
    default:
        return 1;

    }
}

#endif /* WITH_WIN32_SELECT */
