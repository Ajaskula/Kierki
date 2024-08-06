#ifndef SERVER_H
#define SERVER_H

#include <string>

int parseArguments(int argc, char* argv[], uint16_t& port, std::string& file, int& timeout);

class Server{
    public:
        Server(uint16_t port = 0, const string& file = "", int timeout = 5);
        // destruktor
        ~Server();
        Server(uint16_t port = 0, const string& file = "", int timeout = 5)
        : port(port), file(file), timeout(timeout)
        {
            cout << "Instancja serwera została stworzona\n";
        }

    private:
        uint16_t port;
        std::string file;
        int timeout;
}
#endif // SERVER_H