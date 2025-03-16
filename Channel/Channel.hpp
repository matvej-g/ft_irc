
#pragma once

// #include "Server.hpp"
#include <iostream>
#include <vector>

class Channel
{
    private:
        std::string                 _name;
        std::vector<std::string>    _client;
		std::vector<std::string>	_operators;

    public:
		unsigned int						user_limit = 1024;
		bool								topic_restriction = false;
		bool								invite_only = false;
        void                              set_name(std::string new_name);
        std::string                       get_name();
        void                              add_client_to_list(std::string client_name);
        void                              remove_client_from_list(std::string client_name);
        const std::vector<std::string>&   get_client_list() const;
		void								add_operator_to_channel(std::string client_name);
		void								remove_operator_from_channel(std::string client_name);
		std::vector<std::string>			get_operator_list();
};
