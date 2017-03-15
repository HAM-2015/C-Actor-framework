#ifndef __ACTOR_SOCKET_H
#define __ACTOR_SOCKET_H

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include "my_actor.h"

struct socket_result
{
	size_t s;///<�ֽ���/������
	int code;///<������
	bool ok;///<�Ƿ�ɹ�
};

class tcp_acceptor;
/*!
@brief tcpͨ��
*/
class tcp_socket
{
	friend tcp_acceptor;
public:
	typedef socket_result result;
private:
	template <typename Handler>
	struct async_read_op
	{
		typedef RM_CREF(Handler) handler_type;

		async_read_op(Handler& handler, tcp_socket& sck, void* buff, size_t currBytes, size_t totalBytes)
			:_handler(std::forward<Handler>(handler)), _sck(sck), _buffer(buff), _currBytes(currBytes), _totalBytes(totalBytes) {}

		void operator()(const boost::system::error_code& ec, size_t s)
		{
			while (_sck._holdRead)
			{
				run_thread::sleep(0);
			}
			tcp_socket::result res;
			_currBytes += s;
			if (ec || _totalBytes == _currBytes)
			{
				do 
				{
					res = { _currBytes, ec.value(), !ec };
#ifndef HAS_ASIO_CANCEL_IO
					if (boost::asio::error::operation_aborted == res.code && _sck._socket.is_open())
					{
						if (_totalBytes == _currBytes)
						{
							res.ok = true;
							res.code = 0;
						}
						else if (!_sck._cancelRead)
						{
							break;
						}
					}
#endif
					DEBUG_OPERATION(_sck._reading = false);
					_handler(res);
					return;
				} while (0);
			}
			try
			{
				do
				{
#ifdef HAS_ASIO_CANCEL_IO
					if (_sck._cancelRead)
					{
						res = { _currBytes, ec.value(), !ec };
						break;
					}
#endif
					_sck._holdRead = true;
					BREAK_OF_SCOPE_EXEC(_sck._holdRead = false);
					_sck._socket.async_read_some(boost::asio::buffer((char*)_buffer + _currBytes, _totalBytes - _currBytes), std::move(*this));
					if (_sck._cancelRead)
					{
						_sck.cancel_read();
					}
					return;
				} while (0);
			}
			catch (const boost::system::system_error& se)
			{
				res = { _currBytes, se.code().value(), !se.code() };
			}
			DEBUG_OPERATION(_sck._reading = false);
			_handler(res);
		}

#ifdef HAS_ASIO_HANDLER_IS_TRIED
		friend bool asio_handler_is_tried(async_read_op*)
		{
			return true;
		}
#endif

		handler_type _handler;
		tcp_socket& _sck;
		void* const _buffer;
		size_t _currBytes;
		const size_t _totalBytes;
		COPY_CONSTRUCT5(async_read_op, _handler, _sck, _buffer, _currBytes, _totalBytes);
	};

	template <typename Handler>
	struct async_write_op
	{
		typedef RM_CREF(Handler) handler_type;

		async_write_op(Handler& handler, tcp_socket& sck, const void* buff, size_t currBytes, size_t totalBytes)
			:_handler(std::forward<Handler>(handler)), _sck(sck), _buffer(buff), _currBytes(currBytes), _totalBytes(totalBytes) {}

		void operator()(const boost::system::error_code& ec, size_t s)
		{
			while (_sck._holdWrite)
			{
				run_thread::sleep(0);
			}
			tcp_socket::result res;
			_currBytes += s;
			if (ec || _totalBytes == _currBytes)
			{
				do
				{
					res = { _currBytes, ec.value(), !ec };
#ifndef HAS_ASIO_CANCEL_IO
					if (boost::asio::error::operation_aborted == res.code && _sck._socket.is_open())
					{
						if (_totalBytes == _currBytes)
						{
							res.ok = true;
							res.code = 0;
						}
						else if (!_sck._cancelWrite)
						{
							break;
						}
					}
#endif
					DEBUG_OPERATION(_sck._writing = false);
					_handler(res);
					return;
				} while (0);
			}
			try
			{
				do
				{
#ifdef HAS_ASIO_CANCEL_IO
					if (_sck._cancelWrite)
					{
						res = { _currBytes, ec.value(), !ec };
						break;
					}
#endif
					_sck._holdWrite = true;
					BREAK_OF_SCOPE_EXEC(_sck._holdWrite = false);
					_sck._socket.async_write_some(boost::asio::buffer((const char*)_buffer + _currBytes, _totalBytes - _currBytes), std::move(*this));
					if (_sck._cancelWrite)
					{
						_sck.cancel_write();
					}
					return;
				} while (0);
			}
			catch (const boost::system::system_error& se)
			{
				res = { _currBytes, se.code().value(), !se.code() };
			}
			DEBUG_OPERATION(_sck._writing = false);
			_handler(res);
		}

#ifdef HAS_ASIO_HANDLER_IS_TRIED
		friend bool asio_handler_is_tried(async_write_op*)
		{
			return true;
		}
#endif

		handler_type _handler;
		tcp_socket& _sck;
		const void* const _buffer;
		size_t _currBytes;
		const size_t _totalBytes;
		COPY_CONSTRUCT5(async_write_op, _handler, _sck, _buffer, _currBytes, _totalBytes);
	};

#ifdef HAS_ASIO_SEND_FILE
#ifdef __linux__
	template <typename Handler>
	struct async_send_file_op
	{
		typedef RM_CREF(Handler) handler_type;

		async_send_file_op(Handler& handler, tcp_socket& sck, size_t currBytes)
			:_handler(std::forward<Handler>(handler)), _sck(sck), _currBytes(currBytes) {}

		void operator()(const boost::system::error_code& ec, size_t s)
		{
			while (_sck._holdWrite)
			{
				run_thread::sleep(0);
			}
			tcp_socket::result res;
			_currBytes += s;
			_sck._sendFileState.count -= s;
			if (ec || !_sck._sendFileState.count || !s)
			{
				do 
				{
					res = { _currBytes, ec.value(), !ec };
#ifndef HAS_ASIO_CANCEL_IO
					if (boost::asio::error::operation_aborted == res.code && _sck._socket.is_open())
					{
						if (!_sck._sendFileState.count)
						{
							res.ok = true;
							res.code = 0;
						}
						else if (!_sck._cancelWrite)
						{
							unsigned long long tOff = _sck._sendFileState.offset ? *_sck._sendFileState.offset + _currBytes : _currBytes;
							tcp_socket::result tRes = _sck.try_send_file_same(_sck._sendFileState.fd, &tOff, _sck._sendFileState.count);
							if (tRes.s != _sck._sendFileState.count)
							{
								_currBytes += tRes.s;
								_sck._sendFileState.count -= tRes.s;
								break;
							}
							res.ok = tRes.ok;
							res.code = tRes.code;
							res.s += tRes.s;
						}
					}
#endif
					_sck._sendFileState.offset = NULL;
					_sck._sendFileState.count = 0;
					_sck._sendFileState.fd = 0;
					DEBUG_OPERATION(_sck._writing = false);
					_handler(res);
					return;
				} while (0);
			}
			try
			{
				do
				{
#ifdef HAS_ASIO_CANCEL_IO
					if (_sck._cancelWrite)
					{
						res = { _currBytes, ec.value(), !ec };
						break;
					}
#endif
					_sck._holdWrite = true;
					BREAK_OF_SCOPE_EXEC(_sck._holdWrite = false);
					_sck._socket.async_write_some(boost::asio::buffer((const char*)&_sck._sendFileState, -1), std::move(*this));
					if (_sck._cancelWrite)
					{
						_sck.cancel_write();
					}
					return;
				} while (0);
			}
			catch (const boost::system::system_error& se)
			{
				res = { _currBytes, se.code().value(), !se.code() };
			}
			_sck._sendFileState.offset = NULL;
			_sck._sendFileState.count = 0;
			_sck._sendFileState.fd = 0;
			DEBUG_OPERATION(_sck._writing = false);
			_handler(res);
		}

#ifdef HAS_ASIO_HANDLER_IS_TRIED
		friend bool asio_handler_is_tried(async_send_file_op*)
		{
			return true;
		}
#endif

