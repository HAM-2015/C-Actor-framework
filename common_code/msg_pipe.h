#ifndef __MSG_PIPE_H
#define __MSG_PIPE_H

#include "actor_framework.h"
#include "function_type.h"
#include <boost/atomic/atomic.hpp>
#include <boost/thread/shared_mutex.hpp>

/*!
@brief Actor��ͨ�Źܵ�
*/
template <typename T0 = void, typename T1 = void, typename T2 = void, typename T3 = void>
class msg_pipe
{
	typedef typename actor_msg_handle<T0, T1, T2, T3> reader_handle;
public:
	typedef typename func_type<T0, T1, T2, T3>::result writer_type;
	typedef typename std::function<size_t (my_actor*, reader_handle&)> regist_reader;
	typedef typename std::function<writer_type (int timeout)> get_writer_outside;
	__yield_interrupt typedef typename std::function<writer_type (my_actor*, int timeout)> get_writer;
private:
	template <typename T0 = void, typename T1 = void, typename T2 = void, typename T3 = void>
	struct temp_buffer
	{
		typedef msg_param<T0, T1, T2, T3> msg_type;

		void pop(const writer_type& wt)
		{
			while (!_tempBuff.empty())
			{
				auto t = _tempBuff.front();
				_tempBuff.pop_front();
				wt(t->_res0, t->_res1, t->_res2, t->_res3);
			}
		}

		writer_type temp_writer(boost::shared_mutex& mutex, std::shared_ptr<temp_buffer>& st)
		{
			return [st, &mutex](const T0& p0, const T1& p1, const T2& p2, const T3& p3)
			{
				std::shared_ptr<msg_type> t(new msg_type(p0, p1, p2, p3));
				mutex.lock_upgrade();
				st->_tempBuff.push_back(t);
				mutex.unlock_upgrade();
			};
		}

		list<std::shared_ptr<msg_type> > _tempBuff;
	};

	template <typename T0, typename T1, typename T2>
	struct temp_buffer<T0, T1, T2, void>
	{
		typedef msg_param<T0, T1, T2> msg_type;

		void pop(const writer_type& wt)
		{
			while (!_tempBuff.empty())
			{
				auto t = _tempBuff.front();
				_tempBuff.pop_front();
				wt(t->_res0, t->_res1, t->_res2);
			}
		}

		writer_type temp_writer(boost::shared_mutex& mutex, std::shared_ptr<temp_buffer>& st)
		{
			return [st, &mutex](const T0& p0, const T1& p1, const T2& p2)
			{
				std::shared_ptr<msg_type> t(new msg_type(p0, p1, p2));
				mutex.lock_upgrade();
				st->_tempBuff.push_back(t);
				mutex.unlock_upgrade();
			};
		}

		list<std::shared_ptr<msg_type> > _tempBuff;
	};

	template <typename T0, typename T1>
	struct temp_buffer<T0, T1, void, void>
	{
		typedef msg_param<T0, T1> msg_type;

		void pop(const writer_type& wt)
		{
			while (!_tempBuff.empty())
			{
				auto t = _tempBuff.front();
				_tempBuff.pop_front();
				wt(t->_res0, t->_res1);
			}
		}

		writer_type temp_writer(boost::shared_mutex& mutex, std::shared_ptr<temp_buffer>& st)
		{
			return [st, &mutex](const T0& p0, const T1& p1)
			{
				std::shared_ptr<msg_type> t(new msg_type(p0, p1));
				mutex.lock_upgrade();
				st->_tempBuff.push_back(t);
				mutex.unlock_upgrade();
			};
		}

		list<std::shared_ptr<msg_type> > _tempBuff;
	};

	template <typename T0>
	struct temp_buffer<T0, void, void, void>
	{
		typedef msg_param<T0> msg_type;

		void pop(const writer_type& wt)
		{
			while (!_tempBuff.empty())
			{
				auto t = _tempBuff.front();
				_tempBuff.pop_front();
				wt(t->_res0);
			}
		}

		writer_type temp_writer(boost::shared_mutex& mutex, std::shared_ptr<temp_buffer>& st)
		{
			return [st, &mutex](const T0& p0)
			{
				std::shared_ptr<msg_type> t(new msg_type(p0));
				mutex.lock_upgrade();
				st->_tempBuff.push_back(t);
				mutex.unlock_upgrade();
			};
		}

		list<std::shared_ptr<msg_type> > _tempBuff;
	};

	template <>
	struct temp_buffer<void, void, void, void>
	{
		temp_buffer()
			:_count(0)
		{

		}

