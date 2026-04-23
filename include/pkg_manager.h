#ifndef PKG_MANAGER_H
#define PKG_MANAGER_H

#include "kernel.h"

// Информация о пакете
struct package_info {
    char name[64];
    char version[32];
    char description[256];
    char download_url[256];
    u32 size;
};

// Инициализация пакетного менеджера
void pkg_init();

// Поиск пакета в репозитории
int pkg_search(const char* query, struct package_info* results, int max_results);

// Загрузка пакета из GitHub
int pkg_install(const char* repo_owner, const char* repo_name);

// Загрузка конкретного релиза
int pkg_download_release(const char* owner, const char* repo, const char* version, u8* buffer, size_t max_size);

// Парсинг JSON ответа от GitHub API (упрощенный)
int parse_github_releases(const u8* json_response, struct package_info* releases, int max_releases);

// Установка загруженного пакета
int pkg_extract_and_install(const u8* package_data, size_t size);

// Список установленных пакетов
void pkg_list_installed();

// Удаление пакета
int pkg_uninstall(const char* package_name);

#endif // PKG_MANAGER_H
