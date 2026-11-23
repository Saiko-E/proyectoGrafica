#include <iostream>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>
#include <memory>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <filesystem>
#include <windows.h>
#endif

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "SOIL2/SOIL2.h"

struct BallMesh;
// Procesa las teclas presionadas.
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
// Controla el movimiento de la camara con el mouse.
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
// Actualiza el movimiento de la camara.
void DoMovement();
// Actualiza la animacion del tiro penal.
void Animation();
// Actualiza el recorrido de los autobuses.
void updateBusAnimation();
// Prepara una nueva escena del partido.
void beginMatchScene(int scene, bool automatic);
// Solicita cambiar a otra escena.
void requestMatchScene(int scene);
// Reinicia los datos del partido.
void resetMatch();
// Actualiza el tiempo, marcador y jugadas del partido.
void updateMatch(float step);
// Actualiza la transicion entre escenas.
void updateFade(float step);
// Dibuja el marcador y los resultados de penales.
void drawMatchScoreboard(GLint modelLoc, GLuint VAO, const glm::mat4& scene,
    const BallMesh& whiteMesh, const BallMesh& blackMesh,
    GLuint dark, GLuint white, GLuint gold, GLuint red, GLuint green, GLuint pending);
// Dibuja un numero en el marcador.
void drawScoreDigit(GLint modelLoc, GLuint VAO, const glm::mat4& matrix,
    int digit, float x, float y, float z, float width, GLuint texture);
// Crea el efecto visual para cambiar de escena.
GLuint createFadeProgram();
// Dibuja la capa de transicion en pantalla.
void drawFadeOverlay(GLuint program, GLuint vao);
// Lee las posiciones y tamanos de las ruedas.
bool loadBusWheelData(const char* filePath);

// Crea una textura de un solo color.
GLuint CreateSolidColorTexture(GLubyte r, GLubyte g, GLubyte b);
// Carga una textura desde un archivo.
GLuint LoadDiffuseTexture(const char* path);
// Comprueba las texturas asociadas a un modelo.
void VerifyModelTextures(const std::string& modelPath);
// Dibuja un cubo con posicion, tamano y textura.
void drawCube(GLint modelLoc, GLuint VAO, const glm::mat4& parent, glm::vec3 position, glm::vec3 size, GLuint texture, float yaw = 0.0f);
// Dibuja la cancha y sus lineas.
void drawField(GLint modelLoc, GLint textureModeLoc, GLuint VAO, const glm::mat4& scene, GLuint texGrass, GLuint texWhite);
// Dibuja los limites y alrededores del estadio.
void drawPerimeter(GLint modelLoc, GLint textureModeLoc, GLuint VAO,
    const glm::mat4& scene, GLuint texWall, GLuint texFacade, GLuint texFloor, GLuint texAccent);
// Dibuja una porteria.
void drawGoal(GLint modelLoc, GLint textureModeLoc, GLuint VAO, const glm::mat4& scene, float x, float direction, GLuint texPosts, GLuint texNet);
// Dibuja un jugador y sus movimientos.
void drawPlayer(GLint modelLoc, GLuint VAO, GLuint torsoVAO, GLuint headVAO, const glm::mat4& scene,
    glm::vec3 position, float yaw, GLuint texJersey, GLuint texHead,
    GLuint texShorts, GLuint texSkin, GLuint texShoes,
    float kickAngle = 0.0f, float diveAngle = 0.0f, float walkAngle = 0.0f,
    int jerseyNumber = 0, GLuint numberColor = 0,
    float jumpHeight = 0.0f, float headNod = 0.0f);
// Dibuja la atajada del portero.
void drawGoalkeeperSave(GLint modelLoc, GLuint VAO, GLuint torsoVAO, GLuint headVAO,
    const glm::mat4& scene, glm::vec3 position, float yaw,
    GLuint jersey, GLuint face, GLuint shorts, GLuint skin, GLuint shoes,
    GLuint glove, GLuint numberColor, int number, float progress, int direction);
// Dibuja la celebracion del capitan.
void drawCelebrationCaptain(GLint modelLoc, GLuint VAO, GLuint torsoVAO, GLuint headVAO, const glm::mat4& scene,
    glm::vec3 position, float yaw, GLuint texJersey, GLuint texHead,
    GLuint texShorts, GLuint texSkin, GLuint texShoes, int jerseyNumber, GLuint numberColor);

struct BallMesh {
    GLuint VAO = 0, VBO = 0;
    GLsizei vertexCount = 0;
};

// Crea las partes blancas y negras del balon.
void createBall(BallMesh& whiteMesh, BallMesh& blackMesh);
// Genera la geometria de la pista alrededor del campo.
void createRunningTrack(BallMesh& mesh);
// Dibuja la pista del estadio.
void drawRunningTrack(GLint modelLoc, const glm::mat4& scene, const BallMesh& mesh, GLuint texture);
// Dibuja el balon en su posicion actual.
void drawBall(GLint modelLoc, GLint textureModeLoc, const glm::mat4& scene,
    const BallMesh& whiteMesh, const BallMesh& blackMesh,
    GLuint texWhite, GLuint texBlack, float rotation, glm::vec3 position);
// Libera los recursos del balon.
void deleteBallMesh(BallMesh& mesh);
// Dibuja una esfera con una textura.
void drawSolidSphere(GLint modelLoc, const glm::mat4& parent, glm::vec3 position,
    glm::vec3 size, const BallMesh& whiteMesh, const BallMesh& blackMesh, GLuint texture);
// Dibuja los arboles alrededor del estadio.
void drawTrees(GLint modelLoc, GLint textureModeLoc, GLuint VAO, const glm::mat4& scene,
    const BallMesh& whiteMesh, const BallMesh& blackMesh,
    GLuint texBark, GLuint texLeavesDark, GLuint texLeavesLight);
// Dibuja un pajaro en movimiento.
void drawBird(GLint modelLoc, GLint textureModeLoc, GLuint VAO, const glm::mat4& scene,
    const BallMesh& whiteMesh, const BallMesh& blackMesh,
    GLuint texBird, GLuint texBeak, float elapsed);
// Dibuja los fuegos artificiales.
void drawFireworks(GLint modelLoc, GLuint VAO, const glm::mat4& scene,
    float elapsed, GLuint texGold, GLuint texBlue, GLuint texRed, GLuint texWhite);
// Dibuja las torres de iluminacion.
void drawFloodlightTowers(GLint modelLoc, GLint textureModeLoc, GLuint VAO,
    const glm::mat4& scene, GLuint texMetal);
// Dibuja los paneles de las luces.
void drawFloodlightPanels(GLint modelLoc, GLuint VAO);
// Dibuja las camaras que siguen la accion.
void drawBroadcastCameraModels(GLint modelLoc, Shader& lightingShader, Model* cameraBase, Model* cameraHead);
// Dibuja las gradas del estadio.
void drawBleachers(GLint modelLoc, GLint textureModeLoc, GLuint VAO, const glm::mat4& scene,
    GLuint texConcrete, GLuint texSideWall, GLuint texWood, GLuint texWalkway, GLuint texMetal);
// Dibuja al publico en las gradas.
void drawCrowd(GLint modelLoc, GLuint VAO, GLuint jerseyVAO, GLuint headVAO,
    const glm::mat4& scene, GLuint texPumas, GLuint texAtlante,
    GLuint texHead, const GLuint crowdHeads[4], GLuint texPumasShorts, GLuint texAtlanteShorts,
    GLuint texHeadJoy = 0, GLuint texHeadAngry = 0, GLuint texHeadSad = 0, GLuint texHeadSurprise = 0);
// Indica que aficion debe celebrar.
int crowdCelebratingSide();
// Dibuja el dron y sus helices.
void drawDrone(GLint modelLoc, GLint textureModeLoc, GLuint VAO, const glm::mat4& scene,
    const BallMesh& whiteMesh, const BallMesh& blackMesh,
    glm::vec3 position, glm::vec3 target, float elapsed,
    GLuint texBody, GLuint texBlades, GLuint texMetal, GLuint texLens);
// Dibuja el trofeo del partido.
void drawTrophy(GLint modelLoc, GLint textureModeLoc, GLuint VAO, const glm::mat4& scene,
    const BallMesh& whiteMesh, const BallMesh& blackMesh,
    float elapsed, GLuint texGold, GLuint texBase, GLuint texWhite);
// Calcula la vista de la camara.
glm::mat4 getBroadcastView();
// Obtiene la posicion de la camara seleccionada.
glm::vec3 getBroadcastPosition();
// Reinicia el estado de un penal.
void ResetPenalty();
// Genera un numero aleatorio dentro de un rango.
float RandomFloat(float minValue, float maxValue);

const GLuint WIDTH = 1280, HEIGHT = 720;
int SCREEN_WIDTH, SCREEN_HEIGHT;

