#pragma once

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "SOIL2/SOIL2.h"

struct VerticeOBJ {
    glm::vec3 posicion;
    glm::vec3 normal;
    glm::vec2 uv;
};

struct ParteOBJ {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint textura = 0;
    GLsizei vertices = 0;
    glm::vec3 color = glm::vec3(0.8f);
    float alpha = 1.0f;          // 'd' del MTL: < 1 se dibuja con mezcla (vidrio, policarbonato)
    GLuint texturaSpec = 0;      // map_Ks: brillo horneado (1 = superficie pulida)
    float brillo = 32.0f;        // Ns: exponente especular de Phong
    float metalico = 0.0f;       // Pm: 1 = metal (el reflejo Fresnel toma el color del material)
    bool colorEquipo = false;
};

// Carga Wavefront OBJ/MTL sin Assimp para mantener el ejecutable compatible
// con las bibliotecas de 32 y 64 bits que ya contiene el proyecto.
class ModeloOBJ {
public:
    ModeloOBJ() : centro(0.0f), extensionMinima(0.0f), extensionMaxima(1.0f), cargado(false) {}

    bool cargar(const std::string& ruta, const std::string& filtroGrupo = std::string()) {
        std::ifstream archivo(ruta.c_str());
        if (!archivo.is_open()) {
            std::cerr << "No se pudo abrir el OBJ: " << ruta << std::endl;
            return false;
        }

        const std::string carpeta = directorio(ruta);
        std::vector<glm::vec3> posiciones, normales;
        std::vector<glm::vec2> coordenadas;
        std::map<std::string, std::vector<VerticeOBJ> > grupos;
        std::vector<std::string> ordenGrupos;
        std::map<std::string, size_t> indicesGrupo;
        std::vector<std::string> prefijosFiltro;
        std::vector<std::string> prefijosExcluidos;
        if (!filtroGrupo.empty()) {
            std::string prefijo;
            auto guardarPrefijo = [&]() {
                if (prefijo.empty()) return;
                if (prefijo[0] == '!') prefijosExcluidos.push_back(prefijo.substr(1));
                else prefijosFiltro.push_back(prefijo);
                prefijo.clear();
            };
            for (size_t i = 0; i < filtroGrupo.size(); ++i) {
                if (filtroGrupo[i] == '\\' && i + 1 < filtroGrupo.size() && filtroGrupo[i + 1] == '|') {
                    prefijo.push_back('|');
                    ++i;
                } else if (filtroGrupo[i] == '|') guardarPrefijo();
                else prefijo.push_back(filtroGrupo[i]);
            }
            guardarPrefijo();
        }
        std::string materialActual = "default", archivoMaterial, nombreGrupoOBJ;
        glm::vec3 minimo(1.0e30f), maximo(-1.0e30f);

        auto grupoActual = [&]() -> std::vector<VerticeOBJ>& {
            if (indicesGrupo.find(materialActual) == indicesGrupo.end()) {
                indicesGrupo[materialActual] = ordenGrupos.size();
                ordenGrupos.push_back(materialActual);
                grupos[materialActual] = std::vector<VerticeOBJ>();
            }
            return grupos[materialActual];
        };

        std::string linea;
        while (std::getline(archivo, linea)) {
            std::istringstream stream(linea);
            std::string tipo;
            stream >> tipo;
            if (tipo == "v") {
                glm::vec3 p; stream >> p.x >> p.y >> p.z; posiciones.push_back(p);
            } else if (tipo == "vt") {
                glm::vec2 uv; stream >> uv.x >> uv.y; coordenadas.push_back(uv);
            } else if (tipo == "vn") {
                glm::vec3 n; stream >> n.x >> n.y >> n.z; normales.push_back(n);
            } else if (tipo == "mtllib") {
                std::getline(stream, archivoMaterial);
                limpiar(archivoMaterial);
            } else if (tipo == "g" || tipo == "o") {
                std::getline(stream, nombreGrupoOBJ);
                limpiar(nombreGrupoOBJ);
            } else if (tipo == "usemtl") {
                std::getline(stream, materialActual);
                limpiar(materialActual);
            } else if (tipo == "f") {
                bool grupoPermitido = prefijosFiltro.empty();
                for (size_t i = 0; !grupoPermitido && i < prefijosFiltro.size(); ++i)
                    grupoPermitido = nombreGrupoOBJ.compare(0, prefijosFiltro[i].size(), prefijosFiltro[i]) == 0;
                for (size_t i = 0; grupoPermitido && i < prefijosExcluidos.size(); ++i)
                    if (nombreGrupoOBJ.compare(0, prefijosExcluidos[i].size(), prefijosExcluidos[i]) == 0)
                        grupoPermitido = false;
                if (!grupoPermitido) continue;
                std::vector<VerticeOBJ> cara;
                std::string token;
                while (stream >> token) {
                    int vi = 0, ti = 0, ni = 0;
                    leerIndices(token, vi, ti, ni);
                    VerticeOBJ v;
                    v.posicion = posiciones.at(indiceOBJ(vi, posiciones.size()));
                    v.uv = (ti != 0 && !coordenadas.empty()) ? coordenadas.at(indiceOBJ(ti, coordenadas.size())) : glm::vec2(0.0f);
                    v.normal = (ni != 0 && !normales.empty()) ? normales.at(indiceOBJ(ni, normales.size())) : glm::vec3(0.0f);
                    cara.push_back(v);
                }
                if (cara.size() < 3) continue;
                std::vector<VerticeOBJ>& vertices = grupoActual();
                for (size_t i = 1; i + 1 < cara.size(); ++i) {
                    VerticeOBJ triangulo[3] = { cara[0], cara[i], cara[i + 1] };
                    glm::vec3 normal = glm::normalize(glm::cross(triangulo[1].posicion - triangulo[0].posicion,
                                                                  triangulo[2].posicion - triangulo[0].posicion));
                    for (int k = 0; k < 3; ++k) {
                        if (glm::length(triangulo[k].normal) < 0.001f) triangulo[k].normal = normal;
                        vertices.push_back(triangulo[k]);
                        minimo = glm::min(minimo, triangulo[k].posicion);
                        maximo = glm::max(maximo, triangulo[k].posicion);
                    }
                }
            }
        }
        archivo.close();

        if (ordenGrupos.empty()) {
            std::cerr << "El OBJ no contiene caras: " << ruta << std::endl;
            return false;
        }
        centro = (minimo + maximo) * 0.5f;
        extensionMinima = minimo;
        glm::vec3 tamano = maximo - minimo;
        extensionMaxima = glm::max(tamano.x, glm::max(tamano.y, tamano.z));
        if (extensionMaxima < 0.0001f) extensionMaxima = 1.0f;

        std::map<std::string, glm::vec3> colores;
        std::map<std::string, std::string> mapas, mapasSpec;
        std::map<std::string, float> alfas, brillos, metales;
        if (!archivoMaterial.empty()) cargarMaterial(carpeta + archivoMaterial, colores, mapas, alfas, mapasSpec, brillos, metales);

        for (size_t i = 0; i < ordenGrupos.size(); ++i) {
            const std::string& nombre = ordenGrupos[i];
            const std::vector<VerticeOBJ>& vertices = grupos[nombre];
            if (vertices.empty()) continue;
            ParteOBJ parte;
            parte.vertices = (GLsizei)vertices.size();
            parte.colorEquipo = nombre.find("Verde") != std::string::npos;
            if (colores.find(nombre) != colores.end()) parte.color = colores[nombre];
            if (alfas.find(nombre) != alfas.end()) parte.alpha = alfas[nombre];
            if (brillos.find(nombre) != brillos.end()) parte.brillo = brillos[nombre];
            if (metales.find(nombre) != metales.end()) parte.metalico = metales[nombre];
            if (mapasSpec.find(nombre) != mapasSpec.end()) parte.texturaSpec = texturaCompartida(carpeta + mapasSpec[nombre]);
            if (mapas.find(nombre) != mapas.end()) {
                std::string archivoTextura = carpeta + mapas[nombre];
                parte.textura = texturaCompartida(archivoTextura);
                if (parte.textura == 0) std::cerr << "Textura no disponible: " << archivoTextura << std::endl;
            }
            glGenVertexArrays(1, &parte.vao);
            glGenBuffers(1, &parte.vbo);
            glBindVertexArray(parte.vao);
            glBindBuffer(GL_ARRAY_BUFFER, parte.vbo);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VerticeOBJ), vertices.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VerticeOBJ), (void*)offsetof(VerticeOBJ, posicion)); glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VerticeOBJ), (void*)offsetof(VerticeOBJ, normal)); glEnableVertexAttribArray(1);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VerticeOBJ), (void*)offsetof(VerticeOBJ, uv)); glEnableVertexAttribArray(2);
            glBindVertexArray(0);
            partes.push_back(parte);
        }
        cargado = !partes.empty();
        return cargado;
    }

    void dibujar(GLuint programa, const glm::mat4& vista, const glm::mat4& proyeccion,
                 const glm::vec3& posicion, float tamanoObjetivo, float rotacionY, bool iluminacion,
                 float rotacionX = 0.0f) const {
        if (!cargado) return;
        dibujarConMatriz(programa, vista, proyeccion, matrizInstancia(posicion, tamanoObjetivo, rotacionY, rotacionX), iluminacion);
    }

    // Matriz de una instancia: escala el lado mayor a 'tamanoObjetivo', apoya el modelo
    // en el suelo en 'posicion' y lo orienta. Tambien la usan las animaciones glTF que
    // comparten el sistema de coordenadas del OBJ (p. ej. el balon del gol en la porteria).
    glm::mat4 matrizInstancia(const glm::vec3& posicion, float tamanoObjetivo, float rotacionY, float rotacionX = 0.0f) const {
        float escala = tamanoObjetivo / extensionMaxima;
        glm::mat4 modelo(1.0f);
        glm::vec3 posicionConSuelo = posicion;
        // La posición de instancia representa el contacto del modelo con el suelo.
        posicionConSuelo.y += (centro.y - extensionMinima.y) * escala;
        modelo = glm::translate(modelo, posicionConSuelo);
        modelo = glm::rotate(modelo, glm::radians(rotacionY), glm::vec3(0, 1, 0));
        modelo = glm::rotate(modelo, glm::radians(rotacionX), glm::vec3(1, 0, 0));
        modelo = glm::scale(modelo, glm::vec3(escala));
        return glm::translate(modelo, -centro);
    }

    void dibujarConMatriz(GLuint programa, const glm::mat4& vista, const glm::mat4& proyeccion,
                          const glm::mat4& modelo, bool iluminacion,
                          const glm::vec3& colorEquipo = glm::vec3(1.0f), bool aplicarColorEquipo = false,
                          GLuint texturaEquipo = 0) const {
        if (!cargado) return;
        glUniformMatrix4fv(glGetUniformLocation(programa, "model"), 1, GL_FALSE, glm::value_ptr(modelo));
        glUniformMatrix4fv(glGetUniformLocation(programa, "view"), 1, GL_FALSE, glm::value_ptr(vista));
        glUniformMatrix4fv(glGetUniformLocation(programa, "projection"), 1, GL_FALSE, glm::value_ptr(proyeccion));
        glUniform1i(glGetUniformLocation(programa, "lamps"), iluminacion ? 1 : 0);
        glUniform3fv(glGetUniformLocation(programa, "teamColor"), 1, glm::value_ptr(colorEquipo));
        for (size_t i = 0; i < partes.size(); ++i) {
            const ParteOBJ& parte = partes[i];
            GLuint texturaActiva = aplicarColorEquipo && parte.colorEquipo && texturaEquipo ? texturaEquipo : parte.textura;
            bool reemplazarColor = aplicarColorEquipo && parte.colorEquipo && texturaActiva == 0;
            glUniform3fv(glGetUniformLocation(programa, "materialColor"), 1, glm::value_ptr(parte.color));
            glUniform1f(glGetUniformLocation(programa, "materialAlpha"), parte.alpha);
            const bool transparente = parte.alpha < 0.999f;
            if (transparente) {   // vidrio/policarbonato: mezcla sin escribir profundidad
                glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glDepthMask(GL_FALSE);
            }
            glUniform1i(glGetUniformLocation(programa, "teamColorable"), reemplazarColor ? 1 : 0);
            glUniform1i(glGetUniformLocation(programa, "hasTexture"), texturaActiva != 0 ? 1 : 0);
            // Phong/Fresnel: el mapa de brillo solo vale para la textura propia del modelo
            const GLuint spec = texturaActiva == parte.textura ? parte.texturaSpec : 0;
            glUniform1i(glGetUniformLocation(programa, "hasSpecMap"), spec != 0 ? 1 : 0);
            glUniform1f(glGetUniformLocation(programa, "shininess"), parte.brillo);
            glUniform1f(glGetUniformLocation(programa, "metallic"), parte.metalico);
            glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, spec);
            glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, texturaActiva);
            glBindVertexArray(parte.vao); glDrawArrays(GL_TRIANGLES, 0, parte.vertices);
            if (transparente) { glDepthMask(GL_TRUE); glDisable(GL_BLEND); }
        }
        glUniform1f(glGetUniformLocation(programa, "materialAlpha"), 1.0f);
        glUniform1i(glGetUniformLocation(programa, "hasSpecMap"), 0);
        glUniform1f(glGetUniformLocation(programa, "metallic"), 0.0f);
        glBindVertexArray(0);
    }

    void liberar() {
        for (size_t i = 0; i < partes.size(); ++i) {
            // las texturas viven en la cache compartida (otras instancias pueden usarlas)
            if (partes[i].vbo) glDeleteBuffers(1, &partes[i].vbo);
            if (partes[i].vao) glDeleteVertexArrays(1, &partes[i].vao);
        }
        partes.clear(); cargado = false;
    }

    // Cache de texturas: varios materiales/modelos comparten el mismo atlas horneado
    // (p. ej. Goyo tiene 14 materiales sobre una imagen y el jugador se carga 12 veces);
    // sin cache se subiria la misma imagen a la GPU decenas de veces.
    static GLuint texturaCompartida(const std::string& ruta) {
        static std::map<std::string, GLuint> cache;
        std::map<std::string, GLuint>::iterator it = cache.find(ruta);
        if (it != cache.end()) return it->second;
        // Mipmaps: evitan el parpadeo de las texturas finas (butacas, red, LEDs) a distancia.
        GLuint id = SOIL_load_OGL_texture(ruta.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
            SOIL_FLAG_INVERT_Y | SOIL_FLAG_TEXTURE_REPEATS | SOIL_FLAG_MIPMAPS);
        if (id) cache[ruta] = id;
        return id;
    }

    bool listo() const { return cargado; }
    glm::vec3 centroModelo() const { return centro; }
    float extensionVerticalMinima() const { return extensionMinima.y; }
    float extensionMaximaModelo() const { return extensionMaxima; }

