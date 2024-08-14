/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   errors.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isromero <isromero@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/14 16:42:33 by isromero          #+#    #+#             */
/*   Updated: 2024/08/14 16:42:40 by isromero         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ERRORS_HPP
#define ERRORS_HPP

enum StatusErrorCode
{
	NO_ERROR,
	INVALID_REQUEST,
	INVALID_REQUEST_LINE,
	INVALID_HEADER_FORMAT,
	INCOMPLETE_BODY,
	INVALID_REQUEST_TARGET,
	INVALID_METHOD,
	INVALID_CONTENT_LENGTH,
	PAYLOAD_TOO_LARGE,
	URI_TOO_LONG,
	VERSION_NOT_SUPPORTED,
};

#endif
