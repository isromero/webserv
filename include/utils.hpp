/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/14 16:29:19 by isromero          #+#    #+#             */
/*   Updated: 2024/09/02 19:07:40 by isromero         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include "includes.hpp"

std::string secureFilePath(const std::string &path);
std::string readFile(const std::string &filename);
std::string getFilenameAndDate(const std::string &filename);
void trim(std::string &s);
void trimTabs(std::string &s);
const std::string extractMainPath(const std::string &fullPath);
const std::string extractCGIMainPath(const std::string &path);
const std::string extractQueryString(std::string &scriptName);
const std::string extractPathInfo(std::string &scriptName);

template <typename T>
std::string toString(const T &value);

#include "utils.tpp"

#endif
