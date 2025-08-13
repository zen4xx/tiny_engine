#include "GLFW/glfw3.h"
#include "glm/trigonometric.hpp"
#include "tiny_engine.h"

class Camera {
public:
    // Параметры камеры
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // Углы Эйлера
    float Yaw;
    float Pitch;

    // Настройки движения
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // Конструктор
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),
           glm::vec3 up       = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw          = -90.0f,
           float pitch        =  0.0f)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
          MovementSpeed(5.0f),
          MouseSensitivity(0.1f),
          Zoom(45.0f)
    {
        Position = position;
        WorldUp  = up;
        Yaw      = yaw;
        Pitch    = pitch;
        updateCameraVectors();
    }

    // Возвращает матрицу вида
    glm::mat4 GetViewMatrix() const {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // Обработка клавиатуры
    void ProcessKeyboard(int key, float deltaTime) {
        float velocity = MovementSpeed * deltaTime;
        if (key == GLFW_KEY_W)
            Position += Front * velocity;
        if (key == GLFW_KEY_S)
            Position -= Front * velocity;
        if (key == GLFW_KEY_A)
            Position -= Right * velocity;
        if (key == GLFW_KEY_D)
            Position += Right * velocity;
        if (key == GLFW_KEY_SPACE)
            Position += WorldUp * velocity;
        if (key == GLFW_KEY_LEFT_SHIFT)
            Position -= WorldUp * velocity;
    }

    // Обработка движения мыши
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw   += xoffset;
        Pitch += yoffset;

        if (constrainPitch) {
            if (Pitch > 89.0f)  Pitch = 89.0f;
            if (Pitch < -89.0f) Pitch = -89.0f;
        }
        updateCameraVectors();
    }

    // Обработка прокрутки колеса мыши
    void ProcessMouseScroll(float yoffset) {
        Zoom -= yoffset;
        if (Zoom < 1.0f)   Zoom = 1.0f;
        if (Zoom > 45.0f)  Zoom = 45.0f;
    }

private:
    // Пересчитывает векторы направления на основе углов Эйлера
    void updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);

        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up    = glm::normalize(glm::cross(Right, Front));
    }
};
