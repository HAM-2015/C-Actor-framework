#ifndef __ASYNC_BUFFER_H
#define __ASYNC_BUFFER_H

#include <boost/circular_buffer.hpp>
#include <functional>
#include <memory>
#include "actor_framework.h"

struct async_buffer_close_exception {};

/*!
@brief �첽������У���д/�������ɫ��ת��
*/
template <typename T>
class async_buffer
{
	struct push_pck 
	{
		void operator ()(bool closed)
		{
			notified = true;
			ntf(closed);
		}

		bool& notified;
		actor_trig_notifer<bool> ntf;
	};

	struct pop_pck
	{
		void operator ()(bool closed)
		{
			notified = true;
			ntf(closed);
		}

		bool& notified;
		actor_trig_notifer<bool> ntf;
	};
	
	struct buff_push
	{
		template <typename Fst, typename... Args>
		inline void operator ()(Fst&& fst, Args&&... args)
		{
			if (0 == _i)
			{
				_this->_buffer.push_back(TRY_MOVE(fst));
			} 
			else
			{
				buff_push bp = { _this, _i - 1 };
				bp(TRY_MOVE(args)...);
			}
		}

		inline void operator ()()
		{
			assert(false);
		}

		async_buffer* const _this;
		const size_t _i;
	};

	struct buff_pop
	{
		template <typename Fst, typename... Outs>
		inline void operator ()(Fst& fst, Outs&... outs)
		{
			if (0 == _i)
			{
				fst = std::move(_this->_buffer.front());
			} 
			else
			{
				buff_pop bp = { _this, _i - 1 };
				bp(outs...);
			}
		}

		inline void operator ()()
		{
			assert(false);
		}

		async_buffer* const _this;
		const size_t _i;
	};
public:
	struct close_exception : public async_buffer_close_exception {};
public:
	async_buffer(const shared_strand& strand, size_t buffLength, size_t halfLength = 0)
		:_strand(strand), _closed(false), _buffer(buffLength)//, _pushWait(4), _popWait(4)
	{
		_halfLength = halfLength ? halfLength : buffLength / 2;
	}
public:
	/*!
	@brief ��������Ӷ������ݣ�������������͵ȴ�ֱ�����ɹ�
	*/
	template <typename...  TMS>
	void push(my_actor* host, TMS&&... msgs)
	{
		static_assert(sizeof...(TMS) > 0, "");
		size_t pushCount = 0;
		_push(host, [&]()->bool
		{
			buff_push bp = { this, pushCount };
			bp((TMS&&)(msgs)...);
			return sizeof...(TMS) == ++pushCount;
		});
	}

	/*!
	@brief ����������ݣ�������������ͷ���
	@return true ��ӳɹ�
	*/
	template <typename TM>
	bool try_push(my_actor* host, TM&& msg)
	{
		return !!_try_push(host, [&]()->bool
		{
			_buffer.push_back((TM&&)msg);
			return true;
		});
	}

	/*!
	@brief �����ҳ���������ݣ�������зŲ��¾ͷ��������
	@return �����ҳɹ�����˼���
	*/
	template <typename...  TMS>
	size_t try_push(my_actor* host, TMS&&... msgs)
	{
		static_assert(sizeof...(TMS) > 1, "");
		size_t pushCount = 0;
		return _try_push(host, [&]()->bool
		{
			buff_push bp = { this, pushCount };
			bp((TMS&&)(msgs)...);
			return sizeof...(TMS) == ++pushCount;
		});
	}

	/*!
	@brief ��һ��ʱ���ڳ���������ݣ�������е�ʱ���޷���Ӿͷ���
	@return true ��ӳɹ�
	*/
	template <typename TM>
	bool timed_push(int tm, my_actor* host, TM&& msg)
	{
		return !!_timed_push(tm, host, [&]()->bool
		{
			_buffer.push_back((TM&&)msg);
			return true;
		});
	}

	/*!
	@brief ��������һ��ʱ���ڳ�����Ӷ������ݣ�������е�ʱ���޷���Ӿͷ���
	@return �����ҳɹ�����˼���
	*/
	template <typename...  TMS>
	size_t timed_push(int tm, my_actor* host, TMS&&... msgs)
	{
		static_assert(sizeof...(TMS) > 1, "");
		size_t pushCount = 0;
		return _timed_push(tm, host, [&]()->bool
		{
			buff_push bp = { this, pushCount };
			bp((TMS&&)(msgs)...);
			return sizeof...(TMS) == ++pushCount;
		});
	}

	/*!
	@brief ������ǿ����Ӷ������ݣ��������оͶ�����ǰ�������
	@return �����˼�������
	*/
	template <typename...  TMS>
	size_t force_push(my_actor* host, TMS&&... msgs)
	{
		static_assert(sizeof...(TMS) > 0, "");
		size_t pushCount = 0;
		return _force_push(host, [&]()->bool
		{
			buff_push bp = { this, pushCount };
			bp((TMS&&)(msgs)...);
			return sizeof...(TMS) == ++pushCount;
		});
	}

