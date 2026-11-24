#pragma once
// Skybox con dos cube maps (dia y noche) renderizados en Blender: cielo, volcanes
// (Popocatepetl e Iztaccihuatl) y la ciudad alrededor del estadio.
// Los mismos cube maps sirven de mapa de ambiente (environment mapping) para los
// reflejos Fresnel de los modelos.

#include <iostream>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "SOIL2/SOIL2.h"

class Skybox {
public:
    Skybox() : texturaDia(0), texturaNoche(0), programa(0), vao(0), vbo(0) {}

    // Carga <carpeta>/px.jpg ... nz.jpg. Si faltan imagenes usa un degradado generado en memoria.
    void inicializar(const std::string& carpetaDia, const std::string& carpetaNoche) {
        texturaDia = cargarCubo(carpetaDia);
        if (!texturaDia) { std::cerr << "Skybox de dia no encontrado en " << carpetaDia << ", se usa un degradado" << std::endl; texturaDia = cuboRespaldo(true); }
        texturaNoche = cargarCubo(carpetaNoche);
        if (!texturaNoche) { std::cerr << "Skybox de noche no encontrado en " << carpetaNoche << ", se usa un degradado" << std::endl; texturaNoche = cuboRespaldo(false); }

        const char* vs = "#version 330 core\n"
            "layout(location=0) in vec3 p; uniform mat4 view, projection; out vec3 direccion;\n"
            "void main(){ direccion = p; vec4 c = projection * mat4(mat3(view)) * vec4(p, 1.0); gl_Position = c.xyww; }";
        const char* fs = "#version 330 core\n"
            "in vec3 direccion; uniform samplerCube cieloDia; uniform samplerCube cieloNoche; uniform float mezclaDia; out vec4 color;\n"
            "void main(){ vec3 d = normalize(direccion);\n"
            "  vec3 c = mix(texture(cieloNoche, d).rgb, texture(cieloDia, d).rgb, mezclaDia);\n"
            "  color = vec4(c, 1.0); }";
        programa = glCreateProgram();
        GLuint v = compilar(GL_VERTEX_SHADER, vs), f = compilar(GL_FRAGMENT_SHADER, fs);
        glAttachShader(programa, v); glAttachShader(programa, f); glLinkProgram(programa);
        GLint ok = 0; glGetProgramiv(programa, GL_LINK_STATUS, &ok);
        if (!ok) std::cerr << "Skybox: fallo al enlazar el shader" << std::endl;
        glDeleteShader(v); glDeleteShader(f);
        glUseProgram(programa);
        glUniform1i(glGetUniformLocation(programa, "cieloDia"), 2);
        glUniform1i(glGetUniformLocation(programa, "cieloNoche"), 3);

        const float c[] = {
            -1, 1,-1, -1,-1,-1,  1,-1,-1,  1,-1,-1,  1, 1,-1, -1, 1,-1,
            -1,-1, 1, -1,-1,-1, -1, 1,-1, -1, 1,-1, -1, 1, 1, -1,-1, 1,
             1,-1,-1,  1,-1, 1,  1, 1, 1,  1, 1, 1,  1, 1,-1,  1,-1,-1,
            -1,-1, 1, -1, 1, 1,  1, 1, 1,  1, 1, 1,  1,-1, 1, -1,-1, 1,
            -1, 1,-1,  1, 1,-1,  1, 1, 1,  1, 1, 1, -1, 1, 1, -1, 1,-1,
            -1,-1,-1, -1,-1, 1,  1,-1,-1,  1,-1,-1, -1,-1, 1,  1,-1, 1 };
        glGenVertexArrays(1, &vao); glGenBuffers(1, &vbo);
        glBindVertexArray(vao); glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(c), c, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }

