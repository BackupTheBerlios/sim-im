/* One way encryption based on MD5 sum.
   Copyright (C) 1996, 1997, 1999, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* warmenhoven took this file and made it work with the md5.[ch] we
 * already had. isn't that lovely. people should just use linux or
 * freebsd, crypt works properly on those systems. i hate solaris */

#include <QCryptographicHash>

#include "socket/socket.h"

/* Define our magic string to mark salt for MD5 "encryption"
   replacement.  This is meant to be the same as for other MD5 based
   encryption implementations.  */
const char md5_salt_prefix[] = "$1$";

/* Table with characters for base64 transformation.  */
const char b64t[] =
    "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

#define MIN(A, B)	((A < B) ? A : B)
#define MAX(A, B)	((A > B) ? A : B)

char *yahoo_crypt(const char *key, const char *salt)
{
    static char *buffer = NULL;
    static int buflen = 0;
    int needed = 3 + strlen (salt) + 1 + 26 + 1;

    size_t salt_len;
    size_t key_len;
    size_t cnt;
    char *cp;

    if (buflen < needed) {
        buflen = needed;
        if ((buffer = (char*)realloc(buffer, buflen)) == NULL) //warning C6308: 'realloc' might return null pointer: assigning null pointer to 'char * `char * __cdecl yahoo_crypt(char const *,char const *)'::`2'::buffer', which is passed as an argument to 'realloc', will cause the original memory block to be leaked
            return NULL;
    }

    /* Find beginning of salt string.  The prefix should normally always
       be present.  Just in case it is not.  */
    if (strncmp (md5_salt_prefix, salt, sizeof (md5_salt_prefix) - 1) == 0)
        /* Skip salt prefix.  */
        salt += sizeof (md5_salt_prefix) - 1;

    salt_len = MIN (strcspn (salt, "$"), 8);
    key_len = strlen (key);

    std::string ct = key;
    ct += md5_salt_prefix;
    ct += salt;

    std::string ct_alt = key;
    ct_alt += salt;
    ct_alt += key;
    ct_alt = (QCryptographicHash::hash(QByteArray(ct_alt.c_str(), ct_alt.length()), QCryptographicHash::Md5)).data();

    /* Add for any character in the key one byte of the alternate sum.  */
    for (cnt = key_len; cnt > 16; cnt -= 16)
        ct.append(ct_alt.c_str(), 16);
    ct.append(ct_alt.c_str(), cnt);

    char nil[] = "";

    /* The original implementation now does something weird: for every 1
       bit in the key the first 0 is added to the buffer, for every 0
       bit the first character of the key.  This does not seem to be
       what was intended but we have to follow this to be compatible.  */
    for (cnt = key_len; cnt > 0; cnt >>= 1)
        ct.append((cnt & 1) != 0 ? nil : key, 1);

    /* Create intermediate result.  */
    ct_alt = (QCryptographicHash::hash(QByteArray(ct.c_str(), ct.length()), QCryptographicHash::Md5)).data();

    /* Now comes another weirdness.  In fear of password crackers here
       comes a quite long loop which just processes the output of the
       previous round again.  We cannot ignore this here.  */
    for (cnt = 0; cnt < 1000; ++cnt) {
        /* New context.  */
        ct = "";

        /* Add key or last result.  */
        if ((cnt & 1) != 0)
            ct += key;
        else
            ct.append(ct_alt.c_str(), 16);

        /* Add salt for numbers not divisible by 3.  */
        if (cnt % 3 != 0)
            ct += salt;

        /* Add key for numbers not divisible by 7.  */
        if (cnt % 7 != 0)
            ct += key;

        /* Add key or last result.  */
        if ((cnt & 1) != 0)
            ct.append(ct_alt.c_str(), 16);
        else
            ct += key;

        /* Create intermediate result.  */
        ct_alt = (QCryptographicHash::hash(QByteArray(ct.c_str(), ct.length()), QCryptographicHash::Md5)).data();
    }

    /* Now we can construct the result string.  It consists of three
       parts.  */

    strncpy(buffer, md5_salt_prefix, MAX (0, buflen)); //warning C6387: 'argument 1' might be '0': this does not adhere to the specification for the function 'strncpy': Line: 43
    cp = buffer + strlen(buffer); //warning C6387: 'argument 1' might be '0': this does not adhere to the specification for the function 'strlen': Line: 43
								  //warning C6053: Call to 'strncpy' might not zero-terminate string 'buffer': Line: 43
    buflen -= sizeof (md5_salt_prefix);

    strncpy(cp, salt, MIN ((size_t) buflen, salt_len));
    cp = cp + strlen(cp);  //warning C6053: Call to 'strncpy' might not zero-terminate string 'cp': Line: 43
    buflen -= MIN ((size_t) buflen, salt_len);

    if (buflen > 0) {
        *cp++ = '$';
        --buflen;
    }

#define b64_from_24bit(B2, B1, B0, N) \
	{ \
		unsigned int w = ((B2) << 16) | ((B1) << 8) | (B0); \
		int n = (N); \
		while (n-- > 0 && buflen > 0) { \
			*cp++ = b64t[w & 0x3f]; \
			--buflen; \
			w >>= 6; \
		}\
	}

    b64_from_24bit (ct_alt[0], ct_alt[6], ct_alt[12], 4);
    b64_from_24bit (ct_alt[1], ct_alt[7], ct_alt[13], 4);
    b64_from_24bit (ct_alt[2], ct_alt[8], ct_alt[14], 4);
    b64_from_24bit (ct_alt[3], ct_alt[9], ct_alt[15], 4);
    b64_from_24bit (ct_alt[4], ct_alt[10], ct_alt[5], 4);
    b64_from_24bit (0, 0, ct_alt[11], 2);
    if (buflen <= 0) {
        free(buffer);
        buffer = NULL;
    } else
        *cp = '\0';	/* Terminate the string.  */

    return buffer;
}
