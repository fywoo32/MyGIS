#include "cxml2config.h"

void readfun()
{
	CXml2Document doc("configure.xml");
	
	
	char logDirpath[2048];
	if (doc.GetValue("Data/DataSource/LandCoverFilepath", logDirpath) != 0)
	{
		cout<<"LOG FILE LOAD ERROR!"<<endl;
		return;
	}
	
	double percent;
	if (doc.GetValue("Data/Parameters/RandomDecisionForest/DecisionForestSetPersent", &percent) != 0)
	{
		cout<<"LOG FILE LOAD ERROR!"<<endl;
		return;
	}
	
	
	///////////////////////////
	CXml2Node node0 = doc.GetNode("Data/DataSource/LandCoverFiles");
	int nDataCount;
	if (0 != node0.GetAttribute("Filenum", &nDataCount))
	{
		_ilog("Load NODE_AUXLIARY_DATA -Filenum Failed!");
		return;
	}
}


void writefun()
{
	CXml2Document doc("configure_new.xml");
	CXml2Node node0 = doc.CreateNode("Data");
	
	//在node0下面新建
	CXml2Node node1 = node0.CreateNode("HelloPara1");
	node1.SetValue(0.0032);
	
	doc.SaveFile("configure_new.xml");
}