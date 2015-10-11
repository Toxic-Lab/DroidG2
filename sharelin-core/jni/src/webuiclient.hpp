#ifndef WEBUICLIENT_HPP
#define WEBUICLIENT_HPP

#include "io.hpp"
#include "boost/enable_shared_from_this.hpp"
#include <string>
#include "ip.hpp"
#include "filerange.hpp"
#include "asyncfile.hpp"

class WebUIClient: public boost::enable_shared_from_this<WebUIClient>
{
public:
	WebUIClient();
	ba::ip::tcp::socket& Socket() { return socket_; }
	void Start();
	Ip::Endpoint GetEndpoint();

private:
	enum Method {GET, POST};
	void OnHeadersReceived(const bs::error_code&);
	void OnContent(const bs::error_code&);
	void OnFileTransfer(const bs::error_code&);
	void OnPageHeaderSent(const bs::error_code&);
	void OnTransferCompleted(const bs::error_code&);
	void OnFinal(const bs::error_code&);
	void TranslateHeaders();
	void HandleRequest();
	void ServeFile();
	void DoCommand();
	void DetachMe();
	void Close();

	ba::ip::tcp::socket socket_;
	ba::streambuf message_;
	std::vector<char> buffer_;
	std::string content_;
	bool keepAlive_;
	std::string object_;
	file_offset_t offset_;
	FileRange fileRange_;
	bool closed_;
	AsyncFile file_;
	Method method_;
};

#endif //WEBUICLIENT_HPP
