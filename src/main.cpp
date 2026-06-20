
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>


void framebuffer_callback_size(GLFWwindow *window, int w_size, int h_size){
    glViewport(0, 0, w_size, h_size);
}

int main(){

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "ImGUI tutorial", nullptr, nullptr);

    if(window == nullptr){
        std::cout << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "Failed to initialize glad\n";
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_callback_size);

    //create window with graphics contextz
    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());

    //setup dear imgui context 
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();    
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;


    //setup styling
    ImGui::StyleColorsDark();

    //setup scaling
    ImGuiStyle &style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

    //setup renderer backend
    ImGui_ImplGlfw_InitForOpenGL(window, true);

    ImGui_ImplOpenGL3_Init("#version 330"); 

    bool isShowDemo = false;
    ImVec4 color;

    while(!glfwWindowShouldClose(window)){

        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if(isShowDemo){
            ImGui::ShowDemoWindow(&isShowDemo);
        }

        {
            static float f = 0.0f;
            static int counter = 0;
            
            //Title of the window
            ImGui::Begin("Hello World");

            //Text inside the window
            ImGui::Text("This is just tutorial");

            //checkbox
            ImGui::Checkbox("Enable Demo Window", &isShowDemo);
            
            //Slider float
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
            ImGui::Text("Float = %f", f);
            
            //select background color
            ImGui::ColorEdit4("Background Color", (float*)&color);
            
            //button
            if(ImGui::Button("Counter"))
                counter++;
            ImGui::Text("Counter = %d", counter);

            //every window must have an end
            ImGui::End(); 
        }

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        glViewport(0, 0, display_w, display_h);
        glClearColor(color.x, color.y, color.z, color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

}
