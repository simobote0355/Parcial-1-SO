// Monitor.cpp
#include <iostream>
#include <thread>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

void manejarCliente(int descriptorCliente) {
    char buffer[1024];
    ssize_t bytesRecibidos;

    while ((bytesRecibidos = recv(descriptorCliente, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytesRecibidos] = '\0';
        std::cout << "Mensaje recibido del servidor: " << buffer << std::endl;
    }

    close(descriptorCliente);
}

void iniciarClienteMonitor(const char* direccionServidor, int puerto, int numeroCliente) {
    int descriptorCliente = socket(AF_INET, SOCK_STREAM, 0);
    if (descriptorCliente == -1) {
        std::cerr << "Error al crear el socket del cliente monitor.\n";
        return;
    }

    sockaddr_in direccion;
    direccion.sin_family = AF_INET;
    direccion.sin_port = htons(puerto);
    if (inet_pton(AF_INET, direccionServidor, &direccion.sin_addr) <= 0) {
        std::cerr << "Dirección IP inválida.\n";
        close(descriptorCliente);
        return;
    }

    if (connect(descriptorCliente, (sockaddr*)&direccion, sizeof(direccion)) == -1) {
        std::cerr << "Error al conectar con el servidor de chat en el puerto " << puerto << ".\n";
        close(descriptorCliente);
        return;
    }

    std::cout << "Cliente monitor " << numeroCliente << " conectado al servidor de chat en el puerto " << puerto << ".\n";

    // Enviar un identificador de monitor al servidor
    std::string mensajeMonitor = "MONITOR";
    send(descriptorCliente, mensajeMonitor.c_str(), mensajeMonitor.size(), 0);

    // Manejar la recepción de datos del servidor
    manejarCliente(descriptorCliente);
}


int main() {
    // Direcciones y puertos de los servidores de chat
    const char* direccionServidor = "127.0.0.1";
    int puertos[] = {12346, 12357, 12358};

    // Iniciar 3 clientes monitor en hilos separados, conectados a diferentes puertos
    std::thread cliente1m(iniciarClienteMonitor, direccionServidor, puertos[0], 1);
    std::thread cliente2m(iniciarClienteMonitor, direccionServidor, puertos[1], 2);
    std::thread cliente3m(iniciarClienteMonitor, direccionServidor, puertos[2], 3);

    cliente1m.join();
    cliente2m.join();
    cliente3m.join();

    return 0;
}