Camera  camera(glm::vec3(32.0f, 2.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, 0.0f);
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;
glm::vec3 freeCameraPosition(32.0f, 2.0f, 0.0f);
glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
bool fireworksActive = false;
float fireworksStartTime = 0.0f;

float vertices[] = {
     -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
       -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
       -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

       -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
       -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
       -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

       -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
       -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
       -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
       -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
       -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
       -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

       -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
       -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
       -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

       -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
       -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
       -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

enum PenaltyState { READY, PREPARING, KICKING, FLYING, FINISHED };
PenaltyState penaltyState = READY;
float stateTime = 0.0f;
float rotBall = 0.0f;
float kickAngle = 0.0f;
float keeperDive = 0.0f;
float keeperDiveProgress = 0.0f;
float keeperPositionZ = 0.0f;
float targetZ = 0.0f;
float targetY = 0.9f;
int shotSide = 0;
int keeperSide = 0;
int goals = 0;
int saves = 0;
int misses = 0;
bool penaltyRequested = false;
bool keeperCommitted = false;
glm::vec3 ballPosition(41.5f, 0.11f, 0.0f);
std::string penaltyResult = "READY";
int activeCamera = 9;
glm::vec3 broadcastCameraPositions[8] = {
    glm::vec3(-20.0f, 22.0f, -14.0f),
    glm::vec3(20.0f, 22.0f, 14.0f),
    glm::vec3(-24.0f, 0.0f, -42.5f),
    glm::vec3(24.0f, 0.0f, 42.5f),
    glm::vec3(-18.0f, 4.32f, -62.0f),
    glm::vec3(18.0f, 4.32f, 62.0f),
    glm::vec3(-60.5f, 0.0f, 7.2f),
    glm::vec3(60.5f, 0.0f, -7.2f)
};
glm::vec3 trackedBallPosition(0.0f, 0.11f, 0.0f);
glm::vec3 cameraHeadPivot(0.0f, 8.837823f, 0.0f);
const float cameraModelScale = 0.20f;
float cameraModelForwardYawDegrees = 0.0f;
const std::string cameraAssetDirectory = "Models/Camaras/";
const glm::vec3 floodlightPositions[4] = {
    glm::vec3(-64.0f, 0.0f, -68.0f),
    glm::vec3(-64.0f, 0.0f, 68.0f),
    glm::vec3(64.0f, 0.0f, -68.0f),
    glm::vec3(64.0f, 0.0f, 68.0f)
};
const float floodlightHeight = 25.8f;
const glm::vec3 floodlightTargets[4] = {
    glm::vec3(-38.0f, 0.0f, -6.0f),
    glm::vec3(-38.0f, 0.0f, 6.0f),
    glm::vec3(38.0f, 0.0f, -6.0f),
    glm::vec3(38.0f, 0.0f, 6.0f)
};

enum BusState { BUS_WAITING, BUS_STRAIGHT, BUS_TURNING, BUS_COMPLETED };
struct WheelData {
    glm::vec3 position;
    float radius;
    bool isFrontWheel;
};
WheelData busWheelData[6];
const float busPi = 3.14159265358979323846f;
const float busSpeed = 3.0f;
const float busInitialDistance = 28.0f;
const float busTurnRadius = 8.0f;
const float busTurnLength = busTurnRadius * busPi * 0.5f;
const float busRouteLength = busInitialDistance + busTurnLength;
const float busModelScale = 1.85f;
struct TeamBus {
    glm::vec3 position;
    float heading;
    float distanceTraveled;
    float steeringAngle;
    bool moving;
    BusState state;
};
TeamBus teamBuses[2];
bool busAssetsReady = false;
bool arrivalStarted = false;
bool busHeadlightsOn = false;
float teamLineupTime[2] = { 0.0f, 0.0f };
const float playerExitDelay = 0.62f;
const float playerWalkDuration = 3.7f;
const int playersPerTeam = 11;
const float pumasCornerKickTime = 0.55f;
const float pumasShotTime = 4.3f;

const glm::vec3 busPivot(0.36695f, 0.0f, 3.16461f);
// Calcula la posicion, giro y escala del autobus.
glm::mat4 getBusModelMatrix(const TeamBus& bus)
{
    glm::mat4 matrix(1.0f);
    matrix = glm::translate(matrix, bus.position);
    matrix = glm::rotate(matrix, bus.heading, glm::vec3(0.0f, 1.0f, 0.0f));
    matrix = glm::scale(matrix, glm::vec3(busModelScale));
    return glm::translate(matrix, -busPivot);
}

// Obtiene la posicion de un faro del autobus.
glm::vec3 getBusHeadlightPosition(const TeamBus& bus, int index)
{
    glm::vec3 local(index == 0 ? -0.14f : 0.87f, 1.95f, 6.75f);
    return glm::vec3(getBusModelMatrix(bus) * glm::vec4(local, 1.0f));
}

// Define donde bajan los jugadores del autobus.
glm::vec3 getPlayerExitPosition(int team)
{
    float sign = team == 0 ? 1.0f : -1.0f;
    return glm::vec3(sign * 64.8f, 0.0f, -sign * 10.2f);
}

// Define el lugar de cada jugador en la formacion.
glm::vec3 getPlayerLineupPosition(int team, int player)
{
    float sign = team == 0 ? 1.0f : -1.0f;
    return glm::vec3(sign * 59.0f, 0.0f, -26.0f + 5.2f * player);
}

// Calcula hacia donde debe mirar la camara antes del partido.
glm::vec3 getPreMatchFocusTargetForTeam(int team)
{
    glm::vec3 busTarget = teamBuses[team].position + glm::vec3(0.0f, 2.1f, 0.0f);
    if (!arrivalStarted)
        return busTarget;

    if (teamBuses[team].state == BUS_COMPLETED && teamLineupTime[team] > 0.0f) {
        glm::vec3 averagePlayers(0.0f);
        int visiblePlayers = 0;
        for (int player = 0; player < playersPerTeam; ++player) {
            float walkTime = teamLineupTime[team] - player * playerExitDelay;
            if (walkTime < 0.0f) continue;
            float t = glm::clamp(walkTime / playerWalkDuration, 0.0f, 1.0f);
            float eased = t * t * (3.0f - 2.0f * t);
            glm::vec3 start = getPlayerExitPosition(team);
            glm::vec3 finish = getPlayerLineupPosition(team, player);
            averagePlayers += glm::mix(start, finish, eased);
            ++visiblePlayers;
        }
        if (visiblePlayers > 0) {
            averagePlayers /= static_cast<float>(visiblePlayers);
            return glm::mix(busTarget, averagePlayers + glm::vec3(0.0f, 1.0f, 0.0f), 0.62f);
        }
    }
    return busTarget;
}

// Selecciona el punto que seguira la camara.
glm::vec3 getPreMatchFocusTarget()
{
    if (!arrivalStarted)
        return glm::vec3(0.0f, 0.11f, 0.0f);

    if (activeCamera >= 1 && activeCamera <= 8) {
        int team = broadcastCameraPositions[activeCamera - 1].z < 0.0f ? 0 : 1;
        return getPreMatchFocusTargetForTeam(team);
    }

    glm::vec3 leftTarget = getPreMatchFocusTargetForTeam(0);
    glm::vec3 rightTarget = getPreMatchFocusTargetForTeam(1);
    return glm::mix(leftTarget, rightTarget, 0.5f);
}

enum MatchScene {
    SCENE_IDLE, SCENE_ARRIVAL, SCENE_KICKOFF,
    SCENE_PUMAS_GOAL, SCENE_ATLANTE_GOAL, SCENE_PENALTIES, SCENE_FINAL
};
enum FadePhase { FADE_NONE, FADE_OUT, FADE_IN };
MatchScene matchScene = SCENE_IDLE;
MatchScene nextScene = SCENE_IDLE;
FadePhase fadePhase = FADE_NONE;
bool autoSequence = false;
bool sequencePaused = false;
float sceneTime = 0.0f;
float fadeAlpha = 0.0f;
float stadiumLightIntensity = 1.0f;
extern float animationClock;
int pumasScore = 0;
int atlanteScore = 0;
int matchMinute = 0;
int matchSecond = 0;
int penaltyTaken[2] = { 0, 0 };
int penaltyGoals[2] = { 0, 0 };
const int penaltyHistoryLimit = 64;
int penaltyHistory[2][penaltyHistoryLimit] = {};

// Revisa si ya hay un ganador en los penales.
int decidePenaltyWinner(const int taken[2], const int scored[2])
{
    const int limit = 5;
    if (taken[0] <= limit && taken[1] <= limit) {
        if (scored[0] > scored[1] + std::max(0, limit - taken[1])) return 0;
        if (scored[1] > scored[0] + std::max(0, limit - taken[0])) return 1;
        if (taken[0] == limit && taken[1] == limit && scored[0] != scored[1])
            return scored[0] > scored[1] ? 0 : 1;
        return -1;
    }
    if (taken[0] == taken[1] && taken[0] > limit && scored[0] != scored[1])
        return scored[0] > scored[1] ? 0 : 1;
    return -1;
}

// Cuenta los penales anotados por un equipo.
int countPenaltyGoals(int team)
{
    int total = 0;
    for (int i = 0; i < std::min(penaltyTaken[team], penaltyHistoryLimit); ++i)
        if (penaltyHistory[team][i] == 1) ++total;
    return total;
}

int penaltyKickerTeam = 0;
int penaltyWinner = -1;
int penaltyNextKickerTeam = -1;
bool penaltyShotRecorded = false;
float penaltyRestTime = 0.0f;
const float transitionDuration = 1.15f;
const glm::vec3 trophyFocusPosition(0.0f, 2.10f, 37.62f);
// Calcula cuanto se eleva el trofeo.
float trophyLiftHeight()
{
    float t = glm::clamp((sceneTime - 1.15f) / 2.75f, 0.0f, 1.0f);
    return 1.72f * t * t * (3.0f - 2.0f * t);
}

// Devuelve el nombre de la escena actual.
const char* getSceneLabel()
{
    switch (matchScene) {
    case SCENE_ARRIVAL: return "LLEGADA";
    case SCENE_KICKOFF: return "SAQUE INICIAL";
    case SCENE_PUMAS_GOAL: return "CORNER PUMAS";
    case SCENE_ATLANTE_GOAL: return "TIRO LIBRE ATLANTE";
    case SCENE_PENALTIES: return "PENALES";
    case SCENE_FINAL: return "FINAL";
    default: return "INICIO";
    }
}

// Indica que aficion debe celebrar.
int crowdCelebratingSide()
{
    if (matchScene == SCENE_PUMAS_GOAL && sceneTime >= 5.35f) return -1;
    if (matchScene == SCENE_ATLANTE_GOAL && sceneTime >= 4.1f) return 1;
    if (matchScene == SCENE_PENALTIES && penaltyState == FINISHED && penaltyResult == "GOAL")
        return penaltyKickerTeam == 0 ? -1 : 1;
    if (matchScene == SCENE_FINAL) {
        if (penaltyWinner >= 0) return penaltyWinner == 0 ? -1 : 1;
        if (pumasScore > atlanteScore) return -1;
        if (atlanteScore > pumasScore) return 1;
    }
    return 0;
}

// Comprueba si terminaron de salir los jugadores.
bool isLineupComplete()
{
    float endTime = (playersPerTeam - 1) * playerExitDelay + playerWalkDuration;
    return arrivalStarted && teamLineupTime[0] >= endTime && teamLineupTime[1] >= endTime;
}

// Inicia la llegada de los autobuses.
void startArrival()
{
    if (!busAssetsReady) {
        std::cerr << "Bus models missing; cannot start arrival." << std::endl;
        return;
    }
    for (int team = 0; team < 2; ++team) {
        TeamBus& bus = teamBuses[team];
        float sign = team == 0 ? 1.0f : -1.0f;
        float y = bus.position.y;
        bus.position = glm::vec3(sign * 102.0f, y, -sign * 21.0f);
        bus.heading = -sign * busPi * 0.5f;
        bus.distanceTraveled = 0.0f;
        bus.steeringAngle = 0.0f;
        bus.state = BUS_STRAIGHT;
        bus.moving = true;
        teamLineupTime[team] = 0.0f;
    }
    arrivalStarted = true;
    ResetPenalty();
    goals = 0;
    saves = 0;
    misses = 0;
    std::cout << "Team arrival started." << std::endl;
}

// Suaviza el movimiento de las animaciones.
float smoothMatchStep(float t)
{
    t = glm::clamp(t, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

// Calcula la altura de un salto para rematar.
float headerJump(float time, float contact, float halfWindow, float height)
{
    float distance = std::fabs(time - contact);
    if (distance >= halfWindow) return 0.0f;
    return height * 0.5f * (1.0f + std::cos(busPi * distance / halfWindow));
}

// Calcula la posicion de un jugador durante cada jugada.
glm::vec3 getMatchPlayerPosition(int team, int player, float time)
{
    float side = team == 0 ? 1.0f : -1.0f;
    const float lineZ[11] = { 0.0f, -22.0f, -8.0f, 8.0f, 22.0f,
        -18.0f, 0.0f, 18.0f, -16.0f, 0.0f, 16.0f };
    float x = player == 0 ? 50.0f : player < 5 ? 34.0f : player < 8 ? 20.0f : 8.0f;
    glm::vec3 place(side * x, 0.0f, lineZ[player]);

    if (matchScene == SCENE_KICKOFF) {
        if (team == 1 && player == 9) {
            float a = smoothMatchStep((time - 1.0f) / 2.5f);
            return glm::mix(glm::vec3(-1.0f, 0.0f, -0.5f), glm::vec3(-4.0f, 0.0f, 1.5f), a);
        }
        if (team == 1 && player == 10) {
            if (time < 2.3f) {
                float a = smoothMatchStep((time - 0.7f) / 1.6f);
                return glm::mix(glm::vec3(-6.0f, 0.0f, 7.5f), glm::vec3(-7.0f, 0.0f, 6.0f), a);
            }
            float a = smoothMatchStep((time - 2.3f) / 2.3f);
            return glm::mix(glm::vec3(-7.0f, 0.0f, 6.0f), glm::vec3(-18.0f, 0.0f, 12.0f), a);
        }
        if (player > 0) {
            float a = smoothMatchStep((time - 1.2f) / 3.4f);
            place.x += (team == 1 ? -1.0f : -1.0f) * a * (2.0f + player % 3);
            place.z += std::sin(static_cast<float>(player) * 2.1f) * a * 1.5f;
        }
    }
    else if (matchScene == SCENE_PUMAS_GOAL) {
        static const glm::vec3 atlante[11] = {
            {51.0f,0.0f,0.0f}, {45.0f,0.0f,-11.0f}, {46.0f,0.0f,-5.8f},
            {46.0f,0.0f,3.5f}, {45.0f,0.0f,10.0f}, {41.0f,0.0f,-17.0f},
            {40.0f,0.0f,16.0f}, {40.0f,0.0f,-1.0f}, {39.0f,0.0f,11.0f},
            {42.0f,0.0f,5.7f}, {40.0f,0.0f,-9.0f}
        };
        static const glm::vec3 pumas[11] = {
            {-50.0f,0.0f,0.0f}, {9.0f,0.0f,-24.0f}, {17.0f,0.0f,-16.0f},
            {29.0f,0.0f,18.0f}, {27.0f,0.0f,-23.0f}, {35.0f,0.0f,-20.0f},
            {50.8f,0.0f,-33.6f}, {36.0f,0.0f,13.0f}, {32.0f,0.0f,-11.0f},
            {39.0f,0.0f,6.5f}, {40.0f,0.0f,11.0f}
        };
        place = team == 0 ? atlante[player] : pumas[player];
        if (team == 1 && player == 8) {
            float a = smoothMatchStep((time - 0.25f) / 2.45f);
            return glm::mix(place, glm::vec3(43.0f, 0.0f, -5.0f), a);
        }
        if (team == 1 && player == 9) {
            float a = smoothMatchStep((time - 2.05f) / 2.25f);
            return glm::mix(place, glm::vec3(46.5f, 0.0f, -0.6f), a);
        }
        if (player > 0 && !(team == 1 && player == 6)) {
            float a = smoothMatchStep((time - 0.6f - 0.14f * (player % 5)) / 3.0f);
            place.x += (team == 0 ? -0.6f : 1.25f) * a;
            place.z += std::sin(1.7f * player + 0.6f * team) * 1.4f * a;
        }
    }
    else if (matchScene == SCENE_ATLANTE_GOAL) {
        static const glm::vec3 atlante[11] = {
            {49.0f,0.0f,0.0f}, {12.0f,0.0f,-23.0f}, {8.0f,0.0f,-11.0f},
            {4.0f,0.0f,13.0f}, {-5.0f,0.0f,24.0f}, {-16.0f,0.0f,-18.0f},
            {-23.0f,0.0f,13.0f}, {-21.0f,0.0f,22.0f}, {-18.0f,0.0f,-7.0f},
            {-25.0f,0.0f,-2.1f}, {-22.0f,0.0f,9.0f}
        };
        static const glm::vec3 pumas[11] = {
            {-50.6f,0.0f,0.0f}, {-39.0f,0.0f,-4.5f}, {-39.0f,0.0f,-1.5f},
            {-39.0f,0.0f,1.5f}, {-39.0f,0.0f,4.5f}, {-45.0f,0.0f,-12.0f},
            {-44.0f,0.0f,12.0f}, {-34.0f,0.0f,17.0f}, {-37.0f,0.0f,-17.0f},
            {-42.0f,0.0f,22.0f}, {-45.0f,0.0f,-24.0f}
        };
        place = team == 0 ? atlante[player] : pumas[player];
        if (team == 0 && player == 9) {
            float a = smoothMatchStep((time - 0.35f) / 0.95f);
            return glm::mix(place, glm::vec3(-28.0f, 0.0f, 0.0f), a);
        }
        if (team == 1 && player >= 1 && player <= 4) return place;
        if (player > 0) {
            float a = smoothMatchStep((time - 1.1f) / 2.5f);
            place.x += (team == 0 ? -2.8f : -0.65f) * a;
            place.z += std::sin(player * 1.25f) * 1.0f * a;
        }
    }
    return place;
}

// Calcula el giro del jugador hacia el balon.
float yawTowardBall(glm::vec3 player, glm::vec3 ball, float fallback)
{
    glm::vec2 d(ball.x - player.x, ball.z - player.z);
    if (glm::dot(d, d) < 0.12f) return fallback;
    return glm::degrees(std::atan2(d.x, d.y));
}

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
float animationClock = 0.0f;

// Inicializa la ventana, los modelos y la escena principal.
int main()
{
#ifdef _WIN32
    // Las rutas de modelos y shaders se resuelven junto al ejecutable.
    std::vector<wchar_t> executablePath(32768);
    DWORD pathLength = GetModuleFileNameW(nullptr, executablePath.data(),
        static_cast<DWORD>(executablePath.size()));
    if (pathLength == 0 || pathLength >= executablePath.size()) {
        std::cerr << "No se pudo localizar la carpeta del ejecutable." << std::endl;
        return EXIT_FAILURE;
    }
    std::error_code pathError;
    std::filesystem::current_path(
        std::filesystem::path(executablePath.data()).parent_path(), pathError);
    if (pathError) {
        std::cerr << "No se pudo abrir la carpeta del ejecutable: "
            << pathError.message() << std::endl;
        return EXIT_FAILURE;
    }
#endif
    glfwInit();

    GLFWwindow* window = glfwCreateWindow(
        1280,
        720,
        "Pumas vs Atlante - Estadio",
        nullptr,
        nullptr
    );
    if (nullptr == window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();

        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);

    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int width, int height) {
        SCREEN_WIDTH = std::max(1, width);
        SCREEN_HEIGHT = std::max(1, height);
        glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        });

    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit())
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    srand(static_cast<unsigned int>(time(nullptr)));

    Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");
    Shader lampShader("Shader/lamp.vs", "Shader/lamp.frag");
    Shader skyShader("Shader/sky.vs", "Shader/sky.frag");

    const char* wheelFiles[6] = {
        "Models/autobus/llanta_1.obj",
        "Models/autobus/llanta_2.obj",
        "Models/autobus/llanta_3.obj",
        "Models/autobus/llanta_4obj.obj",
        "Models/autobus/llanta_5.obj",
        "Models/autobus/llanta_6.obj"
    };
    const char* bodyFiles[2] = {
        "Models/autobus/carroceria_atlante.obj",
        "Models/autobus/carroceria_pumas.obj"
    };
    busAssetsReady = loadBusWheelData("Models/autobus/ruedas.txt");
    float busGroundY = 0.0f;
    if (busAssetsReady) {
        float lowestWheelPoint = busWheelData[0].position.y - busWheelData[0].radius;
        for (int i = 1; i < 6; ++i)
            lowestWheelPoint = std::min(lowestWheelPoint, busWheelData[i].position.y - busWheelData[i].radius);
        busGroundY = -busModelScale * lowestWheelPoint;
    }
    teamBuses[0] = { glm::vec3(102.0f, busGroundY, -21.0f),
        -busPi * 0.5f, 0.0f, 0.0f, false, BUS_WAITING };
    teamBuses[1] = { glm::vec3(-102.0f, busGroundY, 21.0f),
        busPi * 0.5f, 0.0f, 0.0f, false, BUS_WAITING };
    for (int i = 0; i < 2; ++i) {
        std::ifstream file(bodyFiles[i]);
        if (!file) {
            std::cerr << "Missing team bus: " << bodyFiles[i] << std::endl;
            busAssetsReady = false;
        }
    }
    for (int i = 0; i < 6; ++i) {
        std::ifstream file(wheelFiles[i]);
        if (!file) {
            std::cerr << "Missing bus wheel: " << wheelFiles[i] << std::endl;
            busAssetsReady = false;
        }
    }
    std::unique_ptr<Model> busBodyModels[2];
    std::unique_ptr<Model> busWheelModels[6];
    if (busAssetsReady) {
        for (int i = 0; i < 2; ++i) {
            VerifyModelTextures(bodyFiles[i]);
            busBodyModels[i].reset(new Model(const_cast<char*>(bodyFiles[i])));
        }
        for (int i = 0; i < 6; ++i) {
            VerifyModelTextures(wheelFiles[i]);
            busWheelModels[i].reset(new Model(const_cast<char*>(wheelFiles[i])));
        }
    }

    std::unique_ptr<Model> broadcastCameraBase;
    std::unique_ptr<Model> broadcastCameraHead;
    const std::string cameraBasePath = cameraAssetDirectory + "cameraBase.obj";
    const std::string cameraHeadPath = cameraAssetDirectory + "cameraHead.obj";
    {
        std::ifstream baseFile(cameraBasePath.c_str());
        std::ifstream headFile(cameraHeadPath.c_str());
        if (baseFile.good() && headFile.good()) {
            VerifyModelTextures(cameraBasePath);
            VerifyModelTextures(cameraHeadPath);
            broadcastCameraBase.reset(new Model(const_cast<char*>(cameraBasePath.c_str())));
            broadcastCameraHead.reset(new Model(const_cast<char*>(cameraHeadPath.c_str())));
            std::cout << "Camera meshes loaded separately: fixed base and tracking head." << std::endl;
            std::ifstream pivotFile((cameraAssetDirectory + "camera_pivot.txt").c_str());
            if (pivotFile && !(pivotFile >> cameraHeadPivot.x >> cameraHeadPivot.y >> cameraHeadPivot.z)) {
                cameraHeadPivot = glm::vec3(0.0f, 8.837823f, 0.0f);
                std::cerr << "Invalid camera_pivot.txt; using default pivot." << std::endl;
            }
            std::cout << "Camera pivot Y: " << cameraHeadPivot.y
                << ", camera scale: " << cameraModelScale
                << ", forward correction: " << cameraModelForwardYawDegrees << std::endl;
        }
        else {
            std::cout << "Camera OBJ parts not found; views 1-8 remain available." << std::endl;
        }
    }

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // El uniforme utiliza su propio VAO para aplicar las texturas.
    static_assert(sizeof(vertices) / sizeof(vertices[0]) == 36 * 6, "Expected 36 cube vertices");
    GLfloat jerseyVertices[36 * 8];
    for (int i = 0; i < 36; ++i) {
        for (int j = 0; j < 6; ++j)
            jerseyVertices[i * 8 + j] = vertices[i * 6 + j];
        float x = vertices[i * 6], y = vertices[i * 6 + 1], nz = vertices[i * 6 + 5];
        jerseyVertices[i * 8 + 6] = nz > 0.5f ? 0.02f + (x + 0.5f) * 0.46f :
            nz < -0.5f ? 0.52f + (0.5f - x) * 0.46f : 0.005f;
        jerseyVertices[i * 8 + 7] = std::fabs(nz) > 0.5f ? 0.96f - (y + 0.5f) * 0.92f : 0.005f;
    }
    GLuint jerseyVAO, jerseyVBO;
    glGenVertexArrays(1, &jerseyVAO);
    glGenBuffers(1, &jerseyVBO);
    glBindVertexArray(jerseyVAO);
    glBindBuffer(GL_ARRAY_BUFFER, jerseyVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(jerseyVertices), jerseyVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    // UV de la cabeza según la cara del cubo.
    GLfloat headVertices[36 * 8];
    for (int i = 0; i < 36; ++i) {
        for (int j = 0; j < 6; ++j)
            headVertices[i * 8 + j] = vertices[i * 6 + j];
        float x = vertices[i * 6], y = vertices[i * 6 + 1], z = vertices[i * 6 + 2];
        float nx = vertices[i * 6 + 3], ny = vertices[i * 6 + 4], nz = vertices[i * 6 + 5];
        float column = nz > 0.5f ? 1.0f : nz < -0.5f ? 3.0f :
            nx > 0.5f ? 2.0f : nx < -0.5f ? 0.0f : 1.0f;
        float row = ny > 0.5f ? 0.0f : ny < -0.5f ? 2.0f : 1.0f;
        float u = nz > 0.5f ? x + 0.5f : nz < -0.5f ? 0.5f - x :
            nx > 0.5f ? 0.5f - z : nx < -0.5f ? z + 0.5f : x + 0.5f;
        float v = ny > 0.5f ? z + 0.5f : ny < -0.5f ? 0.5f - z : 0.5f - y;
        headVertices[i * 8 + 6] = (column + 0.02f + 0.96f * u) / 4.0f;
        headVertices[i * 8 + 7] = (row + 0.02f + 0.96f * v) / 3.0f;
    }
    GLuint headVAO, headVBO;
    glGenVertexArrays(1, &headVAO);
    glGenBuffers(1, &headVBO);
    glBindVertexArray(headVAO);
    glBindBuffer(GL_ARRAY_BUFFER, headVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(headVertices), headVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    GLuint fadeProgram = createFadeProgram();
    GLuint fadeVAO, fadeVBO;
    const float fadeVertices[] = { -1.f,-1.f, 1.f,-1.f, 1.f,1.f,
                                   -1.f,-1.f, 1.f,1.f, -1.f,1.f };
    glGenVertexArrays(1, &fadeVAO);
    glGenBuffers(1, &fadeVBO);
    glBindVertexArray(fadeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, fadeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fadeVertices), fadeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    GLuint texGrass = LoadDiffuseTexture("Models/Textures/Grass005_Color.jpg");
    if (!texGrass) texGrass = CreateSolidColorTexture(35, 145, 55);
    GLuint texWhite = CreateSolidColorTexture(245, 245, 245);
    GLuint texBlack = CreateSolidColorTexture(20, 20, 20);
    GLuint texBlue = CreateSolidColorTexture(25, 70, 210);
    GLuint texPumas = LoadDiffuseTexture("Models/Textures/pumas_navy_jersey.png");
    if (!texPumas) texPumas = CreateSolidColorTexture(14, 30, 61);
    GLuint texRed = CreateSolidColorTexture(205, 45, 45);
    GLuint texPenaltyGreen = CreateSolidColorTexture(45, 210, 75);

    GLuint texAtlante = LoadDiffuseTexture("Models/Textures/atlante_white_jersey.png");
    if (!texAtlante) texAtlante = CreateSolidColorTexture(245, 245, 245);
    GLuint texRefereeJersey = LoadDiffuseTexture("Models/Textures/referee_yellow_jersey.png");
    if (!texRefereeJersey) texRefereeJersey = CreateSolidColorTexture(247, 205, 45);
    GLuint texGoalkeeperJersey = LoadDiffuseTexture("Models/Textures/goalkeeper_green_jersey.png");
    if (!texGoalkeeperJersey) texGoalkeeperJersey = CreateSolidColorTexture(32, 185, 95);
    GLuint texBallWhite = LoadDiffuseTexture("Models/Textures/leather_white_diff_1k.jpg");
    if (!texBallWhite) texBallWhite = texWhite;
    GLuint texBallBlack = LoadDiffuseTexture("Models/Textures/fabric_leather_01_diff_1k.jpg");
    if (!texBallBlack) texBallBlack = texBlack;
    GLuint texShorts = CreateSolidColorTexture(35, 35, 55);
    GLuint texSkin = CreateSolidColorTexture(205, 150, 105);
    GLuint texHead = LoadDiffuseTexture("Models/Textures/player_head_textures/atlas_emociones/atlas_neutral.png");
    if (!texHead) texHead = CreateSolidColorTexture(205, 150, 105);
    GLuint texHeadJoy = LoadDiffuseTexture("Models/Textures/player_head_textures/atlas_emociones/atlas_alegria.png");
    if (!texHeadJoy) texHeadJoy = texHead;
    GLuint texHeadAngry = LoadDiffuseTexture("Models/Textures/player_head_textures/atlas_emociones/atlas_enojo.png");
    if (!texHeadAngry) texHeadAngry = texHead;
    GLuint texHeadSad = LoadDiffuseTexture("Models/Textures/player_head_textures/atlas_emociones/atlas_tristeza.png");
    if (!texHeadSad) texHeadSad = texHead;
    GLuint texHeadSurprise = LoadDiffuseTexture("Models/Textures/player_head_textures/atlas_emociones/atlas_sorpresa.png");
    if (!texHeadSurprise) texHeadSurprise = texHead;

    for (GLuint th : { texHead, texHeadJoy, texHeadAngry, texHeadSad, texHeadSurprise }) {
        if (th) {
            glBindTexture(GL_TEXTURE_2D, th);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    GLuint crowdHeads[4] = {};
    for (int i = 0; i < 4; ++i) {
        std::string path = "Models/Textures/player_head_textures/crowd/crowd_head_" +
            std::to_string(i) + ".png";
        crowdHeads[i] = LoadDiffuseTexture(path.c_str());
        if (crowdHeads[i]) {
            glBindTexture(GL_TEXTURE_2D, crowdHeads[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    GLuint texShoes = CreateSolidColorTexture(30, 30, 30);
    GLuint texPosts = LoadDiffuseTexture("Models/Textures/metal_plate_02_diff_1k.jpg");
    if (!texPosts) texPosts = CreateSolidColorTexture(240, 240, 240);
    GLuint texNet = LoadDiffuseTexture("Models/Textures/fabric_leather_01_diff_1k.jpg");
    if (!texNet) texNet = CreateSolidColorTexture(178, 190, 190);
    GLuint texSpecular = CreateSolidColorTexture(35, 35, 35);
    GLuint texBark = LoadDiffuseTexture("Models/Textures/pine_bark_diff_1k.jpg");
    if (!texBark) texBark = CreateSolidColorTexture(110, 73, 43);
    GLuint texLeavesDark = LoadDiffuseTexture("Models/Textures/LeafSet024_Dark.jpg");
    if (!texLeavesDark) texLeavesDark = CreateSolidColorTexture(35, 102, 47);
    GLuint texLeavesLight = LoadDiffuseTexture("Models/Textures/LeafSet024_Light.jpg");
    if (!texLeavesLight) texLeavesLight = CreateSolidColorTexture(62, 141, 67);
    GLuint texBird = LoadDiffuseTexture("Models/Textures/fabric_leather_02_diff_1k.jpg");
    if (!texBird) texBird = CreateSolidColorTexture(83, 89, 96);
    GLuint texBeak = LoadDiffuseTexture("Models/Textures/rough_wood_diff_1k.jpg");
    if (!texBeak) texBeak = CreateSolidColorTexture(241, 167, 52);
    GLuint texDroneBody = LoadDiffuseTexture("Models/Textures/blue_metal_plate_diff_1k.jpg");
    if (!texDroneBody) texDroneBody = CreateSolidColorTexture(35, 35, 55);
    GLuint texDroneArm = LoadDiffuseTexture("Models/Textures/metal_plate_04_diff_1k.jpg");
    if (!texDroneArm) texDroneArm = CreateSolidColorTexture(130, 137, 145);
    GLuint texDroneRotor = LoadDiffuseTexture("Models/Textures/rubber_tiles_diff_1k.jpg");
    if (!texDroneRotor) texDroneRotor = CreateSolidColorTexture(20, 20, 20);
    GLuint texDroneLens = LoadDiffuseTexture("Models/Textures/metal_plate_02_diff_1k.jpg");
    if (!texDroneLens) texDroneLens = CreateSolidColorTexture(25, 70, 210);
    GLuint texGold = LoadDiffuseTexture("Models/Textures/trophy_gold_diff_1k.jpg");
    if (!texGold) texGold = CreateSolidColorTexture(255, 206, 56);
    GLuint texMarble = LoadDiffuseTexture("Models/Textures/marble_01_diff_1k.jpg");
    if (!texMarble) texMarble = CreateSolidColorTexture(18, 18, 20);
    GLuint texDarkWood = LoadDiffuseTexture("Models/Textures/dark_wood_diff_1k.jpg");
    if (!texDarkWood) texDarkWood = CreateSolidColorTexture(65, 42, 22);
    GLuint texFloodlight = CreateSolidColorTexture(130, 137, 145);
    GLuint texTowerMetal = LoadDiffuseTexture("Models/Textures/worn_shutter_diff_1k.jpg");
    if (!texTowerMetal) texTowerMetal = CreateSolidColorTexture(55, 53, 51);
    GLuint texConcrete = CreateSolidColorTexture(183, 181, 175);
    GLuint texFacade = LoadDiffuseTexture("Models/Textures/concrete_diff_1k.jpg");
    if (!texFacade) texFacade = CreateSolidColorTexture(183, 181, 175);
    GLuint texGravelConcrete = LoadDiffuseTexture("Models/Textures/gravel_concrete_03_diff_1k.jpg");
    if (!texGravelConcrete) texGravelConcrete = CreateSolidColorTexture(183, 181, 175);
    GLuint texWalkway = CreateSolidColorTexture(112, 108, 103);
    GLuint texRunningTrack = LoadDiffuseTexture("Models/Textures/running_track_diff_1k.jpg");
    if (!texRunningTrack) texRunningTrack = CreateSolidColorTexture(77, 112, 135);
    GLuint texWood = LoadDiffuseTexture("Models/Textures/old_wooden_floor_02_diff_1k.jpg");
    if (!texWood) texWood = CreateSolidColorTexture(100, 65, 30);
    GLuint texAsphalt = LoadDiffuseTexture("Models/Textures/clean_asphalt_diff_1k.jpg");
    if (!texAsphalt) texAsphalt = CreateSolidColorTexture(112, 108, 103);

    BallMesh whiteMesh, blackMesh;
    createBall(whiteMesh, blackMesh);
    BallMesh runningTrackMesh;
    createRunningTrack(runningTrackMesh);

    lightingShader.Use();
    glUniform1i(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0);
    glUniform1i(glGetUniformLocation(lightingShader.Program, "material.specular"), 1);
    glUniform1i(glGetUniformLocation(lightingShader.Program, "texture_diffuse1"), 0);
    glUniform1i(glGetUniformLocation(lightingShader.Program, "texture_specular1"), 1);
    glUniform1i(glGetUniformLocation(lightingShader.Program, "useImportedModelTexture"), 0);
    glUniform1i(glGetUniformLocation(lightingShader.Program, "cameraNeutralMaterial"), 0);
    glUniform1i(glGetUniformLocation(lightingShader.Program, "proceduralTextureMode"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texSpecular);
    glActiveTexture(GL_TEXTURE0);

    GLint textureModeLoc = glGetUniformLocation(lightingShader.Program, "proceduralTextureMode");
    GLint importedTextureLoc = glGetUniformLocation(lightingShader.Program, "useImportedModelTexture");

    glm::mat4 projection = glm::perspective(glm::radians(camera.GetZoom()), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 300.0f);

    while (!glfwWindowShouldClose(window))
    {

        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        if (!sequencePaused) {
            animationClock += deltaTime;
            DoMovement();
            updateMatch(deltaTime);
            if (matchScene == SCENE_PENALTIES && fadePhase != FADE_OUT)
                Animation();
            if (matchScene == SCENE_ARRIVAL && fadePhase != FADE_OUT)
                updateBusAnimation();
            updateFade(deltaTime);
        }
        const bool ballVisible = matchScene == SCENE_KICKOFF || matchScene == SCENE_PUMAS_GOAL ||
            matchScene == SCENE_ATLANTE_GOAL || matchScene == SCENE_PENALTIES;
        float trackingAmount = 1.0f - std::exp(-4.0f * std::max(0.0f, sequencePaused ? 0.0f : deltaTime));
        glm::vec3 trackedTarget = matchScene == SCENE_FINAL ? trophyFocusPosition :
            ballVisible ? ballPosition :
            ((matchScene == SCENE_ARRIVAL || matchScene == SCENE_IDLE) ? getPreMatchFocusTarget() : glm::vec3(0.0f, 0.11f, 0.0f));
        trackedBallPosition = glm::mix(trackedBallPosition, trackedTarget, trackingAmount);
        std::string title = "PUMAS " + std::to_string(pumasScore) + " - " +
            std::to_string(atlanteScore) + " ATLANTE | " + getSceneLabel() +
            (sequencePaused ? " [PAUSA]" : "") +
            " | PEN " + std::to_string(penaltyGoals[0]) + "-" +
            std::to_string(penaltyGoals[1]) + " | P: tirar penal | SPACE: pausa | R: reiniciar";
        glfwSetWindowTitle(window, title.c_str());

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);

        glm::mat4 modelTemp = glm::mat4(1.0f);

        lightingShader.Use();

        glUniform1i(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0);
        glUniform1i(glGetUniformLocation(lightingShader.Program, "material.specular"), 1);
        glUniform1i(glGetUniformLocation(lightingShader.Program, "texture_diffuse1"), 0);
        glUniform1i(glGetUniformLocation(lightingShader.Program, "texture_specular1"), 1);
        glUniform1i(glGetUniformLocation(lightingShader.Program, "useImportedModelTexture"), 0);
        glUniform1i(glGetUniformLocation(lightingShader.Program, "cameraNeutralMaterial"), 0);
        glUniform1i(glGetUniformLocation(lightingShader.Program, "proceduralTextureMode"), 0);

        GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");
        glm::vec3 viewPosition = getBroadcastPosition();
        glUniform3f(viewPosLoc, viewPosition.x, viewPosition.y, viewPosition.z);

        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), 0.23f, 0.25f, 0.32f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"),
            0.18f, 0.21f, 0.30f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), 0.3f, 0.3f, 0.3f);

        for (int i = 0; i < 4; ++i) {
            glm::vec3 p = floodlightPositions[i] + glm::vec3(0.0f, floodlightHeight, 0.0f);
            std::string name = "pointLights[" + std::to_string(i) + "].";
            glUniform3f(glGetUniformLocation(lightingShader.Program, (name + "position").c_str()), p.x, p.y, p.z);
            glUniform3f(glGetUniformLocation(lightingShader.Program, (name + "ambient").c_str()), 0.0f, 0.0f, 0.0f);
            glUniform3f(glGetUniformLocation(lightingShader.Program, (name + "diffuse").c_str()), 0.0f, 0.0f, 0.0f);
            glUniform3f(glGetUniformLocation(lightingShader.Program, (name + "specular").c_str()), 0.0f, 0.0f, 0.0f);
            glUniform1f(glGetUniformLocation(lightingShader.Program, (name + "constant").c_str()), 1.0f);
            glUniform1f(glGetUniformLocation(lightingShader.Program, (name + "linear").c_str()), 0.09f);
            glUniform1f(glGetUniformLocation(lightingShader.Program, (name + "quadratic").c_str()), 0.032f);
        }

        for (int team = 0; team < 2; ++team) {
            for (int lamp = 0; lamp < 2; ++lamp) {
                int lightIndex = 4 + team * 2 + lamp;
                std::string name = "pointLights[" + std::to_string(lightIndex) + "].";
                glm::vec3 p = getBusHeadlightPosition(teamBuses[team], lamp);
                glUniform3f(glGetUniformLocation(lightingShader.Program, (name + "position").c_str()), p.x, p.y, p.z);
                glUniform3f(glGetUniformLocation(lightingShader.Program, (name + "ambient").c_str()), 0.0f, 0.0f, 0.0f);
                float brightness = (busAssetsReady && busHeadlightsOn) ? 2.0f : 0.0f;
                glUniform3f(glGetUniformLocation(lightingShader.Program, (name + "diffuse").c_str()), brightness, brightness * 0.96f, brightness * 0.83f);
                glUniform3f(glGetUniformLocation(lightingShader.Program, (name + "specular").c_str()), brightness * 0.3f, brightness * 0.3f, brightness * 0.3f);
                glUniform1f(glGetUniformLocation(lightingShader.Program, (name + "constant").c_str()), 1.0f);
                glUniform1f(glGetUniformLocation(lightingShader.Program, (name + "linear").c_str()), 0.13f);
                glUniform1f(glGetUniformLocation(lightingShader.Program, (name + "quadratic").c_str()), 0.07f);
            }
        }

        for (int i = 0; i < 4; ++i) {
            glm::vec3 p = floodlightPositions[i] + glm::vec3(0.0f, floodlightHeight, 0.0f);
            glm::vec3 direction = glm::normalize(floodlightTargets[i] - p);
            std::string name = "spotLights[" + std::to_string(i) + "].";
            glUniform3f(glGetUniformLocation(lightingShader.Program, (name + "position").c_str()), p.x, p.y, p.z);
            glUniform3f(glGetUniformLocation(lightingShader.Program, (name + "direction").c_str()),
                direction.x, direction.y, direction.z);
            glUniform3f(glGetUniformLocation(lightingShader.Program, (name + "ambient").c_str()),
                0.01f * stadiumLightIntensity, 0.01f * stadiumLightIntensity, 0.01f * stadiumLightIntensity);
            glUniform3f(glGetUniformLocation(lightingShader.Program, (name + "diffuse").c_str()),
                1.72f * stadiumLightIntensity, 1.60f * stadiumLightIntensity, 1.40f * stadiumLightIntensity);
            glUniform3f(glGetUniformLocation(lightingShader.Program, (name + "specular").c_str()),
                0.25f * stadiumLightIntensity, 0.25f * stadiumLightIntensity, 0.22f * stadiumLightIntensity);
            glUniform1f(glGetUniformLocation(lightingShader.Program, (name + "constant").c_str()), 1.0f);
            glUniform1f(glGetUniformLocation(lightingShader.Program, (name + "linear").c_str()), 0.0035f);
            glUniform1f(glGetUniformLocation(lightingShader.Program, (name + "quadratic").c_str()), 0.00016f);
            glUniform1f(glGetUniformLocation(lightingShader.Program, (name + "cutOff").c_str()), glm::cos(glm::radians(31.0f)));
            glUniform1f(glGetUniformLocation(lightingShader.Program, (name + "outerCutOff").c_str()), glm::cos(glm::radians(47.0f)));
        }

        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 5.0f);

        glm::mat4 view;
        view = getBroadcastView();
        projection = glm::perspective(glm::radians(activeCamera == 9 ? 62.0f :
            (activeCamera == 1 || activeCamera == 2 ? 54.0f : camera.GetZoom())),
            (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 300.0f);

        skyShader.Use();
        glm::mat4 inverseProjection = glm::inverse(projection);
        glm::mat3 inverseViewRotation = glm::mat3(glm::inverse(view));
        glUniformMatrix4fv(glGetUniformLocation(skyShader.Program, "inverseProjection"), 1, GL_FALSE,
            glm::value_ptr(inverseProjection));
        glUniformMatrix3fv(glGetUniformLocation(skyShader.Program, "inverseViewRotation"), 1, GL_FALSE,
            glm::value_ptr(inverseViewRotation));
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glBindVertexArray(fadeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
        lightingShader.Use();

        GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
        GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
        GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glm::mat4 model(1);

        view = getBroadcastView();
        model = glm::mat4(1);
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);

        modelTemp = model;
        glUniform1i(textureModeLoc, 0);
        glUniform1i(importedTextureLoc, 0);
        drawPerimeter(modelLoc, textureModeLoc, VAO, modelTemp, texConcrete, texFacade, texAsphalt, texWhite);
        drawField(modelLoc, textureModeLoc, VAO, modelTemp, texGrass, texWhite);
        drawRunningTrack(modelLoc, modelTemp, runningTrackMesh, texRunningTrack);
        drawGoal(modelLoc, textureModeLoc, VAO, modelTemp, -52.5f, -1.0f, texPosts, texNet);
        drawGoal(modelLoc, textureModeLoc, VAO, modelTemp, 52.5f, 1.0f, texPosts, texNet);

        const bool penaltyPlayersVisible = matchScene == SCENE_PENALTIES;
        if (penaltyPlayersVisible) {
            int celebratingSide = crowdCelebratingSide();
            GLuint kickerHead = (celebratingSide != 0) ? texHeadJoy :
                ((penaltyState == FINISHED && penaltyResult == "MISSED") ? texHeadAngry : texHead);
            GLuint keeperHead = (celebratingSide != 0) ? texHeadSurprise :
                ((penaltyState == FINISHED && penaltyResult == "MISSED") ? texHeadJoy : texHead);

            drawPlayer(modelLoc, VAO, jerseyVAO, headVAO, modelTemp,
                glm::vec3(40.4f, 0.0f, 0.0f), 90.0f,
                penaltyKickerTeam == 0 ? texPumas : texAtlante,
                kickerHead,
                penaltyKickerTeam == 0 ? texShorts : texRed,
                texSkin, texShoes, kickAngle, 0.0f, 0.0f, 10, texWhite);

            drawGoalkeeperSave(modelLoc, VAO, jerseyVAO, headVAO, modelTemp,
                glm::vec3(51.2f, 0.0f, keeperPositionZ), -90.0f,
                texGoalkeeperJersey, keeperHead, texShorts, texSkin, texShoes,
                texWhite, texWhite, 1,
                keeperDiveProgress, keeperSide);
        }

        if (arrivalStarted && (matchScene == SCENE_ARRIVAL || matchScene == SCENE_IDLE)) {
            for (int team = 0; team < 2; ++team) {
                if (teamBuses[team].state != BUS_COMPLETED || teamLineupTime[team] <= 0.0f) continue;
                for (int player = 0; player < playersPerTeam; ++player) {
                    if (penaltyPlayersVisible && player == playersPerTeam - 1) continue;
                    float walkTime = teamLineupTime[team] - player * playerExitDelay;
                    if (walkTime < 0.0f) continue;
                    float t = glm::clamp(walkTime / playerWalkDuration, 0.0f, 1.0f);
                    float eased = t * t * (3.0f - 2.0f * t);
                    glm::vec3 start = getPlayerExitPosition(team);
                    glm::vec3 finish = getPlayerLineupPosition(team, player);
                    glm::vec3 playerPosition = glm::mix(start, finish, eased);
                    glm::vec3 movement = finish - start;
                    float yaw = t < 1.0f ? glm::degrees(std::atan2(movement.x, movement.z)) :
                        (team == 0 ? -90.0f : 90.0f);
                    float step = t < 1.0f ? 23.0f * sinf(walkTime * 9.0f) : 0.0f;
                    drawPlayer(modelLoc, VAO, jerseyVAO, headVAO, modelTemp,
                        playerPosition, yaw,
                        team == 0 ? texAtlante : texPumas,
                        texHead,
                        team == 0 ? texRed : texShorts, texSkin, texShoes,
                        0.0f, 0.0f, step, player + 1, texWhite);
                }
            }
        }

        if (matchScene == SCENE_FINAL) {
            int winningSide = crowdCelebratingSide();
            int champion = winningSide == 0 ? -1 : (winningSide < 0 ? 1 : 0);
            for (int team = 0; team < 2; ++team) {
                for (int player = 0; player < playersPerTeam; ++player) {
                    glm::vec3 place;
                    float yaw = team == champion ? 180.0f : 0.0f;
                    float walk = 3.0f * sinf(sceneTime * 2.0f + player);
                    if (team == champion) {
                        if (player == 5) {
                            place = glm::vec3(0.0f, 0.0f, 38.0f);
                            drawCelebrationCaptain(modelLoc, VAO, jerseyVAO, headVAO, modelTemp, place, 180.0f,
                                team == 0 ? texAtlante : texPumas, texHeadJoy,
                                team == 0 ? texRed : texShorts, texSkin, texShoes,
                                player + 1, texWhite);
                            continue;
                        }
                        int idx = player < 5 ? player : player - 1;
                        float side = idx < 5 ? -1.0f : 1.0f;
                        int rank = idx < 5 ? idx : idx - 5;
                        place = glm::vec3(side * (4.5f + rank * 2.2f), 0.0f, 41.0f + (rank - 2.0f) * 1.6f);
                    }
                    else {
                        place = glm::vec3(-10.0f + player * 2.0f, 0.0f, 48.0f);
                    }
                    GLuint finalHead = (team == champion) ? texHeadJoy : ((player % 2 == 0) ? texHeadSad : texHeadAngry);
                    drawPlayer(modelLoc, VAO, jerseyVAO, headVAO, modelTemp,
                        place, yaw,
                        team == 0 ? texAtlante : texPumas,
                        finalHead,
                        team == 0 ? texRed : texShorts, texSkin, texShoes,
                        0.0f, 0.0f, walk, player + 1, texWhite);
                }
            }
        }

        if (matchScene == SCENE_KICKOFF || matchScene == SCENE_PUMAS_GOAL || matchScene == SCENE_ATLANTE_GOAL) {
            for (int team = 0; team < 2; ++team) {
                for (int player = 0; player < playersPerTeam; ++player) {
                    glm::vec3 place = getMatchPlayerPosition(team, player, sceneTime);
                    float walk = 0.0f;
                    float kick = 0.0f;
                    float dive = 0.0f;
                    float jump = 0.0f;
                    float nod = 0.0f;
                    float yaw = yawTowardBall(place, ballPosition, team == 1 ? 90.0f : -90.0f);
                    glm::vec3 futurePlace = getMatchPlayerPosition(team, player, sceneTime + 0.08f);
                    bool running = glm::distance(futurePlace, place) > 0.018f;
                    if (running)
                        walk = 23.0f * sinf(sceneTime * 10.0f + player * 0.8f);

                    if (matchScene == SCENE_KICKOFF && team == 1 && player == 9) {
                        if (sceneTime >= 0.60f && sceneTime <= 1.35f)
                            kick = 42.0f * sinf((sceneTime - 0.60f) / 0.75f * busPi);
                    }
                    if (matchScene == SCENE_PUMAS_GOAL) {
                        if (team == 1 && player == 6) {
                            if (sceneTime >= 0.20f && sceneTime < 0.85f)
                                kick = 44.0f * sinf((sceneTime - 0.20f) / 0.65f * busPi);
                            yaw = yawTowardBall(place, glm::vec3(43.0f, 0.0f, -5.0f), 0.0f);
                        }
                        if (team == 1 && player == 8) {
                            jump = headerJump(sceneTime, 2.7f, 0.42f, 0.36f);
                            nod = 35.0f * jump / 0.36f;
                            yaw = yawTowardBall(place, glm::vec3(46.5f, 0.0f, -0.6f), 80.0f);
                        }
                        if (team == 1 && player == 9) {
                            jump = headerJump(sceneTime, pumasShotTime, 0.46f, 0.42f);
                            nod = 40.0f * jump / 0.42f;
                            yaw = yawTowardBall(place, glm::vec3(53.4f, 0.0f, 0.65f), 90.0f);
                        }
                    }
                    if (matchScene == SCENE_ATLANTE_GOAL) {
                        if (team == 0 && player == 9 && sceneTime >= 0.92f && sceneTime <= 1.68f)
                            kick = 42.0f * sinf((sceneTime - 0.92f) / 0.76f * busPi);
                        if (team == 1 && player >= 1 && player <= 4) {
                            jump = headerJump(sceneTime, 1.55f, 0.35f, 0.22f);
                        }
                    }

                    int celebratingSide = crowdCelebratingSide();
                    int teamSide = team == 0 ? 1 : -1;
                    GLuint matchHead = texHead;
                    if (celebratingSide != 0) {
                        matchHead = (teamSide == celebratingSide) ? texHeadJoy :
                            ((player % 2 == 0) ? texHeadSad : texHeadSurprise);
                    }
                    if (player == 0 && (matchScene == SCENE_PUMAS_GOAL || matchScene == SCENE_ATLANTE_GOAL)) {
                        bool homeGoalkeeper = matchScene == SCENE_PUMAS_GOAL && team == 0;
                        bool awayGoalkeeper = matchScene == SCENE_ATLANTE_GOAL && team == 1;
                        if (homeGoalkeeper || awayGoalkeeper) {
                            float goalkeeperYaw = team == 0 ? -90.0f : 90.0f;
                            float progress = homeGoalkeeper ?
                                smoothMatchStep((sceneTime - 4.35f) / 0.58f) :
                                smoothMatchStep((sceneTime - 2.10f) / 0.80f);
                            drawGoalkeeperSave(modelLoc, VAO, jerseyVAO, headVAO, modelTemp,
                                place, goalkeeperYaw, texGoalkeeperJersey, matchHead,
                                texShorts, texSkin, texShoes, texWhite, texWhite,
                                1, progress, 1);
                            continue;
                        }
                    }
                    drawPlayer(modelLoc, VAO, jerseyVAO, headVAO, modelTemp,
                        place, yaw,
                        team == 0 ? texAtlante : texPumas,
                        matchHead,
                        team == 0 ? texRed : texShorts, texSkin, texShoes,
                        kick, dive, walk, player + 1, texWhite, jump, nod);
                }
            }
        }
        if (matchScene == SCENE_KICKOFF || matchScene == SCENE_PUMAS_GOAL ||
            matchScene == SCENE_ATLANTE_GOAL || matchScene == SCENE_PENALTIES ||
            matchScene == SCENE_FINAL) {
            glm::vec3 refereePosition = matchScene == SCENE_KICKOFF ? glm::vec3(0.0f, 0.0f, -23.0f) :
                matchScene == SCENE_PUMAS_GOAL ? glm::vec3(26.0f, 0.0f, 23.0f) :
                matchScene == SCENE_ATLANTE_GOAL ? glm::vec3(-8.0f, 0.0f, 23.0f) :
                matchScene == SCENE_PENALTIES ? glm::vec3(33.0f, 0.0f, 16.0f) :
                glm::vec3(-2.0f, 0.0f, 30.0f);
            drawPlayer(modelLoc, VAO, jerseyVAO, headVAO, modelTemp,
                refereePosition, 180.0f, texWhite, texHead, texBlack, texSkin, texShoes);
        }
        if (ballVisible)
            drawBall(modelLoc, textureModeLoc, modelTemp, whiteMesh, blackMesh,
                texBallWhite, texBallBlack, rotBall, ballPosition);
        drawBleachers(modelLoc, textureModeLoc, VAO, modelTemp, texGravelConcrete, texFacade,
            texWood, texWalkway, texFloodlight);
        drawCrowd(modelLoc, VAO, jerseyVAO, headVAO, modelTemp,
            texPumas, texAtlante, texHead, crowdHeads, texShorts, texRed,
            texHeadJoy, texHeadAngry, texHeadSad, texHeadSurprise);
        drawTrees(modelLoc, textureModeLoc, VAO, modelTemp, whiteMesh, blackMesh,
            texBark, texLeavesDark, texLeavesLight);
        drawTrophy(modelLoc, textureModeLoc, VAO, modelTemp, whiteMesh, blackMesh,
            animationClock, texGold, texMarble, texDarkWood);
        for (int i = 0; i < 2; ++i) {
            if (activeCamera == i + 1) continue;
            drawDrone(modelLoc, textureModeLoc, VAO, modelTemp, whiteMesh, blackMesh,
                broadcastCameraPositions[i], trackedBallPosition, animationClock,
                texDroneBody, texDroneRotor, texDroneArm, texDroneLens);
        }
        drawFloodlightTowers(modelLoc, textureModeLoc, VAO, modelTemp, texTowerMetal);
        glUniform1i(textureModeLoc, 0);
        glUniform1i(importedTextureLoc, 1);
        drawBroadcastCameraModels(modelLoc, lightingShader,
            broadcastCameraBase.get(), broadcastCameraHead.get());
        glUniform1i(importedTextureLoc, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texSpecular);
        glActiveTexture(GL_TEXTURE0);
        drawBird(modelLoc, textureModeLoc, VAO, modelTemp, whiteMesh, blackMesh,
            texBird, texBeak, animationClock);
        if (fireworksActive) {
            float elapsed = animationClock - fireworksStartTime;
            if (elapsed < 4.8f)
                drawFireworks(modelLoc, VAO, modelTemp, elapsed,
                    texGold, texBlue, texRed, texWhite);
            else
                fireworksActive = false;
        }
        if (busAssetsReady) {
            glUniform1i(textureModeLoc, 0);
            glUniform1i(importedTextureLoc, 1);
            for (int team = 0; team < 2; ++team) {
                const TeamBus& bus = teamBuses[team];
                glm::mat4 busMatrix = getBusModelMatrix(bus);
                glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(busMatrix));
                busBodyModels[team]->Draw(lightingShader);
                for (int i = 0; i < 6; ++i) {
                    const glm::vec3 wheelCenter = busWheelData[i].position;
                    glm::mat4 wheelMatrix = glm::translate(busMatrix, wheelCenter);
                    if (busWheelData[i].isFrontWheel)
                        wheelMatrix = glm::rotate(wheelMatrix, bus.steeringAngle,
                            glm::vec3(0.0f, 1.0f, 0.0f));
                    float rollingAngle = -bus.distanceTraveled / (busModelScale * busWheelData[i].radius);
                    wheelMatrix = glm::rotate(wheelMatrix, rollingAngle,
                        glm::vec3(1.0f, 0.0f, 0.0f));
                    wheelMatrix = glm::translate(wheelMatrix, -wheelCenter);
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(wheelMatrix));
                    busWheelModels[i]->Draw(lightingShader);
                }
            }
            glUniform1i(importedTextureLoc, 0);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, texSpecular);
            glActiveTexture(GL_TEXTURE0);
        }

        glUniform1i(textureModeLoc, 0);
        glUniform1i(importedTextureLoc, 0);
        drawMatchScoreboard(modelLoc, VAO, modelTemp, whiteMesh, blackMesh,
            texBlack, texWhite, texGold, texRed, texPenaltyGreen, texFloodlight);

        lampShader.Use();
        GLint lampModelLoc = glGetUniformLocation(lampShader.Program, "model");
        GLint lampViewLoc = glGetUniformLocation(lampShader.Program, "view");
        GLint lampProjectionLoc = glGetUniformLocation(lampShader.Program, "projection");
        glUniform1f(glGetUniformLocation(lampShader.Program, "brightness"),
            std::min(1.0f, 0.20f + 0.45f * stadiumLightIntensity));
        glUniformMatrix4fv(lampViewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(lampProjectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        drawFloodlightPanels(lampModelLoc, VAO);
        if (busAssetsReady && busHeadlightsOn) {
            glBindVertexArray(VAO);
            for (int team = 0; team < 2; ++team) {
                glm::mat4 busMatrix = getBusModelMatrix(teamBuses[team]);
                for (int lamp = 0; lamp < 2; ++lamp) {
                    glm::mat4 lightMatrix = glm::translate(busMatrix,
                        glm::vec3(lamp == 0 ? -0.14f : 0.87f, 1.95f, 6.75f));
                    lightMatrix = glm::scale(lightMatrix, glm::vec3(0.19f, 0.14f, 0.08f));
                    glUniformMatrix4fv(lampModelLoc, 1, GL_FALSE, glm::value_ptr(lightMatrix));
                    glDrawArrays(GL_TRIANGLES, 0, 36);
                }
            }
            glBindVertexArray(0);
        }
        glBindVertexArray(0);

        drawFadeOverlay(fadeProgram, fadeVAO);
        glfwSwapBuffers(window);
    }

    deleteBallMesh(whiteMesh);
    deleteBallMesh(blackMesh);
    deleteBallMesh(runningTrackMesh);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &jerseyVAO);
    glDeleteBuffers(1, &jerseyVBO);
    glDeleteVertexArrays(1, &fadeVAO);
    glDeleteBuffers(1, &fadeVBO);
    glDeleteProgram(fadeProgram);
    glDeleteProgram(skyShader.Program);
    glDeleteTextures(4, crowdHeads);
    GLuint textures[] = { texGrass, texWhite, texBlack, texBlue, texPumas, texRed, texPenaltyGreen, texAtlante, texRefereeJersey,
                         texGoalkeeperJersey, texBallWhite, texBallBlack,
                         texHeadJoy, texHeadAngry, texHeadSad, texHeadSurprise,
                         texDroneBody, texDroneArm, texDroneRotor, texDroneLens,
                         texShorts, texSkin, texShoes, texPosts, texNet,
                         texSpecular, texBark, texLeavesDark,
                         texLeavesLight, texBird, texBeak, texGold, texMarble, texDarkWood, texFloodlight, texTowerMetal, texConcrete, texWalkway, texRunningTrack, texWood, texAsphalt, texFacade, texGravelConcrete };
    glDeleteTextures(sizeof(textures) / sizeof(textures[0]), textures);
    for (int i = 0; i < 2; ++i) busBodyModels[i].reset();
    for (int i = 0; i < 6; ++i) busWheelModels[i].reset();
    broadcastCameraBase.reset();
    broadcastCameraHead.reset();
    glfwTerminate();

    return 0;
}

// Actualiza el movimiento de la camara.
void DoMovement()
{
    if (activeCamera == 1 || activeCamera == 2) {
        glm::vec3& dronePosition = broadcastCameraPositions[activeCamera - 1];
        glm::vec3 horizontal = trackedBallPosition - dronePosition;
        horizontal.y = 0.0f;
        if (glm::dot(horizontal, horizontal) < 0.001f)
            horizontal = glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 forward = glm::normalize(horizontal);
        glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
        float velocity = 11.0f * std::min(deltaTime, 0.1f);
        if (keys[GLFW_KEY_LEFT_SHIFT] || keys[GLFW_KEY_RIGHT_SHIFT])
            velocity *= 2.0f;
        if (keys[GLFW_KEY_W]) dronePosition += forward * velocity;
        if (keys[GLFW_KEY_S]) dronePosition -= forward * velocity;
        if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT]) dronePosition -= right * velocity;
        if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) dronePosition += right * velocity;
        if (keys[GLFW_KEY_Q]) dronePosition.y -= velocity;
        if (keys[GLFW_KEY_E]) dronePosition.y += velocity;
        dronePosition.x = glm::clamp(dronePosition.x, -64.0f, 64.0f);
        dronePosition.y = glm::clamp(dronePosition.y, 7.0f, 34.0f);
        dronePosition.z = glm::clamp(dronePosition.z, -47.0f, 47.0f);
        return;
    }
    if (activeCamera != 0)
        return;

    glm::vec3 forward = camera.GetFront();
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
    float step = 7.5f * std::min(deltaTime, 0.1f);
    if (keys[GLFW_KEY_W]) freeCameraPosition += forward * step;
    if (keys[GLFW_KEY_S]) freeCameraPosition -= forward * step;
    if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT]) freeCameraPosition -= right * step;
    if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) freeCameraPosition += right * step;
    freeCameraPosition.x = glm::clamp(freeCameraPosition.x, -98.0f, 98.0f);
    freeCameraPosition.y = glm::clamp(freeCameraPosition.y, 1.0f, 32.0f);
    freeCameraPosition.z = glm::clamp(freeCameraPosition.z, -75.0f, 75.0f);

}

