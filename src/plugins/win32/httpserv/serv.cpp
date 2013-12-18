#include "serv.hpp"

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/PartHandler.h"
#include "Poco/Net/MediaType.h"
#include "Poco/Net/MessageHeader.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Exception.h"
#include "Poco/URI.h"
#include "Poco/Event.h"
#include "Poco/Mutex.h"
#include "Poco/Thread.h"
#include <iostream>

using Poco::Net::SocketAddress;
using Poco::Net::ServerSocket;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::HTTPServerParams;
using Poco::Net::MessageHeader;
using Poco::Net::MediaType;
using Poco::Net::HTMLForm;
using Poco::Net::NameValueCollection;
using Poco::URI;
using Poco::Event;
using Poco::FastMutex;
using Poco::Thread;

struct PwRequestCallback {
	typedef PwHTTPServer::RequestCallback Callback;
	PwRequestCallback(Callback _cb, void *_param)   : cb(   _cb), param(   _param) {}
	PwRequestCallback(const PwRequestCallback &src) : cb(src.cb), param(src.param) {}
	void invoke(PwRequestResponse *rr) const { cb(rr, param); }
private:
	Callback cb;
	void *param;
};

struct PwRequestResponseImpl : public PwRequestResponse
{
	PwRequestResponseImpl(HTTPServerRequest &_request, HTTPServerResponse& _response)
		: available(true), event(false), request(_request), response(_response)
	{
//		response.setChunkedTransferEncoding(true);
		response.setKeepAlive(false);
		method = request.getMethod();
		uri    = request.getURI();

		URI      uridec(uri);
		path   = uridec.getPath();
		try {
			host = request.getHost();
		} catch (Poco::Exception) {
		} catch (...) {
			throw;
		}
		SocketAddress address(request.clientAddress());
		client = address.toString();
	}
	const String& getMethod() const { return method; }
	const String& getURI()    const { return uri;    }
	const String& getPath()   const { return path;   }
	const String& getHost()   const { return host;   }
	const String& getClient() const { return client; }

	int  getHeader(NameValueCallback cb, void *param) const {
		NameValueCollection::ConstIterator it = request.begin();
		NameValueCollection::ConstIterator end = request.end();
		int cnt = 0;
		for (; it != end; ++it,cnt++) cb(it->first, it->second, param);
		return cnt;
	}
	int  getFormData(NameValueCallback cb, void *param) const {
		HTMLForm form(request, request.stream());
		NameValueCollection::ConstIterator it  = form.begin();
		NameValueCollection::ConstIterator end = form.end();
		int cnt = 0;
		for (; it != end; ++it,cnt++) cb(it->first, it->second, param);
		return cnt;
	}
	inline HTTPResponse::HTTPStatus strToStatus(char const *status) {
		HTTPResponse stat;
		stat.setStatus(status);
		return stat.getStatus();
	}
	void setStatus(char const *status)            { if (avail()) response.setStatusAndReason(strToStatus(status)); }
	void setContentType(char const *type)         { if (avail()) response.setContentType(type); this->type = type; }
	void setRedirect(char const *target)          { if (avail()) response.redirect(target);                        }
	void sendBuffer(void const *buf, Size length) { if (avail()) response.sendBuffer(buf, length);                 }
	void sendFile(char const *path)               { if (avail()) response.sendFile(path, type);                    }
	const String getCharset(char const *mediatype) {
		MediaType mt(mediatype);
		String ret;
		try {
			ret = mt.getParameter("charset");
		} catch (Poco::Exception) {
		} catch (...) {
			throw;
		}
		return ret;
	}
	const String getReason(char const *status) {
		HTTPResponse reason(strToStatus(status));
		return reason.getReason();
	}
	void done() {
		event.set();
		if (!avail()) delete this;
	}
	bool wait(int timeoutsec) {
		if (timeoutsec <= 0) event.wait();
		else if (!event.tryWait(timeoutsec * 1000)) {
			resetAvail();
			return true;
		}
		delete this;
		return false;
	}
private:
	bool available;
	Event event;
	HTTPServerRequest  &request;
	HTTPServerResponse &response;
	String type, method, uri, path, host, client;

	FastMutex lockAvail;
	bool avail() {
		FastMutex::ScopedLock lock(lockAvail);
		return available;
	}
	void resetAvail() {
		FastMutex::ScopedLock lock(lockAvail);
		available = false;
	}
};

struct PwRequestHandler : public HTTPRequestHandler
{
	PwRequestHandler(PwRequestCallback const &_cb, int to) : timeout(to), cb(_cb) {}
	void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) {
		PwRequestResponseImpl *rr = new PwRequestResponseImpl(request, response);
		cb.invoke(rr);
		// 完了するまで待つ
		if (rr->wait(timeout)) {
			// タイムアウトした
			(void)0; // とりあえず何もしない
		}
	}
private:
	int timeout;
	PwRequestCallback const cb;
};

struct PwRequestFactory : public HTTPRequestHandlerFactory
{
	PwRequestFactory(PwRequestCallback const &_cb, int to) : timeout(to), cb(_cb) {}
	HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request) {
		return new PwRequestHandler(cb, timeout);
	}
private:
	int timeout;
	PwRequestCallback const cb;
};

struct PwHTTPServerImpl : public PwHTTPServer
{
	PwHTTPServerImpl(PwRequestCallback const &_cb, int to) : timeout(to), cb(_cb), socket(0), server(0) {}
	~PwHTTPServerImpl() { stop(); }

	int start(int port) {
		stop();
		SocketAddress sock("127.0.0.1", port);
		socket = new ServerSocket(sock);
		server = new HTTPServer(new PwRequestFactory(cb, timeout), *socket, new HTTPServerParams());
		server->start();
		return (int)socket->address().port();
	}
	void stop() {
		if (server == NULL) return;
		server->stop();
		delete server;
		delete socket;
		server = NULL;
		socket = NULL;
	}
private:
	int timeout;
	PwRequestCallback const cb;
	ServerSocket *socket;
	HTTPServer   *server;
};

PwHTTPServer*
PwHTTPServer::Factory(PwHTTPServer::RequestCallback callback, void *param, int tout) {
	PwRequestCallback cb(callback, param);
	return new PwHTTPServerImpl(cb, tout);
}

