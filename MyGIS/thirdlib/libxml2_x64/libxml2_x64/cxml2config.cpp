#include "cxml2config.h"

#define	SUCCESS  0
#define FAIL     -1
#define LENGTH   20
#define MAX_XML_TEXTLENGTH   1024

CXml2Node::CXml2Node(xmlNodePtr pRootNode)
:m_pRootNode(pRootNode)
{

}

vector<char*> CXml2Node::getNodePath(const char* nodeName)
{
	const char* psrc  =nodeName;
	const char* pdest = NULL;

	vector<char*> nodePath;

	while(pdest = strchr( psrc, '/' ))
	{
		int length = pdest - psrc;

		char* szNode= (char *)malloc(length+1);
		memset(szNode, 0, length+1);
		memcpy(szNode, psrc, length);
#ifdef CODING_TRANSFROM
		char* szTemp = g2u(szNode);
		free(szNode);
#else
		char* szTemp = szNode;
#endif
		nodePath.push_back(szTemp);
		psrc = pdest+1;
	}

	if( NULL != psrc)
	{
		int length = strlen(nodeName) - (psrc -nodeName);
		char *szNode= (char*)malloc(length+1);
		memset(szNode, 0, length+1);
		memcpy(szNode, psrc, length);
#ifdef CODING_TRANSFROM
		char* szTemp = g2u(szNode);
		free(szNode);szNode=NULL;
#else
		char* szTemp =szNode;
#endif
		nodePath.push_back(szTemp);
	}

	return nodePath;
}

void CXml2Node::freeNodePath(vector<char*> nodePath)
{
	vector<char*>::iterator iter = nodePath.begin();

	for(; iter != nodePath.end(); iter++)
		free(*iter);

	nodePath.clear();
}



int CXml2Node::GetNodeNum(const char* nodeName, int *num)
{
	if(!IsValid())
		return FAIL;

	if(NULL == nodeName)
		return FAIL;

	xmlNodePtr pCurrentNode = NULL;

	vector<char*> nodePath;

	int iSize=0;
	if('/' == nodeName[0])     //use absolute path
	{
		nodeName +=1;
	}

	nodePath = getNodePath(nodeName);
	iSize =nodePath.size();
	if(0 == iSize)
		return FAIL;
	pCurrentNode=getNode( m_pRootNode ,nodePath[0]);
	if(NULL == pCurrentNode)
	{
		return FAIL;
	}
	
	for(int i=1; i< iSize-1; i++)
	{
		if(NULL == pCurrentNode)
			return FAIL;

		while(NULL != pCurrentNode)
		{
			if(0 == xmlStrcmp(pCurrentNode->name, BAD_CAST nodePath[i]))
			{
				break;
			}
			pCurrentNode = pCurrentNode->next;

		}
		
		if(NULL == pCurrentNode)
		{
			freeNodePath(nodePath);
			return FAIL;
		}

		pCurrentNode= pCurrentNode->xmlChildrenNode;//not including
	}	

	*num =0;
	while(NULL != pCurrentNode)
	{
		if(0 == xmlStrcmp(pCurrentNode->name, BAD_CAST nodePath[iSize-1]))
		{
			*num +=1;
		}
		pCurrentNode = pCurrentNode->next;
	}
	
	freeNodePath(nodePath);
	return SUCCESS;
}

#ifdef CODING_TRANSFROM
 char* CXml2Node::g2u(const char* szInbuf)
{
	size_t   nInLength  = strlen(szInbuf);
	size_t   nOutLength = 2* nInLength+1;
	char *szOut      = (char *)malloc(nOutLength);
	memset(szOut, 0, nOutLength);
	if(NULL == szOut)
		return NULL;

	if(0 != code_convert("gbk", "utf-8", szInbuf, nInLength, szOut, nOutLength))
	{
		return NULL;
	}
	return szOut;
}

 char* CXml2Node::u2g(const char* szInbuf)
{
	size_t   nInLength  = strlen(szInbuf);
	size_t   nOutLength = 2* nInLength+1;
	char *szOut      = (char *)malloc(nOutLength);
	memset(szOut, 0, nOutLength);
	if(NULL == szOut)
		return NULL;

	if(0 != code_convert("utf-8", "gbk",  szInbuf, nInLength, szOut, nOutLength))
	{
		return NULL;
	}
	return szOut;
}

