#ifndef SERVIDORCHAT_H
#define SERVIDORCHAT_H

#include "Usuario.h"
#include <string>
#include <vector>
#include <mutex>

class ServidorChat {
public:
    ServidorChat(int puerto);
    void iniciar();
    void monitorearEstado();

private:
    void manejarCliente(int descriptorCliente);
    void enviarMensajeATodos(const std::string& mensaje, int descriptorRemitente);
    void enviarListaUsuarios(int descriptorCliente);
    void enviarDetallesConexion(int descriptorCliente);
    void enviarEstadoMonitoreo(int descriptorCliente);
    int conectarAServidorRemoto(const std::string& ipServidor, int puertoServidor); // Declaración


    int puerto;  // Puerto en el que escucha el servidor
    int descriptorServidor;  // Descriptor del socket del servidor
    std::vector<Usuario> usuarios;  // Lista de usuarios conectados
    std::vector<int> usuariosMonitores;
    std::mutex mutexUsuarios;  // Mutex para proteger el acceso a la lista de usuarios
    
};

#endif // SERVIDORCHAT_H
