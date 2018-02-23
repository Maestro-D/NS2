#ifndef		CONFIGURATIONREADER_HPP_
# define	CONFIGURATIONREADER_HPP_

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

class ConfigurationReader
{

private:
    std::ifstream _infile;
    std::string epurLine(std::string);
    std::pair <std::string, std::string> getLineDatas(std::string);

public:
    ConfigurationReader(std::string _configurationFilePath);
    ~ConfigurationReader();
    std::map <std::string, std::vector <std::pair <std::string, std::string>>> getDatas();
};

#endif		/* !CONFIGURATIONREADER_HPP_ */
