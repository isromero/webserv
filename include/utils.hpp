/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/14 16:29:19 by isromero          #+#    #+#             */
/*   Updated: 2024/08/16 21:36:10 by isromero         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <string>
#include <sstream>
#include <cerrno>
#include <fstream>
#include <cstring>
#include <ctime>
#include <iomanip>

std::string secureFilePath(const std::string &path);
std::string readFile(const std::string &filename);
std::string getFilenameAndDate(const std::string &filename);

template <typename T>
std::string toString(const T &value);

#include "utils.tpp"

#endif
