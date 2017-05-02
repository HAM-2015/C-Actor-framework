#ifndef __ACTOR_CPP_H
#define __ACTOR_CPP_H

/*

CHECK_SELF ���ü�⵱ǰ�����������ĸ�Actor��
ENABLE_QT_ACTOR ����QT����QT-UI�߳�������Actor
ENABLE_UV_ACTOR ������Node.js uv�߳�������Actor
ENABLE_NEXT_TICK ����next_tick����
ENABLE_CHECK_LOST ����֪ͨ�����ʧ���
ENABLE_DUMP_STACK ����ջ������
PRINT_ACTOR_STACK ���actor��ջ����ӡ��־
DISABLE_AUTO_STACK ����ջ�ռ��Զ���������
DISABLE_BOOST_TIMER ����boost��ʱ������waitable_timer��ʱ
ENABLE_GLOBAL_TIMER ����ȫ�ֶ�ʱ��(DISABLE_BOOST_TIMER��ʹ��)
ENABLE_TLS_CHECK_SELF ����TLS������⵱ǰ�����������ĸ�Actor��
ENABLE_ASIO_HANDLER_ALLOCATE_EX ����asio handler��չ������
ENABLE_ASIO_PRE_OP ����tcp/udp��async_ioʱ�ȳ��Է�����io��ʧ�ܺ���Ͷ���첽����

*/

#include "actor_mutex.cpp"
#include "actor_socket.cpp"
#include "actor_timer.cpp"
#include "async_timer.cpp"
#include "bind_node_run.cpp"
#include "bind_qt_run.cpp"
#include "context_pool.cpp"
#include "context_yield.cpp"
#include "generator.cpp"
#include "io_engine.cpp"
#include "my_actor.cpp"
#include "qt_strand.cpp"
#include "run_thread.cpp"
#include "scattered.cpp"
#include "shared_strand.cpp"
#include "strand_ex.cpp"
#include "trace_stack.cpp"
#include "uv_strand.cpp"
#include "waitable_timer.cpp"

#endif