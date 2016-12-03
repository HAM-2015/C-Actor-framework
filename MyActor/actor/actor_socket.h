#ifndef __ACTOR_SOCKET_H
#define __ACTOR_SOCKET_H

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>
#include "my_actor.h"

/*!
@brief tcpͨ��
*/
class tcp_socket
{
public:
	struct result
	{
		size_t s;///<�ֽ���
		bool ok;///<�Ƿ�ɹ�
	};
public:
	tcp_socket(boost::asio::io_service& ios);
	~tcp_socket();
public:
	/*!
	@brief ����no_delay����
	*/
	bool no_delay();

	/*!
	@brief ���ض˿�IP
	*/
	std::string local_endpoint(unsigned short& port);

	/*!
	@brief Զ�˶˿�IP
	*/
	std::string remote_endpoint(unsigned short& port);

	/*!
	@brief ��ȡboost socket����
	*/
	boost::asio::ip::tcp::socket& boost_socket();

	/*!
	@brief �ͻ���ģʽ������Զ�˷�����
	*/
	bool connect(my_actor* host, const char* remoteIp, unsigned short remotePort);

	/*!
	@brief ��buff�������ڶ�ȡ���ݣ�ֱ������
	*/
	result read(my_actor* host, void* buff, size_t length);

	/*!
	@brief ��buff�������ڶ�ȡ���ݣ��ж��ٶ�����
	*/
	result read_some(my_actor* host, void* buff, size_t length);

	/*!
	@brief ��buff�������ڵ�����ȫ�����ͳ�ȥ
	*/
	result write(my_actor* host, const void* buff, size_t length);

	/*!
	@brief ��buff�������ڵ����ݷ��ͳ�ȥ���ܷ������Ƕ���
	*/
	result write_some(my_actor* host, const void* buff, size_t length);

	/*!
	@brief ��msʱ�䷶Χ�ڣ��ͻ���ģʽ������Զ�˷�����
	*/
	bool timed_connect(my_actor* host, int ms, bool& overtime, const char* remoteIp, unsigned short remotePort);

	/*!
	@brief ��msʱ�䷶Χ�ڣ���buff�������ڶ�ȡ���ݣ�ֱ������
	*/
	result timed_read(my_actor* host, int ms, bool& overtime, void* buff, size_t length);

	/*!
	@brief ��msʱ�䷶Χ�ڣ���buff�������ڶ�ȡ���ݣ��ж��ٶ�����
	*/
	result timed_read_some(my_actor* host, int ms, bool& overtime, void* buff, size_t length);

	/*!
	@brief ��msʱ�䷶Χ�ڣ���buff�������ڵ�����ȫ�����ͳ�ȥ
	*/
	result timed_write(my_actor* host, int ms, bool& overtime, const void* buff, size_t length);

	/*!
	@brief ��msʱ�䷶Χ�ڣ���buff�������ڵ����ݷ��ͳ�ȥ���ܷ������Ƕ���
	*/
	result timed_write_some(my_actor* host, int ms, bool& overtime, const void* buff, size_t length);

	/*!
	@brief �ر�socket
	*/
	bool close();

