#ifndef __LAMBDA_REF_H
#define __LAMBDA_REF_H

//�û���Ƕlambda���ⲿ�������ò��񣬿��Լ�Сֱ��"&����"sizeof(lambda)�Ĵ�С����ߵ���Ч��
#define LAMBDA_REF1(__NAME__, __P0__)\
	typedef decltype(__P0__) _BOND_LR_(type0, __LINE__); \
struct\
{\
	_BOND_LR_(type0, __LINE__)& __P0__; \
} __NAME__ = { __P0__ };
//------------------------------------------------------------------------------------------

#define LAMBDA_REF2(__NAME__, __P0__, __P1__)\
	typedef decltype(__P0__) _BOND_LR_(type0, __LINE__); \
	typedef decltype(__P1__) _BOND_LR_(type1, __LINE__); \
struct\
{\
	_BOND_LR_(type0, __LINE__)& __P0__; \
	_BOND_LR_(type1, __LINE__)& __P1__; \
} __NAME__ = { __P0__, __P1__ };
//------------------------------------------------------------------------------------------

#define LAMBDA_REF3(__NAME__, __P0__, __P1__, __P2__)\
	typedef decltype(__P0__) _BOND_LR_(type0, __LINE__); \
	typedef decltype(__P1__) _BOND_LR_(type1, __LINE__); \
	typedef decltype(__P2__) _BOND_LR_(type2, __LINE__); \
struct\
{\
	_BOND_LR_(type0, __LINE__)& __P0__; \
	_BOND_LR_(type1, __LINE__)& __P1__; \
	_BOND_LR_(type2, __LINE__)& __P2__; \
} __NAME__ = { __P0__, __P1__, __P2__ };
//------------------------------------------------------------------------------------------

#define LAMBDA_REF4(__NAME__, __P0__, __P1__, __P2__, __P3__)\
	typedef decltype(__P0__) _BOND_LR_(type0, __LINE__); \
	typedef decltype(__P1__) _BOND_LR_(type1, __LINE__); \
	typedef decltype(__P2__) _BOND_LR_(type2, __LINE__); \
	typedef decltype(__P3__) _BOND_LR_(type3, __LINE__); \
struct\
{\
	_BOND_LR_(type0, __LINE__)& __P0__; \
	_BOND_LR_(type1, __LINE__)& __P1__; \
	_BOND_LR_(type2, __LINE__)& __P2__; \
	_BOND_LR_(type3, __LINE__)& __P3__; \
} __NAME__ = { __P0__, __P1__, __P2__, __P3__ };
//------------------------------------------------------------------------------------------

#define LAMBDA_REF5(__NAME__, __P0__, __P1__, __P2__, __P3__, __P4__)\
	typedef decltype(__P0__) _BOND_LR_(type0, __LINE__); \
	typedef decltype(__P1__) _BOND_LR_(type1, __LINE__); \
	typedef decltype(__P2__) _BOND_LR_(type2, __LINE__); \
	typedef decltype(__P3__) _BOND_LR_(type3, __LINE__); \
	typedef decltype(__P4__) _BOND_LR_(type4, __LINE__); \
struct\
{\
	_BOND_LR_(type0, __LINE__)& __P0__; \
	_BOND_LR_(type1, __LINE__)& __P1__; \
	_BOND_LR_(type2, __LINE__)& __P2__; \
	_BOND_LR_(type3, __LINE__)& __P3__; \
	_BOND_LR_(type4, __LINE__)& __P4__; \
} __NAME__ = { __P0__, __P1__, __P2__, __P3__, __P4__ };
//------------------------------------------------------------------------------------------

#define LAMBDA_REF6(__NAME__, __P0__, __P1__, __P2__, __P3__, __P4__, __P5__)\
	typedef decltype(__P0__) _BOND_LR_(type0, __LINE__); \
	typedef decltype(__P1__) _BOND_LR_(type1, __LINE__); \
	typedef decltype(__P2__) _BOND_LR_(type2, __LINE__); \
	typedef decltype(__P3__) _BOND_LR_(type3, __LINE__); \
	typedef decltype(__P4__) _BOND_LR_(type4, __LINE__); \
	typedef decltype(__P5__) _BOND_LR_(type5, __LINE__); \
