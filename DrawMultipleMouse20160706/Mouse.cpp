#include "Mouse.h"


Mouse::Mouse(void)
{
	this->Brush = NULL;
	this->ClientMouseX = 0;
	this->ClientMouseY = 0;
	
	this->MyFont = new Gdiplus::Font(L"�l�r �S�V�b�N", 9);
	this->BlackBrush = new Gdiplus::SolidBrush(Color(255, 0, 0, 0));
	this->MyFormat = new Gdiplus::StringFormat();
	this->MyFormat->SetAlignment(StringAlignmentNear);
}

Mouse::~Mouse(void)
{
	//delete Brush;
	//delete MyFont;
	//delete BlackBrush;
}

void Mouse::SetClient(int x, int y, int width, int height)
{
	// Window�̈ʒu��T�C�Y�̕ϓ��ɔ����}�E�X���W�̍Čv�Z���K�v	
}

void Mouse::SetBrush(Color c)
{
	if (NULL != this->Brush) 
	{ delete this->Brush; }
	this->Brush = new SolidBrush(c);
}

void Mouse::Draw(Graphics* g)
{
	g->FillEllipse(this->Brush, this->ClientMouseX - (10/2), this->ClientMouseY - (10/2), 10, 10);
	g->DrawString(DeviceName.c_str(), -1, this->MyFont, RectF(ClientMouseX + (10/2), ClientMouseY - (10/2), 800, 100), this->MyFormat, this->BlackBrush);
}