// Procesa las teclas presionadas.
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }

    if (action != GLFW_PRESS)
        return;

    if (key >= GLFW_KEY_1 && key <= GLFW_KEY_9) {
        activeCamera = key - GLFW_KEY_1 + 1;
    }
    if (key == GLFW_KEY_0) {
        activeCamera = 0;
        firstMouse = true;
    }
    if (key == GLFW_KEY_LEFT_BRACKET)
        cameraModelForwardYawDegrees -= 90.0f;
    if (key == GLFW_KEY_RIGHT_BRACKET)
        cameraModelForwardYawDegrees += 90.0f;
    if (key == GLFW_KEY_UP)
        stadiumLightIntensity = std::min(2.5f, stadiumLightIntensity + 0.15f);
    if (key == GLFW_KEY_DOWN)
        stadiumLightIntensity = std::max(0.15f, stadiumLightIntensity - 0.15f);
    if (key == GLFW_KEY_H)
        busHeadlightsOn = !busHeadlightsOn;
    if (key == GLFW_KEY_R) {
        resetMatch();
        return;
    }
    if (key == GLFW_KEY_SPACE) {
        if (matchScene == SCENE_IDLE) {
            resetMatch();
            autoSequence = true;
            beginMatchScene(SCENE_ARRIVAL, true);
        }
        else {
            sequencePaused = !sequencePaused;
        }
        return;
    }
    if (key == GLFW_KEY_L) {
        resetMatch();
        autoSequence = true;
        beginMatchScene(SCENE_ARRIVAL, true);
        return;
    }
    MatchScene chosen = SCENE_IDLE;
    if (key == GLFW_KEY_Z) chosen = SCENE_ARRIVAL;
    if (key == GLFW_KEY_X) chosen = SCENE_KICKOFF;
    if (key == GLFW_KEY_C) chosen = SCENE_PUMAS_GOAL;
    if (key == GLFW_KEY_V) chosen = SCENE_ATLANTE_GOAL;
    if (key == GLFW_KEY_N) {
        chosen = SCENE_PENALTIES;
    }
    if (key == GLFW_KEY_M) chosen = SCENE_FINAL;
    if (chosen != SCENE_IDLE) {
        autoSequence = false;
        sequencePaused = false;
        requestMatchScene(static_cast<int>(chosen));
        return;
    }
    if (key == GLFW_KEY_P && matchScene == SCENE_PENALTIES && fadePhase == FADE_NONE) {
        autoSequence = false;
        if (penaltyWinner < 0) {
            if (penaltyState == READY) penaltyRequested = true;
        }
        return;
    }
    if (key == GLFW_KEY_F) {
        fireworksActive = true;
        fireworksStartTime = animationClock;
    }
}