		handler_type _handler;
		tcp_socket& _sck;
		size_t _currBytes;
		COPY_CONSTRUCT3(async_send_file_op, _handler, _sck, _currBytes);
	};
#endif
#endif

public:
	tcp_socket(io_engine& ios);
	~tcp_socket();
public:
	/*!
	@brief ����no_delay����
	*/
	result no_delay();

	/*!
	@brief linux���Ż��첽���أ���������ݣ���async_xxx������ֱ�ӻص���
	*/
	void pre_option();

	/*!
	@brief
	*/
	bool is_pre_option();

	/*!
	@brief ���ض˿�IP
	*/
	std::string local_endpoint(unsigned short& port);

	/*!
	@brief Զ�˶˿�IP
	*/
	std::string remote_endpoint(unsigned short& port);

	/*!
	@brief ����һ��Զ��Ŀ��
	*/
	static boost::asio::ip::tcp::endpoint make_endpoint(const char* remoteIp, unsigned short remotePort);

	/*!
	@brief �ͻ���ģʽ������Զ�˷�����
	*/
	result connect(my_actor* host, const boost::asio::ip::tcp::endpoint& remoteEndpoint);
	result connect(my_actor* host, const char* remoteIp, unsigned short remotePort);

	/*!
	@brief ȡ����ǰ�����첽����
	*/
	result cancel();

	/*!
	@brief ȡ����ǰ�첽������
	*/
	result cancel_read();

	/*!
	@brief ȡ����ǰ�첽д����
	*/
	result cancel_write();

	/*!
	@brief ���������ڶ�ȡ���ݣ�ֱ������
	*/
	result read(my_actor* host, void* buff, size_t length);

	/*!
	@brief ���������ڶ�ȡ���ݣ��ж��ٶ�����
	*/
	result read_some(my_actor* host, void* buff, size_t length);

	/*!
	@brief ������ȫ�����ͳ�ȥ
	*/
	result write(my_actor* host, const void* buff, size_t length);

	/*!
	@brief �����ݷ��ͳ�ȥ���ܷ������Ƕ���
	*/
	result write_some(my_actor* host, const void* buff, size_t length);

	/*!
	@brief ��msʱ�䷶Χ�ڣ��ͻ���ģʽ������Զ�˷�����
	*/
	result timed_connect(my_actor* host, int ms, const boost::asio::ip::tcp::endpoint& remoteEndpoint);
	result timed_connect(my_actor* host, int ms, const char* remoteIp, unsigned short remotePort);

	/*!
	@brief ��msʱ�䷶Χ�ڣ����������ڶ�ȡ���ݣ�ֱ������
	*/
	result timed_read(my_actor* host, int ms, void* buff, size_t length);

	/*!
	@brief ��msʱ�䷶Χ�ڣ����������ڶ�ȡ���ݣ��ж��ٶ�����
	*/
	result timed_read_some(my_actor* host, int ms, void* buff, size_t length);

	/*!
	@brief ��msʱ�䷶Χ�ڣ�������ȫ�����ͳ�ȥ
	*/
	result timed_write(my_actor* host, int ms, const void* buff, size_t length);

	/*!
	@brief ��msʱ�䷶Χ�ڣ������ݷ��ͳ�ȥ���ܷ������Ƕ���
	*/
	result timed_write_some(my_actor* host, int ms, const void* buff, size_t length);

	/*!
	@brief �ر�socket
	*/
	result close();

	/*!
	@brief �Ƿ�ûclose
	*/
	bool is_open();

	/*!
	@brief ����
	*/
	void swap(tcp_socket& other);

	/*!
	@brief ��ԭʼ�������
	*/
	result assign(boost::asio::detail::socket_type sckFd);
	result assign_v6(boost::asio::detail::socket_type sckFd);
	
	/*!
	@brief ��ȡԭʼ���
	*/
	boost::asio::detail::socket_type native();

	/*!
	@brief �첽ģʽ�£��ͻ���ģʽ������Զ�˷�����
	*/
	template <typename Handler>
	bool async_connect(const boost::asio::ip::tcp::endpoint& remoteEndpoint, Handler&& handler)
	{
		assert(!_writing);
		DEBUG_OPERATION(_writing = true);
		result res;
		try
		{
			_socket.async_connect(remoteEndpoint, std::bind([this](Handler& handler, const boost::system::error_code& ec)
			{
				if (!ec)
				{
					set_internal_non_blocking();
				}
				result res = { 0, ec.value(), !ec };
				DEBUG_OPERATION(_writing = false);
				handler(res);
			}, std::forward<Handler>(handler), __1));
			return false;
		}
		catch (const boost::system::system_error& se)
		{
			res = { 0, se.code().value(), !se.code() };
		}
#if (_DEBUG || DEBUG)
		return check_immed_callback(std::forward<Handler>(handler), res, _writing);
#else
		return check_immed_callback(std::forward<Handler>(handler), res);
#endif
	}

	template <typename Handler>
	bool async_connect(const char* remoteIp, unsigned short remotePort, Handler&& handler)
	{
		return async_connect(make_endpoint(remoteIp, remotePort), std::forward<Handler>(handler));
	}

	/*!
	@brief �첽ģʽ�£����������ڶ�ȡ���ݣ�ֱ������
	*/
	template <typename Handler>
	bool async_read(void* buff, size_t length, Handler&& handler)
	{
		assert(!_reading);
		DEBUG_OPERATION(_reading = true);
		result res;
		size_t trySize = 0;
		_cancelRead = false;
#ifdef ENABLE_ASIO_PRE_OP
		if (is_pre_option())
		{
			res = try_read_same(buff, length);
			if (!res.ok)
			{
				if (!try_again(res))
				{
					DEBUG_OPERATION(_reading = false);
					handler(res);
					return true;
				}
			}
			else if (res.s == length)
			{
				DEBUG_OPERATION(_reading = false);
				handler(res);
				return true;
			}
			else
			{
				trySize = res.s;
			}
		}
#endif
		try
		{
			if (buff && length)
			{
#ifdef HAS_ASIO_HANDLER_IS_TRIED
#ifdef ENABLE_ASIO_PRE_OP
				if (is_pre_option())
				{
					_socket.async_read_some(boost::asio::buffer((char*)buff + trySize, length - trySize),
						async_read_op<Handler>(handler, *this, buff, trySize, length));
				}
				else
#endif
				{
					_socket.async_read_some(boost::asio::buffer((char*)buff + trySize, length - trySize),
						wrap_no_tried(async_read_op<Handler>(handler, *this, buff, trySize, length)));
				}
#else
				_socket.async_read_some(boost::asio::buffer((char*)buff + trySize, length - trySize),
					async_read_op<Handler>(handler, *this, buff, trySize, length));
#endif
			}
			else
			{
#if defined(ENABLE_ASIO_PRE_OP) && defined(HAS_ASIO_HANDLER_IS_TRIED)
				if (is_pre_option())
				{
					_socket.async_read_some(boost::asio::null_buffers(), wrap_tried(std::bind([this, buff, length](Handler& handler, const boost::system::error_code& ec, size_t s)
					{
						async_read_some_handler(buff, length, std::move(handler), ec, s);
					}, std::forward<Handler>(handler), __1, __2)));
				}
				else
#endif
				{
					_socket.async_read_some(boost::asio::null_buffers(), std::bind([this, buff, length](Handler& handler, const boost::system::error_code& ec, size_t s)
					{
						async_read_some_handler(buff, length, std::move(handler), ec, s);
					}, std::forward<Handler>(handler), __1, __2));
				}
			}
			return false;
		}
		catch (const boost::system::system_error& se)
		{
			res = { 0, se.code().value(), !se.code() };
		}
#if (_DEBUG || DEBUG)
		return check_immed_callback(std::forward<Handler>(handler), res, _reading);
#else
		return check_immed_callback(std::forward<Handler>(handler), res);
#endif
	}

