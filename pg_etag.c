/* Copyright (c) 2017, Ernst-Georg Schmid */
/* */
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
/* */
/* Original idea and the binary2hex function: Copyright (c) 2009..2012, Dirk Jagdmann <doj@cubic.org> */
/* md5agg - PostgreSQL md5(text) aggregate function http://llg.cubic.org/pg-mdagg/ */


#include "postgres.h"
#include <string.h>
#include "fmgr.h"
#include "utils/elog.h"
#include "blake2.h"

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

#define BLAKE2B_DIGEST_LENGTH 64*sizeof(char)
#define ctx (blake2b_state*) VARDATA(state)

static inline char *binary2hex(unsigned char *data, unsigned len, char *buf);

//lint -esym(818,data) -esym(952,len) ignore non const parameters
static inline char *binary2hex(unsigned char *data, unsigned len, char *buf)
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
        case 0:
            *s++='0';
            break;
        case 1:
            *s++='1';
            break;
        case 2:
            *s++='2';
            break;
        case 3:
            *s++='3';
            break;
        case 4:
            *s++='4';
            break;
        case 5:
            *s++='5';
            break;
        case 6:
            *s++='6';
            break;
        case 7:
            *s++='7';
            break;
        case 8:
            *s++='8';
            break;
        case 9:
            *s++='9';
            break;
        case 10:
            *s++='a';
            break;
        case 11:
            *s++='b';
            break;
        case 12:
            *s++='c';
            break;
        case 13:
            *s++='d';
            break;
        case 14:
            *s++='e';
            break;
        case 15:
            *s++='f';
            break;
        default:
            *s++='!';
            break;
        }
        switch(data[i] & 15)
        {
        case 0:
            *s++='0';
            break;
        case 1:
            *s++='1';
            break;
        case 2:
            *s++='2';
            break;
        case 3:
            *s++='3';
            break;
        case 4:
            *s++='4';
            break;
        case 5:
            *s++='5';
            break;
        case 6:
            *s++='6';
            break;
        case 7:
            *s++='7';
            break;
        case 8:
            *s++='8';
            break;
        case 9:
            *s++='9';
            break;
        case 10:
            *s++='a';
            break;
        case 11:
            *s++='b';
            break;
        case 12:
            *s++='c';
            break;
        case 13:
            *s++='d';
            break;
        case 14:
            *s++='e';
            break;
        case 15:
            *s++='f';
            break;
        default:
            *s++='?';
            break;
        }
    }
    *s=0;

    return buf;
}

static inline bytea *etag_state(bytea *state, const int state_len, const unsigned char *buf, const int buf_len)
{
    /* if this is the first invocation of the state function, initialize the BLAKE2 context */
    if(state_len == 0)
    {
        const int state_size=VARHDRSZ + sizeof(blake2b_state);
        state = palloc(state_size);
        SET_VARSIZE(state, state_size);
        blake2b_init(ctx, BLAKE2B_DIGEST_LENGTH);
    }
    /* check the state parameter for required length */
    else if(state_len != sizeof(blake2b_state))
    {
        /* abort the function/transaction as the length is not of MD5 context */
        ereport(ERROR,
                (errcode(ERRCODE_DATA_CORRUPTED),
                 errmsg("pg_etag_state(): size mismatch: state=%i ctx=%i", state_len, (int)sizeof(blake2b_state))
                ));
        return NULL;
    }

    /* is there txt to update? */
    if(buf_len > 0)
        blake2b_update(ctx, buf, buf_len);

    return state;
}

static inline text *etag_single(const unsigned char *buf, const int buf_len, int blake2_digest_len)
{

    blake2b_state state[1];
    unsigned char digest[blake2_digest_len];
    const int ret_size=VARHDRSZ + blake2_digest_len*2;
    text *ret = NULL;

    if (blake2_digest_len < 1)
        blake2_digest_len = 1;

    if (blake2_digest_len > 64)
        blake2_digest_len = 64;

    ret = palloc(ret_size + 1); /* C-string terminator from binary2hex() */

    blake2b_init(state, blake2_digest_len);
    blake2b_update(state, (const unsigned char*) buf, buf_len);
    blake2b_final(state, (void*) &digest[0], blake2_digest_len);

    /* convert binary digest to hex characters */
    SET_VARSIZE(ret, ret_size);
    binary2hex(digest, sizeof(digest), VARDATA(ret));

    return ret;
}

