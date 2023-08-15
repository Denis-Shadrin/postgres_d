#include <stdio.h>
#include "postgres.h"
#include "executor/executor.h"
#include "fmgr.h"
#include "postgres.h"
#include "utils/builtins.h"
#include "funcapi.h"
#include "utils/memutils.h"

#include "storage/lwlock.h"
#include "storage/shmem.h"
#include "miscadmin.h"

#include "common/logging.h"

#include <unistd.h>
#include "storage/latch.h"

#include "utils/wait_event.h"


PG_MODULE_MAGIC;
//вывод сообщения
PG_FUNCTION_INFO_V1(hi_message2);

Datum hi_message2 (PG_FUNCTION_ARGS)
{
    ereport(NOTICE, errmsg("Hi"));
    PG_RETURN_NULL();
}

PG_FUNCTION_INFO_V1(get_int_x2);
//возврат int * 2
Datum get_int_x2 (PG_FUNCTION_ARGS)
{
    PG_RETURN_INT32(PG_GETARG_INT32(0) * 2);
}

	PG_FUNCTION_INFO_V1(get_text_aggresive);
//добовить в полученонму тексту !!!
Datum get_text_aggresive (PG_FUNCTION_ARGS)
{
	char * input_tex;
	char * buf;

	
	input_tex= text_to_cstring(PG_GETARG_TEXT_P_COPY(0));
	
	ereport(NOTICE, errmsg("%ld", strlen(input_tex)));
	
	buf =(char *) palloc(strlen(input_tex)+4);//4 это место под 3 новых символа и терминирующий ноль

	buf = psprintf("%s!!!",input_tex);

    PG_RETURN_TEXT_P(cstring_to_text(buf));
}
// композитынй тип
typedef struct
{
int x2, x3;
} test_cn;

PG_FUNCTION_INFO_V1(test_t_in);
//считывает числа из строки
Datum test_t_in(PG_FUNCTION_ARGS)
{
	char *s = PG_GETARG_CSTRING(0);
	
	test_cn *a = (test_cn*)palloc0(sizeof(test_cn));
	
	sscanf(s, "(%d,%d)", &(a->x2), &(a->x3));
	//
	// кажется, лучше использовать strtok (src, '(,)') и потом atoi чтобы перегнать строчку в число
	
	PG_RETURN_POINTER(a);
}


PG_FUNCTION_INFO_V1(test_t_out);
//получает композитынй тип и выводит его
Datum test_t_out(PG_FUNCTION_ARGS)
{
test_cn *inputed_test_cn ;
char *s;
inputed_test_cn = (test_cn*)PG_GETARG_POINTER(0);

 s = (char*)palloc0(100);

snprintf(s, 100, "(%d,%d)", inputed_test_cn ->x2, inputed_test_cn ->x3);
//psprintf("format", params, ...) - возвращаемое значение - динамечески выделенная строчка
PG_RETURN_CSTRING(s);
}

PG_FUNCTION_INFO_V1(test_t_multiply);
//получает композитынй тип и возвращает новый с изменёнными данными
Datum test_t_multiply(PG_FUNCTION_ARGS)
{
test_cn *a = (test_cn*)PG_GETARG_POINTER(0);

test_cn *b = (test_cn*)palloc0(sizeof(test_cn));

b->x2 = a->x2 * a->x2;
b->x3 = a->x3 *a->x3 * a->x3;

PG_RETURN_POINTER(b);
} 


PG_FUNCTION_INFO_V1(test_tb);
//получает из записи поле и возвращает true если меньше либо = limit
Datum
test_tb(PG_FUNCTION_ARGS)
{
	HeapTupleHeader  t = PG_GETARG_HEAPTUPLEHEADER(0);
    int32            limit = PG_GETARG_INT32(1);
    bool isnull;
    Datum num1;

    num1 = GetAttributeByName(t, "num1", &isnull);
    if (isnull)
        PG_RETURN_BOOL(false);

    PG_RETURN_BOOL(DatumGetInt32(num1) <= limit);
}

PG_FUNCTION_INFO_V1(pg_return_test);
//возврат записи по типам аттрибутов указанной таблицы
Datum
pg_return_test(PG_FUNCTION_ARGS)
{
	TypeFuncClass		funcResClass;
	TupleDesc 			resTupDesc;	
	char *values[3];
	HeapTuple h;
	
	funcResClass = get_call_result_type(fcinfo, NULL, &resTupDesc);	
	
	if (funcResClass != TYPEFUNC_COMPOSITE)
		elog(ERROR, "Function returning record called in context that cannot accept type record");

	values[0] = psprintf("%d", 65);
	values[1] = psprintf("test");
	values[2] = psprintf("%d", false);

	h = BuildTupleFromCStrings(TupleDescGetAttInMetadata(resTupDesc), values);

return HeapTupleGetDatum(h);

}

