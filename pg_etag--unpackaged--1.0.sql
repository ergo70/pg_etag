CREATE OR REPLACE FUNCTION pg_etag_state(BYTEA, text)
RETURNS BYTEA
AS 'pg_etag', 'pg_etag_state'
LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION pg_etag_state(BYTEA, text) IS 'State function for etag_agg(text) aggregate.';

CREATE OR REPLACE FUNCTION pg_etag_state(BYTEA, BYTEA)
RETURNS BYTEA
AS 'pg_etag', 'pg_etag_state_b'
LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION pg_etag_state(BYTEA, BYTEA) IS 'State function for etag_agg(text) aggregate.';

CREATE OR REPLACE FUNCTION pg_etag_final(BYTEA)
RETURNS text
AS 'pg_etag', 'pg_etag_final'
LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION pg_etag_final(text) IS 'Finalize function for etag_agg(text) aggregate.';

DROP AGGREGATE IF EXISTS etag_agg(text);
CREATE AGGREGATE etag_agg(text) (
  SFUNC = pg_etag_state,
  STYPE = BYTEA,
  INITCOND = '',
  FINALFUNC = pg_etag_final
);
COMMENT ON AGGREGATE etag_agg(text) IS 'ETag BLAKE2 aggregate function.';

DROP AGGREGATE IF EXISTS etag_agg(BYTEA);
CREATE AGGREGATE etag_agg(BYTEA) (
  SFUNC = pg_etag_state,
  STYPE = BYTEA,
  INITCOND = '',
  FINALFUNC = pg_etag_final
);
COMMENT ON AGGREGATE etag_agg(BYTEA) IS 'ETag BLAKE2 aggregate function.';

CREATE OR REPLACE FUNCTION etag(text)
RETURNS text
AS 'pg_etag', 'pg_etag_single'
LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION etag(text) IS 'Non aggregate (single input) ETag BLAKE2 function.';

CREATE OR REPLACE FUNCTION etag(BYTEA)
RETURNS text
AS 'pg_etag', 'pg_etag_single_b'
LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION etag(BYTEA) IS 'Non aggregate (single input) ETag BLAKE2 function.';

CREATE OR REPLACE FUNCTION blake2(text, integer default 64)
RETURNS text
AS 'pg_etag', 'pg_blake2_single'
LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION blake2(text, integer) IS 'Non aggregate (single input) BLAKE2 function.';

CREATE OR REPLACE FUNCTION blake2(BYTEA, integer default 64)
RETURNS text
AS 'pg_etag', 'pg_blake2_single_b'
LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION blake2(BYTEA, integer) IS 'Non aggregate (single input) BLAKE2 function.';

CREATE OR REPLACE FUNCTION public.etag(id oid)
RETURNS text
AS $$
SELECT etag_agg("data") FROM (SELECT l."data" from pg_largeobject l WHERE l.loid=id ORDER BY l.pageno ASC) t; 
$$
LANGUAGE sql VOLATILE strict;