// Reinicia los datos del partido.
void resetMatch()
{
    matchScene = SCENE_IDLE;
    nextScene = SCENE_IDLE;
    fadePhase = FADE_NONE;
    autoSequence = false;
    sequencePaused = false;
    fadeAlpha = 0.0f;
    sceneTime = 0.0f;
    pumasScore = atlanteScore = 0;
    matchMinute = matchSecond = 0;
    penaltyTaken[0] = penaltyTaken[1] = 0;
    penaltyGoals[0] = penaltyGoals[1] = 0;
    for (int team = 0; team < 2; ++team)
        for (int attempt = 0; attempt < penaltyHistoryLimit; ++attempt)
            penaltyHistory[team][attempt] = -1;
    penaltyKickerTeam = 0;
    penaltyWinner = -1;
    penaltyNextKickerTeam = -1;
    penaltyShotRecorded = false;
    penaltyRestTime = 0.0f;
    arrivalStarted = false;
    fireworksActive = false;
    for (int team = 0; team < 2; ++team) {
        float sign = team == 0 ? 1.0f : -1.0f;
        TeamBus& bus = teamBuses[team];
        bus.position = glm::vec3(sign * 102.0f, bus.position.y, -sign * 21.0f);
        bus.heading = -sign * busPi * 0.5f;
        bus.distanceTraveled = 0.0f;
        bus.steeringAngle = 0.0f;
        bus.moving = false;
        bus.state = BUS_WAITING;
        teamLineupTime[team] = 0.0f;
    }
    ResetPenalty();
    goals = saves = misses = 0;
}

// Prepara una nueva escena del partido.
void beginMatchScene(int next, bool automatic)
{
    matchScene = static_cast<MatchScene>(next);
    autoSequence = automatic;
    sceneTime = 0.0f;
    fadePhase = FADE_IN;
    fadeAlpha = 1.0f;
    if (matchScene == SCENE_ARRIVAL) {
        startArrival();
    }
    if (matchScene == SCENE_KICKOFF) {
        ballPosition = glm::vec3(0.0f, 0.11f, 0.0f);
        matchMinute = matchSecond = 0;
    }
    if (matchScene == SCENE_PUMAS_GOAL) {
        pumasScore = 0;
        atlanteScore = 0;
        matchMinute = 43;
        matchSecond = 0;
        ballPosition = glm::vec3(50.8f, 0.11f, -33.0f);
    }
    if (matchScene == SCENE_ATLANTE_GOAL) {
        pumasScore = 1;
        atlanteScore = 0;
        matchMinute = 87;
        matchSecond = 0;
        ballPosition = glm::vec3(-28.0f, 0.11f, 0.0f);
    }
    if (matchScene == SCENE_PENALTIES) {
        pumasScore = 1;
        atlanteScore = 1;
        matchMinute = 90;
        matchSecond = 0;
        penaltyTaken[0] = penaltyTaken[1] = 0;
        penaltyGoals[0] = penaltyGoals[1] = 0;
        for (int team = 0; team < 2; ++team)
            for (int attempt = 0; attempt < penaltyHistoryLimit; ++attempt)
                penaltyHistory[team][attempt] = -1;
        penaltyKickerTeam = 0;
        penaltyWinner = -1;
        penaltyNextKickerTeam = -1;
        penaltyShotRecorded = false;
        penaltyRestTime = 0.0f;
        ResetPenalty();
        goals = saves = misses = 0;
    }
    if (matchScene == SCENE_FINAL) {
        matchMinute = 90;
        matchSecond = 0;
        fireworksActive = true;
        fireworksStartTime = animationClock;
    }
}

// Solicita cambiar a otra escena.
void requestMatchScene(int next)
{
    if (matchScene == SCENE_IDLE && fadePhase == FADE_NONE) {
        beginMatchScene(next, autoSequence);
        return;
    }
    nextScene = static_cast<MatchScene>(next);
    fadePhase = FADE_OUT;
}

// Actualiza la transicion entre escenas.
void updateFade(float step)
{
    if (fadePhase == FADE_OUT) {
        fadeAlpha = std::min(1.0f, fadeAlpha + step / transitionDuration);
        if (fadeAlpha >= 1.0f) {
            int following = static_cast<int>(nextScene);
            beginMatchScene(following, autoSequence);
            fadeAlpha = 1.0f;
        }
    }
    else if (fadePhase == FADE_IN) {
        fadeAlpha = std::max(0.0f, fadeAlpha - step / transitionDuration);
        if (fadeAlpha <= 0.0f) fadePhase = FADE_NONE;
    }
}

