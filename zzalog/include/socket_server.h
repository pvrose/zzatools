#ifndef __SOCKET_SERVER__
#define __SOCKET_SERVER__

#include "utils.h"

#include <string>
#include <cstdint>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#ifdef _WIN32
#include <WinSock2.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/socket.h>
#define SOCKET int
#define SOCKADDR_IN sockaddr_in
#endif



//! This class provides an interface to use a generic socket as a server.
	class socket_server
	{

	public:

		//! Supported protocols
		enum protocol_t {
			HTTP,
			UDP
		};
		//! Constructor
		 
		//! \param protocol Create serverfor this protocol.
		//! \param address Network address for other end of socket.
		//! \param port_num Port number for other end of socket.
		socket_server(protocol_t protocol, std::string address, int port_num);
		//! Destructor.
		~socket_server();

		//! Close the socket
		
		//! \param external Waits for interface to quiesce.
		void close_server(bool external);
		//! Start running the server
		void run_server();
		//! Returns true if this server is listening
		bool has_server();
		//! Set callback to handle requests. 
		void callback(int(*do_request)(std::stringstream&));
		//! Send response
		int send_response(std::istream& response);

	protected:

		//! Response from client.
		enum client_status {
			OK = 0,           //!< Response OK
			NG = 1,           //!< No good.
			BLOCK = 2         //!< Response would block.
		};

		//! Receive a datagram from UDP client
		int rcv_packet();
		//! Accept the client - returns client status.
		client_status accept_client();
		//! Error handler - \p phase indicates the peocess that errored.
		void handle_error(const char* phase);
		//! Send request - std::set by call-back
		int (*do_request)(std::stringstream& request);

		//! Open socket and create server
		int create_server();
		//! Print diagnostic data
		void dump(std::string line);
		//! Callback from server std::thread to handle packet.
		static void cb_th_packet(void* v);
		//! Start the server std::thread.
		static void thread_run(socket_server* that);

		//! Server socket
		SOCKET server_;
		//! HTTP Client
		SOCKET client_;
		//! Current client address
		SOCKADDR_IN client_addr_;
		//! Previous client address
		std::string prev_addr_;
		//! Previous client port number
		int prev_port_;
		//! Host IP address e.g. 127.0.0.1
		std::string host_id_;
		//! port number
		int port_num_;
		//! Server address
		std::string address_;
		//! protocol
		protocol_t protocol_;
		//! Socket is closing
		std::atomic<bool> closing_;
		//! Socket has closed.
		std::atomic<bool> closed_;
		//! Separate std::thread to handle socket transfers.
		std::thread* th_socket_;
		//! Packet std::queue
		std::queue<std::string> q_packet_;
		//! Lock to avoid pushing into the packet std::queue and pulling from it at the same time.
		std::mutex mu_packet_;

	};

#endif

