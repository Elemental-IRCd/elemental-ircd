/*
 *  ircd-ratbox: A slightly useful ircd.
 *  commio-ssl.h: A header for the ssl code
 *
 *  Copyright (C) 2008-2012 ircd-ratbox development team
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

#ifndef _COMMIO_SSL_H
#define _COMMIO_SSL_H

int rb_init_ssl(void);
void rb_ssl_accept_setup(rb_fde_t *F, rb_fde_t *new_F, struct sockaddr *st, int addrlen);
void rb_ssl_shutdown(rb_fde_t *F);
ssize_t rb_ssl_read(rb_fde_t *F, void *buf, size_t count);
ssize_t rb_ssl_write(rb_fde_t *F, const void *buf, size_t count);
void rb_get_ssl_info(char *buf, size_t length);

#endif
