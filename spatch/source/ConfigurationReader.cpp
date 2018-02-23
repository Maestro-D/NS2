#include "ConfigurationReader.hpp"

ConfigurationReader::ConfigurationReader(std::string configurationFilePath)
    : _infile(configurationFilePath.c_str())
{
    if (!this->_infile) {
        std::cerr << "Can't open file " << configurationFilePath << std::endl;
        std::exit(-1);
    }
}

ConfigurationReader::~ConfigurationReader()
{
    this->_infile.close();
}

std::map <std::string, std::vector <std::pair <std::string, std::string>>> ConfigurationReader::getDatas()
{
    std::map <std::string, std::vector <std::pair <std::string, std::string>>> datas;
    std::string line;
    std::string section;

    while (std::getline(this->_infile, line)) {
        line = this->epurLine(line);
        if (line[0] == '[') {
            section = line.substr(1, (line.find(']') - 1));
            datas[section] = {};
        } else if (line.length() > 1 && line != "" && line.find('=') != -1) {
            if (line.find('=') > 1 && (line.length() - 1) - (line.find('=') + 1) > 1) {
                datas[section].push_back(getLineDatas(line));
            } else {
                std::cerr << "Can't read spatch config file correctly, please check [" << section << "] configuration." << std::endl;
            std::exit(-1);
            }
        }
    }
    return (datas);
}

std::string ConfigurationReader::epurLine(std::string line) {
    std::string epuredLine;

    for (int i = 0; line[i] && line[i] != '#' && line[i] != '\n' && line[i] != '\r'; i++) {
        if (line[i] != ' ') {
            epuredLine.push_back(line[i]);
        }
    }
    return (epuredLine);
}

std::pair <std::string, std::string> ConfigurationReader::getLineDatas(std::string line) {
    std::string keys;
    std::string values;

    keys = line.substr(0, (line.find('=')));
    values = line.substr((line.find('=') + 1), (line.length() - 1));
    return (std::pair <std::string, std::string>(keys, values));
}
