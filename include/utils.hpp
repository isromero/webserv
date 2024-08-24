/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/14 16:29:19 by isromero          #+#    #+#             */
/*   Updated: 2024/08/24 10:54:57 by isromero         ###   ########.fr       */
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
#include <cctype>

std::string secureFilePath(const std::string &path);
std::string readFile(const std::string &filename);
std::string getFilenameAndDate(const std::string &filename);
bool saveFile(const std::string &content, const std::string &filename, std::string &savedPath);
void trim(std::string &s);
void trimTabs(std::string &s);

template <typename T>
std::string toString(const T &value);

#include "utils.tpp"

#endif
