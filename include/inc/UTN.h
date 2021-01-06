#pragma once  
#include "stdafx.h"  
#include "windows.h"  
#include <iostream>  
#include <string>  
using namespace std;

//**************string******************//  
// ASCII��Unicode��ת  
wstring AsciiToUnicode(const string& str);
string  UnicodeToAscii(const wstring& wstr);
// UTF8��Unicode��ת  
wstring Utf8ToUnicode(const string& str);
string  UnicodeToUtf8(const wstring& wstr);
// ASCII��UTF8��ת  
string  AsciiToUtf8(const string& str);
string  Utf8ToAscii(const string& str);
//**************CString******************//  
// ASCII��Unicode��ת  
CStringW    AsciiToUnicode_CSTR(const CStringA& str);
CStringA    UnicodeToAscii_CSTR(const CStringW& wstr);
// UTF8��Unicode��ת  
CStringW    Utf8ToUnicode_CSTR(const CStringA& str);
CStringA    UnicodeToUtf8_CSTR(const CStringW& wstr);
// ASCII��UTF8��ת  
CStringA    AsciiToUtf8_CSTR(const CStringA& str);
CStringA    Utf8ToAscii_CSTR(const CStringA& str);
/************string-int***************/
// string ת Int  
int StringToInt(const string& str);
string IntToString(int i);
string IntToString(char i);
string IntToString(double i);