	/*!
	@brief ����һ�����ݣ�������пվ͵ȴ�ֱ��������
	*/
	T pop(my_actor* host)
	{
		char resBuf[sizeof(T)];
		_pop(host, [&]()->bool
		{
			new(resBuf)T(std::move(_buffer.front()));
			_buffer.pop_front();
			return true;
		});
		OUT_OF_SCOPE(
		{
			typedef T TP_;
			((TP_*)resBuf)->~TP_();
		});
		return std::move(*(T*)resBuf);
	}

	/*!
	@brief �����ҵ����������ݣ�������в����͵ȴ�ֱ���ɹ�
	*/
	template <typename... OTMS>
	void pop(my_actor* host, OTMS&... outs)
	{
		static_assert(sizeof...(OTMS) > 0, "");
		size_t popCount = 0;
		_pop(host, [&]()->bool
		{
			buff_pop bp = { this, popCount };
			bp(outs...);
			_buffer.pop_front();
			return sizeof...(OTMS) == ++popCount;
		});
	}

	/*!
	@brief ���Ե���һ�����ݣ��������Ϊ�վͷ���
	@return �ɹ��õ���Ϣ
	*/
	template <typename OTM>
	bool try_pop(my_actor* host, OTM& out)
	{
		size_t popCount = 0;
		return !!_try_pop(host, [&]()->bool
		{
			out = std::move(_buffer.front());
			_buffer.pop_front();
			return true;
		});
	}

	/*!
	@brief �����ҳ��Ե����������ݣ�������в�������ʣ�µ�
	@return �����ҵõ���������
	*/
	template <typename... OTMS>
	size_t try_pop(my_actor* host, OTMS&... outs)
	{
		static_assert(sizeof...(OTMS) > 1, "");
		size_t popCount = 0;
		return _try_pop(host, [&]()->bool
		{
			buff_pop bp = { this, popCount };
			bp(outs...);
			_buffer.pop_front();
			return sizeof...(OTMS) == ++popCount;
		});
	}

	/*!
	@brief ��һ��ʱ���ڳ��Ե���һ�����ݣ�������е�ʱ�������ݾͷ���
	@return true �ɹ��õ���Ϣ
	*/
	template <typename OTM>
	bool timed_pop(int tm, my_actor* host, OTM& out)
	{
		return !!_timed_pop(tm, host, [&]()->bool
		{
			out = std::move(_buffer.front());
			_buffer.pop_front();
			return true;
		});
	}

	/*!
	@brief ��������һ��ʱ���ڳ��Ե����������ݣ�������е�ʱ�������ͷ���ʣ�µ�
	@return �ɹ��õ�������Ϣ
	*/
	template <typename... OTMS>
	size_t timed_pop(int tm, my_actor* host, OTMS&... outs)
	{
		static_assert(sizeof...(OTMS) > 1, "");
		size_t popCount = 0;
		return _timed_pop(tm, host, [&]()->bool
		{
			buff_pop bp = { this, popCount };
			bp(outs...);
			_buffer.pop_front();
			return sizeof...(OTMS) == ++popCount;
		});
	}

	void close(my_actor* host, bool clearBuff = true)
	{
		host->lock_quit();
		host->send(_strand, [&]
		{
			_closed = true;
			if (clearBuff)
			{
				_buffer.clear();
			}
			while (!_pushWait.empty())
			{
				_pushWait.front()(true);
				_pushWait.pop_front();
			}
			while (!_popWait.empty())
			{
				_popWait.front()(true);
				_popWait.pop_front();
			}
		});
		host->unlock_quit();
	}

	size_t length(my_actor* host)
	{
		host->lock_quit();
		size_t l = 0;
		host->send(_strand, [&]
		{
			l = _buffer.size();
		});
		host->unlock_quit();
		return l;
	}

	size_t snap_length()
	{
		return _buffer.size();
	}

	void reset()
	{
		assert(_closed);
		_closed = false;
	}
private:
	async_buffer(const async_buffer&){};
	void operator=(const async_buffer&){};

