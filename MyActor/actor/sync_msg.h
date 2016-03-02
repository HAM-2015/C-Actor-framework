#ifndef __SYNC_MSG_H
#define __SYNC_MSG_H

#include "actor_framework.h"
#include "msg_queue.h"
#include "try_move.h"
#include "lambda_ref.h"

struct sync_csp_exception {};

struct sync_csp_close_exception : public sync_csp_exception {};

struct sync_msg_close_exception : public sync_csp_close_exception {};

struct csp_channel_close_exception : public sync_csp_close_exception {};

struct csp_try_invoke_exception : public sync_csp_exception {};

struct csp_timed_invoke_exception : public sync_csp_exception {};

struct csp_try_wait_exception : public sync_csp_exception {};

struct csp_timed_wait_exception : public sync_csp_exception {};


//////////////////////////////////////////////////////////////////////////
template <typename TP, typename TM>
struct SendMove_ : public try_move<TM>
{
};

template <typename TP, typename TM>
struct SendMove_<TP&, TM&&>
{
	static inline TM& move(TM& p0)
	{
		return p0;
	}
};

template <typename TP, typename TM>
struct SendMove_<TP&&, TM&&>
{
	static inline TM&& move(TM& p0)
	{
		return (TM&&)p0;
	}
};
//////////////////////////////////////////////////////////////////////////
template <typename TP>
struct TakeMove_
{
	static inline TP&& move(TP& p0)
	{
		return (TP&&)p0;
	}
};

template <typename TP>
struct TakeMove_<TP&>
{
	static inline TP& move(TP& p0)
	{
		return p0;
	}
};

template <typename TP>
struct TakeMove_<TP&&>
{
	static inline TP&& move(TP& p0)
	{
		return (TP&&)p0;
	}
};


template <typename T>
struct sync_msg_result_type
{
	typedef T type;
};

template <typename T>
struct sync_msg_result_type<T&>
{
	typedef std::reference_wrapper<T> type;
};

template <typename T>
struct sync_msg_result_type<T&&>
{
	typedef T type;
};

/*!
@brief ͬ��������Ϣ�������д����ɫ��ת���������ͷ��ȵ���Ϣȡ���ŷ���
*/
template <typename T>
class sync_msg
{
	typedef RM_REF(T) ST;
	typedef typename sync_msg_result_type<T>::type RT;

	//////////////////////////////////////////////////////////////////////////
	struct send_wait
	{
		bool can_move;
		bool& notified;
		ST& src_msg;
		actor_trig_notifer<bool> ntf;
	};

	struct take_wait
	{
		bool* can_move;
		bool& notified;
		unsigned char* dst;
		actor_trig_notifer<bool>& takeOk;
		actor_trig_notifer<bool> ntf;
	};
public:
	struct close_exception : public sync_msg_close_exception {};
public:
	sync_msg(const shared_strand& strand)
		:_closed(false), _strand(strand), _sendWait(4), _takeWait(4) {}
public:
	/*!
	@brief ͬ������һ����Ϣ��ֱ���Է�ȡ���󷵻�
	*/
	template <typename TM>
	void send(my_actor* host, TM&& msg)
	{
		host->lock_quit();
		actor_trig_handle<bool> ath;
		bool closed = false;
		bool notified = false;
		LAMBDA_THIS_REF5(ref5, host, msg, ath, closed, notified);
		host->send(_strand, [&ref5]
		{
			if (ref5->_closed)
			{
				ref5.closed = true;
			} 
			else
			{
				auto& _takeWait = ref5->_takeWait;
				if (_takeWait.empty())
				{
					ref5->_sendWait.push_front({ try_move<TM&&>::can_move, ref5.notified, (ST&)ref5.msg });
					ref5->_sendWait.front().ntf = ref5.host->make_trig_notifer_to_self(ref5.ath);
				}
				else
				{
					take_wait& wt = _takeWait.back();
					new(wt.dst)RT(SendMove_<T, TM&&>::move(ref5.msg));
					if (wt.can_move)
					{
						*wt.can_move = try_move<TM&&>::can_move || !(std::is_reference<T>::value);
					}
					wt.notified = true;
					wt.takeOk = ref5.host->make_trig_notifer_to_self(ref5.ath);
					wt.ntf(false);
					_takeWait.pop_back();
				}
			}
		});
		if (!closed)
		{
			closed = host->wait_trig(ath);
		}
		if (closed)
		{
			host->unlock_quit();
			throw close_exception();
		}
		host->unlock_quit();
	}

