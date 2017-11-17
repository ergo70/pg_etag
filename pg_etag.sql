-- @(#) md5agg(text) aggregate function for PostgreSQL
-- $Header: /code/pg-mdagg/md5agg.sql,v 1.1.1.1 2009/07/08 07:29:49 doj Exp $

CREATE OR REPLACE FUNCTION etag_state(text, text)
RETURNS text
AS 'pg_etag', 'pg_etag_state'
LANGUAGE C STRICT;
COMMENT ON FUNCTION etag_state(text, text) IS 'State function for pg_etag(text) aggregate.';

CREATE OR REPLACE FUNCTION pg_etag_final(text)
RETURNS text
AS 'pg_etag', 'pg_etag_final'
LANGUAGE C STRICT;
COMMENT ON FUNCTION etag_final(text) IS 'Finalize function for pg_etag(text) aggregate.';

DROP AGGREGATE IF EXISTS etag_agg(text);
CREATE AGGREGATE etag_agg(text) (
  SFUNC = etag_state,
  STYPE = text,
  INITCOND = '',
  FINALFUNC = etag_final
);
COMMENT ON AGGREGATE etag_agg(text) IS 'ETag BLAKE2 aggregate function.';
