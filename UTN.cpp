#pragma once  
#include "stdafx.h"  
#include "UTN.h"  

wstring AsciiToUnicode(const string& str) {
	// Ԥ��-�������п��ֽڵĳ���    
	int unicodeLen = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, nullptr, 0);
	// ��ָ�򻺳�����ָ����������ڴ�    
	wchar_t *pUnicode = (wchar_t*)malloc(sizeof(wchar_t)*unicodeLen);
	// ��ʼ�򻺳���ת���ֽ�    
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, pUnicode, unicodeLen);
	wstring ret_str = pUnicode;
	free(pUnicode);
	return ret_str;
}
string UnicodeToAscii(const wstring& wstr) {
	// Ԥ��-�������ж��ֽڵĳ���    
	int ansiiLen = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
	// ��ָ�򻺳�����ָ����������ڴ�    
	char *pAssii = (char*)malloc(sizeof(char)*ansiiLen);
	// ��ʼ�򻺳���ת���ֽ�    
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, pAssii, ansiiLen, nullptr, nullptr);
	string ret_str = pAssii;
	free(pAssii);
	return ret_str;
}
wstring Utf8ToUnicode(const string& str) {
	// Ԥ��-�������п��ֽڵĳ���    
	int unicodeLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
	// ��ָ�򻺳�����ָ����������ڴ�    
	wchar_t *pUnicode = (wchar_t*)malloc(sizeof(wchar_t)*unicodeLen);
	// ��ʼ�򻺳���ת���ֽ�    
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, pUnicode, unicodeLen);
	wstring ret_str = pUnicode;
	free(pUnicode);
	return ret_str;
}
string UnicodeToUtf8(const wstring& wstr) {
	// Ԥ��-�������ж��ֽڵĳ���    
	int ansiiLen = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
	// ��ָ�򻺳�����ָ����������ڴ�    
	char *pAssii = (char*)malloc(sizeof(char)*ansiiLen);
	// ��ʼ�򻺳���ת���ֽ�    
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, pAssii, ansiiLen, nullptr, nullptr);
	string ret_str = pAssii;
	free(pAssii);
	return ret_str;
}
string AsciiToUtf8(const string& str) {
	return UnicodeToUtf8(AsciiToUnicode(str));
}
string Utf8ToAscii(const string& str) {
	return UnicodeToAscii(Utf8ToUnicode(str));
}
// ASCII��Unicode��ת  
CStringW    AsciiToUnicode_CSTR(const CStringA& str) {
	return AsciiToUnicode(LPCSTR(str)).c_str();
}
CStringA    UnicodeToAscii_CSTR(const CStringW& wstr) {
	return UnicodeToAscii(LPCWSTR(wstr)).c_str();
}
// UTF8��Unicode��ת  
CStringW    Utf8ToUnicode_CSTR(const CStringA& str) {
	return Utf8ToUnicode(LPCSTR(str)).c_str();
}
CStringA    UnicodeToUtf8_CSTR(const CStringW& wstr) {
	return UnicodeToUtf8(LPCWSTR(wstr)).c_str();
}
// ASCII��UTF8��ת  
CStringA    AsciiToUtf8_CSTR(const CStringA& str) {
	return UnicodeToUtf8_CSTR(AsciiToUnicode_CSTR(str));
}
CStringA    Utf8ToAscii_CSTR(const CStringA& str) {
	return UnicodeToAscii_CSTR(Utf8ToUnicode_CSTR(str));
}
// string �� Int ��ת  
int StringToInt(const string& str) {
	return atoi(str.c_str());
}
string IntToString(int i) {
	char ch[1024];
	memset(ch, 0, 1024);
	sprintf_s(ch, sizeof(ch), "%d", i);
	return ch;
}
string IntToString(char i) {
	char ch[1024];
	memset(ch, 0, 1024);
	sprintf_s(ch, sizeof(ch), "%c", i);
	return ch;
}
string IntToString(double i) {
	char ch[1024];
	memset(ch, 0, 1024);
	sprintf_s(ch, sizeof(ch), "%f", i);
	return ch;
}