	/*!
	@brief ����ͬ������һ����Ϣ������Է��ڵȴ���Ϣ��ɹ�
	@return �ɹ�����true
	*/
	template <typename TM>
	bool try_send(my_actor* host, TM&& msg)
	{
		host->lock_quit();
		actor_trig_handle<bool> ath;
		bool ok = false;
		bool closed = false;
		LAMBDA_THIS_REF5(ref5, host, msg, ath, ok, closed);
		host->send(_strand, [&ref5]
		{
			if (ref5->_closed)
			{
				ref5.closed = true;
			}
			else
			{
				auto& _takeWait = ref5->_takeWait;
				if (!_takeWait.empty())
				{
					ref5.ok = true;
					take_wait& wt = _takeWait.back();
					new(wt.dst)RT(SendMove_<T, TM&&>::move(ref5.msg));
					if (wt.can_move)
					{
						*wt.can_move = try_move<TM&&>::can_move || !(std::is_reference<T>::value);
					}
					wt.takeOk = ref5.host->make_trig_notifer_to_self(ref5.ath);
					wt.ntf(false);
					_takeWait.pop_back();
				}
			}
		});
		if (!closed && ok)
		{
			closed = host->wait_trig(ath);
		}
		if (closed)
		{
			host->unlock_quit();
			throw close_exception();
		}
		host->unlock_quit();
		return ok;
	}

	/*!
	@brief ����ͬ������һ����Ϣ���Է���һ��ʱ����ȡ����ɹ�
	@return �ɹ�����true
	*/
	template <typename TM>
	bool timed_send(int tm, my_actor* host, TM&& msg)
	{
		host->lock_quit();
		actor_trig_handle<bool> ath;
		typename msg_list<send_wait>::iterator mit;
		bool closed = false;
		bool notified = false;
		LAMBDA_THIS_REF6(ref6, host, msg, ath, mit, closed, notified);
		host->send(_strand, [&ref6]
		{
			if (ref6->_closed)
			{
				ref6.closed = true;
			}
			else
			{
				auto& _takeWait = ref6->_takeWait;
				if (_takeWait.empty())
				{
					ref6->_sendWait.push_front({ try_move<TM&&>::can_move, ref6.notified, (T&)ref6.msg });
					ref6->_sendWait.front().ntf = ref6.host->make_trig_notifer_to_self(ref6.ath);
					ref6.mit = ref6->_sendWait.begin();
				}
				else
				{
					take_wait& wt = _takeWait.back();
					new(wt.dst)RT(SendMove_<T, TM&&>::move(ref6.msg));
					if (wt.can_move)
					{
						*wt.can_move = try_move<TM&&>::can_move || !(std::is_reference<T>::value);
					}
					wt.notified = true;
					wt.takeOk = ref6.host->make_trig_notifer_to_self(ref6.ath);
					wt.ntf(false);
					_takeWait.pop_back();
				}
			}
		});
		if (!closed)
		{
			if (!host->timed_wait_trig(tm, ath, closed))
			{
				host->send(_strand, [&ref6]
				{
					if (!ref6.notified)
					{
						ref6->_sendWait.erase(ref6.mit);
					}
				});
				if (notified)
				{
					closed = host->wait_trig(ath);
				}
			}
		}
		if (closed)
		{
			host->unlock_quit();
			throw close_exception();
		}
		host->unlock_quit();
		return notified;
	}

