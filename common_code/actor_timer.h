#ifndef __ACTOR_TIMER_H
#define __ACTOR_TIMER_H

#include "shared_strand.h"
#include "msg_queue.h"
#include <boost/thread/mutex.hpp>
#include <boost/asio/high_resolution_timer.hpp>

typedef boost::asio::basic_waitable_timer<boost::chrono::high_resolution_clock> timer_type;

class boost_strand;
class mfc_strand;
class wx_strand;

class actor_timer
{
	typedef std::function<void()> call_back;
	typedef msg_list<call_back>::shared_node_alloc list_alloc;
	typedef std::shared_ptr<msg_list<call_back, list_alloc> > handler_list;
	typedef msg_map<unsigned long long, handler_list>::node_alloc map_alloc;
	typedef msg_map<unsigned long long, handler_list, map_alloc> handler_table;

	friend boost_strand;
	friend mfc_strand;
	friend wx_strand;
public:
	class timer_handle 
	{
		friend actor_timer;

		std::weak_ptr<msg_list<call_back, list_alloc> > _handlerList;
		msg_list<call_back, list_alloc>::iterator _handlerNode;
		handler_table::iterator _tableNode;
	};
private:
	actor_timer(shared_strand strand);
	~actor_timer();
public:
	timer_handle time_out(unsigned long long us, const std::function<void()>& h);
	void cancel(timer_handle& th);
private:
	void timer_loop(unsigned long long us);
private:
	ios_proxy& _ios;
	bool _looping;
	size_t _timerCount;
	timer_type* _timer;
	list_alloc _listAlloc;
	shared_strand _strand;
	handler_table _handlerTable;
	unsigned long long _extTimerFinish;
	std::weak_ptr<boost_strand> _weakStrand;
};

#endif