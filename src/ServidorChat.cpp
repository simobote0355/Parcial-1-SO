#include "ServidorChat.h"
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <cstring>
#include <chrono>
#include <mutex>
#include <vector>
#include <string>

// Constructor que inicializa el puerto del servidor
ServidorChat::ServidorChat(int puerto)
    : puerto(puerto), descriptorServidor(-1) {}

//////
int ServidorChat::conectarAServidorRemoto(const std::string& ipServidor, int puertoServidor) {
    int descriptorCliente = socket(AF_INET, SOCK_STREAM, 0);
    if (descriptorCliente == -1) {
        std::cerr << "Error al crear el socket de cliente.\n";
        return -1;
    }

    sockaddr_in direccionServidor;
    direccionServidor.sin_family = AF_INET;
    direccionServidor.sin_port = htons(puertoServidor);
    inet_pton(AF_INET, ipServidor.c_str(), &direccionServidor.sin_addr);

    if (connect(descriptorCliente, (sockaddr*)&direccionServidor, sizeof(direccionServidor)) == -1) {
        //std::cerr << "Error al conectar con el servidor remoto.\n";
        close(descriptorCliente);
        return -1;
    }

    return descriptorCliente;
}
//////

// Método para iniciar el servidor
void ServidorChat::iniciar() {
    // Crear el socket del servidor
    descriptorServidor = socket(AF_INET, SOCK_STREAM, 0);
    if (descriptorServidor == -1) {
        std::cerr << "Error al crear el socket del servidor.\n";
        return;
    }

    sockaddr_in direccionServidor;
    direccionServidor.sin_family = AF_INET;
    direccionServidor.sin_port = htons(puerto);
    direccionServidor.sin_addr.s_addr = INADDR_ANY;

    // Asociar el socket a la dirección y puerto
    if (bind(descriptorServidor, (sockaddr*)&direccionServidor, sizeof(direccionServidor)) == -1) {
        std::cerr << "Error al hacer bind del socket del servidor.\n";
        return;
    }

    // Poner el servidor en modo escucha
    if (listen(descriptorServidor, 10) == -1) {
        std::cerr << "Error al poner el servidor en modo escucha.\n";
        return;
    }

    std::cout << "Servidor iniciado en el puerto " << puerto << ". Esperando conexiones...\n";




    // Aceptar conexiones entrantes
    while (true) {
        sockaddr_in direccionCliente;
        socklen_t tamanoDireccionCliente = sizeof(direccionCliente);
        int descriptorCliente = accept(descriptorServidor, (sockaddr*)&direccionCliente, &tamanoDireccionCliente);

        if (descriptorCliente == -1) {
            std::cerr << "Error al aceptar la conexión de un cliente.\n";
            continue;
        }
        if (usuarios.size() >= 5) {
            std::string mensaje = "Servidor lleno. No se pueden conectar más clientes.";
            send(descriptorCliente, mensaje.c_str(), mensaje.size(), 0);
            close(descriptorCliente);
            continue;
        }
        // Crear un hilo para manejar el cliente
        std::thread hiloCliente(&ServidorChat::manejarCliente, this, descriptorCliente);
        hiloCliente.detach();
    }
}

