// Project assignment 3 solution; compile with C++11.
// Source: Beej's Guide to Network Programming examples: listener.c and talker.c
// DESIGN:
// uses 2 sockets, one for receiving, used by one thread; one for sending, used by the other thread
// The sender sends a UDP packet every second. Waiting is performed on a condition variable, so the thread can wake up if the receiver receives something. The sender sends a single byte as payload, taken from the global value field.
// The receiver changes this global value upon receipt of a packet; it then notifies the sender who will send a new packet using the updated value.


#include <iostream>
#include <thread>
#include <bitset>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <cstdlib>
#include <ctime>
#include <cerrno>
#include <cstring>
#include <string>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <gtest/gtest.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <zlib.h>
#include <atomic>

#include "SimpleHeader.h";

struct stat fileStats;
#define MYPORT "5010"	// the port on which we receive
#define DESTPORT "5000"  // the port where we send packets
#define SZ 20

/* Cmd line arguments: bouncer destination_addr */

#define MAXBUFLEN 524

/* Since we use threads, we declare some global variables first.
It works just fine to declare the globals outside of a struct, but gathered in a struct just feels you are more in control :)
*/




struct globals_t {
    // the byte value being sent
  std::condition_variable cv_timer;  // for the timed wait
  // protect concurrent access by the other thread;
  std::mutex exclude_other_mtx;

  std::atomic<int> socketStatus = ATOMIC_VAR_INIT(0);
  std::atomic<bool> overrideStatus = ATOMIC_VAR_INIT(false);

  int seq = 0;
};

struct globals_t gl;

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


// creates the socket for receiving, using family to identify the protocol and port for the port we will be receiving on
// family = AF_INET or AF_INET6 or AF_UNSPEC
// port = string representing the port number we bind to as receiver
// return
//   -1 on error
//   the socked descriptor if OK
int create_sock_recv(int family, const char * port) {
  struct addrinfo hints, *servinfo, *p;
  int rv;

  int the_sock;

    // start with the hints (local address, UDP, etc
  memset(&hints, 0, sizeof hints);
  hints.ai_family = family; // set to AF_INET to use IPv4
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE; // use own IP

  if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
    std::cerr << "getaddrinfo for recv: " << gai_strerror(rv) << std::endl;
    return -1;
  }

  // loop through all the results and bind to the first we can
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((the_sock = socket(p->ai_family, p->ai_socktype,
			 p->ai_protocol)) == -1) {
      std::cerr << "recv socket: " << std::strerror(errno) << std::endl;
      continue;
    }

    if (bind(the_sock, p->ai_addr, p->ai_addrlen) == -1) {
      close(the_sock);
      std::cerr << "recv socket bind: " << std::strerror(errno) << std::endl;
      continue;
    }

    break;
  }

  if (p == NULL) {
    std::cerr << "recv socket: failed to find a suitable network" << std::endl;
    return -1;
  }

  freeaddrinfo(servinfo);
  return the_sock;
}


// creates the socket for sending, using family to identify the protocol and port to send to
// family = AF_INET or AF_INET6 or AF_UNSPEC
// port = string representing the port number we send to
// dest_host = string representing the destination address or name
// return
//   -1 on error
//   the socked descriptor if OK
int create_sock_send(int family, const char * port, const char * dest_host) {
  int sockfd;
  struct addrinfo hints, *servinfo, *p;
  int rv;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = family; // set to AF_INET to use IPv4
  hints.ai_socktype = SOCK_DGRAM;

  if ((rv = getaddrinfo(dest_host, port, &hints, &servinfo)) != 0) {
    std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
    return -1;
  }

  // loop through all the results and make a socket
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
			 p->ai_protocol)) == -1) {
      std::cerr << "send socket: " << std::strerror(errno) << std::endl;
      continue;
    }

    break;
  }

  if (p == NULL) {
    std::cerr << "send socket: failed to create" << std::endl;
    return -1;
  }

  // connect, save the destination address with the socket so we can use send instead of sendto
  if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
    std::cerr << "Send socket connect: " << std::strerror(errno) << std::endl;
    return -1;
  }

  return sockfd;
}


