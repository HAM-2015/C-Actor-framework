// actor_test.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "ios_proxy.h"
#include "actor_framework.h"
#include "async_buffer.h"
#include "scattered.h"
#include <list>
#include <Windows.h>

using namespace std;

void check_key_down(my_actor* self, int id)
{//��ⰴ������
	while (GetAsyncKeyState(id) >= 0)
	{
		self->sleep(1);
	}
}

void check_key_up(my_actor* self, int id)
{//��ⰴ������
	while (GetAsyncKeyState(id) < 0)
	{
		self->sleep(1);
	}
}

void check_key_test(my_actor* self, int id)
{
	while (true)
	{//�������º󣬼�ⵯ��1000ms��û�е�����ǳ�ʱ����
		check_key_down(self, id);//��ⰴ��
		child_actor_handle checkUp = self->create_child_actor([&](my_actor* self){check_key_up(self, id); }, 24 kB);//����һ����ⵯ�����Actor
		self->child_actor_run(checkUp);//��ʼ������Actor
		self->delay_trig(1000, [&]{checkUp.get_actor()->notify_quit(); });//���õ���ʱ������ʱ��ǿ�ƹر�checkUp
		if (self->child_actor_wait_quit(checkUp))//�����˳��ķ���true����ǿ�ƹرյķ���false
		{
			self->cancel_delay_trig();
			printf("ok %d\n", id);
		} 
		else
		{//����ʱ
			printf("timeout %d\n", id);
			check_key_up(self, id);//��ⵯ��
		}
	}
}

void actor_print(my_actor* self, int id)
{//���º��ȴ�ӡһ���ַ���500ms��ÿ��50ms��ӡһ��
	printf("_");
	self->sleep(500);
	while (true)
	{
		printf("-");
		self->sleep(50);
	}
}

void actor_test_print(my_actor* self, int id)
{
	while (true)
	{
		check_key_down(self, id);//��ⰴ��
		child_actor_handle ch = self->create_child_actor(boost::bind(&actor_print, _1, id));
		self->child_actor_run(ch);
		check_key_up(self, id);//��ⵯ��
		self->child_actor_force_quit(ch);
	}
}

void actor_suspend(my_actor* self, const list<actor_handle>& chs)
{
	while (true)
	{
		check_key_down(self, VK_RBUTTON);
		self->actors_suspend(chs);
		check_key_up(self, VK_RBUTTON);
	}
}

void actor_resume(my_actor* self, const list<actor_handle>& chs)
{
	while (true)
	{
		check_key_down(self, VK_LBUTTON);
		self->actors_resume(chs);
		check_key_up(self, VK_LBUTTON);
	}
}

void check_both_up(my_actor* self, int id, bool& own, const bool& another)
{
	goto __start;
	while (!another)
	{
		self->sleep(1);
		own = false;
__start:
		check_key_up(self, id);
		own = true;
	}
}