int CXml2Node::code_convert(const char* szfrom_charset, const char* szto_charset, const char* szInBuf, size_t inLength, char* szOutBuf, size_t outLength)
{
	iconv_t cd;
	const char** pin = &szInBuf;
	char** pout= &szOutBuf;

	cd = iconv_open(szto_charset, szfrom_charset);
	if(0 == cd)
		return -1;

	memset(szOutBuf, 0, outLength);
	if(0 != iconv(cd, pin, &inLength, pout, &outLength))
	{
		iconv_close(cd);
		return -1;
	}

	return 0;
}

#endif



bool CXml2Node::IsValid()
{
	return (NULL != m_pRootNode);
}

int CXml2Node::GetValue(xmlChar** charValue)
{
	if(!IsValid())
		return FAIL;

	*charValue = xmlXPathCastNodeToString(m_pRootNode);
	return SUCCESS;
}

int  CXml2Node::GetValue(char *charValue)
{
	xmlChar *szTemp = NULL;
	if(SUCCESS == GetValue(&szTemp))
	{
#ifdef CODING_TRANSFROM
		char *szTmp = u2g((char*) szTemp);
		strcpy(charValue, szTmp);
		free(szTmp);
#else
		strcpy(charValue, (char*)szTemp);
#endif
		xmlFree(szTemp);
		return SUCCESS;
	}
	return FAIL;
}
int  CXml2Node::GetValue(short *shortValue)
{
	char szTemp[MAX_XML_TEXTLENGTH]= {0};
	if(SUCCESS == GetValue( szTemp))
	{
		char *szTmp = (char*) szTemp;
		*shortValue = atoi(szTmp);
		return SUCCESS;
	}
	return FAIL;
}
int  CXml2Node::GetValue(int *intValue)
{
	char szTemp[MAX_XML_TEXTLENGTH]= {0};
	if(SUCCESS == GetValue( szTemp))
	{
		char *szTmp = (char*) szTemp;
		*intValue = atoi(szTmp);
		return SUCCESS;
	}
	return FAIL;
}
int  CXml2Node::GetValue(long *longValue)
{
	char szTemp[MAX_XML_TEXTLENGTH]= {0};
	if(SUCCESS == GetValue( szTemp))
	{
		char *szTmp = (char*) szTemp;
		*longValue = atoi(szTmp);
		return SUCCESS;
	}
	return FAIL;
}
int  CXml2Node::GetValue(float *floatValue)
{
	char szTemp[MAX_XML_TEXTLENGTH]= {0};
	if(SUCCESS == GetValue(szTemp))
	{
		char *szTmp = (char*) szTemp;
		*floatValue = atof(szTmp);
		return SUCCESS;
	}
	return FAIL;
}	
int  CXml2Node::GetValue(double *doubleValue)
{
	char szTemp[MAX_XML_TEXTLENGTH]= {0};
	if(SUCCESS == GetValue(szTemp))
	{
		char *szTmp = (char*) szTemp;
		*doubleValue = atof(szTmp);
		return SUCCESS;
	}
	return FAIL;
}

CXml2Node CXml2Node::GetNode(const char* nodeName, int index)
{
	CXml2Node node;

	if(!IsValid())
		return node;

	if(NULL == nodeName)
		return node;

	xmlNodePtr pCurrentNode = NULL;
	vector<char*> nodePath;
	int iSize=0;

	if('/' == nodeName[0])     //use absolute path
	{
		nodePath = getNodePath(nodeName+1);
		iSize =nodePath.size();
		if(0 == iSize)
			return node;
		if(0 != xmlStrcmp(m_pRootNode->name, BAD_CAST nodePath[0]))
		{
			freeNodePath(nodePath);
			return node;
		}

		pCurrentNode = m_pRootNode;
	}
	else                       //use relative path
	{
		nodePath = getNodePath(nodeName);
		iSize =nodePath.size();
		if(0 == iSize)
			return node;

		pCurrentNode=getNode( m_pRootNode ,nodePath[0]);
		if(NULL == pCurrentNode)
		{
			return node;
		}
	}
	
	for(int i=1; i< iSize-1; i++)
	{
		pCurrentNode= pCurrentNode->xmlChildrenNode;//not including
		
		if(NULL == pCurrentNode)
			return node;

		while(NULL != pCurrentNode)
		{
			if(0 == xmlStrcmp(pCurrentNode->name, BAD_CAST nodePath[i]))
			{
				break;
			}
			
			pCurrentNode = pCurrentNode->next;
		}

		if(NULL == pCurrentNode)
		{
			freeNodePath(nodePath);
			return node;
		}
	}	
	
	
	int num=0;
	while(NULL != pCurrentNode)
	{
		if(0 == xmlStrcmp(pCurrentNode->name, BAD_CAST nodePath[iSize-1]))
		{
			if(num == index)
			{
				node = CXml2Node(pCurrentNode);
				break;
			}
			num++;
		}
		pCurrentNode = pCurrentNode->next;
	}

	freeNodePath(nodePath);
	return node;
}