		void pop(const writer_type& wt)
		{
			while (_count)
			{
				_count--;
				wt();
			}
		}

		writer_type temp_writer(boost::shared_mutex& mutex, std::shared_ptr<temp_buffer>& st)
		{
			return [st]()
			{
				st->_count++;
			};
		}

		boost::atomic<size_t> _count;
	};

	struct wrapped_param 
	{
		size_t _count;
		writer_type _handler;
		boost::shared_mutex _mutex;
	};

	class wrapped_invoke
	{
	public:
		wrapped_invoke(const std::shared_ptr<wrapped_param>& param)
		{
			_param = param;
			_param->_count = 0;
		}
	public:
		void operator()()
		{
			boost::shared_lock<boost::shared_mutex> sl(_param->_mutex);
			_param->_handler();
		}

		void operator()() const
		{
			boost::shared_lock<boost::shared_mutex> sl(_param->_mutex);
			_param->_handler();
		}

		template <typename Arg1>
		void operator()(const Arg1& arg1)
		{
			boost::shared_lock<boost::shared_mutex> sl(_param->_mutex);
			_param->_handler(arg1);
		}

		template <typename Arg1>
		void operator()(const Arg1& arg1) const
		{
			boost::shared_lock<boost::shared_mutex> sl(_param->_mutex);
			_param->_handler(arg1);
		}

		template <typename Arg1, typename Arg2>
		void operator()(const Arg1& arg1, const Arg2& arg2)
		{
			boost::shared_lock<boost::shared_mutex> sl(_param->_mutex);
			_param->_handler(arg1, arg2);
		}

		template <typename Arg1, typename Arg2>
		void operator()(const Arg1& arg1, const Arg2& arg2) const
		{
			boost::shared_lock<boost::shared_mutex> sl(_param->_mutex);
			_param->_handler(arg1, arg2);
		}

		template <typename Arg1, typename Arg2, typename Arg3>
		void operator()(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3)
		{
			boost::shared_lock<boost::shared_mutex> sl(_param->_mutex);
			_param->_handler(arg1, arg2, arg3);
		}

		template <typename Arg1, typename Arg2, typename Arg3>
		void operator()(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3) const
		{
			boost::shared_lock<boost::shared_mutex> sl(_param->_mutex);
			_param->_handler(arg1, arg2, arg3);
		}

		template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
		void operator()(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3,
			const Arg4& arg4)
		{
			boost::shared_lock<boost::shared_mutex> sl(_param->_mutex);
			_param->_handler(arg1, arg2, arg3, arg4);
		}

		template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
		void operator()(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3,
			const Arg4& arg4) const
		{
			boost::shared_lock<boost::shared_mutex> sl(_param->_mutex);
			_param->_handler(arg1, arg2, arg3, arg4);
		}
	public:
		std::shared_ptr<wrapped_param> _param;
	};
private:
	struct pipe_param
	{
		pipe_param()
			:_hasWriter(false) {DEBUG_OPERATION(_regCount = 0);}
		~pipe_param()
		{

		}
		bool _hasWriter;
		writer_type _writer;
		boost::mutex _mutex;
		list<std::function<void ()> > _getList;
		DEBUG_OPERATION(boost::atomic<size_t> _regCount);
	};

	struct outsite_pipe_param
	{
		outsite_pipe_param()
			:_hasWriter(false), _waitCount(0) {DEBUG_OPERATION(_regCount = 0);}
		~outsite_pipe_param()
		{

		}
		bool _hasWriter;
		writer_type _writer;
		size_t _waitCount;
		boost::mutex _mutex;
		boost::condition_variable _conVar;
		DEBUG_OPERATION(boost::atomic<size_t> _regCount);
	};
public:
	struct io 
	{
		regist_reader reader;
		writer_type writer;
	};
private:
	msg_pipe()
	{

	}
public:
	/*!
	@brief ����һ��Actor��Ϣ�ܵ�
	@param writer ����д�ܵ�����
	@return ����ע���ȡ���ݵĺ���
	*/
	static regist_reader make(__out writer_type& writer)
	{
		std::shared_ptr<wrapped_param> wrappedParam(new wrapped_param);
		std::shared_ptr<temp_buffer<T0, T1, T2, T3> > tempBuff(new temp_buffer<T0, T1, T2, T3>());
		wrappedParam->_handler = tempBuff->temp_writer(wrappedParam->_mutex, tempBuff);
		writer = wrapped_invoke(wrappedParam);

		std::weak_ptr<temp_buffer<T0, T1, T2, T3> > weakBuff = tempBuff;
		return [wrappedParam, weakBuff](my_actor* hostActor, reader_handle& rh)->size_t
		{
			SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
			size_t regCount = 0;
			{
				auto tb = weakBuff.lock();
				boost::unique_lock<boost::shared_mutex> ul(wrappedParam->_mutex);
				wrappedParam->_handler = hostActor->make_msg_notifer(rh);
				regCount = wrappedParam->_count++;
				if (tb)
				{
					tb->pop(wrappedParam->_handler);
				}
			}
			SetThreadPriority(GetCurrentThread(), hostActor->this_strand()->get_ios_proxy().getPriority());
			return regCount;
		};
	}

