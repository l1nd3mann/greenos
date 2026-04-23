#include "kernel.h"
#include "net.h"

// Глобальная структура сетевого интерфейса
static struct net_interface net_if = {0};

// Виртуальный сетевой драйвер (для эмуляции в QEMU)
// В реальности здесь был бы драйвер e1000 или virtio-net

// Инициализация сети (DHCP или статическая конфигурация)
void net_init() {
    terminal_writestring("[NET] Initializing network...\n");
    
    // Для MVP используем статическую конфигурацию
    // В реальности здесь был бы DHCP клиент
    
    // Статический IP для тестирования
    net_if.ip.addr[0] = 10;
    net_if.ip.addr[1] = 0;
    net_if.ip.addr[2] = 2;
    net_if.ip.addr[3] = 15;
    
    net_if.gateway.addr[0] = 10;
    net_if.gateway.addr[1] = 0;
    net_if.gateway.addr[2] = 2;
    net_if.gateway.addr[3] = 2;
    
    net_if.netmask.addr[0] = 255;
    net_if.netmask.addr[1] = 255;
    net_if.netmask.addr[2] = 255;
    net_if.netmask.addr[3] = 0;
    
    // DNS сервер (Google DNS)
    net_if.dns.addr[0] = 8;
    net_if.dns.addr[1] = 8;
    net_if.dns.addr[2] = 8;
    net_if.dns.addr[3] = 8;
    
    // MAC адрес (фейковый для эмуляции)
    net_if.mac.addr[0] = 0x52;
    net_if.mac.addr[1] = 0x54;
    net_if.mac.addr[2] = 0x00;
    net_if.mac.addr[3] = 0x12;
    net_if.mac.addr[4] = 0x34;
    net_if.mac.addr[5] = 0x56;
    
    net_if.initialized = 1;
    
    terminal_writestring("[NET] IP: ");
    for (int i = 0; i < 4; i++) {
        terminal_writedecimal(net_if.ip.addr[i]);
        if (i < 3) terminal_putchar('.');
    }
    terminal_writestring("\n");
    
    terminal_writestring("[NET] Gateway: ");
    for (int i = 0; i < 4; i++) {
        terminal_writedecimal(net_if.gateway.addr[i]);
        if (i < 3) terminal_putchar('.');
    }
    terminal_writestring("\n");
    
    terminal_writestring("[NET] DNS: ");
    for (int i = 0; i < 4; i++) {
        terminal_writedecimal(net_if.dns.addr[i]);
        if (i < 3) terminal_putchar('.');
    }
    terminal_writestring("\n");
    
    terminal_writestring("[NET] Network initialized!\n");
}

// Отправка пакета (заглушка для MVP)
int net_send_packet(const u8* data, size_t len) {
    if (!net_if.initialized) {
        return -1;
    }
    
    // В реальности здесь была бы отправка через регистры сетевой карты
    // Для эмуляции в QEMU с virtio-net или e1000
    
    terminal_writestring("[NET] Sending packet (");
    terminal_writedecimal(len);
    terminal_writestring(" bytes)\n");
    
    return len;
}

// Получение пакета (заглушка для MVP)
int net_recv_packet(u8* buffer, size_t max_len) {
    if (!net_if.initialized) {
        return -1;
    }
    
    // В реальности здесь было бы чтение из буфера сетевой карты
    // Возвращаем 0 для индикации отсутствия пакетов
    
    return 0;
}

// Расчет контрольной суммы IP
static u16 ip_checksum(void* buf, size_t len) {
    u32 sum = 0;
    u16* ptr = (u16*)buf;
    
    while (len > 1) {
        sum += *ptr++;
        len -= 2;
    }
    
    if (len == 1) {
        sum += *(u8*)ptr;
    }
    
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    return ~sum;
}

// Отправка IP пакета
int ip_send(const struct ip_addr* dest, u8 protocol, const u8* data, size_t len) {
    u8 packet[1500];  // MTU
    struct ipv4_header* ip_hdr = (struct ipv4_header*)packet;
    
    // Заполнение IP заголовка
    ip_hdr->version_ihl = 0x45;  // IPv4, 5 слов заголовка
    ip_hdr->dscp_ecn = 0;
    ip_hdr->total_length = sizeof(struct ipv4_header) + len;
    ip_hdr->identification = 0;
    ip_hdr->flags_fragment = 0;
    ip_hdr->ttl = 64;
    ip_hdr->protocol = protocol;
    ip_hdr->checksum = 0;
    
    // Копирование IP адресов
    ip_hdr->src_ip = *(u32*)net_if.ip.addr;
    ip_hdr->dest_ip = *(u32*)dest->addr;
    
    // Расчет контрольной суммы
    ip_hdr->checksum = ip_checksum(ip_hdr, sizeof(struct ipv4_header));
    
    // Копирование данных
    memcpy(packet + sizeof(struct ipv4_header), data, len);
    
    // Отправка
    return net_send_packet(packet, sizeof(struct ipv4_header) + len);
}