// Actualiza el tiempo, marcador y jugadas del partido.
void updateMatch(float step)
{
    if (matchScene == SCENE_IDLE || fadePhase == FADE_OUT) return;
    if (fadePhase == FADE_IN) return;
    sceneTime += step;
    if (matchScene == SCENE_ARRIVAL) {
        if (autoSequence && isLineupComplete()) requestMatchScene(SCENE_KICKOFF);
        return;
    }
    if (matchScene == SCENE_KICKOFF) {
        float t = sceneTime;
        if (t < 1.0f)
            ballPosition = glm::vec3(0.0f, 0.11f, 0.0f);
        else if (t < 2.3f) {
            float a = glm::clamp((t - 1.0f) / 1.3f, 0.0f, 1.0f);
            ballPosition = glm::mix(glm::vec3(0.0f, 0.11f, 0.0f), glm::vec3(-7.0f, 0.11f, 6.0f), a);
        }
        else if (t < 4.6f) {
            float a = glm::clamp((t - 2.3f) / 2.3f, 0.0f, 1.0f);
            ballPosition = glm::mix(glm::vec3(-7.0f, 0.11f, 6.0f), glm::vec3(-18.0f, 0.11f, 12.0f), a);
        }
        matchMinute = 0;
        matchSecond = std::min(59, static_cast<int>(sceneTime * 6.0f));
        if (autoSequence && sceneTime >= 6.2f) requestMatchScene(SCENE_PUMAS_GOAL);
        return;
    }
    if (matchScene == SCENE_PUMAS_GOAL) {
        float t = sceneTime;
        const glm::vec3 start(50.8f, 0.11f, -33.0f);
        const glm::vec3 firstHeader(43.0f, 2.05f, -5.0f);
        const glm::vec3 winningHeader(46.5f, 2.13f, -0.6f);
        const glm::vec3 net(53.4f, 1.35f, 0.65f);
        if (t < pumasCornerKickTime) {
            ballPosition = start;
        }
        else if (t < 2.7f) {
            float a = glm::clamp((t - pumasCornerKickTime) / (2.7f - pumasCornerKickTime), 0.0f, 1.0f);
            ballPosition = glm::mix(start, firstHeader, a) + glm::vec3(0.0f, 2.0f * sinf(busPi * a), 0.0f);
        }
        else if (t < pumasShotTime) {
            float a = glm::clamp((t - 2.7f) / (pumasShotTime - 2.7f), 0.0f, 1.0f);
            ballPosition = glm::mix(firstHeader, winningHeader, a) + glm::vec3(0.0f, 0.65f * sinf(busPi * a), 0.0f);
        }
        else {
            float a = glm::clamp((t - pumasShotTime) / (5.35f - pumasShotTime), 0.0f, 1.0f);
            ballPosition = glm::mix(winningHeader, net, a);
        }
        matchSecond = std::min(59, static_cast<int>(t));
        if (t >= 5.35f) pumasScore = 1;
        if (autoSequence && t >= 8.3f) requestMatchScene(SCENE_ATLANTE_GOAL);
        return;
    }
    if (matchScene == SCENE_ATLANTE_GOAL) {
        float a = glm::clamp((sceneTime - 1.3f) / 2.5f, 0.0f, 1.0f);
        glm::vec3 from(-28.0f, 0.11f, 0.0f), to(-53.0f, 2.0f, -3.1f);
        ballPosition = glm::mix(from, to, a) + glm::vec3(0, 4.8f * sinf(busPi * a), -2.5f * sinf(busPi * a));
        matchSecond = std::min(59, static_cast<int>(sceneTime));
        if (sceneTime >= 4.1f) atlanteScore = 1;
        if (autoSequence && sceneTime >= 6.5f) requestMatchScene(SCENE_PENALTIES);
        return;
    }
    if (matchScene == SCENE_PENALTIES) {
        matchMinute = 90;
        if (penaltyState == FINISHED && keeperCommitted)
            keeperDiveProgress = glm::clamp(keeperDiveProgress + step / 0.35f, 0.0f, 1.0f);
        if (penaltyState == FINISHED && !penaltyShotRecorded) {
            if (penaltyTaken[penaltyKickerTeam] >= penaltyHistoryLimit) {
                std::cerr << "Penalty history limit reached. No winner was assigned." << std::endl;
                penaltyShotRecorded = true;
                penaltyRestTime = 0.0f;
                autoSequence = false;
                return;
            }
            penaltyHistory[penaltyKickerTeam][penaltyTaken[penaltyKickerTeam]] =
                penaltyResult == "GOAL" ? 1 : 0;
            ++penaltyTaken[penaltyKickerTeam];
            penaltyGoals[0] = countPenaltyGoals(0);
            penaltyGoals[1] = countPenaltyGoals(1);
            penaltyShotRecorded = true;
            penaltyRestTime = 0.0f;
            penaltyNextKickerTeam = 1 - penaltyKickerTeam;
            penaltyWinner = decidePenaltyWinner(penaltyTaken, penaltyGoals);
            if (penaltyWinner >= 0) {
                std::cout << "Winner: " << (penaltyWinner == 0 ? "Pumas" : "Atlante")
                    << " | Penalties " << penaltyGoals[0] << "-" << penaltyGoals[1]
                    << " | Attempts " << penaltyTaken[0] << "-" << penaltyTaken[1] << std::endl;
            }
        }
        if (penaltyShotRecorded) {
            penaltyRestTime += step;
            if (penaltyWinner >= 0) {
                if (autoSequence && penaltyRestTime >= 2.3f && fadePhase == FADE_NONE)
                    requestMatchScene(SCENE_FINAL);
            }
            else if (penaltyRestTime >= 1.55f) {
                penaltyKickerTeam = penaltyNextKickerTeam;
                ResetPenalty();
                penaltyShotRecorded = false;
                penaltyRestTime = 0.0f;
                if (autoSequence) penaltyRequested = true;
            }
        }
        if (autoSequence && penaltyWinner < 0 && !penaltyShotRecorded && penaltyState == READY && sceneTime >= 1.3f) {
            penaltyRequested = true;
        }
        return;
    }
}

// Genera un numero aleatorio dentro de un rango.
float RandomFloat(float minValue, float maxValue)
{
    return minValue + (maxValue - minValue) *
        (static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
}

// Reinicia el estado de un penal.
void ResetPenalty()
{
    penaltyState = READY;
    stateTime = 0.0f;
    rotBall = 0.0f;
    kickAngle = 0.0f;
    keeperDive = 0.0f;
    keeperDiveProgress = 0.0f;
    keeperPositionZ = 0.0f;
    targetZ = 0.0f;
    targetY = 0.9f;
    shotSide = 0;
    keeperSide = 0;
    keeperCommitted = false;
    penaltyRequested = false;
    ballPosition = glm::vec3(41.5f, 0.11f, 0.0f);
    penaltyResult = "READY";
}

// Actualiza la animacion del tiro penal.
void Animation()
{
    if (penaltyRequested) {
        ResetPenalty();
        penaltyState = PREPARING;
        penaltyResult = "PREPARING";
        targetZ = RandomFloat(-4.0f, 4.0f);
        targetY = RandomFloat(0.35f, 2.15f);
        shotSide = (targetZ < -1.1f) ? -1 : ((targetZ > 1.1f) ? 1 : 0);
        penaltyRequested = false;
    }

    if (penaltyState == READY || penaltyState == FINISHED)
        return;

    stateTime += deltaTime;

    if (penaltyState == PREPARING) {
        float t = std::min(stateTime / 0.45f, 1.0f);
        kickAngle = -35.0f * t;
        if (t >= 1.0f) {
            penaltyState = KICKING;
            stateTime = 0.0f;
        }
    }
    else if (penaltyState == KICKING) {
        float t = std::min(stateTime / 0.24f, 1.0f);
        kickAngle = -35.0f + 100.0f * t;
        if (t >= 1.0f) {
            penaltyState = FLYING;
            penaltyResult = "BALL IN FLIGHT";
            stateTime = 0.0f;
        }
    }
    else if (penaltyState == FLYING) {
        float t = std::min(stateTime / 1.1f, 1.0f);
        float travel = std::min(t * 11.9f, 11.9f);
        ballPosition.x = 41.5f + travel;
        ballPosition.z = targetZ * t;
        ballPosition.y = 0.11f + (targetY - 0.11f) * t +
            0.60f * sinf(3.1415926535f * t);
        rotBall += 600.0f * deltaTime;
        kickAngle = 65.0f * (1.0f - t);

        if (!keeperCommitted && stateTime >= 0.15f) {
            keeperSide = ((rand() % 100) < 45) ? shotSide : (rand() % 3 - 1);
            keeperCommitted = true;
        }
        if (keeperCommitted) {
            float reaction = std::max(0.0f, std::min((stateTime - 0.15f) / 0.62f, 1.0f));
            keeperPositionZ = keeperSide * 2.0f * reaction;
            keeperDive = keeperSide * 55.0f * reaction;
            keeperDiveProgress = reaction;
        }

        if (ballPosition.x >= 51.2f && t < 1.0f) {
            bool withinReach = std::fabs(ballPosition.z - keeperPositionZ) <= 1.25f &&
                ballPosition.y <= 2.20f;
            if (withinReach) {
                penaltyState = FINISHED;
                penaltyResult = "SAVED";
                ++saves;
                std::cout << "Keeper saved the penalty! Shot side: " << shotSide
                    << ", goalkeeper side: " << keeperSide << std::endl;
                return;
            }
        }

        if (t >= 1.0f) {
            penaltyState = FINISHED;
            if (std::fabs(targetZ) <= 3.55f && targetY <= 2.33f) {
                penaltyResult = "GOAL";
                ++goals;
            }
            else {
                penaltyResult = "MISS";
                ++misses;
            }
            std::cout << penaltyResult << "! Shot side: " << shotSide
                << ", goalkeeper side: " << keeperSide << std::endl;
        }
    }
}

// Controla el movimiento de la camara con el mouse.
void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
    if (activeCamera != 0) {
        firstMouse = true;
        return;
    }
    if (firstMouse)
    {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }

    GLfloat xOffset = xPos - lastX;
    GLfloat yOffset = lastY - yPos;

    lastX = xPos;
    lastY = yPos;

    camera.ProcessMouseMovement(xOffset, yOffset);
}

// Crea una textura de un solo color.
GLuint CreateSolidColorTexture(GLubyte r, GLubyte g, GLubyte b)
{
    GLuint texture;
    GLubyte pixel[] = { r, g, b };
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, pixel);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

