// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Error/Error.h"
#include "Core/Network/IpAddress.h"
#include "Core/Base/Macros.h"
#include "Core/Base/Platform.h"
#include "Core/Base/Types.h"

#if RIO_PLATFORM_POSIX
	#include <errno.h>
	#include <fcntl.h> // fcntl
	#include <netinet/in.h> // htons, htonl, ...
	#include <sys/socket.h>
	#include <unistd.h> // close
#elif RIO_PLATFORM_WINDOWS
	#include "Device/Windows/Headers_Windows.h" // already includes <WinSock2.h>
	#pragma comment(lib, "Ws2_32.lib")
#endif // RIO_PLATFORM_

namespace Rio
{

struct ConnectResult
{
	enum 
	{ 
		NO_ERROR,
		BAD_SOCKET,
		REFUSED,
		TIMEOUT,
		UNKNOWN 
	} error;
};

struct BindResult
{
	enum
	{
		NO_ERROR,
		BAD_SOCKET,
		ADDRESS_IN_USE,
		UNKNOWN
	} error;
};

struct ReadResult
{
	enum 
	{ 
		NO_ERROR,
		BAD_SOCKET,
		REMOTE_CLOSED,
		TIMEOUT,
		UNKNOWN 
	} error;
	uint32_t bytesRead;
};

struct WriteResult
{
	enum 
	{ 
		NO_ERROR, 
		BAD_SOCKET, 
		REMOTE_CLOSED, 
		TIMEOUT, 
		UNKNOWN 
	} error;
	uint32_t bytesWritten;
};

struct AcceptResult
{
	enum 
	{ 
		NO_ERROR, 
		BAD_SOCKET, 
		NO_CONNECTION, 
		UNKNOWN 
	} error;
};

struct TcpSocket
{
	TcpSocket()
	{
	}

	void open()
	{
		socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#if RIO_PLATFORM_POSIX
		RIO_ASSERT(socket >= 0, "socket: errno = %d", errno);
#elif RIO_PLATFORM_WINDOWS
		RIO_ASSERT(socket >= 0, "socket: WSAGetLastError = %d", WSAGetLastError());
#endif
	}

	void close()
	{
#if RIO_PLATFORM_POSIX
		if (socket != 0)
		{
			::close(socket);
			socket = 0;
		}
#elif RIO_PLATFORM_WINDOWS
		if (socket != INVALID_SOCKET)
		{
			::closesocket(socket);
			socket = INVALID_SOCKET;
		}
#endif
	}

	ConnectResult connect(const IpAddress& ip, uint16_t port)
	{
		close();
		open();

		sockaddr_in addrIn;
		addrIn.sin_family = AF_INET;
		addrIn.sin_addr.s_addr = htonl(ip.getAddress());
		addrIn.sin_port = htons(port);

		int err = ::connect(socket, (const sockaddr*)&addrIn, sizeof(sockaddr_in));

		ConnectResult cr;
		cr.error = ConnectResult::NO_ERROR;

		if (err == 0)
		{
			return cr;
		}

#if RIO_PLATFORM_POSIX
		if (errno == ECONNREFUSED)
		{
			cr.error = ConnectResult::REFUSED;
		}
		else if (errno == ETIMEDOUT)
		{
			cr.error = ConnectResult::TIMEOUT;
		}
		else
		{
			cr.error = ConnectResult::UNKNOWN;
		}
#elif RIO_PLATFORM_WINDOWS
		int wsaErr = WSAGetLastError();
		if (wsaErr == WSAECONNREFUSED)
		{
			cr.error = ConnectResult::REFUSED;
		}
		else if (wsaErr == WSAETIMEDOUT)
		{
			cr.error = ConnectResult::TIMEOUT;
		}
		else
		{
			cr.error = ConnectResult::UNKNOWN;
		}
#endif // RIO_PLATFORM_
		return cr;
	}

