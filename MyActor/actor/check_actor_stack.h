#ifndef __CHECK_ACTOR_STACK_H
#define __CHECK_ACTOR_STACK_H

//Ĭ�϶�ջ��С64k
#define kB	*1024

#ifdef WIN32
#define DEFAULT_STACKSIZE	(64 kB - STACK_RESERVED_SPACE_SIZE)
#elif __linux__
#define DEFAULT_STACKSIZE	CORO_CONTEXT_STATE_SPACE
#endif

#if (_DEBUG || DEBUG)
#define STACK_SIZE(__debug__, __release__) (__debug__)
#define STACK_SIZE_REL(__release__) DEFAULT_STACKSIZE
#else
#define STACK_SIZE(__debug__, __release__) (__release__)
#define STACK_SIZE_REL(__release__) (__release__)
#endif
//////////////////////////////////////////////////////////////////////////

#if (_DEBUG || DEBUG)
#	if (defined _WIN64) || (defined __x86_64__)
#define STACK_SIZE64(__debug32__, __debug64__, __release32, __release64__) (__debug64__)
#	else
#define STACK_SIZE64(__debug32__, __debug64__, __release32, __release64__) (__debug32__)
#	endif
#else
#	if (defined _WIN64) || (defined __x86_64__)
#define STACK_SIZE64(__debug32__, __debug64__, __release32, __release64__) (__release64__)
#define STACK_SIZE_REL64(__release32__, __release64__) (__release64__)
#	else
#define STACK_SIZE64(__debug32__, __debug64__, __release32, __release64__) (__release32)
#define STACK_SIZE_REL64(__release32__, __release64__) (__release32__)
#	endif
#endif
//////////////////////////////////////////////////////////////////////////

//��ջ��Ԥ���ռ䣬����ջ���
#if (_DEBUG || DEBUG)
#		if (defined _WIN64) || (defined __x86_64__)
#define STACK_RESERVED_SPACE_SIZE		(24 kB)
#		else
#define STACK_RESERVED_SPACE_SIZE		(16 kB)
#		endif
#	else
#		if (defined _WIN64) || (defined __x86_64__)
#define STACK_RESERVED_SPACE_SIZE		(16 kB)
#		else
#define STACK_RESERVED_SPACE_SIZE		(8 kB)
#		endif
#	endif

//ҳ���С
#define PAGE_SIZE					(4 kB)

//ջ״̬�����ռ�
#ifdef WIN32
#if (_DEBUG || DEBUG)
#define CORO_CONTEXT_STATE_SPACE	(3 * PAGE_SIZE)
#else
#define CORO_CONTEXT_STATE_SPACE	(2 * PAGE_SIZE)
#endif
#elif __linux__
#define CORO_CONTEXT_STATE_SPACE	(1 * PAGE_SIZE)
#endif

#endif