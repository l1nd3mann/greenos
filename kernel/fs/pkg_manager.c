#include "kernel.h"
#include "net.h"
#include "pkg_manager.h"

// Простая реализация пакетного менеджера для загрузки из GitHub

#define MAX_PACKAGES 16
static struct package_info installed_packages[MAX_PACKAGES];
static int installed_count = 0;

// Инициализация пакетного менеджера
void pkg_init() {
    terminal_writestring("[PKG] Package manager initialized\n");
    installed_count = 0;
}

// Поиск пакета (для MVP - заглушка)
int pkg_search(const char* query, struct package_info* results, int max_results) {
    terminal_writestring("[PKG] Searching for: ");
    terminal_writestring(query);
    terminal_writestring("\n");
    
    // Для MVP возвращаем тестовые результаты
    if (max_results > 0) {
        strcpy(results[0].name, "example-app");
        strcpy(results[0].version, "1.0.0");
        strcpy(results[0].description, "Example application for MiniOS");
        strcpy(results[0].download_url, "https://github.com/user/example-app/releases/download/v1.0.0/app.bin");
        results[0].size = 1024;
        return 1;
    }
    
    return 0;
}

// Простой парсер JSON для GitHub API (очень упрощенный)
static int find_json_string(const u8* json, const char* key, char* output, int max_len) {
    const u8* pos = strstr((const char*)json, key);
    if (!pos) return -1;
    
    pos += strlen(key);
    
    // Пропускаем пробелы и двоеточие
    while (*pos == ' ' || *pos == ':' || *pos == '"') pos++;
    
    if (*pos != '"') return -1;
    pos++;  // Пропускаем открывающую кавычку
    
    int i = 0;
    while (*pos && *pos != '"' && i < max_len - 1) {
        output[i++] = *pos++;
    }
    output[i] = 0;
    
    return 0;
}

// Парсинг ответа GitHub API
int parse_github_releases(const u8* json_response, struct package_info* releases, int max_releases) {
    terminal_writestring("[PKG] Parsing GitHub API response...\n");
    
    // Очень простой парсер - ищет теги и URL
    // В реальности нужен полноценный JSON парсер
    
    int count = 0;
    const u8* pos = json_response;
    
    // Ищем "tag_name" в JSON
    while (*pos && count < max_releases) {
        const u8* tag_pos = strstr((const char*)pos, "\"tag_name\"");
        if (!tag_pos) break;
        
        pos = tag_pos;
        
        // Извлекаем имя тега
        find_json_string(pos, "\"tag_name\"", releases[count].version, 32);
        
        // Формируем имя пакета (без snprintf)
        int name_len = 0;
        const char* pkg_prefix = "package-v";
        while (*pkg_prefix && name_len < 62) releases[count].name[name_len++] = *pkg_prefix++;
        char* ver_ptr = releases[count].version;
        while (*ver_ptr && name_len < 62) releases[count].name[name_len++] = *ver_ptr++;
        releases[count].name[name_len] = 0;
        
        // Извлекаем URL архива
        const u8* url_pos = strstr((const char*)pos, "\"tarball_url\"");
        if (url_pos) {
            find_json_string(url_pos, "\"tarball_url\"", releases[count].download_url, 256);
        }
        
        strcpy(releases[count].description, "GitHub release");
        releases[count].size = 0;  // Будет заполнено при загрузке
        
        count++;
        pos++;
    }
    
    terminal_writestring("[PKG] Found ");
    terminal_writedecimal(count);
    terminal_writestring(" release(s)\n");
    
    return count;
}

// Загрузка релиза из GitHub
int pkg_download_release(const char* owner, const char* repo, const char* version, u8* buffer, size_t max_size) {
    terminal_writestring("[PKG] Downloading from GitHub: ");
    terminal_writestring(owner);
    terminal_putchar('/');
    terminal_writestring(repo);
    terminal_writestring(" @ ");
    terminal_writestring(version);
    terminal_writestring("\n");
    
    // Формируем URL для GitHub API (без snprintf)
    char api_url[256];
    int url_len = 0;
    const char* prefix = "/repos/";
    const char* slash = "/";
    const char* tarball = "/tarball/";
    
    while (*prefix && url_len < 254) api_url[url_len++] = *prefix++;
    while (*owner && url_len < 254) api_url[url_len++] = *owner++;
    while (*slash && url_len < 254) api_url[url_len++] = *slash++;
    while (*repo && url_len < 254) api_url[url_len++] = *repo++;
    while (*tarball && url_len < 254) api_url[url_len++] = *tarball++;
    while (*version && url_len < 254) api_url[url_len++] = *version++;
    api_url[url_len] = 0;
    
    // Резолвим DNS
    struct ip_addr github_ip;
    if (dns_resolve("api.github.com", &github_ip) != 0) {
        terminal_writestring("[PKG] DNS resolution failed\n");
        return -1;
    }
    
    // HTTP GET запрос к GitHub API
    u8 response[4096];
    int resp_len = http_get(&github_ip, api_url, "api.github.com", response, sizeof(response));
    
    if (resp_len <= 0) {
        terminal_writestring("[PKG] Download failed\n");
        return -1;
    }
    
    // Проверяем HTTP статус
    if (strstr((char*)response, "200 OK") == 0) {
        terminal_writestring("[PKG] HTTP error\n");
        return -1;
    }
    
    // Находим начало тела ответа (после \r\n\r\n)
    u8* body_start = strstr((char*)response, "\r\n\r\n");
    if (!body_start) {
        terminal_writestring("[PKG] Invalid HTTP response\n");
        return -1;
    }
    body_start += 4;  // Пропускаем \r\n\r\n
    
    size_t body_len = resp_len - (body_start - response);
    
    if (body_len >= max_size) {
        body_len = max_size - 1;
    }
    
    memcpy(buffer, body_start, body_len);
    buffer[body_len] = 0;
    
    terminal_writestring("[PKG] Downloaded ");
    terminal_writedecimal(body_len);
    terminal_writestring(" bytes\n");
    
    return body_len;
}