	/*!
	@brief �첽ģʽ�£�����ȡ���ݣ��ж��ٶ�����
	*/
	template <typename Handler>
	bool async_read_some(void* buff, size_t length, Handler&& handler)
	{
		assert(!_reading);
		DEBUG_OPERATION(_reading = true);
		result res;
		_cancelRead = false;
#ifdef ENABLE_ASIO_PRE_OP
		if (is_pre_option())
		{
			res = try_read_same(buff, length);
			if (res.ok || !try_again(res))
			{
				DEBUG_OPERATION(_reading = false);
				handler(res);
				return true;
			}
		}
#endif
		try
		{
#if defined(ENABLE_ASIO_PRE_OP) && defined(HAS_ASIO_HANDLER_IS_TRIED)
			if (is_pre_option())
			{
				if (buff && length)
				{
					_socket.async_read_some(boost::asio::buffer(buff, length), wrap_tried(std::bind([this, buff, length](Handler& handler, const boost::system::error_code& ec, size_t s)
					{
						async_read_some_handler(buff, length, std::move(handler), ec, s);
					}, std::forward<Handler>(handler), __1, __2)));
				}
				else
				{
					_socket.async_read_some(boost::asio::null_buffers(), wrap_tried(std::bind([this, buff, length](Handler& handler, const boost::system::error_code& ec, size_t s)
					{
						async_read_some_handler(buff, length, std::move(handler), ec, s);
					}, std::forward<Handler>(handler), __1, __2)));
				}
			}
			else
#endif
			{
				if (buff && length)
				{
					_socket.async_read_some(boost::asio::buffer(buff, length), std::bind([this, buff, length](Handler& handler, const boost::system::error_code& ec, size_t s)
					{
						async_read_some_handler(buff, length, std::move(handler), ec, s);
					}, std::forward<Handler>(handler), __1, __2));
				}
				else
				{
					_socket.async_read_some(boost::asio::null_buffers(), std::bind([this, buff, length](Handler& handler, const boost::system::error_code& ec, size_t s)
					{
						async_read_some_handler(buff, length, std::move(handler), ec, s);
					}, std::forward<Handler>(handler), __1, __2));
				}
			}
			return false;
		}
		catch (const boost::system::system_error& se)
		{
			res = { 0, se.code().value(), !se.code() };
		}
#if (_DEBUG || DEBUG)
		return check_immed_callback(std::forward<Handler>(handler), res, _reading);
#else
		return check_immed_callback(std::forward<Handler>(handler), res);
#endif
	}

	/*!
	@brief �첽ģʽ�£�������ȫ�����ͳ�ȥ
	*/
	template <typename Handler>
	bool async_write(const void* buff, size_t length, Handler&& handler)
	{
		assert(!_writing);
		DEBUG_OPERATION(_writing = true);
		result res;
		size_t trySize = 0;
		_cancelWrite = false;
#ifdef ENABLE_ASIO_PRE_OP
		if (is_pre_option())
		{
			res = try_write_same(buff, length);
			if (!res.ok)
			{
				if (!try_again(res))
				{
					DEBUG_OPERATION(_writing = false);
					handler(res);
					return true;
				}
			}
			else if (res.s == length)
			{
				DEBUG_OPERATION(_writing = false);
				handler(res);
				return true;
			}
			else
			{
				trySize = res.s;
			}
		}
#endif
		try
		{
			if (buff && length)
			{
#ifdef HAS_ASIO_HANDLER_IS_TRIED
#ifdef ENABLE_ASIO_PRE_OP
				if (is_pre_option())
				{
					_socket.async_write_some(boost::asio::buffer((const char*)buff + trySize, length - trySize),
						async_write_op<Handler>(handler, *this, buff, trySize, length));
				}
				else
#endif
				{
					_socket.async_write_some(boost::asio::buffer((const char*)buff + trySize, length - trySize),
						wrap_no_tried(async_write_op<Handler>(handler, *this, buff, trySize, length)));
				}
#else
				_socket.async_write_some(boost::asio::buffer((const char*)buff + trySize, length - trySize),
					async_write_op<Handler>(handler, *this, buff, trySize, length));
#endif
			}
			else
			{
#if defined(ENABLE_ASIO_PRE_OP) && defined(HAS_ASIO_HANDLER_IS_TRIED)
				if (is_pre_option())
				{
					_socket.async_write_some(boost::asio::null_buffers(), wrap_tried(std::bind([this, buff, length](Handler& handler, const boost::system::error_code& ec, size_t s)
					{
						async_write_some_handler(buff, length, std::move(handler), ec, s);
					}, std::forward<Handler>(handler), __1, __2)));
				}
				else
#endif
				{
					_socket.async_write_some(boost::asio::null_buffers(), std::bind([this, buff, length](Handler& handler, const boost::system::error_code& ec, size_t s)
					{
						async_write_some_handler(buff, length, std::move(handler), ec, s);
					}, std::forward<Handler>(handler), __1, __2));
				}
			}
			return false;
		}
		catch (const boost::system::system_error& se)
		{
			res = { 0, se.code().value(), !se.code() };
		}
#if (_DEBUG || DEBUG)
		return check_immed_callback(std::forward<Handler>(handler), res, _writing);
#else
		return check_immed_callback(std::forward<Handler>(handler), res);
#endif
	}

	/*!
	@brief �첽ģʽ�£������ݷ��ͳ�ȥ���ܷ������Ƕ���
	*/
	template <typename Handler>
	bool async_write_some(const void* buff, size_t length, Handler&& handler)
	{
		assert(!_writing);
		DEBUG_OPERATION(_writing = true);
		result res;
		_cancelWrite = false;
#ifdef ENABLE_ASIO_PRE_OP
		if (is_pre_option())
		{
			res = try_write_same(buff, length);
			if (res.ok || !try_again(res))
			{
				DEBUG_OPERATION(_writing = false);
				handler(res);
				return true;
			}
		}
#endif
		try
		{
#if defined(ENABLE_ASIO_PRE_OP) && defined(HAS_ASIO_HANDLER_IS_TRIED)
			if (is_pre_option())
			{
				if (buff && length)
				{
					_socket.async_write_some(boost::asio::buffer(buff, length), wrap_tried(std::bind([this, buff, length](Handler& handler, const boost::system::error_code& ec, size_t s)
					{
						async_write_some_handler(buff, length, std::move(handler), ec, s);
					}, std::forward<Handler>(handler), __1, __2)));
				}
				else
				{
					_socket.async_write_some(boost::asio::null_buffers(), wrap_tried(std::bind([this, buff, length](Handler& handler, const boost::system::error_code& ec, size_t s)
					{
						async_write_some_handler(buff, length, std::move(handler), ec, s);
					}, std::forward<Handler>(handler), __1, __2)));
				}
			}
			else
#endif
			{
				if (buff && length)
				{
					_socket.async_write_some(boost::asio::buffer(buff, length), std::bind([this, buff, length](Handler& handler, const boost::system::error_code& ec, size_t s)
					{
						async_write_some_handler(buff, length, std::move(handler), ec, s);
					}, std::forward<Handler>(handler), __1, __2));
				}
				else
				{
					_socket.async_write_some(boost::asio::null_buffers(), std::bind([this, buff, length](Handler& handler, const boost::system::error_code& ec, size_t s)
					{
						async_write_some_handler(buff, length, std::move(handler), ec, s);
					}, std::forward<Handler>(handler), __1, __2));
				}
			}
			return false;
		}
		catch (const boost::system::system_error& se)
		{
			res = { 0, se.code().value(), !se.code() };
		}
#if (_DEBUG || DEBUG)
		return check_immed_callback(std::forward<Handler>(handler), res, _writing);
#else
		return check_immed_callback(std::forward<Handler>(handler), res);
#endif
	}

#ifdef HAS_ASIO_SEND_FILE
	/*!
	@brief ����һ���ļ�
	*/
#ifdef __linux__
	template <typename Handler>
	bool async_send_file(int fd, unsigned long long* offset, size_t length, Handler&& handler)
	{
		assert(!_writing);
		DEBUG_OPERATION(_writing = true);
		result res;
		assert(!_sendFileState.fd);
		_cancelWrite = false;
		res = try_send_file_same(fd, offset, length);
		try
		{
			if (res.ok && res.s && res.s != length)
			{
				_sendFileState.offset = offset;
				_sendFileState.count = length - res.s;
				_sendFileState.fd = fd;
				_socket.async_write_some(boost::asio::buffer((const char*)&_sendFileState, -1), async_send_file_op<Handler>(handler, *this, res.s));
				return false;
			}
		}
		catch (const boost::system::system_error& se)
		{
			res = { 0, se.code().value(), !se.code() };
		}
#if (_DEBUG || DEBUG)
		return check_immed_callback(std::forward<Handler>(handler), res, _writing);
#else
		return check_immed_callback(std::forward<Handler>(handler), res);
#endif
	}
#elif WIN32
	template <typename Handler>
	bool async_send_file(HANDLE hFile, unsigned long long* offset, size_t length, Handler&& handler)
	{
		assert(!_writing);
		DEBUG_OPERATION(_writing = true);
		result res;
		_cancelWrite = false;
		if (!init_send_file(hFile, offset, length) || 0 == length)
		{
			res = { 0, length ? ::WSAGetLastError() : 0, 0 == length };
		}
		else
		{
			try
			{
				boost::asio::detail::send_file_pck pck = { hFile, (DWORD)length };
				_socket.async_write_some(boost::asio::buffer((const char*)&pck, -1), std::bind([this, hFile, offset, length](Handler& handler, const boost::system::error_code& ec, size_t s)
				{
					async_send_file_handler(0, hFile, offset, length, std::move(handler), ec, s);
				}, std::forward<Handler>(handler), __1, __2));
				return false;
			}
			catch (const boost::system::system_error& se)
			{
				res = { 0, se.code().value(), !se.code() };
			}
		}
#if (_DEBUG || DEBUG)
		return check_immed_callback(std::forward<Handler>(handler), res, _writing);
#else
		return check_immed_callback(std::forward<Handler>(handler), res);
#endif
	}
#endif
#endif

