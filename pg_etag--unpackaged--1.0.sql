CREATE OR REPLACE FUNCTION pg_etag_state(text, text)
RETURNS text
AS 'pg_etag', 'pg_etag_state'
LANGUAGE C STRICT;
COMMENT ON FUNCTION etag_state(text, text) IS 'State function for etag_agg(text) aggregate.';

CREATE OR REPLACE FUNCTION pg_etag_final(text)
RETURNS text
AS 'pg_etag', 'pg_etag_final'
LANGUAGE C STRICT;
COMMENT ON FUNCTION etag_final(text) IS 'Finalize function for etag_agg(text) aggregate.';

DROP AGGREGATE IF EXISTS etag_agg(text);
CREATE AGGREGATE etag_agg(text) (
  SFUNC = pg_etag_state,
  STYPE = text,
  INITCOND = '',
  FINALFUNC = pg_etag_final
);
COMMENT ON AGGREGATE etag_agg(text) IS 'ETag BLAKE2 aggregate function.';

CREATE OR REPLACE FUNCTION etag(text)
RETURNS text
AS 'pg_etag', 'pg_etag_single'
LANGUAGE C STRICT;
COMMENT ON FUNCTION etag(text) IS 'Non aggregate (single input) ETag BLAKE2 function.';

CREATE OR REPLACE FUNCTION public.etag(id oid)
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
    RAISE EXCEPTION 'No large object with id=% found!', id;
  END IF;

  fd   := lo_open(id, INV_READ);
  size := lo_lseek(fd, 0, SEEK_END);
  PERFORM lo_lseek(fd, 0, SEEK_SET);
  hashval := etag(loread(fd, size));
  PERFORM lo_close(fd);
  RETURN hashval;
 END;
$$
LANGUAGE plpgsql stable strict;