struct\
{\
	_BOND_LR_(type0, __LINE__)& __P0__; \
	_BOND_LR_(type1, __LINE__)& __P1__; \
	_BOND_LR_(type2, __LINE__)& __P2__; \
	_BOND_LR_(type3, __LINE__)& __P3__; \
	_BOND_LR_(type4, __LINE__)& __P4__; \
	_BOND_LR_(type5, __LINE__)& __P5__; \
} __NAME__ = { __P0__, __P1__, __P2__, __P3__, __P4__, __P5__ };
//------------------------------------------------------------------------------------------

#define LAMBDA_REF7(__NAME__, __P0__, __P1__, __P2__, __P3__, __P4__, __P5__, __P6__)\
	typedef decltype(__P0__) _BOND_LR_(type0, __LINE__); \
	typedef decltype(__P1__) _BOND_LR_(type1, __LINE__); \
	typedef decltype(__P2__) _BOND_LR_(type2, __LINE__); \
	typedef decltype(__P3__) _BOND_LR_(type3, __LINE__); \
	typedef decltype(__P4__) _BOND_LR_(type4, __LINE__); \
	typedef decltype(__P5__) _BOND_LR_(type5, __LINE__); \
	typedef decltype(__P6__) _BOND_LR_(type6, __LINE__); \
struct\
{\
	_BOND_LR_(type0, __LINE__)& __P0__; \
	_BOND_LR_(type1, __LINE__)& __P1__; \
	_BOND_LR_(type2, __LINE__)& __P2__; \
	_BOND_LR_(type3, __LINE__)& __P3__; \
	_BOND_LR_(type4, __LINE__)& __P4__; \
	_BOND_LR_(type5, __LINE__)& __P5__; \
	_BOND_LR_(type6, __LINE__)& __P6__; \
} __NAME__ = { __P0__, __P1__, __P2__, __P3__, __P4__, __P5__, __P6__ };
//------------------------------------------------------------------------------------------

#define LAMBDA_REF8(__NAME__, __P0__, __P1__, __P2__, __P3__, __P4__, __P5__, __P6__, __P7__)\
	typedef decltype(__P0__) _BOND_LR_(type0, __LINE__); \
	typedef decltype(__P1__) _BOND_LR_(type1, __LINE__); \
	typedef decltype(__P2__) _BOND_LR_(type2, __LINE__); \
	typedef decltype(__P3__) _BOND_LR_(type3, __LINE__); \
	typedef decltype(__P4__) _BOND_LR_(type4, __LINE__); \
	typedef decltype(__P5__) _BOND_LR_(type5, __LINE__); \
	typedef decltype(__P6__) _BOND_LR_(type6, __LINE__); \
	typedef decltype(__P7__) _BOND_LR_(type7, __LINE__); \
struct\
{\
	_BOND_LR_(type0, __LINE__)& __P0__; \
	_BOND_LR_(type1, __LINE__)& __P1__; \
	_BOND_LR_(type2, __LINE__)& __P2__; \
	_BOND_LR_(type3, __LINE__)& __P3__; \
	_BOND_LR_(type4, __LINE__)& __P4__; \
	_BOND_LR_(type5, __LINE__)& __P5__; \
	_BOND_LR_(type6, __LINE__)& __P6__; \
	_BOND_LR_(type7, __LINE__)& __P7__; \
} __NAME__ = { __P0__, __P1__, __P2__, __P3__, __P4__, __P5__, __P6__, __P7__ };
//------------------------------------------------------------------------------------------

#define LAMBDA_REF9(__NAME__, __P0__, __P1__, __P2__, __P3__, __P4__, __P5__, __P6__, __P7__, __P8__)\
	typedef decltype(__P0__) _BOND_LR_(type0, __LINE__); \
	typedef decltype(__P1__) _BOND_LR_(type1, __LINE__); \
	typedef decltype(__P2__) _BOND_LR_(type2, __LINE__); \
	typedef decltype(__P3__) _BOND_LR_(type3, __LINE__); \
	typedef decltype(__P4__) _BOND_LR_(type4, __LINE__); \
	typedef decltype(__P5__) _BOND_LR_(type5, __LINE__); \
	typedef decltype(__P6__) _BOND_LR_(type6, __LINE__); \
	typedef decltype(__P7__) _BOND_LR_(type7, __LINE__); \
	typedef decltype(__P8__) _BOND_LR_(type8, __LINE__); \