	/*!
	@brief ����������д������
	*/
	result try_write_same(const void* buff, size_t length);

	/*!
	@brief ���������Զ�ȡ����
	*/
	result try_read_same(void* buff, size_t length);

	/*!
	@brief ����������һ��д��������
	@param lastBytes����Ϊ NULL ʱ����д������һ������ʵ��д�˶����ֽ�
	@return lastBytes ��Ϊ NULL ʱ����ʵ��д���˶��ٸ��������ݣ�Ϊ NULL ʱ�ܹ�д�˶����ֽ�����
	*/
	result try_mwrite_same(const void* const* buffs, const size_t* lengths, size_t count, size_t* lastBytes = NULL);

	/*!
	@brief ����������һ�ζ�ȡ�������
	@param lastBytes����Ϊ NULL ʱ���ض�ȡ�����һ������ʵ��д�˶����ֽ�
	@return lastBytes ��Ϊ NULL ʱ����ʵ�ʶ�ȡ�˶��ٸ��������ݣ�Ϊ NULL ʱ�ܹ���ȡ�˶����ֽ�����
	*/
	result try_mread_same(void* const* buffs, const size_t* lengths, size_t count, size_t* lastBytes = NULL);

	/*!
	@brief try_io����ʧ���Ƿ�����ΪEAGAIN
	*/
	static bool try_again(const result& res);
private:
	template <typename Handler>
	void async_read_some_handler(void* buff, size_t length, Handler&& handler, const boost::system::error_code& ec, size_t s)
	{
		result res = { s, ec.value(), !ec };
#ifndef HAS_ASIO_CANCEL_IO
		while (_holdRead)
		{
			run_thread::sleep(0);
		}
		if (boost::asio::error::operation_aborted == res.code && _socket.is_open())
		{
			if (res.s)
			{
				res.ok = true;
				res.code = 0;
			}
			else if (!_cancelRead)
			{
				try
				{
					_holdRead = true;
					BREAK_OF_SCOPE_EXEC(_holdRead = false);
					if (buff && length)
					{
						_socket.async_read_some(boost::asio::buffer(buff, length), std::bind([this, buff, length](Handler& handler, const boost::system::error_code& ec, size_t s)
						{
							async_read_some_handler(buff, length, std::move(handler), ec, s);
						}, std::forward<Handler>(handler), __1, __2));
					}
					else
					{
						_socket.async_read_some(boost::asio::null_buffers(), std::bind([this, buff, length](Handler& handler, const boost::system::error_code& ec, size_t s)
						{
							async_read_some_handler(buff, length, std::move(handler), ec, s);
						}, std::forward<Handler>(handler), __1, __2));
					}
					if (_cancelRead)
					{
						cancel_read();
					}
					return;
				}
				catch (const boost::system::system_error& se)
				{
					res = { 0, se.code().value(), !se.code() };
				}
			}
		}
#endif
		DEBUG_OPERATION(_reading = false);
		handler(res);
	}

	template <typename Handler>
	void async_write_some_handler(const void* buff, size_t length, Handler&& handler, const boost::system::error_code& ec, size_t s)
	{
		result res = { s, ec.value(), !ec };
#ifndef HAS_ASIO_CANCEL_IO
		while (_holdWrite)
		{
			run_thread::sleep(0);
		}
		if (boost::asio::error::operation_aborted == res.code && _socket.is_open())
		{
			if (res.s)
			{
				res.ok = true;
				res.code = 0;
			}
			else if (!_cancelWrite)
			{
				try
				{
					_holdWrite = true;
					BREAK_OF_SCOPE_EXEC(_holdWrite = false);
					if (buff && length)
					{
						_socket.async_write_some(boost::asio::buffer(buff, length), std::bind([this, buff, length](Handler& handler, const boost::system::error_code& ec, size_t s)
						{
							async_write_some_handler(buff, length, std::move(handler), ec, s);
						}, std::forward<Handler>(handler), __1, __2));
					}
					else
					{
						_socket.async_write_some(boost::asio::null_buffers(), std::bind([this, buff, length](Handler& handler, const boost::system::error_code& ec, size_t s)
						{
							async_write_some_handler(buff, length, std::move(handler), ec, s);
						}, std::forward<Handler>(handler), __1, __2));
					}
					if (_cancelWrite)
					{
						cancel_write();
					}
					return;
				}
				catch (const boost::system::system_error& se)
				{
					res = { 0, se.code().value(), !se.code() };
				}
			}
		}
#endif
		DEBUG_OPERATION(_writing = false);
		handler(res);
	}

#ifdef HAS_ASIO_SEND_FILE
#ifdef WIN32
	template <typename Handler>
	void async_send_file_handler(size_t currSize, HANDLE hFile, unsigned long long* offset, size_t length, Handler&& handler, const boost::system::error_code& ec, size_t s)
	{
		if (offset)
		{
			*offset += (unsigned long long)s;
		}
		currSize += s;
		result res = { currSize, ec.value(), !ec };
#ifndef HAS_ASIO_CANCEL_IO
		while (_holdWrite)
		{
			run_thread::sleep(0);
		}
		if (boost::asio::error::operation_aborted == res.code && _socket.is_open())
		{
			if (length == currSize)
			{
				res.ok = true;
				res.code = 0;
			}
			else if (!_cancelWrite)
			{
				length -= currSize;
				unsigned long long tOff = offset ? *offset + currSize : currSize;
				if (init_send_file(hFile, &tOff, length))
				{
					try
					{
						_holdWrite = true;
						BREAK_OF_SCOPE_EXEC(_holdWrite = false);
						boost::asio::detail::send_file_pck pck = { hFile, (DWORD)length };
						_socket.async_write_some(boost::asio::buffer((const char*)&pck, -1), std::bind([this, currSize, hFile, offset, length](Handler& handler, const boost::system::error_code& ec, size_t s)
						{
							async_send_file_handler(currSize, hFile, offset, length, std::move(handler), ec, s);
						}, std::forward<Handler>(handler), __1, __2));
						if (_cancelWrite)
						{
							cancel_write();
						}
						return;
					}
					catch (const boost::system::system_error& se)
					{
						res = { 0, se.code().value(), !se.code() };
					}
				}
			}
		}
#endif
		DEBUG_OPERATION(_writing = false);
		handler(res);
	}
#endif
#endif

#if (_DEBUG || DEBUG)
	template <typename Handler>
	bool check_immed_callback(Handler&& handler, result& res, bool& sign)
	{
#ifdef ENABLE_ASIO_PRE_OP
		if (is_pre_option())
		{
			sign = false;
			handler(res);
			return true;
		}
#endif
		_socket.get_io_service().post(std::bind([&sign](Handler& handler, result& res)
		{
			sign = false;
			handler(res);
		}, std::forward<Handler>(handler), res));
		return false;
	}
#else
	template <typename Handler>
	bool check_immed_callback(Handler&& handler, result& res)
	{
#ifdef ENABLE_ASIO_PRE_OP
		if (is_pre_option())
		{
			handler(res);
			return true;
		}
#endif
		_socket.get_io_service().post(std::bind([](Handler& handler, result& res)
		{
			handler(res);
		}, std::forward<Handler>(handler), res));
		return false;
	}
#endif
private:
#ifdef HAS_ASIO_SEND_FILE
#ifdef __linux__
	result try_send_file_same(int fd, unsigned long long* offset, size_t& length);
#elif WIN32
	bool init_send_file(HANDLE hFile, unsigned long long* offset, size_t& length);
#endif
#endif
	result _try_mwrite_same(const void* const* buffs, const size_t* lengths, size_t count);
	result _try_mread_same(void* const* buffs, const size_t* lengths, size_t count);
	void set_internal_non_blocking();
private:
	boost::asio::ip::tcp::socket _socket;
#ifdef HAS_ASIO_SEND_FILE
#ifdef __linux__
	boost::asio::detail::socket_ops::send_file_pck _sendFileState;
#endif
#endif
	volatile bool _holdRead;
	volatile bool _holdWrite;
	volatile bool _cancelRead;
	volatile bool _cancelWrite;
	bool _nonBlocking;
#ifdef ENABLE_ASIO_PRE_OP
	bool _preOption;
#endif
#if (_DEBUG || DEBUG)
	bool _reading;
	bool _writing;
#endif
	NONE_COPY(tcp_socket);
};

