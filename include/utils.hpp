/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/14 16:29:19 by isromero          #+#    #+#             */
/*   Updated: 2024/08/28 18:54:15 by isromero         ###   ########.fr       */
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

template <typename T>
std::string toString(const T &value);

#include "utils.tpp"

#endif