// Carga una textura desde un archivo.
GLuint LoadDiffuseTexture(const char* path)
{
    int width, height;
    unsigned char* image = SOIL_load_image(path, &width, &height, 0, SOIL_LOAD_RGB);
    if (!image) {
        std::cerr << "Texture not loaded: " << path << " (" << SOIL_last_result() << ")" << std::endl;
        return 0;
    }
    GLuint texture;
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

// Comprueba las texturas asociadas a un modelo.
void VerifyModelTextures(const std::string& modelPath)
{
    std::ifstream objFile(modelPath.c_str());
    if (!objFile) {
        std::cerr << "OBJ missing: " << modelPath << std::endl;
        return;
    }
    const std::string::size_type slash = modelPath.find_last_of("/\\");
    const std::string folder = slash == std::string::npos ? "" : modelPath.substr(0, slash + 1);
    std::string line;
    bool foundUV = false;
    bool foundMaterials = false;
    while (std::getline(objFile, line)) {
        if (line.compare(0, 3, "vt ") == 0) foundUV = true;
        if (line.compare(0, 7, "mtllib ") != 0) continue;
        foundMaterials = true;
        std::string mtlName = line.substr(7);
        if (!mtlName.empty() && mtlName.back() == '\r') mtlName.pop_back();
        for (char& c : mtlName) if (c == '\\') c = '/';
        std::ifstream mtlFile((folder + mtlName).c_str());
        if (!mtlFile) {
            std::cerr << "MTL missing: " << folder + mtlName << std::endl;
            continue;
        }
        std::string mtlLine;
        while (std::getline(mtlFile, mtlLine)) {
            if (mtlLine.compare(0, 7, "map_Kd ") != 0) continue;
            std::string image = mtlLine.substr(7);
            while (!image.empty() && (image.back() == '\r' || image.back() == ' ')) image.pop_back();
            while (!image.empty() && image.front() == ' ') image.erase(image.begin());
            for (char& c : image) if (c == '\\') c = '/';
            if (!image.empty() && image.front() == '"' && image.back() == '"')
                image = image.substr(1, image.size() - 2);
            if (image.empty()) continue;
            std::ifstream imageFile((folder + image).c_str(), std::ios::binary);
            if (!imageFile) std::cerr << "Texture missing for " << modelPath << ": " << folder + image << std::endl;
        }
    }
    if (!foundMaterials) std::cerr << "OBJ without MTL reference: " << modelPath << std::endl;
    if (!foundUV) std::cerr << "OBJ without UV coordinates (vt): " << modelPath << std::endl;
}

// Dibuja un cubo con posicion, tamano y textura.
void drawCube(GLint modelLoc, GLuint VAO, const glm::mat4& parent,
    glm::vec3 position, glm::vec3 size, GLuint texture, float yaw)
{
    glm::mat4 model = glm::translate(parent, position);
    model = glm::rotate(model, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, size);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// Dibuja la cancha y sus lineas.
void drawField(GLint modelLoc, GLint textureModeLoc, GLuint VAO, const glm::mat4& scene,
    GLuint texGrass, GLuint texWhite)
{
    // Superficie del terreno en Y = 0.
    glUniform1i(textureModeLoc, 1);
    drawCube(modelLoc, VAO, scene, glm::vec3(0, -0.04f, 0),
        glm::vec3(105, 0.08f, 68), texGrass);
    glUniform1i(textureModeLoc, 0);

    for (int side = -1; side <= 1; side += 2) {
        drawCube(modelLoc, VAO, scene, glm::vec3(0, 0.012f, side * 34.0f),
            glm::vec3(105, 0.024f, 0.12f), texWhite);
        drawCube(modelLoc, VAO, scene, glm::vec3(side * 52.5f, 0.012f, 0),
            glm::vec3(0.12f, 0.024f, 68), texWhite);
    }
    drawCube(modelLoc, VAO, scene, glm::vec3(0, 0.012f, 0),
        glm::vec3(0.12f, 0.024f, 68), texWhite);

    const float PI = 3.14159265358979323846f;
    const int segments = 48;
    const float radius = 9.15f;
    for (int i = 0; i < segments; ++i) {
        float a = 2.0f * PI * i / segments;
        float b = 2.0f * PI * (i + 1) / segments;
        float m = (a + b) * 0.5f;
        drawCube(modelLoc, VAO, scene,
            glm::vec3(radius * cosf(m), 0.018f, radius * sinf(m)),
            glm::vec3(2.0f * radius * sinf((b - a) / 2.0f) + 0.02f,
                0.012f, 0.12f),
            texWhite, glm::degrees(-m - PI * 0.5f));
    }
    drawCube(modelLoc, VAO, scene, glm::vec3(0, 0.02f, 0),
        glm::vec3(0.3f, 0.03f, 0.3f), texWhite);

    for (int side = -1; side <= 1; side += 2) {
        const float penaltyX = side * 36.0f;
        const float goalAreaX = side * 47.0f;
        drawCube(modelLoc, VAO, scene, glm::vec3(penaltyX, 0.018f, 0),
            glm::vec3(0.12f, 0.02f, 40.32f), texWhite);
        drawCube(modelLoc, VAO, scene, glm::vec3(side * 44.25f, 0.018f, -20.16f),
            glm::vec3(16.5f, 0.02f, 0.12f), texWhite);
        drawCube(modelLoc, VAO, scene, glm::vec3(side * 44.25f, 0.018f, 20.16f),
            glm::vec3(16.5f, 0.02f, 0.12f), texWhite);
        drawCube(modelLoc, VAO, scene, glm::vec3(goalAreaX, 0.018f, 0),
            glm::vec3(0.12f, 0.02f, 18.32f), texWhite);
        drawCube(modelLoc, VAO, scene, glm::vec3(side * 49.75f, 0.018f, -9.16f),
            glm::vec3(5.5f, 0.02f, 0.12f), texWhite);
        drawCube(modelLoc, VAO, scene, glm::vec3(side * 49.75f, 0.018f, 9.16f),
            glm::vec3(5.5f, 0.02f, 0.12f), texWhite);
        drawCube(modelLoc, VAO, scene, glm::vec3(side * 41.5f, 0.018f, 0),
            glm::vec3(0.3f, 0.03f, 0.3f), texWhite);
    }
}

// Genera la geometria de la pista alrededor del campo.
void createRunningTrack(BallMesh& mesh)
{
    constexpr float halfLength = 52.5f, halfWidth = 34.0f;
    constexpr float innerRadius = 0.25f, trackWidth = 14.25f;
    constexpr int curveSteps = 12;
    const float pi = 3.14159265358979323846f;
    static_assert(halfLength + innerRadius + trackWidth < 103.0f &&
        halfWidth + innerRadius + trackWidth < 49.45f && innerRadius > 0.12f,
        "Track must clear the walls, stands and pitch markings");
    struct Sample { glm::vec2 inner, outward; float distance; };
    std::vector<Sample> path;
    float distance = 0.0f;
    auto addSample = [&](glm::vec2 inner, glm::vec2 outward, float length) {
        distance += length;
        path.push_back({ inner, outward, distance });
        };
    addSample(glm::vec2(-halfLength, -halfWidth - innerRadius), glm::vec2(0, -1), 0.0f);
    addSample(glm::vec2(halfLength, -halfWidth - innerRadius), glm::vec2(0, -1), 2.0f * halfLength);
    for (int corner = 0; corner < 4; ++corner) {
        glm::vec2 center(corner < 2 ? halfLength : -halfLength,
            corner == 0 || corner == 3 ? -halfWidth : halfWidth);
        for (int step = 1; step <= curveSteps; ++step) {
            float angle = -pi * 0.5f + (corner + static_cast<float>(step) / curveSteps) * pi * 0.5f;
            glm::vec2 normal(cosf(angle), sinf(angle));
            addSample(center + innerRadius * normal, normal, pi * innerRadius / (2.0f * curveSteps));
        }
        if (corner == 0)
            addSample(glm::vec2(halfLength + innerRadius, halfWidth), glm::vec2(1, 0), 2.0f * halfWidth);
        if (corner == 1)
            addSample(glm::vec2(-halfLength, halfWidth + innerRadius), glm::vec2(0, 1), 2.0f * halfLength);
        if (corner == 2)
            addSample(glm::vec2(-halfLength - innerRadius, -halfWidth), glm::vec2(-1, 0), 2.0f * halfWidth);
    }

    std::vector<GLfloat> vertices;
    auto vertex = [&vertices](glm::vec2 point, float distance, float across) {
        vertices.insert(vertices.end(), { point.x, 0.015f, point.y,
            0.0f, 1.0f, 0.0f, distance / 4.0f, across / 4.0f });
        };
    for (size_t i = 1; i < path.size(); ++i) {
        const Sample& a = path[i - 1];
        const Sample& b = path[i];
        vertex(a.inner, a.distance, 0.0f);
        vertex(b.inner, b.distance, 0.0f);
        vertex(a.inner + trackWidth * a.outward, a.distance, trackWidth);
        vertex(b.inner, b.distance, 0.0f);
        vertex(b.inner + trackWidth * b.outward, b.distance, trackWidth);
        vertex(a.inner + trackWidth * a.outward, a.distance, trackWidth);
    }
    mesh.vertexCount = static_cast<GLsizei>(vertices.size() / 8);
    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glBindVertexArray(mesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

// Dibuja la pista del estadio.
void drawRunningTrack(GLint modelLoc, const glm::mat4& scene, const BallMesh& mesh, GLuint texture)
{
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(scene));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(mesh.VAO);
    glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);
    glBindVertexArray(0);
}

// Dibuja los limites y alrededores del estadio.
void drawPerimeter(GLint modelLoc, GLint textureModeLoc, GLuint VAO,
    const glm::mat4& scene, GLuint texWall, GLuint texFacade, GLuint texFloor, GLuint texAccent)
{
    constexpr float halfLength = 103.0f, halfWidth = 80.0f;
    static_assert(halfLength > 85.0f && halfWidth > 71.2f,
        "El perimetro debe contener el autobus y los arboles");
    glUniform1i(textureModeLoc, 4);
    drawCube(modelLoc, VAO, scene, glm::vec3(0.0f, -0.22f, 0.0f),
        glm::vec3(2.0f * halfLength, 0.22f, 2.0f * halfWidth), texFloor);
    glUniform1i(textureModeLoc, 0);

    glUniform1i(textureModeLoc, 5);
    for (int side = -1; side <= 1; side += 2) {
        float gateCenterZ = -side * 21.0f;
        float gateHalfWidth = 7.0f;
        float lowEnd = gateCenterZ - gateHalfWidth;
        float highBegin = gateCenterZ + gateHalfWidth;
        for (int part = 0; part < 2; ++part) {
            float from = (part == 0) ? -halfWidth : highBegin;
            float to = (part == 0) ? lowEnd : halfWidth;
            float center = (from + to) * 0.5f;
            float width = to - from;
            drawCube(modelLoc, VAO, scene, glm::vec3(side * halfLength, 2.0f, center),
                glm::vec3(0.5f, 4.0f, width), texFacade);
            drawCube(modelLoc, VAO, scene, glm::vec3(side * (halfLength + 0.30f), 3.65f, center),
                glm::vec3(0.10f, 0.22f, width), texAccent);
        }
        for (int edge = -1; edge <= 1; edge += 2)
            drawCube(modelLoc, VAO, scene,
                glm::vec3(side * halfLength, 2.5f, gateCenterZ + edge * gateHalfWidth),
                glm::vec3(0.85f, 5.0f, 0.85f), texWall);
        drawCube(modelLoc, VAO, scene,
            glm::vec3(side * halfLength, 5.1f, gateCenterZ),
            glm::vec3(0.9f, 0.35f, gateHalfWidth * 2.0f + 0.9f), texAccent);

        for (int section = -1; section <= 1; section += 2) {
            float x = section * (halfLength + 2.0f) * 0.5f;
            drawCube(modelLoc, VAO, scene, glm::vec3(x, 2.0f, side * halfWidth),
                glm::vec3(halfLength - 2.0f, 4.0f, 0.5f), texFacade);
            drawCube(modelLoc, VAO, scene, glm::vec3(x, 3.65f, side * (halfWidth + 0.30f)),
                glm::vec3(halfLength - 2.0f, 0.22f, 0.10f), texAccent);
        }
        drawCube(modelLoc, VAO, scene, glm::vec3(0.0f, -0.01f, side * 72.0f),
            glm::vec3(4.0f, 0.08f, 16.0f), texWall);
        for (int jamb = -1; jamb <= 1; jamb += 2)
            drawCube(modelLoc, VAO, scene, glm::vec3(jamb * 2.25f, 1.8f, side * halfWidth),
                glm::vec3(0.5f, 3.6f, 0.7f), texFacade);
        drawCube(modelLoc, VAO, scene, glm::vec3(0.0f, 3.95f, side * halfWidth),
            glm::vec3(5.0f, 0.7f, 0.7f), texFacade);
    }
    glUniform1i(textureModeLoc, 0);
}

// Dibuja una porteria.
void drawGoal(GLint modelLoc, GLint textureModeLoc, GLuint VAO, const glm::mat4& scene,
    float x, float direction, GLuint texPosts, GLuint texNet)
{
    glUniform1i(textureModeLoc, 7);
    glm::mat4 parent = glm::translate(scene, glm::vec3(x, 0, 0));
    // Portería: 7.32 x 2.44 m.
    drawCube(modelLoc, VAO, parent, glm::vec3(0, 1.22f, -3.66f),
        glm::vec3(0.13f, 2.44f, 0.13f), texPosts);
    drawCube(modelLoc, VAO, parent, glm::vec3(0, 1.22f, 3.66f),
        glm::vec3(0.13f, 2.44f, 0.13f), texPosts);
    drawCube(modelLoc, VAO, parent, glm::vec3(0, 2.44f, 0),
        glm::vec3(0.13f, 0.13f, 7.32f), texPosts);
    float depth = direction * 1.75f;
    for (int i = 0; i <= 14; ++i) {
        float z = -3.66f + (7.32f / 14.0f) * i;
        drawCube(modelLoc, VAO, parent, glm::vec3(depth, 1.15f, z),
            glm::vec3(0.014f, 2.3f, 0.014f), texNet);
    }
    for (int i = 0; i <= 6; ++i) {
        float y = 2.3f * i / 6.0f;
        drawCube(modelLoc, VAO, parent, glm::vec3(depth, y, 0),
            glm::vec3(0.014f, 0.014f, 7.32f), texNet);
    }
    for (int i = 0; i <= 14; ++i) {
        float z = -3.66f + (7.32f / 14.0f) * i;
        drawCube(modelLoc, VAO, parent, glm::vec3(depth / 2.0f, 2.4f, z),
            glm::vec3(1.75f, 0.014f, 0.014f), texNet);
    }
    glUniform1i(textureModeLoc, 0);
}

// Dibuja un jugador y sus movimientos.
void drawPlayer(GLint modelLoc, GLuint VAO, GLuint torsoVAO, GLuint headVAO, const glm::mat4& scene,
    glm::vec3 position, float yaw, GLuint texJersey, GLuint texHead,
    GLuint texShorts, GLuint texSkin, GLuint texShoes,
    float kickAngle, float diveAngle, float walkAngle, int jerseyNumber, GLuint numberColor,
    float jumpHeight, float headNod)
{
    // Articulaciones de brazos y piernas.
    glm::mat4 model = glm::translate(scene, position + glm::vec3(0.0f, jumpHeight, 0.0f));
    model = glm::rotate(model, glm::radians(yaw), glm::vec3(0, 1, 0));
    if (std::fabs(diveAngle) > 0.01f) {
        model = glm::translate(model, glm::vec3(0.0f, 0.82f, 0.0f));
        model = glm::rotate(model, glm::radians(diveAngle), glm::vec3(0, 0, 1));
        model = glm::translate(model, glm::vec3(0.0f, -0.82f, 0.0f));
    }
    glm::mat4 modelTemp = model;

    drawCube(modelLoc, torsoVAO, modelTemp, glm::vec3(0, 1.20f, 0),
        glm::vec3(0.45f, 0.64f, 0.25f), texJersey);
    if (jerseyNumber > 0 && numberColor != 0) {
        drawCube(modelLoc, VAO, modelTemp, glm::vec3(0, 1.20f, -0.131f),
            glm::vec3(0.31f, 0.32f, 0.014f), texShorts);
        glm::mat4 digitsFacingOut = glm::scale(modelTemp, glm::vec3(-1.0f, 1.0f, 1.0f));
        float left = jerseyNumber >= 10 ? -0.12f : -0.06f;
        if (jerseyNumber >= 10)
            drawScoreDigit(modelLoc, VAO, digitsFacingOut, jerseyNumber / 10,
                left, 1.10f, -0.146f, 0.11f, numberColor);
        drawScoreDigit(modelLoc, VAO, digitsFacingOut, jerseyNumber % 10,
            jerseyNumber >= 10 ? 0.015f : left, 1.10f, -0.146f,
            0.11f, numberColor);
    }
    drawCube(modelLoc, VAO, modelTemp, glm::vec3(0, 1.55f, 0),
        glm::vec3(0.12f, 0.13f, 0.12f), texSkin);
    glm::mat4 noddingHead = glm::translate(modelTemp, glm::vec3(0.0f, 1.67f, 0.0f));
    noddingHead = glm::rotate(noddingHead, glm::radians(headNod), glm::vec3(1.0f, 0.0f, 0.0f));
    drawCube(modelLoc, headVAO, noddingHead, glm::vec3(0.0f, 0.10f, 0.0f),
        glm::vec3(0.27f, 0.29f, 0.27f), texHead);

    for (int side = -1; side <= 1; side += 2) {
        glm::mat4 shoulder = glm::translate(modelTemp, glm::vec3(side * 0.3f, 1.43f, 0));
        shoulder = glm::rotate(shoulder, glm::radians(-20.0f - side * walkAngle * 0.6f), glm::vec3(1, 0, 0));
        shoulder = glm::rotate(shoulder, glm::radians(side * std::fabs(diveAngle) * 1.1f), glm::vec3(0, 0, 1));
        drawCube(modelLoc, VAO, shoulder, glm::vec3(0, -0.15f, 0),
            glm::vec3(0.14f, 0.3f, 0.14f), texJersey);
        glm::mat4 elbow = glm::translate(shoulder, glm::vec3(0, -0.3f, 0));
        drawCube(modelLoc, VAO, elbow, glm::vec3(0, -0.16f, 0),
            glm::vec3(0.11f, 0.32f, 0.11f), texSkin);
        glm::mat4 hip = glm::translate(modelTemp, glm::vec3(side * 0.13f, 0.90f, 0));
        if (std::fabs(walkAngle) > 0.01f)
            hip = glm::rotate(hip, glm::radians(side * walkAngle), glm::vec3(1, 0, 0));
        else if (side == 1)
            hip = glm::rotate(hip, glm::radians(-kickAngle), glm::vec3(1, 0, 0));
        drawCube(modelLoc, VAO, hip, glm::vec3(0, -0.19f, 0),
            glm::vec3(0.19f, 0.38f, 0.19f), texShorts);
        glm::mat4 knee = glm::translate(hip, glm::vec3(0, -0.38f, 0));
        drawCube(modelLoc, VAO, knee, glm::vec3(0, -0.19f, 0),
            glm::vec3(0.15f, 0.38f, 0.15f), texSkin);
        drawCube(modelLoc, VAO, knee, glm::vec3(0, -0.42f, 0.09f),
            glm::vec3(0.17f, 0.11f, 0.29f), texShoes);
    }
}

// Dibuja la atajada del portero.
void drawGoalkeeperSave(GLint modelLoc, GLuint VAO, GLuint torsoVAO, GLuint headVAO,
    const glm::mat4& scene, glm::vec3 position, float yaw,
    GLuint jersey, GLuint face, GLuint shorts, GLuint skin, GLuint shoes,
    GLuint glove, GLuint numberColor, int number, float progress, int direction)
{
    float p = glm::clamp(progress, 0.0f, 1.0f);
    float armsFirst = smoothMatchStep(p * 2.15f);
    float bodyFall = smoothMatchStep(p);
    float legsFollow = smoothMatchStep((p - 0.16f) / 0.84f);
    float side = direction < 0 ? -1.0f : 1.0f;
    if (direction == 0) { bodyFall = 0.0f; legsFollow = 0.0f; }

    glm::mat4 base = glm::translate(scene, position);
    base = glm::rotate(base, glm::radians(yaw), glm::vec3(0, 1, 0));
    auto leaning = [&](float amount) {
        glm::mat4 m = glm::translate(base, glm::vec3(0.0f, 0.19f, 0.0f));
        m = glm::rotate(m, glm::radians(-side * 86.0f * amount), glm::vec3(0, 0, 1));
        return glm::translate(m, glm::vec3(0.0f, -0.19f, 0.0f));
        };
    glm::mat4 upper = leaning(bodyFall);
    glm::mat4 lower = leaning(legsFollow);

    drawCube(modelLoc, torsoVAO, upper, glm::vec3(0.0f, 1.20f, 0.0f),
        glm::vec3(0.45f, 0.64f, 0.25f), jersey);
    if (number > 0 && numberColor) {
        drawCube(modelLoc, VAO, upper, glm::vec3(0, 1.20f, -0.131f),
            glm::vec3(0.30f, 0.32f, 0.014f), shorts);
        glm::mat4 frontDigits = glm::scale(upper, glm::vec3(-1.0f, 1.0f, 1.0f));
        drawScoreDigit(modelLoc, VAO, frontDigits, number,
            -0.06f, 1.10f, -0.146f, 0.11f, numberColor);
    }
    drawCube(modelLoc, VAO, upper, glm::vec3(0.0f, 1.55f, 0.0f),
        glm::vec3(0.12f, 0.13f, 0.12f), skin);
    drawCube(modelLoc, headVAO, upper, glm::vec3(0.0f, 1.77f, 0.0f),
        glm::vec3(0.27f, 0.29f, 0.27f), face);

    for (int armSide = -1; armSide <= 1; armSide += 2) {
        glm::mat4 shoulder = glm::translate(upper, glm::vec3(armSide * 0.29f, 1.43f, 0.0f));
        shoulder = glm::rotate(shoulder,
            glm::radians(-8.0f - 164.0f * armsFirst), glm::vec3(1, 0, 0));
        shoulder = glm::rotate(shoulder,
            glm::radians(armSide * 11.0f * armsFirst), glm::vec3(0, 0, 1));
        drawCube(modelLoc, VAO, shoulder, glm::vec3(0, -0.15f, 0),
            glm::vec3(0.14f, 0.31f, 0.14f), jersey);
        glm::mat4 elbow = glm::translate(shoulder, glm::vec3(0, -0.30f, 0));
        drawCube(modelLoc, VAO, elbow, glm::vec3(0, -0.16f, 0),
            glm::vec3(0.12f, 0.32f, 0.12f), skin);
        drawCube(modelLoc, VAO, elbow, glm::vec3(0, -0.33f, 0),
            glm::vec3(0.19f, 0.16f, 0.17f), glove);

        glm::mat4 hip = glm::translate(lower, glm::vec3(armSide * 0.13f, 0.90f, 0));
        hip = glm::rotate(hip, glm::radians(-side * 8.0f * legsFollow), glm::vec3(0, 0, 1));
        drawCube(modelLoc, VAO, hip, glm::vec3(0, -0.19f, 0),
            glm::vec3(0.19f, 0.38f, 0.19f), shorts);
        glm::mat4 knee = glm::translate(hip, glm::vec3(0, -0.38f, 0));
        drawCube(modelLoc, VAO, knee, glm::vec3(0, -0.19f, 0),
            glm::vec3(0.15f, 0.38f, 0.15f), skin);
        drawCube(modelLoc, VAO, knee, glm::vec3(0, -0.42f, 0.09f),
            glm::vec3(0.17f, 0.11f, 0.29f), shoes);
    }
}

// Dibuja la celebracion del capitan.
void drawCelebrationCaptain(GLint modelLoc, GLuint VAO, GLuint torsoVAO, GLuint headVAO, const glm::mat4& scene,
    glm::vec3 position, float yaw, GLuint texJersey, GLuint texHead,
    GLuint texShorts, GLuint texSkin, GLuint texShoes, int jerseyNumber, GLuint numberColor)
{
    glm::mat4 model = glm::translate(scene, position);
    model = glm::rotate(model, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));

    drawCube(modelLoc, torsoVAO, model, glm::vec3(0.0f, 1.20f, 0.0f),
        glm::vec3(0.45f, 0.64f, 0.25f), texJersey);
    if (jerseyNumber > 0 && numberColor != 0) {
        drawCube(modelLoc, VAO, model, glm::vec3(0.0f, 1.20f, -0.131f),
            glm::vec3(0.31f, 0.32f, 0.014f), texShorts);
        glm::mat4 digits = glm::scale(model, glm::vec3(-1.0f, 1.0f, 1.0f));
        float left = jerseyNumber >= 10 ? -0.12f : -0.06f;
        if (jerseyNumber >= 10)
            drawScoreDigit(modelLoc, VAO, digits, jerseyNumber / 10,
                left, 1.10f, -0.146f, 0.11f, numberColor);
        drawScoreDigit(modelLoc, VAO, digits, jerseyNumber % 10,
            jerseyNumber >= 10 ? 0.015f : left, 1.10f, -0.146f, 0.11f, numberColor);
    }
    drawCube(modelLoc, VAO, model, glm::vec3(0.0f, 1.55f, 0.0f),
        glm::vec3(0.12f, 0.13f, 0.12f), texSkin);
    drawCube(modelLoc, headVAO, model, glm::vec3(0.0f, 1.77f, 0.0f),
        glm::vec3(0.27f, 0.29f, 0.27f), texHead);

    auto armSegment = [&](glm::vec3 a, glm::vec3 b, GLuint material, float thickness) {
        glm::vec3 direction = b - a;
        float length = glm::length(direction);
        if (length < 0.0001f) return;
        direction /= length;
        glm::vec3 axis = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), direction);
        if (glm::length(axis) < 0.0001f) axis = glm::vec3(1.0f, 0.0f, 0.0f);
        else axis = glm::normalize(axis);
        float angle = std::acos(glm::clamp(direction.y, -1.0f, 1.0f));
        glm::mat4 piece = glm::translate(model, (a + b) * 0.5f);
        piece = glm::rotate(piece, angle, axis);
        drawCube(modelLoc, VAO, piece, glm::vec3(0.0f),
            glm::vec3(thickness, length, thickness), material);
        };

    float handleY = 0.02f + trophyLiftHeight() + 0.37f * 1.36f;
    for (int side = -1; side <= 1; side += 2) {
        glm::vec3 shoulder(side * 0.30f, 1.43f, 0.03f);
        glm::vec3 hand(side * 0.30f, handleY, 0.38f);
        glm::vec3 elbow(side * 0.52f, (shoulder.y + hand.y) * 0.5f, 0.20f);
        armSegment(shoulder, elbow, texJersey, 0.145f);
        armSegment(elbow, hand, texSkin, 0.115f);
        drawCube(modelLoc, VAO, model, hand, glm::vec3(0.16f, 0.15f, 0.17f), texSkin);

        glm::mat4 hip = glm::translate(model, glm::vec3(side * 0.13f, 0.90f, 0));
        drawCube(modelLoc, VAO, hip, glm::vec3(0, -0.19f, 0),
            glm::vec3(0.19f, 0.38f, 0.19f), texShorts);
        glm::mat4 knee = glm::translate(hip, glm::vec3(0, -0.38f, 0));
        drawCube(modelLoc, VAO, knee, glm::vec3(0, -0.19f, 0),
            glm::vec3(0.15f, 0.38f, 0.15f), texSkin);
        drawCube(modelLoc, VAO, knee, glm::vec3(0, -0.42f, 0.09f),
            glm::vec3(0.17f, 0.11f, 0.29f), texShoes);
    }
}

static glm::vec3 SpherePoint(float theta, float phi)
{
    return glm::vec3(sinf(theta) * cosf(phi), cosf(theta),
        sinf(theta) * sinf(phi));
}

static void AddVertex(std::vector<GLfloat>& v, glm::vec3 p)
{
    // Vértices: posición XYZ y normal XYZ.
    v.push_back(p.x); v.push_back(p.y); v.push_back(p.z);
    v.push_back(p.x); v.push_back(p.y); v.push_back(p.z);
}

static bool IsBlackPatch(glm::vec3 normal)
{
    const float d = 1.61803398875f;
    const glm::vec3 centers[] = {
        {0, 1, d}, {0, -1, d}, {0, 1, -d}, {0, -1, -d},
        {1, d, 0}, {-1, d, 0}, {1, -d, 0}, {-1, -d, 0},
        {d, 0, 1}, {-d, 0, 1}, {d, 0, -1}, {-d, 0, -1}
    };
    for (int i = 0; i < 12; ++i)
        if (glm::dot(normal, glm::normalize(centers[i])) > 0.925f)
            return true;
    return false;
}

static void AddTriangle(std::vector<GLfloat>& whiteVertices,
    std::vector<GLfloat>& blackVertices,
    glm::vec3 a, glm::vec3 b, glm::vec3 c)
{
    glm::vec3 direction = glm::normalize(a + b + c);
    std::vector<GLfloat>& target = IsBlackPatch(direction) ? blackVertices : whiteVertices;
    AddVertex(target, a);
    AddVertex(target, b);
    AddVertex(target, c);
}