	/*!
	@brief �첽ģʽ�£��ͻ���ģʽ������Զ�˷�����
	*/
	template <typename Handler>
	void async_connect(const char* remoteIp, unsigned short remotePort, Handler&& handler)
	{
		_socket.async_connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(remoteIp), remotePort), std::bind([](Handler& handler, const boost::system::error_code& ec)
		{
			handler(!ec);
		}, std::forward<Handler>(handler), __1));
	}

	/*!
	@brief �첽ģʽ�£���buff�������ڶ�ȡ���ݣ�ֱ������
	*/
	template <typename Handler>
	void async_read(void* buff, size_t length, Handler&& handler)
	{
		boost::asio::async_read(_socket, boost::asio::buffer(buff, length), std::bind([](Handler& handler, const boost::system::error_code& ec, size_t s)
		{
			result res = { s, !ec };
			handler(res);
		}, std::forward<Handler>(handler), __1, __2));
	}

	/*!
	@brief �첽ģʽ�£���buff�������ڶ�ȡ���ݣ��ж��ٶ�����
	*/
	template <typename Handler>
	void async_read_some(void* buff, size_t length, Handler&& handler)
	{
		_socket.async_read_some(boost::asio::buffer(buff, length), std::bind([](Handler& handler, const boost::system::error_code& ec, size_t s)
		{
			result res = { s, !ec };
			handler(res);
		}, std::forward<Handler>(handler), __1, __2));
	}

	/*!
	@brief �첽ģʽ�£���buff�������ڵ�����ȫ�����ͳ�ȥ
	*/
	template <typename Handler>
	void async_write(const void* buff, size_t length, Handler&& handler)
	{
		boost::asio::async_write(_socket, boost::asio::buffer(buff, length), std::bind([](Handler& handler, const boost::system::error_code& ec, size_t s)
		{
			result res = { s, !ec };
			handler(res);
		}, std::forward<Handler>(handler), __1, __2));
	}

	/*!
	@brief �첽ģʽ�£���buff�������ڵ����ݷ��ͳ�ȥ���ܷ������Ƕ���
	*/
	template <typename Handler>
	void async_write_some(const void* buff, size_t length, Handler&& handler)
	{
		_socket.async_write_some(boost::asio::buffer(buff, length), std::bind([](Handler& handler, const boost::system::error_code& ec, size_t s)
		{
			result res = { s, !ec };
			handler(res);
		}, std::forward<Handler>(handler), __1, __2));
	}
private:
	boost::asio::ip::tcp::socket _socket;
	NONE_COPY(tcp_socket);
};

/*!
@brief ������������
*/
class tcp_acceptor
{
public:
	tcp_acceptor(boost::asio::io_service& ios);
	~tcp_acceptor();
public:
	/*!
	@brief ֻ�����ض�ip���Ӹ÷�����
	*/
	bool open(const char* ip, unsigned short port);

	/*!
	@brief ip v4�´򿪷�����
	*/
	bool open_v4(unsigned short port);

	/*!
	@brief ip v6�´򿪷�����
	*/
	bool open_v6(unsigned short port);

	/*!
	@brief �ر�������
	*/
	bool close();

	/*!
	@brief ��ȡboost acceptor����
	*/
	boost::asio::ip::tcp::acceptor& boost_acceptor();

	/*!
	@brief ��socket�����ͻ�������
	*/
	bool accept(my_actor* host, tcp_socket& socket);

	/*!
	@brief ��msʱ�䷶Χ�ڣ���socket�����ͻ�������
	*/
	bool timed_accept(my_actor* host, int ms, bool& overtime, tcp_socket& socket);

	/*!
	@brief �첽ģʽ�£���socket�����ͻ�������
	*/
	template <typename Handler>
	void async_accept(tcp_socket& socket, Handler&& handler)
	{
		_acceptor->async_accept(socket.boost_socket(), std::bind([](Handler& handler, const boost::system::error_code& ec)
		{
			handler(!ec);
		}, std::forward<Handler>(handler), __1));
	}
private:
	boost::asio::io_service& _ios;
	stack_obj<boost::asio::ip::tcp::acceptor, false> _acceptor;
	bool _opend;
	NONE_COPY(tcp_acceptor);
};

/*!
@brief udpͨ��
*/
class udp_socket
{
public:
	typedef boost::asio::ip::udp::endpoint remote_sender_endpoint;

	struct result
	{
		size_t s;///<�ֽ���
		bool ok;///<�Ƿ�ɹ�
	};
public:
	udp_socket(boost::asio::io_service& ios);
	~udp_socket();
public:
	/*!
	@brief �ر�socket
	*/
	bool close();

