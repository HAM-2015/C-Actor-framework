// actor_test.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "ios_proxy.h"
#include "actor_framework.h"
#include "msg_pipe.h"
#include "async_buffer.h"
#include "time_info.h"
#include <list>
#include <Windows.h>

using namespace std;

void check_key_down(boost_actor* actor, int id)
{//��ⰴ������
	while (GetAsyncKeyState(id) >= 0)
	{
		actor->sleep(1);
	}
}

void check_key_up(boost_actor* actor, int id)
{//��ⰴ������
	while (GetAsyncKeyState(id) < 0)
	{
		actor->sleep(1);
	}
}

void check_key_test(boost_actor* actor, int id)
{
	while (true)
	{//�������º󣬼�ⵯ��1000ms��û�е�����ǳ�ʱ����
		check_key_down(actor, id);//��ⰴ��
		child_actor_handle checkUp = actor->create_child_actor(boost::bind(&check_key_up, _1, id), 24 kB);//����һ����ⵯ�����Actor
		actor->child_actor_run(checkUp);//��ʼ������Actor
		actor->delay_trig(1000, boost::bind(&boost_actor::notify_force_quit, checkUp.get_actor()));//���õ���ʱ������ʱ��ǿ�ƹر�checkUp
		if (actor->child_actor_wait_quit(checkUp))//�����˳��ķ���true����ǿ�ƹرյķ���false
		{
			actor->cancel_delay_trig();
			printf("ok %d\n", id);
		} 
		else
		{//����ʱ
			printf("timeout %d\n", id);
			check_key_up(actor, id);//��ⵯ��
		}
	}
}

void actor_print(boost_actor* actor, int id)
{//���º��ȴ�ӡһ���ַ���500ms��ÿ��50ms��ӡһ��
	printf("_");
	actor->sleep(500);
	while (true)
	{
		printf("-");
		actor->sleep(50);
	}
}

void actor_test_print(boost_actor* actor, int id)
{
	while (true)
	{
		check_key_down(actor, id);//��ⰴ��
		child_actor_handle ch = actor->create_child_actor(boost::bind(&actor_print, _1, id));
		actor->child_actor_run(ch);
		check_key_up(actor, id);//��ⵯ��
		actor->child_actor_force_quit(ch);
	}
}

void actor_suspend(boost_actor* actor, const list<actor_handle>& chs)
{
	while (true)
	{
		check_key_down(actor, VK_RBUTTON);
		actor->actors_suspend(chs);
		check_key_up(actor, VK_RBUTTON);
	}
}

void actor_resume(boost_actor* actor, const list<actor_handle>& chs)
{
	while (true)
	{
		check_key_down(actor, VK_LBUTTON);
		actor->actors_resume(chs);
		check_key_up(actor, VK_LBUTTON);
	}
}

void check_both_up(boost_actor* actor, int id, bool& own, const bool& another)
{
	goto __start;
	while (!another)
	{
		actor->sleep(1);
		own = false;
__start:
		check_key_up(actor, id);
		own = true;
	}
}

void check_two_down(boost_actor* actor, int dt, int id1, int id2)
{
	while (true)
	{
		{//������������Ƿ񶼴��ڵ���״̬
			bool st1 = false;
			bool st2 = false;
			child_actor_handle up1 = actor->create_child_actor(boost::bind(&check_both_up, _1, id1, boost::ref(st1), boost::ref(st2)));
			child_actor_handle up2 = actor->create_child_actor(boost::bind(&check_both_up, _1, id2, boost::ref(st2), boost::ref(st1)));
			async_trig_handle<bool> ath;
			auto nh = actor->begin_trig(ath);
			up1.get_actor()->append_quit_callback(nh);
			up2.get_actor()->append_quit_callback(nh);
			actor->child_actor_run(up1);
			actor->child_actor_run(up2);
			if (actor->timed_wait_trig(ath, st1, 3000))
			{
				actor->child_actor_force_quit(up1);
				actor->child_actor_force_quit(up2);
			}
			else
			{
				printf("�˳� check_two_down\n");
				actor->child_actor_force_quit(up1);
				actor->child_actor_force_quit(up2);
				actor->close_trig(ath);
				break;
			}
		}
		{
			child_actor_handle check1 = actor->create_child_actor(boost::bind(&check_key_down, _1, id1));
			child_actor_handle check2 = actor->create_child_actor(boost::bind(&check_key_down, _1, id2));
			async_trig_handle<actor_handle, bool> ath;
			auto nh = actor->begin_trig(ath);
			check1.get_actor()->append_quit_callback(boost::bind(nh, check2.get_actor(), _1));
			check2.get_actor()->append_quit_callback(boost::bind(nh, check1.get_actor(), _1));
			actor->child_actor_run(check1);
			actor->child_actor_run(check2);
			actor_handle ah;
			bool ok = false;
			actor->wait_trig(ath, ah, ok);
			actor->delay_trig(dt, boost::bind(&boost_actor::notify_force_quit, ah));
			if (actor->actor_wait_quit(ah))
			{
				printf("*success*\n");
			} 
			else
			{
				printf("*failure*\n");
			}
			actor->cancel_delay_trig();
			actor->child_actor_force_quit(check1);
			actor->child_actor_force_quit(check2);
		}
	}
}

