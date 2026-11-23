#pragma once

// Auto.h
// Prepartido del estadio: llegada del autobus visitante, salida y entrada de los
// jugadores, y paseo del carrito medico por la cancha. Cuando termina, el
// programa pasa al estado PARTIDO (el que ya existia en Estadio.cpp).
//
// Clases:
//   Recorrido          : camino 2D (X,Z) por arcos de longitud, con rumbo y curvatura.
//   PerfilVelocidad    : arranque, crucero y frenado suaves (trapecio de velocidad).
//   VehiculoOBJ        : carroceria + ruedas independientes (giran y se orientan).
//   VehiculoRuta       : un VehiculoOBJ que sigue un Recorrido.
//   Autobus            : modelo Realista/autobus (el camion del equipo visitante).
//   CarritoMedico      : modelo Realista/carrito_emergencias.
//   EntradaEquipo      : maquina de estados PREPARTIDO -> PARTIDO que une todo.
//
// Convenciones: el rumbo es el mismo que usa Estadio.cpp, frente = (sin r, 0, cos r).

#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "ModeloOBJ.h"

namespace auto_cfg {
    const float PI = 3.14159265359f;

    // Puerta del estadio por la que entra el visitante (lado de su banca, Z positivo).
    const float PUERTA_X = -22.2f;
    const float PUERTA_Z = 62.0f;
    // Donde se estaciona el autobus (paralelo a la fachada, frente a la puerta).
    const float ESTACIONAMIENTO_Z = 74.0f;
    // Carril exterior por el que llega el autobus antes de girar a la derecha.
    const float CARRIL_X = 95.0f;
    const float CARRIL_INICIO_Z = -75.0f;
    const float RADIO_GIRO_BUS = 10.0f;
    // Los jugadores se ocultan al cruzar la puerta y reaparecen en la boca de
    // la cancha, porque el modelo del estadio no tiene tunel interior.
    const float Z_ANTES_PUERTA = 63.8f;
    const float Z_BOCA_CANCHA = 33.2f;
    const float SEGUNDOS_OCULTOS = 1.3f;
    const float VELOCIDAD_JUGADOR = 4.8f;
    const float RETARDO_ENTRE_JUGADORES = 0.6f;
    const float PAUSA_PUERTA_BUS = 1.0f;

    // Formacion final del visitante. Debe coincidir con equipoBlanco de Estadio.cpp.
    const glm::vec2 FORMACION_VISITANTE[11] = {
        glm::vec2(-30, 0), glm::vec2(-18, -18), glm::vec2(-18, -6), glm::vec2(-18, 6), glm::vec2(-18, 18),
        glm::vec2(-5, -13), glm::vec2(-5, 0), glm::vec2(-5, 13), glm::vec2(8, -13), glm::vec2(8, 0), glm::vec2(8, 13)
    };
}

// Camino en el plano XZ muestreado en una polilinea con longitud acumulada.
class Recorrido {
public:
    void limpiar() { puntos.clear(); acumulado.clear(); }

    void agregarPunto(const glm::vec2& p) {
        if (!puntos.empty() && glm::distance(puntos.back(), p) < 1.0e-4f) return;
        puntos.push_back(p);
        if (puntos.size() == 1) acumulado.push_back(0.0f);
        else acumulado.push_back(acumulado.back() + glm::distance(puntos[puntos.size() - 2], p));
    }

    // Arco de circunferencia: punto = centro + radio * (cos a, sin a), con a en radianes.
    void agregarArco(const glm::vec2& centro, float radio, float a0, float a1, int pasos = 32) {
        for (int i = 0; i <= pasos; ++i) {
            float a = glm::mix(a0, a1, (float)i / pasos);
            agregarPunto(centro + radio * glm::vec2(std::cos(a), std::sin(a)));
        }
    }

