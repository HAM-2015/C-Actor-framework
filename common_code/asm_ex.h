#ifndef __GET_SP_H
#define __GET_SP_H

#define   LIBPATH(p, f)   p##f 

#ifdef _WIN64
#pragma comment(lib, LIBPATH(__FILE__, "./../get_sp_x64.lib"))
#else
#pragma comment(lib, LIBPATH(__FILE__, "./../get_sp_x86.lib"))
#pragma comment(lib, LIBPATH(__FILE__, "./../64div32_x86.lib"))
#endif

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
extern "C" unsigned __fastcall u64div32(unsigned long long a, unsigned b, unsigned* pys = NULL);
extern "C" int __fastcall i64div32(long long a, int b, int* pys = NULL);

#else


#endif

#endif