	/*!
	@brief ȡ��һ����Ϣ��ֱ������Ϣ�����ŷ���
	*/
	RT take(my_actor* host)
	{
		host->lock_quit();
		actor_trig_handle<bool> ath;
		actor_trig_notifer<bool> ntf;
		unsigned char msgBuf[sizeof(RT)];
		bool wait = false;
		bool closed = false;
		bool notified = false;
		LAMBDA_THIS_REF7(ref6, host, ath, ntf, msgBuf, wait, closed, notified);
		host->send(_strand, [&ref6]
		{
			if (ref6->_closed)
			{
				ref6.closed = true;
			}
			else
			{
				auto& _sendWait = ref6->_sendWait;
				if (_sendWait.empty())
				{
					ref6.wait = true;
					ref6->_takeWait.push_front({ NULL, ref6.notified, ref6.msgBuf, ref6.ntf });
					ref6->_takeWait.front().ntf = ref6.host->make_trig_notifer_to_self(ref6.ath);
				}
				else
				{
					send_wait& wt = _sendWait.back();
					new(ref6.msgBuf)RT(wt.can_move ? TakeMove_<T>::move(wt.src_msg) : wt.src_msg);
					wt.notified = true;
					ref6.ntf = std::move(wt.ntf);
					_sendWait.pop_back();
				}
			}
		});
		if (wait)
		{
			closed = host->wait_trig(ath);
		}
		if (!closed)
		{
			OUT_OF_SCOPE(
			{
				typedef RT TP_;
				((TP_*)msgBuf)->~TP_();
				ntf(false);
			});
			host->unlock_quit();
			return std::move((RT&)*(RT*)msgBuf);
		}
		host->unlock_quit();
		throw close_exception();
	}

	/*!
	@brief ����ȡ��һ����Ϣ������оͳɹ�
	@return �ɹ�����true
	*/
	template <typename TM>
	bool try_take(my_actor* host, TM& out)
	{
		return try_take_ct(host, [&](bool rval, ST& msg)
		{
			out = rval ? std::move(msg) : msg;
		});
	}

	bool try_take(my_actor* host, stack_obj<T>& out)
	{
		return try_take_ct(host, [&](bool rval, ST& msg)
		{
			out.create(rval ? std::move(msg) : msg);
		});
	}

	/*!
	@brief ����ȡ��һ����Ϣ����һ��ʱ����ȡ���ͳɹ�
	@return �ɹ�����true
	*/
	template <typename TM>
	bool timed_take(int tm, my_actor* host, TM& out)
	{
		return timed_take_ct(tm, host, [&](bool rval, ST& msg)
		{
			out = rval ? std::move(msg) : msg;
		});
	}

	bool timed_take(int tm, my_actor* host, stack_obj<T>& out)
	{
		return timed_take_ct(tm, host, [&](bool rval, ST& msg)
		{
			out.create(rval ? std::move(msg) : msg);
		});
	}

	/*!
	@brief �ر���Ϣͨ��������ִ���߽��׳� close_exception �쳣
	*/
	void close(my_actor* host)
	{
		host->lock_quit();
		host->send(_strand, [this]
		{
			_closed = true;
			while (!_takeWait.empty())
			{
				auto& wt = _takeWait.front();
				wt.notified = true;
				wt.ntf(true);
				_takeWait.pop_front();
			}
			while (!_sendWait.empty())
			{
				auto& st = _sendWait.front();
				st.notified = true;
				st.ntf(true);
				_sendWait.pop_front();
			}
		});
		host->unlock_quit();
	}

	/*!
	@brief close ������
	*/
	void reset()
	{
		assert(_closed);
		_closed = false;
	}
private:
	template <typename CT>
	bool try_take_ct(my_actor* host, const CT& ct)
	{
		host->lock_quit();
		actor_trig_handle<bool> ath;
		bool ok = false;
		bool closed = false;
		LAMBDA_THIS_REF5(ref5, host, ct, ath, ok, closed);
		host->send(_strand, [&ref5]
		{
			if (ref5->_closed)
			{
				ref5.closed = true;
			}
			else
			{
				auto& _sendWait = ref5->_sendWait;
				if (!_sendWait.empty())
				{
					ref5.ok = true;
					send_wait& wt = _sendWait.back();
					ref5.ct(wt.can_move, wt.src_msg);
					wt.notified = true;
					wt.ntf(false);
					_sendWait.pop_back();
				}
			}
		});
		if (closed)
		{
			host->unlock_quit();
			throw close_exception();
		}
		host->unlock_quit();
		return ok;
	}