	BindResult bind(uint16_t port)
	{
		close();
		open();
		setReuseAddress(true);

		sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = htonl(INADDR_ANY);
		address.sin_port = htons(port);

		int err = ::bind(socket, (const sockaddr*)&address, sizeof(sockaddr_in));

		BindResult bindResult;
		bindResult.error = BindResult::NO_ERROR;

		if (err == 0)
		{
			return bindResult;
		}

#if RIO_PLATFORM_POSIX
		if (errno == EBADF)
		{
			bindResult.error = BindResult::BAD_SOCKET;
		}
		else if (errno == EADDRINUSE)
		{
			bindResult.error = BindResult::ADDRESS_IN_USE;
		}
		else
		{
			bindResult.error = BindResult::UNKNOWN;
		}
#elif RIO_PLATFORM_WINDOWS
		int wsaErr = WSAGetLastError();
		if (wsaErr == WSAEADDRINUSE)
		{
			bindResult.error = BindResult::ADDRESS_IN_USE;
		}
		else
		{
			bindResult.error = BindResult::UNKNOWN;
		}
#endif // RIO_PLATFORM_
		return bindResult;
	}

	void listen(uint32_t max)
	{
		int err = ::listen(socket, max);
#if RIO_PLATFORM_POSIX
		RIO_ASSERT(err == 0, "listen: errno = %d", errno);
#elif RIO_PLATFORM_WINDOWS
		RIO_ASSERT(err == 0, "listen: WSAGetLastError = %d", WSAGetLastError());
#endif
		RIO_UNUSED(err);
	}

