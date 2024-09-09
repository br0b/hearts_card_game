
![out](https://github.com/user-attachments/assets/b7f8cdde-b574-4128-a92a-230753246c98)

A CLI implementation of the Hearts card game written in C++. Uses the server-client architecture with TCP/IP for communication.
Supports both IPv4 and IPv6.

1. Build the project with `make`
2. Run server with `./kierki-serwer -f <game config file> [-p <port number>] [-t <timeout in seconds>]`.
3. Run a client with `./kierki-klient -h <host> -p <port> {-N, -E, -S, -W} {-a} {-4} {-6}`.
