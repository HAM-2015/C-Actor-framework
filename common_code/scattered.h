#ifndef __SCATTERED_H
#define __SCATTERED_H

#define   LIBPATH(p, f)   p##f 

#ifdef _WIN64
#pragma comment(lib, LIBPATH(__FILE__, "./../get_sp_x64.lib"))
#else
#pragma comment(lib, LIBPATH(__FILE__, "./../get_sp_x86.lib"))
#pragma comment(lib, LIBPATH(__FILE__, "./../64div32_x86.lib"))
#endif // _WIN64

#undef LIBPATH

/*!
@brief ��ȡ��ǰesp/rspջ���Ĵ���ֵ
*/
extern "C" void* __stdcall get_sp();

/*!
@brief ��ȡCPU����
*/
extern "C" unsigned long long __fastcall cpu_tick();

#ifndef _WIN64

/*!
@brief 64λ��������32λ������ע��ȷ���̲��ᳬ��32bit�������������������
@param a ������
@param b ����
@param pys ��������
@return ��
*/
extern "C" unsigned __fastcall u64div32(unsigned long long a, unsigned b, unsigned* pys = 0);
extern "C" int __fastcall i64div32(long long a, int b, int* pys = 0);

#else // _WIN64


#endif // _WIN64

/*!
@brief ���ø߾���ʱ��
*/
void enable_high_resolution();

/*!
@brief ��������Ȩ����Ϊ����ʵʱ��
*/
void enable_realtime_priority();

/*!
@brief ���ó������ȼ�
REALTIME_PRIORITY_CLASS
HIGH_PRIORITY_CLASS
ABOVE_NORMAL_PRIORITY_CLASS
NORMAL_PRIORITY_CLASS
BELOW_NORMAL_PRIORITY_CLASS
IDLE_PRIORITY_CLASS
*/
void set_priority(int p);

/*!
@brief ��ȡӲ��ʱ���
*/
long long get_tick_us();
long long get_tick_ms();
int get_tick_s();

/*!
@brief ���std::function
*/
template <typename F>
inline void clear_function(F& f)
{
	f = F();
}

#ifdef _DEBUG
#define DEBUG_OPERATION(__exp__)	__exp__
#else
#define DEBUG_OPERATION(__exp__)
#endif

#define REF_STRUCT_NAME(__NAME__) __ref_##__NAME__

#define LAMBDA_REF2(__NAME__, __P0__, __P1__)\
struct REF_STRUCT_NAME(__NAME__)\
{\
	REF_STRUCT_NAME(__NAME__)(decltype(__P0__)& p0, decltype(__P1__)& p1)\
	:__P0__(p0), __P1__(p1){}\
	decltype(__P0__)& __P0__; \
	decltype(__P1__)& __P1__; \
}; \
	REF_STRUCT_NAME(__NAME__) __NAME__(__P0__, __P1__);

#define LAMBDA_REF3(__NAME__, __P0__, __P1__, __P2__)\
struct REF_STRUCT_NAME(__NAME__)\
{\
	REF_STRUCT_NAME(__NAME__)(decltype(__P0__)& p0, decltype(__P1__)& p1, decltype(__P2__)& p2)\
	:__P0__(p0), __P1__(p1), __P2__(p2) {}\
	decltype(__P0__)& __P0__; \
	decltype(__P1__)& __P1__; \
	decltype(__P2__)& __P2__; \
}; \
	REF_STRUCT_NAME(__NAME__) __NAME__(__P0__, __P1__, __P2__);

#define LAMBDA_REF4(__NAME__, __P0__, __P1__, __P2__, __P3__)\
struct REF_STRUCT_NAME(__NAME__)\
{\
	REF_STRUCT_NAME(__NAME__)(decltype(__P0__)& p0, decltype(__P1__)& p1, decltype(__P2__)& p2, decltype(__P3__)& p3)\
	:__P0__(p0), __P1__(p1), __P2__(p2), __P3__(p3) {}\
	decltype(__P0__)& __P0__; \
	decltype(__P1__)& __P1__; \
	decltype(__P2__)& __P2__; \
	decltype(__P3__)& __P3__; \
}; \
	REF_STRUCT_NAME(__NAME__) __NAME__(__P0__, __P1__, __P2__, __P3__);

#define LAMBDA_REF5(__NAME__, __P0__, __P1__, __P2__, __P3__, __P4__)\
struct REF_STRUCT_NAME(__NAME__)\
{\
	REF_STRUCT_NAME(__NAME__)\
	(decltype(__P0__)& p0, decltype(__P1__)& p1, decltype(__P2__)& p2, decltype(__P3__)& p3, decltype(__P4__)& p4)\
	:__P0__(p0), __P1__(p1), __P2__(p2), __P3__(p3), __P4__(p4) {}\
	decltype(__P0__)& __P0__; \
	decltype(__P1__)& __P1__; \
	decltype(__P2__)& __P2__; \
	decltype(__P3__)& __P3__; \
	decltype(__P4__)& __P4__; \
}; \
	REF_STRUCT_NAME(__NAME__) __NAME__(__P0__, __P1__, __P2__, __P3__, __P4__);

#define LAMBDA_REF6(__NAME__, __P0__, __P1__, __P2__, __P3__, __P4__, __P5__)\
struct REF_STRUCT_NAME(__NAME__)\
{\
	REF_STRUCT_NAME(__NAME__)\
	(decltype(__P0__)& p0, decltype(__P1__)& p1, decltype(__P2__)& p2, decltype(__P3__)& p3, decltype(__P4__)& p4, decltype(__P5__)& p5)\
	:__P0__(p0), __P1__(p1), __P2__(p2), __P3__(p3), __P4__(p4), __P5__(p5){}\
	decltype(__P0__)& __P0__; \
	decltype(__P1__)& __P1__; \
	decltype(__P2__)& __P2__; \
	decltype(__P3__)& __P3__; \
	decltype(__P4__)& __P4__; \
	decltype(__P5__)& __P5__; \
}; \
	REF_STRUCT_NAME(__NAME__) __NAME__(__P0__, __P1__, __P2__, __P3__, __P4__, __P5__);

#endif