#ifndef __ACTOR_CPP_H
#define __ACTOR_CPP_H

/*

CHECK_SELF ���ü�⵱ǰ�����������ĸ�Actor��
ENABLE_QT_ACTOR ������QT-UI�߳�������Actor
ENABLE_NEXT_TICK ����next_tick���� 
ENABLE_CHECK_LOST ����֪ͨ�����ʧ���
DISABLE_BOOST_CORO ����boost::coroutine
ENALBE_TLS_CHECK_SELF ����TLS������⵱ǰ�����������ĸ�Actor��
DISABLE_HIGH_RESOLUTION ���ø߾��ȼ�ʱ

*/

#include "actor_framework.cpp"
#include "actor_mutex.cpp"
#include "actor_stack.cpp"
#include "actor_timer.cpp"
#include "io_engine.cpp"
#include "scattered.cpp"
#include "shared_strand.cpp"
#include "strand_ex.cpp"
#include "qt_strand.cpp"
#include "bind_qt_run.cpp"

#endif