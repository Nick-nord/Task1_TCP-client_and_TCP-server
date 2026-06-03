// TCPServer.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <cstring> // Для memset

// --- Кроссплатформенные заголовки ---
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib") // Линковка Winsock библиотеки для Windows
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h> // Для close()
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

using namespace std;

// Мьютекс для безопасного вывода в консоль из разных потоков
std::mutex cout_mutex;

// Константы
const char* SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 55555;
const int MAX_CLIENTS = 100; // Максимальное количество ожидающих соединений в очереди

/**
 * @brief Обрабатывает логику общения с одним клиентом.
 * Эта функция выполняется в отдельном потоке для каждого соединения.
 *
 * @param client_sock Дескриптор сокета подключенного клиента.
 */
void handle_client(SOCKET client_sock) {
    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];

    // Блокируем мьютекс перед выводом в консоль
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        cout << "[+] The client has connected. Socket: " << client_sock << endl;
    }

    // Принимаем данные от клиента
    int bytes_received = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received > 0) {
        // Завершаем строку нулевым символом
        buffer[bytes_received] = '\0';

        // Безопасный вывод в консоль
        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            cout << "[>] Received from the client (" << bytes_received << " bytes): " << buffer << endl;
        }

        // Формируем ответ
        string response = "Server response: I received your message!";

        // Отправляем ответ клиенту
        send(client_sock, response.c_str(), response.length(), 0);
        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            cout << "[<] Sent to the client: " << response << endl;
        }
    }
    else {
        std::lock_guard<std::mutex> lock(cout_mutex);
        cerr << "[-] An error occurred when receiving data or the client was disconnected." << endl;
    }

    // Закрываем сокет клиента
#ifdef _WIN32
    closesocket(client_sock);
#else
    close(client_sock);
#endif

    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        cout << "[-] The client has disconnected. Socket: " << client_sock << endl;
    }
}

int main() {
    vector<thread> threads; // Вектор для хранения потоков
    SOCKET listen_sock, client_sock;
    sockaddr_in server_addr, client_addr;

    // --- Инициализация сети ---
#ifdef _WIN32
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        cerr << "Initialization error Winsock!" << endl;
        return 1;
    }
#endif

    // Создание слушающего сокета
    listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_sock == INVALID_SOCKET) {
        cerr << "Failed to create a socket! Error code: ";
#ifdef _WIN32
        cerr << WSAGetLastError();
#else
        cerr << errno;
#endif
        cerr << endl;
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    // Подготовка структуры адреса сервера
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // Привязка сокета к адресу и порту
    if (bind(listen_sock, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        cerr << "Socket binding failed!" << endl;
#ifdef _WIN32
        closesocket(listen_sock);
        WSACleanup();
#else
        close(listen_sock);
#endif
        return 1;
    }

    // Начало прослушивания порта
    if (listen(listen_sock, MAX_CLIENTS) == SOCKET_ERROR) {
        cerr << "Listening failed!" << endl;
#ifdef _WIN32
        closesocket(listen_sock);
        WSACleanup();
#else
        close(listen_sock);
#endif
        return 1;
    }

    cout << "=== The server is running on " << SERVER_IP << ":" << SERVER_PORT << " ===" << endl;

    socklen_t client_addr_len = sizeof(client_addr);

    // Главный цикл принятия соединений
    while (true) {
        std::cout << "[*] Waiting for a new connection..." << endl;
        client_sock = accept(listen_sock, (sockaddr*)&client_addr, &client_addr_len);
        if (client_sock == INVALID_SOCKET) {
            cerr << "Connection acceptance error!" << endl;
            continue; // Продолжаем ждать новых клиентов
        }

        // Создаем новый поток для обработки клиента
        threads.emplace_back(handle_client, client_sock);

        // Отсоединяем поток (detach), чтобы он завершился самостоятельно,
        // а ресурсы освободились автоматически.
        threads.back().detach();
    }

    // Освобождение ресурсов (в данном коде недостижимо из-за бесконечного цикла)
#ifdef _WIN32
    closesocket(listen_sock);
    WSACleanup();
#else
    close(listen_sock);
#endif

    for (auto& th : threads) {
        if (th.joinable()) th.join();
    }

    return 0;
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
