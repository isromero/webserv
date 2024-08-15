/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   status.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/14 16:42:33 by isromero          #+#    #+#             */
/*   Updated: 2024/08/15 10:54:38 by isromero         ###   ########.fr       */
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
	ERROR_405,
	ERROR_411,
	ERROR_413,
	ERROR_414,
	ERROR_505,
	SUCCESS_200,
	SUCCESS_201,
	SUCCESS_202,
	SUCCESS_204,
	SUCCESS_206,
	SUCCESS_301,
	SUCCESS_302,
	SUCCESS_303,
	SUCCESS_304,
};

#endif
