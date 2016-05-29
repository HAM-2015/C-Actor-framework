#ifndef __TRACE_STACK
#define __TRACE_STACK

#include "scattered.h"

#if (defined ENABLE_DUMP_STACK || defined PRINT_ACTOR_STACK)
struct stack_line_info
{
	stack_line_info(){}

	stack_line_info(stack_line_info&& s)
		:line(s.line), file(std::move(s.file)), module(std::move(s.module)), symbolName(std::move(s.symbolName)) {}

	stack_line_info(const stack_line_info& s)
		:line(s.line), file(s.file), module(s.module), symbolName(s.symbolName) {}

	void operator=(stack_line_info&& s)
	{
		line = s.line;
		file = std::move(s.file);
		module = std::move(s.module);
		symbolName = std::move(s.symbolName);
	}

	void operator=(const stack_line_info& s)
	{
		line = s.line;
		file = s.file;
		module = s.module;
		symbolName = s.symbolName;
	}

	friend std::ostream& operator <<(std::ostream& out, const stack_line_info& s)
	{
		out << "(line:" << s.line << ", file:" << s.file << ", module:" << s.module << ", symbolName:" << s.symbolName << ")";
		return out;
	}

	friend std::wostream& operator <<(std::wostream& out, const stack_line_info& s)
	{
		out << "(line:" << s.line << ", file:" << s.file.c_str() << ", module:" << s.module.c_str() << ", symbolName:" << s.symbolName.c_str() << ")";
		return out;
	}

	int line;
	std::string file;
	std::string module;
	std::string symbolName;
};

/*!
@brief ��ȡ��ǰ���ö�ջ��Ϣ
@param maxDepth ��ȡ��ǰ��ջ������߲�Σ����32��
*/
std::list<stack_line_info> get_stack_list(size_t maxDepth = 32, size_t offset = 0, bool module = false, bool symbolName = false);
std::list<stack_line_info> get_stack_list(void* bp, void* sp, void* ip, size_t maxDepth = 32, size_t offset = 0, bool module = false, bool symbolName = false);
#endif

#ifdef PRINT_ACTOR_STACK

//��¼��ǰActor�����Ϣ
#define ACTOR_POSITION(__host__) \
{\
	__host__->_checkStackFree = true; \
	stack_line_info __line; \
	__line.line = __LINE__; \
	__line.file = __FILE__; \
	__line.module = "actor position"; \
	__host__->_createStack->push_front(__line); \
}

//��¼��ǰActor�����Ϣ
#define SELF_POSITION ACTOR_POSITION(self)

/*!
@brief ��ջ���������Ϣ
*/
void stack_overflow_format(int size, const std::list<stack_line_info>& createStack);
void stack_overflow_format(int size, std::list<stack_line_info>&& createStack);

#else

//��¼��ǰActor�����Ϣ
#define ACTOR_POSITION(__host__)
#define SELF_POSITION

#endif

void install_check_stack();
void uninstall_check_stack();

#endif