-- @(#) md5agg(text) aggregate function for PostgreSQL
-- $Header: /code/pg-mdagg/md5agg.sql,v 1.1.1.1 2009/07/08 07:29:49 doj Exp $

CREATE OR REPLACE FUNCTION md5agg_state(text, text)
RETURNS text
AS 'md5agg', 'md5agg_state'
LANGUAGE C STRICT;
COMMENT ON FUNCTION md5agg_state(text, text) IS 'State function for md5agg(text) aggregate.';

CREATE OR REPLACE FUNCTION md5agg_final(text)
RETURNS text
AS 'md5agg', 'md5agg_final'
LANGUAGE C STRICT;
COMMENT ON FUNCTION md5agg_final(text) IS 'Finalize function for md5agg(text) aggregate.';

DROP AGGREGATE IF EXISTS md5agg(text);
CREATE AGGREGATE md5agg(text) (
  SFUNC = md5agg_state,
  STYPE = text,
  INITCOND = '',
  FINALFUNC = md5agg_final
);
COMMENT ON AGGREGATE md5agg(text) IS 'MD5 aggregate function.';