/*!
@brief ������������
*/
class tcp_acceptor
{
public:
	tcp_acceptor(io_engine& ios);
	~tcp_acceptor();
public:
	/*!
	@brief ֻ�����ض�ip���Ӹ÷�����
	*/
	tcp_socket::result open(const char* ip, unsigned short port);
	tcp_socket::result open(const boost::asio::ip::tcp::endpoint& endPoint);

	/*!
	@brief ip v4�´򿪷�����
	*/
	tcp_socket::result open_v4(unsigned short port);

	/*!
	@brief ip v6�´򿪷�����
	*/
	tcp_socket::result open_v6(unsigned short port);

	/*!
	@brief ��ԭʼ�������
	*/
	tcp_socket::result assign(boost::asio::detail::socket_type accFd);
	tcp_socket::result assign_v6(boost::asio::detail::socket_type accFd);

	/*!
	@brief ��ȡԭʼ���
	*/
	boost::asio::detail::socket_type native();

	/*!
	@brief ȡ��һ���첽����
	*/
	tcp_socket::result cancel();

	/*!
	@brief �ر�������
	*/
	tcp_socket::result close();

	/*!
	@brief �Ƿ�ûclose
	*/
	bool is_open();

	/*!
	@brief ����
	*/
	void swap(tcp_acceptor& other);

	/*!
	@brief linux���Ż��첽���أ���������ݣ���async_xxx������ֱ�ӻص���
	*/
	void pre_option();

	/*!
	@brief
	*/
	bool is_pre_option();

	/*!
	@brief ��socket�����ͻ�������
	*/
	tcp_socket::result accept(my_actor* host, tcp_socket& socket);

	/*!
	@brief ��msʱ�䷶Χ�ڣ���socket�����ͻ�������
	*/
	tcp_socket::result timed_accept(my_actor* host, int ms, tcp_socket& socket);

	/*!
	@brief �첽ģʽ�£���socket�����ͻ�������
	*/
	template <typename Handler>
	bool async_accept(tcp_socket& socket, Handler&& handler)
	{
		tcp_socket::result res;
#ifdef ENABLE_ASIO_PRE_OP
		if (is_pre_option())
		{
			res = try_accept(socket);
			if (res.ok || !tcp_socket::try_again(res))
			{
				handler(res);
				return true;
			}
		}
#endif
		try
		{
#if defined(ENABLE_ASIO_PRE_OP) && defined(HAS_ASIO_HANDLER_IS_TRIED)
			if (is_pre_option())
			{
				_acceptor->async_accept(socket._socket, wrap_tried(std::bind([&socket](Handler& handler, const boost::system::error_code& ec)
				{
					if (!ec)
					{
						socket.set_internal_non_blocking();
					}
					tcp_socket::result res = { 0, ec.value(), !ec };
					handler(res);
				}, std::forward<Handler>(handler), __1)));
			}
			else
#endif
			{
				_acceptor->async_accept(socket._socket, std::bind([&socket](Handler& handler, const boost::system::error_code& ec)
				{
					if (!ec)
					{
						socket.set_internal_non_blocking();
					}
					tcp_socket::result res = { 0, ec.value(), !ec };
					handler(res);
				}, std::forward<Handler>(handler), __1));
			}
			return false;
		}
		catch (const boost::system::system_error& se)
		{
			res = { 0, se.code().value(), !se.code() };
		}
#ifdef ENABLE_ASIO_PRE_OP
		if (is_pre_option())
		{
			handler(res);
			return true;
		}
#endif
		_acceptor->get_io_service().post(std::bind([](Handler& handler, tcp_socket::result& res)
		{
			handler(res);
		}, std::forward<Handler>(handler), res));
		return false;
	}
private:
	void set_internal_non_blocking();
	tcp_socket::result try_accept(tcp_socket& socket);
private:
	io_engine* _ios;
	stack_obj<boost::asio::ip::tcp::acceptor> _acceptor;
	bool _nonBlocking;
#ifdef ENABLE_ASIO_PRE_OP
	bool _preOption;
#endif
	NONE_COPY(tcp_acceptor);
};

/*!
@brief udpͨ��
*/
class udp_socket
{
public:
	typedef socket_result result;
public:
	udp_socket(io_engine& ios);
	~udp_socket();
public:
	/*!
	@brief �ر�socket
	*/
	result close();

	/*!
	@brief �Ƿ�ûclose
	*/
	bool is_open();

	/*!
	@brief ȡ�������첽����
	*/
	result cancel();

	/*!
	@brief ȡ��һ���첽���ղ���
	*/
	result cancel_receive();

	/*!
	@brief ȡ��һ���첽���Ͳ���
	*/
	result cancel_send();

	/*!
	@brief ip v4ģʽ��socket
	*/
	result open_v4();

	/*!
	@brief ip v6ģʽ��socket
	*/
	result open_v6();

	/*!
	@brief �󶨱���һ��ip��ĳ���˿ڽ��շ�������
	*/
	result bind(const char* ip, unsigned short port);

	/*!
	@brief ��ip v4��ĳ���˿ڽ��շ�������
	*/
	result bind_v4(unsigned short port);

	/*!
	@brief ��ip v6��ĳ���˿ڽ��շ�������
	*/
	result bind_v6(unsigned short port);

	/*!
	@brief �򿪲���ip v4��ĳ���˿ڽ��շ�������
	*/
	result open_bind_v4(unsigned short port);

	/*!
	@brief �򿪲���ip v6��ĳ���˿ڽ��շ�������
	*/
	result open_bind_v6(unsigned short port);

	/*!
	@brief ����
	*/
	void swap(udp_socket& other);
	
	/*!
	@brief ��ԭʼ�������
	*/
	result assign(boost::asio::detail::socket_type sckFd);
	result assign_v6(boost::asio::detail::socket_type sckFd);

	/*!
	@brief ��ȡԭʼ���
	*/
	boost::asio::detail::socket_type native();

	/*!
	@brief linux���Ż��첽���أ���������ݣ���async_xxx������ֱ�ӻص���
	*/
	void pre_option();

	/*!
	@brief 
	*/
	bool is_pre_option();

	/*!
	@brief �趨һ��Զ�̶˿���ΪĬ�Ϸ��ͽ���Ŀ��
	*/
	result connect(const char* remoteIp, unsigned short remotePort);
	result connect(const boost::asio::ip::udp::endpoint& remoteEndpoint);

