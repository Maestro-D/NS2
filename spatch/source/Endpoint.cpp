#include "Endpoint.hpp"

Endpoint::Endpoint(std::string _name, std::string _ipAdress, std::string _port)
    : name(_name), ipAdress(_ipAdress), port(_port)
{
    this->usersAccessControl = {};
}

Endpoint::~Endpoint()
{
}