void check_two_down(my_actor* self, int dt, int id1, int id2)
{
	while (true)
	{
		{//������������Ƿ񶼴��ڵ���״̬
			bool st1 = false;
			bool st2 = false;
			child_actor_handle up1 = self->create_child_actor([&](my_actor* self){check_both_up(self, id1, st1, st2); });
			child_actor_handle up2 = self->create_child_actor([&](my_actor* self){check_both_up(self, id2, st2, st1); });
			actor_trig_handle<bool> ath;
			auto nh = self->make_trig_notifer(ath);
			up1.get_actor()->append_quit_callback(nh);
			up2.get_actor()->append_quit_callback(nh);
			self->child_actor_run(up1);
			self->child_actor_run(up2);
			LAMBDA_REF3(ref3, self, up1, up2);
			if (!self->timed_wait_trig(3000, ath, [&ref3](bool)
			{
				ref3.self->child_actor_force_quit(ref3.up1);
				ref3.self->child_actor_force_quit(ref3.up2);
			}))
			{
				printf("�˳� check_two_down\n");
				self->child_actor_force_quit(up1);
				self->child_actor_force_quit(up2);
				self->close_trig_notifer(ath);
				break;
			}
		}
		{
			child_actor_handle check1 = self->create_child_actor([&](my_actor* self){check_key_down(self, id1); });
			child_actor_handle check2 = self->create_child_actor([&](my_actor* self){check_key_down(self, id2); });
			actor_trig_handle<actor_handle, bool> ath;
			auto nh = self->make_trig_notifer(ath);
			check1.get_actor()->append_quit_callback([&](bool ok){nh(check2.get_actor(), ok); });
			check2.get_actor()->append_quit_callback([&](bool ok){nh(check1.get_actor(), ok); });
			self->child_actor_run(check1);
			self->child_actor_run(check2);
			actor_handle ah;
			bool ok = false;
			self->wait_trig(ath, ah, ok);
			self->delay_trig(dt, [&]{ah->notify_quit(); });
			if (self->actor_wait_quit(ah))
			{
				printf("*success*\n");
			} 
			else
			{
				printf("*failure*\n");
			}
			self->cancel_delay_trig();
			self->child_actor_force_quit(check1);
			self->child_actor_force_quit(check2);
		}
	}
}

void wait_key(my_actor* self, int id, std::function<void (int)> cb)
{
	check_key_up(self, id);
	while (true)
	{
		check_key_down(self, id);
		cb(id);
		check_key_up(self, id);
	}
}

void shift_key(my_actor* self, actor_handle pauseactor)
{
	list<child_actor_handle::ptr> childs;
	actor_msg_handle<int> amh;
	auto h = self->make_msg_notifer(amh);
	for (int i = 'A'; i <= 'Z'; i++)
	{
		auto tp = child_actor_handle::make_ptr();
		*tp = self->create_child_actor(boost::bind(&wait_key, _1, i, h));
		self->child_actor_run(*tp);
		childs.push_back(tp);
	}
	while (true)
	{
		int id = self->wait_msg(amh);
		if ('P' == id)
		{
			bool isPause = self->actor_switch(pauseactor);
			printf("%s���ܲ���\n", isPause? "��ͣ": "�ָ�");
		}
		else
		{
			printf("shift+%c\n", id);
		}
	}
	self->close_msg_notifer(amh);
	self->child_actors_force_quit(childs);
}

void test_shift(my_actor* self, actor_handle pauseactor)
{
	child_actor_handle ch;
	while (true)
	{
		check_key_down(self, VK_SHIFT);
		ch = self->create_child_actor(boost::bind(&shift_key, _1, pauseactor));
		self->child_actor_run(ch);
		check_key_up(self, VK_SHIFT);
		self->child_actor_force_quit(ch);
	}
}

void perfor_test(my_actor* self, ios_proxy& ios)
{
	self->check_stack();
	vector<shared_strand> strands;
	strands.resize(ios.threadNumber());
	for (size_t i = 0; i < strands.size(); i++)
	{
		strands[i] = boost_strand::create(ios);
	}

	for (int n = 1; n < 200; n++)
	{
		int num = n*n;
		long long tk = get_tick_us();
		list<child_actor_handle::ptr> childList;
		vector<int> count;
		count.resize(num);
#ifdef _DEBUG
		size_t stackSize = DEFAULT_STACKSIZE;
#else
		size_t stackSize = 12 kB;
#endif
		for (int i = 0; i < num; i++)
		{
			count[i] = 0;
			auto newactor = child_actor_handle::make_ptr();
			*newactor = self->create_child_actor(strands[i%strands.size()], [&count, i](my_actor* self)
			{
				while (true)
				{
					count[i]++;
					self->sleep(0);
				}
			}, stackSize);
			childList.push_front(newactor);
			self->child_actor_run(*childList.front());
		}
		self->sleep(1000);
		self->child_actors_force_quit(childList);
		int ct = 0;
		for (int i = 0; i < num; i++)
		{
			ct += count[i];
		}
		double f = (double)ct * 1000000 / (get_tick_us()-tk);
		printf("Actor��=%d, �л�Ƶ��=%d\n", num, (int)f);
	}
	printf("���ܲ��Խ���\n");
}