    // Curva de Hermite: sale de p0 con tangente t0 y llega a p1 con tangente t1.
    void agregarHermite(const glm::vec2& p0, const glm::vec2& t0, const glm::vec2& p1, const glm::vec2& t1, int pasos = 40) {
        for (int i = 0; i <= pasos; ++i) {
            float u = (float)i / pasos, u2 = u * u, u3 = u2 * u;
            agregarPunto((2 * u3 - 3 * u2 + 1) * p0 + (u3 - 2 * u2 + u) * t0 + (-2 * u3 + 3 * u2) * p1 + (u3 - u2) * t1);
        }
    }

    float longitud() const { return acumulado.empty() ? 0.0f : acumulado.back(); }

    glm::vec2 posicion(float s) const {
        if (puntos.empty()) return glm::vec2(0.0f);
        s = glm::clamp(s, 0.0f, longitud());
        size_t i = std::upper_bound(acumulado.begin(), acumulado.end(), s) - acumulado.begin();
        if (i == 0) return puntos.front();
        if (i >= puntos.size()) return puntos.back();
        float tramo = acumulado[i] - acumulado[i - 1];
        float u = tramo > 1.0e-6f ? (s - acumulado[i - 1]) / tramo : 0.0f;
        return glm::mix(puntos[i - 1], puntos[i], u);
    }

    // Rumbo en radianes en el punto s (0 = +Z, positivo hacia +X).
    float rumbo(float s) const {
        float a = glm::clamp(s - 0.5f, 0.0f, longitud()), b = glm::clamp(s + 0.5f, 0.0f, longitud());
        if (b - a < 1.0e-4f) { a = std::max(0.0f, longitud() - 1.0f); b = longitud(); }
        glm::vec2 d = posicion(b) - posicion(a);
        return std::atan2(d.x, d.y);
    }

    // Cambio de rumbo por unidad de longitud (positivo = giro hacia +X desde +Z).
    float curvatura(float s) const {
        float d = rumbo(s + 1.0f) - rumbo(s - 1.0f);
        while (d > auto_cfg::PI) d -= 2.0f * auto_cfg::PI;
        while (d < -auto_cfg::PI) d += 2.0f * auto_cfg::PI;
        return d / 2.0f;
    }

private:
    std::vector<glm::vec2> puntos;
    std::vector<float> acumulado;
};

// Velocidad trapezoidal: acelera, mantiene crucero y frena para detenerse justo al final.
struct PerfilVelocidad {
    float largo = 0.0f, vmax = 1.0f, aceleracion = 1.0f;
    float tAcel = 0.0f, dAcel = 0.0f, tCrucero = 0.0f, total = 0.0f;

    void configurar(float longitud, float velocidadMaxima, float acel) {
        largo = longitud; vmax = velocidadMaxima; aceleracion = acel;
        tAcel = vmax / aceleracion; dAcel = 0.5f * aceleracion * tAcel * tAcel;
        if (2.0f * dAcel > largo) { // recorrido corto: no alcanza la velocidad maxima
            tAcel = std::sqrt(largo / aceleracion); vmax = aceleracion * tAcel; dAcel = 0.5f * largo; tCrucero = 0.0f;
        }
        else tCrucero = (largo - 2.0f * dAcel) / vmax;
        total = 2.0f * tAcel + tCrucero;
    }

    float distancia(float t) const {
        if (t <= 0.0f) return 0.0f;
        if (t >= total) return largo;
        if (t < tAcel) return 0.5f * aceleracion * t * t;
        if (t < tAcel + tCrucero) return dAcel + vmax * (t - tAcel);
        float resta = total - t;
        return largo - 0.5f * aceleracion * resta * resta;
    }
};

struct DefinicionRueda {
    std::string filtro;     // prefijo del grupo OBJ de la rueda
    glm::vec3 pivote;       // centro de la rueda en coordenadas del modelo
    float radio;            // radio en unidades del modelo
    bool delantera;         // gira con la direccion
};

// Carroceria y ruedas cargadas por separado desde un mismo OBJ (por nombre de grupo).
class VehiculoOBJ {
public:
    float escala = 1.0f;
    glm::vec3 pivoteCuerpo = glm::vec3(0.0f);   // punto del modelo que queda sobre la posicion
    float desfaseRumbo = 0.0f;                  // giro para que el frente del modelo apunte a +Z
    glm::vec3 ejeRodar = glm::vec3(1, 0, 0);
    float signoRodar = 1.0f;
    float alturaBase = 0.0f;                    // sube o baja el modelo para apoyar las ruedas