	AcceptResult acceptInternal(TcpSocket& tcpSocket)
	{
		int err = ::accept(socket, NULL, NULL);

		AcceptResult acceptResult;
		acceptResult.error = AcceptResult::NO_ERROR;

#if RIO_PLATFORM_POSIX
		if (err >= 0)
		{
			tcpSocket.socket = err;
		}
		else if (err == -1 && errno == EBADF)
		{
			acceptResult.error = AcceptResult::BAD_SOCKET;
		}
		else if (err == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
		{
			acceptResult.error = AcceptResult::NO_CONNECTION;
		}
		else
		{
			acceptResult.error = AcceptResult::UNKNOWN;
		}
#elif RIO_PLATFORM_WINDOWS
		if (err != INVALID_SOCKET)
		{
			tcpSocket.socket = err;
			return acceptResult;
		}

		int wsaErr = WSAGetLastError();
		if (wsaErr == WSAEWOULDBLOCK)
		{
			acceptResult.error = AcceptResult::NO_CONNECTION;
		}
		else
		{
			acceptResult.error = AcceptResult::UNKNOWN;
		}
#endif // RIO_PLATFORM_
		return acceptResult;
	}

	AcceptResult acceptNonblock(TcpSocket& c)
	{
		setBlocking(false);
		return acceptInternal(c);
	}

	AcceptResult accept(TcpSocket& c)
	{
		setBlocking(true);
		return acceptInternal(c);
	}

	ReadResult readInternal(void* data, uint32_t size)
	{
		ReadResult readResult;
		readResult.error = ReadResult::NO_ERROR;
		readResult.bytesRead = 0;

		char* buffer = (char*)data;
		uint32_t toRead = size;

		while (toRead > 0)
		{
#if RIO_PLATFORM_POSIX
			ssize_t bytesRead = ::recv(socket, buffer, toRead, 0);

			if (bytesRead == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
			{
				return readResult;
			}
			else if (bytesRead == -1 && errno == ETIMEDOUT)
			{
				readResult.error = ReadResult::TIMEOUT;
				return readResult;
			}
			else if (bytesRead == 0)
			{
				readResult.error = ReadResult::REMOTE_CLOSED;
				return readResult;
			}
#elif RIO_PLATFORM_WINDOWS
			int bytesRead = ::recv(socket, buffer, (int)toRead, 0);

			if (bytesRead == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK)
			{
				return readResult;
			}
			else if (bytesRead == SOCKET_ERROR && WSAGetLastError() == WSAETIMEDOUT)
			{
				readResult.error = ReadResult::TIMEOUT;
				return readResult;
			}
			else if (bytesRead == 0)
			{
				readResult.error = ReadResult::REMOTE_CLOSED;
				return readResult;
			}
#endif // RIO_PLATFORM_
			buffer += bytesRead;
			toRead -= bytesRead;
			readResult.bytesRead += bytesRead;
		}

		return readResult;
	}

	ReadResult readNonblock(void* data, uint32_t size)
	{
		setBlocking(false);
		return readInternal(data, size);
	}

	ReadResult read(void* data, uint32_t size)
	{
		setBlocking(true);
		return readInternal(data, size);
	}

	WriteResult writeInternal(const void* data, uint32_t size)
	{
		WriteResult wr;
		wr.error = WriteResult::NO_ERROR;
		wr.bytesWritten = 0;

		const char* buffer = (const char*)data;
		uint32_t toSend = size;

		while (toSend > 0)
		{
#if RIO_PLATFORM_POSIX
			ssize_t bytesWritten = ::send(socket, buffer, toSend, 0);

			if (bytesWritten == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
			{
				return wr;
			}
			else if (bytesWritten == -1 && errno == ETIMEDOUT)
			{
				wr.error = WriteResult::TIMEOUT;
				return wr;
			}
			else if (bytesWritten == 0)
			{
				wr.error = WriteResult::REMOTE_CLOSED;
				return wr;
			}
			else
			{
				wr.error = WriteResult::UNKNOWN;
				return wr;
			}
#elif RIO_PLATFORM_WINDOWS
			int bytesWritten = ::send(socket, buffer, (int)toSend, 0);

			if (bytesWritten == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK)
			{
				return wr;
			}
			else if (bytesWritten == SOCKET_ERROR && WSAGetLastError() == WSAETIMEDOUT)
			{
				wr.error = WriteResult::TIMEOUT;
				return wr;
			}
			else if (bytesWritten == 0)
			{
				wr.error = WriteResult::REMOTE_CLOSED;
				return wr;
			}
			else
			{
				wr.error = WriteResult::UNKNOWN;
				return wr;
			}
#endif // RIO_PLATFORM_
			buffer += bytesWritten;
			toSend -= bytesWritten;
			wr.bytesWritten += bytesWritten;
		}
		return wr;
	}

	WriteResult writeNonblocking(const void* data, uint32_t size)
	{
		setBlocking(false);
		return writeInternal(data, size);
	}

	WriteResult write(const void* data, uint32_t size)
	{
		setBlocking(true);
		return writeInternal(data, size);
	}

	void setBlocking(bool blocking)
	{
#if RIO_PLATFORM_POSIX
		int flags = fcntl(socket, F_GETFL, 0);
		fcntl(socket, F_SETFL, blocking ? (flags & ~O_NONBLOCK) : O_NONBLOCK);
#elif RIO_PLATFORM_WINDOWS
		u_long nonBlocking = blocking ? 0 : 1;
		ioctlsocket(socket, FIONBIO, &nonBlocking);
#endif // RIO_PLATFORM_
	}

	void setReuseAddress(bool reuse)
	{
		int optVal = (int)reuse;
		int err = setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&optVal, sizeof(optVal));
#if RIO_PLATFORM_POSIX
		RIO_ASSERT(err == 0, "setsockopt: errno = %d", errno);
#elif RIO_PLATFORM_WINDOWS
		RIO_ASSERT(err == 0, "setsockopt: WSAGetLastError = %d", WSAGetLastError());
#endif
		RIO_UNUSED(err);
	}

	void setTimeout(uint32_t seconds)
	{
		struct timeval timeout;
		timeout.tv_sec = seconds;
		timeout.tv_usec = 0;
#if RIO_PLATFORM_POSIX
		int err = setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
		RIO_ASSERT(err == 0, "setsockopt: errno: %d", errno);
		err = setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
		RIO_ASSERT(err == 0, "setsockopt: errno: %d", errno);
#elif RIO_PLATFORM_WINDOWS
		int err = setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
		RIO_ASSERT(err == 0, "setsockopt: WSAGetLastError: %d", WSAGetLastError());
		err = setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
		RIO_ASSERT(err == 0, "setsockopt: WSAGetLastError: %d", WSAGetLastError());
#endif
		RIO_UNUSED(err);
	}

#if RIO_PLATFORM_POSIX
	int socket = 0;
#elif RIO_PLATFORM_WINDOWS
	SOCKET socket = INVALID_SOCKET;
#endif
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka