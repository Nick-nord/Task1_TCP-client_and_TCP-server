// Задание 1. «TCP-клиент» и «Многопоточный TCP-сервер».cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

// Для успешной компиляции необходимо подключить библиотеку Ws2_32.lib
//#pragma comment(lib, "Ws2_32.lib")

int main()
{
    // --- 1. Инициализация Winsock ---
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "Initialization error Winsock: " << iResult << std::endl;
        return 1;
    }

    // --- 2. Создание сокета ---
    SOCKET ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
    // Создаётся объект сокета.
    // AF_INET: Указывает, что мы используем протокол IPv4.
    // SOCK_STREAM : Указывает на использование потокового сокета, который обеспечивает надёжную доставку данных(TCP).
    // IPPROTO_TCP : Явно указывает протокол TCP.
    if (ConnectSocket == INVALID_SOCKET) {
        std::cerr << "Error when creating a socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // --- 3. Настройка адреса сервера ---
    // Заполняется структура sockaddr_in, которая содержит информацию о том, куда нужно подключаться : 
    // IP - адрес("127.0.0.1" — это адрес вашего же компьютера, localhost) и порт(55555).
    // Номер порта должен совпадать с тем, на котором ожидает подключения ваш сервер.
    sockaddr_in clientService;
    clientService.sin_family = AF_INET; // IPv4
    // inet_addr преобразует строковый IP-адрес в числовой формат
    clientService.sin_addr.s_addr = inet_addr("127.0.0.1"); // Адрес сервера (localhost)
    clientService.sin_port = htons(55555);                 // Порт сервера

    // --- 4. Подключение к серверу ---
    // Это ключевое отличие от UDP. Клиент инициирует активное соединение с сервером. Если сервер не запущен или недоступен, эта функция завершится с ошибкой.
    iResult = connect(ConnectSocket, (SOCKADDR*)&clientService, sizeof(clientService));
    if (iResult == SOCKET_ERROR) {
        std::cerr << "Couldn't connect to the server. Mistake: " << WSAGetLastError() << std::endl;
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to the server." << std::endl;

    // --- 5. Отправка строки на сервер ---
    // Функция send используется для передачи данных через уже установленное соединение. 
    // Ей не требуется указывать адрес получателя, так как он уже известен после успешного вызова connect.
    const char* sendbuf = "A test string from the client";
    iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
    if (iResult == SOCKET_ERROR) {
        std::cerr << "Error sending data: " << WSAGetLastError() << std::endl;
    }
    else {
        std::cout << "Bytes sent: " << iResult << std::endl;
    }

    // --- 6. Получение ответа от сервера ---
    // Функция recv блокирует выполнение программы до тех пор, пока не придут данные от сервера или соединение не будет разорвано. Прочитанные данные помещаются в буфер recvbuf.
    char recvbuf[512];
    int bytesReceived = recv(ConnectSocket, recvbuf, 512, 0);
    if (bytesReceived > 0) {
        std::cout << "Response from the server (" << bytesReceived << " bytes): ";
        std::cout.write(recvbuf, bytesReceived);
        std::cout << std::endl;
    }
    else if (bytesReceived == 0) {
        std::cout << "The connection is closed by the server." << std::endl;
    }
    else {
        std::cerr << "Error receiving data: " << WSAGetLastError() << std::endl;
    }

    // --- 7. Закрытие соединения и очистка ресурсов ---
    // После обмена данными сокет корректно закрывается.Сначала вызывается shutdown для завершения операций ввода - вывода, 
    // а затем closesocket для освобождения дескриптора сокета.Наконец, WSACleanup() выгружает библиотеку Winsock.
    iResult = shutdown(ConnectSocket, SD_SEND); // Запрещаем дальнейшую отправку данных по сокету
    if (iResult == SOCKET_ERROR) {
        std::cerr << "Error when shutting down the socket: " << WSAGetLastError() << std::endl;
    }

    closesocket(ConnectSocket);
    WSACleanup();

    std::cout << "The client has completed the work." << std::endl;

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
