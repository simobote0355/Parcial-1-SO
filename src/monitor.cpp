#include "MonitorChat.h"
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <cstring>
#include <fcntl.h>
#include <sys/file.h>

// Constructor que inicializa la dirección IP y el puerto del servidor
MonitorChat::MonitorChat(const std::string& direccionIP, int puerto)
    : direccionIP(direccionIP), puerto(puerto), descriptorMonitor(-1), conectado(false), archivoBloqueo("informacion_monitor.lock") {
    // Abrir el archivo para escritura (añadir o crear si no existe)
    archivo.open("informacion_monitor.txt", std::ios::out | std::ios::app);
    if (!archivo.is_open()) {
        std::cerr << "Error al abrir el archivo.\n";
    }
}

// Método para conectar al servidor
void MonitorChat::conectarAlServidor() {
    descriptorMonitor = socket(AF_INET, SOCK_STREAM, 0);
    if (descriptorMonitor == -1) {
        std::cerr << "Error al crear el socket del monitor.\n";
        return;
    }

    sockaddr_in direccionServidor;
    direccionServidor.sin_family = AF_INET;
    direccionServidor.sin_port = htons(puerto);
    inet_pton(AF_INET, direccionIP.c_str(), &direccionServidor.sin_addr);

    if (connect(descriptorMonitor, (sockaddr*)&direccionServidor, sizeof(direccionServidor)) == -1) {
        std::cerr << "Error al conectar al servidor.\n";
        return;
    }

    conectado = true;

    std::thread hiloRecibir(&MonitorChat::recibirInformacion, this);
    hiloRecibir.detach();
}

// Método para solicitar información al servidor
void MonitorChat::solicitarInformacion() {
    if (conectado) {
        std::string solicitud = "@conexion";
        send(descriptorMonitor, solicitud.c_str(), solicitud.size(), 0);
    }
    if (conectado) {
        std::string solicitud = "@monitorear";
        send(descriptorMonitor, solicitud.c_str(), solicitud.size(), 0);
    }
}

// Método para desconectar del servidor
void MonitorChat::desconectar() {
    if (conectado) {
        close(descriptorMonitor);
        conectado = false;
    }
    if (archivo.is_open()) {
        archivo.close();  // Cerrar el archivo al desconectar
    }
}

// Método para escribir en el archivo
void MonitorChat::escribirEnArchivo(const std::string& mensaje) {
    if (archivo.is_open()) {
        int fd = open(archivoBloqueo.c_str(), O_CREAT | O_RDWR, 0666);
        if (fd == -1) {
            std::cerr << "Error al abrir el archivo de bloqueo.\n";
            return;
        }
        
        // Adquirir el bloqueo
        if (flock(fd, LOCK_EX) == -1) {
            std::cerr << "Error al adquirir el bloqueo.\n";
            close(fd);
            return;
        }

        archivo << mensaje << std::endl;  // Escribir el mensaje en el archivo

        // Liberar el bloqueo
        flock(fd, LOCK_UN);
        close(fd);
    } else {
        std::cerr << "Error: El archivo no está abierto.\n";
    }
}

// Método para recibir la información del servidor
void MonitorChat::recibirInformacion() {
    char buffer[1024];
    while (conectado) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytesRecibidos = recv(descriptorMonitor, buffer, 1024, 0);
        if (bytesRecibidos <= 0) {
            std::cerr << "Desconectado del servidor.\n";
            desconectar();
            break;
        }
        std::string mensaje = std::string(buffer, bytesRecibidos);
        escribirEnArchivo(mensaje);  // Escribir la información en el archivo
        std:: cerr << mensaje;
    }
}
