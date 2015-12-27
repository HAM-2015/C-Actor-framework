#ifndef __ACTOR_CPP_H
#define __ACTOR_CPP_H

/*

CHECK_SELF ���ü�⵱ǰ�����������ĸ�Actor��
ENABLE_QT_UI ����QT-UI
ENABLE_QT_ACTOR ������QT-UI�߳�������Actor
ENABLE_NEXT_TICK ����next_tick���� 
ENABLE_WIN_FIBER windows������fiber
ENABLE_CHECK_LOST ����֪ͨ�����ʧ���
PRINT_ACTOR_STACK ���actor��ջ����ӡ��־
DISABLE_BOOST_CORO ����boost::coroutine
DISABLE_AUTO_STACK ����ջ�ռ��Զ���������
DISABLE_HIGH_TIMER ����high_resolution_timer��ʱ��������deadline_timer��ʱ
DISABLE_BOOST_TIMER ����boost��ʱ������waitable_timer��ʱ
ENABLE_TLS_CHECK_SELF ����TLS������⵱ǰ�����������ĸ�Actor��
ENABLE_CHECK_FUNC_STACK ���ú�����ջ���ļ��

windows�Ƽ�����:

ENABLE_NEXT_TICK
DISABLE_BOOST_CORO
ENABLE_WIN_FIBER
DISABLE_HIGH_TIMER
DISABLE_BOOST_TIMER

*/

#include "actor_framework.cpp"
#include "actor_mutex.cpp"
#include "actor_timer.cpp"
#include "async_timer.cpp"
#include "bind_qt_run.cpp"
#include "context_pool.cpp"
#include "context_yield.cpp"
#include "io_engine.cpp"
#include "qt_strand.cpp"
#include "scattered.cpp"
#include "shared_strand.cpp"
#include "strand_ex.cpp"
#include "waitable_timer.cpp"

#endif