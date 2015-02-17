/*
Copyright (c) 2012-2014 The SSDB Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
*/
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#ifdef FASTOREDIS
    #ifdef OS_WIN
        #define F_EINTR 0
    #else
        #include <sys/socket.h>
        #define F_EINTR EINTR
    #endif
#else
#include <sys/socket.h>
#endif

#include "link.h"

#include "link_redis.cpp"

#define MAX_PACKET_SIZE		32 * 1024 * 1024
#define ZERO_BUFFER_SIZE	8

int Link::min_recv_buf = 8 * 1024;
int Link::min_send_buf = 8 * 1024;


Link::Link(bool is_server){
	redis = NULL;

	sock = -1;
	noblock_ = false;
	error_ = false;
	remote_ip[0] = '\0';
	remote_port = -1;
	auth = false;
	
	if(is_server){
		input = output = NULL;
	}else{
		// alloc memory lazily
		//input = new Buffer(Link::min_recv_buf);
		//output = new Buffer(Link::min_send_buf);
		input = new Buffer(ZERO_BUFFER_SIZE);
		output = new Buffer(ZERO_BUFFER_SIZE);
	}
}

Link::~Link(){
	if(redis){
		delete redis;
	}
	if(input){
		delete input;
	}
	if(output){
		delete output;
	}
	this->close();
}

void Link::close(){
	if(sock >= 0){
		::close(sock);
	}
}

void Link::nodelay(bool enable){
    int opt = enable? 1 : 0;
#ifdef FASTOREDIS
    #ifdef OS_WIN
        ::setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt, sizeof(opt));
    #else
        ::setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void *)&opt, sizeof(opt));
    #endif
#else
	::setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void *)&opt, sizeof(opt));
#endif
}

void Link::keepalive(bool enable){
	int opt = enable? 1 : 0;
#ifdef FASTOREDIS
    #ifdef OS_WIN
        ::setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (const char *)&opt, sizeof(opt));
    #else
        ::setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (void *)&opt, sizeof(opt));
    #endif
#else
	::setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (void *)&opt, sizeof(opt));
#endif
}

void Link::noblock(bool enable){
	noblock_ = enable;
#ifdef FASTOREDIS
    #ifdef OS_WIN
        unsigned long flags = !enable;
        int res = ioctlsocket(sock, FIONBIO, &flags);
    #else
        if(enable){
            ::fcntl(sock, F_SETFL, O_NONBLOCK | O_RDWR);
        }else{
            ::fcntl(sock, F_SETFL, O_RDWR);
        }
    #endif
#else
	if(enable){
		::fcntl(sock, F_SETFL, O_NONBLOCK | O_RDWR);
	}else{
		::fcntl(sock, F_SETFL, O_RDWR);
	}
#endif
}


Link* Link::connect(const char *ip, int port){
	Link *link;
	int sock = -1;

	struct sockaddr_in addr;
#ifdef FASTOREDIS
    #ifdef OS_WIN
        unsigned long hostaddr = inet_addr(ip);
        addr.sin_family = AF_INET;
        addr.sin_port = htons((short)port);
        addr.sin_addr.s_addr = hostaddr;
    #else
        bzero(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons((short)port);
        inet_pton(AF_INET, ip, &addr.sin_addr);
    #endif
#else
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons((short)port);
    inet_pton(AF_INET, ip, &addr.sin_addr);
#endif

	if((sock = ::socket(AF_INET, SOCK_STREAM, 0)) == -1){
		goto sock_err;
	}
	if(::connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1){
		goto sock_err;
	}

	//log_debug("fd: %d, connect to %s:%d", sock, ip, port);
	link = new Link();
	link->sock = sock;
	link->keepalive(true);
	return link;
sock_err:
	//log_debug("connect to %s:%d failed: %s", ip, port, strerror(errno));
	if(sock >= 0){
		::close(sock);
	}
	return NULL;
}

Link* Link::listen(const char *ip, int port){
	Link *link;
	int sock = -1;

	int opt = 1;
	struct sockaddr_in addr;
#ifdef FASTOREDIS
    #ifdef OS_WIN
        unsigned long hostaddr = inet_addr(ip);
        addr.sin_family = AF_INET;
        addr.sin_port = htons((short)port);
        addr.sin_addr.s_addr = hostaddr;
    #else
        bzero(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons((short)port);
        inet_pton(AF_INET, ip, &addr.sin_addr);
    #endif
#else
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons((short)port);
	inet_pton(AF_INET, ip, &addr.sin_addr);
#endif
	if((sock = ::socket(AF_INET, SOCK_STREAM, 0)) == -1){
		goto sock_err;
	}
#ifdef FASTOREDIS
    #ifdef OS_WIN
        if(::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)) == -1){
            goto sock_err;
        }
    #else
        if(::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1){
            goto sock_err;
        }
    #endif
#else
	if(::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1){
		goto sock_err;
	}
#endif
	if(::bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1){
		goto sock_err;
	}
	if(::listen(sock, 1024) == -1){
		goto sock_err;
	}
	//log_debug("server socket fd: %d, listen on: %s:%d", sock, ip, port);

	link = new Link(true);
	link->sock = sock;
	snprintf(link->remote_ip, sizeof(link->remote_ip), "%s", ip);
	link->remote_port = port;
	return link;
sock_err:
	//log_debug("listen %s:%d failed: %s", ip, port, strerror(errno));
	if(sock >= 0){
		::close(sock);
	}
	return NULL;
}

