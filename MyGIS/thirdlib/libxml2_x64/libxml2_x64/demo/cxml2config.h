#ifndef CXML2CONFIG_H_
#define CXML2CONFIG_H_

#include "libxml/xpath.h"
#include "libxml/parser.h"
#include <iostream>
#include <vector>
using namespace std;

#if defined(WINDOWS) ||defined(WIN32)
#define CODING_TRANSFROM
#endif
/*
* @brief CXml2Node is the basic unit to read/write xml with the help of libxml2
*  for Node, it read it sub items by default
*/
class CXml2Node
{
public:
	CXml2Node(xmlNodePtr pRootNode=NULL);
	bool IsValid();
	virtual int GetNodeNum(const char* nodeName, int *num);
	virtual CXml2Node GetNode(const char* nodeName, int index=0);
	virtual CXml2Node CreateNode(const char* nodeName);
	CXml2Node AppendNode(const char* nodeName);
	int RemoveNode(const char* nodeName, int index=0);
	
	int GetValue(char *charValue);
	int GetValue(short *shortValue);
	int GetValue(int *intValue);
	int GetValue(long *longValue);
	int GetValue(float *floatValue);
	int GetValue(double *doubleValue);

	int GetValue(const char* nodeName, char *charValue);
	int GetValue(const char* nodeName, short *shortValue);
	int GetValue(const char* nodeName, int *intValue);
	int GetValue(const char* nodeName, long *longValue);
	int GetValue(const char* nodeName, float *floatValue);
	int GetValue(const char* nodeName, double *doubleValue);

	int SetValue(const char* nodeName, const char *charValue);
	int SetValue(const char* nodeName, const short shortValue);
	int SetValue(const char* nodeName, const int intValue);
	int SetValue(const char* nodeName, const long longValue);
	int SetValue(const char* nodeName, const float floatValue);
	int SetValue(const char* nodeName, const double doubleValue);

	int SetValue( const char *charValue);
	int SetValue( const short shortValue);
	int SetValue( const int intValue);
	int SetValue( const long longValue);
	int SetValue( const float floatValue);
	int SetValue( const double doubleValue);

	int GetAttribute(const char* attrName, char* charValue);
	int GetAttribute(const char* attrName, short *shortValue);
	int GetAttribute(const char* attrName, int *intValue);
	int GetAttribute(const char* attrName, long *longValue);
	int GetAttribute(const char* attrName, float *floatValue);
	int GetAttribute(const char* attrName, double *doubleValue);

	int SetAttribute(const char* attrName, const char* charValue);
	int SetAttribute(const char* attrName, const short shortValue);
	int SetAttribute(const char* attrName, const int intValue);
	int SetAttribute(const char* attrName, const long longValue);
	int SetAttribute(const char* attrName, const float floatValue);
	int SetAttribute(const char* attrName, const double doubleValue);
	
#ifdef CODING_TRANSFROM
	static  char* g2u(const char* szInbuf);
	static	char* u2g(const char* szInbuf);
#endif
protected:
	xmlNodePtr m_pRootNode;
#ifdef CODING_TRANSFROM
	//help functions use in CXml2Node
	static	int code_convert(const char* szfrom_charset, const char* szto_charset, const char* szInBuf, size_t inLength, char* szOutBuf, size_t outLength);
#endif
	
	static	vector<char*> getNodePath(const char* nodeName);
	static	void freeNodePath(vector<char*> nodePath);
	xmlNodePtr getNode(xmlNodePtr pNode, const char* nodeName);
	
	int GetValue(xmlChar** charValue);
	int GetValue(const char* nodeName, xmlChar** charValue);
	int SetValue(const xmlChar* charValue);
	int GetAttribute(const char* attrName, xmlChar** charValue);
	int SetAttribute(const char* attrName, const xmlChar*  charValue);
};

/*
 * @brief CXml2Document, the document read from root node
*/
class CXml2Document
	:public CXml2Node
{
public:
	CXml2Document(const char* xmlFileName);
	CXml2Document();

	bool IsValid();
	int  SaveFile(const char* xmlFileName);

	virtual CXml2Node GetNode(const char* nodeName, int index=0);
	virtual CXml2Node CreateNode(const char* nodeName);
	virtual int GetNodeNum(const char* nodeName, int *num);
	virtual ~CXml2Document();
private:	
	xmlDocPtr m_pDoc;
};

#endif /*CXML2CONFIG_H_*/
