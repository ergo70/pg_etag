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