PG_FUNCTION_INFO_V1(pg_etag_state);
Datum pg_etag_state(PG_FUNCTION_ARGS)
{
    bytea *retval = NULL;

    bytea *state = PG_GETARG_BYTEA_P(0);
    const int state_len=VARSIZE(state) - VARHDRSZ;

    text *txt = PG_GETARG_TEXT_P(1);
    const int txt_len=VARSIZE(txt) - VARHDRSZ;

    retval = etag_state(state, state_len, (const unsigned char*) VARDATA(txt), txt_len);

    if (retval == NULL) PG_RETURN_NULL();

    PG_RETURN_BYTEA_P(retval);
}

PG_FUNCTION_INFO_V1(pg_etag_state_b);
Datum pg_etag_state_b(PG_FUNCTION_ARGS)
{
    bytea *retval = NULL;

    bytea *state = PG_GETARG_BYTEA_P(0);
    const int state_len=VARSIZE(state) - VARHDRSZ;

    bytea *data = PG_GETARG_BYTEA_P(1);
    const int data_len=VARSIZE(data) - VARHDRSZ;

    retval = etag_state(state, state_len, (const unsigned char*) VARDATA(data), data_len);

    if (retval == NULL) PG_RETURN_NULL();

    PG_RETURN_BYTEA_P(retval);
}

PG_FUNCTION_INFO_V1(pg_etag_final);
Datum pg_etag_final(PG_FUNCTION_ARGS)
{
    bytea *state = PG_GETARG_BYTEA_P(0);
    const int state_len=VARSIZE(state) - VARHDRSZ;

    const int ret_size=VARHDRSZ + BLAKE2B_DIGEST_LENGTH*2;
    text *ret = palloc(ret_size + 1); /* C-string terminator from binary2hex() */

    /* check if state parameter has required length */
    if(state_len != sizeof(blake2b_state))
    {
        PG_RETURN_NULL();
    }
    else
    {
        /* calculate digest */
        unsigned char digest[BLAKE2B_DIGEST_LENGTH];
        blake2b_final(ctx, (void*) &digest[0], BLAKE2B_DIGEST_LENGTH);
        /* convert binary digest to hex characters */
        SET_VARSIZE(ret, ret_size);
        binary2hex(digest, sizeof(digest), VARDATA(ret));
    }

    PG_RETURN_TEXT_P(ret);
}

PG_FUNCTION_INFO_V1(pg_etag_single);
Datum pg_etag_single(PG_FUNCTION_ARGS)
{
    text *txt = PG_GETARG_TEXT_P(0);
    const int txt_len=VARSIZE(txt) - VARHDRSZ;

    PG_RETURN_TEXT_P(etag_single((const unsigned char*) VARDATA(txt), txt_len, BLAKE2B_DIGEST_LENGTH));
}

PG_FUNCTION_INFO_V1(pg_etag_single_b);
Datum pg_etag_single_b(PG_FUNCTION_ARGS)
{
    bytea *data = PG_GETARG_BYTEA_P(0);
    const int data_len=VARSIZE(data) - VARHDRSZ;

    PG_RETURN_TEXT_P(etag_single((const unsigned char*) VARDATA(data), data_len, BLAKE2B_DIGEST_LENGTH));
}

PG_FUNCTION_INFO_V1(pg_blake2_single);
Datum pg_blake2_single(PG_FUNCTION_ARGS)
{
    text *txt = PG_GETARG_TEXT_P(0);
    const int txt_len=VARSIZE(txt) - VARHDRSZ;
    int32 blake2_digest_len = PG_GETARG_INT32(1);

    if (blake2_digest_len < 1)
        blake2_digest_len = 1;

    if (blake2_digest_len > 64)
        blake2_digest_len = 64;

    PG_RETURN_TEXT_P(etag_single((const unsigned char*) VARDATA(txt), txt_len, blake2_digest_len));
}

PG_FUNCTION_INFO_V1(pg_blake2_single_b);
Datum pg_blake2_single_b(PG_FUNCTION_ARGS)
{
    bytea *data = PG_GETARG_BYTEA_P(0);
    const int data_len=VARSIZE(data) - VARHDRSZ;
    int32 blake2_digest_len = PG_GETARG_INT32(1);

    if (blake2_digest_len < 1)
        blake2_digest_len = 1;

    if (blake2_digest_len > 64)
        blake2_digest_len = 64;

    PG_RETURN_TEXT_P(etag_single((const unsigned char*) VARDATA(data), data_len, blake2_digest_len));
}
