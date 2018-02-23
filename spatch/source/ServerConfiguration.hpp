#ifndef		SERVERCONFIGURATION_HPP_
# define	SERVERCONFIGURATION_HPP_

#include <string>

class ServerConfiguration
{

public:
    ServerConfiguration(std::string _port);
    ~ServerConfiguration();
    const std::string port;
};

#endif		/* !SERVERCONFIGURATION_HPP_ */