    bool cargar(const std::string& ruta, const std::string& filtroCarroceria, const std::vector<DefinicionRueda>& defs) {
        bool ok = carroceria.cargar(ruta, filtroCarroceria);
        ruedas.clear();
        for (const DefinicionRueda& d : defs) {
            RuedaOBJ r; r.def = d;
            if (!r.modelo.cargar(ruta, d.filtro)) { std::cerr << "No se pudo cargar la rueda " << d.filtro << " de " << ruta << std::endl; ok = false; }
            ruedas.push_back(r);
        }
        return ok;
    }

    // distancia: metros recorridos (hace rodar las ruedas); giro: direccion de las delanteras (rad).
    void dibujar(GLuint programa, const glm::mat4& vista, const glm::mat4& proyeccion, const glm::vec3& posicion,
        float rumbo, float distancia, float giro, bool iluminacion) const {
        glm::mat4 m(1.0f);
        m = glm::translate(m, posicion + glm::vec3(0.0f, alturaBase, 0.0f));
        m = glm::rotate(m, rumbo + desfaseRumbo, glm::vec3(0, 1, 0));
        m = glm::scale(m, glm::vec3(escala));
        m = glm::translate(m, -pivoteCuerpo);
        carroceria.dibujarConMatriz(programa, vista, proyeccion, m, iluminacion);
        for (const RuedaOBJ& r : ruedas) {
            glm::mat4 w = glm::translate(m, r.def.pivote);
            if (r.def.delantera) w = glm::rotate(w, giro, glm::vec3(0, 1, 0));
            w = glm::rotate(w, signoRodar * distancia / (escala * r.def.radio), ejeRodar);
            w = glm::translate(w, -r.def.pivote);
            r.modelo.dibujarConMatriz(programa, vista, proyeccion, w, iluminacion);
        }
    }

    // Convierte un punto del modelo a coordenadas del mundo.
    glm::vec3 aMundo(const glm::vec3& posicion, float rumbo, const glm::vec3& local) const {
        glm::mat4 m(1.0f);
        m = glm::translate(m, posicion + glm::vec3(0.0f, alturaBase, 0.0f));
        m = glm::rotate(m, rumbo + desfaseRumbo, glm::vec3(0, 1, 0));
        m = glm::scale(m, glm::vec3(escala));
        m = glm::translate(m, -pivoteCuerpo);
        return glm::vec3(m * glm::vec4(local, 1.0f));
    }

private:
    struct RuedaOBJ { ModeloOBJ modelo; DefinicionRueda def; };
    ModeloOBJ carroceria;
    std::vector<RuedaOBJ> ruedas;
};

// Vehiculo que sigue un Recorrido con perfil de velocidad, o se coloca a mano.
class VehiculoRuta {
public:
    VehiculoOBJ modelo;
    Recorrido ruta;
    float distanciaEntreEjes = 3.0f;   // para calcular cuanto giran las ruedas delanteras
    float giroMaximo = glm::radians(28.0f);

    glm::vec3 posicion = glm::vec3(0.0f);
    float rumbo = 0.0f, rodada = 0.0f, giro = 0.0f;

    void iniciar(float velocidadMax, float aceleracion, float retardoInicial) {
        perfil.configurar(ruta.longitud(), velocidadMax, aceleracion);
        retardo = retardoInicial; t = 0.0f; activo = true; terminado = false;
        aplicarPose(0.0f);
    }

    void actualizar(float dt) {
        if (!activo) return;
        if (retardo > 0.0f) { retardo -= dt; return; }
        t += dt;
        if (t >= perfil.total) { t = perfil.total; activo = false; terminado = true; }
        aplicarPose(perfil.distancia(t));
    }

    void terminarYa() { retardo = 0.0f; t = perfil.total; activo = false; terminado = true; aplicarPose(perfil.largo); }

    bool llego() const { return terminado; }