	template <typename CT>
	bool timed_take_ct(int tm, my_actor* host, const CT& ct)
	{
		host->lock_quit();
		actor_trig_handle<bool> ath;
		actor_trig_notifer<bool> ntf;
		typename msg_list<take_wait>::iterator mit;
		unsigned char msgBuf[sizeof(RT)];
		bool wait = false;
		bool closed = false;
		bool notified = false;
		bool can_move = false;
		LAMBDA_REF5(ref4, wait, closed, notified, can_move, ntf);
		LAMBDA_THIS_REF6(ref6, ref4, host, ct, ath, mit, msgBuf);
		host->send(_strand, [&ref6]
		{
			auto& ref4 = ref6.ref9;
			if (ref6->_closed)
			{
				ref4.closed = true;
			}
			else
			{
				auto& _sendWait = ref6->_sendWait;
				if (_sendWait.empty())
				{
					ref4.wait = true;
					ref6->_takeWait.push_front({ &ref4.can_move, ref4.notified, ref6.msgBuf, ref4.ntf });
					ref6->_takeWait.front().ntf = ref6.host->make_trig_notifer_to_self(ref6.ath);
					ref6.mit = ref6->_takeWait.begin();
				}
				else
				{
					send_wait& wt = _sendWait.back();
					ref6.ct(wt.can_move, wt.src_msg);
					wt.notified = true;
					wt.ntf(false);
					_sendWait.pop_back();
				}
			}
		});
		bool ok = true;
		if (wait)
		{
			if (!host->timed_wait_trig(tm, ath, closed))
			{
				host->send(_strand, [&ref6]
				{
					if (!ref6.ref9.notified)
					{
						ref6->_takeWait.erase(ref6.mit);
					}
				});
				ok = notified;
				if (notified)
				{
					closed = host->wait_trig(ath);
				}
			}
			if (notified && !closed)
			{
				ct(can_move, *(RT*)msgBuf);
				((RT*)msgBuf)->~RT();
				ntf(false);
			}
		}
		if (closed)
		{
			host->unlock_quit();
			throw close_exception();
		}
		host->unlock_quit();
		return ok;
	}

	sync_msg(const sync_msg&){};
	void operator=(const sync_msg&){};
private:
	shared_strand _strand;
	msg_list<take_wait> _takeWait;
	msg_list<send_wait> _sendWait;
	bool _closed;
};

/*!
@brief csp�������ã���ֱ��ʹ��
*/
template <typename T, typename R>
class CspChannel_
{
	struct send_wait
	{
		bool& notified;
		T& srcMsg;
		unsigned char* res;
		actor_trig_notifer<bool> ntf;
	};

	struct take_wait
	{
		bool& notified;
		T*& srcMsg;
		unsigned char*& res;
		actor_trig_notifer<bool>& ntfSend;
		actor_trig_notifer<bool> ntf;
	};
protected:
	CspChannel_(const shared_strand& strand)
		:_closed(false), _strand(strand), _takeWait(4), _sendWait(4)
	{
		DEBUG_OPERATION(_thrownCloseExp = false);
	}
	~CspChannel_() {}
public:
	/*!
	@brief ��ʼ׼��ִ�жԷ���һ��������ִ����Ϻ󷵻ؽ��
	@return ���ؽ��
	*/
	template <typename TM>
	R send(my_actor* host, TM&& msg)
	{
		host->lock_quit();
		actor_trig_handle<bool> ath;
		unsigned char resBuf[sizeof(R)];
		bool closed = false;
		bool notified = false;
		LAMBDA_THIS_REF6(ref6, host, msg, ath, resBuf, closed, notified);
		host->send(_strand, [&ref6]
		{
			if (ref6->_closed)
			{
				ref6.closed = true;
			}
			else
			{
				auto& _takeWait = ref6->_takeWait;
				if (_takeWait.empty())
				{
					ref6->_sendWait.push_front({ ref6.notified, ref6.msg, ref6.resBuf });
					ref6->_sendWait.front().ntf = ref6.host->make_trig_notifer_to_self(ref6.ath);
				}
				else
				{
					take_wait& wt = _takeWait.back();
					wt.srcMsg = &ref6.msg;
					wt.notified = true;
					wt.res = ref6.resBuf;
					wt.ntfSend = ref6.host->make_trig_notifer_to_self(ref6.ath);
					wt.ntf(false);
					_takeWait.pop_back();
				}
			}
		});
		if (!closed)
		{
			closed = host->wait_trig(ath);
		}
		if (closed)
		{
			host->unlock_quit();
			throw_close_exception();
		}
		OUT_OF_SCOPE(
		{
			typedef R TP_;
			((TP_*)resBuf)->~TP_();
		});
		host->unlock_quit();
		return std::move(*(R*)resBuf);
	}