void wait_key(boost_actor* actor, int id, boost::function<void (int)> cb)
{
	check_key_up(actor, id);
	while (true)
	{
		check_key_down(actor, id);
		cb(id);
		check_key_up(actor, id);
	}
}

void shift_key(boost_actor* actor, actor_handle pauseactor)
{
	list<child_actor_handle::ptr> childs;
	actor_msg_handle<int> amh;
	auto h = actor->make_msg_notify(amh);
	for (int i = 'A'; i <= 'Z'; i++)
	{
		auto tp = child_actor_handle::make_ptr();
		*tp = actor->create_child_actor(boost::bind(&wait_key, _1, i, h));
		actor->child_actor_run(*tp);
		childs.push_back(tp);
	}
	while (true)
	{
		int id = actor->pump_msg(amh);
		if ('P' == id)
		{
			bool isPause = actor->actor_switch(pauseactor);
			printf("%s���ܲ���\n", isPause? "��ͣ": "�ָ�");
		}
		else
		{
			printf("shift+%c\n", id);
		}
	}
	actor->close_msg_notify(amh);
	actor->child_actors_force_quit(childs);
}

void test_shift(boost_actor* actor, actor_handle pauseactor)
{
	child_actor_handle ch;
	while (true)
	{
		check_key_down(actor, VK_SHIFT);
		ch = actor->create_child_actor(boost::bind(&shift_key, _1, pauseactor));
		actor->child_actor_run(ch);
		check_key_up(actor, VK_SHIFT);
		actor->child_actor_force_quit(ch);
	}
}

void test_producer(boost_actor* actor, boost::function<void (int, int)> writer)
{//������
	for (int i = 0; true; i++)
	{
		writer(i, (int)actor->this_id());
		actor->sleep(2100);
	}
}

void test_consumer(boost_actor* actor, actor_msg_handle<int, int>& amh)
{//������
	while (true)
	{
		int p0;
		int id;
		actor->pump_msg(amh, p0, id);
		printf("%d-%d id=%d\n", p0, (int)amh.get_size(), id);
		actor->sleep(1000);
	}
	actor->close_msg_notify(amh);
}

void count_test(boost_actor* actor, int& ct)
{
	while (true)
	{
		ct++;
		actor->sleep(0);
	}
}

void perfor_test(boost_actor* actor, ios_proxy& ios, int actorNum)
{
	int* count = (int*)_alloca(actorNum*sizeof(int));
	actor->check_stack();
	vector<shared_strand> strands;
	strands.resize(ios.threadNumber());
	for (int i = 0; i < (int)ios.threadNumber(); i++)
	{
		strands[i] = boost_strand::create(ios);
	}

	for (int num = 1; true; num = num % actorNum + 1)
	{
		long long tk = get_tick_us();
		list<child_actor_handle::ptr> childList;
		for (int i = 0; i < num; i++)
		{
			count[i] = 0;
			auto newactor = child_actor_handle::make_ptr();
			*newactor = actor->create_child_actor(strands[i%strands.size()], boost::bind(&count_test, _1, boost::ref(count[i])));
			childList.push_front(newactor);
			actor->child_actor_run(*childList.front());
		}
		actor->sleep(1000);
		actor->child_actors_force_quit(childList);
		int ct = 0;
		for (int i = 0; i < num; i++)
		{
			ct += count[i];
		}
		double f = (double)ct * 1000000 / (get_tick_us()-tk);
		printf("Actor��=%d, �л�Ƶ��=%d\n", num, (int)f);
	}
}

