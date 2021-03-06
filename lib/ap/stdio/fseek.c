/*
 * Copyright (c) 2005-2014 Rich Felker, et al.
 * Copyright (c) 2015-2016 Álvaro Jurado et al.
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE.mit file.
 */

#include "stdio_impl.h"

int __fseeko_unlocked(FILE *f, off_t off, int whence)
{
	/* Adjust relative offset for unread data in buffer, if any. */
	if (whence == SEEK_CUR) off -= f->rend - f->rpos;

	/* Flush write buffer, and report error on failure. */
	if (f->wpos > f->wbase) {
		f->write(f, 0, 0);
		if (!f->wpos) return -1;
	}

	/* Leave writing mode */
	f->wpos = f->wbase = f->wend = 0;

	/* Perform the underlying seek. */
	if (f->seek(f, off, whence) < 0) return -1;

	/* If seek succeeded, file is seekable and we discard read buffer. */
	f->rpos = f->rend = 0;
	f->flags &= ~F_EOF;

	return 0;
}

int fseeko(FILE *f, off_t off, int whence)
{
	int result;
	FLOCK(f);
	result = __fseeko_unlocked(f, off, whence);
	FUNLOCK(f);
	return result;
}

int fseek(FILE *f, long off, int whence)
{
	return fseeko(f, off, whence);
}
