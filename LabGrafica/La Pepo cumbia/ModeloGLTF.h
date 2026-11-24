#pragma once
// Cargador glTF 2.0 con esqueleto (skinning por GPU) y animaciones exportadas desde Blender.
// Usa cgltf (un solo encabezado, licencia MIT) para leer el archivo .gltf/.bin;
// la evaluacion de la animacion y el dibujo se hacen aqui con OpenGL 3.3.
//
//  - Animacion de huesos: cada vertice guarda hasta 4 huesos (JOINTS_0) y sus pesos (WEIGHTS_0).
//    En cada cuadro se calcula la matriz global de cada hueso y se envia al vertex shader
//    como  huesos[i] = global(hueso i) * inversaBind(i).
//  - Animacion de objetos: los canales de traslacion/rotacion/escala de nodos sin piel
//    (por ejemplo el balon del gol) mueven directamente la malla.

#include <cmath>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ModeloOBJ.h"   // cache de texturas compartida (ModeloOBJ::texturaCompartida)

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

class ModeloGLTF {
public:
    static const int MAX_HUESOS = 32;   // debe coincidir con el arreglo 'huesos' del vertex shader

    ModeloGLTF() : minimo(0.0f), maximo(0.0f), centro(0.0f), extension(1.0f), cargado(false) {}

