/* Copyright (c) 2009..2012, Dirk Jagdmann <doj@cubic.org> */
/* All rights reserved. */
/* Redistribution and use in source and binary forms, with or */
/* without modification, are permitted provided that the following */
/* conditions are met: */
/*  * Redistributions of source code must retain the above copyright */
/*    notice, this list of conditions and the following disclaimer. */
/*  * Redistributions in binary form must reproduce the above */
/*    copyright notice, this list of conditions and the following */
/*    disclaimer in the documentation and/or other materials */
/*    provided with the distribution. */
/*  * Neither the name of Cubic nor the names of its */
/*    contributors may be used to endorse or promote products */
/*    derived from this software without specific prior written */
/*    permission. */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS */
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT */
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR */
/* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT */
/* HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT */
/* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY */
/* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT */
/* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE */
/* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

/* $Header: /code/pg-mdagg/md5agg.c,v 1.4 2012/09/29 07:47:59 doj Exp $ */

/* PostgreSQL md5(text) aggregate function */

#include "postgres.h"
#include <string.h>
#include "fmgr.h"
#include "utils/elog.h"
PG_MODULE_MAGIC;

#if defined(__linux__) || defined(__APPLE__)
#include "md5.c"
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__sun__)
#include <sys/types.h>
#include <md5.h>
#else
#error unknown operating system!
#endif

//lint -esym(818,data) -esym(952,len) ignore non const parameters
char *binary2hex(unsigned char *data, unsigned len, char *buf)
{
  unsigned i;
  char *s=buf;

  if(!data || !len) return NULL;

  if(!buf)
    {
      buf=(char*)malloc(len*2 + 1);
      if(!buf) return NULL;
    }

  for(i=0; i!=len; ++i)
    {
      switch(data[i] >> 4)
	{
	case 0: *s++='0'; break;
	case 1: *s++='1'; break;
	case 2: *s++='2'; break;
	case 3: *s++='3'; break;
	case 4: *s++='4'; break;
	case 5: *s++='5'; break;
	case 6: *s++='6'; break;
	case 7: *s++='7'; break;
	case 8: *s++='8'; break;
	case 9: *s++='9'; break;
	case 10: *s++='a'; break;
	case 11: *s++='b'; break;
	case 12: *s++='c'; break;
	case 13: *s++='d'; break;
	case 14: *s++='e'; break;
	case 15: *s++='f'; break;
	default: *s++='!'; break;
	}
      switch(data[i] & 15)
	{
	case 0: *s++='0'; break;
	case 1: *s++='1'; break;
	case 2: *s++='2'; break;
	case 3: *s++='3'; break;
	case 4: *s++='4'; break;
	case 5: *s++='5'; break;
	case 6: *s++='6'; break;
	case 7: *s++='7'; break;
	case 8: *s++='8'; break;
	case 9: *s++='9'; break;
	case 10: *s++='a'; break;
	case 11: *s++='b'; break;
	case 12: *s++='c'; break;
	case 13: *s++='d'; break;
	case 14: *s++='e'; break;
	case 15: *s++='f'; break;
	default: *s++='?'; break;
	}
    }
  *s=0;

  return buf;
}

#define ctx (MD5_CTX*) VARDATA(state)

PG_FUNCTION_INFO_V1(md5agg_state);
Datum md5agg_state(PG_FUNCTION_ARGS)
{
  text *state = PG_GETARG_TEXT_P/*_COPY*/(0); /* do we really need the _COPY here? As this is an aggregate function it will not receive a direct TOAST pointer */
  const int state_len=VARSIZE(state) - VARHDRSZ;

  text *txt = PG_GETARG_TEXT_P(1);
  const int txt_len=VARSIZE(txt) - VARHDRSZ;

#if 0
  /* some debug output of the parameters */
  char a[128],b[128];
  memset(a, 0, sizeof(a));
  memset(b, 0, sizeof(b));
  memcpy(a, VARDATA(state), state_len);
  memcpy(b, VARDATA(txt), txt_len);
  ereport(LOG,
	  (errcode(ERRCODE_WARNING),
	   errmsg("md5agg_state(): state=%i '%s' txt=%i '%s'", state_len, a, txt_len, b)
	   ));
#endif

  /* if this is the first invocation of the state function, initialize the MD5 context */
  if(state_len == 0)
    {
      const int state_size=VARHDRSZ + sizeof(MD5_CTX);
      state = palloc(state_size);
      SET_VARSIZE(state, state_size);
      MD5Init(ctx);
    }
  /* check the state parameter for required length */
  else if(state_len != sizeof(MD5_CTX))
    {
      /* abort the function/transaction as the length is not of MD5 context */
      ereport(ERROR,
	      (errcode(ERRCODE_DATA_CORRUPTED),
	       errmsg("md5agg_state(): size mismatch: state=%i ctx=%i", state_len, (int)sizeof(MD5_CTX))
	       ));
      PG_RETURN_NULL();
    }

  /* is there txt to update? */
  if(txt_len > 0)
    MD5Update(ctx, VARDATA(txt), txt_len);

  PG_RETURN_TEXT_P(state);
}

PG_FUNCTION_INFO_V1(md5agg_final);
Datum md5agg_final(PG_FUNCTION_ARGS)
{
  text *state = PG_GETARG_TEXT_P(0);
  const int state_len=VARSIZE(state) - VARHDRSZ;

  const int ret_size=VARHDRSZ + MD5_DIGEST_LENGTH*2;
  text *ret = palloc(ret_size + 1); /* C-string terminator from binary2hex() */

  /* check if state parameter has required length */
  if(state_len != sizeof(MD5_CTX))
    {
      PG_RETURN_NULL();
    }
  else
    {
      /* calculate digest */
      unsigned char digest[MD5_DIGEST_LENGTH];
      MD5Final(digest, ctx);
      /* convert binary digest to hex characters */
      SET_VARSIZE(ret, ret_size);
      binary2hex(digest, sizeof(digest), VARDATA(ret));
    }

  PG_RETURN_TEXT_P(ret);
}