	/*!
	@brief ����ִ�жԷ���һ������������Է��ڵȴ�ִ�оͳɹ���ʧ���׳� try_invoke_exception �쳣
	@return �ɹ����ؽ��
	*/
	template <typename TM>
	R try_send(my_actor* host, TM&& msg)
	{
		host->lock_quit();
		actor_trig_handle<bool> ath;
		unsigned char resBuf[sizeof(R)];
		bool closed = false;
		bool has = false;
		LAMBDA_THIS_REF6(ref6, host, msg, ath, resBuf, closed, has);
		host->send(_strand, [&ref6]
		{
			if (ref6->_closed)
			{
				ref6.closed = true;
			}
			else
			{
				auto& _takeWait = ref6->_takeWait;
				if (!_takeWait.empty())
				{
					ref6.has = true;
					take_wait& wt = _takeWait.back();
					wt.srcMsg = &ref6.msg;
					wt.notified = true;
					wt.res = ref6.resBuf;
					wt.ntfSend = ref6.host->make_trig_notifer_to_self(ref6.ath);
					wt.ntf(false);
					_takeWait.pop_back();
				}
			}
		});
		if (!closed)
		{
			if (!has)
			{
				host->unlock_quit();
				throw_try_send_exception();
			}
			closed = host->wait_trig(ath);
		}
		if (closed)
		{
			host->unlock_quit();
			throw_close_exception();
		}
		OUT_OF_SCOPE(
		{
			typedef R TP_;
			((TP_*)resBuf)->~TP_();
		});
		host->unlock_quit();
		return std::move(*(R*)resBuf);
	}

	/*!
	@brief ����ִ�жԷ���һ������������Է���һ��ʱ�����ṩ�����ͳɹ���ʧ���׳� timed_invoke_exception �쳣
	@return �ɹ����ؽ��
	*/
	template <typename TM>
	R timed_send(int tm, my_actor* host, TM&& msg)
	{
		host->lock_quit();
		actor_trig_handle<bool> ath;
		unsigned char resBuf[sizeof(R)];
		typename msg_list<send_wait>::iterator nit;
		bool closed = false;
		bool notified = false;
		bool has = false;
		LAMBDA_THIS_REF8(ref8, host, msg, ath, resBuf, nit, closed, notified, has);
		host->send(_strand, [&ref8]
		{
			if (ref8->_closed)
			{
				ref8.closed = true;
			}
			else
			{
				auto& _takeWait = ref8->_takeWait;
				if (_takeWait.empty())
				{
					ref8->_sendWait.push_front({ ref8.notified, ref8.msg, ref8.resBuf });
					ref8->_sendWait.front().ntf = ref8.host->make_trig_notifer_to_self(ref8.ath);
					ref8.nit = ref8->_sendWait.begin();
				}
				else
				{
					ref8.has = true;
					take_wait& wt = _takeWait.back();
					wt.srcMsg = &ref8.msg;
					wt.notified = true;
					wt.res = ref8.resBuf;
					wt.ntfSend = ref8.host->make_trig_notifer_to_self(ref8.ath);
					wt.ntf(false);
					_takeWait.pop_back();
				}
			}
		});
		if (!closed)
		{
			if (!has)
			{
				if (!host->timed_wait_trig(tm, ath, closed))
				{
					host->send(_strand, [&ref8]
					{
						if (ref8.notified)
						{
							ref8.has = true;
						}
						else
						{
							ref8->_sendWait.erase(ref8.nit);
						}
					});
					if (!has)
					{
						host->unlock_quit();
						throw_timed_send_exception();
					}
				}
			}
			if (has)
			{
				closed = host->wait_trig(ath);
			}
		}
		if (closed)
		{
			host->unlock_quit();
			throw_close_exception();
		}
		OUT_OF_SCOPE(
		{
			typedef R TP_;
			((TP_*)resBuf)->~TP_();
		});
		host->unlock_quit();
		return std::move(*(R*)resBuf);
	}