    // Control manual (flechas): ruedas segun el desplazamiento real.
    void colocarManual(const glm::vec3& nuevaPosicion, float nuevoRumbo) {
        glm::vec3 delta = nuevaPosicion - posicion;
        float avance = glm::length(delta);
        if (avance > 1.0e-5f) {
            glm::vec3 frente(std::sin(rumbo), 0.0f, std::cos(rumbo));
            rodada += (glm::dot(delta, frente) >= 0.0f ? 1.0f : -1.0f) * avance;
            float dr = nuevoRumbo - rumbo;
            while (dr > auto_cfg::PI) dr -= 2.0f * auto_cfg::PI;
            while (dr < -auto_cfg::PI) dr += 2.0f * auto_cfg::PI;
            giro = glm::clamp(std::atan(distanciaEntreEjes * dr / avance), -giroMaximo, giroMaximo);
        }
        else giro *= 0.9f;
        posicion = nuevaPosicion; rumbo = nuevoRumbo;
    }

    void dibujar(GLuint programa, const glm::mat4& vista, const glm::mat4& proyeccion, bool iluminacion) const {
        modelo.dibujar(programa, vista, proyeccion, posicion, rumbo, rodada, giro, iluminacion);
    }

protected:
    float yBase = 0.0f;   // altura del suelo para la posicion

private:
    PerfilVelocidad perfil;
    float t = 0.0f, retardo = 0.0f;
    bool activo = false, terminado = false;

    void aplicarPose(float s) {
        glm::vec2 p = ruta.posicion(s);
        posicion = glm::vec3(p.x, yBase, p.y);
        rumbo = ruta.rumbo(s);
        rodada = s;
        giro = glm::clamp(std::atan(distanciaEntreEjes * ruta.curvatura(s)), -giroMaximo, giroMaximo);
    }
};

// Camion del equipo visitante (Models/Realista/autobus).
class Autobus : public VehiculoRuta {
public:
    // Punto de bajada en coordenadas del modelo: lado derecho (X local negativo, el
    // frente es +Z), delante del eje delantero. El OBJ no trae una puerta modelada.
    glm::vec3 puertaLocal = glm::vec3(-0.45f, 0.0f, 6.05f);

    bool cargar(const std::string& carpeta) {
        const float radio = 0.345834f, y = 1.342273f;
        std::vector<DefinicionRueda> ruedas = {
            { "llanta_1",    glm::vec3(-0.078053f, y, 5.364608f), radio, true },
            { "llanta_2",    glm::vec3(0.811947f,  y, 5.364608f), radio, true },
            { "llanta_3",    glm::vec3(-0.078053f, y, 1.664608f), radio, false },
            { "llanta_4obj", glm::vec3(0.811947f,  y, 1.664608f), radio, false },
            { "llanta_5",    glm::vec3(-0.078053f, y, 0.964608f), radio, false },
            { "llanta_6",    glm::vec3(0.811947f,  y, 0.964608f), radio, false }
        };
        modelo.escala = 1.85f;
        modelo.pivoteCuerpo = glm::vec3(0.36695f, 0.0f, 3.16461f);
        modelo.desfaseRumbo = 0.0f;
        modelo.ejeRodar = glm::vec3(1, 0, 0);
        modelo.signoRodar = 1.0f;
        modelo.alturaBase = -modelo.escala * (y - radio);   // la parte baja de las llantas toca el suelo
        distanciaEntreEjes = (5.364608f - 1.664608f) * modelo.escala;
        return modelo.cargar(carpeta + "autobus/autobus.obj", "!llanta_", ruedas);
    }

    glm::vec3 puertaMundo() const { return modelo.aMundo(posicion, rumbo, puertaLocal); }
    // Distancia del centro del autobus a su puerta a lo largo del frente (con el escalado).
    float puertaAdelante() const { return (puertaLocal.z - modelo.pivoteCuerpo.z) * modelo.escala; }
    float puertaLateral() const { return (puertaLocal.x - modelo.pivoteCuerpo.x) * modelo.escala; }
};

