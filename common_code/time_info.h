#ifndef __TIME_INFO_H
#define __TIME_INFO_H

/*!
@brief ���ø߾���ʱ��
*/
void enable_high_resolution();

/*!
@brief ��������Ȩ����Ϊ����ʵʱ��
*/
void enable_realtime_priority();

/*!
@brief ��ȡӲ��ʱ���(us)
*/
long long get_tick();

#endif