private:
    std::vector<ParteOBJ> partes;
    glm::vec3 centro;
    glm::vec3 extensionMinima;
    float extensionMaxima;
    bool cargado;

    static std::string directorio(const std::string& ruta) {
        size_t p = ruta.find_last_of("/\\");
        return p == std::string::npos ? std::string() : ruta.substr(0, p + 1);
    }
    static void limpiar(std::string& texto) {
        size_t a = texto.find_first_not_of(" \t\r\n"), b = texto.find_last_not_of(" \t\r\n");
        texto = (a == std::string::npos) ? std::string() : texto.substr(a, b - a + 1);
    }
    static int indiceOBJ(int indice, size_t cuenta) {
        return indice > 0 ? indice - 1 : (int)cuenta + indice;
    }
    static void leerIndices(const std::string& token, int& vi, int& ti, int& ni) {
        std::istringstream s(token); std::string part;
        if (std::getline(s, part, '/')) vi = part.empty() ? 0 : std::stoi(part);
        if (std::getline(s, part, '/')) ti = part.empty() ? 0 : std::stoi(part);
        if (std::getline(s, part, '/')) ni = part.empty() ? 0 : std::stoi(part);
    }
    static void cargarMaterial(const std::string& ruta, std::map<std::string, glm::vec3>& colores,
                               std::map<std::string, std::string>& mapas, std::map<std::string, float>& alfas,
                               std::map<std::string, std::string>& mapasSpec, std::map<std::string, float>& brillos,
                               std::map<std::string, float>& metales) {
        std::ifstream archivo(ruta.c_str());
        if (!archivo.is_open()) return;
        std::string linea, actual;
        while (std::getline(archivo, linea)) {
            std::istringstream s(linea); std::string tipo; s >> tipo;
            if (tipo == "newmtl") { std::getline(s, actual); limpiar(actual); colores[actual] = glm::vec3(0.8f); }
            else if (tipo == "Kd" && !actual.empty()) s >> colores[actual].r >> colores[actual].g >> colores[actual].b;
            else if (tipo == "d" && !actual.empty()) { float a = 1.0f; s >> a; alfas[actual] = a < 0.05f ? 1.0f : a; }
            else if (tipo == "map_Kd" && !actual.empty()) { std::getline(s, mapas[actual]); limpiar(mapas[actual]); }
            else if (tipo == "map_Ks" && !actual.empty()) { std::getline(s, mapasSpec[actual]); limpiar(mapasSpec[actual]); }
            else if (tipo == "Ns" && !actual.empty()) s >> brillos[actual];
            else if (tipo == "Pm" && !actual.empty()) s >> metales[actual];
        }
    }
};