	/*!
	@brief �ȴ��Է�ִ��һ�������壬�������ڷ��ضԷ���Ҫ��ִ�н��
	*/
	template <typename Func>
	void take(my_actor* host, Func&& h)
	{
		host->lock_quit();
		actor_trig_handle<bool> ath;
		actor_trig_notifer<bool> ntfSend;
		T* srcMsg = NULL;
		unsigned char* res = NULL;
		bool wait = false;
		bool closed = false;
		bool notified = false;
		LAMBDA_THIS_REF8(ref8, host, ath, ntfSend, srcMsg, res, wait, closed, notified);
		host->send(_strand, [&ref8]
		{
			if (ref8->_closed)
			{
				ref8.closed = true;
			}
			else
			{
				auto& _sendWait = ref8->_sendWait;
				if (_sendWait.empty())
				{
					ref8.wait = true;
					ref8->_takeWait.push_front({ ref8.notified, ref8.srcMsg, ref8.res, ref8.ntfSend });
					ref8->_takeWait.front().ntf = ref8.host->make_trig_notifer_to_self(ref8.ath);
				}
				else
				{
					send_wait& wt = _sendWait.back();
					ref8.srcMsg = &wt.srcMsg;
					wt.notified = true;
					ref8.ntfSend = std::move(wt.ntf);
					ref8.res = wt.res;
					_sendWait.pop_back();
				}
			}
		});
		if (wait)
		{
			closed = host->wait_trig(ath);
		}
		if (!closed)
		{
			bool ok = false;
			OUT_OF_SCOPE({ ntfSend(!ok); });
			BEGIN_TRY_
			{
				new(res)R(h(*srcMsg));
			}
			CATCH_FOR(sync_csp_close_exception)
			{
				assert(_thrownCloseExp);
				DEBUG_OPERATION(_thrownCloseExp = false);
				host->unlock_quit();
				throw_close_exception();
			}
			END_TRY_;
			ok = true;
			host->unlock_quit();
			return;
		}
		host->unlock_quit();
		throw_close_exception();
	}

	/*!
	@brief ����ִ��һ�������壬����Է���׼��ִ����ɹ��������׳� try_wait_exception �쳣
	*/
	template <typename Func>
	void try_take(my_actor* host, Func&& h)
	{
		host->lock_quit();
		actor_trig_notifer<bool> ntfSend;
		T* srcMsg = NULL;
		unsigned char* res = NULL;
		bool closed = false;
		bool has = false;
		LAMBDA_REF2(ref2, closed, has);
		LAMBDA_THIS_REF5(ref5, ref2, host, ntfSend, srcMsg, res);
		host->send(_strand, [&ref5]
		{
			auto& ref2 = ref5.ref2;
			if (ref5->_closed)
			{
				ref2.closed = true;
			}
			else
			{
				auto& _sendWait = ref5->_sendWait;
				if (!_sendWait.empty())
				{
					ref2.has = true;
					send_wait& wt = _sendWait.back();
					ref5.srcMsg = &wt.srcMsg;
					wt.notified = true;
					ref5.ntfSend = std::move(wt.ntf);
					ref5.res = wt.res;
					_sendWait.pop_back();
				}
			}
		});
		if (closed)
		{
			host->unlock_quit();
			throw_close_exception();
		}
		if (!has)
		{
			host->unlock_quit();
			throw_try_take_exception();
		}
		bool ok = false;
		OUT_OF_SCOPE({ ntfSend(!ok); });
		BEGIN_TRY_
		{
			new(res)R(h(*srcMsg));
		}
		CATCH_FOR(sync_csp_close_exception)
		{
			assert(_thrownCloseExp);
			DEBUG_OPERATION(_thrownCloseExp = false);
			host->unlock_quit();
			throw_close_exception();
		}
		END_TRY_;
		ok = true;
		host->unlock_quit();
	}

