#include "Channel/Channel.hpp"
#include "Server/Server.hpp"
#include <algorithm>


void	execute_operator_cmd(const Server::MsgTokens &tokenized_message, Client &operator_client, Server &server)
{
	std::cout << "\n" << std::endl;
	std::cout << "prefix: " << tokenized_message.prefix << std::endl;
	std::cout << "command: " << tokenized_message.command << std::endl;
	std::cout << "params: ";
	for (auto &it : tokenized_message.params)
	{
		std::cout << it << "-";
	}
	std::cout << std::endl;
	std::cout << "trailing: " << tokenized_message.trailing << std::endl;
	std::cout << "\n" << std::endl;

	std::string	command = tokenized_message.command;
	if (command == "KICK")
	{
		//kick <channel> <client> [<optional comment>], param[1] = channel, param[2] = tager client
		if (tokenized_message.params.size() < 2)
		{
			std::string errorMsg = ":server 461 "
									+ operator_client.get_nick_name()
									+ " KICK : use /kick <channel> <client> [<optional comment>]\r\n";
			putstr_fd(errorMsg, operator_client.get_sockfd());
		}
		Channel *channel = server.get_channel_by_name(tokenized_message.params[1]);
		if (!channel)
		{
			std::string errorMsg = ":server 403 "
									+ operator_client.get_nick_name() + " "
									+ tokenized_message.params[1]
									+ " :No such channel\r\n";
			putstr_fd(errorMsg, operator_client.get_sockfd());
			return;
		}
		std::vector<std::string> operator_list = channel->get_operator_list();
		if (std::find(operator_list.begin(), operator_list.end(), operator_client.get_nick_name()) == operator_list.end())
		{
			std::string errorMsg = ":server 482 " 
									+ operator_client.get_nick_name() + " " 
									+ channel->get_name() 
									+ " :You're not channel operator\r\n";
			putstr_fd(errorMsg, operator_client.get_sockfd());
			return;
		} 
		kick_client(tokenized_message, *channel, tokenized_message.params[1], operator_client, server);
	}
	else if (command == "INVITE")
	{
		//invite <client> <channel>, param[1] = target client, param[2] = channel
		if (tokenized_message.params.size() < 2)
		{
			std::string errorMsg = ":server 461 "
									+ operator_client.get_nick_name()
									+ " INVITE : use /invite <client> <channel>\r\n";
			putstr_fd(errorMsg, operator_client.get_sockfd());
			return;
		}
		Channel *channel = server.get_channel_by_name(tokenized_message.params[1]);
		if (!channel)
		{
			std::string errorMsg = ":server 403 "
									+ operator_client.get_nick_name() + " "
									+ tokenized_message.params[1]
									+ " :No such channel\r\n";
			putstr_fd(errorMsg, operator_client.get_sockfd());
			return;
		}
		std::vector<std::string> operator_list = channel->get_operator_list();
		if (std::find(operator_list.begin(), operator_list.end(), operator_client.get_nick_name()) == operator_list.end())
		{
			std::string errorMsg = ":server 482 " 
									+ operator_client.get_nick_name() + " " 
									+ channel->get_name() 
									+ " :You're not channel operator\r\n";
			putstr_fd(errorMsg, operator_client.get_sockfd());
			return;
		} 
		invite_client(*channel, tokenized_message.params[0], operator_client, server);
	}
	else if (command == "TOPIC")
	{
		//topic <channel> [<topic>], param[1] = channel
		if (tokenized_message.params.empty())
		{
			std::string errorMsg = ":server 461 "
									+ operator_client.get_nick_name()
									+ " TOPIC : use /topic <channel> [<topic>]\r\n";
			putstr_fd(errorMsg, operator_client.get_sockfd());
			return;
		}
		Channel *channel = server.get_channel_by_name(tokenized_message.params[0]);
		if (!channel)
		{
			std::string errorMsg = ":server 403 "
									+ operator_client.get_nick_name() + " "
									+ tokenized_message.params[0]
									+ " :No such channel\r\n";
			putstr_fd(errorMsg, operator_client.get_sockfd());
			return;
		}
		set_topic(tokenized_message, *channel, operator_client, server);
	}
	else if (command == "MODE")
		return;//handle mode
	return;
}

