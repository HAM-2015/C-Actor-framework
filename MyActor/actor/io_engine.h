#ifndef __IO_ENGINE_H
#define __IO_ENGINE_H

#include <boost/asio/io_service.hpp>
#include <boost/atomic/atomic.hpp>
#include <boost/thread.hpp>
#include <set>
#include <vector>

class StrandEx_;
class ActorTimer_;

class io_engine
{
	friend StrandEx_;
	friend ActorTimer_;
public:
#ifdef _MSC_VER
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
#elif __GNUG__
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
	io_engine();
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
	@brief ��⵱ǰ�����Ƿ��ڱ���������ִ��
	*/
	bool runningInThisIos();

	/*!
	@brief �������߳���
	*/
	size_t threadNumber();

#ifdef _MSC_VER
	/*!
	@brief ��������������̣߳�����win����Ч��
	*/
	void suspend();

	/*!
	@brief �ָ������������̣߳�����win����Ч��
	*/
	void resume();
#endif

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
	@brief ��ȡCPU������
	*/
	static unsigned physicalConcurrency();

	/*!
	@brief ��ȡCPU�߳���
	*/
	static unsigned hardwareConcurrency();

	/*!
	@brief �����߳�ID
	*/
	const std::set<boost::thread::id>& threadsID();

	/*!
	@brief ��������������
	*/
	operator boost::asio::io_service& () const;

	/*!
	@brief ��ȡtlsֵ
	@param 0 <= i < 64
	*/
	static void* getTlsValue(int i);

	/*!
	@brief ����tlsֵ
	@param 0 <= i < 64
	*/
	static void setTlsValue(int i, void* val);

	/*!
	@brief ��ȡĳ��tls�����ռ�
	@param 0 <= i < 64
	*/
	static void** getTlsValuePtr(int i);
private:
	void* getImpl();
	void freeImpl(void* impl);
	void* getTimer();
	void freeTimer(void* timer);
private:
	bool _opend;
	void* _implPool;
	void* _timerPool;
	priority _priority;
	boost::mutex _runMutex;
	boost::mutex _ctrlMutex;
	boost::asio::io_service _ios;
	boost::thread_group _runThreads;
	boost::atomic<long long> _runCount;
	std::set<boost::thread::id> _threadsID;
	boost::asio::io_service::work* _runLock;
	static boost::thread_specific_ptr<void*> _tls;///<64λvoid*
#ifdef _MSC_VER
	std::vector<HANDLE> _handleList;
#elif __GNUG__
	sched _policy;
	std::vector<pthread_attr_t> _handleList;
#endif
};

#endif