#include "Channel/Channel.hpp"
#include "Server/Server.hpp"
#include <algorithm>


void	execute_operator_cmd(const Server::MsgTokens tokenized_message, Client &operator_client, Channel &channel, Server &server)
{
	std::vector<std::string> operator_list = channel.get_operator_list();
	if (std::find(operator_list.begin(), operator_list.end(), operator_client.get_nick_name()) == operator_list.end())
		return;
	std::string	command = tokenized_message.command;
	if (command == "KICK")
		kick_client(tokenized_message, channel, tokenized_message.params[1], operator_client, server);
	else if (command == "INVITE")
		return;//handle Invite
	else if (command == "TOPIC")
		return;//handle topic
	else if (command == "MODE")
		return;//handle mode
	return;
}

void kick_client(const Server::MsgTokens tokenized_message, Channel &channel, std::string target_nickname, Client &operator_client, Server &server)
{
	std::vector<std::string> client_list = channel.get_client_list();
	if (std::find(client_list.begin(), client_list.end(), target_nickname) == client_list.end())
    {
        putstr_fd("client: [" + target_nickname + "] is not in the Channel: [" + channel.get_name() + "]\n", operator_client.get_sockfd());
        return;
    }
	if (target_nickname == operator_client.get_nick_name())
		return;
	//if target client is found in the channel, remove them from client list, and operator list
	channel.remove_client_from_list(target_nickname);
	channel.remove_operator_from_channel(target_nickname);
	//use default, or trailing as comment
	std::string comment;
	if (tokenized_message.trailing.empty())
		comment = operator_client.get_nick_name();
	else
		comment = tokenized_message.trailing;
	//build kick message
	std::string kick_message = ":"
								+ operator_client.get_nick_name() + "!"
								+ operator_client.get_user_name() + "@localhost KICK "
								+ channel.get_name() + " "
								+ target_nickname + " :"
								+ comment + "\n";
	//send kick message to all clients in the channel
	for (std::vector<std::string>::const_iterator it = client_list.begin(); it != client_list.end(); ++it)
	{
		Client* client = server.get_client_by_nickname(*it);
		if (client)
			putstr_fd(kick_message, client->get_sockfd());
	}
	//send message to kicked client
	int target_fd = server.get_client_by_nickname(target_nickname)->get_sockfd();
	putstr_fd("You were kicked from the Channel:" + channel.get_name(), target_fd);
}