// ********** the workers ***********
// the sender thread;
// sockfd: socket opened for sending the datagrams
void send_thread2( int sockfd, void* buf )
{
  int numBytes;



  // if we're waiting to receive, dont send anything
  while( true )
  {
    // don't send if we're receiving
    while( gl.socketStatus.load() == 1 )
    {
      std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    if( send(sockfd, (char *)buf, 525, 0) != -1 )
    {
      std::cerr << "Sender thread: " << std::strerror(errno) << std::endl;
      std::cout << "Data packet with Seq Num: " << (unsigned int)(((char*)buf)[1]) << "\n\n";
      // tell receiver thread that it needs to start receiving now
      gl.socketStatus = 1;
    }
    else
    {
      // server isnt running or something
      std::cout << "[ERROR] Send Thread2: Receiver offline?\n";
    }

    gl.overrideStatus = false;
  }
}

void recv_thread2( int sockfd )
{
  char s[INET6_ADDRSTRLEN];
  socklen_t addr_len;
  int numbytes;
  std::ofstream ackPacket("ackPacket.txt", std::ios::binary);
  struct sockaddr_storage their_addr;
  char buf[MAXBUFLEN];

  addr_len = sizeof(their_addr);

  while( true )
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Waiting for acknowledgement...\n\n";

    // dont start trying to receive if we are still sending data
    while( gl.socketStatus.load() == 0 )
    {
      std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    struct timeval timeout = {4, 0};
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof timeout);

    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1)
    {
      std::cerr << "[ERROR] Recv thread2: " << std::strerror(errno) << "\n";
      gl.socketStatus = 0;
      continue;
    }

    int packetType = ((buf[0] >> 6) & (0x3));



    std::cout << "Recv thread2: " << buf[3] << " from " << inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s)
    << "  Packet Type: " << packetType << "\n\n";

    ackPacket.write(buf, 524);
    ackPacket.close();

    // let send thread know that we received data, and that it can send something again
    gl.socketStatus = 0;
  }
}



// the receiver thread;
// sockfd: socket opened for receiving the datagrams

unsigned long calcCRC32(std::string binaryFile)
{

  using namespace std;
  unsigned long crc;
  string file;
  ifstream f("input.bin", ios::in);
  string line;
  while(getline(f, line)) {
    file.append(line);
  }

  crc=crc32(0L, NULL, 0);
  crc=crc32(crc, reinterpret_cast<const Bytef*>(file.c_str()), file.size());

  //cout << "The crc32 value for: " << file << " is " << hex << crc << endl;
  f.close();
  return crc;
}


// cmd line: bouncer destination
int main(int argc, const char* argv[])
{
int sock_send, sock_recv;


  // are args OK?
  if (argc != 4) {
     std::cerr << "Usage: " << argv[0] << "[file] [addr] [port]" << std::endl;
    return 2;
  }

  // args: family, port as string
  if ((sock_recv = create_sock_recv(AF_INET, argv[3])) < 0) {
    std::cerr << "I failed to recv" << std::endl;
    return 1;
  }

  // args: family, port as string, destination name or addr as string
  if ((sock_send = create_sock_send(AF_INET, argv[3], argv[2])) < 0) {
    std::cerr << "I failed to send" << std::endl;
    return 1;
  }

  SimpleHeader *h_ = new SimpleHeader();

  // open the input file in binary and read it to the data array
  std::ifstream is (argv[1], std::ifstream::binary);
  is.seekg (0, is.end);
  int length = is.tellg();
  is.seekg (0, is.beg);

  is.read(h_->thePayload(), length);
  is.close();



// Setting up the packet
  unsigned int payloadSize = h_->getLength();

  h_->setType(1);
  h_->setTR(1);
  h_->setWindow(20);
  h_->setSequence(gl.seq);
  h_->setLength(payloadSize);
  h_->setTimestamp(0xFFFFFFFF);
  h_->setCRC1(calcCRC32(argv[1]));
  void* x = h_->thePacket();

  std::cout << payloadSize << std::endl;



  // start the threads
  std::thread tsender(send_thread2, sock_send, x);
  std::thread treceiver(recv_thread2, sock_recv);

  tsender.join();
  treceiver.join();


  close(sock_send);
  close(sock_recv);

  return 0;
}
