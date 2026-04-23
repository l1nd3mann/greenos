#ifndef NET_H
#define NET_H

#include "kernel.h"

// MAC адрес (6 байт)
struct mac_addr {
    u8 addr[6];
};

// Ethernet заголовок
struct ethernet_header {
    u8 dest_mac[6];
    u8 src_mac[6];
    u16 type;
} __attribute__((packed));

// IPv4 заголовок
struct ipv4_header {
    u8 version_ihl;
    u8 dscp_ecn;
    u16 total_length;
    u16 identification;
    u16 flags_fragment;
    u8 ttl;
    u8 protocol;
    u16 checksum;
    u32 src_ip;
    u32 dest_ip;
} __attribute__((packed));

// UDP заголовок
struct udp_header {
    u16 src_port;
    u16 dest_port;
    u16 length;
    u16 checksum;
} __attribute__((packed));

// TCP заголовок (упрощенный)
struct tcp_header {
    u16 src_port;
    u16 dest_port;
    u32 seq_num;
    u32 ack_num;
    u8 data_offset;
    u8 flags;
    u16 window;
    u16 checksum;
    u16 urgent_ptr;
} __attribute__((packed));

// DNS заголовок
struct dns_header {
    u16 id;
    u16 flags;
    u16 q_count;
    u16 a_count;
    u16 auth_count;
    u16 add_count;
} __attribute__((packed));

// HTTP запрос
struct http_request {
    char method[8];
    char path[256];
    char host[128];
};

// Сетевой интерфейс
struct net_interface {
    struct mac_addr mac;
    struct ip_addr ip;
    struct ip_addr gateway;
    struct ip_addr netmask;
    struct ip_addr dns;
    u8 initialized;
};

// Функции инициализации сети
void net_init();
int net_send_packet(const u8* data, size_t len);
int net_recv_packet(u8* buffer, size_t max_len);

// IP функции
int ip_send(const struct ip_addr* dest, u8 protocol, const u8* data, size_t len);
int ip_recv(u8* buffer, size_t max_len, struct ip_addr* src, u8* protocol);

// UDP функции
int udp_send(const struct ip_addr* dest, u16 port, const u8* data, size_t len);
int udp_recv(u8* buffer, size_t max_len, struct ip_addr* src, u16* port);

// TCP функции (упрощенные)
int tcp_connect(const struct ip_addr* dest, u16 port);
int tcp_send(int socket, const u8* data, size_t len);
int tcp_recv(int socket, u8* buffer, size_t max_len);
void tcp_close(int socket);

// DNS функции
int dns_resolve(const char* hostname, struct ip_addr* result);

// HTTP функции
int http_get(const struct ip_addr* server, const char* path, const char* host, u8* buffer, size_t max_len);

#endif // NET_H