    // Deja los cube maps en las unidades 2 (dia) y 3 (noche) para el skybox y los reflejos.
    void enlazar() const {
        glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_CUBE_MAP, texturaDia);
        glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_CUBE_MAP, texturaNoche);
        glActiveTexture(GL_TEXTURE0);
    }

    // Se dibuja primero, sin escribir profundidad: todo lo demas queda delante.
    void dibujar(const glm::mat4& vista, const glm::mat4& proyeccion, float mezclaDia) const {
        if (!programa) return;
        enlazar();
        glDepthMask(GL_FALSE); glDisable(GL_DEPTH_TEST);
        glUseProgram(programa);
        glUniformMatrix4fv(glGetUniformLocation(programa, "view"), 1, GL_FALSE, glm::value_ptr(vista));
        glUniformMatrix4fv(glGetUniformLocation(programa, "projection"), 1, GL_FALSE, glm::value_ptr(proyeccion));
        glUniform1f(glGetUniformLocation(programa, "mezclaDia"), mezclaDia);
        glBindVertexArray(vao); glDrawArrays(GL_TRIANGLES, 0, 36); glBindVertexArray(0);
        glEnable(GL_DEPTH_TEST); glDepthMask(GL_TRUE);
    }

    void liberar() {
        if (texturaDia) glDeleteTextures(1, &texturaDia);
        if (texturaNoche) glDeleteTextures(1, &texturaNoche);
        if (vbo) glDeleteBuffers(1, &vbo);
        if (vao) glDeleteVertexArrays(1, &vao);
        if (programa) glDeleteProgram(programa);
        texturaDia = texturaNoche = vbo = vao = programa = 0;
    }

    GLuint texturaDia, texturaNoche;

private:
    GLuint programa, vao, vbo;

    static GLuint compilar(GLenum tipo, const char* fuente) {
        GLuint s = glCreateShader(tipo); glShaderSource(s, 1, &fuente, nullptr); glCompileShader(s);
        GLint ok = 0; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
        if (!ok) { char log[1024]; glGetShaderInfoLog(s, 1024, nullptr, log); std::cerr << "Skybox shader: " << log << std::endl; }
        return s;
    }

    // Orden de OpenGL: +X, -X, +Y, -Y, +Z, -Z. Las caras se generaron con la convencion
    // del cube map (fila superior = t 0), por eso se cargan sin invertir.
    static GLuint cargarCubo(const std::string& carpeta) {
        // Se leen las imagenes con SOIL_load_image y se suben a mano: SOIL_load_OGL_cubemap
        // consulta extensiones con glGetString(GL_EXTENSIONS), que no existe en el perfil Core.
        const char* nombres[6] = { "px.jpg", "nx.jpg", "py.jpg", "ny.jpg", "pz.jpg", "nz.jpg" };
        GLuint id = 0; glGenTextures(1, &id); glBindTexture(GL_TEXTURE_CUBE_MAP, id);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        int lado = -1;
        for (int i = 0; i < 6; ++i) {
            const std::string ruta = carpeta + "/" + nombres[i];
            int w = 0, h = 0, canales = 0;
            unsigned char* datos = SOIL_load_image(ruta.c_str(), &w, &h, &canales, SOIL_LOAD_RGB);
            bool valida = datos && w == h && w > 0 && (lado < 0 || w == lado);
            if (!valida) {
                if (datos) SOIL_free_image_data(datos);
                std::cerr << "Cara del skybox invalida o faltante: " << ruta << std::endl;
                glBindTexture(GL_TEXTURE_CUBE_MAP, 0); glDeleteTextures(1, &id); glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
                return 0;
            }
            lado = w;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, datos);
            SOIL_free_image_data(datos);
        }
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        return id;
    }

    // Degradado vertical de respaldo (para no fallar si falta la carpeta de imagenes).
    static GLuint cuboRespaldo(bool dia) {
        const int N = 32; unsigned char px[N * N * 3];
        GLuint id; glGenTextures(1, &id); glBindTexture(GL_TEXTURE_CUBE_MAP, id);
        const glm::vec3 arriba = dia ? glm::vec3(0.35f, 0.55f, 0.85f) : glm::vec3(0.01f, 0.015f, 0.05f);
        const glm::vec3 horizonte = dia ? glm::vec3(0.78f, 0.85f, 0.92f) : glm::vec3(0.12f, 0.08f, 0.08f);
        for (int cara = 0; cara < 6; ++cara) {
            for (int y = 0; y < N; ++y) for (int x = 0; x < N; ++x) {
                float t = (cara == 2) ? 1.0f : (cara == 3 ? 0.0f : 1.0f - (y + 0.5f) / N);
                glm::vec3 col = t > 0.5f ? glm::mix(horizonte, arriba, (t - 0.5f) * 2.0f) : horizonte * (0.3f + t);
                int k = (y * N + x) * 3;
                px[k] = (unsigned char)(255 * col.r); px[k + 1] = (unsigned char)(255 * col.g); px[k + 2] = (unsigned char)(255 * col.b);
            }
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + cara, 0, GL_RGB, N, N, 0, GL_RGB, GL_UNSIGNED_BYTE, px);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        return id;
    }
};
