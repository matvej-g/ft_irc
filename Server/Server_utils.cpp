
#include "Server.hpp"

int Server::get_client_index_through_name(std::string client_name)
{
    for (unsigned int i = 0; i < this->_client.size(); i++)
    {
        if (client_name == this->_client[i].get_nick_name())
            return(i);
    }
    return (-1);
}

int Server::get_channel_index_through_name(std::string channel_name)
{
    for (unsigned int i = 0; i < this->_channel.size(); i++)
    {
        if (channel_name == this->_channel[i].get_name())
            return (i);
    }
    return (-1);
}

int Server::get_sockfd()
{
    return (this->_sockfd);
}

int Server::get_port()
{
    return (this->_port);
}

struct sockaddr_in Server::get_server_address()
{
    return (this->_server_address);
}

Client* Server::get_client_by_nickname(const std::string &nickname)
{
	int index = get_client_index_through_name(nickname);
	if (index == -1)
		return (NULL);
	return &(_client[index]);
}

Channel* Server::get_channel_by_name(const std::string &channel_name)
{
	int index = get_channel_index_through_name(channel_name);
	if (index < 0)
		return nullptr;
	return &(_channel[index]);
}