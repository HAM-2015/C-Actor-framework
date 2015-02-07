#ifndef __CORO_STACK_H
#define __CORO_STACK_H

#include <boost/coroutine/stack_allocator.hpp>
#include <boost/coroutine/stack_context.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/atomic/atomic.hpp>
#include <list>
#include <vector>
#include <map>

using namespace std;

struct stack_pck 
{
	boost::coroutines::stack_context _stack;
	size_t _size;
};

/*!
@brief Э��ջ��
*/
class coro_stack_pool
{
	struct stack_pool_pck 
	{
		boost::mutex _mutex;
		list<stack_pck> _pool;
	};
private:
	coro_stack_pool();
public:
	~coro_stack_pool();
	static void enable();
	static bool isEnable();
public:
	static stack_pck getStack(size_t size);
	static void recovery(stack_pck& stack);
private:
	boost::mutex _mutex;
	boost::coroutines::stack_allocator _all;
	boost::atomic<int> _stackCount;
	vector<stack_pool_pck*> _stackPool;
	static boost::shared_ptr<coro_stack_pool> _coroStackPool;
};

#endif