// Установка пакета из GitHub репозитория
int pkg_install(const char* repo_owner, const char* repo_name) {
    terminal_writestring("[PKG] Installing package: ");
    terminal_writestring(repo_owner);
    terminal_putchar('/');
    terminal_writestring(repo_name);
    terminal_writestring("\n");
    
    // 1. Получаем список релизов через GitHub API (без snprintf)
    char api_url[256];
    int url_len = 0;
    const char* releases_prefix = "/repos/";
    const char* slash = "/";
    const char* releases_suffix = "/releases";
    
    while (*releases_prefix && url_len < 254) api_url[url_len++] = *releases_prefix++;
    while (*repo_owner && url_len < 254) api_url[url_len++] = *repo_owner++;
    while (*slash && url_len < 254) api_url[url_len++] = *slash++;
    while (*repo_name && url_len < 254) api_url[url_len++] = *repo_name++;
    while (*releases_suffix && url_len < 254) api_url[url_len++] = *releases_suffix++;
    api_url[url_len] = 0;
    
    // Резолвим DNS
    struct ip_addr github_ip;
    if (dns_resolve("api.github.com", &github_ip) != 0) {
        return -1;
    }
    
    // 2. Делаем HTTP запрос к GitHub API
    u8 response[8192];
    int resp_len = http_get(&github_ip, api_url, "api.github.com", response, sizeof(response));
    
    if (resp_len <= 0) {
        terminal_writestring("[PKG] Failed to fetch releases\n");
        return -1;
    }
    
    // 3. Парсим ответ
    struct package_info releases[16];
    int release_count = parse_github_releases(response, releases, 16);
    
    if (release_count == 0) {
        terminal_writestring("[PKG] No releases found\n");
        return -1;
    }
    
    // 4. Берем последний релиз
    terminal_writestring("[PKG] Latest release: ");
    terminal_writestring(releases[0].version);
    terminal_writestring("\n");
    
    // 5. Скачиваем релиз
    u8 package_data[16384];
    int pkg_size = pkg_download_release(repo_owner, repo_name, releases[0].version, 
                                         package_data, sizeof(package_data));
    
    if (pkg_size <= 0) {
        return -1;
    }
    
    // 6. Устанавливаем пакет
    return pkg_extract_and_install(package_data, pkg_size);
}

// Извлечение и установка пакета
int pkg_extract_and_install(const u8* package_data, size_t size) {
    terminal_writestring("[PKG] Extracting and installing package...\n");
    
    // Для MVP просто сохраняем данные в "файловую систему"
    // В реальности здесь была бы распаковка tar.gz и копирование файлов
    
    if (installed_count >= MAX_PACKAGES) {
        terminal_writestring("[PKG] Too many packages installed\n");
        return -1;
    }
    
    // Добавляем в список установленных
    strcpy(installed_packages[installed_count].name, "installed-app");
    strcpy(installed_packages[installed_count].version, "1.0.0");
    strcpy(installed_packages[installed_count].description, "Installed from GitHub");
    installed_packages[installed_count].size = size;
    
    installed_count++;
    
    terminal_writestring("[PKG] Package installed successfully!\n");
    terminal_writestring("[PKG] Total installed: ");
    terminal_writedecimal(installed_count);
    terminal_writestring("\n");
    
    return 0;
}

// Список установленных пакетов
void pkg_list_installed() {
    terminal_writestring("\n=== Installed Packages ===\n");
    
    if (installed_count == 0) {
        terminal_writestring("No packages installed\n");
        return;
    }
    
    for (int i = 0; i < installed_count; i++) {
        terminal_writestring("  - ");
        terminal_writestring(installed_packages[i].name);
        terminal_writestring(" v");
        terminal_writestring(installed_packages[i].version);
        terminal_writestring(" (");
        terminal_writedecimal(installed_packages[i].size);
        terminal_writestring(" bytes)\n");
    }
    
    terminal_writestring("=========================\n");
}

// Удаление пакета
int pkg_uninstall(const char* package_name) {
    terminal_writestring("[PKG] Uninstalling: ");
    terminal_writestring(package_name);
    terminal_writestring("\n");
    
    // Поиск пакета в списке
    for (int i = 0; i < installed_count; i++) {
        if (strcmp(installed_packages[i].name, package_name) == 0) {
            // Удаляем из списка (сдвигаем элементы)
            for (int j = i; j < installed_count - 1; j++) {
                installed_packages[j] = installed_packages[j + 1];
            }
            installed_count--;
            
            terminal_writestring("[PKG] Package uninstalled\n");
            return 0;
        }
    }
    
    terminal_writestring("[PKG] Package not found\n");
    return -1;
}
