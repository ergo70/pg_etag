# pg_etag

Fast ETag generation for rows and result sets with BLAKE2 hashes.

## Example

### Fast client ([SQL Tabs](https://www.sqltabs.com/))

```sql
CREATE TABLE omr AS SELECT t.t AS id, clock_timestamp() AS ts, NULL::TEXT AS etag FROM generate_series(1,1000000) t

UPDATE omr SET etag=etag(id::TEXT || ts::TEXT)

ALTER TABLE omr
  ADD CONSTRAINT pk_omr PRIMARY KEY (id);
```

Query:

```sql
SELECT * FROM omr WHERE id BETWEEN 500 AND 1700 ORDER BY id ASC;
```

~ 5 ms

ETag of query result:

```sql
SELECT etag_agg(t.etag) FROM (SELECT etag FROM omr WHERE id BETWEEN 500 AND 1700 ORDER BY id ASC) t;
```

~ 1.5 ms

### Slow client ([pgAdmin3](https://www.pgadmin.org/download/))

```sql
CREATE TABLE omr AS SELECT t.t AS id, clock_timestamp() AS ts, NULL::TEXT AS etag FROM generate_series(1,1000000) t

UPDATE omr SET etag=etag(id::TEXT || ts::TEXT)

ALTER TABLE omr
  ADD CONSTRAINT pk_omr PRIMARY KEY (id);
```

Query:

```sql
SELECT * FROM omr WHERE id BETWEEN 500 AND 1700 ORDER BY id ASC;
```

~ 145 ms

ETag of query result:

```sql
SELECT etag_agg(t.etag) FROM (SELECT etag FROM omr WHERE id BETWEEN 500 AND 1700 ORDER BY id ASC) t;
```

~ 11 ms
