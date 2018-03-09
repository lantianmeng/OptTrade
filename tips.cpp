#include "stdafx.h"
#include "tips.h"
#include <string>

MyString::MyString()
{
	if (buf == nullptr)
	{
		buf = new char[DEFAULT_SIZE];
	}
}

MyString::MyString(const MyString & s)
{
	if (s.buf != nullptr)
	{
		buf = new char[strlen(s.buf) + 1];
		strcpy_s(buf, strlen(buf), s.buf);
	}
	else
		buf = nullptr;
}

MyString::~MyString()
{
    if(buf != nullptr)
    {
		delete []buf;
    }
}

MyString & MyString::operator=(const MyString& s)
{
	if (this != &s)
	{
		delete[] buf;
		buf = new char[strlen(s.buf) + 1];
		strcpy_s(buf, strlen(buf), s.buf);
	}
	return *this;
}