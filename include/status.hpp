/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   status.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adgutier <adgutier@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/14 16:42:33 by isromero          #+#    #+#             */
/*   Updated: 2024/09/08 17:34:07 by adgutier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef STATUS_HPP
#define STATUS_HPP

// NO_STATUS_CODE is when request parsing is correct
// ERROR_STATUS_CODE are the possible errors that can be returned because request
// SUCCESS_STATUS_CODE are the possible success that can be returned because request
enum StatusCode
{
	NO_STATUS_CODE,
	ERROR_400,
	ERROR_403,
	ERROR_404,
	ERROR_405,
	ERROR_408,
	ERROR_411,
	ERROR_413,
	ERROR_414,
	ERROR_415,
	ERROR_500,
	ERROR_505,
	SUCCESS_200,
	SUCCESS_201,
	SUCCESS_204,
	REDIRECTION_301,
};

#endif