// Manejar la comunicación con un cliente
void ServidorChat::manejarCliente(int descriptorCliente) {
    char buffer[1024];

    // Pedir el nombre del usuario
    send(descriptorCliente, "Ingrese su nombre: ", 19, 0);
    ssize_t bytesRecibidos = recv(descriptorCliente, buffer, 1024, 0);
    if (bytesRecibidos <= 0) {
        close(descriptorCliente);
        return;
    }
    std::string nombreUsuario = std::string(buffer, bytesRecibidos);
    nombreUsuario.erase(nombreUsuario.find_last_not_of(" \n\r\t") + 1);

    // Pedir el nombre de la película
    send(descriptorCliente, "Ingrese el nombre de la película que deseas ver: ", 35, 0);
    bytesRecibidos = recv(descriptorCliente, buffer, 1024, 0);
    if (bytesRecibidos <= 0) {
        close(descriptorCliente);
        return;
    }
    std::string peliUsuario = std::string(buffer, bytesRecibidos);
    peliUsuario.erase(peliUsuario.find_last_not_of(" \n\r\t") + 1);

    {
        std::lock_guard<std::mutex> lock(mutexUsuarios);
        usuarios.emplace_back(nombreUsuario, peliUsuario, descriptorCliente);
    }

    std::string mensajeBienvenida = nombreUsuario + " se ha conectado.\n";
    enviarMensajeATodos(mensajeBienvenida, descriptorCliente);
//////

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        bytesRecibidos = recv(descriptorCliente, buffer, 1024, 0);

        if (bytesRecibidos <= 0) {
            {
                std::lock_guard<std::mutex> lock(mutexUsuarios);
                for (auto it = usuarios.begin(); it != usuarios.end(); ++it) {
                    if (it->obtenerDescriptorSocket() == descriptorCliente) {
                        std::string mensajeDespedida = it->obtenerNombreUsuario() + " se ha desconectado.\n";
                        enviarMensajeATodos(mensajeDespedida, descriptorCliente);
                        usuarios.erase(it);
                        break;
                    }
                }
            }
            close(descriptorCliente);
            break;
        }

        std::string mensaje = std::string(buffer, bytesRecibidos);
        
        //Procesar comandos del protocolo
        if (mensaje.substr(0, 9) == "@usuarios") {
            enviarListaUsuarios(descriptorCliente);
        } else if (mensaje.substr(0, 9) == "@conexion") {
            enviarDetallesConexion(descriptorCliente);
        } else if (mensaje.substr(0, 6) == "@salir") {
            close(descriptorCliente);
            break;
        } else if (mensaje.substr(0, 11) == "@monitorear") {
            enviarEstadoMonitoreo(descriptorCliente);
        }else if (mensaje.substr(0, 2) == "@h") {
            std::string ayuda = "Comandos disponibles:\n"
                                "@usuarios - Lista de usuarios conectados\n"
                                "@conexion - Muestra la conexión y el número de usuarios\n"
                                "@salir - Desconectar del chat\n";
            send(descriptorCliente, ayuda.c_str(), ayuda.size(), 0);
        } else {
            mensaje = nombreUsuario + ": " + mensaje;
            enviarMensajeATodos(mensaje, descriptorCliente);
        }
    }
}

// Enviar un mensaje a todos los usuarios conectados, excepto al remitente
void ServidorChat::enviarMensajeATodos(const std::string& mensaje, int descriptorRemitente) {
    std::lock_guard<std::mutex> lock(mutexUsuarios);
    for (const auto& usuario : usuarios) {
        if (usuario.obtenerDescriptorSocket() != descriptorRemitente) {
            send(usuario.obtenerDescriptorSocket(), mensaje.c_str(), mensaje.size(), 0);
        }
    }
}

// Enviar la lista de usuarios conectados al cliente especificado
void ServidorChat::enviarListaUsuarios(int descriptorCliente) {
    std::lock_guard<std::mutex> lock(mutexUsuarios);
    std::string listaUsuarios = "Usuarios conectados:\n";
    for (const auto& usuario : usuarios) {
        listaUsuarios += usuario.obtenerNombreUsuario() + "\n";
    }
    send(descriptorCliente, listaUsuarios.c_str(), listaUsuarios.size(), 0);
}

// Enviar los detalles de la conexión y el número de usuarios conectados
void ServidorChat::enviarDetallesConexion(int descriptorCliente) {
    std::lock_guard<std::mutex> lock(mutexUsuarios);
    std::string detalles = "Número de usuarios conectados: " + std::to_string(usuarios.size()-1) + "\n";
    send(descriptorCliente, detalles.c_str(), detalles.size(), 0);
}



void ServidorChat::enviarEstadoMonitoreo(int descriptorCliente) {
    std::lock_guard<std::mutex> lock(mutexUsuarios);
    std::string estadoMonitoreo = "Estado de los clientes:\n";
    for (const auto& usuario : usuarios) {
        estadoMonitoreo += usuario.obtenerNombreUsuario() + " está viendo " + usuario.obtenerPeliculaUsuario() +"\n";
        
    }
    send(descriptorCliente, estadoMonitoreo.c_str(), estadoMonitoreo.size(), 0);
}