static void CreateBallMesh(BallMesh& mesh, const std::vector<GLfloat>& vertexData)
{
    mesh.vertexCount = static_cast<GLsizei>(vertexData.size() / 6);
    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glBindVertexArray(mesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat),
        (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

// Crea las partes blancas y negras del balon.
void createBall(BallMesh& whiteMesh, BallMesh& blackMesh)
{
    const float PI = 3.14159265358979323846f;
    const int latitudeSegments = 24, longitudeSegments = 48;
    std::vector<GLfloat> whiteVertices, blackVertices;
    for (int i = 0; i < latitudeSegments; ++i) {
        float t0 = PI * i / latitudeSegments;
        float t1 = PI * (i + 1) / latitudeSegments;
        for (int j = 0; j < longitudeSegments; ++j) {
            float p0 = 2.0f * PI * j / longitudeSegments;
            float p1 = 2.0f * PI * (j + 1) / longitudeSegments;
            glm::vec3 a = SpherePoint(t0, p0);
            glm::vec3 b = SpherePoint(t0, p1);
            glm::vec3 c = SpherePoint(t1, p0);
            glm::vec3 d = SpherePoint(t1, p1);
            AddTriangle(whiteVertices, blackVertices, a, d, c);
            AddTriangle(whiteVertices, blackVertices, a, b, d);
        }
    }
    CreateBallMesh(whiteMesh, whiteVertices);
    CreateBallMesh(blackMesh, blackVertices);
}

// Dibuja el balon en su posicion actual.
void drawBall(GLint modelLoc, GLint textureModeLoc, const glm::mat4& scene,
    const BallMesh& whiteMesh, const BallMesh& blackMesh,
    GLuint texWhite, GLuint texBlack, float rotation, glm::vec3 position)
{
    glUniform1i(textureModeLoc, 6);
    glm::mat4 model = glm::translate(scene, position);
    model = glm::rotate(model, glm::radians(rotation), glm::vec3(0, 0, 1));
    model = glm::scale(model, glm::vec3(0.11f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texWhite);
    glBindVertexArray(whiteMesh.VAO);
    glDrawArrays(GL_TRIANGLES, 0, whiteMesh.vertexCount);
    glBindTexture(GL_TEXTURE_2D, texBlack);
    glBindVertexArray(blackMesh.VAO);
    glDrawArrays(GL_TRIANGLES, 0, blackMesh.vertexCount);
    glBindVertexArray(0);
    glUniform1i(textureModeLoc, 0);
}

// Libera los recursos del balon.
void deleteBallMesh(BallMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.VAO);
    glDeleteBuffers(1, &mesh.VBO);
}

// Dibuja una esfera con una textura.
void drawSolidSphere(GLint modelLoc, const glm::mat4& parent, glm::vec3 position,
    glm::vec3 size, const BallMesh& whiteMesh, const BallMesh& blackMesh, GLuint texture)
{
    glm::mat4 model = glm::translate(parent, position);
    model = glm::scale(model, size);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(whiteMesh.VAO);
    glDrawArrays(GL_TRIANGLES, 0, whiteMesh.vertexCount);
    glBindVertexArray(blackMesh.VAO);
    glDrawArrays(GL_TRIANGLES, 0, blackMesh.vertexCount);
    glBindVertexArray(0);
}

// Dibuja los arboles alrededor del estadio.
void drawTrees(GLint modelLoc, GLint textureModeLoc, GLuint VAO, const glm::mat4& scene,
    const BallMesh& whiteMesh, const BallMesh& blackMesh,
    GLuint texBark, GLuint texLeavesDark, GLuint texLeavesLight)
{
    for (int side = -1; side <= 1; side += 2) {
        for (int i = -3; i <= 3; ++i) {
            if (i == 0) continue;
            float x = static_cast<float>(i) * 15.5f;
            float z = side * (69.0f + ((i + 3) % 2) * 2.2f);
            float height = 3.5f + static_cast<float>((i + 3) % 3) * 0.45f;
            glUniform1i(textureModeLoc, 2);
            drawCube(modelLoc, VAO, scene, glm::vec3(x, height * 0.5f, z),
                glm::vec3(0.5f, height, 0.5f), texBark);
            glUniform1i(textureModeLoc, 0);
            glUniform1i(textureModeLoc, 3);
            drawSolidSphere(modelLoc, scene, glm::vec3(x, height + 0.35f, z),
                glm::vec3(2.0f, 1.65f, 1.8f), whiteMesh, blackMesh, texLeavesDark);
            drawSolidSphere(modelLoc, scene, glm::vec3(x + 0.5f, height + 1.2f, z - 0.45f),
                glm::vec3(1.35f, 1.2f, 1.25f), whiteMesh, blackMesh, texLeavesLight);
            glUniform1i(textureModeLoc, 0);
        }
    }
}

// Dibuja un pajaro en movimiento.
void drawBird(GLint modelLoc, GLint textureModeLoc, GLuint VAO, const glm::mat4& scene,
    const BallMesh& whiteMesh, const BallMesh& blackMesh,
    GLuint texBird, GLuint texBeak, float elapsed)
{
    float orbit = elapsed * 0.24f;
    float x = 58.0f * cosf(orbit);
    float z = 41.0f * sinf(orbit);
    float y = 11.5f + 1.8f * sinf(elapsed * 0.46f);

    glm::mat4 bird = glm::translate(scene, glm::vec3(x, y, z));
    glm::vec3 direction(-58.0f * sinf(orbit), 0.0f, 41.0f * cosf(orbit));
    float yaw = atan2f(-direction.z, direction.x);
    bird = glm::rotate(bird, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    bird = glm::rotate(bird, glm::radians(4.0f * sinf(elapsed * 0.8f)),
        glm::vec3(0.0f, 0.0f, 1.0f));

    glUniform1i(textureModeLoc, 6);
    drawSolidSphere(modelLoc, bird, glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.55f, 0.20f, 0.24f), whiteMesh, blackMesh, texBird);
    drawSolidSphere(modelLoc, bird, glm::vec3(0.48f, 0.12f, 0.0f),
        glm::vec3(0.23f, 0.23f, 0.20f), whiteMesh, blackMesh, texBird);

    glUniform1i(textureModeLoc, 7);
    drawCube(modelLoc, VAO, bird, glm::vec3(0.72f, 0.07f, 0.0f),
        glm::vec3(0.24f, 0.09f, 0.14f), texBeak);
    drawCube(modelLoc, VAO, bird, glm::vec3(-0.64f, 0.05f, 0.0f),
        glm::vec3(0.48f, 0.06f, 0.36f), texBird);

    float flap = glm::radians(35.0f * sinf(elapsed * 8.0f));
    for (int side = -1; side <= 1; side += 2) {
        glm::mat4 wing = glm::translate(bird, glm::vec3(-0.05f, 0.07f, side * 0.16f));
        wing = glm::rotate(wing, side * flap, glm::vec3(1.0f, 0.0f, 0.0f));
        drawCube(modelLoc, VAO, wing, glm::vec3(-0.03f, 0.0f, side * 0.55f),
            glm::vec3(0.55f, 0.075f, 1.0f), texBird);
        glm::mat4 outerWing = glm::translate(wing, glm::vec3(0.0f, 0.0f, side * 1.0f));
        outerWing = glm::rotate(outerWing, side * flap * 0.55f,
            glm::vec3(1.0f, 0.0f, 0.0f));
        drawCube(modelLoc, VAO, outerWing, glm::vec3(-0.12f, 0.0f, side * 0.32f),
            glm::vec3(0.40f, 0.05f, 0.66f), texBird);
    }
    glUniform1i(textureModeLoc, 0);
}

// Dibuja los fuegos artificiales.
void drawFireworks(GLint modelLoc, GLuint VAO, const glm::mat4& scene,
    float elapsed, GLuint texGold, GLuint texBlue, GLuint texRed, GLuint texWhite)
{
    const float pi = 3.14159265358979323846f;
    for (int burst = 0; burst < 3; ++burst) {
        float local = elapsed - 0.48f * burst;
        if (local < 0.0f || local > 3.2f)
            continue;

        float x = 60.0f + (burst % 2 == 0 ? 1.0f : -3.0f);
        float z = -21.0f + burst * 20.0f;
        float explodeAt = 0.60f;
        float baseHeight = 10.0f + burst * 1.7f;

        if (local < explodeAt) {
            float y = baseHeight * local / explodeAt;
            drawCube(modelLoc, VAO, scene, glm::vec3(x, y, z),
                glm::vec3(0.13f, 0.45f, 0.13f), texGold);
            continue;
        }

        float t = local - explodeAt;
        float radius = 7.0f * t;
        for (int i = 0; i < 32; ++i) {
            float theta = pi * (static_cast<float>((i * 11) % 31) + 0.5f) / 31.0f;
            float phi = 2.0f * pi * (static_cast<float>(i) / 1.61803398875f);
            glm::vec3 unit(sinf(theta) * cosf(phi), cosf(theta), sinf(theta) * sinf(phi));
            glm::vec3 p(x + unit.x * radius,
                baseHeight + unit.y * radius - 1.35f * t * t,
                z + unit.z * radius);
            GLuint color = i % 4 == 0 ? texGold :
                (i % 4 == 1 ? texBlue : (i % 4 == 2 ? texRed : texWhite));
            float size = std::max(0.05f, 0.23f * (1.0f - t / 3.0f));
            drawCube(modelLoc, VAO, scene, p,
                glm::vec3(size, size * 1.5f, size), color);
        }
    }
}

// Actualiza el recorrido de los autobuses.
void updateBusAnimation()
{
    if (!busAssetsReady || !arrivalStarted) return;
    float frameStep = std::max(0.0f, std::min(deltaTime, 0.1f));
    for (int team = 0; team < 2; ++team) {
        TeamBus& bus = teamBuses[team];
        if (!bus.moving) {
            if (bus.state == BUS_COMPLETED)
                teamLineupTime[team] += frameStep;
            continue;
        }
        bus.distanceTraveled = std::min(busRouteLength,
            bus.distanceTraveled + busSpeed * frameStep);
        float distance = bus.distanceTraveled;
        float sign = team == 0 ? 1.0f : -1.0f;
        float y = bus.position.y;
        if (distance <= busInitialDistance) {
            bus.state = BUS_STRAIGHT;
            bus.position = glm::vec3(sign * (102.0f - distance), y, -sign * 21.0f);
            bus.heading = -sign * busPi * 0.5f;
            bus.steeringAngle = 0.0f;
        }
        else {
            bus.state = BUS_TURNING;
            float angle = glm::clamp((distance - busInitialDistance) / busTurnRadius,
                0.0f, busPi * 0.5f);
            bus.position = glm::vec3(sign * (74.0f - busTurnRadius * sinf(angle)),
                y, -sign * (21.0f - busTurnRadius * (1.0f - cosf(angle))));
            bus.heading = (team == 0 ? -busPi * 0.5f : busPi * 0.5f) + angle;
            bus.steeringAngle = glm::radians(18.0f) * sinf(2.0f * angle);
        }
        if (distance >= busRouteLength) {
            bus.position = glm::vec3(sign * 66.0f, y, -sign * 13.0f);
            bus.heading = team == 0 ? 0.0f : busPi;
            bus.steeringAngle = 0.0f;
            bus.state = BUS_COMPLETED;
            bus.moving = false;
            teamLineupTime[team] = 0.0f;
        }
    }
}

// Lee las posiciones y tamanos de las ruedas.
bool loadBusWheelData(const char* filePath)
{
    std::ifstream fileStream(filePath);
    if (!fileStream) {
        std::cerr << "Missing bus wheel data: " << filePath << std::endl;
        return false;
    }
    std::string line;
    int i = 0;
    while (std::getline(fileStream, line) && i < 6) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream lineStream(line);
        float x, y, z, radius;
        int frontWheelFlag;
        if (!(lineStream >> x >> y >> z >> radius >> frontWheelFlag) || radius <= 0.0f) {
            std::cerr << "Invalid bus wheel data: " << filePath << std::endl;
            return false;
        }
        busWheelData[i++] = { glm::vec3(x, y, z), radius, frontWheelFlag != 0 };
    }
    if (i != 6) {
        std::cerr << "Expected 6 wheels in: " << filePath << std::endl;
        return false;
    }
    return true;
}

// Obtiene la posicion de la camara seleccionada.
glm::vec3 getBroadcastPosition()
{
    if (activeCamera == 9)
        return matchScene == SCENE_FINAL ? glm::vec3(0.0f, 92.0f, 38.0f) : glm::vec3(0.0f, 92.0f, 0.0f);
    if (activeCamera >= 1 && activeCamera <= 2) {
        const glm::vec3& dronePosition = broadcastCameraPositions[activeCamera - 1];
        glm::vec3 forward = trackedBallPosition - dronePosition;
        forward.y = 0.0f;
        if (glm::length(forward) < 0.0001f)
            forward = glm::vec3(1.0f, 0.0f, 0.0f);
        forward = glm::normalize(forward);
        return dronePosition + forward * 2.0f + glm::vec3(0.0f, -0.72f, 0.0f);
    }
    if (activeCamera >= 3 && activeCamera <= 8) {
        const glm::vec3& cameraPosition = broadcastCameraPositions[activeCamera - 1];
        glm::vec3 pivot = cameraPosition + cameraModelScale * cameraHeadPivot;
        glm::vec3 forward = trackedBallPosition - pivot;
        if (glm::length(forward) < 0.0001f)
            forward = glm::vec3(1.0f, 0.0f, 0.0f);
        return pivot + glm::normalize(forward) * 1.5f;
    }
    return freeCameraPosition;
}

// Calcula la vista de la camara.
glm::mat4 getBroadcastView()
{
    if (activeCamera == 9)
        return glm::lookAt(getBroadcastPosition(),
            matchScene == SCENE_FINAL ? trophyFocusPosition : glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, -1.0f));
    if (activeCamera >= 1 && activeCamera <= 8)
        return glm::lookAt(getBroadcastPosition(), trackedBallPosition,
            glm::vec3(0.0f, 1.0f, 0.0f));
    return glm::lookAt(freeCameraPosition, freeCameraPosition + camera.GetFront(),
        glm::vec3(0.0f, 1.0f, 0.0f));
}

// Dibuja las torres de iluminacion.
void drawFloodlightTowers(GLint modelLoc, GLint textureModeLoc, GLuint VAO,
    const glm::mat4& scene, GLuint texMetal)
{
    glUniform1i(textureModeLoc, 5);
    for (int i = 0; i < 4; ++i) {
        const glm::vec3& p = floodlightPositions[i];
        drawCube(modelLoc, VAO, scene, p + glm::vec3(0.0f, floodlightHeight * 0.49f, 0.0f),
            glm::vec3(0.60f, floodlightHeight * 0.98f, 0.60f), texMetal);
        glm::vec3 aim = floodlightTargets[i] - (p + glm::vec3(0.0f, floodlightHeight, 0.0f));
        float facing = std::atan2(aim.x, aim.z);
        float downwardTilt = std::atan2(-aim.y, glm::length(glm::vec2(aim.x, aim.z)));
        glm::mat4 mount = glm::translate(scene, p + glm::vec3(0.0f, floodlightHeight, 0.0f));
        mount = glm::rotate(mount, facing, glm::vec3(0.0f, 1.0f, 0.0f));
        mount = glm::rotate(mount, downwardTilt, glm::vec3(1.0f, 0.0f, 0.0f));
        drawCube(modelLoc, VAO, mount, glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(5.25f, 1.6f, 0.48f), texMetal);
        for (int beam = 1; beam < 7; ++beam) {
            float y = beam * (floodlightHeight - 1.0f) / 7.0f;
            drawCube(modelLoc, VAO, scene, p + glm::vec3(0.0f, y, 0.0f),
                glm::vec3(1.15f, 0.10f, 1.05f), texMetal);
        }
    }
    glUniform1i(textureModeLoc, 0);
}

