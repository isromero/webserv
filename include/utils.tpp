/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.tpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/14 16:34:03 by isromero          #+#    #+#             */
/*   Updated: 2024/08/14 16:34:30 by isromero         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

template <typename T>
std::string toString(const T &value)
{
	std::ostringstream ss;
	ss << value;
	return ss.str();
}