struct\
{\
	_BOND_LR_(type0, __LINE__)& __P0__; \
	_BOND_LR_(type1, __LINE__)& __P1__; \
	_BOND_LR_(type2, __LINE__)& __P2__; \
	_BOND_LR_(type3, __LINE__)& __P3__; \
	_BOND_LR_(type4, __LINE__)& __P4__; \
	_BOND_LR_(type5, __LINE__)& __P5__; \
	_BOND_LR_(type6, __LINE__)& __P6__; \
	_BOND_LR_(type7, __LINE__)& __P7__; \
	_BOND_LR_(type8, __LINE__)& __P8__; \
} __NAME__ = { __P0__, __P1__, __P2__, __P3__, __P4__, __P5__, __P6__, __P7__, __P8__ };
//------------------------------------------------------------------------------------------

#define LAMBDA_REF10(__NAME__, __P0__, __P1__, __P2__, __P3__, __P4__, __P5__, __P6__, __P7__, __P8__, __P9__)\
	typedef decltype(__P0__) _BOND_LR_(type0, __LINE__); \
	typedef decltype(__P1__) _BOND_LR_(type1, __LINE__); \
	typedef decltype(__P2__) _BOND_LR_(type2, __LINE__); \
	typedef decltype(__P3__) _BOND_LR_(type3, __LINE__); \
	typedef decltype(__P4__) _BOND_LR_(type4, __LINE__); \
	typedef decltype(__P5__) _BOND_LR_(type5, __LINE__); \
	typedef decltype(__P6__) _BOND_LR_(type6, __LINE__); \
	typedef decltype(__P7__) _BOND_LR_(type7, __LINE__); \
	typedef decltype(__P8__) _BOND_LR_(type8, __LINE__); \
	typedef decltype(__P9__) _BOND_LR_(type9, __LINE__); \
struct\
{\
	_BOND_LR_(type0, __LINE__)& __P0__; \
	_BOND_LR_(type1, __LINE__)& __P1__; \
	_BOND_LR_(type2, __LINE__)& __P2__; \
	_BOND_LR_(type3, __LINE__)& __P3__; \
	_BOND_LR_(type4, __LINE__)& __P4__; \
	_BOND_LR_(type5, __LINE__)& __P5__; \
	_BOND_LR_(type6, __LINE__)& __P6__; \
	_BOND_LR_(type7, __LINE__)& __P7__; \
	_BOND_LR_(type8, __LINE__)& __P8__; \
	_BOND_LR_(type9, __LINE__)& __P9__; \
} __NAME__ = { __P0__, __P1__, __P2__, __P3__, __P4__, __P5__, __P6__, __P7__, __P8__, __P9__ };
//////////////////////////////////////////////////////////////////////////

//�û���Ƕlambda���ⲿ�������ò��������ǰthis����ͨ��->���ʣ������Լ�Сֱ��"&����"sizeof(lambda)�Ĵ�С����ߵ���Ч��
#define LAMBDA_THIS_REF1(__NAME__, __P0__)\
	typedef decltype(this) this_type; \
	typedef decltype(__P0__) _BOND_LR_(type0, __LINE__); \
struct\
{\
	this_type get(){ return __this; }\
	this_type operator->(){ return __this; }\
	this_type __this; \
	_BOND_LR_(type0, __LINE__)& __P0__; \
} __NAME__ = { this, __P0__ };
//------------------------------------------------------------------------------------------

#define LAMBDA_THIS_REF2(__NAME__, __P0__, __P1__)\
	typedef decltype(this) this_type; \
	typedef decltype(__P0__) _BOND_LR_(type0, __LINE__); \
	typedef decltype(__P1__) _BOND_LR_(type1, __LINE__); \
struct\
{\
	this_type get(){ return __this; }\
	this_type operator->(){ return __this; }\
	this_type __this; \
	_BOND_LR_(type0, __LINE__)& __P0__; \
	_BOND_LR_(type1, __LINE__)& __P1__; \
} __NAME__ = { this, __P0__, __P1__ };
//------------------------------------------------------------------------------------------

