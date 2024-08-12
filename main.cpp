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
	(void)argc;
	(void)argv;
	Server server;

#if defined(__linux__)
	server.runLinux();
#elif defined(__APPLE__)
	server.runMac();
#endif

	return 0;
}