// Carrito de emergencias medicas (Models/Realista/carrito_emergencias).
class CarritoMedico : public VehiculoRuta {
public:
    bool cargar(const std::string& carpeta) {
        const float radio = 0.22f, y = 0.22f;
        std::vector<DefinicionRueda> ruedas = {
            { "rueda_tras_der", glm::vec3(-0.78f, y,  0.45f), radio, false },
            { "rueda_tras_izq", glm::vec3(-0.78f, y, -0.45f), radio, false },
            { "rueda_del_der",  glm::vec3(0.78f,  y,  0.45f), radio, true },
            { "rueda_del_izq",  glm::vec3(0.78f,  y, -0.45f), radio, true }
        };
        modelo.escala = 1.6f;
        modelo.pivoteCuerpo = glm::vec3(0.0f);
        modelo.desfaseRumbo = -auto_cfg::PI * 0.5f;   // el frente del modelo es +X
        modelo.ejeRodar = glm::vec3(0, 0, 1);
        modelo.signoRodar = -1.0f;
        modelo.alturaBase = 0.0f;
        distanciaEntreEjes = 1.56f * modelo.escala;
        return modelo.cargar(carpeta + "carrito_emergencias/carrito_emergencias_completo.obj", "!rueda_", ruedas);
    }
};

// Funcion que dibuja un jugador visitante: posicion, rumbo en grados, si camina y la fase del paso (rad).
typedef std::function<void(const glm::vec3&, float, bool, float)> DibujaVisitante;

class EntradaEquipo {
public:
    enum Fase { PREPARTIDO, PARTIDO };

    // Carga los modelos. Si falta alguno, el programa arranca directo en PARTIDO.
    bool cargar(const std::string& carpetaRealista = "Models/Realista/") {
        auto inicio = std::chrono::steady_clock::now();
        bool okBus = autobus.cargar(carpetaRealista);
        bool okCarrito = carrito.cargar(carpetaRealista);
        double segundos = std::chrono::duration<double>(std::chrono::steady_clock::now() - inicio).count();
        std::cout << "Autobus y carrito medico cargados en " << segundos << " s" << std::endl;
        cargado = okBus && okCarrito;
        if (!cargado) {
            std::cerr << "Faltan modelos en " << carpetaRealista << "; se omite el prepartido." << std::endl;
            fase = PARTIDO;
            return false;
        }
        reiniciar();
        return true;
    }