#define LAMBDA_THIS_REF3(__NAME__, __P0__, __P1__, __P2__)\
	typedef decltype(this) this_type; \
	typedef decltype(__P0__) _BOND_LR_(type0, __LINE__); \
	typedef decltype(__P1__) _BOND_LR_(type1, __LINE__); \
	typedef decltype(__P2__) _BOND_LR_(type2, __LINE__); \
struct\
{\
	this_type get(){ return __this; }\
	this_type operator->(){ return __this; }\
	this_type __this; \
	_BOND_LR_(type0, __LINE__)& __P0__; \
	_BOND_LR_(type1, __LINE__)& __P1__; \
	_BOND_LR_(type2, __LINE__)& __P2__; \
} __NAME__ = { this, __P0__, __P1__, __P2__ };
//------------------------------------------------------------------------------------------

#define LAMBDA_THIS_REF4(__NAME__, __P0__, __P1__, __P2__, __P3__)\
	typedef decltype(this) this_type; \
	typedef decltype(__P0__) _BOND_LR_(type0, __LINE__); \
	typedef decltype(__P1__) _BOND_LR_(type1, __LINE__); \
	typedef decltype(__P2__) _BOND_LR_(type2, __LINE__); \
	typedef decltype(__P3__) _BOND_LR_(type3, __LINE__); \
struct\
{\
	this_type get(){ return __this; }\
	this_type operator->(){ return __this; }\
	this_type __this; \
	_BOND_LR_(type0, __LINE__)& __P0__; \
	_BOND_LR_(type1, __LINE__)& __P1__; \
	_BOND_LR_(type2, __LINE__)& __P2__; \
	_BOND_LR_(type3, __LINE__)& __P3__; \
} __NAME__ = { this, __P0__, __P1__, __P2__, __P3__ };
//------------------------------------------------------------------------------------------

#define LAMBDA_THIS_REF5(__NAME__, __P0__, __P1__, __P2__, __P3__, __P4__)\
	typedef decltype(this) this_type; \
	typedef decltype(__P0__) _BOND_LR_(type0, __LINE__); \
	typedef decltype(__P1__) _BOND_LR_(type1, __LINE__); \
	typedef decltype(__P2__) _BOND_LR_(type2, __LINE__); \
	typedef decltype(__P3__) _BOND_LR_(type3, __LINE__); \
	typedef decltype(__P4__) _BOND_LR_(type4, __LINE__); \
struct\
{\
	this_type get(){ return __this; }\
	this_type operator->(){ return __this; }\
	this_type __this; \
	_BOND_LR_(type0, __LINE__)& __P0__; \
	_BOND_LR_(type1, __LINE__)& __P1__; \
	_BOND_LR_(type2, __LINE__)& __P2__; \
	_BOND_LR_(type3, __LINE__)& __P3__; \
	_BOND_LR_(type4, __LINE__)& __P4__; \
} __NAME__ = { this, __P0__, __P1__, __P2__, __P3__, __P4__ };
//------------------------------------------------------------------------------------------

#define LAMBDA_THIS_REF6(__NAME__, __P0__, __P1__, __P2__, __P3__, __P4__, __P5__)\
	typedef decltype(this) this_type; \
	typedef decltype(__P0__) _BOND_LR_(type0, __LINE__); \
	typedef decltype(__P1__) _BOND_LR_(type1, __LINE__); \
	typedef decltype(__P2__) _BOND_LR_(type2, __LINE__); \
	typedef decltype(__P3__) _BOND_LR_(type3, __LINE__); \
	typedef decltype(__P4__) _BOND_LR_(type4, __LINE__); \
	typedef decltype(__P5__) _BOND_LR_(type5, __LINE__); \
struct\
{\
	this_type get(){ return __this; }\
	this_type operator->(){ return __this; }\
	this_type __this; \
	_BOND_LR_(type0, __LINE__)& __P0__; \
	_BOND_LR_(type1, __LINE__)& __P1__; \
	_BOND_LR_(type2, __LINE__)& __P2__; \
	_BOND_LR_(type3, __LINE__)& __P3__; \
	_BOND_LR_(type4, __LINE__)& __P4__; \
	_BOND_LR_(type5, __LINE__)& __P5__; \
} __NAME__ = { this, __P0__, __P1__, __P2__, __P3__, __P4__, __P5__ };
//------------------------------------------------------------------------------------------