    bool cargar(const std::string& ruta) {
        liberar();
        cgltf_options opciones; std::memset(&opciones, 0, sizeof(opciones));
        cgltf_data* datos = nullptr;
        if (cgltf_parse_file(&opciones, ruta.c_str(), &datos) != cgltf_result_success) {
            std::cerr << "No se pudo leer el glTF: " << ruta << std::endl; return false;
        }
        if (cgltf_load_buffers(&opciones, datos, ruta.c_str()) != cgltf_result_success) {
            std::cerr << "No se pudieron cargar los buffers del glTF: " << ruta << std::endl;
            cgltf_free(datos); return false;
        }
        const std::string carpeta = directorio(ruta);

        // --- nodos (jerarquia de transformaciones) ---
        nodos.resize(datos->nodes_count);
        for (size_t i = 0; i < datos->nodes_count; ++i) {
            const cgltf_node& n = datos->nodes[i]; Nodo& x = nodos[i];
            x.padre = n.parent ? (int)(n.parent - datos->nodes) : -1;
            x.malla = n.mesh ? (int)(n.mesh - datos->meshes) : -1;
            x.piel = n.skin ? (int)(n.skin - datos->skins) : -1;
            x.tieneMatriz = n.has_matrix != 0;
            if (x.tieneMatriz) x.matriz = glm::make_mat4(n.matrix);
            x.t0 = n.has_translation ? glm::vec3(n.translation[0], n.translation[1], n.translation[2]) : glm::vec3(0.0f);
            x.r0 = n.has_rotation ? glm::quat(n.rotation[3], n.rotation[0], n.rotation[1], n.rotation[2]) : glm::quat(1, 0, 0, 0);
            x.s0 = n.has_scale ? glm::vec3(n.scale[0], n.scale[1], n.scale[2]) : glm::vec3(1.0f);
        }
        // Orden padre -> hijos para calcular las matrices globales en un solo recorrido.
        orden.clear();
        std::vector<int> pila;
        for (size_t i = 0; i < nodos.size(); ++i) if (nodos[i].padre < 0) pila.push_back((int)i);
        while (!pila.empty()) {
            int k = pila.back(); pila.pop_back(); orden.push_back(k);
            const cgltf_node& n = datos->nodes[k];
            for (size_t c = 0; c < n.children_count; ++c) pila.push_back((int)(n.children[c] - datos->nodes));
        }

        // --- pieles (esqueletos) ---
        pieles.resize(datos->skins_count);
        for (size_t i = 0; i < datos->skins_count; ++i) {
            const cgltf_skin& s = datos->skins[i]; Piel& p = pieles[i];
            if (s.joints_count > (size_t)MAX_HUESOS) {
                std::cerr << "El esqueleto tiene " << s.joints_count << " huesos (max " << MAX_HUESOS << "): " << ruta << std::endl;
                cgltf_free(datos); liberar(); return false;
            }
            for (size_t j = 0; j < s.joints_count; ++j) {
                p.huesos.push_back((int)(s.joints[j] - datos->nodes));
                glm::mat4 inversa(1.0f);
                if (s.inverse_bind_matrices) cgltf_accessor_read_float(s.inverse_bind_matrices, j, glm::value_ptr(inversa), 16);
                p.inversas.push_back(inversa);
            }
            p.matrices.assign(p.huesos.size(), glm::mat4(1.0f));
        }

        // --- mallas ---
        std::vector<std::vector<VerticeCPU> > copiaCPU;   // solo para calcular la caja en pose de reposo
        std::vector<int> copiaNodo;
        mallas.resize(datos->meshes_count);
        for (size_t m = 0; m < datos->meshes_count; ++m) {
            const cgltf_mesh& malla = datos->meshes[m];
            for (size_t q = 0; q < malla.primitives_count; ++q) {
                const cgltf_primitive& prim = malla.primitives[q];
                if (prim.type != cgltf_primitive_type_triangles) continue;
                const cgltf_accessor *pos = nullptr, *nor = nullptr, *uv = nullptr, *jnt = nullptr, *pes = nullptr;
                for (size_t a = 0; a < prim.attributes_count; ++a) {
                    const cgltf_attribute& at = prim.attributes[a];
                    if (at.type == cgltf_attribute_type_position) pos = at.data;
                    else if (at.type == cgltf_attribute_type_normal) nor = at.data;
                    else if (at.type == cgltf_attribute_type_texcoord && at.index == 0) uv = at.data;
                    else if (at.type == cgltf_attribute_type_joints && at.index == 0) jnt = at.data;
                    else if (at.type == cgltf_attribute_type_weights && at.index == 0) pes = at.data;
                }
                if (!pos || pos->count == 0) continue;
                std::vector<VerticeGLTF> v(pos->count);
                for (size_t k = 0; k < pos->count; ++k) {
                    VerticeGLTF& x = v[k];
                    cgltf_accessor_read_float(pos, k, glm::value_ptr(x.posicion), 3);
                    if (nor) cgltf_accessor_read_float(nor, k, glm::value_ptr(x.normal), 3); else x.normal = glm::vec3(0, 1, 0);
                    if (uv) cgltf_accessor_read_float(uv, k, glm::value_ptr(x.uv), 2); else x.uv = glm::vec2(0.0f);
                    x.uv.y = 1.0f - x.uv.y;   // glTF tiene V hacia abajo; las texturas se cargan invertidas como en los OBJ
                    if (jnt && pes) {
                        cgltf_uint j4[4] = { 0, 0, 0, 0 };
                        cgltf_accessor_read_uint(jnt, k, j4, 4);
                        for (int c = 0; c < 4; ++c) x.huesos[c] = (float)(j4[c] < (cgltf_uint)MAX_HUESOS ? j4[c] : 0);
                        cgltf_accessor_read_float(pes, k, glm::value_ptr(x.pesos), 4);
                        float suma = x.pesos.x + x.pesos.y + x.pesos.z + x.pesos.w;
                        x.pesos = suma > 0.0001f ? x.pesos / suma : glm::vec4(1, 0, 0, 0);
                    }
                    else { x.huesos = glm::vec4(0.0f); x.pesos = glm::vec4(1, 0, 0, 0); }
                }
                std::vector<GLuint> indices;
                if (prim.indices) {
                    indices.resize(prim.indices->count);
                    for (size_t k = 0; k < prim.indices->count; ++k) {
                        size_t idx = cgltf_accessor_read_index(prim.indices, k);
                        indices[k] = (GLuint)(idx < v.size() ? idx : 0);
                    }
                }
                else { indices.resize(v.size()); for (size_t k = 0; k < v.size(); ++k) indices[k] = (GLuint)k; }

                Primitiva p;
                p.indices = (GLsizei)indices.size();
                p.conPiel = (jnt && pes);
                if (prim.material) {
                    const cgltf_material& mat = *prim.material;
                    if (mat.has_pbr_metallic_roughness) {
                        const cgltf_pbr_metallic_roughness& pbr = mat.pbr_metallic_roughness;
                        p.color = glm::vec3(pbr.base_color_factor[0], pbr.base_color_factor[1], pbr.base_color_factor[2]);
                        p.metalico = pbr.metallic_factor;
                        if (pbr.base_color_texture.texture && pbr.base_color_texture.texture->image &&
                            pbr.base_color_texture.texture->image->uri) {
                            std::string uri = pbr.base_color_texture.texture->image->uri;
                            std::vector<char> buf(uri.begin(), uri.end()); buf.push_back('\0');
                            cgltf_decode_uri(&buf[0]);
                            std::string archivo = carpeta + std::string(&buf[0]);
                            p.textura = ModeloOBJ::texturaCompartida(archivo);
                            if (!p.textura) std::cerr << "Textura no disponible: " << archivo << std::endl;
                            // mapa de brillo horneado junto al albedo (mismo nombre con _spec)
                            size_t pa = archivo.rfind("_albedo");
                            if (pa != std::string::npos)
                                p.texturaSpec = ModeloOBJ::texturaCompartida(archivo.substr(0, pa) + "_spec" + archivo.substr(pa + 7));
                        }
                    }
                }
                glGenVertexArrays(1, &p.vao); glGenBuffers(1, &p.vbo); glGenBuffers(1, &p.ebo);
                glBindVertexArray(p.vao);
                glBindBuffer(GL_ARRAY_BUFFER, p.vbo);
                glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(VerticeGLTF), v.data(), GL_STATIC_DRAW);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p.ebo);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VerticeGLTF), (void*)offsetof(VerticeGLTF, posicion)); glEnableVertexAttribArray(0);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VerticeGLTF), (void*)offsetof(VerticeGLTF, normal)); glEnableVertexAttribArray(1);
                glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VerticeGLTF), (void*)offsetof(VerticeGLTF, uv)); glEnableVertexAttribArray(2);
                glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(VerticeGLTF), (void*)offsetof(VerticeGLTF, huesos)); glEnableVertexAttribArray(3);
                glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(VerticeGLTF), (void*)offsetof(VerticeGLTF, pesos)); glEnableVertexAttribArray(4);
                glBindVertexArray(0);
                mallas[m].push_back(p);

                std::vector<VerticeCPU> cpu(v.size());
                for (size_t k = 0; k < v.size(); ++k) { cpu[k].p = v[k].posicion; cpu[k].j = v[k].huesos; cpu[k].w = v[k].pesos; }
                copiaCPU.push_back(cpu); copiaNodo.push_back((int)m);
            }
        }

        // --- animaciones ---
        for (size_t a = 0; a < datos->animations_count; ++a) {
            const cgltf_animation& an = datos->animations[a];
            Animacion A; A.nombre = an.name ? an.name : "Animacion"; A.duracion = 0.0f;
            for (size_t c = 0; c < an.channels_count; ++c) {
                const cgltf_animation_channel& ch = an.channels[c];
                if (!ch.target_node || !ch.sampler) continue;
                Canal K; K.nodo = (int)(ch.target_node - datos->nodes);
                if (ch.target_path == cgltf_animation_path_type_translation) K.tipo = 0;
                else if (ch.target_path == cgltf_animation_path_type_rotation) K.tipo = 1;
                else if (ch.target_path == cgltf_animation_path_type_scale) K.tipo = 2;
                else continue;   // pesos de morph targets: no se usan
                K.interpolacion = ch.sampler->interpolation == cgltf_interpolation_type_step ? 1 :
                    (ch.sampler->interpolation == cgltf_interpolation_type_cubic_spline ? 2 : 0);
                const cgltf_accessor* entrada = ch.sampler->input; const cgltf_accessor* salida = ch.sampler->output;
                K.componentes = K.tipo == 1 ? 4 : 3;
                K.tiempos.resize(entrada->count);
                for (size_t k = 0; k < entrada->count; ++k) cgltf_accessor_read_float(entrada, k, &K.tiempos[k], 1);
                K.valores.resize(salida->count * K.componentes);
                for (size_t k = 0; k < salida->count; ++k) cgltf_accessor_read_float(salida, k, &K.valores[k * K.componentes], K.componentes);
                if (K.tiempos.empty() || K.valores.size() < K.tiempos.size() * K.componentes) continue;
                if (K.tiempos.back() > A.duracion) A.duracion = K.tiempos.back();
                A.canales.push_back(K);
            }
            animaciones.push_back(A);
        }
        cgltf_free(datos);

        // --- caja envolvente en pose de reposo (para escalar y apoyar en el suelo) ---
        pose(-1, 0.0f);
        bool primero = true;
        for (size_t i = 0; i < nodos.size(); ++i) {
            if (nodos[i].malla < 0) continue;
            for (size_t c = 0; c < copiaCPU.size(); ++c) {
                if (copiaNodo[c] != nodos[i].malla) continue;
                for (size_t k = 0; k < copiaCPU[c].size(); k += 7) {   // muestra 1 de cada 7 vertices
                    const VerticeCPU& x = copiaCPU[c][k];
                    glm::mat4 m = nodos[i].global;
                    if (nodos[i].piel >= 0) {
                        const Piel& p = pieles[nodos[i].piel];
                        m = glm::mat4(0.0f);
                        for (int h = 0; h < 4; ++h) {
                            int j = (int)x.j[h]; if (j < 0 || j >= (int)p.matrices.size()) j = 0;
                            m += p.matrices[j] * x.w[h];
                        }
                    }
                    glm::vec3 w = glm::vec3(m * glm::vec4(x.p, 1.0f));
                    if (primero) { minimo = maximo = w; primero = false; }
                    minimo = glm::min(minimo, w); maximo = glm::max(maximo, w);
                }
            }
        }
        centro = (minimo + maximo) * 0.5f;
        glm::vec3 t = maximo - minimo;
        extension = glm::max(t.x, glm::max(t.y, t.z)); if (extension < 0.0001f) extension = 1.0f;
        cargado = !mallas.empty();
        return cargado;
    }

    int numeroAnimaciones() const { return (int)animaciones.size(); }
    float duracion(int a) const { return (a >= 0 && a < (int)animaciones.size()) ? animaciones[a].duracion : 0.0f; }
    const std::string nombreAnimacion(int a) const { return (a >= 0 && a < (int)animaciones.size()) ? animaciones[a].nombre : std::string(); }

    // Coloca el esqueleto en el instante 'tiempo' (segundos) de la animacion 'a'.
    // a < 0 deja la pose de reposo.
    void pose(int a, float tiempo) {
        for (size_t i = 0; i < nodos.size(); ++i) { nodos[i].t = nodos[i].t0; nodos[i].r = nodos[i].r0; nodos[i].s = nodos[i].s0; }
        if (a >= 0 && a < (int)animaciones.size()) {
            const Animacion& A = animaciones[a];
            for (size_t c = 0; c < A.canales.size(); ++c) aplicarCanal(A.canales[c], tiempo);
        }
        for (size_t o = 0; o < orden.size(); ++o) {
            Nodo& n = nodos[orden[o]];
            glm::mat4 local = n.tieneMatriz ? n.matriz :
                glm::translate(glm::mat4(1.0f), n.t) * glm::mat4_cast(n.r) * glm::scale(glm::mat4(1.0f), n.s);
            n.global = n.padre >= 0 ? nodos[n.padre].global * local : local;
        }
        for (size_t s = 0; s < pieles.size(); ++s)
            for (size_t j = 0; j < pieles[s].huesos.size(); ++j)
                pieles[s].matrices[j] = nodos[pieles[s].huesos[j]].global * pieles[s].inversas[j];
    }

    // Matriz que escala el modelo a 'tamano' (lado mayor de su caja), lo apoya en el suelo
    // en 'posicion' y lo gira 'rotacionY' grados, igual que ModeloOBJ::dibujar.
    glm::mat4 matrizInstancia(const glm::vec3& posicion, float tamano, float rotacionY) const {
        float escala = tamano / extension;
        glm::vec3 p = posicion; p.y += (centro.y - minimo.y) * escala;
        glm::mat4 m = glm::translate(glm::mat4(1.0f), p);
        m = glm::rotate(m, glm::radians(rotacionY), glm::vec3(0, 1, 0));
        m = glm::scale(m, glm::vec3(escala));
        return glm::translate(m, -centro);
    }

    void dibujar(GLuint programa, const glm::mat4& vista, const glm::mat4& proyeccion, const glm::mat4& modelo, bool iluminacion) const {
        if (!cargado) return;
        glUniformMatrix4fv(glGetUniformLocation(programa, "view"), 1, GL_FALSE, glm::value_ptr(vista));
        glUniformMatrix4fv(glGetUniformLocation(programa, "projection"), 1, GL_FALSE, glm::value_ptr(proyeccion));
        glUniform1i(glGetUniformLocation(programa, "lamps"), iluminacion ? 1 : 0);
        glUniform1i(glGetUniformLocation(programa, "teamColorable"), 0);
        glUniform1f(glGetUniformLocation(programa, "materialAlpha"), 1.0f);
        const GLint locModelo = glGetUniformLocation(programa, "model");
        const GLint locPiel = glGetUniformLocation(programa, "skinned");
        const GLint locHuesos = glGetUniformLocation(programa, "huesos[0]");
        for (size_t i = 0; i < nodos.size(); ++i) {
            const Nodo& n = nodos[i];
            if (n.malla < 0) continue;
            const bool conPiel = n.piel >= 0 && !pieles[n.piel].matrices.empty();
            // Con piel, las matrices de los huesos ya contienen la transformacion del nodo.
            glm::mat4 m = conPiel ? modelo : modelo * n.global;
            glUniformMatrix4fv(locModelo, 1, GL_FALSE, glm::value_ptr(m));
            if (conPiel) glUniformMatrix4fv(locHuesos, (GLsizei)pieles[n.piel].matrices.size(), GL_FALSE, glm::value_ptr(pieles[n.piel].matrices[0]));
            for (size_t q = 0; q < mallas[n.malla].size(); ++q) {
                const Primitiva& p = mallas[n.malla][q];
                glUniform1i(locPiel, (conPiel && p.conPiel) ? 1 : 0);
                glUniform3fv(glGetUniformLocation(programa, "materialColor"), 1, glm::value_ptr(p.color));
                glUniform1i(glGetUniformLocation(programa, "hasTexture"), p.textura ? 1 : 0);
                glUniform1i(glGetUniformLocation(programa, "hasSpecMap"), p.texturaSpec ? 1 : 0);
                glUniform1f(glGetUniformLocation(programa, "metallic"), p.metalico);
                glUniform1f(glGetUniformLocation(programa, "shininess"), 48.0f);
                glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, p.texturaSpec);
                glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, p.textura);
                glBindVertexArray(p.vao);
                glDrawElements(GL_TRIANGLES, p.indices, GL_UNSIGNED_INT, (void*)0);
            }
        }
        glUniform1i(locPiel, 0);
        glUniform1i(glGetUniformLocation(programa, "hasSpecMap"), 0);
        glUniform1f(glGetUniformLocation(programa, "metallic"), 0.0f);
        glBindVertexArray(0);
    }

    void liberar() {
        for (size_t m = 0; m < mallas.size(); ++m)
            for (size_t q = 0; q < mallas[m].size(); ++q) {
                Primitiva& p = mallas[m][q];
                if (p.vbo) glDeleteBuffers(1, &p.vbo);
                if (p.ebo) glDeleteBuffers(1, &p.ebo);
                if (p.vao) glDeleteVertexArrays(1, &p.vao);
            }
        mallas.clear(); nodos.clear(); orden.clear(); pieles.clear(); animaciones.clear(); cargado = false;
    }

    bool listo() const { return cargado; }
    glm::vec3 minimo, maximo;

