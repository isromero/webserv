/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/12 14:14:08 by isromero          #+#    #+#             */
/*   Updated: 2024/08/12 14:14:08 by isromero         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

int main(int argc, char **argv)
{
	if (argc > 2)
	{
		std::cerr << "Usage: " << argv[0] << " <config_file>" << " OR " << argv[0] << std::endl;
		return 1;
	}

	try
	{
		if (argc == 1)
		{
			Server server;
#if defined(__linux__)
			server.runLinux();
#elif defined(__APPLE__)
			server.runMac();
#endif
		}

		else
		{
			Server server(argv[1]);
#if defined(__linux__)
			server.runLinux();
#elif defined(__APPLE__)
			server.runMac();
#endif
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}

	return 0;
}