int CXml2Node::GetValue(const char* nodeName, xmlChar** charValue)
{
	CXml2Node node = GetNode(nodeName);
	if(!node.IsValid())
		return FAIL;

	*charValue = xmlXPathCastNodeToString(node.m_pRootNode);
	return SUCCESS;

}
int CXml2Node::GetValue(const char* nodeName, char *charValue)
{
	xmlChar *szTemp = NULL;
	if(SUCCESS == GetValue(nodeName, &szTemp))
	{
#ifdef CODING_TRANSFROM
		char *szTmp = u2g((char*) szTemp);
		strcpy(charValue, szTmp);
		free(szTmp);
#else
		strcpy(charValue, (char*)szTemp);
#endif
		xmlFree(szTemp);
		return SUCCESS;
	}
	return FAIL;
}
int CXml2Node::GetValue(const char* nodeName, short *shortValue)
{

	char szTemp[MAX_XML_TEXTLENGTH]= {0};
	if(SUCCESS == GetValue(nodeName, szTemp))
	{
		char *szTmp = (char*) szTemp;
		*shortValue = atoi(szTmp);
		return SUCCESS;
	}
	return FAIL;
}
int CXml2Node::GetValue(const char* nodeName, int *intValue)
{
	char szTemp[MAX_XML_TEXTLENGTH]= {0};
	if(SUCCESS == GetValue(nodeName, szTemp))
	{
		char *szTmp = (char*) szTemp;
		*intValue = atoi(szTmp);
		return SUCCESS;
	}
	return FAIL;
}
int CXml2Node::GetValue(const char* nodeName, long *longValue)
{
	char szTemp[MAX_XML_TEXTLENGTH]= {0};
	if(SUCCESS == GetValue(nodeName, szTemp))
	{
		char *szTmp = (char*) szTemp;
		*longValue = atoi(szTmp);
		return SUCCESS;
	}
	return FAIL;
}
int CXml2Node::GetValue(const char* nodeName, float *floatValue)
{
	char szTemp[MAX_XML_TEXTLENGTH]= {0};
	if(SUCCESS == GetValue(nodeName, szTemp))
	{
		char *szTmp = (char*) szTemp;
		*floatValue = atof(szTmp);
		return SUCCESS;
	}
	return FAIL;
}
int CXml2Node::GetValue(const char* nodeName, double *doubleValue)
{
	char szTemp[MAX_XML_TEXTLENGTH]= {0};
	if(SUCCESS == GetValue(nodeName, szTemp))
	{
		char *szTmp = (char*) szTemp;
		*doubleValue = atof(szTmp);
		return SUCCESS;
	}
	return FAIL;
}

int  CXml2Node::SetValue(const char* nodeName, const char *charValue)
{
	CXml2Node node = GetNode(nodeName);
	if(!node.IsValid())
		node = CreateNode(nodeName);
	return node.SetValue(charValue);
}
int  CXml2Node::SetValue(const char* nodeName,const  short shortValue)
{
	CXml2Node node = GetNode(nodeName);
	if(!node.IsValid())
		node = CreateNode(nodeName);
	return node.SetValue(shortValue);
	
}
int  CXml2Node::SetValue(const char* nodeName,const  int intValue)
{
	CXml2Node node = GetNode(nodeName);
	if(!node.IsValid())
		node = CreateNode(nodeName);
	return node.SetValue(intValue);
	
}
int  CXml2Node::SetValue(const char* nodeName,const  long longValue)
{
	CXml2Node node = GetNode(nodeName);
	if(!node.IsValid())
		node = CreateNode(nodeName);
	return node.SetValue(longValue);
	
}
int  CXml2Node::SetValue(const char* nodeName,const  float floatValue)
{
	CXml2Node node = GetNode(nodeName);
	if(!node.IsValid())
		node = CreateNode(nodeName);
	return node.SetValue(floatValue);
}
int  CXml2Node::SetValue(const char* nodeName,const  double doubleValue)
{
	CXml2Node node = GetNode(nodeName);
	if(!node.IsValid())
		node =CreateNode(nodeName);
	return node.SetValue(doubleValue);
}

