#ifndef __TFTJ_OUTPUT_READER__
#define __TFTJ_OUTPUT_READER__

#include <string>

class OutputReader
{
public:
	virtual void newRecord(const std::string& str) = 0;
	virtual void endRecord() = 0;
	virtual void endParsing() = 0;
	virtual void received(int keyIndex, int startIndex, int endIndex) = 0;
	virtual ~OutputReader() {};
};

class OutputCsv : public OutputReader
{
	const std::string* val;
	std::vector<std::string> keys;
	std::ofstream myfile;
	std::vector<std::pair<bool, std::string>> values;

public:
	OutputCsv(std::vector<std::string>& a, const std::string& fileName, bool printHeader = false)
	{
		keys = a;
		values.resize(keys.size(), std::make_pair(false, ""));
		myfile.open(fileName);

		if (printHeader)
		{
			for (int i = 0; i < keys.size() - 1; ++i)
			{
				myfile << keys[i] << '\t';
			}
			myfile << keys[keys.size() - 1] << '\n';
		}
	}

	virtual void newRecord(const std::string& str)
	{
		val = &str;
		for (int i = 0; i < values.size(); ++i)
		{
			values[i] = std::make_pair(false, "");
		}
	}

	virtual void endRecord()
	{
		for (int i = 0; i < keys.size() - 1; ++i)
		{
			myfile << values[i].second << '\t';
		}
		myfile << values[keys.size() - 1].second << '\n';
	}

	virtual void endParsing()
	{
		myfile.close();
	}

	virtual void received(int keyIndex, int startIndex, int endIndex)
	{
		values[keyIndex] = std::make_pair(true, val->substr(startIndex, endIndex - startIndex + 1));
	}

	virtual ~OutputCsv() {};
};


#endif