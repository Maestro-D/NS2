#ifndef		ENDPOINT_HPP_
# define	ENDPOINT_HPP_

#include <string>
#include <map>
#include <vector>

class User;

class Endpoint
{

public:
    Endpoint(std::string _name, std::string _ipAdress, std::string _port);
    ~Endpoint();
    const std::string name;
    const std::string ipAdress;
    const std::string port;
    std::map<User *, std::vector<std::string>> usersAccessControl;
};

#endif		/* !ENDPOINT_HPP_ */
