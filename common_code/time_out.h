#ifndef __TIME_OUT_H
#define __TIME_OUT_H

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>
#include "shared_strand.h"

class deadline_timer_trig;
class system_timer_trig;
class steady_timer_trig;
class high_resolution_timer_trig;
class super_resolution_timer_trig;
/*!
@brief �첽��ʱ����
*/
class timeout_trig: public boost::enable_shared_from_this<timeout_trig>
{
	friend class deadline_timer_trig;
	friend class system_timer_trig;
	friend class steady_timer_trig;
	friend class high_resolution_timer_trig;
	friend class super_resolution_timer_trig;
public:
	enum timer_type
	{
		deadline_timer,
		system_timer,
		steady_timer,
		high_resolution_timer,
		super_resolution_timer
	};
protected:
	timeout_trig(shared_strand st);
public:
	virtual ~timeout_trig();
public:
	/*!
	@brief ����һ����ʱ��
	*/
	static boost::shared_ptr<timeout_trig> create(shared_strand st, timer_type type = deadline_timer);

	/*!
	@brief ����һ�����ڵ�ǰϵͳʱ��Ķ�ʱ��������ϵͳʱ���Ӱ�춨ʱ
	*/
	static boost::shared_ptr<timeout_trig> create_deadline(shared_strand st);

	/*!
	@brief ����һ�����ڵ�ǰϵͳʱ��Ķ�ʱ��������ϵͳʱ���Ӱ�춨ʱ
	*/
	static boost::shared_ptr<timeout_trig> create_system(shared_strand st);

	/*!
	@brief ����һ���ȶ���ʱ��������ϵͳʱ��Ӱ��
	*/
	static boost::shared_ptr<timeout_trig> create_steady(shared_strand st);

	/*!
	@brief ����һ���߾��ȶ�ʱ��������ϵͳʱ��Ӱ��
	*/
	static boost::shared_ptr<timeout_trig> create_high_resolution(shared_strand st);
	
	/*!
	@brief ����һ�����߾��ȶ�ʱ��������ϵͳʱ��Ӱ�죬��Ҫ������query_performance_counter::enable()
	*/
	static boost::shared_ptr<timeout_trig> create_super_resolution(shared_strand st);

	/*!
	@brief �����ڴ��ģʽ�´������󣬲��������ڴ��Ǹ��������
	@param sharedPoolMem �ڴ�ض��󣨸��ڴ����п��ܲ�ֻһ�����󣩣���������Ҳ����һ�ݸ����������б���sharedPoolMem�����Ķ����������Զ������ڴ�
	@param objPtr ���ζ���ʹ�õĵ�ַ
	*/
	static boost::shared_ptr<timeout_trig> create_in_pool(boost::shared_ptr<void> sharedPoolMem, void* objPtr, shared_strand st, timer_type tp);

	/*!
	@brief ��ȡ����ĳ��timer���ͺ�Ķ����С
	*/
	static size_t object_size(timer_type tp);

	/*!
	@brief ���ø߾���ʱ��
	*/
	static void enable_high_resolution();

	/*!
	@brief ��������Ȩ����Ϊ����ʵʱ��
	*/
	static void enable_realtime_priority();

	/*!
	@brief ��ȡϵͳʱ���(us)
	*/
	static long long get_tick();
public:
	/*!
	@brief ��ʼ��ʱ
	@param ms ������/΢��
	@param h ��������
	*/
	void timeOut(int ms, const boost::function<void ()>& h);
	
	/*!
	@brief ȡ���ϴμ�ʱ�����¿�ʼ��ʱ
	@param ms ������/΢��
	@param h ��������
	*/
	void timeOutAgain(int ms, const boost::function<void ()>& h);

	/*!
	@brief ȡ����ʱ������ȡ���󽫲��ٵ��ûص�����
	*/
	void cancel();

	/*!
	@brief �����ʱ�����ָ������ʣ���ʱ
	*/
	void suspend();

	/*!
	@brief �ָ���ʱ��
	*/
	void resume();

	/*!
	@brief ����ʱ�����Ͼʹ�����������Ϻ󷵻�
	*/
	void trigger();

	/*!
	@brief ��ȡ��ʱʣ��ʱ��
	*/
	int surplusTime();

	/*!
	@brief ����΢�뼶
	*/
	void enableMicrosec();
protected:
	virtual void expiresTimer() = 0;
	virtual void cancelTimer() = 0;
	void timeoutHandler(int id, const boost::system::error_code& error);
protected:
	int _timeoutID;
	int _isMillisec;
	bool _completed;
	bool _suspend;
	shared_strand _strand;
	boost::posix_time::microsec _time;
	boost::posix_time::ptime _stampBegin;
	boost::posix_time::ptime _stampEnd;
	boost::function<void ()> _h;
};

#endif