void create_null_actor(boost_actor* actor, int* count)
{
	while (true)
	{
		size_t yieldTick = actor->yield_count();
		list<child_actor_handle::ptr> childList;
		for (int i = 0; i < 100; i++)
		{
			(*count)++;
			auto newactor = child_actor_handle::make_ptr();
			*newactor = actor->create_child_actor([](boost_actor*){});
			childList.push_back(newactor);
		}
		actor->child_actors_force_quit(childList);
		if (actor->yield_count() == yieldTick)
		{
			actor->sleep(0);
		}
	}
}

void create_actor_test(boost_actor* actor)
{
	while (true)
	{
		int count = 0;
		child_actor_handle createactor = actor->create_child_actor(boost::bind(&create_null_actor, _1, &count));
		actor->child_actor_run(createactor);
		actor->sleep(1000);
		actor->child_actor_force_quit(createactor);
		printf("1�봴��Actor��=%d\n", count);
	}
}

void async_buffer_read(boost_actor* actor, boost::shared_ptr<async_buffer<int> > buffer, msg_pipe<>::regist_reader regWaitFull, int max)
{
	actor_msg_handle<> amh;
	regWaitFull(actor, amh);
	goto __start;
	while (true)
	{
		actor->sleep(500);
		int i;
		bool empty = buffer->pop(i);
		printf("%d\n", i);
		if (max == i)
		{
			break;
		}
		if (empty)
		{
			printf("read �����\n");
			__start:
			actor->pump_msg(amh);
			printf("read ����������\n");
		}
	}
	actor->close_msg_notify(amh);
}

void async_buffer_test(boost_actor* actor)
{
	msg_pipe<>::writer_type fullNotify;
	msg_pipe<>::regist_reader regWaitFull = msg_pipe<>::make(fullNotify);
	boost::shared_ptr<async_buffer<int> > buffer = async_buffer<int>::create(10);
	actor_msg_handle<> amh;
	int testCyc = 30;
	buffer->setNotify(fullNotify, actor->make_msg_notify(amh));
	child_actor_handle readactor = actor->create_child_actor(boost::bind(&async_buffer_read, _1, buffer, regWaitFull, testCyc-1));
	actor->child_actor_run(readactor);
	for (int i = 0; i < testCyc; i++)
	{
		check_key_down(actor, VK_CONTROL);
		check_key_up(actor, VK_CONTROL);
		if (buffer->push(i))
		{
			printf("writer ������\n");
			actor->pump_msg(amh);
			printf("writer ������\n");
		}
	}
	actor->child_actor_wait_quit(readactor);
	actor->close_msg_notify(amh);
	printf("������Խ���\n");
}

