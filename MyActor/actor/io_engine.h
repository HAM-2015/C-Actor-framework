#ifndef __IO_ENGINE_H
#define __IO_ENGINE_H

#include <algorithm>
#include <boost/asio/io_service.hpp>
#include <condition_variable>
#include <atomic>
#include <mutex>
#include <set>
#include <vector>
#include "scattered.h"
#include "strand_ex.h"
#include "mem_pool.h"
#include "run_thread.h"

class my_actor;
class boost_strand;
#ifdef DISABLE_BOOST_TIMER
class WaitableTimer_;
class WaitableTimerEvent_;
#endif

class io_engine
{
	friend boost_strand;
#ifdef DISABLE_BOOST_TIMER
	friend WaitableTimerEvent_;
#endif
public:
#ifdef WIN32
	enum priority
	{
		time_critical = THREAD_PRIORITY_TIME_CRITICAL,
		highest = THREAD_PRIORITY_HIGHEST,
		above_normal = THREAD_PRIORITY_ABOVE_NORMAL,
		normal = THREAD_PRIORITY_NORMAL,
		below_normal = THREAD_PRIORITY_BELOW_NORMAL,
		lowest = THREAD_PRIORITY_LOWEST,
		idle = THREAD_PRIORITY_IDLE
	};

	enum sched
	{
		sched_fifo = 0,
		sched_rr = 0,
		sched_other = 0
	};
#elif __linux__
	enum priority
	{
		time_critical = 99,
		highest = 83,
		above_normal = 66,
		normal = 50,
		below_normal = 33,
		lowest = 16,
		idle = 0
	};

	enum sched
	{
		sched_fifo = SCHED_FIFO,
		sched_rr = SCHED_RR,
		sched_other = SCHED_OTHER
	};
#endif
public:
	io_engine(bool enableTimer = true, const char* title = NULL);
	~io_engine();
public:
	/*!
	@brief ��ʼ���е���������������������Ϻ���������
	@param threadNum �����������߳���
	@param policy �̵߳��Ȳ���(linux����Ч��win�º���)
	*/
	void run(size_t threadNum = 1, sched policy = sched_other);

	/*!
	@brief �ȴ���������������ʱ����
	*/
	void stop();

	/*!
	@brief �ı�����߳�����֮ǰ���߳̽��˳�
	*/
	void changeThreadNumber(size_t threadNum);

	/*!
	@brief ��⵱ǰ�����Ƿ��ڱ���������ִ��
	*/
	bool runningInThisIos();

	/*!
	@brief �������߳���
	*/
	size_t threadNumber();

	/*!
	@brief ��������
	*/
	void suspend();

	/*!
	@brief �ָ�����
	*/
	void resume();

	/*!
	@brief �������߳����ȼ����ã���linux�� sched_fifo, sched_rr ��Ч ��
	*/
	void runPriority(priority pri);

	/*!
	@brief ��ȡ��ǰ���������ȼ�
	*/
	priority getPriority();

	/*!
	@brief ��ȡ���������δ�run()��stop()��ĵ�������
	*/
	long long getRunCount();

	/*!
	@brief �����߳�ID
	*/
	const std::set<run_thread::thread_id>& threadsID();

	/*!
	@brief ios title
	*/
	const std::string& title();

	/*!
	@brief ��������������
	*/
	operator boost::asio::io_service& () const;

	/*!
	@brief �ڷ�ios�߳��г�ʼ��һ��tls�ռ�
	*/
	static void setTlsBuff(void** buf);

	/*!
	@brief ��ȡtlsֵ
	@param 0 <= i < 64
	*/
	static void* getTlsValue(int i);

	/*!
	@brief ��ȡtlsֵ����
	@param 0 <= i < 64
	*/
	static void*& getTlsValueRef(int i);

	/*!
	@brief ����tlsֵ
	@param 0 <= i < 64
	*/
	static void setTlsValue(int i, void* val);

	/*!
	@brief ����tlsֵ
	@param 0 <= i < 64
	*/
	static void* swapTlsValue(int i, void* val);

	/*!
	@brief ��ȡtls�����ռ�
	*/
	static void** getTlsValueBuff();
private:
	friend my_actor;
	static void install();
	static void uninstall();
private:
	void _run(size_t threadNum, sched policy);
	void _stop();
private:
	bool _opend;
	bool _suspend;
	shared_obj_pool<boost_strand>* _strandPool;
#ifdef DISABLE_BOOST_TIMER
#ifdef ENABLE_GLOBAL_TIMER
	static WaitableTimer_* _waitableTimer;
#else
	WaitableTimer_* _waitableTimer;
#endif
#endif
	priority _priority;
	std::string _title;
	std::mutex _runMutex;
	std::mutex _ctrlMutex;
	std::atomic<long long> _runCount;
	std::set<run_thread::thread_id> _threadsID;
	std::list<run_thread*> _runThreads;
	boost::asio::io_service _ios;
	boost::asio::io_service::work* _runLock;
#ifdef WIN32
	std::vector<HANDLE> _handleList;
#elif __linux__
	sched _policy;
	std::vector<pthread_attr_t> _handleList;
#endif
	static tls_space* _tls;
};

#endif