Link* Link::accept(){
	Link *link;
	int client_sock;
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	while((client_sock = ::accept(sock, (struct sockaddr *)&addr, &addrlen)) == -1){
        if(errno != F_EINTR){
			//log_error("socket %d accept failed: %s", sock, strerror(errno));
			return NULL;
		}
	}

	struct linger opt = {1, 0};
#ifdef FASTOREDIS
    #ifdef OS_WIN
        int ret = ::setsockopt(client_sock, SOL_SOCKET, SO_LINGER, (const char *)&opt, sizeof(opt));
    #else
        int ret = ::setsockopt(client_sock, SOL_SOCKET, SO_LINGER, (void *)&opt, sizeof(opt));
    #endif
#else
	int ret = ::setsockopt(client_sock, SOL_SOCKET, SO_LINGER, (void *)&opt, sizeof(opt));
#endif
    if (ret != 0) {
		//log_error("socket %d set linger failed: %s", client_sock, strerror(errno));
	}

	link = new Link();
	link->sock = client_sock;
	link->keepalive(true);
#ifdef FASTOREDIS
    #ifdef OS_WIN
    #else
        inet_ntop(AF_INET, &addr.sin_addr, link->remote_ip, sizeof(link->remote_ip));
    #endif
#else
	inet_ntop(AF_INET, &addr.sin_addr, link->remote_ip, sizeof(link->remote_ip));
#endif
	link->remote_port = ntohs(addr.sin_port);
	return link;
}

int Link::read(){
	if(input->total() == ZERO_BUFFER_SIZE){
		input->grow();
	}
	int ret = 0;
	int want;
	input->nice();
	while((want = input->space()) > 0){
		// test
		//want = 1;
#ifdef FASTOREDIS
    #ifdef OS_WIN
            int len = ::recv(sock, input->slot(), want, 0);
    #else
            int len = ::read(sock, input->slot(), want);
    #endif
#else
		int len = ::read(sock, input->slot(), want);
#endif
		if(len == -1){
            if(errno == F_EINTR){
				continue;
			}else if(errno == EWOULDBLOCK){
				break;
			}else{
				//log_debug("fd: %d, read: -1, want: %d, error: %s", sock, want, strerror(errno));
				return -1;
			}
		}else{
			//log_debug("fd: %d, want=%d, read: %d", sock, want, len);
			if(len == 0){
				return 0;
			}
			ret += len;
			input->incr(len);
		}
		if(!noblock_){
			break;
		}
	}
	//log_debug("read %d", ret);
	return ret;
}

int Link::write(){
	if(output->total() == ZERO_BUFFER_SIZE){
		output->grow();
	}
	int ret = 0;
	int want;
	while((want = output->size()) > 0){
		// test
		//want = 1;
#ifdef FASTOREDIS
    #ifdef OS_WIN
            int len = ::send(sock, output->data(), want, 0);
    #else
            int len = ::write(sock, output->data(), want);
    #endif
#else
		int len = ::write(sock, output->data(), want);
#endif
        if(len == -1){
            if(errno == F_EINTR){
				continue;
			}else if(errno == EWOULDBLOCK){
				break;
			}else{
				//log_debug("fd: %d, write: -1, error: %s", sock, strerror(errno));
				return -1;
			}
		}else{
			//log_debug("fd: %d, want: %d, write: %d", sock, want, len);
			if(len == 0){
				// ?
				break;
			}
			ret += len;
			output->decr(len);
		}
		if(!noblock_){
			break;
		}
	}
	output->nice();
	return ret;
}

int Link::flush(){
	int len = 0;
	while(!output->empty()){
		int ret = this->write();
		if(ret == -1){
			return -1;
		}
		len += ret;
	}
	return len;
}

