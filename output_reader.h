#ifndef __TFTJ_OUTPUT_READER__
#define __TFTJ_OUTPUT_READER__

#include <string>

class OutputReader
{
public:
	virtual void newRecord(const std::string& str) = 0;
	virtual void endParsing() = 0;
	virtual void received(int keyIndex, int startIndex, int endIndex) = 0;
	virtual ~OutputReader() {};
};

class BaseOutputReader : public OutputReader
{
	const std::string* val;
public:
	virtual void newRecord(const std::string& str)
	{
		val = &str;
	}

	virtual void endParsing() 
	{
	}

	virtual void received(int keyIndex, int startIndex, int endIndex)
	{

	}


	virtual ~BaseOutputReader() {};
};



#endif