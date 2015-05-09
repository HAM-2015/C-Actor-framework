// wx_test.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "wx_ui.h"
#include <wx/wx.h>

class MyApp :public wxApp
{
public:
	bool OnInit()
	{
		ios_proxy ios;
		ios.run();
		{
			wx_ui wxTest(ios);
			wxTest.ShowModal();
		}
		ios.stop();
		return false;
	}
};

IMPLEMENT_APP(MyApp)
