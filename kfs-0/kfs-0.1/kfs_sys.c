/* kfs_sys.c
 *
 * Copyright (c) 1993 by William W. Dorsey
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 1, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "kfs_int.h"


typedef struct {
	char	*path;
	int	cmd;
	caddr_t	data;
} altioctl_arg;


void
vno_altioctl(altioctl_arg *uap)
{
struct vnode	*vp;

	if( kfs_trace )
		printf("vno_altcntl: path $%x, cmd $%x, data $%x\n", (u_int) uap->path, uap->cmd, (u_int) uap->data);

	if( (u.u_error = lookupname(uap->path, UIO_USERSPACE, FOLLOW_LINK, 0, &vp)) != 0 )
		return;
	u.u_error = VOP_IOCTL(vp, uap->cmd, uap->data, 0, u.u_cred);
	VN_RELE(vp);
}
