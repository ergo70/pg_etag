-- @(#) test cases for MD5 aggregate function
-- $Header: /code/pg-mdagg/md5agg-test.sql,v 1.1.1.1 2009/07/08 07:29:49 doj Exp $

CREATE TEMPORARY TABLE md5agg_test(t text);
INSERT INTO md5agg_test VALUES ('message'), (' '), ('digest');
-- this test should be TRUE
SELECT (SELECT md5agg(t) from md5agg_test) = 'f96b697d7cb7938d525a2f31aaf161d0' as "compare must be TRUE";

CREATE TEMPORARY TABLE md5agg_a(i int, t text);
INSERT INTO md5agg_a VALUES
(0,'a'),
(1,'b'),
(2,'c'),
(3,'d'),
(4,'e'),
(5,'f'),
(6,'g'),
(7,'h'),
(8,'i'),
(9,'j');
CREATE TEMPORARY TABLE md5agg_b AS
SELECT i,t from md5agg_a;
UPDATE md5agg_b set t='D' where i=3;

-- this test should be TRUE
SELECT (select md5agg(i) from (select i::text from md5agg_a order by i) as tab)
       =
       (select md5agg(i) from (select i::text from md5agg_b order by i) as tab)
       as "compare col i must be TRUE";

-- this test should be FALSE
SELECT (select md5agg(t) from (select t from md5agg_a order by i) as tab)
       =
       (select md5agg(t) from (select t from md5agg_b order by i) as tab)
       as "compare col t must be FALSE";