PG_FUNCTION_INFO_V1(pg_return_test2);
//заменяет текст в строке композитного типа и умножает число из композитного типа на 2 если what = true
Datum
pg_return_test2(PG_FUNCTION_ARGS)
{
	HeapTupleHeader  t;
    bool isnull,isnull2,what;
	int32 num1;
	HeapTuple h;
	TypeFuncClass		funcResClass;
	TupleDesc 			resTupDesc;	
	char *values[3];
	
    t = PG_GETARG_HEAPTUPLEHEADER(0);
	num1 = GetAttributeByName(t, "num1", &isnull2);
	if (isnull2)
        num1=0;
	what = GetAttributeByName(t, "what", &isnull);
	if (isnull)
        what=false;
	else if (what){
		num1*=2;
	}

	funcResClass = get_call_result_type(fcinfo, NULL, &resTupDesc);	
	
	if (funcResClass != TYPEFUNC_COMPOSITE)
		elog(ERROR, "Function returning record called in context that cannot accept type record");

	values[0] = psprintf("%d", num1);
    values[1] = psprintf("work");
	values[2] = psprintf("%d", what);

	h = BuildTupleFromCStrings(TupleDescGetAttInMetadata(resTupDesc), values);

return HeapTupleGetDatum(h);

}


PG_FUNCTION_INFO_V1(pg_return_test3);
//возврат новой таблицы с данными
Datum pg_return_test3(PG_FUNCTION_ARGS) // strct_array - однонаправленный список, где указатель на следующий элемент лежит в next
{
	ReturnSetInfo 	*rsinfo;
	Datum		values[3];
	bool		nulls[2];
	
	InitMaterializedSRF(fcinfo, 0);
	rsinfo = (ReturnSetInfo *) fcinfo->resultinfo;
	memset(nulls, 0, sizeof(nulls));
	
	for (int i=0;i<10;i++){
		
		values[0] = Int32GetDatum(i);
		values[1] = CStringGetTextDatum("test");
		values[2] = BoolGetDatum(true);

		tuplestore_putvalues(rsinfo->setResult, rsinfo->setDesc, values, nulls);
	}
	
	return (Datum) 0;
}

PG_FUNCTION_INFO_V1(func_for_rtest);
//возврат перемноженного int
Datum func_for_rtest (PG_FUNCTION_ARGS)
{
    PG_RETURN_INT32(PG_GETARG_INT32(0) * PG_GETARG_INT32(1));
}

//запись и получение числа. shared memory 
/*
typedef struct shared_num
{
	int num;
} shared_num;

static shmem_request_hook_type prev_shmem_request_hook = NULL;
static void requestSharedMemory(void);
static shared_num *sn = NULL;
void _PG_init(void);

void
_PG_init(void)
{
	prev_shmem_request_hook = shmem_request_hook;
	shmem_request_hook = requestSharedMemory;

}

static void
requestSharedMemory(void)
{
if(prev_shmem_request_hook)
prev_shmem_request_hook();

	RequestAddinShmemSpace(sizeof(sn));
RequestNamedLWLockTranche("shared_num", 1);
}

static void startupSharedMemory(void) {
	bool found;
	
	LWLockAcquire(AddinShmemInitLock, LW_EXCLUSIVE);
	sn = ShmemInitStruct("shared_num", sizeof(sn), &found);
	
	

	if (!found) 
	{
       sn->num = 0;
	}
	LWLockRelease(AddinShmemInitLock);
}

PG_FUNCTION_INFO_V1(test_shmem_set);
Datum test_shmem_set (PG_FUNCTION_ARGS)
{	
	 if (sn == NULL) {
		 startupSharedMemory();
    }
    
	sn->num = PG_GETARG_INT32(0);
    PG_RETURN_INT32(sn->num);
}


PG_FUNCTION_INFO_V1(test_shmem_get);
Datum test_shmem_get (PG_FUNCTION_ARGS)
{
	if (sn == NULL) {
        startupSharedMemory();
    }
    
    PG_RETURN_INT32(sn->num);
}
*/
///////////////////////////////////////////////////////////////////

