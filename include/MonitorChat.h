#ifndef MONITORCHAT_H
#define MONITORCHAT_H

#include <string>
#include <fstream>

class MonitorChat {
public:
    MonitorChat(const std::string& direccionIP, int puerto);
    void conectarAlServidor();
    void solicitarInformacion();
    void desconectar();

private:
    void recibirInformacion();
    void escribirEnArchivo(const std::string& mensaje);

    std::string direccionIP;  // Dirección IP del servidor
    int puerto;               // Puerto del servidor
    int descriptorMonitor;    // Descriptor del socket del monitor
    bool conectado;           // Estado de la conexión
    std::ofstream archivo;    // Archivo para escribir la información
    std::string archivoBloqueo;  // Archivo de bloqueo
};

#endif // MONITORCHAT_H