void kick_client(const Server::MsgTokens &tokenized_message, Channel &channel, std::string target_nickname, Client &operator_client, Server &server)
{
	std::vector<std::string> client_list = channel.get_client_list();
	if (std::find(client_list.begin(), client_list.end(), target_nickname) == client_list.end())
    {
		std::string errorMsg = ":server 441 " 
								+ operator_client.get_nick_name() + " " 
								+ channel.get_name() 
								+ " :Client is not in the Channel\r\n";
		putstr_fd(errorMsg, operator_client.get_sockfd());
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
}


void invite_client(Channel &channel, std::string target_nickname, Client &operator_client, Server &server)
{
	Client* target_client = server.get_client_by_nickname(target_nickname);
	std::vector<std::string> client_list = channel.get_client_list();
	if (!target_client)
    {
		std::string errorMsg = ":server 401 " 
								+ operator_client.get_nick_name() + " " 
								+ channel.get_name() 
								+ " :no such client\r\n";
		putstr_fd(errorMsg, operator_client.get_sockfd());
		return;
    }
	//check if client is in the channel
	if (std::find(client_list.begin(), client_list.end(), target_nickname) == client_list.end())
	{
		//build invite message for the target_client
		std::string inviteMsg = ":" + operator_client.get_nick_name()
								+ " INVITE " + target_nickname
								+ " " + channel.get_name() + "\r\n";
		putstr_fd(inviteMsg, target_client->get_sockfd());
		//send confirmation message to client, that the invite was send successfully
		std::string reply = ":server 341 "
							+ operator_client.get_nick_name() + " "
							+ target_nickname + " " + channel.get_name() + "\r\n";
		putstr_fd(reply, operator_client.get_sockfd());
	}
	else //target is already in the channel
	{
		std::string errorMsg = ":server 443 " 
								+ operator_client.get_nick_name() + " " 
								+ channel.get_name() 
								+ " :User is already on channel\r\n";
		putstr_fd(errorMsg, operator_client.get_sockfd());
		return;
	}
}

void set_topic(const Server::MsgTokens &tokenized_message, Channel &channel, Client &operator_client, Server &server)
{
	// /topic room1 → Query the topic.
	// /topic room1 :<new topic> → Set a new topic.
	// /topic room1 : → Unset the topic.
	std::vector<std::string> client_list = channel.get_client_list();
	if (std::find(client_list.begin(), client_list.end(), operator_client.get_nick_name()) == client_list.end())
	{
		std::string errorMsg = ":server 442 " 
								+ operator_client.get_nick_name() + " " 
								+ channel.get_name() 
								+ " :Client not in the Channel\r\n";
		putstr_fd(errorMsg, operator_client.get_sockfd());
		return;
	}
	if (tokenized_message.trailing.empty())
	{
		if (channel.get_topic().empty())
		{
			std::string reply = ":server 331 "
								+ operator_client.get_nick_name() + " "
								+ channel.get_name() 
								+ ": No topic set\r\n";
			putstr_fd(reply, operator_client.get_sockfd());
			return;
		}
		else
		{
			std::string reply = ":server 332 "
								+ operator_client.get_nick_name() + " "
								+ channel.get_name() 
								+ " :" + channel.get_topic() + "\r\n";
			putstr_fd(reply, operator_client.get_sockfd());
			return;
		}
	}
	if (channel.topic_restriction)
	{
		std::vector<std::string> operator_list = channel.get_operator_list();
		if (std::find(operator_list.begin(), operator_list.end(), operator_client.get_nick_name()) == operator_list.end())
		{
			std::string errorMsg = ":server 482 " 
									+ operator_client.get_nick_name() + " " 
									+ channel.get_name() 
									+ " :You're not channel operator\r\n";
			putstr_fd(errorMsg, operator_client.get_sockfd());
			return;
		}
	}
	std::string new_topic = tokenized_message.trailing;
	if (!new_topic.empty() && new_topic[0] == ':')
		new_topic.erase(0, 1);
	channel.set_topic(new_topic);
	//send message to all clients
	std::string topic_message = ":"
								+ operator_client.get_nick_name() + "!"
								+ operator_client.get_user_name() + "@localhost TOPIC "
								+ channel.get_name() + " "
								+ " :"+ channel.get_topic() + "\r\n";
	//notify all clients in the channel for topic change
	for (std::vector<std::string>::const_iterator it = client_list.begin(); it != client_list.end(); ++it)
	{
		Client* client = server.get_client_by_nickname(*it);
		if (client)
			putstr_fd(topic_message, client->get_sockfd());
	}
	return;
}