//запись и получение текста. shared memory 
/*
typedef struct shared_text
{
	char * text;
} shared_text;

static shmem_request_hook_type prev_shmem_request_hook2 = NULL;
static void requestSharedMemory(void);

static shared_text *st = NULL;

void _PG_init(void);

void
_PG_init(void)
{
	prev_shmem_request_hook2 = shmem_request_hook;
	shmem_request_hook = requestSharedMemory;
}

static void
requestSharedMemory(void)
{
	if(prev_shmem_request_hook2)
	prev_shmem_request_hook2();

	RequestAddinShmemSpace(sizeof(st)+4);
	RequestNamedLWLockTranche("shared_text", 1);
}

static void startupSharedMemory(void) {
	bool found;
	ereport(NOTICE, errmsg("0"));
	LWLockAcquire(AddinShmemInitLock, LW_EXCLUSIVE);
	st = ShmemInitStruct("shared_text", sizeof(st)+4, &found);
	ereport(NOTICE, errmsg("1"));
	if (!found) 
	{
		ereport(NOTICE, errmsg("2"));
       st->text = 0;
	}
	LWLockRelease(AddinShmemInitLock);
}

PG_FUNCTION_INFO_V1(test_shmem_set_t);

Datum test_shmem_set_t (PG_FUNCTION_ARGS)
{	
	char * input_tex;
	
	input_tex= text_to_cstring(PG_GETARG_TEXT_P_COPY(0));
	
	ereport(NOTICE, errmsg("%ld", strlen(input_tex)));
	//buf =(char *) palloc(strlen(input_tex)+4);//4 это место под 3 новых символа и терминирующий ноль

	//st->text = psprintf("%s!!!",input_tex);
	//st->text = psprintf("b");
	ereport(NOTICE, errmsg("00"));
	 if (st == NULL) {
		 ereport(NOTICE, errmsg("11"));
		 startupSharedMemory();
		 ereport(NOTICE, errmsg("22"));
    }
	st->text = psprintf("%s!!!",input_tex);
	PG_RETURN_TEXT_P(cstring_to_text(st->text));
}


PG_FUNCTION_INFO_V1(test_shmem_get_t);
Datum test_shmem_get_t (PG_FUNCTION_ARGS)
{
	if (st == NULL) {
        startupSharedMemory();
    }
    
    PG_RETURN_TEXT_P(cstring_to_text(st->text));
}
*/
////////////////////////////////////////////////////////////

//тестовая работа с latch. метод синхронизации потоков

static shmem_request_hook_type prev_shmem_request_hook = NULL;
static void requestSharedMemory(void);
static Latch *test_latch = NULL;
void _PG_init(void);

void
_PG_init(void)
{
	prev_shmem_request_hook = shmem_request_hook;
	shmem_request_hook = requestSharedMemory;
}

static void
requestSharedMemory(void)
{
	if(prev_shmem_request_hook)
	prev_shmem_request_hook();

	RequestAddinShmemSpace(MAXALIGN(sizeof(Latch)));
    RequestNamedLWLockTranche("test_latch", 1);
}

static void startupSharedMemory(void)
{
	bool found;

	LWLockAcquire(AddinShmemInitLock, LW_EXCLUSIVE);
	test_latch = ShmemInitStruct("test_latch", sizeof(test_latch), &found);

	if (!found)
		{
		InitSharedLatch(test_latch);
		}
	LWLockRelease(AddinShmemInitLock);
}

PG_FUNCTION_INFO_V1(latch_test1);

Datum latch_test1 (PG_FUNCTION_ARGS)
{
	char		*input_text;
	
	input_text = text_to_cstring(PG_GETARG_TEXT_P_COPY(0));
	
	if (test_latch == NULL)
		{
		startupSharedMemory();
		}
	OwnLatch(test_latch);

	ResetLatch(test_latch);

	WaitLatch(test_latch, WL_LATCH_SET | WL_TIMEOUT, 10000, PG_WAIT_EXTENSION);
	
	ereport(NOTICE, errmsg("%s", input_text));

	DisownLatch(test_latch);

	
   PG_RETURN_NULL();
}

PG_FUNCTION_INFO_V1(latch_test2);

Datum latch_test2 (PG_FUNCTION_ARGS)
{
if (test_latch == NULL)
{
startupSharedMemory();
}

SetLatch(test_latch);

PG_RETURN_NULL();
}