private:
    struct VerticeGLTF { glm::vec3 posicion; glm::vec3 normal; glm::vec2 uv; glm::vec4 huesos; glm::vec4 pesos; };
    struct VerticeCPU { glm::vec3 p; glm::vec4 j; glm::vec4 w; };
    struct Nodo {
        int padre = -1, malla = -1, piel = -1;
        bool tieneMatriz = false; glm::mat4 matriz = glm::mat4(1.0f);
        glm::vec3 t0 = glm::vec3(0.0f), s0 = glm::vec3(1.0f); glm::quat r0 = glm::quat(1, 0, 0, 0);
        glm::vec3 t = glm::vec3(0.0f), s = glm::vec3(1.0f); glm::quat r = glm::quat(1, 0, 0, 0);
        glm::mat4 global = glm::mat4(1.0f);
    };
    struct Primitiva {
        GLuint vao = 0, vbo = 0, ebo = 0, textura = 0, texturaSpec = 0;
        GLsizei indices = 0; glm::vec3 color = glm::vec3(1.0f); float metalico = 0.0f; bool conPiel = false;
    };
    struct Piel { std::vector<int> huesos; std::vector<glm::mat4> inversas; std::vector<glm::mat4> matrices; };
    struct Canal { int nodo = 0, tipo = 0, interpolacion = 0, componentes = 3; std::vector<float> tiempos; std::vector<float> valores; };
    struct Animacion { std::string nombre; float duracion = 0.0f; std::vector<Canal> canales; };

    std::vector<Nodo> nodos;
    std::vector<int> orden;
    std::vector<std::vector<Primitiva> > mallas;
    std::vector<Piel> pieles;
    std::vector<Animacion> animaciones;
    glm::vec3 centro;
    float extension;
    bool cargado;

    // Valor del fotograma k del canal (en splines cubicas el valor esta en medio de las tangentes).
    static const float* valor(const Canal& c, size_t k) {
        size_t indice = c.interpolacion == 2 ? k * 3 + 1 : k;
        size_t maximoIndice = c.valores.size() / c.componentes - 1;
        if (indice > maximoIndice) indice = maximoIndice;
        return &c.valores[indice * c.componentes];
    }

    void aplicarCanal(const Canal& c, float t) {
        if (c.nodo < 0 || c.nodo >= (int)nodos.size()) return;
        const std::vector<float>& T = c.tiempos;
        size_t k1 = 0;
        while (k1 < T.size() && T[k1] < t) ++k1;
        size_t k0 = k1 == 0 ? 0 : k1 - 1;
        if (k1 >= T.size()) k1 = T.size() - 1;
        float u = 0.0f;
        if (k1 != k0 && c.interpolacion != 1) u = glm::clamp((t - T[k0]) / (T[k1] - T[k0]), 0.0f, 1.0f);
        const float* a = valor(c, k0); const float* b = valor(c, k1);
        Nodo& n = nodos[c.nodo];
        if (c.tipo == 1) {
            glm::quat qa(a[3], a[0], a[1], a[2]), qb(b[3], b[0], b[1], b[2]);
            n.r = glm::normalize(glm::slerp(qa, qb, u));
        }
        else {
            glm::vec3 va(a[0], a[1], a[2]), vb(b[0], b[1], b[2]);
            if (c.tipo == 0) n.t = glm::mix(va, vb, u); else n.s = glm::mix(va, vb, u);
        }
    }

    static std::string directorio(const std::string& ruta) {
        size_t p = ruta.find_last_of("/\\");
        return p == std::string::npos ? std::string() : ruta.substr(0, p + 1);
    }
};
