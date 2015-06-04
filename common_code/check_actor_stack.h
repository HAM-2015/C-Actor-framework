#ifndef __CHECK_ACTOR_STACK_H
#define __CHECK_ACTOR_STACK_H

//Ĭ�϶�ջ��С64k
#define kB	*1024
#define DEFAULT_STACKSIZE	(64 kB)

//��ջ��Ԥ���ռ䣬����ջ���
#ifdef CHECK_ACTOR_STACK
#	ifdef _DEBUG
#		ifdef _WIN64
#			define STACK_RESERVED_SPACE_SIZE		(24 kB)
#		else
#			define STACK_RESERVED_SPACE_SIZE		(16 kB)
#		endif
#	else
#		ifdef _WIN64
#			define STACK_RESERVED_SPACE_SIZE		(16 kB)
#		else
#			define STACK_RESERVED_SPACE_SIZE		(8 kB)
#		endif
#	endif
#else //CHECK_ACTOR_STACK
#	define STACK_RESERVED_SPACE_SIZE		0
#endif //CHECK_ACTOR_STACK

#endif