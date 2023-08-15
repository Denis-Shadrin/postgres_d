CREATE FUNCTION hi_message()
RETURNS VOID AS $$
BEGIN
RAISE NOTICE 'HI!';
END;
$$ LANGUAGE plpgsql;
--вывод сообщения
CREATE FUNCTION hi_message2 ()
RETURNS void
AS 'MODULE_PATHNAME'
LANGUAGE C;
--//возврат int * 2
CREATE FUNCTION get_int_x2 (int4)
RETURNS int4
AS 'MODULE_PATHNAME'
LANGUAGE C;
--//добовить в полученонму тексту !!!
CREATE FUNCTION get_text_aggresive (text)
RETURNS text
AS 'MODULE_PATHNAME'

LANGUAGE C;


CREATE TYPE test_cn;
--считывает числа из строки
CREATE OR REPLACE FUNCTION test_t_in( cstring )
RETURNS test_cn
AS 'MODULE_PATHNAME'
LANGUAGE C IMMUTABLE STRICT;
--получает композитынй тип и выводит его
CREATE OR REPLACE FUNCTION test_t_out( test_cn )
RETURNS cstring
AS 'MODULE_PATHNAME'
LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE test_cn
(
internallength = 8,
input = test_t_in,
output = test_t_out
);
--получает композитынй тип и возвращает новый с изменёнными данными
CREATE OR REPLACE FUNCTION test_t_multiply(test_cn)
RETURNS test_cn
AS 'MODULE_PATHNAME'
LANGUAGE C;

--получает из записи поле и возвращает true если меньше либо = limit
CREATE FUNCTION test_tb(test_table, integer) RETURNS boolean
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;
--возврат записи по типам аттрибутов указанной таблицы
CREATE FUNCTION pg_return_test() RETURNS test_table
AS 'MODULE_PATHNAME'
LANGUAGE C;
--заменяет текст в строке композитного типа и умножает число из композитного типа на 2 если what = true
CREATE FUNCTION pg_return_test2(test_table) RETURNS test_table
AS 'MODULE_PATHNAME'
LANGUAGE C;
--возврат новой таблицы с данными
CREATE FUNCTION pg_return_test3() RETURNS SETOF test_table
AS 'MODULE_PATHNAME'
LANGUAGE C;

--возврат перемноженного int
CREATE FUNCTION func_for_rtest (integer,integer)
RETURNS integer
AS 'MODULE_PATHNAME'
LANGUAGE C;

--запись и получение числа. shared memory 

--CREATE FUNCTION test_shmem_set (integer)
--RETURNS integer
--AS 'MODULE_PATHNAME'
--LANGUAGE C;
--
--CREATE FUNCTION test_shmem_get ()
--RETURNS integer
--AS 'MODULE_PATHNAME'
--LANGUAGE C;

--//запись и получение текста. shared memory 

--CREATE FUNCTION test_shmem_set_t (text)
--RETURNS text
--AS 'MODULE_PATHNAME'
--LANGUAGE C;
--
--CREATE FUNCTION test_shmem_get_t ()
--RETURNS text
--AS 'MODULE_PATHNAME'
--LANGUAGE C;

--тестовая работа с latch. метод синхронизации потоков
CREATE FUNCTION latch_test1 (text)
RETURNS void
AS 'MODULE_PATHNAME'
LANGUAGE C;

CREATE FUNCTION latch_test2 ()
RETURNS void
AS 'MODULE_PATHNAME'
LANGUAGE C;