// UDP отправка
int udp_send(const struct ip_addr* dest, u16 port, const u8* data, size_t len) {
    u8 packet[1500];
    struct udp_header* udp_hdr = (struct udp_header*)(packet + sizeof(struct ipv4_header));
    
    // Заполнение UDP заголовка
    udp_hdr->src_port = 0x1337;  // Случайный порт
    udp_hdr->dest_port = port;
    udp_hdr->length = sizeof(struct udp_header) + len;
    udp_hdr->checksum = 0;  // Опционально для UDP over IPv4
    
    // Отправка через IP
    return ip_send(dest, 17, packet + sizeof(struct ipv4_header), 
                   sizeof(struct udp_header) + len);
}

// DNS разрешение (упрощенное)
int dns_resolve(const char* hostname, struct ip_addr* result) {
    terminal_writestring("[DNS] Resolving: ");
    terminal_writestring(hostname);
    terminal_writestring("\n");
    
    // Для MVP хардкодим некоторые адреса
    if (strcmp(hostname, "api.github.com") == 0) {
        result->addr[0] = 140;
        result->addr[1] = 82;
        result->addr[2] = 114;
        result->addr[3] = 16;
        terminal_writestring("[DNS] Resolved to 140.82.114.16\n");
        return 0;
    }
    
    if (strcmp(hostname, "github.com") == 0) {
        result->addr[0] = 140;
        result->addr[1] = 82;
        result->addr[2] = 112;
        result->addr[3] = 4;
        terminal_writestring("[DNS] Resolved to 140.82.112.4\n");
        return 0;
    }
    
    // В реальности здесь был бы DNS запрос через UDP порт 53
    terminal_writestring("[DNS] Resolution failed\n");
    return -1;
}

// HTTP GET запрос
int http_get(const struct ip_addr* server, const char* path, const char* host, u8* buffer, size_t max_len) {
    terminal_writestring("[HTTP] GET https://");
    terminal_writestring(host);
    terminal_writestring(path);
    terminal_writestring("\n");
    
    // Формирование HTTP запроса (без sprintf для простоты)
    char request[512];
    int req_len = 0;
    
    // Копируем строки вручную
    const char* http_get_str = "GET ";
    const char* http_ver = " HTTP/1.1\r\n";
    const char* host_str = "Host: ";
    const char* user_agent = "User-Agent: MiniOS/1.0\r\n";
    const char* accept = "Accept: */*\r\n";
    const char* connection = "Connection: close\r\n";
    const char* crlf = "\r\n";
    
    while (*http_get_str && req_len < 510) request[req_len++] = *http_get_str++;
    while (*path && req_len < 510) request[req_len++] = *path++;
    while (*http_ver && req_len < 510) request[req_len++] = *http_ver++;
    
    while (*host_str && req_len < 510) request[req_len++] = *host_str++;
    while (*host && req_len < 510) request[req_len++] = *host++;
    while (*crlf && req_len < 510) request[req_len++] = *crlf++;
    
    while (*user_agent && req_len < 510) request[req_len++] = *user_agent++;
    while (*accept && req_len < 510) request[req_len++] = *accept++;
    while (*connection && req_len < 510) request[req_len++] = *connection++;
    while (*crlf && req_len < 510) request[req_len++] = *crlf++;
    
    request[req_len] = 0;
    
    terminal_writestring("[HTTP] Sending request...\n");
    
    // Для MVP возвращаем заглушку
    // В реальности здесь была бы отправка TCP пакета и получение ответа
    
    const char* mock_response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
    size_t resp_len = strlen(mock_response);
    
    if (resp_len >= max_len) {
        resp_len = max_len - 1;
    }
    
    memcpy(buffer, mock_response, resp_len);
    buffer[resp_len] = 0;
    
    terminal_writestring("[HTTP] Response received\n");
    
    return resp_len;
}
