-- Copyright (c) 2009, Dirk Jagdmann <doj@cubic.org>
-- All rights reserved.
-- Redistribution and use in source and binary forms, with or
-- without modification, are permitted provided that the following
-- conditions are met:
--  * Redistributions of source code must retain the above copyright
--    notice, this list of conditions and the following disclaimer.
--  * Redistributions in binary form must reproduce the above
--    copyright notice, this list of conditions and the following
--    disclaimer in the documentation and/or other materials
--    provided with the distribution.
--  * Neither the name of Cubic nor the names of its
--    contributors may be used to endorse or promote products
--    derived from this software without specific prior written
--    permission.
-- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
-- "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
-- LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
-- A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
-- HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
-- SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
-- LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
-- DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
-- THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
-- (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
-- OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

-- $Header: /code/pg-mdagg/md5-oid.sql,v 1.1 2009/07/08 08:00:22 doj Exp $

/** FUNCTION md5(id oid)

Calculates the MD5 sum of a large object. If no large object is found
the MD5 sum of the string value of id is calculated.

@param id OID of large object

@return MD5 sum

@author Dirk Jagdmann <doj@cubic.org> with ideas from the pgsql maillist

*/
CREATE OR REPLACE FUNCTION public.md5(id oid)
RETURNS text
AS $$
 DECLARE
  fd        integer;
  size      integer;
  c         int8;
  hashval   text;
  INV_READ  constant integer := x'40000'::integer; -- from libpq-fs.h
  SEEK_SET  constant integer := 0;
  SEEK_END  constant integer := 2;
 BEGIN

  SELECT loid INTO c FROM pg_catalog.pg_largeobject WHERE loid=id limit 1;
  IF NOT FOUND THEN
    RAISE NOTICE 'no large object % found, returning md5 of string', id;
    RETURN md5(id::text);
  END IF;

  fd   := lo_open(id, INV_READ);
  size := lo_lseek(fd, 0, SEEK_END);
  PERFORM lo_lseek(fd, 0, SEEK_SET);
  hashval := md5(loread(fd, size));
  PERFORM lo_close(fd);
  RETURN hashval;
 END;
$$
LANGUAGE plpgsql stable strict;
