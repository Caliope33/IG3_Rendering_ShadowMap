#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <iostream>

#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
//#define TINYGLTF_IMPLEMENTATION
//#define STB_IMAGE_IMPLEMENTATION
//#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"
#include "InputHandler.h"
#include "Model.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f; 
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
glm::vec3 lightPos2(-2.0f,1.2f,1.0f);

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    //For callbacks: 
    //Utilisation of Camera* cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    glfwSetWindowUserPointer(window, &camera);

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader MaterialShader("materials.vs", "materials.fs");
    Shader lightShader("1.light_cube.vs", "1.light_cube.fs");

    Model Cube1("Box.gltf");//load the file and creates VAO/VBO
    Model Cube2("Box.gltf");//on peut aussi mettre un cylindre
    Model Light1("Box.gltf");
    Model Light2("Box.gltf");
    
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);// input

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- Rendu de l'objet PBR ---
        MaterialShader.use();
        //UPDATE Uniforms in materials.fs
        MaterialShader.setInt("u_LightCount",2);

        MaterialShader.setVec3("u_LightPos[0]", lightPos);
        MaterialShader.setVec3("u_LightPos[1]", lightPos2);

        MaterialShader.setVec3("u_CameraPos", camera.Position);
        
        //--- Rendu de l'objet PBR 1 ---
        //PBR Parameters
        MaterialShader.setVec3("u_BaseColor", 2.0f, 0.5f, 1.0f);//Lilac
        MaterialShader.setFloat("u_Metallic", 0.8f);   // Très métallique
        MaterialShader.setFloat("u_Roughness", 0.3f);  // Assez lisse
        
        glm::mat4 projection=glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10.0f); //100.0f
        glm::mat4 view = camera.GetViewMatrix();
        MaterialShader.setMat4("projection", projection);
        MaterialShader.setMat4("view", view);
        glm::mat4 model1 = glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, 0.0f, 0.0f));
        MaterialShader.setMat4("model", model1);
        Cube1.Draw();

        //--- Rendu de l'objet PBR 2 ---
        // On réutilise le même shader, seuls le modèle et les paramètres PBR changent
        glm::mat4 model2 = glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, 0.0f, 0.0f));
        MaterialShader.setMat4("model", model2);
        MaterialShader.setVec3("u_BaseColor", 0.5f, 1.0f, 0.2f); // couleur différente
        MaterialShader.setFloat("u_Metallic", 0.1f);
        MaterialShader.setFloat("u_Roughness", 0.8f);
        Cube2.Draw();

        // --- Rendu de la lampe ---
        lightShader.use();
        lightShader.setMat4("projection", projection);
        lightShader.setMat4("view", view);
        glm::mat4 lightModel1 = glm::translate(glm::mat4(1.0f), lightPos);
        lightModel1 = glm::scale(lightModel1, glm::vec3(0.1f));
        lightShader.setMat4("model", lightModel1);
        Light1.Draw();

        glm::mat4 lightModel2 = glm::translate(glm::mat4(1.0f), lightPos2);
        lightModel2 = glm::scale(lightModel2, glm::vec3(0.1f));
        lightShader.setMat4("model", lightModel2);
        Light2.Draw();


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}