	/*!
	@brief �������ݵ�ָ��Ŀ��
	*/
	result send_to(my_actor* host, const boost::asio::ip::udp::endpoint& remoteEndpoint, const void* buff, size_t length, int flags = 0);

	/*!
	@brief �������ݵ�ָ��Ŀ��
	*/
	result send_to(my_actor* host, const char* remoteIp, unsigned short remotePort, const void* buff, size_t length, int flags = 0);

	/*!
	@brief ��connect�ɹ��󣬷������ݵ�Ĭ��Ŀ��
	*/
	result send(my_actor* host, const void* buff, size_t length, int flags = 0);

	/*!
	@brief ����Զ�˷��͵����ݵ�������������¼��Զ�˵�ַ
	*/
	result receive_from(my_actor* host, boost::asio::ip::udp::endpoint& remoteEndpoint, void* buff, size_t length, int flags = 0);
	result receive_from(my_actor* host, void* buff, size_t length, int flags = 0);

	/*!
	@brief ����Զ�˷��͵����ݵ�������
	*/
	result receive(my_actor* host, void* buff, size_t length, int flags = 0);

	/*!
	@brief ��msʱ�䷶Χ�ڣ��������ݵ�ָ��Ŀ�꣨����ϵͳ�������˻ᵼ�·���������
	*/
	result timed_send_to(my_actor* host, int ms, const boost::asio::ip::udp::endpoint& remoteEndpoint, const void* buff, size_t length, int flags = 0);

	/*!
	@brief ��msʱ�䷶Χ�ڣ��������ݵ�ָ��Ŀ�꣨����ϵͳ�������˻ᵼ�·���������
	*/
	result timed_send_to(my_actor* host, int ms, const char* remoteIp, unsigned short remotePort, const void* buff, size_t length, int flags = 0);

	/*!
	@brief connect�ɹ�����msʱ�䷶Χ�ڣ��������ݵ�Ĭ��Ŀ�꣨����ϵͳ�������˻ᵼ�·���������
	*/
	result timed_send(my_actor* host, int ms, const void* buff, size_t length, int flags = 0);

	/*!
	@brief ��msʱ�䷶Χ�ڣ�����Զ�˷��͵����ݵ�������������¼��Զ�˵�ַ
	*/
	result timed_receive_from(my_actor* host, boost::asio::ip::udp::endpoint& remoteEndpoint, int ms, void* buff, size_t length, int flags = 0);
	result timed_receive_from(my_actor* host, int ms, void* buff, size_t length, int flags = 0);

	/*!
	@brief ��msʱ�䷶Χ�ڣ�����Զ�˷��͵����ݵ�������
	*/
	result timed_receive(my_actor* host, int ms, void* buff, size_t length, int flags = 0);

	/*!
	@brief ����һ��Զ��Ŀ��
	*/
	static boost::asio::ip::udp::endpoint make_endpoint(const char* remoteIp, unsigned short remotePort);

	/*!
	@brief receive_from��ɺ��ȡԶ�˵�ַ
	*/
	const boost::asio::ip::udp::endpoint& last_remote_sender_endpoint();

	/*!
	@brief ����Զ�˵�ַ
	*/
	void reset_remote_sender_endpoint();

	/*!
	@brief �첽ģʽ�£��������ݵ�ָ��Ŀ��
	*/
	template <typename Handler>
	bool async_send_to(const boost::asio::ip::udp::endpoint& remoteEndpoint, const void* buff, size_t length, Handler&& handler, int flags = 0)
	{
		result res;
#ifndef HAS_ASIO_CANCEL_IO
		_cancelSend = false;
#endif
#ifdef ENABLE_ASIO_PRE_OP
		if (is_pre_option())
		{
			res = try_send_to(remoteEndpoint, buff, length, flags);
			if (res.ok || !try_again(res))
			{
				handler(res);
				return true;
			}
		}
#endif
		try
		{
#if defined(ENABLE_ASIO_PRE_OP) && defined(HAS_ASIO_HANDLER_IS_TRIED)
			if (is_pre_option())
			{
				if (buff && length)
				{
					_socket.async_send_to(boost::asio::buffer(buff, length), remoteEndpoint, flags, wrap_tried(std::bind([this, remoteEndpoint, buff, length, flags](Handler& handler, const boost::system::error_code& ec, size_t s)
					{
						async_send_to_handler(remoteEndpoint, buff, length, std::move(handler), flags, ec, s);
					}, std::forward<Handler>(handler), __1, __2)));
				}
				else
				{
					_socket.async_send_to(boost::asio::null_buffers(), remoteEndpoint, flags, wrap_tried(std::bind([this, remoteEndpoint, buff, length, flags](Handler& handler, const boost::system::error_code& ec, size_t s)
					{
						async_send_to_handler(remoteEndpoint, buff, length, std::move(handler), flags, ec, s);
					}, std::forward<Handler>(handler), __1, __2)));
				}
			}
			else
#endif
			{
				if (buff && length)
				{
					_socket.async_send_to(boost::asio::buffer(buff, length), remoteEndpoint, flags, std::bind([this, remoteEndpoint, buff, length, flags](Handler& handler, const boost::system::error_code& ec, size_t s)
					{
						async_send_to_handler(remoteEndpoint, buff, length, std::move(handler), flags, ec, s);
					}, std::forward<Handler>(handler), __1, __2));
				}
				else
				{
					_socket.async_send_to(boost::asio::null_buffers(), remoteEndpoint, flags, std::bind([this, remoteEndpoint, buff, length, flags](Handler& handler, const boost::system::error_code& ec, size_t s)
					{
						async_send_to_handler(remoteEndpoint, buff, length, std::move(handler), flags, ec, s);
					}, std::forward<Handler>(handler), __1, __2));
				}
			}
			return false;
		}
		catch (const boost::system::system_error& se)
		{
			res = { 0, se.code().value(), !se.code() };
		}
		return check_immed_callback(std::forward<Handler>(handler), res);
	}