	/*!
	@brief ����ִ��һ�������壬����Է���һ��ʱ����ִ����ɹ��������׳� timed_wait_exception �쳣
	*/
	template <typename Func>
	void timed_take(int tm, my_actor* host, Func&& h)
	{
		host->lock_quit();
		actor_trig_handle<bool> ath;
		actor_trig_notifer<bool> ntfSend;
		typename msg_list<take_wait>::iterator wit;
		T* srcMsg = NULL;
		unsigned char* res = NULL;
		bool wait = false;
		bool closed = false;
		bool notified = false;
		LAMBDA_THIS_REF9(ref9, wait, closed, notified, wit, host, ath, ntfSend, srcMsg, res);
		host->send(_strand, [&ref9]
		{
			if (ref9->_closed)
			{
				ref9.closed = true;
			}
			else
			{
				auto& _sendWait = ref9->_sendWait;
				if (_sendWait.empty())
				{
					ref9.wait = true;
					ref9->_takeWait.push_front({ ref9.notified, ref9.srcMsg, ref9.res, ref9.ntfSend });
					ref9->_takeWait.front().ntf = ref9.host->make_trig_notifer_to_self(ref9.ath);
					ref9.wit = ref9->_takeWait.begin();
				}
				else
				{
					send_wait& wt = _sendWait.back();
					ref9.srcMsg = &wt.srcMsg;
					wt.notified = true;
					ref9.ntfSend = std::move(wt.ntf);
					ref9.res = wt.res;
					_sendWait.pop_back();
				}
			}
		});
		if (!closed)
		{
			if (wait)
			{
				if (!host->timed_wait_trig(tm, ath, closed))
				{
					host->send(_strand, [&ref9]
					{
						ref9.wait = ref9.notified;
						if (!ref9.notified)
						{
							ref9->_takeWait.erase(ref9.wit);
						}
					});
					if (wait)
					{
						closed = host->wait_trig(ath);
					}
					if (!wait && !closed)
					{
						host->unlock_quit();
						throw_timed_take_exception();
					}
				}
			}
		}
		if (closed)
		{
			host->unlock_quit();
			throw_close_exception();
		}
		bool ok = false;
		OUT_OF_SCOPE({ ntfSend(!ok); });
		BEGIN_TRY_
		{
			new(res)R(h(*srcMsg));
		}
		CATCH_FOR(sync_csp_close_exception)
		{
			assert(_thrownCloseExp);
			DEBUG_OPERATION(_thrownCloseExp = false);
			host->unlock_quit();
			throw_close_exception();
		}
		END_TRY_;
		ok = true;
		host->unlock_quit();
	}

	/*!
	@brief �ر�ִ��ͨ��������ִ���׳� close_exception �쳣
	*/
	void close(my_actor* host)
	{
		host->lock_quit();
		host->send(_strand, [this]
		{
			_closed = true;
			while (!_takeWait.empty())
			{
				auto& wt = _takeWait.front();
				wt.notified = true;
				wt.ntf(true);
				assert(wt.ntfSend.empty());
				_takeWait.pop_front();
			}
			while (!_sendWait.empty())
			{
				auto& st = _sendWait.front();
				st.notified = true;
				st.ntf(true);
				_sendWait.pop_front();
			}
		});
		host->unlock_quit();
	}

	/*!
	@brief close ������
	*/
	void reset()
	{
		assert(_closed);
		_closed = false;
		DEBUG_OPERATION(_thrownCloseExp = false);
	}
private:
	CspChannel_(const CspChannel_&){};
	void operator=(const CspChannel_&){};

	virtual void throw_close_exception() = 0;
	virtual void throw_try_send_exception() = 0;
	virtual void throw_timed_send_exception() = 0;
	virtual void throw_try_take_exception() = 0;
	virtual void throw_timed_take_exception() = 0;
private:
	shared_strand _strand;
	msg_list<take_wait> _takeWait;
	msg_list<send_wait> _sendWait;
	bool _closed;
protected:
	DEBUG_OPERATION(bool _thrownCloseExp);
};

//////////////////////////////////////////////////////////////////////////

struct VoidReturn_
{
};

template <typename T = void>
struct ReturnType_
{
	typedef TYPE_PIPE(T) type;
};

template <>
struct ReturnType_<void>
{
	typedef VoidReturn_ type;
};


template <typename R = void>
struct CspInvokeBaseFunc_
{
	typedef R&& result_type;

	template <typename H, typename TUPLE>
	static inline R invoke(H& h, TUPLE&& t)
	{
		return tuple_invoke<R>(h, TRY_MOVE(t));
	}
};

template <>
struct CspInvokeBaseFunc_<void>
{
	typedef void result_type;

	template <typename H, typename TUPLE>
	static inline VoidReturn_ invoke(H& h, TUPLE&& t)
	{
		tuple_invoke(h, TRY_MOVE(t));
		return VoidReturn_();
	}
};