const std::vector<Bytes>* Link::recv(){
	this->recv_data.clear();

	if(input->empty()){
		return &this->recv_data;
	}

	// TODO: 记住上回的解析状态
	int parsed = 0;
	int size = input->size();
	char *head = input->data();
	
	// Redis protocol supports
	if(head[0] == '*'){
		if(redis == NULL){
			redis = new RedisLink();
		}
		const std::vector<Bytes> *ret = redis->recv_req(input);
		if(ret){
			this->recv_data = *ret;
			return &this->recv_data;
		}else{
			return NULL;
		}
	}

	// ignore leading empty lines
	while(size > 0 && (head[0] == '\n' || head[0] == '\r')){
		head ++;
		size --;
		parsed ++;
	}

	while(size > 0){
		char *body = (char *)memchr(head, '\n', size);
		if(body == NULL){
			break;
		}
		body ++;

		int head_len = body - head;
		if(head_len == 1 || (head_len == 2 && head[0] == '\r')){
			// packet end
			parsed += head_len;
			input->decr(parsed);
			return &this->recv_data;;
		}
		if(head[0] < '0' || head[0] > '9'){
			//log_warn("bad format");
			return NULL;
		}

		char head_str[20];
		if(head_len > (int)sizeof(head_str) - 1){
			return NULL;
		}
		memcpy(head_str, head, head_len - 1); // no '\n'
		head_str[head_len - 1] = '\0';

		int body_len = atoi(head_str);
		if(body_len < 0){
			//log_warn("bad format");
			return NULL;
		}
		//log_debug("size: %d, head_len: %d, body_len: %d", size, head_len, body_len);
		size -= head_len + body_len;
		if(size < 0){
			break;
		}

		this->recv_data.push_back(Bytes(body, body_len));

		head += head_len + body_len;
		parsed += head_len + body_len;
		if(size > 0 && head[0] == '\n'){
			head += 1;
			size -= 1;
			parsed += 1;
		}else if(size > 1 && head[0] == '\r' && head[1] == '\n'){
			head += 2;
			size -= 2;
			parsed += 2;
		}else{
			break;
		}
		if(parsed > MAX_PACKET_SIZE){
			 //log_warn("fd: %d, exceed max packet size, parsed: %d", this->sock, parsed);
			 return NULL;
		}
	}

	if(input->space() == 0){
		input->nice();
		if(input->space() == 0){
			if(input->grow() == -1){
				//log_error("fd: %d, unable to resize input buffer!", this->sock);
				return NULL;
			}
			//log_debug("fd: %d, resize input buffer, %s", this->sock, input->stats().c_str());
		}
	}

	// not ready
	this->recv_data.clear();
	return &this->recv_data;
}

int Link::send(const std::vector<std::string> &resp){
	// Redis protocol supports
	if(this->redis){
		return this->redis->send_resp(this->output, resp);
	}
	
	for(int i=0; i<resp.size(); i++){
		output->append_record(resp[i]);
	}
	output->append('\n');
	return 0;
}

int Link::send(const std::vector<Bytes> &resp){
	for(int i=0; i<resp.size(); i++){
		output->append_record(resp[i]);
	}
	output->append('\n');
	return 0;
}

int Link::send(const Bytes &s1){
	output->append_record(s1);
	output->append('\n');
	return 0;
}

int Link::send(const Bytes &s1, const Bytes &s2){
	output->append_record(s1);
	output->append_record(s2);
	output->append('\n');
	return 0;
}

int Link::send(const Bytes &s1, const Bytes &s2, const Bytes &s3){
	output->append_record(s1);
	output->append_record(s2);
	output->append_record(s3);
	output->append('\n');
	return 0;
}

int Link::send(const Bytes &s1, const Bytes &s2, const Bytes &s3, const Bytes &s4){
	output->append_record(s1);
	output->append_record(s2);
	output->append_record(s3);
	output->append_record(s4);
	output->append('\n');
	return 0;
}

int Link::send(const Bytes &s1, const Bytes &s2, const Bytes &s3, const Bytes &s4, const Bytes &s5){
	output->append_record(s1);
	output->append_record(s2);
	output->append_record(s3);
	output->append_record(s4);
	output->append_record(s5);
	output->append('\n');
	return 0;
}

const std::vector<Bytes>* Link::response(){
	while(1){
		const std::vector<Bytes> *resp = this->recv();
		if(resp == NULL){
			return NULL;
		}else if(resp->empty()){
			if(this->read() <= 0){
				return NULL;
			}
		}else{
			return resp;
		}
	}
	return NULL;
}

const std::vector<Bytes>* Link::request(const Bytes &s1){
	if(this->send(s1) == -1){
		return NULL;
	}
	if(this->flush() == -1){
		return NULL;
	}
	return this->response();
}

const std::vector<Bytes>* Link::request(const Bytes &s1, const Bytes &s2){
	if(this->send(s1, s2) == -1){
		return NULL;
	}
	if(this->flush() == -1){
		return NULL;
	}
	return this->response();
}

const std::vector<Bytes>* Link::request(const Bytes &s1, const Bytes &s2, const Bytes &s3){
	if(this->send(s1, s2, s3) == -1){
		return NULL;
	}
	if(this->flush() == -1){
		return NULL;
	}
	return this->response();
}

const std::vector<Bytes>* Link::request(const Bytes &s1, const Bytes &s2, const Bytes &s3, const Bytes &s4){
	if(this->send(s1, s2, s3, s4) == -1){
		return NULL;
	}
	if(this->flush() == -1){
		return NULL;
	}
	return this->response();
}

const std::vector<Bytes>* Link::request(const Bytes &s1, const Bytes &s2, const Bytes &s3, const Bytes &s4, const Bytes &s5){
	if(this->send(s1, s2, s3, s4, s5) == -1){
		return NULL;
	}
	if(this->flush() == -1){
		return NULL;
	}
	return this->response();
}

#if 0
int main(){
	//Link link;
	//link.listen("127.0.0.1", 8888);
	Link *link = Link::connect("127.0.0.1", 8080);
	printf("%d\n", link);
	getchar();
	return 0;
}
#endif