// Dibuja los paneles de las luces.
void drawFloodlightPanels(GLint modelLoc, GLuint VAO)
{
    glBindVertexArray(VAO);
    for (int i = 0; i < 4; ++i) {
        const glm::vec3& p = floodlightPositions[i];
        glm::vec3 aim = floodlightTargets[i] - (p + glm::vec3(0.0f, floodlightHeight, 0.0f));
        float facing = std::atan2(aim.x, aim.z);
        float downwardTilt = std::atan2(-aim.y, glm::length(glm::vec2(aim.x, aim.z)));
        glm::mat4 mount = glm::translate(glm::mat4(1.0f), p + glm::vec3(0.0f, floodlightHeight, 0.0f));
        mount = glm::rotate(mount, facing, glm::vec3(0.0f, 1.0f, 0.0f));
        mount = glm::rotate(mount, downwardTilt, glm::vec3(1.0f, 0.0f, 0.0f));
        for (int row = 0; row < 2; ++row) {
            for (int col = 0; col < 7; ++col) {
                glm::mat4 m = glm::translate(mount,
                    glm::vec3((col - 3.0f) * 0.68f, (row - 0.5f) * 0.68f, 0.30f));
                m = glm::scale(m, glm::vec3(0.54f, 0.48f, 0.12f));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }
    }
    glBindVertexArray(0);
}

// Dibuja las camaras que siguen la accion.
void drawBroadcastCameraModels(GLint modelLoc, Shader& lightingShader, Model* cameraBase, Model* cameraHead)
{
    if (!cameraBase || !cameraHead) return;
    glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
    glUniform1i(glGetUniformLocation(lightingShader.Program, "proceduralTextureMode"), 0);
    glUniform1i(glGetUniformLocation(lightingShader.Program, "useImportedModelTexture"), 1);
    GLint neutralCameraLoc = glGetUniformLocation(lightingShader.Program, "cameraNeutralMaterial");
    glUniform1i(neutralCameraLoc, 1);
    for (int i = 2; i < 8; ++i) {
        if (activeCamera == i + 1) continue;
        const glm::vec3& position = broadcastCameraPositions[i];
        glm::mat4 baseMatrix = glm::translate(glm::mat4(1.0f), position);
        baseMatrix = glm::scale(baseMatrix, glm::vec3(cameraModelScale));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(baseMatrix));
        cameraBase->Draw(lightingShader);

        glm::vec3 pivotWorld = position + cameraModelScale * cameraHeadPivot;
        glm::vec3 direction = trackedBallPosition - pivotWorld;
        float horizontalDistance = glm::length(glm::vec2(direction.x, direction.z));
        if (glm::length(direction) < 0.0001f)
            direction = glm::vec3(1.0f, 0.0f, 0.0f);
        float yaw = std::atan2(-direction.z, direction.x) + glm::radians(cameraModelForwardYawDegrees);
        float pitch = std::atan2(direction.y, std::max(0.0001f, horizontalDistance));
        pitch = glm::clamp(pitch, glm::radians(-65.0f), glm::radians(65.0f));

        glm::mat4 headMatrix = glm::translate(baseMatrix, cameraHeadPivot);
        headMatrix = glm::rotate(headMatrix, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
        headMatrix = glm::rotate(headMatrix, pitch, glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(headMatrix));
        cameraHead->Draw(lightingShader);
    }
    glUniform1i(neutralCameraLoc, 0);
}

// Dibuja las gradas del estadio.
void drawBleachers(GLint modelLoc, GLint textureModeLoc, GLuint VAO, const glm::mat4& scene,
    GLuint texConcrete, GLuint texSideWall, GLuint texWood, GLuint texWalkway, GLuint texMetal)
{
    for (int side = -1; side <= 1; side += 2) {
        for (int p = -9; p <= 9; ++p) {
            float x = p * 5.0f;
            drawCube(modelLoc, VAO, scene, glm::vec3(x, 0.53f, side * 49.45f),
                glm::vec3(0.10f, 1.06f, 0.10f), texMetal);
        }
        drawCube(modelLoc, VAO, scene, glm::vec3(0.0f, 1.05f, side * 49.45f),
            glm::vec3(90.0f, 0.08f, 0.08f), texMetal);
        for (int row = 0; row < 8; ++row) {
            float z = side * (52.0f + 2.0f * row);
            float height = 0.72f * (row + 1);
            glUniform1i(textureModeLoc, 5);
            drawCube(modelLoc, VAO, scene, glm::vec3(0.0f, height * 0.5f, z),
                glm::vec3(90.0f, height, 2.0f), texConcrete);
            for (int section = 0; section < 11; ++section) {
                if (section == 5) continue;
                float x = (section - 5) * 7.55f;
                drawCube(modelLoc, VAO, scene,
                    glm::vec3(x, height + 0.12f, z - side * 0.15f),
                    glm::vec3(7.15f, 0.23f, 1.15f), texWood);
                drawCube(modelLoc, VAO, scene,
                    glm::vec3(x, height + 0.40f, z + side * 0.48f),
                    glm::vec3(7.15f, 0.60f, 0.19f), texWood);
            }
            glUniform1i(textureModeLoc, 0);
            drawCube(modelLoc, VAO, scene,
                glm::vec3(0.0f, height + 0.025f, z),
                glm::vec3(5.9f, 0.05f, 1.85f), texWalkway);
        }
        glUniform1i(textureModeLoc, 5);
        for (int i = -1; i <= 1; i += 2) {
            drawCube(modelLoc, VAO, scene,
                glm::vec3(i * 45.3f, 3.3f, side * 59.0f),
                glm::vec3(0.20f, 6.6f, 16.0f), texSideWall);
        }
        glUniform1i(textureModeLoc, 0);
    }
}

// Dibuja al publico en las gradas.
void drawCrowd(GLint modelLoc, GLuint VAO, GLuint jerseyVAO, GLuint headVAO,
    const glm::mat4& scene, GLuint texPumas, GLuint texAtlante,
    GLuint texHead, const GLuint crowdHeads[4], GLuint texPumasShorts, GLuint texAtlanteShorts,
    GLuint texHeadJoy, GLuint texHeadAngry, GLuint texHeadSad, GLuint texHeadSurprise)
{
    float cheerTime = matchScene == SCENE_PUMAS_GOAL ? sceneTime - 5.35f :
        matchScene == SCENE_ATLANTE_GOAL ? sceneTime - 4.1f :
        matchScene == SCENE_PENALTIES ? penaltyRestTime : sceneTime;
    int celebratingSide = crowdCelebratingSide();
    for (int side = -1; side <= 1; side += 2) {
        bool celebrating = side == crowdCelebratingSide();
        bool isRivalConceding = (celebratingSide != 0 && side != celebratingSide);
        GLuint shirt = side < 0 ? texPumas : texAtlante;
        for (int row = 1; row <= 6; ++row) {
            float height = 0.72f * (row + 1);
            float z = side * (52.0f + 2.0f * row);
            for (int section = 0; section < 11; ++section) {
                if (section == 5) continue;
                for (int seat = 0; seat < 2; ++seat) {
                    float x = (section - 5) * 7.55f + (seat == 0 ? -1.6f : 1.6f);
                    float phase = row * 0.8f + section * 1.4f + seat;
                    float jump = celebrating ? 0.52f * glm::clamp(cheerTime / 0.25f, 0.0f, 1.0f) *
                        std::max(0.0f, sinf(cheerTime * 8.0f + phase)) : 0.0f;
                    glm::mat4 person = glm::translate(scene, glm::vec3(x, height + 0.235f + jump, z));
                    person = glm::rotate(person, glm::radians(side < 0 ? 0.0f : 180.0f),
                        glm::vec3(0, 1, 0));
                    GLuint face = crowdHeads[(section + 2 * row + seat) % 4];
                    if (celebrating && texHeadJoy) {
                        face = texHeadJoy;
                    }
                    else if (isRivalConceding) {
                        face = ((section + row + seat) % 3 == 0) ? texHeadSurprise :
                            (((section + seat) % 2 == 0) ? texHeadAngry : texHeadSad);
                    }
                    drawCube(modelLoc, jerseyVAO, person, glm::vec3(0, 0.33f, 0),
                        glm::vec3(0.43f, 0.62f, 0.23f), shirt);
                    drawCube(modelLoc, headVAO, person, glm::vec3(0, 0.79f, 0),
                        glm::vec3(0.28f, 0.28f, 0.28f), face ? face : texHead);
                    if (celebrating) {
                        for (int leg = -1; leg <= 1; leg += 2)
                            drawCube(modelLoc, VAO, person, glm::vec3(leg * 0.11f, -0.15f, 0.13f),
                                glm::vec3(0.15f, 0.42f, 0.25f),
                                side < 0 ? texPumasShorts : texAtlanteShorts);
                    }
                    for (int arm = -1; arm <= 1; arm += 2) {
                        if (celebrating) {
                            glm::mat4 shoulder = glm::translate(person, glm::vec3(arm * 0.25f, 0.43f, 0));
                            shoulder = glm::rotate(shoulder,
                                glm::radians(-arm * (35.0f + 20.0f * sinf(cheerTime * 9.0f + phase))),
                                glm::vec3(0, 0, 1));
                            drawCube(modelLoc, VAO, shoulder, glm::vec3(0, 0.18f, 0),
                                glm::vec3(0.10f, 0.34f, 0.11f), shirt);
                        }
                        else
                            drawCube(modelLoc, VAO, person, glm::vec3(arm * 0.27f, 0.34f, 0),
                                glm::vec3(0.10f, 0.34f, 0.11f), shirt);
                    }
                }
            }
        }
    }
}

// Dibuja el dron y sus helices.
void drawDrone(GLint modelLoc, GLint textureModeLoc, GLuint VAO, const glm::mat4& scene,
    const BallMesh& whiteMesh, const BallMesh& blackMesh,
    glm::vec3 position, glm::vec3 target, float elapsed,
    GLuint texBody, GLuint texBlades, GLuint texMetal, GLuint texLens)
{
    glm::vec3 horizontal = target - position;
    horizontal.y = 0.0f;
    float yaw = std::atan2(-horizontal.z, horizontal.x);
    glm::mat4 drone = glm::translate(scene, position);
    drone = glm::rotate(drone, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    drone = glm::rotate(drone, glm::radians(1.8f * sinf(elapsed * 2.0f)),
        glm::vec3(0.0f, 0.0f, 1.0f));

    glUniform1i(textureModeLoc, 7);
    drawCube(modelLoc, VAO, drone, glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.45f, 0.32f, 0.95f), texBody);
    drawCube(modelLoc, VAO, drone, glm::vec3(0.1f, 0.22f, 0.0f),
        glm::vec3(0.65f, 0.13f, 0.63f), texMetal);
    for (int sx = -1; sx <= 1; sx += 2) {
        for (int sz = -1; sz <= 1; sz += 2) {
            glm::vec3 rotorCenter(sx * 1.0f, 0.10f, sz * 1.0f);
            float armYaw = std::atan2(-static_cast<float>(sz), static_cast<float>(sx));
            glm::mat4 arm = glm::rotate(drone, armYaw, glm::vec3(0.0f, 1.0f, 0.0f));
            glUniform1i(textureModeLoc, 7);
            drawCube(modelLoc, VAO, arm, glm::vec3(0.69f, 0.04f, 0.0f),
                glm::vec3(1.46f, 0.11f, 0.13f), texMetal);
            glUniform1i(textureModeLoc, 6);
            drawSolidSphere(modelLoc, drone, rotorCenter,
                glm::vec3(0.25f, 0.13f, 0.25f), whiteMesh, blackMesh, texBody);
            glm::mat4 rotor = glm::translate(drone, rotorCenter + glm::vec3(0.0f, 0.16f, 0.0f));
            rotor = glm::rotate(rotor, (sx * sz) * elapsed * 31.0f,
                glm::vec3(0.0f, 1.0f, 0.0f));
            glUniform1i(textureModeLoc, 7);
            drawCube(modelLoc, VAO, rotor, glm::vec3(0.0f),
                glm::vec3(1.08f, 0.035f, 0.11f), texBlades);
            drawCube(modelLoc, VAO, rotor, glm::vec3(0.0f),
                glm::vec3(0.11f, 0.035f, 1.08f), texBlades);
            drawCube(modelLoc, VAO, drone, glm::vec3(sx * 0.45f, -0.32f, sz * 0.32f),
                glm::vec3(0.10f, 0.55f, 0.10f), texMetal);
        }
    }
    glUniform1i(textureModeLoc, 7);
    drawCube(modelLoc, VAO, drone, glm::vec3(0.41f, -0.30f, 0.0f),
        glm::vec3(0.50f, 0.34f, 0.40f), texBody);
    glUniform1i(textureModeLoc, 6);
    drawSolidSphere(modelLoc, drone, glm::vec3(0.68f, -0.30f, 0.0f),
        glm::vec3(0.14f, 0.14f, 0.14f), whiteMesh, blackMesh, texLens);
    glUniform1i(textureModeLoc, 0);
}

// Dibuja el trofeo del partido.
void drawTrophy(GLint modelLoc, GLint textureModeLoc, GLuint VAO, const glm::mat4& scene,
    const BallMesh& whiteMesh, const BallMesh& blackMesh,
    float elapsed, GLuint texGold, GLuint texBase, GLuint texWhite)
{
    glUniform1i(textureModeLoc, 7);
    bool finalLift = matchScene == SCENE_FINAL;
    glm::mat4 pedestal = glm::translate(scene, glm::vec3(0.0f, 0.0f, 44.0f));
    if (!finalLift) {
        drawCube(modelLoc, VAO, pedestal, glm::vec3(0.0f, 0.25f, 0.0f),
            glm::vec3(3.6f, 0.5f, 3.1f), texBase);
        drawCube(modelLoc, VAO, pedestal, glm::vec3(0.0f, 0.55f, 0.0f),
            glm::vec3(2.0f, 0.1f, 1.8f), texWhite);
    }
    glm::vec3 position = finalLift ? glm::vec3(0.0f, 0.02f + trophyLiftHeight(), 37.62f)
        : glm::vec3(0.0f, 0.65f, 44.0f);
    glm::mat4 trophy = glm::translate(scene, position);
    if (!finalLift)
        trophy = glm::rotate(trophy, glm::radians(elapsed * 32.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    trophy = glm::scale(trophy, glm::vec3(finalLift ? 0.37f : 0.64f));
    drawCube(modelLoc, VAO, trophy, glm::vec3(0.0f, 0.12f, 0.0f),
        glm::vec3(0.95f, 0.24f, 0.95f), texGold);
    drawCube(modelLoc, VAO, trophy, glm::vec3(0.0f, 0.44f, 0.0f),
        glm::vec3(0.22f, 0.55f, 0.22f), texGold);
    drawCube(modelLoc, VAO, trophy, glm::vec3(0.0f, 0.76f, 0.0f),
        glm::vec3(0.70f, 0.13f, 0.70f), texGold);
    glUniform1i(textureModeLoc, 6);
    drawSolidSphere(modelLoc, trophy, glm::vec3(0.0f, 1.23f, 0.0f),
        glm::vec3(0.63f, 0.56f, 0.55f), whiteMesh, blackMesh, texGold);
    glUniform1i(textureModeLoc, 7);
    drawCube(modelLoc, VAO, trophy, glm::vec3(0.0f, 1.69f, 0.0f),
        glm::vec3(1.23f, 0.12f, 1.04f), texGold);
    for (int side = -1; side <= 1; side += 2) {
        drawCube(modelLoc, VAO, trophy, glm::vec3(side * 0.80f, 1.36f, 0.0f),
            glm::vec3(0.11f, 0.57f, 0.11f), texGold);
        drawCube(modelLoc, VAO, trophy, glm::vec3(side * 0.61f, 1.61f, 0.0f),
            glm::vec3(0.44f, 0.11f, 0.12f), texGold);
        drawCube(modelLoc, VAO, trophy, glm::vec3(side * 0.61f, 1.10f, 0.0f),
            glm::vec3(0.43f, 0.10f, 0.12f), texGold);
    }
    glUniform1i(textureModeLoc, 0);
}

// Dibuja un numero en el marcador.
void drawScoreDigit(GLint modelLoc, GLuint VAO, const glm::mat4& matrix,
    int digit, float x, float y, float z, float width, GLuint texture)
{
    static const int mask[10] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F };
    if (digit < 0 || digit > 9) return;
    float thickness = width * 0.12f;
    float height = width * 1.85f;
    for (int s = 0; s < 7; ++s) {
        if (!(mask[digit] & (1 << s))) continue;
        bool horizontal = (s == 0 || s == 3 || s == 6);
        float sx = x + ((s == 1 || s == 2) ? width * 0.43f :
            ((s == 4 || s == 5) ? -width * 0.43f : 0.0f));
        float sy = y + ((s == 0) ? height : (s == 6 ? height * 0.5f :
            (s == 3 ? 0.0f : ((s == 1 || s == 5) ? height * 0.75f : height * 0.25f))));
        glm::vec3 size = horizontal ? glm::vec3(width, thickness, thickness) :
            glm::vec3(thickness, height * 0.47f, thickness);
        drawCube(modelLoc, VAO, matrix, glm::vec3(sx, sy, z), size, texture);
    }
}

// Obtiene el patron para dibujar una letra.
const char* getScoreGlyph(char letter, int row)
{
    static const char* P[] = { "11110","10001","10001","11110","10000","10000","10000" };
    static const char* U[] = { "10001","10001","10001","10001","10001","10001","01110" };
    static const char* M[] = { "10001","11011","10101","10101","10001","10001","10001" };
    static const char* A[] = { "01110","10001","10001","11111","10001","10001","10001" };
    static const char* S[] = { "01111","10000","10000","01110","00001","00001","11110" };
    static const char* T[] = { "11111","00100","00100","00100","00100","00100","00100" };
    static const char* L[] = { "10000","10000","10000","10000","10000","10000","11111" };
    static const char* N[] = { "10001","11001","10101","10011","10001","10001","10001" };
    static const char* E[] = { "11111","10000","10000","11110","10000","10000","11111" };
    static const char* I[] = { "11111","00100","00100","00100","00100","00100","11111" };
    static const char* O[] = { "01110","10001","10001","10001","10001","10001","01110" };
    static const char* D[] = { "11110","10001","10001","10001","10001","10001","11110" };
    static const char* R[] = { "11110","10001","10001","11110","10100","10010","10001" };
    const char** data = nullptr;
    switch (letter) {
    case 'P': data = P;break; case 'U':data = U;break; case 'M':data = M;break;
    case 'A':data = A;break; case 'S':data = S;break; case 'T':data = T;break;
    case 'L':data = L;break; case 'N':data = N;break; case 'E':data = E;break;
    case 'I':data = I;break; case 'O':data = O;break; case 'D':data = D;break;
    case 'R':data = R;break;
    default:return "00000";
    }
    return data[row];
}

// Dibuja una palabra en el marcador.
void drawScoreWord(GLint modelLoc, GLuint VAO, const glm::mat4& matrix,
    const std::string& word, float x, float y, float z, float dot, GLuint texture)
{
    for (size_t k = 0; k < word.length(); ++k) {
        for (int row = 0; row < 7; ++row) {
            const char* pattern = getScoreGlyph(word[k], row);
            for (int col = 0; col < 5; ++col) {
                if (pattern[col] != '1') continue;
                drawCube(modelLoc, VAO, matrix,
                    glm::vec3(x + (k * 6 + col) * dot, y - row * dot, z),
                    glm::vec3(dot * 0.85f, dot * 0.85f, 0.045f), texture);
            }
        }
    }
}

// Dibuja el marcador y los resultados de penales.
void drawMatchScoreboard(GLint modelLoc, GLuint VAO, const glm::mat4& scene,
    const BallMesh& whiteMesh, const BallMesh& blackMesh,
    GLuint dark, GLuint white, GLuint gold, GLuint red, GLuint green, GLuint pending)
{
    glm::mat4 board = glm::translate(scene, glm::vec3(0.0f, 0.0f, -70.0f));
    drawCube(modelLoc, VAO, board, glm::vec3(-7.5f, 5.8f, 0),
        glm::vec3(0.45f, 11.6f, 0.45f), dark);
    drawCube(modelLoc, VAO, board, glm::vec3(7.5f, 5.8f, 0),
        glm::vec3(0.45f, 11.6f, 0.45f), dark);
    drawCube(modelLoc, VAO, board, glm::vec3(0.0f, 11.60f, 0),
        glm::vec3(18.0f, 7.40f, 0.55f), dark);
    drawScoreWord(modelLoc, VAO, board, "PUMAS", -7.6f, 14.2f, 0.35f, 0.15f, gold);
    drawScoreWord(modelLoc, VAO, board, "ATLANTE", 1.4f, 14.2f, 0.35f, 0.15f, white);
    drawScoreDigit(modelLoc, VAO, board, std::min(9, pumasScore), -5.7f, 10.9f, 0.40f, 1.15f, white);
    drawScoreDigit(modelLoc, VAO, board, std::min(9, atlanteScore), 5.5f, 10.9f, 0.40f, 1.15f, white);
    drawCube(modelLoc, VAO, board, glm::vec3(-0.2f, 12.1f, 0.42f),
        glm::vec3(0.25f, 0.25f, 0.10f), red);
    drawCube(modelLoc, VAO, board, glm::vec3(-0.2f, 11.3f, 0.42f),
        glm::vec3(0.25f, 0.25f, 0.10f), red);
    drawScoreDigit(modelLoc, VAO, board, (matchMinute / 10) % 10, -2.2f, 9.6f, 0.39f, 0.42f, gold);
    drawScoreDigit(modelLoc, VAO, board, matchMinute % 10, -1.55f, 9.6f, 0.39f, 0.42f, gold);
    drawCube(modelLoc, VAO, board, glm::vec3(-0.80f, 10.0f, 0.39f),
        glm::vec3(0.10f, 0.10f, 0.08f), gold);
    drawCube(modelLoc, VAO, board, glm::vec3(-0.80f, 10.35f, 0.39f),
        glm::vec3(0.10f, 0.10f, 0.08f), gold);
    drawScoreDigit(modelLoc, VAO, board, (matchSecond / 10) % 10, 0.04f, 9.6f, 0.39f, 0.42f, gold);
    drawScoreDigit(modelLoc, VAO, board, matchSecond % 10, 0.68f, 9.6f, 0.39f, 0.42f, gold);
    if (matchScene == SCENE_PENALTIES || matchScene == SCENE_FINAL) {
        drawScoreWord(modelLoc, VAO, board, "PEN", -7.8f, 9.7f, 0.39f, 0.13f, red);
        drawScoreDigit(modelLoc, VAO, board, std::min(9, penaltyGoals[0]), -4.45f, 9.15f, 0.4f, 0.42f, white);
        drawScoreDigit(modelLoc, VAO, board, std::min(9, penaltyGoals[1]), 4.45f, 9.15f, 0.4f, 0.42f, white);
        for (int team = 0; team < 2; ++team) {
            for (int attempt = 0; attempt < 5; ++attempt) {
                GLuint color = pending;
                if (attempt < penaltyTaken[team] && attempt < penaltyHistoryLimit) {
                    if (penaltyHistory[team][attempt] == 1) color = green;
                    else if (penaltyHistory[team][attempt] == 0) color = red;
                }
                float x = team == 0 ? -5.95f + attempt * 0.58f
                    : 3.63f + attempt * 0.58f;
                glm::vec3 dot(x, 8.80f, 0.45f);
                drawSolidSphere(modelLoc, board, dot, glm::vec3(0.24f, 0.24f, 0.11f),
                    whiteMesh, blackMesh, white);
                dot.z += 0.085f;
                drawSolidSphere(modelLoc, board, dot, glm::vec3(0.175f, 0.175f, 0.11f),
                    whiteMesh, blackMesh, color);
            }
            if (penaltyTaken[team] > 5) {
                int extraStart = std::max(5, penaltyTaken[team] - 5);
                for (int extra = 0; extra < 5; ++extra) {
                    int index = extraStart + extra;
                    if (index >= penaltyTaken[team] || index >= penaltyHistoryLimit) break;
                    GLuint color = penaltyHistory[team][index] == 1 ? green : red;
                    float x = team == 0 ? -5.95f + extra * 0.58f
                        : 3.63f + extra * 0.58f;
                    glm::vec3 dot(x, 8.24f, 0.45f);
                    drawSolidSphere(modelLoc, board, dot, glm::vec3(0.15f, 0.15f, 0.10f),
                        whiteMesh, blackMesh, white);
                    dot.z += 0.08f;
                    drawSolidSphere(modelLoc, board, dot, glm::vec3(0.11f, 0.11f, 0.08f),
                        whiteMesh, blackMesh, color);
                }
            }
        }
    }
}

// Crea el efecto visual para cambiar de escena.
GLuint createFadeProgram()
{
    const char* vs = "#version 330 core\nlayout(location=0) in vec2 p;\nvoid main(){gl_Position=vec4(p,0.0,1.0);}\n";
    const char* fs = "#version 330 core\nout vec4 c;\nuniform float fadeAlpha;\nvoid main(){c=vec4(0.0,0.0,0.0,fadeAlpha);}\n";
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vertex, 1, &vs, nullptr);
    glShaderSource(fragment, 1, &fs, nullptr);
    glCompileShader(vertex);
    glCompileShader(fragment);
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    return program;
}

// Dibuja la capa de transicion en pantalla.
void drawFadeOverlay(GLuint program, GLuint vao)
{
    if (fadeAlpha <= 0.0f) return;
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(program);
    glUniform1f(glGetUniformLocation(program, "fadeAlpha"), fadeAlpha);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}