#define LAMBDA_THIS_REF7(__NAME__, __P0__, __P1__, __P2__, __P3__, __P4__, __P5__, __P6__)\
	typedef decltype(this) this_type; \
	typedef decltype(__P0__) _BOND_LR_(type0, __LINE__); \
	typedef decltype(__P1__) _BOND_LR_(type1, __LINE__); \
	typedef decltype(__P2__) _BOND_LR_(type2, __LINE__); \
	typedef decltype(__P3__) _BOND_LR_(type3, __LINE__); \
	typedef decltype(__P4__) _BOND_LR_(type4, __LINE__); \
	typedef decltype(__P5__) _BOND_LR_(type5, __LINE__); \
	typedef decltype(__P6__) _BOND_LR_(type6, __LINE__); \
struct\
{\
	this_type get(){ return __this; }\
	this_type operator->(){ return __this; }\
	this_type __this; \
	_BOND_LR_(type0, __LINE__)& __P0__; \
	_BOND_LR_(type1, __LINE__)& __P1__; \
	_BOND_LR_(type2, __LINE__)& __P2__; \
	_BOND_LR_(type3, __LINE__)& __P3__; \
	_BOND_LR_(type4, __LINE__)& __P4__; \
	_BOND_LR_(type5, __LINE__)& __P5__; \
	_BOND_LR_(type6, __LINE__)& __P6__; \
} __NAME__ = { this, __P0__, __P1__, __P2__, __P3__, __P4__, __P5__, __P6__ };
//------------------------------------------------------------------------------------------

#define LAMBDA_THIS_REF8(__NAME__, __P0__, __P1__, __P2__, __P3__, __P4__, __P5__, __P6__, __P7__)\
	typedef decltype(this) this_type; \
	typedef decltype(__P0__) _BOND_LR_(type0, __LINE__); \
	typedef decltype(__P1__) _BOND_LR_(type1, __LINE__); \
	typedef decltype(__P2__) _BOND_LR_(type2, __LINE__); \
	typedef decltype(__P3__) _BOND_LR_(type3, __LINE__); \
	typedef decltype(__P4__) _BOND_LR_(type4, __LINE__); \
	typedef decltype(__P5__) _BOND_LR_(type5, __LINE__); \
	typedef decltype(__P6__) _BOND_LR_(type6, __LINE__); \
	typedef decltype(__P7__) _BOND_LR_(type7, __LINE__); \
struct\
{\
	this_type get(){ return __this; }\
	this_type operator->(){ return __this; }\
	this_type __this; \
	_BOND_LR_(type0, __LINE__)& __P0__; \
	_BOND_LR_(type1, __LINE__)& __P1__; \
	_BOND_LR_(type2, __LINE__)& __P2__; \
	_BOND_LR_(type3, __LINE__)& __P3__; \
	_BOND_LR_(type4, __LINE__)& __P4__; \
	_BOND_LR_(type5, __LINE__)& __P5__; \
	_BOND_LR_(type6, __LINE__)& __P6__; \
	_BOND_LR_(type7, __LINE__)& __P7__; \
} __NAME__ = { this, __P0__, __P1__, __P2__, __P3__, __P4__, __P5__, __P6__, __P7__ };
//------------------------------------------------------------------------------------------

#define LAMBDA_THIS_REF9(__NAME__, __P0__, __P1__, __P2__, __P3__, __P4__, __P5__, __P6__, __P7__, __P8__)\
	typedef decltype(this) this_type; \
	typedef decltype(__P0__) _BOND_LR_(type0, __LINE__); \
	typedef decltype(__P1__) _BOND_LR_(type1, __LINE__); \
	typedef decltype(__P2__) _BOND_LR_(type2, __LINE__); \
	typedef decltype(__P3__) _BOND_LR_(type3, __LINE__); \
	typedef decltype(__P4__) _BOND_LR_(type4, __LINE__); \
	typedef decltype(__P5__) _BOND_LR_(type5, __LINE__); \
	typedef decltype(__P6__) _BOND_LR_(type6, __LINE__); \
	typedef decltype(__P7__) _BOND_LR_(type7, __LINE__); \
	typedef decltype(__P8__) _BOND_LR_(type8, __LINE__); \