	/*!
	@brief ����һ��io
	*/
	static io make_io()
	{
		io res;
		res.reader = make(res.writer);
		return res;
	}
	
	/*!
	@brief Actor�ڴ���һ����Ϣ�ܵ�
	@param getWriterFunc ���ػ�ȡд�ܵ��ĺ�����ֻ���ڵ���regist_reader���غ���ܴӸú����еõ����
	@return ����ע���ȡ���ݵĺ�����ֻ��ע��һ��
	*/
	static regist_reader make(__out get_writer& getWriterFunc)
	{
		std::shared_ptr<pipe_param> pipeParam(new pipe_param);
		getWriterFunc = [pipeParam](my_actor* hostActor, int timeout)->writer_type
		{
			actor_trig_handle<> ath;
			pipeParam->_mutex.lock();
			if (!pipeParam->_hasWriter)
			{
				pipeParam->_getList.push_back(hostActor->make_trig_notifer(ath));
				pipeParam->_mutex.unlock();
				if (timeout >= 0)
				{
					hostActor->open_timer();
				}
				if (!hostActor->timed_wait_trig(ath, timeout))
				{
					return msg_pipe<T0, T1, T2, T3>::writer_type();
				}
			}
			else
			{
				pipeParam->_mutex.unlock();
			}
			return pipeParam->_writer;
		};

		std::weak_ptr<pipe_param> weakParam = pipeParam;
		return [weakParam](my_actor* hostActor, reader_handle& rh)->size_t
		{
			std::shared_ptr<pipe_param> pipeParam = weakParam.lock();
			if (pipeParam)
			{
				assert(0 == pipeParam->_regCount++);
				pipeParam->_writer = hostActor->make_msg_notifer(rh);
				pipeParam->_mutex.lock();
				pipeParam->_hasWriter = true;
				while (!pipeParam->_getList.empty())
				{
					pipeParam->_getList.front()();
					pipeParam->_getList.pop_front();
				}
				pipeParam->_mutex.unlock();
				return 0;
			}
			return -1;
		};
	}
	
	/*!
	@brief ͬ�ϣ�get_writer_outside ֻ��������� regist_reader ��ͬ�ĵ������ڻ�ȡ writer_type
	*/
	static regist_reader make(__out get_writer_outside& getWriterFunc)
	{
		std::shared_ptr<outsite_pipe_param> pipeParam(new outsite_pipe_param);
		getWriterFunc = [pipeParam](int timeout)->writer_type
		{
			{
				boost::unique_lock<boost::mutex> ul(pipeParam->_mutex);
				if (!pipeParam->_hasWriter)
				{
					pipeParam->_waitCount++;
					if (timeout < 0)
					{
						pipeParam->_conVar.wait(ul);
					}
					else if (!pipeParam->_conVar.timed_wait(ul, boost::posix_time::milliseconds(timeout)))
					{
						return msg_pipe<T0, T1, T2, T3>::writer_type();
					}
				}
			}
			return pipeParam->_writer;
		};

		std::weak_ptr<outsite_pipe_param> weakParam = pipeParam;
		return [weakParam](my_actor* hostActor, reader_handle& rh)->size_t
		{
			std::shared_ptr<outsite_pipe_param> pipeParam = weakParam.lock();
			if (pipeParam)
			{
				assert(0 == pipeParam->_regCount++);
				pipeParam->_writer = hostActor->make_msg_notifer(rh);
				boost::unique_lock<boost::mutex> ul(pipeParam->_mutex);
				pipeParam->_hasWriter = true;
				if (pipeParam->_waitCount)
				{
					pipeParam->_waitCount = 0;
					pipeParam->_conVar.notify_all();
				}
				return 0;
			}
			return -1;
		};
	}
private:
};

#endif