void create_null_actor(my_actor* self, int* count)
{
	while (true)
	{
		size_t yieldTick = self->yield_count();
		list<child_actor_handle::ptr> childList;
		for (int i = 0; i < 100; i++)
		{
			(*count)++;
			auto newactor = child_actor_handle::make_ptr();
			*newactor = self->create_child_actor([](my_actor*){});
			childList.push_back(newactor);
		}
		self->child_actors_force_quit(childList);
		if (self->yield_count() == yieldTick)
		{
			self->yield();
		}
	}
}

void create_actor_test(my_actor* self)
{
	while (true)
	{
		int count = 0;
		child_actor_handle createactor = self->create_child_actor(boost::bind(&create_null_actor, _1, &count));
		self->child_actor_run(createactor);
		self->sleep(1000);
		self->child_actor_force_quit(createactor);
		printf("1�봴��Actor��=%d\n", count);
	}
}

void actor_test(my_actor* self)
{
	ios_proxy perforIos;//���ڲ��Զ��߳��µ�Actor�л�����
	perforIos.run(ios_proxy::hardwareConcurrency());//�������߳�����ΪCPU�߳���
	perforIos.runPriority(ios_proxy::idle);//���������ȼ�����Ϊ���
	child_actor_handle actorLeft = self->create_child_actor(boost::bind(&check_key_test, _1, VK_LEFT));
	child_actor_handle actorRight = self->create_child_actor(boost::bind(&check_key_test, _1, VK_RIGHT));
	child_actor_handle actorUp = self->create_child_actor(boost::bind(&check_key_test, _1, VK_UP));
	child_actor_handle actorDown = self->create_child_actor(boost::bind(&check_key_test, _1, VK_DOWN));
	child_actor_handle actorPrint = self->create_child_actor(boost::bind(&actor_test_print, _1, VK_SPACE));//���ո��
	child_actor_handle actorTwo = self->create_child_actor(boost::bind(&check_two_down, _1, 10, 'D', 'F'));//���D,F���Ƿ�ͬʱ����(���ü�����10ms)
	child_actor_handle actorPerfor = self->create_child_actor(boost::bind(&perfor_test, _1, boost::ref(perforIos)));//Actor�л����ܲ���
	child_actor_handle actorCreate = self->create_child_actor(boost::bind(&create_actor_test, _1));//Actor�������ܲ���
	child_actor_handle actorShift = self->create_child_actor(boost::bind(&test_shift, _1, actorPerfor.get_actor()));//shift+��ĸ���
	child_actor_handle actorProducer1;
	child_actor_handle actorProducer2;
	child_actor_handle actorConsumer;
	child_actor_handle actorMutex1;
	child_actor_handle actorMutex2;
	child_actor_handle actorMutex3;
	child_actor_handle actorConVarWait;
	child_actor_handle actorConVarNtf;
	child_actor_handle buffPush;
	child_actor_handle buffPop;
	//����������/������ģ�Ͳ���
	{//ģ����Ϣת����ת���л�
		actorConsumer = self->create_child_actor(self->self_strand()->clone(), [](my_actor* self)
		{//������
			auto agentRun = [](my_actor* self)
			{
				auto conCmh = self->connect_msg_pump<int, int>();
				int p0;
				int id;
				self->pump_msg(conCmh, p0, id);
				printf("����:%d ������:%d ������:%d\n", p0, id, (int)self->self_id());
				while (true)
				{
					BEGIN_TRY_
					{
						self->pump_msg(conCmh, p0, id, true);
						printf("����:%d ������:%d ������:%d\n", p0, id, (int)self->self_id());
					}
					CATCH_PUMP_DISCONNECTED
					{
						printf("������:%d ���Ͽ�\n", (int)self->self_id());//��Ϣ�ñ����رգ��׳��쳣
						self->pump_msg(conCmh, p0, id);
						printf("����:%d ������:%d ������:%d\n", p0, id, (int)self->self_id());
					}
					END_TRY_;
				}
			};
			auto st = self->self_strand()->clone();
			child_actor_handle agent1 = self->create_child_actor(st, agentRun, 64 kB);
			child_actor_handle agent2 = self->create_child_actor(st, agentRun, 64 kB);
			self->child_actor_run(agent1);
			self->child_actor_run(agent2);
			while (true)
			{
				self->msg_agent_to<int, int>(agent1);//��Ϣ��agent1����
				self->sleep(5000);
				self->msg_agent_to<int, int>(agent2);//�Ͽ�agent1�����л���agent2����
				self->sleep(2000);
				//self->msg_agent_off<int, int>();//�Ͽ�agent2����
				//self->sleep(200);
				auto conCmh = self->connect_msg_pump<int, int>();//ֹͣ��Ϣ�������Լ�����
				for (int i = 0; i < 3; i++)
				{
					self->pump_msg<int, int>(conCmh, [self](int p0, int id)
					{
						printf("����:%d ������:%d ������:%d\n", p0, id, (int)self->self_id());
					});
				}
			}
		});
		self->child_actor_run(actorConsumer);
		auto writer = self->connect_msg_notifer_to<int, int>(actorConsumer);//������Ϣ��actorConsumer
		auto test_producer = [writer](my_actor* self)
		{//������
			for (int i = 0; true; i++)
			{
				writer(i, (int)self->self_id());
				self->sleep(2000);
			}
		};
		actorProducer1 = self->create_child_actor(test_producer);
		actorProducer2 = self->create_child_actor(test_producer);
//		self->child_actor_run(actorProducer1);
//		self->child_actor_run(actorProducer2);
	}
	{
		actor_mutex amutex(self->self_strand());
		auto actorMutexH = [amutex](my_actor* self)
		{//ģ��actor_mutex��������
			while (true)
			{
				my_actor::quit_guard qg(self);//���������ڼ䲻��Actorǿ���˳�
				//actor_lock_guard lg(amutex, self);
				if (amutex.timed_lock(800, self))
				{
					for (int i = 0; i < 10 && !self->quit_msg(); i++)
					{
						printf("%d--%d\n", i, (int)self->self_id());
						self->sleep(100);
					}
					amutex.unlock(self);
				}
				else
				{
					printf("---\n");
				}
			}
		};
		actorMutex1 = self->create_child_actor(actorMutexH);
		actorMutex2 = self->create_child_actor(actorMutexH);
		actorMutex3 = self->create_child_actor(actorMutexH);
		//self->child_actor_run(actorMutex1);//ģ��actor_mutex��������
		//self->child_actor_run(actorMutex2);
		//self->child_actor_run(actorMutex3);
	}
	{//������������
		std::shared_ptr<bool> waiting(new bool(false));
		actor_mutex amutex(self->self_strand());
		actor_condition_variable aconVar(self->self_strand());
		actorConVarWait = self->create_child_actor([waiting, amutex, aconVar](my_actor* self)
		{
			while (true)
			{
				actor_lock_guard lg(amutex, self);
				assert(!*waiting);
				*waiting = true;
				if (aconVar.timed_wait(90, self, lg))
				{
					printf("notify\n");
				}
				else
				{
					*waiting = false;
					printf("tm\n");
				}
			}
		});
		actorConVarNtf = self->create_child_actor([&](my_actor* self)
		{
			while (true)
			{
				{
					actor_lock_guard lg(amutex, self);
					if (*waiting)
					{
						*waiting = false;
						aconVar.notify_one(self);
					}
				}
				self->sleep(100);
			}
		});
		//self->child_actor_run(actorConVarWait);
		//self->child_actor_run(actorConVarNtf);
	}
	async_buffer<passing_test> abuff(2, self->self_strand());
	{//�첽���в���
		buffPush = self->create_child_actor(self->self_strand(), [&](my_actor* self)
		{
			int i = 0;
			try
			{
				while (true)
				{
					abuff.push(self, passing_test(i++));
				}
			}
			catch (async_buffer<int>::close_exception)
			{
				printf("--\n");
			}
		});
		buffPop = self->create_child_actor(self->self_strand(), [&](my_actor* self)
		{
			int i = 0;
			try
			{
				while (true)
				{
					passing_test id = abuff.pop(self);
					printf("%d\n", id._count->_id);
					self->sleep(1000);
				}
			}
			catch (async_buffer<int>::close_exception)
			{
				printf("!--\n");
			}
		});
		//self->child_actor_run(buffPush);
		//self->child_actor_run(buffPop);
	}
	list<actor_handle> chs;//��Ҫ�������Actor���󣬿��Դ��·�ע�ͼ�������
	chs.push_back(actorLeft.get_actor());
	chs.push_back(actorRight.get_actor());
	chs.push_back(actorUp.get_actor());
	chs.push_back(actorDown.get_actor());
	chs.push_back(actorPrint.get_actor());
	chs.push_back(actorShift.get_actor());
	chs.push_back(actorTwo.get_actor());
//	chs.push_back(actorConsumer.get_actor());
//	chs.push_back(actorPerfor.get_actor());
	chs.push_back(actorProducer1.get_actor());
	chs.push_back(actorProducer2.get_actor());
	chs.push_back(actorCreate.get_actor());
	child_actor_handle actorSuspend = self->create_child_actor(boost::bind(&actor_suspend, _1, boost::ref(chs)), 32 kB);//�������Ҽ���ͣ�������
	child_actor_handle actorResume = self->create_child_actor(boost::bind(&actor_resume, _1, boost::ref(chs)), 32 kB);//����������ָ��������
	self->child_actor_run(actorLeft);
	self->child_actor_run(actorRight);
	self->child_actor_run(actorUp);
	self->child_actor_run(actorDown);
	self->child_actor_run(actorPrint);
	self->child_actor_run(actorShift);
	self->child_actor_run(actorTwo);
	self->child_actor_run(actorPerfor);
//	self->child_actor_run(actorCreate);
	self->child_actor_run(actorSuspend);
	self->child_actor_run(actorResume);
	self->child_actor_suspend(actorPerfor);
	check_key_down(self, VK_ESCAPE);//ESC���˳�
	self->child_actor_force_quit(actorLeft);
	self->child_actor_force_quit(actorRight);
	self->child_actor_force_quit(actorUp);
	self->child_actor_force_quit(actorDown);
	self->child_actor_force_quit(actorPrint);
	self->child_actor_force_quit(actorShift);
	self->child_actor_force_quit(actorTwo);
	self->child_actor_force_quit(actorPerfor);
	self->child_actor_force_quit(actorConsumer);
	self->child_actor_force_quit(actorProducer1);
	self->child_actor_force_quit(actorProducer2);
	self->child_actor_force_quit(actorCreate);
	self->child_actor_force_quit(actorSuspend);
	self->child_actor_force_quit(actorResume);
	actorMutex1.get_actor()->notify_quit();
	actorMutex2.get_actor()->notify_quit();
	actorMutex3.get_actor()->notify_quit();
	self->child_actor_wait_quit(actorMutex1);
	self->child_actor_wait_quit(actorMutex2);
	self->child_actor_wait_quit(actorMutex3);
	self->child_actor_force_quit(actorConVarWait);
	self->child_actor_force_quit(actorConVarNtf);
	abuff.close(self);
	self->child_actor_force_quit(buffPush);
	self->child_actor_force_quit(buffPop);
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
	my_actor::enable_stack_pool();
	ios_proxy ios;
	ios.run();
	{
		actor_handle actorTest = my_actor::create(boost_strand::create(ios), boost::bind(&actor_test, _1));
		actorTest->notify_run();
		actorTest->outside_wait_quit();
	}
 	ios.stop();
	return 0;
}