void actor_test(boost_actor* actor)
{
	ios_proxy perforIos;//���ڲ��Զ��߳��µ�Actor�л�����
	perforIos.run(ios_proxy::hardwareConcurrency());//�������߳�����ΪCPU�߳���
	perforIos.runPriority(ios_proxy::idle);//���������ȼ�����Ϊ���
	child_actor_handle actorLeft = actor->create_child_actor(boost::bind(&check_key_test, _1, VK_LEFT));
	child_actor_handle actorRight = actor->create_child_actor(boost::bind(&check_key_test, _1, VK_RIGHT));
	child_actor_handle actorUp = actor->create_child_actor(boost::bind(&check_key_test, _1, VK_UP));
	child_actor_handle actorDown = actor->create_child_actor(boost::bind(&check_key_test, _1, VK_DOWN));
	child_actor_handle actorPrint = actor->create_child_actor(boost::bind(&actor_test_print, _1, VK_SPACE));//���ո��
	child_actor_handle actorTwo = actor->create_child_actor(boost::bind(&check_two_down, _1, 10, 'D', 'F'));//���D,F���Ƿ�ͬʱ����(���ü�����10ms)
	child_actor_handle actorPerfor = actor->create_child_actor(boost::bind(&perfor_test, _1, boost::ref(perforIos), 200));//Actor�л����ܲ���
	child_actor_handle actorCreate = actor->create_child_actor(boost::bind(&create_actor_test, _1));//Actor�������ܲ���
	child_actor_handle actorShift = actor->create_child_actor(boost::bind(&test_shift, _1, actorPerfor.get_actor()));//shift+��ĸ���
	child_actor_handle actorBuffer = actor->create_child_actor(boost::bind(&async_buffer_test, _1));//�첽������в���
	child_actor_handle actorProducer1;
	child_actor_handle actorProducer2;
	child_actor_handle actorConsumer;
	//����������/������ģ�Ͳ���
	actor_msg_handle<int, int> conCmh;
	{
		actorConsumer = actor->create_child_actor(boost::bind(&test_consumer, _1, boost::ref(conCmh)));
		auto h = actorConsumer.get_actor()->make_msg_notify(conCmh);
		actorProducer1 = actor->create_child_actor(boost::bind(&test_producer, _1, h));
		actorProducer2 = actor->create_child_actor(boost::bind(&test_producer, _1, h));
	}

	list<actor_handle> chs;//��Ҫ�������Actor���󣬿��Դ��·�ע�ͼ�������
	chs.push_back(actorLeft.get_actor());
	chs.push_back(actorRight.get_actor());
	chs.push_back(actorUp.get_actor());
	chs.push_back(actorDown.get_actor());
	chs.push_back(actorPrint.get_actor());
	chs.push_back(actorShift.get_actor());
	chs.push_back(actorTwo.get_actor());
	chs.push_back(actorConsumer.get_actor());
//	chs.push_back(actorPerfor.get_actor());
	chs.push_back(actorProducer1.get_actor());
	chs.push_back(actorProducer2.get_actor());
	chs.push_back(actorCreate.get_actor());
	child_actor_handle actorSuspend = actor->create_child_actor(boost::bind(&actor_suspend, _1, boost::ref(chs)), 32 kB);//�������Ҽ���ͣ�������
	child_actor_handle actorResume = actor->create_child_actor(boost::bind(&actor_resume, _1, boost::ref(chs)), 32 kB);//����������ָ��������
	actor->child_actor_run(actorLeft);
	actor->child_actor_run(actorRight);
	actor->child_actor_run(actorUp);
	actor->child_actor_run(actorDown);
	actor->child_actor_run(actorPrint);
	actor->child_actor_run(actorShift);
	actor->child_actor_run(actorTwo);
	actor->child_actor_run(actorPerfor);
// 	actor->child_actor_run(actorConsumer);
// 	actor->child_actor_run(actorProducer1);
// 	actor->child_actor_run(actorProducer2);
//	actor->child_actor_run(actorCreate);
	actor->child_actor_run(actorSuspend);
	actor->child_actor_run(actorResume);
	actor->child_actor_run(actorBuffer);
	actor->child_actor_suspend(actorPerfor);
	check_key_down(actor, VK_ESCAPE);//ESC���˳�
	actor->child_actor_force_quit(actorLeft);
	actor->child_actor_force_quit(actorRight);
	actor->child_actor_force_quit(actorUp);
	actor->child_actor_force_quit(actorDown);
	actor->child_actor_force_quit(actorPrint);
	actor->child_actor_force_quit(actorShift);
	actor->child_actor_force_quit(actorTwo);
	actor->child_actor_force_quit(actorPerfor);
	actor->child_actor_force_quit(actorConsumer);
	actor->child_actor_force_quit(actorProducer1);
	actor->child_actor_force_quit(actorProducer2);
	actor->child_actor_force_quit(actorCreate);
	actor->child_actor_force_quit(actorSuspend);
	actor->child_actor_force_quit(actorResume);
	actor->child_actor_force_quit(actorBuffer);
	perforIos.stop();
}


	/*
	�߼����Ʋ��Գ���
	�������ҷ������⣬���º�1000ms�ڵ����ӡok�������ӡtimeout;
	�ո����⣬���º��ӡ _ ��500ms��û�е���ÿ��50ms��ӡ - ;
	D,F����⣬10ms��������"ͬʱ"����(����"ͬʱ"��������һǰһ��)��ӡsuccess�������ӡfailure;
	��갴����⣬�Ҽ����¹�������������»ָ�����;
	������/������ģ�Ͳ���;
	ESC���˳�;
	ע�⣺ĳЩ���̿��ܲ�֧��2�����ϰ���ͬʱ����
	*/
int main(int argc, char* argv[])
{
	enable_high_resolution();
	boost_actor::enable_stack_pool();
	ios_proxy ios;
	ios.run();
	{
		actor_handle actorTest = boost_actor::create(boost_strand::create(ios), boost::bind(&actor_test, _1));
		actorTest->notify_start_run();
		actorTest->outside_wait_quit();
	}
 	ios.stop();
	return 0;
}