template <typename R = void, typename... ARGS>
class CspInvokeBase_ : public CspChannel_<typename std::tuple<ARGS&...>, typename ReturnType_<R>::type>
{
	typedef std::tuple<ARGS&...> msg_type;
	typedef typename ReturnType_<R>::type return_type;
	typedef CspChannel_<msg_type, return_type> base_csp_channel;

protected:
	CspInvokeBase_(const shared_strand& strand)
		:base_csp_channel(strand) {}
	~CspInvokeBase_() {}
public:
	template <typename Func>
	void wait_invoke(my_actor* host, Func&& h)
	{
		base_csp_channel::take(host, [&](msg_type& msg)->return_type
		{
			return CspInvokeBaseFunc_<R>::invoke(h, msg);
		});
	}

	template <typename Func>
	void try_wait_invoke(my_actor* host, Func&& h)
	{
		base_csp_channel::try_take(host, [&](msg_type& msg)->return_type
		{
			return CspInvokeBaseFunc_<R>::invoke(h, msg);
		});
	}

	template <typename Func>
	void timed_wait_invoke(int tm, my_actor* host, Func&& h)
	{
		base_csp_channel::timed_take(tm, host, [&](msg_type& msg)->return_type
		{
			return CspInvokeBaseFunc_<R>::invoke(h, msg);
		});
	}

	template <typename... Args>
	R invoke(my_actor* host, Args&&... args)
	{
		return (typename CspInvokeBaseFunc_<R>::result_type)base_csp_channel::send(host, std::tuple<Args&...>(args...));
	}

	template <typename... Args>
	R invoke_rval(my_actor* host, Args&&... args)
	{
		return try_rval_invoke<R>([&](ARGS&... nargs)
		{
			return invoke(host, nargs...);
		}, TRY_MOVE(args)...);
	}

	template <typename... Args>
	R try_invoke(my_actor* host, Args&&... args)
	{
		return (typename CspInvokeBaseFunc_<R>::result_type)base_csp_channel::try_send(host, std::tuple<Args&...>(args...));
	}

	template <typename... Args>
	R try_invoke_rval(my_actor* host, Args&&... args)
	{
		return try_rval_invoke<R>([&](ARGS&... nargs)
		{
			return try_invoke(host, nargs...);
		}, TRY_MOVE(args)...);
	}

	template <typename... Args>
	R timed_invoke(int tm, my_actor* host, Args&&... args)
	{
		return (typename CspInvokeBaseFunc_<R>::result_type)base_csp_channel::timed_send(tm, host, std::tuple<Args&...>(args...));
	}

	template <typename... Args>
	R timed_invoke_rval(int tm, my_actor* host, Args&&... args)
	{
		return try_rval_invoke<R>([&](ARGS&... nargs)
		{
			return timed_invoke(tm, host, nargs...);
		}, TRY_MOVE(args)...);
	}
};

template <typename R, typename... ARGS>
class csp_invoke;

/*!
@brief CSPģ����Ϣ�������д����ɫ��ת�����ɵݹ飩�����ͷ�����Ϣȡ����������ɺ�ŷ���
*/
template <typename R, typename... ARGS>
class csp_invoke
	<R(ARGS...)>: public CspInvokeBase_<R, ARGS...>
{
	typedef CspInvokeBase_<R, ARGS...> Parent;
public:
	struct close_exception : public csp_channel_close_exception {};
	struct try_invoke_exception : public csp_try_invoke_exception {};
	struct timed_invoke_exception : public csp_timed_invoke_exception {};
	struct try_wait_exception : public csp_try_wait_exception {};
	struct timed_wait_exception : public csp_timed_wait_exception {};
public:
	csp_invoke(const shared_strand& strand)
		:Parent(strand) {}
private:
	void throw_close_exception()
	{
		assert(!Parent::_thrownCloseExp);
		DEBUG_OPERATION(Parent::_thrownCloseExp = true);
		throw close_exception();
	}

	void throw_try_send_exception()
	{
		throw try_invoke_exception();
	}

	void throw_timed_send_exception()
	{
		throw timed_invoke_exception();
	}

	void throw_try_take_exception()
	{
		throw try_wait_exception();
	}

	void throw_timed_take_exception()
	{
		throw timed_wait_exception();
	}
};

#endif