int CXml2Node::SetValue(const  xmlChar *charValue)
{
	if(!IsValid())
		return FAIL;

	xmlNodeSetContent(m_pRootNode, charValue);
	return SUCCESS;
}

int CXml2Node::SetValue(const  char *charValue)
{
#ifdef CODING_TRANSFROM
	char *szTemp = g2u(charValue);
#else
	char *szTemp = strdup(charValue);
#endif 
	int ret= SetValue(BAD_CAST szTemp);
	free(szTemp);
	return ret;

}
int CXml2Node::SetValue(const  short shortValue)
{
	char szBuffer[LENGTH]={0};
	sprintf(szBuffer, "%d", shortValue);
	return SetValue( szBuffer);
}
int CXml2Node::SetValue(const  int intValue)
{
	char szBuffer[LENGTH]={0};
	sprintf(szBuffer, "%d", intValue);
	return SetValue( szBuffer);
}
int CXml2Node::SetValue(const  long longValue)
{
	char szBuffer[LENGTH]={0};
	sprintf(szBuffer, "%l", longValue);
	return SetValue( szBuffer);
}
int CXml2Node::SetValue(const  float floatValue)
{
	char szBuffer[LENGTH]={0};
	sprintf(szBuffer, "%f", floatValue);
	return SetValue( szBuffer);
}
int CXml2Node::SetValue(const  double doubleValue)
{
	char szBuffer[LENGTH]={0};
	sprintf(szBuffer, "%lf", doubleValue);
	return SetValue( szBuffer);
}

int CXml2Node::GetAttribute(const char* attrName, xmlChar** charValue)
{
	if (NULL == attrName)
		return FAIL;
	if(!IsValid())
		return FAIL;
	*charValue = xmlGetProp(m_pRootNode, BAD_CAST attrName);
	return SUCCESS;
}
int  CXml2Node::GetAttribute(const char* attrName, char* charValue)
{
	xmlChar *szTemp =NULL;
	if(SUCCESS == GetAttribute(attrName, &szTemp))
	{
		if(NULL == szTemp)
			return FAIL;
		
#ifdef CODING_TRANSFROM
		char *szTmp = u2g((char*) szTemp);//unicode to chinese
		strcpy(charValue, szTmp);
		free(szTmp);
#else
		strcpy(charValue, (char*)szTemp);
#endif
		xmlFree(szTemp);
		return SUCCESS;
	}
	return FAIL;
}
int CXml2Node::GetAttribute(const char* attrName, short *shortValue)
{
	char szTemp[MAX_XML_TEXTLENGTH]={0};
	if(SUCCESS == GetAttribute(attrName, szTemp))
	{
		if(NULL == szTemp)
			return FAIL;
		char *szTmp = (char*) szTemp;
		*shortValue = atoi(szTmp);
		return SUCCESS;
	}
	return FAIL;
}
int CXml2Node::GetAttribute(const char* attrName, int *intValue)
{
	char szTemp[MAX_XML_TEXTLENGTH]={0};
	if(SUCCESS == GetAttribute(attrName, szTemp))
	{
		if(NULL == szTemp)
			return FAIL;
		char *szTmp = (char*) szTemp;
		*intValue = atoi(szTmp);
		return SUCCESS;
	}
	return FAIL;
}
int CXml2Node::GetAttribute(const char* attrName, long *longValue)
{
	char szTemp[MAX_XML_TEXTLENGTH]={0};
	if(SUCCESS == GetAttribute(attrName, szTemp))
	{
		if(NULL == szTemp)
			return FAIL;
		char *szTmp = (char*) szTemp;
		*longValue = atol(szTmp);
		return SUCCESS;
	}
	return FAIL;
}
int CXml2Node::GetAttribute(const char* attrName, float *floatValue)
{
	char szTemp[MAX_XML_TEXTLENGTH]={0};
	if(SUCCESS == GetAttribute(attrName, szTemp))
	{
		if(NULL == szTemp)
			return FAIL;
		char *szTmp = (char*) szTemp;
		*floatValue = atof(szTmp);
		return SUCCESS;
	}
	return FAIL;
}
int CXml2Node::GetAttribute(const char* attrName, double *doubleValue)
{
	char szTemp[MAX_XML_TEXTLENGTH]={0};
	if(SUCCESS == GetAttribute(attrName, szTemp))
	{
		if(NULL == szTemp)
			return FAIL;
		char *szTmp = (char*) szTemp;
		*doubleValue = atof(szTmp);
		return SUCCESS;
	}
	return FAIL;
}