	template <typename Func>
	void _push(my_actor* host, Func&& h)
	{
		host->lock_quit();
		while (true)
		{
			actor_trig_handle<bool> ath;
			bool notified = false;
			bool isFull = false;
			bool closed = false;
			LAMBDA_THIS_REF6(ref6, host, h, ath, notified, isFull, closed);
			host->send(_strand, [&ref6]
			{
				ref6.closed = ref6->_closed;
				if (!ref6->_closed)
				{
					while (true)
					{
						bool break_ = true;
						ref6.isFull = ref6->_buffer.full();
						if (!ref6.isFull)
						{
							break_ = ref6.h();
							auto& _popWait = ref6->_popWait;
							if (!_popWait.empty())
							{
								_popWait.back()(false);
								_popWait.pop_back();
							}
						}
						else
						{
							ref6->_pushWait.push_front({ ref6.notified });
							ref6->_pushWait.front().ntf = ref6.host->make_trig_notifer_to_self(ref6.ath);
						}
						if (break_)
						{
							break;
						}
					}
				}
			});
			if (closed)
			{
				host->unlock_quit();
				throw close_exception();
			}
			if (!isFull)
			{
				host->unlock_quit();
				return;
			}
			if (host->wait_trig(ath))
			{
				host->unlock_quit();
				throw close_exception();
			}
		}
		host->unlock_quit();
	}

	template <typename Func>
	size_t _try_push(my_actor* host, Func&& h)
	{
		host->lock_quit();
		bool closed = false;
		size_t pushCount = 0;
		LAMBDA_THIS_REF4(ref4, host, h, closed, pushCount);
		host->send(_strand, [&ref4]
		{
			ref4.closed = ref4->_closed;
			if (!ref4->_closed)
			{
				while (true)
				{
					bool break_ = true;
					if (!ref4->_buffer.full())
					{
						ref4.pushCount++;
						break_ = ref4.h();
						auto& _popWait = ref4->_popWait;
						if (!_popWait.empty())
						{
							_popWait.back()(false);
							_popWait.pop_back();
						}
					}
					if (break_)
					{
						break;
					}
				}
			}
		});
		if (closed)
		{
			host->unlock_quit();
			throw close_exception();
		}
		host->unlock_quit();
		return pushCount;
	}

	template <typename Func>
	size_t _timed_push(int tm, my_actor* host, Func&& h)
	{
		host->lock_quit();
		long long utm = (long long)tm * 1000;
		size_t pushCount = 0;
		while (true)
		{
			actor_trig_handle<bool> ath;
			typename msg_list<push_pck>::iterator mit;
			bool notified = false;
			bool isFull = false;
			bool closed = false;
			LAMBDA_THIS_REF8(ref8, notified, isFull, closed, host, h, ath, mit, pushCount);
			host->send(_strand, [&ref8]
			{
				ref8.closed = ref8->_closed;
				if (!ref8->_closed)
				{
					while (true)
					{
						bool break_ = true;
						ref8.isFull = ref8->_buffer.full();
						if (!ref8.isFull)
						{
							ref8.pushCount++;
							break_ = ref8.h();
							auto& _popWait = ref8->_popWait;
							if (!_popWait.empty())
							{
								_popWait.back()(false);
								_popWait.pop_back();
							}
						}
						else
						{
							ref8->_pushWait.push_front({ ref8.notified });
							ref8->_pushWait.front().ntf = ref8.host->make_trig_notifer_to_self(ref8.ath);
							ref8.mit = ref8->_pushWait.begin();
						}
						if (break_)
						{
							break;
						}
					}
				}
			});
			if (!closed)
			{
				if (!isFull)
				{
					host->unlock_quit();
					return pushCount;
				}
				long long bt = get_tick_us();
				if (!host->timed_wait_trig(utm / 1000, ath, closed))
				{
					host->send(_strand, [&ref8]
					{
						if (!ref8.notified)
						{
							ref8->_pushWait.erase(ref8.mit);
						}
					});
					if (notified)
					{
						closed = host->wait_trig(ath);
					}
				}
				utm -= get_tick_us() - bt;
				if (utm < 1000)
				{
					host->unlock_quit();
					return pushCount;
				}
			}
			if (closed)
			{
				host->unlock_quit();
				throw close_exception();
			}
		}
		host->unlock_quit();
		return pushCount;
	}

	template <typename Func>
	size_t _force_push(my_actor* host, Func&& h)
	{
		host->lock_quit();
		bool closed = false;
		size_t lostCount = 0;
		LAMBDA_THIS_REF4(ref4, host, h, closed, lostCount);
		host->send(_strand, [&ref4]
		{
			ref4.closed = ref4->_closed;
			if (!ref4->_closed)
			{
				while (true)
				{
					if (ref4->_buffer.full())
					{
						ref4.lostCount++;
						ref4->_buffer.pop_front();
					}
					bool break_ = ref4.h();
					auto& _popWait = ref4->_popWait;
					if (!_popWait.empty())
					{
						_popWait.back()(false);
						_popWait.pop_back();
					}
					if (break_)
					{
						break;
					}
				}
			}
		});
		if (closed)
		{
			host->unlock_quit();
			throw close_exception();
		}
		host->unlock_quit();
		return lostCount;
	}

