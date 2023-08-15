CREATE TABLE test_table (
num1 int,
str1 text,
what bool
);
CREATE EXTENSION hi;

SELECT get_int_x2(2) as gix2;
SELECT func_for_rTest(2,4) as ffrt;