int CXml2Node::SetAttribute(const char* attrName,const  xmlChar*  charValue)
{
	if(NULL == attrName)
		return FAIL;
	if(!IsValid())
		return FAIL;

	xmlSetProp(m_pRootNode,BAD_CAST attrName, charValue);
	return SUCCESS;
}
int CXml2Node::SetAttribute(const char* attrName,const  char* charValue)
{
#ifdef CODING_TRANSFROM
	char* szTemp = g2u(charValue);
	int ret = SetAttribute(attrName, BAD_CAST szTemp);
	free(szTemp);
#else
	 int ret = SetAttribute(attrName, BAD_CAST charValue);
#endif
	return ret;
}
int CXml2Node::SetAttribute(const char* attrName,const  short shortValue)
{
	char szbuffer[LENGTH]={0};
	sprintf(szbuffer,"%d", shortValue);
	return SetAttribute(attrName,  szbuffer);
}
int CXml2Node::SetAttribute(const char* attrName,const  int intValue)
{
	char szbuffer[LENGTH]={0};
	sprintf(szbuffer,"%d", intValue);
	return SetAttribute(attrName,  szbuffer);
}
int CXml2Node::SetAttribute(const char* attrName,const  long longValue)
{
	char szbuffer[LENGTH]={0};
	sprintf(szbuffer,"%l", longValue);
	return SetAttribute(attrName,  szbuffer);
}
int CXml2Node::SetAttribute(const char* attrName,const  float floatValue)
{
	char szbuffer[LENGTH]={0};
	sprintf(szbuffer,"%f", floatValue);
	return SetAttribute(attrName,  szbuffer);
}
int CXml2Node::SetAttribute(const char* attrName,const  double doubleValue)
{
	char szbuffer[LENGTH]={0};
	sprintf(szbuffer,"%lf", doubleValue);
	return SetAttribute(attrName,  szbuffer);
}

CXml2Node CXml2Node::AppendNode(const char* nodeName)
{
	CXml2Node node;
	if(!IsValid())
		return node;
	if(NULL == nodeName)
		return node;

	CXml2Node tempNode = GetNode(nodeName);
	if (!tempNode.IsValid())
	{
		node = CreateNode(nodeName);
	}
	else
	{
		const char *pdest=strrchr( nodeName, '/' );
		if(NULL == pdest)
			pdest = nodeName;
		else
			pdest +=1;

		xmlNodePtr pNode= tempNode.m_pRootNode;
		while(NULL != pNode->next)
		{
			pNode = pNode->next;
		}

		xmlNodePtr pElement = xmlNewNode(NULL, BAD_CAST pdest);
		xmlAddSibling(pNode,  pElement);
		node = CXml2Node( pElement);
	}
	
	return node; 
}

int CXml2Node::RemoveNode(const char* nodeName,int index)
{
	if(!IsValid())
		return FAIL;
	if(NULL == nodeName)
		return FAIL;

	CXml2Node node = GetNode(nodeName, index);
	if (!node.IsValid())
	{
		return FAIL;
	}
	else
	{
		xmlNodePtr pNode = node.m_pRootNode;
		xmlUnlinkNode(pNode);
		xmlFreeNode(pNode);
		return SUCCESS; 
	}
}