	/*!
	@brief �첽ģʽ�£��������ݵ�ָ��Ŀ��
	*/
	template <typename Handler>
	bool async_send_to(const char* remoteIp, unsigned short remotePort, const void* buff, size_t length, Handler&& handler, int flags = 0)
	{
		return async_send_to(boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(remoteIp), remotePort), buff, length, std::forward<Handler>(handler), flags);
	}

	/*!
	@brief �첽ģʽ�£��������ݵ�Ĭ��Ŀ��(connect�ɹ���)
	*/
	template <typename Handler>
	bool async_send(const void* buff, size_t length, Handler&& handler, int flags = 0)
	{
		result res;
#ifndef HAS_ASIO_CANCEL_IO
		_cancelSend = false;
#endif
#ifdef ENABLE_ASIO_PRE_OP
		if (is_pre_option())
		{
			res = try_send(buff, length, flags);
			if (res.ok || !try_again(res))
			{
				handler(res);
				return true;
			}
		}
#endif
		try
		{
#if defined(ENABLE_ASIO_PRE_OP) && defined(HAS_ASIO_HANDLER_IS_TRIED)
			if (is_pre_option())
			{
				if (buff && length)
				{
					_socket.async_send(boost::asio::buffer(buff, length), flags, wrap_tried(std::bind([this, buff, length, flags](Handler& handler, const boost::system::error_code& ec, size_t s)
					{
						async_send_handler(buff, length, std::move(handler), flags, ec, s);
					}, std::forward<Handler>(handler), __1, __2)));
				}
				else
				{
					_socket.async_send(boost::asio::null_buffers(), flags, wrap_tried(std::bind([this, buff, length, flags](Handler& handler, const boost::system::error_code& ec, size_t s)
					{
						async_send_handler(buff, length, std::move(handler), flags, ec, s);
					}, std::forward<Handler>(handler), __1, __2)));
				}
			}
			else
#endif
			{
				if (buff && length)
				{
					_socket.async_send(boost::asio::buffer(buff, length), flags, std::bind([this, buff, length, flags](Handler& handler, const boost::system::error_code& ec, size_t s)
					{
						async_send_handler(buff, length, std::move(handler), flags, ec, s);
					}, std::forward<Handler>(handler), __1, __2));
				}
				else
				{
					_socket.async_send(boost::asio::null_buffers(), flags, std::bind([this, buff, length, flags](Handler& handler, const boost::system::error_code& ec, size_t s)
					{
						async_send_handler(buff, length, std::move(handler), flags, ec, s);
					}, std::forward<Handler>(handler), __1, __2));
				}
			}
			return false;
		}
		catch (const boost::system::system_error& se)
		{
			res = { 0, se.code().value(), !se.code() };
		}
		return check_immed_callback(std::forward<Handler>(handler), res);
	}

	/*!
	@brief �첽ģʽ�£�����Զ�˷��͵����ݵ�������������¼��Զ�˵�ַ
	*/
	template <typename Handler>
	bool async_receive_from(boost::asio::ip::udp::endpoint& remoteEndpoint, void* buff, size_t length, Handler&& handler, int flags = 0)
	{
		result res;
#ifndef HAS_ASIO_CANCEL_IO
		_cancelRecv = false;
#endif
#ifdef ENABLE_ASIO_PRE_OP
		if (is_pre_option())
		{
			res = try_receive_from(remoteEndpoint, buff, length, flags);
			if (res.ok || !try_again(res))
			{
				handler(res);
				return true;
			}
		}
#endif
		try
		{
#if defined(ENABLE_ASIO_PRE_OP) && defined(HAS_ASIO_HANDLER_IS_TRIED)
			if (is_pre_option())
			{
				if (buff && length)
				{
					_socket.async_receive_from(boost::asio::buffer(buff, length), remoteEndpoint, flags, wrap_tried(std::bind([this, &remoteEndpoint, buff, length, flags](Handler& handler, const boost::system::error_code& ec, size_t s)
					{
						async_receive_from_handler(remoteEndpoint, buff, length, std::move(handler), flags, ec, s);
					}, std::forward<Handler>(handler), __1, __2)));
				}
				else
				{
					_socket.async_receive_from(boost::asio::null_buffers(), remoteEndpoint, flags, wrap_tried(std::bind([this, &remoteEndpoint, buff, length, flags](Handler& handler, const boost::system::error_code& ec, size_t s)
					{
						async_receive_from_handler(remoteEndpoint, buff, length, std::move(handler), flags, ec, s);
					}, std::forward<Handler>(handler), __1, __2)));
				}
			}
			else
#endif
			{
				if (buff && length)
				{
					_socket.async_receive_from(boost::asio::buffer(buff, length), remoteEndpoint, flags, std::bind([this, &remoteEndpoint, buff, length, flags](Handler& handler, const boost::system::error_code& ec, size_t s)
					{
						async_receive_from_handler(remoteEndpoint, buff, length, std::move(handler), flags, ec, s);
					}, std::forward<Handler>(handler), __1, __2));
				}
				else
				{
					_socket.async_receive_from(boost::asio::null_buffers(), remoteEndpoint, flags, std::bind([this, &remoteEndpoint, buff, length, flags](Handler& handler, const boost::system::error_code& ec, size_t s)
					{
						async_receive_from_handler(remoteEndpoint, buff, length, std::move(handler), flags, ec, s);
					}, std::forward<Handler>(handler), __1, __2));
				}
			}
			return false;
		}
		catch (const boost::system::system_error& se)
		{
			res = { 0, se.code().value(), !se.code() };
		}
		return check_immed_callback(std::forward<Handler>(handler), res);
	}

	template <typename Handler>
	bool async_receive_from(void* buff, size_t length, Handler&& handler, int flags = 0)
	{
		return async_receive_from(_remoteSenderEndpoint, buff, length, std::forward<Handler>(handler), flags);
	}

	/*!
	@brief �첽ģʽ�£�����Զ�˷��͵����ݵ�������
	*/
	template <typename Handler>
	bool async_receive(void* buff, size_t length, Handler&& handler, int flags = 0)
	{
		result res;
#ifndef HAS_ASIO_CANCEL_IO
		_cancelRecv = false;
#endif
#ifdef ENABLE_ASIO_PRE_OP
		if (is_pre_option())
		{
			res = try_receive(buff, length, flags);
			if (res.ok || !try_again(res))
			{
				handler(res);
				return true;
			}
		}
#endif
		try
		{
#if defined(ENABLE_ASIO_PRE_OP) && defined(HAS_ASIO_HANDLER_IS_TRIED)
			if (is_pre_option())
			{
				if (buff && length)
				{
					_socket.async_receive(boost::asio::buffer(buff, length), flags, wrap_tried(std::bind([this, buff, length, flags](Handler& handler, const boost::system::error_code& ec, size_t s)
					{
						async_receive_handler(buff, length, std::move(handler), flags, ec, s);
					}, std::forward<Handler>(handler), __1, __2)));
				}
				else
				{
					_socket.async_receive(boost::asio::null_buffers(), flags, wrap_tried(std::bind([this, buff, length, flags](Handler& handler, const boost::system::error_code& ec, size_t s)
					{
						async_receive_handler(buff, length, std::move(handler), flags, ec, s);
					}, std::forward<Handler>(handler), __1, __2)));
				}
			}
			else
#endif
			{
				if (buff && length)
				{
					_socket.async_receive(boost::asio::buffer(buff, length), flags, std::bind([this, buff, length, flags](Handler& handler, const boost::system::error_code& ec, size_t s)
					{
						async_receive_handler(buff, length, std::move(handler), flags, ec, s);
					}, std::forward<Handler>(handler), __1, __2));
				}
				else
				{
					_socket.async_receive(boost::asio::null_buffers(), flags, std::bind([this, buff, length, flags](Handler& handler, const boost::system::error_code& ec, size_t s)
					{
						async_receive_handler(buff, length, std::move(handler), flags, ec, s);
					}, std::forward<Handler>(handler), __1, __2));
				}
			}
			return false;
		}
		catch (const boost::system::system_error& se)
		{
			res = { 0, se.code().value(), !se.code() };
		}
		return check_immed_callback(std::forward<Handler>(handler), res);
	}

	/*!
	@brief ���������Է�������
	*/
	result try_send(const void* buff, size_t length, int flags = 0);

	/*!
	@brief ���������Է������ݵ�ָ��Ŀ��
	*/
	result try_send_to(const char* remoteIp, unsigned short remotePort, const void* buff, size_t length, int flags = 0);
	result try_send_to(const boost::asio::ip::udp::endpoint& remoteEndpoint, const void* buff, size_t length, int flags = 0);

	/*!
	@brief ���������Խ�������
	*/
	result try_receive(void* buff, size_t length, int flags = 0);

	/*!
	@brief ���������Խ������ݣ�����¼��Զ�˵�ַ
	*/
	result try_receive_from(boost::asio::ip::udp::endpoint& remoteEndpoint, void* buff, size_t length, int flags = 0);
	result try_receive_from(void* buff, size_t length, int flags = 0);

	/*!
	@brief ����������һ�η��Ͷ�������
	@brief bytes, ÿ������ʵ�ʷ����ֽ���
	@return ʵ�ʷ�����������
	*/
	result try_msend(const void* const* buffs, const size_t* lengths, size_t count, size_t* bytes = NULL, int flags = 0);

	/*!
	@brief ����������һ�η��Ͷ������ݵ�ָ��Ŀ��
	@brief bytes, ÿ������ʵ�ʷ����ֽ���
	@return ʵ�ʷ�����������
	*/
	result try_msend_to(const boost::asio::ip::udp::endpoint* remoteEndpoints, const void* const* buffs, const size_t* lengths, size_t count, size_t* bytes = NULL, int flags = 0);
	result try_msend_to(const boost::asio::ip::udp::endpoint& remoteEndpoint, const void* const* buffs, const size_t* lengths, size_t count, size_t* bytes = NULL, int flags = 0);

	/*!
	@brief ����������һ�ν��ն�������
	@brief bytes, ÿ�黺��ʵ�ʽ����ֽ���
	@return ʵ�ʽ�����������
	*/
	result try_mreceive(void* const* buffs, const size_t* lengths, size_t count, size_t* bytes = NULL, int flags = 0);

	/*!
	@brief ����������һ�ν��ն������ݣ�����¼��Զ�˵�ַ
	@brief bytes, ÿ�黺��ʵ�ʽ����ֽ���
	@return ʵ�ʽ�����������
	*/
	result try_mreceive_from(boost::asio::ip::udp::endpoint* remoteEndpoints, void* const* buffs, const size_t* lengths, size_t count, size_t* bytes = NULL, int flags = 0);

	/*!
	@brief try_io����ʧ���Ƿ�����ΪEAGAIN
	*/
	static bool try_again(const result& res);
