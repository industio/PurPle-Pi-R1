/* -*- linux-c -*-
 * Copyright (C) 2005-2016 Red Hat Inc.
 *
 * This file is part of systemtap, and is free software.  You can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License (GPL); either version 2, or (at your option) any
 * later version.
 */
#ifndef _STP_STRING_H_
#define _STP_STRING_H_

#define to_oct_digit(c) ((c) + '0')
#define to_hex_digit(c) ({ char __c = (c); __c += (__c < 10 ? '0' : 'A' - 10); __c; })

static int _stp_text_str(char *out, const char *in, int inlen, int outlen, int quoted, int user, int buffer);

#endif /* _STP_STRING_H_ */