CXml2Node CXml2Node::CreateNode(const char* nodeName)
{
	CXml2Node node;

	if(!IsValid())
	{
		return node;
	}

	xmlNodePtr pCurrentNode = NULL;
	vector<char*> nodePath;

	int iSize = 0;
	if('/' == nodeName[0])     
	{
		nodeName +=1;
	}
	
	nodePath = getNodePath(nodeName);
	iSize =nodePath.size();
	if(0 == iSize)
		return node;
	
	pCurrentNode=getNode( m_pRootNode ,nodePath[0]);
	
	if(1 == iSize)
	{
			pCurrentNode=xmlNewNode(NULL, BAD_CAST nodePath[0]);
			xmlAddChild(m_pRootNode, pCurrentNode);		
	}
	else
	{
		if (NULL == pCurrentNode) {
			pCurrentNode=xmlNewNode(NULL, BAD_CAST nodePath[0]);
			xmlAddChild(m_pRootNode, pCurrentNode);
		}
	}
	
	xmlNodePtr pTempNode = NULL;
	for(int i=1; i<iSize;i++)
	{
		pTempNode = getNode(pCurrentNode, nodePath[i]);
		if(i == iSize-1)
		{
			if(NULL == pTempNode)
			{
				pTempNode = xmlNewNode(NULL,BAD_CAST nodePath[i]);
				xmlAddChild(pCurrentNode, pTempNode);		
			}
		}
		else
		{
			pTempNode = xmlNewNode(NULL,BAD_CAST nodePath[i]);
			xmlAddChild(pCurrentNode, pTempNode);
		}
		pCurrentNode = pTempNode;
	}

	freeNodePath(nodePath);

	node = CXml2Node( pCurrentNode);
	return node; 
}


xmlNodePtr CXml2Node::getNode(xmlNodePtr pCurrentNode, const char* nodeName)
{
	if(NULL == pCurrentNode)
		return NULL;
	xmlNodePtr pNode = NULL;

	pCurrentNode =pCurrentNode->xmlChildrenNode;

	while(pCurrentNode)
	{
		if(0 == xmlStrcmp(pCurrentNode->name, BAD_CAST nodeName))
		{
			pNode = pCurrentNode;
			break;
		}
		pCurrentNode = pCurrentNode->next;
	}
	return pNode;
}


//////////////////////
CXml2Document::CXml2Document(const char* xmlFileName)
:CXml2Node(), m_pDoc(NULL)
{
	xmlDocPtr pDoc = NULL;

	if (NULL == (pDoc = xmlReadFile(xmlFileName,"UTF-8",XML_PARSE_NOBLANKS)))
	{
		std::cerr << "Unable to Open Xml File"<< std::endl;
		xmlFreeDoc(m_pDoc); m_pDoc=NULL;
		return ;
	}

	m_pDoc =pDoc;

	xmlNodePtr pRootNode = NULL;
	if( NULL == (pRootNode = xmlDocGetRootElement(pDoc)))
	{
		std::cerr << "Unable to Get Root Node"<< std::endl;
		return ;
	}

	m_pRootNode = pRootNode;
}

CXml2Document::CXml2Document()
:CXml2Node(), m_pDoc(NULL)
{

}

int CXml2Document::SaveFile(const char* xmlFileName)
{
	if(!IsValid())
		return FAIL;

	xmlSaveFormatFileEnc(xmlFileName, m_pDoc,"UTF-8", 1);

	return SUCCESS;
}

CXml2Document::~CXml2Document()
{
	if( NULL != m_pDoc)
		xmlFreeDoc(m_pDoc); m_pDoc=NULL;
}

bool CXml2Document::IsValid()
{
	return (NULL != m_pRootNode  && NULL != m_pDoc);
}