    // Vuelve al inicio del prepartido.
    void reiniciar() {
        if (!cargado) return;
        using namespace auto_cfg;
        fase = PREPARTIDO; tiempo = 0.0f; tiempoDescenso = 0.0f; saltarSolicitado = false;

        // Autobus: carril exterior recto, giro a la derecha y frenado frente a la puerta.
        float xParada = PUERTA_X + autobus.puertaAdelante();
        autobus.ruta.limpiar();
        float zGiro = ESTACIONAMIENTO_Z - RADIO_GIRO_BUS;
        autobus.ruta.agregarPunto(glm::vec2(CARRIL_X, CARRIL_INICIO_Z));
        autobus.ruta.agregarPunto(glm::vec2(CARRIL_X, zGiro));
        // Centro del arco a la derecha del autobus; a pasa de 0 a 90 grados (rumbo +Z -> -X).
        autobus.ruta.agregarArco(glm::vec2(CARRIL_X - RADIO_GIRO_BUS, zGiro), RADIO_GIRO_BUS, 0.0f, PI * 0.5f);
        autobus.ruta.agregarPunto(glm::vec2(xParada, ESTACIONAMIENTO_Z));
        autobus.iniciar(16.0f, 3.5f, 0.5f);

        // Carrito medico: sale de su lugar de reposo, da una vuelta a la cancha y regresa.
        // Recorre una superelipse (|x/a|^4 + |z/b|^4 = 1) entre las lineas y las bancas,
        // en sentido horario: pasa primero por el lado del visitante (x<0, z>0) y lo
        // deja libre antes de que los jugadores entren a la cancha.
        const glm::vec2 reposo(-30.0f, -27.0f);
        const glm::vec2 frenteReposo(-1.0f, 0.0f);
        const float a = 42.0f, b = 21.0f;
        auto raizSigno = [](float c) { return (c < 0.0f ? -1.0f : 1.0f) * std::sqrt(std::fabs(c)); };
        auto elipse = [&](float th) { return glm::vec2(a * raizSigno(std::cos(th)), b * raizSigno(std::sin(th))); };
        auto tangente = [&](float th) { return glm::normalize(elipse(th - 0.01f) - elipse(th + 0.01f)); };
        float th1 = glm::radians(235.0f), th2 = glm::radians(-110.0f);
        glm::vec2 e1 = elipse(th1), e2 = elipse(th2);
        carrito.ruta.limpiar();
        carrito.ruta.agregarPunto(reposo);
        carrito.ruta.agregarHermite(reposo, frenteReposo * glm::distance(reposo, e1) * 1.2f, e1, tangente(th1) * glm::distance(reposo, e1) * 1.2f);
        for (int i = 1; i <= 160; ++i) carrito.ruta.agregarPunto(elipse(glm::mix(th1, th2, i / 160.0f)));
        carrito.ruta.agregarHermite(e2, tangente(th2) * glm::distance(reposo, e2) * 1.2f, reposo, frenteReposo * glm::distance(reposo, e2) * 1.2f);
        carrito.iniciar(7.5f, 2.5f, 1.0f);

        // Calendario de cada jugador visitante (instante de salida y duracion de cada tramo).
        for (int i = 0; i < 11; ++i) {
            Visitante& v = visitantes[i];
            float lateral = ((i % 3) - 1) * 1.2f;
            v.antesPuerta = glm::vec2(PUERTA_X + lateral, Z_ANTES_PUERTA);
            v.bocaCancha = glm::vec2(PUERTA_X + lateral * 2.0f, Z_BOCA_CANCHA);
            v.meta = FORMACION_VISITANTE[i];
            v.salida = PAUSA_PUERTA_BUS + i * RETARDO_ENTRE_JUGADORES;
        }
        focoSuave = glm::vec3(CARRIL_X, 0.0f, CARRIL_INICIO_Z);
    }

    // Termina de inmediato: autobus estacionado, carrito en reposo y visitantes en formacion.
    void saltar() { saltarSolicitado = true; }

    // Llamar una vez por cuadro, despues de mover el autobus con el teclado.
    // En PREPARTIDO escribe la pose animada del autobus; en PARTIDO la lee para dibujarlo.
    void actualizar(float dt, glm::vec3& posicionAutobus, float& rumboAutobusGrados) {
        if (!cargado) return;
        if (fase == PREPARTIDO) {
            if (saltarSolicitado) {
                autobus.terminarYa(); carrito.terminarYa();
                tiempoDescenso = duracionVisitantes() + 10.0f;
            }
            tiempo += dt;
            autobus.actualizar(dt);
            carrito.actualizar(dt);
            if (autobus.llego()) tiempoDescenso += dt;
            posicionAutobus = autobus.posicion;
            rumboAutobusGrados = glm::degrees(autobus.rumbo);
            if (autobus.llego() && carrito.llego() && tiempoDescenso >= duracionVisitantes() + 0.8f) {
                fase = PARTIDO;
                std::cout << "Prepartido terminado: inicia el partido." << std::endl;
            }
            actualizarFoco(dt, saltarSolicitado);
        }
        else {
            autobus.colocarManual(posicionAutobus, glm::radians(rumboAutobusGrados));
        }
    }

    bool enPrepartido() const { return cargado && fase == PREPARTIDO; }
    Fase estado() const { return fase; }

    // Punto que debe mirar la camara durante el prepartido.
    glm::vec3 foco() const { return focoSuave; }

    // Autobus y carrito. Llamar con el programa de modelos activo.
    void dibujarVehiculos(GLuint programa, const glm::mat4& vista, const glm::mat4& proyeccion, bool iluminacion) const {
        if (!cargado) return;
        autobus.dibujar(programa, vista, proyeccion, iluminacion);
        carrito.dibujar(programa, vista, proyeccion, iluminacion);
    }