struct\
{\
	this_type get(){ return __this; }\
	this_type operator->(){ return __this; }\
	this_type __this; \
	_BOND_LR_(type0, __LINE__)& __P0__; \
	_BOND_LR_(type1, __LINE__)& __P1__; \
	_BOND_LR_(type2, __LINE__)& __P2__; \
	_BOND_LR_(type3, __LINE__)& __P3__; \
	_BOND_LR_(type4, __LINE__)& __P4__; \
	_BOND_LR_(type5, __LINE__)& __P5__; \
	_BOND_LR_(type6, __LINE__)& __P6__; \
	_BOND_LR_(type7, __LINE__)& __P7__; \
	_BOND_LR_(type8, __LINE__)& __P8__; \
} __NAME__ = { this, __P0__, __P1__, __P2__, __P3__, __P4__, __P5__, __P6__, __P7__, __P8__ };
//------------------------------------------------------------------------------------------

#define LAMBDA_THIS_REF10(__NAME__, __P0__, __P1__, __P2__, __P3__, __P4__, __P5__, __P6__, __P7__, __P8__, __P9__)\
	typedef decltype(this) this_type; \
	typedef decltype(__P0__) _BOND_LR_(type0, __LINE__); \
	typedef decltype(__P1__) _BOND_LR_(type1, __LINE__); \
	typedef decltype(__P2__) _BOND_LR_(type2, __LINE__); \
	typedef decltype(__P3__) _BOND_LR_(type3, __LINE__); \
	typedef decltype(__P4__) _BOND_LR_(type4, __LINE__); \
	typedef decltype(__P5__) _BOND_LR_(type5, __LINE__); \
	typedef decltype(__P6__) _BOND_LR_(type6, __LINE__); \
	typedef decltype(__P7__) _BOND_LR_(type7, __LINE__); \
	typedef decltype(__P8__) _BOND_LR_(type8, __LINE__); \
	typedef decltype(__P9__) _BOND_LR_(type9, __LINE__); \
struct\
{\
	this_type get(){ return __this; }\
	this_type operator->(){ return __this; }\
	this_type __this; \
	_BOND_LR_(type0, __LINE__)& __P0__; \
	_BOND_LR_(type1, __LINE__)& __P1__; \
	_BOND_LR_(type2, __LINE__)& __P2__; \
	_BOND_LR_(type3, __LINE__)& __P3__; \
	_BOND_LR_(type4, __LINE__)& __P4__; \
	_BOND_LR_(type5, __LINE__)& __P5__; \
	_BOND_LR_(type6, __LINE__)& __P6__; \
	_BOND_LR_(type7, __LINE__)& __P7__; \
	_BOND_LR_(type8, __LINE__)& __P8__; \
	_BOND_LR_(type9, __LINE__)& __P9__; \
} __NAME__ = { this, __P0__, __P1__, __P2__, __P3__, __P4__, __P5__, __P6__, __P7__, __P8__, __P9__ };

#define _PP_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, N, ...) N
#define _PP_NARG(...) _BOND_LR(_PP_ARG_N(__VA_ARGS__, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0),)

#define _BOND_LR(a, b) a##b
#define _BOND_LR_(a, b) _BOND_LR(a, b)
#define _BOND_LR__(a, b) _BOND_LR_(a, b)
//�û���Ƕlambda���ⲿ�������ò��񣬿��Լ�Сֱ��"&����"sizeof(lambda)�Ĵ�С����ߵ���Ч��
#define LAMBDA_REF(__NAME__, ...) _BOND_LR__(LAMBDA_REF, _PP_NARG(__VA_ARGS__))(__NAME__, __VA_ARGS__)
//�û���Ƕlambda���ⲿ�������ò��������ǰthis����ͨ��->���ʣ������Լ�Сֱ��"&����"sizeof(lambda)�Ĵ�С����ߵ���Ч��
#define LAMBDA_THIS_REF(__NAME__, ...) _BOND_LR__(LAMBDA_THIS_REF, _PP_NARG(__VA_ARGS__))(__NAME__, __VA_ARGS__)

#endif