#include "meng.h"
#include "common.h"
#include <assert.h>

void on_meng_main_quit()
{
	meng * p = get_meng();
	assert(p);
	p->status = ms_end;
	assert(p->magic == 0xDEADBEEF);
	swap_context(p->last_context, p->father_context);
}

MENG_API meng * meng_create(meng_main func, size_t stacksize, void * arg)
{
	size_t size = sizeof(meng) + stacksize + CONTEXT_SIZE * 2;
	meng * ret = (meng *)MMALLOC(size);
	memset(ret, 0, size);
	ret->arg = arg;
	ret->func = func;
	ret->stack = (char *)ret + sizeof(meng);
	ret->father_context = (char *)ret + sizeof(meng) + stacksize;
	ret->last_context = (char *)ret + sizeof(meng) + stacksize + CONTEXT_SIZE;
	ret->stacksize = stacksize;
	ret->status = ms_start;
	ret->magic = 0xDEADBEEF;

	ini_context(ret->last_context);

	// 设置初始值
	long * sp = (long*)(ret->stack + stacksize);
	sp -= 2; // arg
	*sp = (long)ret;
	*(sp + 1) = (long)arg;
	*--sp = (long)on_meng_main_quit; // return
	*(long *)(ret->last_context + CONTEXT_RIP_POS) = (long)func;
	*(long *)(ret->last_context + CONTEXT_RBP_POS) = (long)sp;
	*(long *)(ret->last_context + CONTEXT_RSP_POS) = (long)sp;
	*(long *)(ret->last_context + CONTEXT_RDI_POS) = (long)ret;
	*(long *)(ret->last_context + CONTEXT_RSI_POS) = (long)arg;

	return ret;
}

// 跳转到新m执行，上下文保存到m中，同时把旧的m的上下文取出
MENG_API void meng_run(meng * m)
{
	if (m->status == ms_start)
	{
		swap_context(m->father_context, m->last_context);
	}
}

MENG_API bool meng_end(meng * m)
{
	return m->status == ms_end;
}

MENG_API void meng_delete(meng * m)
{
	MFREE(m);
}

MENG_API void meng_yield(meng * m)
{
	swap_context(m->last_context, m->father_context);
}