    // Jugadores visitantes saliendo del autobus. Solo dibuja durante el PREPARTIDO.
    void dibujarVisitantes(const DibujaVisitante& dibuja) const {
        if (!enPrepartido() || !autobus.llego()) return;
        using namespace auto_cfg;
        for (int i = 0; i < 11; ++i) {
            const Visitante& v = visitantes[i];
            float tau = tiempoDescenso - v.salida;
            if (tau < 0.0f) continue;
            glm::vec2 puerta = puertaBus();
            float t1 = glm::distance(puerta, v.antesPuerta) / VELOCIDAD_JUGADOR;
            float t3 = glm::distance(v.bocaCancha, v.meta) / VELOCIDAD_JUGADOR;
            float pasoFase = 2.0f * PI * 1.35f * tau + i * 0.9f;
            if (tau < t1) {
                glm::vec2 p = glm::mix(puerta, v.antesPuerta, tau / t1);
                dibuja(glm::vec3(p.x, 0.0f, p.y), rumboGrados(v.antesPuerta - puerta), true, pasoFase);
            }
            else if (tau < t1 + SEGUNDOS_OCULTOS) continue;   // dentro del estadio
            else if (tau < t1 + SEGUNDOS_OCULTOS + t3) {
                float u = (tau - t1 - SEGUNDOS_OCULTOS) / t3;
                glm::vec2 p = glm::mix(v.bocaCancha, v.meta, u);
                dibuja(glm::vec3(p.x, 0.0f, p.y), rumboGrados(v.meta - v.bocaCancha), true, pasoFase);
            }
            else dibuja(glm::vec3(v.meta.x, 0.0f, v.meta.y), 90.0f, false, 0.0f);
        }
    }

    const Autobus& verAutobus() const { return autobus; }
    const CarritoMedico& verCarrito() const { return carrito; }

private:
    struct Visitante {
        float salida = 0.0f;
        glm::vec2 antesPuerta, bocaCancha, meta;
    };

    Autobus autobus;
    CarritoMedico carrito;
    Visitante visitantes[11];
    Fase fase = PARTIDO;
    bool cargado = false, saltarSolicitado = false;
    float tiempo = 0.0f, tiempoDescenso = 0.0f;
    glm::vec3 focoSuave = glm::vec3(0.0f);

    static float rumboGrados(const glm::vec2& d) { return glm::degrees(std::atan2(d.x, d.y)); }

    // Punto de salida de los jugadores: la puerta del autobus pegada al piso.
    glm::vec2 puertaBus() const {
        glm::vec3 p = autobus.puertaMundo();
        return glm::vec2(p.x, p.z);
    }

    // Segundos, desde que el autobus se detiene, hasta que el ultimo jugador esta en su lugar.
    float duracionVisitantes() const {
        using namespace auto_cfg;
        float fin = 0.0f;
        glm::vec2 puerta = puertaBus();
        for (int i = 0; i < 11; ++i) {
            const Visitante& v = visitantes[i];
            float duracion = v.salida + glm::distance(puerta, v.antesPuerta) / VELOCIDAD_JUGADOR + SEGUNDOS_OCULTOS
                + glm::distance(v.bocaCancha, v.meta) / VELOCIDAD_JUGADOR;
            fin = std::max(fin, duracion);
        }
        return fin;
    }

    void actualizarFoco(float dt, bool inmediato) {
        glm::vec3 objetivo;
        if (!autobus.llego()) objetivo = autobus.posicion;
        else {
            // Primero la puerta del autobus; despues la camara se desplaza a la cancha.
            float u = glm::clamp((tiempoDescenso - 3.0f) / 4.0f, 0.0f, 1.0f);
            u = u * u * (3.0f - 2.0f * u);
            glm::vec3 puerta = autobus.puertaMundo();
            objetivo = glm::mix(glm::vec3(puerta.x, 0.0f, puerta.z - 6.0f), glm::vec3(-12.0f, 0.0f, 12.0f), u);
        }
        if (inmediato) focoSuave = objetivo;
        else focoSuave += (objetivo - focoSuave) * (1.0f - std::exp(-3.0f * dt));
    }
};
