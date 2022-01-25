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
#include <sys/stat.h>

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
  unsigned int seq = 0;  // the current sequence number
  std::condition_variable cv_timer;  // for the timed wait
  char i_ipv4[INET6_ADDRSTRLEN];
  // protect concurrent access by the other thread;
  std::mutex exclude_other_mtx;

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

void* createAck(){
  SimpleHeader *h_ = new SimpleHeader();
  unsigned int payloadSize = h_->getLength();

  h_->setType(2);
  h_->setTR(0x1);
  h_->setWindow(10);
  h_->setSequence(gl.seq);
  h_->setLength(0);
  h_->setTimestamp(0xFFFFFFFF);
  return h_->thePacket();
}

void* createNAck(){
  SimpleHeader *h_ = new SimpleHeader();
  unsigned int payloadSize = h_->getLength();

  h_->setType(3);
  h_->setTR(0x0);
  h_->setWindow(10);
  h_->setSequence(gl.seq);
  h_->setLength(0);
  h_->setTimestamp(0xFFFFFFFF);
  return h_->thePacket();
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


// the receiver thread;
// sockfd: socket opened for receiving the datagrams
void recv_thread(int sockfd) {
  char s[INET6_ADDRSTRLEN];
  socklen_t addr_len;
  int numbytes;
  std::ofstream ifi("test.txt", std::ios::binary);
  std::ofstream dataPacket("dataPacket.txt", std::ios::binary);

  struct sockaddr_storage their_addr;
  char buf[524];

  int x;


  addr_len = sizeof(their_addr);


  std::cout << "Receiving...." << std::endl;

  while (1) {
    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN, 0,
			     (struct sockaddr *)&their_addr, &addr_len)) == -1) {
      std::cerr << "receiver: " << std::strerror(errno) << std::endl;
    }

    int packetType = ((buf[0] >> 6) & (0x3));


    std::cout << "receiver: " << " from " << inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr),s, sizeof s) <<
    " Packet Type: " << packetType << std::endl;

    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), gl.i_ipv4, sizeof gl.i_ipv4);

    std::cout << gl.i_ipv4;
    // Start and end Index of Payload
    const unsigned int s_indexPayload = 0xC;
    const unsigned int e_indexPayload = ((buf[2] << 8) | buf[3]) + s_indexPayload;

    // Construct a new array just containing the payload
     char* msg = new char[e_indexPayload - s_indexPayload];
     char* packet = (char *)buf;






    for (int i = 0; i < e_indexPayload - s_indexPayload; i++)
    {
      msg[i] = buf[i + 12];
    }




    // Write payload to file
      ifi.write(msg, e_indexPayload - s_indexPayload);

      ifi.close();

      dataPacket.write(buf, 524);
      dataPacket.close();

    // wake up the sender
    gl.cv_timer.notify_one();






  }

}

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
void send_thread(int sockfd, void* buf) {
  int numbytes;

  // create a mutex and a lock for the condition variable
  std::mutex mtx;
  std::unique_lock<std::mutex> lock(mtx);

  while (1) {
    // send then wait on cv
    if (send(sockfd, buf, 524, 0) != 1) {
      std::cerr << "Sender thread: " << std::strerror(errno) << std::endl;
    }

    if (gl.cv_timer.wait_for(lock, std::chrono::milliseconds(3000)) == std::cv_status::timeout) {
        //send_thread(sockfd, createNAck());
        //std::cout << "Sent NAck \n";
    }
    else {
      // we can do something in case the timer expired. Nothing for us now.
    }
  }
}






// cmd line: bouncer destination
int main(int argc, const char* argv[])
{
int sock_recv, sock_send;
  // are args OK?
  if (argc != 3) {
     std::cerr << "Usage: " << argv[0] << " [output file] [port]" << std::endl;
    return 2;
  }

  // args: family, port as string
  if ((sock_recv = create_sock_recv(AF_INET, argv[2])) < 0) {
    std::cerr << "I failed to recv" << std::endl;
    return 1;
  }



SimpleHeader *h_ = new SimpleHeader();
unsigned int payloadSize = h_->getLength();

h_->setType(2);
h_->setTR(0x1);
h_->setWindow(10);
h_->setSequence(gl.seq);
h_->setLength(0x4E);
h_->setTimestamp(0xFFFFFFFF);
void* x = h_->thePacket();



    // one socket for sending, another for receiving


  std::thread treceiver(recv_thread, sock_recv);

  std::thread ack(send_thread, sock_send, x);

  treceiver.join();
  if ((sock_send = create_sock_send(AF_INET, argv[2], gl.i_ipv4)) < 0) {
    std::cerr << "I failed to send" << std::endl;
    return 1;
  }
  ack.join();


  close(sock_recv);
  close(sock_send);


  return 0;
}
