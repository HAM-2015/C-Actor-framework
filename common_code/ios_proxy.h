#ifndef __IOS_PROXY_H
#define __IOS_PROXY_H

#include <boost/asio/io_service.hpp>
#include <boost/atomic/atomic.hpp>
#include <boost/thread.hpp>
#include <set>
#include <list>
#include <vector>

class strand_ex;

/*!
@brief io_service��������װ
*/
class ios_proxy
{
	friend strand_ex;
public:
	enum priority
	{
		above_normal = THREAD_PRIORITY_ABOVE_NORMAL,
		below_normal = THREAD_PRIORITY_BELOW_NORMAL,
		highest = THREAD_PRIORITY_HIGHEST,
		idle = THREAD_PRIORITY_IDLE,
		lowest = THREAD_PRIORITY_LOWEST,
		normal = THREAD_PRIORITY_NORMAL,
		time_critical = THREAD_PRIORITY_TIME_CRITICAL
	};
public:
	ios_proxy();
	~ios_proxy();
public:
	/*!
	@brief ��ʼ���е���������������������Ϻ���������
	@param threadNum �����������߳���
	*/
	void run(size_t threadNum = 1);

	/*!
	@brief �ȴ���������������ʱ����
	*/
	void stop();

	/*!
	@brief ��������������߳�
	*/
	void suspend();

	/*!
	@brief �ָ������������߳�
	*/
	void resume();

	/*!
	@brief ��⵱ǰ�����Ƿ��ڱ���������ִ��
	*/
	bool runningInThisIos();

	/*!
	@brief �������߳���
	*/
	size_t threadNumber();

	/*!
	@brief �������߳����ȼ�����
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
	@brief ����CPU����
	*/
	void cpuAffinity(unsigned mask);

	/*!
	@brief ��������������
	*/
	operator boost::asio::io_service& () const;
private:
	void* getImpl();
	void freeImpl(void* impl);
private:
	bool _opend;
	size_t _implCount;
	priority _priority;
	std::set<boost::thread::id> _threadIDs;
	std::list<void*> _implPool;
	std::vector<HANDLE> _handleList;
	boost::atomic<long long> _runCount;
	boost::mutex _ctrlMutex;
	boost::mutex _runMutex;
	boost::mutex _implMutex;
	boost::asio::io_service _ios;
	boost::asio::io_service::work* _runLock;
	boost::thread_group _runThreads;
};

#endif