	template <typename Func>
	void _pop(my_actor* host, Func&& out)
	{
		host->lock_quit();
		while (true)
		{
			actor_trig_handle<bool> ath;
			bool notified = false;
			bool isEmpty = false;
			bool closed = false;
			LAMBDA_THIS_REF6(ref6, host, out, ath, notified, isEmpty, closed);
			host->send(_strand, [&ref6]
			{
				ref6.closed = ref6->_closed;
				if (!ref6->_closed)
				{
					while (true)
					{
						bool break_ = true;
						auto& _buffer = ref6->_buffer;
						auto& _pushWait = ref6->_pushWait;
						ref6.isEmpty = _buffer.empty();
						if (!ref6.isEmpty)
						{
							break_ = ref6.out();
						}
						else
						{
							ref6->_popWait.push_front({ ref6.notified });
							ref6->_popWait.front().ntf = ref6.host->make_trig_notifer_to_self(ref6.ath);
						}
						if (!_pushWait.empty() && _buffer.size() <= ref6->_halfLength)
						{
							_pushWait.back()(false);
							_pushWait.pop_back();
						}
						if (break_)
						{
							break;
						}
					}
				}
			});
			if (closed)
			{
				host->unlock_quit();
				throw close_exception();
			}
			if (!isEmpty)
			{
				host->unlock_quit();
				return;
			}
			if (host->wait_trig(ath))
			{
				host->unlock_quit();
				throw close_exception();
			}
		}
		host->unlock_quit();
	}

	template <typename Func>
	size_t _try_pop(my_actor* host, Func&& out)
	{
		host->lock_quit();
		bool closed = false;
		bool popCount = 0;
		LAMBDA_THIS_REF4(ref4, host, out, closed, popCount);
		host->send(_strand, [&ref4]
		{
			ref4.closed = ref4->_closed;
			if (!ref4->_closed)
			{
				while (true)
				{
					bool break_ = true;
					auto& _buffer = ref4->_buffer;
					auto& _pushWait = ref4->_pushWait;
					if (!_buffer.empty())
					{
						ref4.popCount++;
						break_ = ref4.out();
					}
					if (!_pushWait.empty() && _buffer.size() <= ref4->_halfLength)
					{
						_pushWait.back()(false);
						_pushWait.pop_back();
					}
					if (break_)
					{
						break;
					}
				}
			}
		});
		if (closed)
		{
			host->unlock_quit();
			throw close_exception();
		}
		host->unlock_quit();
		return popCount;
	}

	template <typename Func>
	size_t _timed_pop(int tm, my_actor* host, Func&& out)
	{
		host->lock_quit();
		long long utm = (long long)tm * 1000;
		size_t popCount = 0;
		while (true)
		{
			actor_trig_handle<bool> ath;
			typename msg_list<pop_pck>::iterator mit;
			bool notified = false;
			bool isEmpty = false;
			bool closed = false;
			LAMBDA_THIS_REF8(ref8, notified, isEmpty, closed, host, out, ath, mit, popCount);
			host->send(_strand, [&ref8]
			{
				ref8.closed = ref8->_closed;
				if (!ref8->_closed)
				{
					while (true)
					{
						bool break_ = true;
						auto& _buffer = ref8->_buffer;
						auto& _pushWait = ref8->_pushWait;
						ref8.isEmpty = _buffer.empty();
						if (!ref8.isEmpty)
						{
							ref8.popCount++;
							break_ = ref8.out();
						}
						else
						{
							ref8->_popWait.push_front({ ref8.notified });
							ref8->_popWait.front().ntf = ref8.host->make_trig_notifer_to_self(ref8.ath);
							ref8.mit = ref8->_popWait.begin();
						}
						if (!_pushWait.empty() && _buffer.size() <= ref8->_halfLength)
						{
							_pushWait.back()(false);
							_pushWait.pop_back();
						}
						if (break_)
						{
							break;
						}
					}
				}
			});
			if (!closed)
			{
				if (!isEmpty)
				{
					host->unlock_quit();
					return popCount;
				}
				long long bt = get_tick_us();
				if (!host->timed_wait_trig(utm / 1000, ath, closed))
				{
					host->send(_strand, [&ref8]
					{
						if (!ref8.notified)
						{
							ref8->_popWait.erase(ref8.mit);
						}
					});
					if (notified)
					{
						closed = host->wait_trig(ath);
					}
				}
				utm -= get_tick_us() - bt;
				if (utm < 1000)
				{
					host->unlock_quit();
					return popCount;
				}
			}
			if (closed)
			{
				host->unlock_quit();
				throw close_exception();
			}
		}
		host->unlock_quit();
		return popCount;
	}
private:
	shared_strand _strand;
	boost::circular_buffer<T> _buffer;
	msg_list<push_pck> _pushWait;
	msg_list<pop_pck> _popWait;
	size_t _halfLength;
	bool _closed;
};

#endif