	/*!
	@brief ip v4ģʽ��socket
	*/
	bool open_v4();

	/*!
	@brief ip v6ģʽ��socket
	*/
	bool open_v6();

	/*!
	@brief �󶨱���һ��ip��ĳ���˿ڽ��շ�������
	*/
	bool bind(const char* ip, unsigned short port);

	/*!
	@brief ��ip v4��ĳ���˿ڽ��շ�������
	*/
	bool bind_v4(unsigned short port);

	/*!
	@brief ��ip v6��ĳ���˿ڽ��շ�������
	*/
	bool bind_v6(unsigned short port);

	/*!
	@brief �򿪲���ip v4��ĳ���˿ڽ��շ�������
	*/
	bool open_bind_v4(unsigned short port);

	/*!
	@brief �򿪲���ip v6��ĳ���˿ڽ��շ�������
	*/
	bool open_bind_v6(unsigned short port);

	/*!
	@brief ��ȡboost socket����
	*/
	boost::asio::ip::udp::socket& boost_socket();

	/*!
	@brief �趨һ��Զ�̶˿���ΪĬ�Ϸ��ͽ���Ŀ��
	*/
	bool connect(my_actor* host, const char* remoteIp, unsigned short remotePort);
	bool sync_connect(const char* remoteIp, unsigned short remotePort);

	/*!
	@brief ����buff���������ݵ�ָ��Ŀ��
	*/
	result send_to(my_actor* host, const remote_sender_endpoint& remoteEndpoint, const void* buff, size_t length, int flags = 0);

	/*!
	@brief ����buff���������ݵ�ָ��Ŀ��
	*/
	result send_to(my_actor* host, const char* remoteIp, unsigned short remotePort, const void* buff, size_t length, int flags = 0);

	/*!
	@brief ����buff���������ݵ�Ĭ��Ŀ��(connect�ɹ���)
	*/
	result send(my_actor* host, const void* buff, size_t length, int flags = 0);

	/*!
	@brief ����Զ�˷��͵����ݵ�buff������������¼��Զ�˵�ַ
	*/
	result receive_from(my_actor* host, void* buff, size_t length, int flags = 0);

	/*!
	@brief ����Զ�˷��͵����ݵ�buff������
	*/
	result receive(my_actor* host, void* buff, size_t length, int flags = 0);

	/*!
	@brief ��msʱ�䷶Χ�ڣ����Ͷ��趨һ��Զ�̶˿���ΪĬ�Ϸ��ͽ���Ŀ��
	*/
	bool timed_connect(my_actor* host, int ms, bool& overtime, const char* remoteIp, unsigned short remotePort);

	/*!
	@brief ��msʱ�䷶Χ�ڣ�����buff���������ݵ�ָ��Ŀ��
	*/
	result timed_send_to(my_actor* host, int ms, bool& overtime, const remote_sender_endpoint& remoteEndpoint, const void* buff, size_t length, int flags = 0);

	/*!
	@brief ��msʱ�䷶Χ�ڣ�����buff���������ݵ�ָ��Ŀ��
	*/
	result timed_send_to(my_actor* host, int ms, bool& overtime, const char* remoteIp, unsigned short remotePort, const void* buff, size_t length, int flags = 0);

	/*!
	@brief ��msʱ�䷶Χ�ڣ�����buff���������ݵ�Ĭ��Ŀ��(connect�ɹ���)
	*/
	result timed_send(my_actor* host, int ms, bool& overtime, const void* buff, size_t length, int flags = 0);

	/*!
	@brief ��msʱ�䷶Χ�ڣ�����Զ�˷��͵����ݵ�buff������������¼��Զ�˵�ַ
	*/
	result timed_receive_from(my_actor* host, int ms, bool& overtime, void* buff, size_t length, int flags = 0);

	/*!
	@brief ��msʱ�䷶Χ�ڣ�����Զ�˷��͵����ݵ�buff������
	*/
	result timed_receive(my_actor* host, int ms, bool& overtime, void* buff, size_t length, int flags = 0);