private:
	template <typename Handler>
	void async_send_to_handler(const boost::asio::ip::udp::endpoint& remoteEndpoint, const void* buff, size_t length, Handler&& handler, int flags, const boost::system::error_code& ec, size_t s)
	{
		result res = { s, ec.value(), !ec };
#ifndef HAS_ASIO_CANCEL_IO
		while (_holdSend)
		{
			run_thread::sleep(0);
		}
		if (boost::asio::error::operation_aborted == res.code && _socket.is_open())
		{
			if (res.s)
			{
				res.ok = true;
				res.code = 0;
			}
			else if (!_cancelSend)
			{
				try
				{
					_holdSend = true;
					BREAK_OF_SCOPE_EXEC(_holdSend = false);
					if (buff && length)
					{
						_socket.async_send_to(boost::asio::buffer(buff, length), remoteEndpoint, flags, std::bind([this, remoteEndpoint, buff, length, flags](Handler& handler, const boost::system::error_code& ec, size_t s)
						{
							async_send_to_handler(remoteEndpoint, buff, length, std::move(handler), flags, ec, s);
						}, std::forward<Handler>(handler), __1, __2));
					}
					else
					{
						_socket.async_send_to(boost::asio::null_buffers(), remoteEndpoint, flags, std::bind([this, remoteEndpoint, buff, length, flags](Handler& handler, const boost::system::error_code& ec, size_t s)
						{
							async_send_to_handler(remoteEndpoint, buff, length, std::move(handler), flags, ec, s);
						}, std::forward<Handler>(handler), __1, __2));
					}
					if (_cancelSend)
					{
						cancel_send();
					}
					return;
				}
				catch (const boost::system::system_error& se)
				{
					res = { 0, se.code().value(), !se.code() };
				}
			}
		}
#endif
		handler(res);
	}

	template <typename Handler>
	void async_send_handler(const void* buff, size_t length, Handler&& handler, int flags, const boost::system::error_code& ec, size_t s)
	{
		result res = { s, ec.value(), !ec };
#ifndef HAS_ASIO_CANCEL_IO
		while (_holdSend)
		{
			run_thread::sleep(0);
		}
		if (boost::asio::error::operation_aborted == res.code && _socket.is_open())
		{
			if (res.s)
			{
				res.ok = true;
				res.code = 0;
			}
			else if (!_cancelSend)
			{
				try
				{
					_holdSend = true;
					BREAK_OF_SCOPE_EXEC(_holdSend = false);
					if (buff && length)
					{
						_socket.async_send(boost::asio::buffer(buff, length), flags, std::bind([this, buff, length, flags](Handler& handler, const boost::system::error_code& ec, size_t s)
						{
							async_send_handler(buff, length, std::move(handler), flags, ec, s);
						}, std::forward<Handler>(handler), __1, __2));
					}
					else
					{
						_socket.async_send(boost::asio::null_buffers(), flags, std::bind([this, buff, length, flags](Handler& handler, const boost::system::error_code& ec, size_t s)
						{
							async_send_handler(buff, length, std::move(handler), flags, ec, s);
						}, std::forward<Handler>(handler), __1, __2));
					}
					if (_cancelSend)
					{
						cancel_send();
					}
					return;
				}
				catch (const boost::system::system_error& se)
				{
					res = { 0, se.code().value(), !se.code() };
				}
			}
		}
#endif
		handler(res);
	}

	template <typename Handler>
	void async_receive_from_handler(boost::asio::ip::udp::endpoint& remoteEndpoint, void* buff, size_t length, Handler&& handler, int flags, const boost::system::error_code& ec, size_t s)
	{
		result res = { s, ec.value(), !ec };
#ifndef HAS_ASIO_CANCEL_IO
		while (_holdRecv)
		{
			run_thread::sleep(0);
		}
		if (boost::asio::error::operation_aborted == res.code && _socket.is_open())
		{
			if (res.s)
			{
				res.ok = true;
				res.code = 0;
			}
			else if (!_cancelRecv)
			{
				try
				{
					_holdRecv = true;
					BREAK_OF_SCOPE_EXEC(_holdRecv = false);
					if (buff && length)
					{
						_socket.async_receive_from(boost::asio::buffer(buff, length), remoteEndpoint, flags, std::bind([this, &remoteEndpoint, buff, length, flags](Handler& handler, const boost::system::error_code& ec, size_t s)
						{
							async_receive_from_handler(remoteEndpoint, buff, length, std::move(handler), flags, ec, s);
						}, std::forward<Handler>(handler), __1, __2));
					}
					else
					{
						_socket.async_receive_from(boost::asio::null_buffers(), remoteEndpoint, flags, std::bind([this, &remoteEndpoint, buff, length, flags](Handler& handler, const boost::system::error_code& ec, size_t s)
						{
							async_receive_from_handler(remoteEndpoint, buff, length, std::move(handler), flags, ec, s);
						}, std::forward<Handler>(handler), __1, __2));
					}
					if (_cancelRecv)
					{
						cancel_receive();
					}
					return;
				}
				catch (const boost::system::system_error& se)
				{
					res = { 0, se.code().value(), !se.code() };
				}
			}
		}
#endif
		handler(res);
	}

	template <typename Handler>
	void async_receive_handler(void* buff, size_t length, Handler&& handler, int flags, const boost::system::error_code& ec, size_t s)
	{
		result res = { s, ec.value(), !ec };
#ifndef HAS_ASIO_CANCEL_IO
		while (_holdRecv)
		{
			run_thread::sleep(0);
		}
		if (boost::asio::error::operation_aborted == res.code && _socket.is_open())
		{
			if (res.s)
			{
				res.ok = true;
				res.code = 0;
			}
			else if (!_cancelRecv)
			{
				try
				{
					_holdRecv = true;
					BREAK_OF_SCOPE_EXEC(_holdRecv = false);
					if (buff && length)
					{
						_socket.async_receive(boost::asio::buffer(buff, length), flags, std::bind([this, buff, length, flags](Handler& handler, const boost::system::error_code& ec, size_t s)
						{
							async_receive_handler(buff, length, std::move(handler), flags, ec, s);
						}, std::forward<Handler>(handler), __1, __2));
					}
					else
					{
						_socket.async_receive(boost::asio::null_buffers(), flags, std::bind([this, buff, length, flags](Handler& handler, const boost::system::error_code& ec, size_t s)
						{
							async_receive_handler(buff, length, std::move(handler), flags, ec, s);
						}, std::forward<Handler>(handler), __1, __2));
					}
					if (_cancelRecv)
					{
						cancel_receive();
					}
					return;
				}
				catch (const boost::system::system_error& se)
				{
					res = { 0, se.code().value(), !se.code() };
				}
			}
		}
#endif
		handler(res);
	}
	
	template <typename Handler>
	bool check_immed_callback(Handler&& handler, result& res)
	{
#ifdef ENABLE_ASIO_PRE_OP
		if (is_pre_option())
		{
			handler(res);
			return true;
		}
#endif
		_socket.get_io_service().post(std::bind([](Handler& handler, result& res)
		{
			handler(res);
		}, std::forward<Handler>(handler), res));
		return false;
	}
private:
	void set_internal_non_blocking();
private:
	boost::asio::ip::udp::socket _socket;
	boost::asio::ip::udp::endpoint _remoteSenderEndpoint;
#ifndef HAS_ASIO_CANCEL_IO
	volatile bool _holdRecv;
	volatile bool _holdSend;
	volatile bool _cancelRecv;
	volatile bool _cancelSend;
#endif
	bool _nonBlocking;
#ifdef ENABLE_ASIO_PRE_OP
	bool _preOption;
#endif
	NONE_COPY(udp_socket);
};

#endif