#ifndef USUARIO_H
#define USUARIO_H

#include <string>

class Usuario {
public:
    Usuario(const std::string& nombreUsuario, const std::string& peliUsuario, int descriptorSocket);
    std::string obtenerNombreUsuario() const;
    std::string obtenerPeliculaUsuario() const;
    int obtenerDescriptorSocket() const;

private:
    std::string nombreUsuario;  // Nombre del usuario
    std::string peliUsuario;
    int descriptorSocket;      // Descriptor del socket del usuario
};

#endif // USUARIO_H