	/*!
	@brief ����һ��Զ��Ŀ��
	*/
	static remote_sender_endpoint make_endpoint(const char* remoteIp, unsigned short remotePort);

	/*!
	@brief receive_from��ɺ��ȡԶ�˵�ַ
	*/
	const remote_sender_endpoint& last_remote_sender_endpoint();

	/*!
	@brief ����Զ�˵�ַ
	*/
	void reset_remote_sender_endpoint();

	/*!
	@brief �첽ģʽ�£����Ͷ��趨һ��Զ�̶˿���ΪĬ�Ϸ��ͽ���Ŀ��
	*/
	template <typename Handler>
	void async_connect(const char* remoteIp, unsigned short remotePort, Handler&& handler)
	{
		_socket.async_connect(boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(remoteIp), remotePort), std::bind([](Handler& handler, const boost::system::error_code& ec)
		{
			handler(!ec);
		}, std::forward<Handler>(handler), __1));
	}

	/*!
	@brief �첽ģʽ�£�����buff���������ݵ�ָ��Ŀ��
	*/
	template <typename Handler>
	void async_send_to(const remote_sender_endpoint& remoteEndpoint, const void* buff, size_t length, Handler&& handler, int flags = 0)
	{
		_socket.async_send_to(boost::asio::buffer(buff, length), remoteEndpoint, flags, std::bind([](Handler& handler, const boost::system::error_code& ec, size_t s)
		{
			result res = { s, !ec };
			handler(res);
		}, std::forward<Handler>(handler), __1, __2));
	}

	/*!
	@brief �첽ģʽ�£�����buff���������ݵ�ָ��Ŀ��
	*/
	template <typename Handler>
	void async_send_to(const char* remoteIp, unsigned short remotePort, const void* buff, size_t length, Handler&& handler, int flags = 0)
	{
		async_send_to(remote_sender_endpoint(boost::asio::ip::address::from_string(remoteIp), remotePort), buff, length, std::forward<Handler>(handler), flags);
	}

	/*!
	@brief �첽ģʽ�£�����buff���������ݵ�Ĭ��Ŀ��(connect�ɹ���)
	*/
	template <typename Handler>
	void async_send(const void* buff, size_t length, Handler&& handler, int flags = 0)
	{
		_socket.async_send(boost::asio::buffer(buff, length), flags, std::bind([](Handler& handler, const boost::system::error_code& ec, size_t s)
		{
			result res = { s, !ec };
			handler(res);
		}, std::forward<Handler>(handler), __1, __2));
	}

	/*!
	@brief �첽ģʽ�£�����Զ�˷��͵����ݵ�buff������������¼��Զ�˵�ַ
	*/
	template <typename Handler>
	void async_receive_from(void* buff, size_t length, Handler&& handler, int flags = 0)
	{
		_socket.async_receive_from(boost::asio::buffer(buff, length), _remoteSenderEndpoint, flags, std::bind([](Handler& handler, const boost::system::error_code& ec, size_t s)
		{
			result res = { s, !ec };
			handler(res);
		}, std::forward<Handler>(handler), __1, __2));
	}

	/*!
	@brief �첽ģʽ�£�����Զ�˷��͵����ݵ�buff������
	*/
	template <typename Handler>
	void async_receive(void* buff, size_t length, Handler&& handler, int flags = 0)
	{
		_socket.async_receive(boost::asio::buffer(buff, length), flags, std::bind([](Handler& handler, const boost::system::error_code& ec, size_t s)
		{
			result res = { s, !ec };
			handler(res);
		}, std::forward<Handler>(handler), __1, __2));
	}
private:
	boost::asio::ip::udp::socket _socket;
	boost::asio::ip::udp::endpoint _remoteSenderEndpoint;
	NONE_COPY(udp_socket);
};

#endif