CXml2Node CXml2Document::CreateNode(const char* nodeName)
{
	CXml2Node node;
	if(NULL == nodeName)
		return node;

	if(NULL == m_pDoc)
		m_pDoc = xmlNewDoc(BAD_CAST "1.0");

	vector<char*> nodePath;

	if('/' == nodeName[0]) 
	{
		nodeName +=1; 
	}

	nodePath = getNodePath(nodeName);
	int iSize =nodePath.size();
	if(0 == iSize)
		return node;

	if(NULL == m_pRootNode)
	{
		xmlNodePtr pRootNode = xmlNewNode(NULL,BAD_CAST nodePath[0]);
		xmlDocSetRootElement(m_pDoc, pRootNode);
		m_pRootNode = pRootNode;
	}
	else
	{
		if(0 != xmlStrcmp(m_pRootNode->name, BAD_CAST nodePath[0]))
		{
			freeNodePath(nodePath);
			return node;
		}
	}
	
	xmlNodePtr pNode = m_pRootNode;
	node= CXml2Node(pNode);

	xmlNodePtr pTempNode = NULL;
	for(int i=1; i<iSize;i++)
	{
		pTempNode = getNode(pNode, nodePath[i]);
		if(NULL == pTempNode)
		{
			pTempNode = xmlNewNode(NULL, BAD_CAST nodePath[i]);
			xmlAddChild(pNode, pTempNode);		
		}
		pNode = pTempNode;
		node= CXml2Node(pNode);
	}

	freeNodePath(nodePath);
	return node;
}


CXml2Node CXml2Document::GetNode(const char* nodeName, int index)
{
	CXml2Node node;

	if(!IsValid())
		return node;

	if(NULL == nodeName)
		return node;

	xmlNodePtr pCurrentNode = NULL;
	vector<char*> nodePath;
	int iSize=0;

	if('/' == nodeName[0])     //use absolute path
	{
		nodeName += 1;
	}

	nodePath = getNodePath(nodeName);
	iSize =nodePath.size();
	if(0 == iSize)
		return node;
	if(0 != xmlStrcmp(m_pRootNode->name, BAD_CAST nodePath[0]))
	{
		freeNodePath(nodePath);
		return node;
	}

	pCurrentNode = m_pRootNode;
	if(1 == iSize)
	{
		freeNodePath(nodePath);
		return CXml2Node(pCurrentNode);
	}

	pCurrentNode = pCurrentNode->xmlChildrenNode;
	for(int i=1; i< iSize-1; i++)
	{
		if(NULL == pCurrentNode)
			return node;

		while(NULL != pCurrentNode)
		{
			if(0 == xmlStrcmp(pCurrentNode->name, BAD_CAST nodePath[i]))
			{
				break;
			}
			pCurrentNode = pCurrentNode->next;

		}

		if(NULL == pCurrentNode)
		{
			freeNodePath(nodePath);
			return node;
		}

		pCurrentNode= pCurrentNode->xmlChildrenNode;//not including
	}	

	int num=0;
	while(NULL != pCurrentNode)
	{
		if(0 == xmlStrcmp(pCurrentNode->name, BAD_CAST nodePath[iSize-1]))
		{
			if(num == index)
			{
				node = CXml2Node(pCurrentNode);
				break;
			}
			num++;
		}
		pCurrentNode = pCurrentNode->next;
	}

	freeNodePath(nodePath);
	return node;
}


int CXml2Document::GetNodeNum(const char* nodeName, int *num)
{
	if(!IsValid())
		return FAIL;

	if(NULL == nodeName)
		return FAIL;

	xmlNodePtr pCurrentNode = NULL;

	vector<char*> nodePath;

	int iSize=0;
	if('/' == nodeName[0])     //use absolute path
	{
		nodeName +=1;
	}

	nodePath = getNodePath(nodeName);
	iSize =nodePath.size();
	if(0 == iSize)
		return FAIL;
	if(0 != xmlStrcmp(m_pRootNode->name, BAD_CAST nodePath[0]))
	{
		freeNodePath(nodePath);
		return FAIL;
	}
	pCurrentNode = m_pRootNode;

	
	for(int i=1; i< iSize-1; i++)
	{
		if(NULL == pCurrentNode)
			return FAIL;

		while(NULL != pCurrentNode)
		{
			if(0 == xmlStrcmp(pCurrentNode->name, BAD_CAST nodePath[i]))
			{
				break;
			}
			pCurrentNode = pCurrentNode->next;

		}

		if(NULL == pCurrentNode)
		{
			freeNodePath(nodePath);
			return FAIL;
		}

		pCurrentNode= pCurrentNode->xmlChildrenNode;//not including
	}	

	*num =0;
	while(NULL != pCurrentNode)
	{
		if(0 == xmlStrcmp(pCurrentNode->name, BAD_CAST nodePath[iSize-1]))
		{
			*num +=1;
		}
		pCurrentNode = pCurrentNode->next;
	}

	freeNodePath(nodePath);
	return SUCCESS;
}

