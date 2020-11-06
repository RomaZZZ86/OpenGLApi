/*
    src/example1.cpp -- C++ version of an example application that shows
    how to use the various widget classes. For a Python implementation, see
    '../python/example1.py'.

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include"main.h"


#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include"creatFont.h"
#include"butRect.h"
//#include"GuiExp.h"


using namespace cv;
using namespace nanogui;

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::pair;
using std::to_string;

//  онстанты
 unsigned int SCR_WIDTH = 800;
 unsigned int SCR_HEIGHT = 600;

 unsigned int VBO, VAO, EBO;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);



class MyGLCanvas : public GLCanvas {
public:
    MyGLCanvas(Widget* parent) : GLCanvas(parent), mRotation(Vector3f(0.25f, 0.5f, 0.33f)) {
       

        mShader.init(
            /* An identifying name */
            "a_simple_shader",

            /* Vertex shader */
            "#version 330\n"
            "uniform mat4 modelViewProj;\n"
            "in vec3 position;\n"
            "in vec3 color;\n"
            "out vec4 frag_color;\n"
            "void main() {\n"
            "    frag_color = 3.0 * modelViewProj * vec4(color, 1.0);\n"
            "    gl_Position = modelViewProj * vec4(position, 1.0);\n"
            "}",

            /* Fragment shader */
            "#version 330\n"
            "out vec4 color;\n"
            "in vec4 frag_color;\n"
            "void main() {\n"
            "    color = frag_color;\n"
            "}"
        );

        MatrixXu indices(3, 12); /* Draw a cube */
        indices.col(0) << 0, 1, 3;
        indices.col(1) << 3, 2, 1;
        indices.col(2) << 3, 2, 6;
        indices.col(3) << 6, 7, 3;
        indices.col(4) << 7, 6, 5;
        indices.col(5) << 5, 4, 7;
        indices.col(6) << 4, 5, 1;
        indices.col(7) << 1, 0, 4;
        indices.col(8) << 4, 0, 3;
        indices.col(9) << 3, 7, 4;
        indices.col(10) << 5, 6, 2;
        indices.col(11) << 2, 1, 5;

        MatrixXf positions(3, 8);
        positions.col(0) << -1, 1, 1;
        positions.col(1) << -1, 1, -1;
        positions.col(2) << 1, 1, -1;
        positions.col(3) << 1, 1, 1;
        positions.col(4) << -1, -1, 1;
        positions.col(5) << -1, -1, -1;
        positions.col(6) << 1, -1, -1;
        positions.col(7) << 1, -1, 1;

        MatrixXf colors(3, 12);
        colors.col(0) << 1, 0, 0;
        colors.col(1) << 0, 1, 0;
        colors.col(2) << 1, 1, 0;
        colors.col(3) << 0, 0, 1;
        colors.col(4) << 1, 0, 1;
        colors.col(5) << 0, 1, 1;
        colors.col(6) << 1, 1, 1;
        colors.col(7) << 0.5, 0.5, 0.5;
        colors.col(8) << 1, 0, 0.5;
        colors.col(9) << 1, 0.5, 0;
        colors.col(10) << 0.5, 1, 0;
        colors.col(11) << 0.5, 1, 0.5;

        mShader.bind();
        mShader.uploadIndices(indices);

        mShader.uploadAttrib("position", positions);
        mShader.uploadAttrib("color", colors);
    }

    ~MyGLCanvas() {
        mShader.free();
    }

    void setRotation(nanogui::Vector3f vRotation) {
        mRotation = vRotation;
    }

    virtual void drawGL() override {
       

        mShader.bind();

        Matrix4f mvp;
        mvp.setIdentity();
        float fTime = (float)glfwGetTime();
        mvp.topLeftCorner<3, 3>() = Eigen::Matrix3f(Eigen::AngleAxisf(mRotation[0] * fTime, Vector3f::UnitX()) *
            Eigen::AngleAxisf(mRotation[1] * fTime, Vector3f::UnitY()) *
            Eigen::AngleAxisf(mRotation[2] * fTime, Vector3f::UnitZ())) * 0.25f;

        mShader.setUniform("modelViewProj", mvp);

        glEnable(GL_DEPTH_TEST);
        /* Draw 12 triangles starting at index 0 */
        mShader.drawIndexed(GL_TRIANGLES, 0, 12);
        glDisable(GL_DEPTH_TEST);
    }

private:
    GLShader mShader;
    Eigen::Vector3f mRotation;
};

class GLTexture {
public:
    using handleType = std::unique_ptr<uint8_t[], void(*)(void*)>;
    GLTexture() = default;
    GLTexture(const std::string& textureName)
        : mTextureName(textureName), mTextureId(0) {}

    GLTexture(const std::string& textureName, GLint textureId)
        : mTextureName(textureName), mTextureId(textureId) {}

    GLTexture(const GLTexture& other) = delete;
    GLTexture(GLTexture&& other) noexcept
        : mTextureName(std::move(other.mTextureName)),
        mTextureId(other.mTextureId) {
        other.mTextureId = 0;
    }
    GLTexture& operator=(const GLTexture& other) = delete;
    GLTexture& operator=(GLTexture&& other) noexcept {
        mTextureName = std::move(other.mTextureName);
        std::swap(mTextureId, other.mTextureId);
        return *this;
    }
    ~GLTexture() noexcept {
        if (mTextureId)
            glDeleteTextures(1, &mTextureId);
    }

    GLuint texture() const { return mTextureId; }
    const std::string& textureName() const { return mTextureName; }

    /**
    *  Load a file in memory and create an OpenGL texture.
    *  Returns a handle type (an std::unique_ptr) to the loaded pixels.
    */
    handleType load(const std::string& fileName) {
        if (mTextureId) {
            glDeleteTextures(1, &mTextureId);
            mTextureId = 0;
        }
        int force_channels = 0;
        int w, h, n;
        handleType textureData(stbi_load(fileName.c_str(), &w, &h, &n, force_channels), stbi_image_free);
        if (!textureData)
            throw std::invalid_argument("Could not load texture data from file " + fileName);
        glGenTextures(1, &mTextureId);
        glBindTexture(GL_TEXTURE_2D, mTextureId);
        GLint internalFormat;
        GLint format;
        switch (n) {
        case 1: internalFormat = GL_R8; format = GL_RED; break;
        case 2: internalFormat = GL_RG8; format = GL_RG; break;
        case 3: internalFormat = GL_RGB8; format = GL_RGB; break;
        case 4: internalFormat = GL_RGBA8; format = GL_RGBA; break;
        default: internalFormat = 0; format = 0; break;
        }
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, textureData.get());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        return textureData;
    }

private:
    std::string mTextureName;
    GLuint mTextureId;
};


static GLuint bindCVMat2GLTexture(Mat& image) {
    if (image.empty()) {
        std::cout << "image emoty" << std::endl;


    }
    GLuint TexturID;
    //using handleType = std::unique_ptr<uint8_t[], void(*)(void*)>;
   
    //glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glGenTextures(1, &TexturID);
    glBindTexture(GL_TEXTURE_2D, TexturID);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);//GL_NEAREST GL_LINEAR GL_LINEAR_MIPMAP_LINEAR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);//GL_LINEAR

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );//GL_CLAMP_TO_EDGE
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );//GL_CLAMP_TO_EDGE

    float borderColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    cvtColor(image, image, COLOR_RGB2BGR);

    GLenum inpuColorFormat = GL_RGB;

    if(image.channels()==1) inpuColorFormat = GL_RGB;

    if (image.ptr()) {

        glTexImage2D(GL_TEXTURE_2D,
            0,
            GL_RGB,
            image.cols,
            image.rows,
            0,
            inpuColorFormat,
            GL_UNSIGNED_BYTE,
            image.ptr()
        );


        glGenerateMipmap(GL_TEXTURE_2D);
    }
    return TexturID;

}
/*
static void resize_callback(GLFWwindow* window, int new_width, int new_height) {
    glViewport(0, 0, SCR_WIDTH = new_width, SCR_HEIGHT = new_height);
    //glMatrixMode(GL_PROJECTION);
    //glLoadIdentity();
    //glOrtho(0, 0, SCR_WIDTH, SCR_HEIGHT, 0.0, 0.0, 100.0);
    //glMatrixMode(GL_MODELVIEW);
    
}

static void draw_frame( Mat& frame) {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glMatrixMode(GL_MODELVIEW);
    glEnable(GL_TEXTURE_2D);
    GLuint image_tex = bindCVMat2GLTexture(frame);

    //glBegin(GL_QUADS);
    glTexCoordP2ui(0, 0); glVertexP2ui(0, 0);
    glTexCoordP2ui(0, 1); glVertexP2ui(0, SCR_HEIGHT);
    glTexCoordP2ui(1, 1); glVertexP2ui(SCR_WIDTH, SCR_HEIGHT);
    glTexCoordP2ui(1, 0); glVertexP2ui(SCR_WIDTH, 0);
    glEndConditionalRender();

    glDeleteTextures(1, &image_tex);
    glDisable(GL_TEXTURE);


}
*/
static void init_rect() {
    // задание вершин (и буфера(ов)) и настройка вершинных атрибутов
  // ------------------------------------------------------------------
    float vertices[] = {
        // координаты          // цвета           // текстурные координаты
         1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // верхн€€ права€ вершина
         1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // нижн€€ права€ вершина
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // нижн€€ лева€ вершина
        -1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // верхн€€ лева€ вершина 
    };
    unsigned int indices[] = {
        0, 1, 3, // первый треугольник
        1, 2, 3  // второй треугольник
    };
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // координатные атрибуты
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // цветовые атрибуты
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // атрибуты текстурных координат
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
}

//=================================TEXT==============================================

//===============================================================================




enum test_enum {
    Item1 = 0,
    Item2,
    Item3,
    Item4
};

bool bvar = true;
int ivar = 12345678;
double dvar = 3.1415926;
float fvar = (float)dvar;
std::string strval = "A string";
test_enum enumval = Item2;
Color colval(0.5f, 0.5f, 0.7f, 1.f);

Screen* screen = nullptr;





int main()
{
    // glfw: инициализаци€ и конфигурирование
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    glfwWindowHint(GLFW_SAMPLES, 0);
    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);


#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // расскоментируйте эту строчку, если используете macOS
#endif

    // glfw создание окна
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL for Ravesli.com", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

     // glad: загрузка всех указателей на OpenGL-функции
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // компилирование нашей шейдерной программы
   // ------------------------------------
    Shader ourShader;
    ourShader.init("vertex.vs", "frag.fs");
    // Create a nanogui screen and pass the glfw pointer to initialize

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glfwSwapInterval(1);
    glfwSwapBuffers(window);

    screen = new Screen();
    screen->initialize(window, true);
   

    



    // Create nanogui gui
     //-------------------------CV2---------------------------
    Mat frame;
    //--- INITIALIZE VIDEOCAPTURE
    VideoCapture cap(0);


    cap.open(!cap.isOpened());
    // check if we succeeded
    if (!cap.isOpened()) {
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }
    //-------------------------------------------------------
    using imagesDataType = vector<pair<GLTexture, GLTexture::handleType>>;
    imagesDataType CVData;
    GLuint mTextureId;

    Mat image = imread("shema.jpg");

    
    //---------------------------------------------------------------


    bool enabled = true;
  //  ref<Widget> nui = new  Window(screen, "ISOTOP");
  //  nui->setLayout(new GroupLayout());
  //  nui->setPosition(Vector2i(6, 48));
 //   nui->setSize(Vector2i(900, 900));

    //auto  hui = new GLImageView(nui);

    //--------------------------------------------------
    init();

    ref<Widget> Pnui = new  Window(screen, "PANEL");
    Pnui->setLayout(new GroupLayout());
    Pnui->setPosition(Vector2i(6, 48));
    Pnui->setSize(Vector2i(640, 480));

    Widget* panel = new Widget(Pnui);
    panel->setLayout(new BoxLayout(Orientation::Horizontal,
        Alignment::Middle, 0, 20));

    Slider* slider = new Slider(panel);
    slider->setValue(0.5f);
    slider->setFixedWidth(300);

    new Label(Pnui, "Red: ");

    ref<Widget> butUI = new Button(panel,"PUSH");

    TabWidget* tabWidget = Pnui->add<TabWidget>();

    Widget* layer = tabWidget->createTab("Color Wheel");
    layer->setLayout(new GroupLayout());
    auto  hui = new GLImageView(layer);

    layer = tabWidget->createTab("Function Graph");
    layer->setLayout(new GroupLayout());

    layer->add<Label>("Function graph widget", "sans-bold");
    new CheckBox(layer);
    
    
    ButRect* butREC = new ButRect(layer);
    butREC->setBackgroundColor(Color(255, 0, 0, 255));

    

    //65std::cout << "SIZE LYOT " << layer->height() << std::endl;

   
   

     
    
   
    //----------------------------------------------------------------
    /*
    FormHelper* gui = new FormHelper(screen);
    ref<Window> nanoguiWindow = gui->addWindow(Eigen::Vector2i(320, 20), "GUI");
    gui->addGroup("Basic types");
    
    gui->addVariable("bool", bvar)->setTooltip("Test tooltip.");
    gui->addVariable("string", strval);
    
    gui->addGroup("Validating fields");
    gui->addVariable("int", ivar)->setSpinnable(true);
    gui->addVariable("float", fvar)->setTooltip("Test.");
    gui->addVariable("double", dvar)->setSpinnable(true);
 
    gui->addGroup("Complex types");
    gui->addVariable("Enumeration", enumval, enabled)->setItems({ "Item 1", "Item 2", "Item 3" ,"Item 4" });
    
    
    gui->addVariable("Color", colval)
        ->setFinalCallback([](const Color& c) {
        std::cout << "ColorPicker Final Callback: ["
            << c.r() << ", "
            << c.g() << ", "
            << c.b() << ", "
            << c.w() << "]" << std::endl;
            });

    gui->addGroup("Other widgets");
    gui->addButton("A button", []() { std::cout << "Button pressed." << std::endl; })->setTooltip("Testing a much longer tooltip, that will wrap around to new lines multiple times.");;
    */
    screen->setVisible(true);
    screen->performLayout();
    //nanoguiWindow->center();



    glfwSetCursorPosCallback(window,
        [](GLFWwindow*, double x, double y) {
            screen->cursorPosCallbackEvent(x, y);
           // butREC->cursorCallback((float)x, (float)y);
            
            
           // std::cout << "KEY CALLBACK" << x << "-" << y << std::endl;
        }
    );

  
    glfwSetMouseButtonCallback(window,
        [](GLFWwindow*, int button, int action, int modifiers) {
            screen->mouseButtonCallbackEvent(button, action, modifiers);
        }
    );
    
    glfwSetKeyCallback(window,
        [](GLFWwindow*, int key, int scancode, int action, int mods) {
            screen->keyCallbackEvent(key, scancode, action, mods);
        }
    );

    glfwSetCharCallback(window,
        [](GLFWwindow*, unsigned int codepoint) {
            screen->charCallbackEvent(codepoint);
        }
    );

    glfwSetDropCallback(window,
        [](GLFWwindow*, int count, const char** filenames) {
            screen->dropCallbackEvent(count, filenames);
        }
    );

    glfwSetScrollCallback(window,
        [](GLFWwindow*, double x, double y) {
            screen->scrollCallbackEvent(x, y);
        }
    );

    glfwSetFramebufferSizeCallback(window,
        [](GLFWwindow*, int width, int height) {
            screen->resizeCallbackEvent(width, height);
        }
    );
    //=========================WHILE===================================================================

  
   // Window* window = new Window(this, "GLCanvas Demo");
    
    /*
    nanoguiWindow->setPosition(Vector2i(150, 15));
    nanoguiWindow->setLayout(new GroupLayout());

    MyGLCanvas*  mCanvas = new MyGLCanvas(nanoguiWindow);
    mCanvas->setBackgroundColor({ 100, 100, 100, 255 });
    mCanvas->setSize({ 300, 300 });

    Widget* tools = new Widget(nanoguiWindow);
    tools->setLayout(new BoxLayout(Orientation::Vertical,
        Alignment::Minimum, 0, 5));

    Button* b0 = new Button(tools, "Random Color");
    mCanvas->setBackgroundColor(Vector4i(rand() % 256, rand() % 256, rand() % 256, 255));
    mCanvas->setRotation(Vector3f((rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f));
    b0->setCallback([nanoguiWindow]() {std::cout << "Button pressed III." << std::endl; });

    Button* b1 = new Button(tools, "Random Rotation");
    b1->setCallback([nanoguiWindow]() { std::cout << "Button pressed IIII." << std::endl; });

    //====================================================================================================
    ref<Window> imageWindow = gui->addWindow(Eigen::Vector2i(30, 20), "GUI");
    //auto imageWindow = new Window(nanoguiWindow, "Selected image");
    gui->addGroup("Selected image");
    imageWindow->setPosition(Vector2i(70, 15));
    imageWindow->setLayout(new GroupLayout());
 
    using imagesDataType = vector<pair<GLTexture, GLTexture::handleType>>;
    imagesDataType mImagesData;
    GLTexture img;
    auto data = img.load("icon8.png");
    mImagesData.emplace_back(std::move(img), std::move(data));
    auto imageView = new ImageView(imageWindow, mImagesData[0].first.texture());
    */
   //====================TEXTURGL==========================================================
   
    init_rect();

   
    /*
    ref<Window> cvWindow = gui->addWindow(Eigen::Vector2i(300, 200), "GUI");
    //auto imageWindow = new Window(nanoguiWindow, "Selected image");
    gui->addGroup("CV image");
    cvWindow->setPosition(Vector2i(170, 150));
    cvWindow->setLayout(new GroupLayout());

    */

    //-------------------------------------------------------------------------------------------

   
   

   

    // CVData.emplace_back(std::move(mTextureId), std::move(mTextureId));
    // auto cvView = new ImageView(cvWindow, mTextureId);
    /*
     auto but = new Button(cvWindow,"ST", ENTYPO_ICON_ROCKET);
     
     but->setBackgroundColor(Color(255, 0, 0, 100));
     but->fixedWidth();
     but->setTooltip("This button has a fairly long tooltip. It is so long, in "
         "fact, that the shown text will span several lines.");

     new Label(cvWindow, "Toggle buttons", "sans-bold");
    // but->preferredSize(&stGui);
     
     Widget* toolsD = new Widget(cvWindow);
     toolsD->setLayout(new BoxLayout(Orientation::Vertical,
         Alignment::Middle, 0, 6));
   
     auto b = new Button(cvWindow, "ERIS");
     b = new ToolButton(toolsD, ENTYPO_ICON_CLOUD);
     b = new ToolButton(toolsD, ENTYPO_ICON_CONTROLLER_FAST_FORWARD);
     b = new ToolButton(toolsD, ENTYPO_ICON_COMPASS);
     b = new ToolButton(toolsD, ENTYPO_ICON_INSTALL);
     
     new Label(cvWindow, "Popup buttons", "sans-bold");
     PopupButton* popupBtn = new PopupButton(cvWindow, "Popup", ENTYPO_ICON_EXPORT);
     Popup* popup = popupBtn->popup();
     popup->setLayout(new GroupLayout());
     new Label(popup, "Arbitrary widgets can be placed here");
     new CheckBox(popup, "A check box");

     // popup right
     popupBtn = new PopupButton(popup, "Recursive popup", ENTYPO_ICON_FLASH);
     Popup* popupRight = popupBtn->popup();
     popupRight->setLayout(new GroupLayout());
     new CheckBox(popupRight, "Another check box");
     // popup left
     popupBtn = new PopupButton(popup, "Recursive popup", ENTYPO_ICON_FLASH);
     popupBtn->setSide(Popup::Side::Left);
     Popup* popupLeft = popupBtn->popup();
     popupLeft->setLayout(new GroupLayout());
     new CheckBox(popupLeft, "Another check box");


     auto windowS = new Window(screen, "Basic widgets");
     windowS->setPosition(Vector2i(200, 15));
     windowS->setLayout(new GroupLayout());

     new Label(windowS, "Message dialog", "sans-bold");
     tools = new Widget(windowS);
     tools->setLayout(new BoxLayout(Orientation::Horizontal,
         Alignment::Middle, 0, 6));

     auto bb = new Button(tools, "Info");
     bb->setCallback([&] {
         auto dlg = new MessageDialog(windowS, MessageDialog::Type::Information, "Title", "This is an information message");
         dlg->setCallback([](int result) { cout << "Dialog result: " << result << endl; });
         });
     bb = new Button(tools, "Warn");
     bb->setCallback([&] {
         auto dlg = new MessageDialog(windowS, MessageDialog::Type::Warning, "Title", "This is a warning message");
         dlg->setCallback([](int result) { cout << "Dialog result: " << result << endl; });
         });
     bb = new Button(tools, "Ask");
     bb->setCallback([&] {
         auto dlg = new MessageDialog(windowS, MessageDialog::Type::Warning, "Title", "This is a question message", "Yes", "No", true);
         dlg->setCallback([](int result) { cout << "Dialog result: " << result << endl; });
         });

      
     new Label(windowS, "Slider and text box", "sans-bold");

     Widget* panel = new Widget(windowS);
     panel->setLayout(new BoxLayout(Orientation::Horizontal,
         Alignment::Middle, 0, 20));

     Slider* slider = new Slider(panel);
     slider->setValue(0.5f);
     slider->setFixedWidth(80);

     TextBox* textBox = new TextBox(panel);
     textBox->setFixedSize(Vector2i(60, 25));
     textBox->setValue("50");
     textBox->setUnits("%");
     slider->setCallback([textBox](float value) {
         textBox->setValue(std::to_string((int)(value * 100)));
         });
     slider->setFinalCallback([&](float value) {
         cout << "Final slider value: " << (int)(value * 100) << endl;
         });
     textBox->setFixedSize(Vector2i(60, 25));
     textBox->setFontSize(20);
     textBox->setAlignment(TextBox::Alignment::Right);


     TabWidget* tabWidget = windowS->add<TabWidget>();

     Widget* layer = tabWidget->createTab("Color Wheel");
     layer->setLayout(new GroupLayout());

     layer->add<Label>("Color wheel widget", "sans-bold");
     layer->add<ColorWheel>();

     //-----------------------
     //auto windTab=new  Window(screen, "Basic widgets");
      Window windTab(screen, "Basic widgets");
     //windTab->setPosition(Vector2i(100, 15));
     using tabDataType = vector<pair<GLTexture, GLTexture::handleType>>;
     tabDataType tabData;
     GLTexture imgD;
     auto dataE = imgD.load("icon8.png");
     tabData.emplace_back(std::move(imgD), std::move(dataE));
     new ImageView(&windTab, tabData[0].first.texture());
     
     layer = tabWidget->createTab("IMG");
     layer->setLayout(new GroupLayout());
     tabWidget->addTab("JYN", cvWindow);

     //-------------------------------------------------------------------

     layer = tabWidget->createTab("Function Graph");
     layer->setLayout(new GroupLayout());

     layer->add<Label>("Function graph widget", "sans-bold");

     Graph* graph = layer->add<Graph>("Some Function");

     graph->setHeader("E = 2.35e-3");
     graph->setFooter("Iteration 89");
     VectorXf& func = graph->values();
     func.resize(100);
     for (int i = 0; i < 100; ++i)
         func[i] = 0.5f * (0.5f * std::sin(i / 10.f) +
             0.5f * std::cos(i / 23.f) + 1);

     // Dummy tab used to represent the last tab button.
     tabWidget->createTab("+");
    
     // A simple counter.
     int counter = 1;
     tabWidget->setCallback([tabWidget, windowS, counter](int index) mutable {
         if (index == (tabWidget->tabCount() - 1)) {
             // When the "+" tab has been clicked, simply add a new tab.
             string tabName = "Dynamic " + to_string(counter);
             Widget* layerDyn = tabWidget->createTab(index, tabName);
             layerDyn->setLayout(new GroupLayout());
             layerDyn->add<Label>("Function graph widget", "sans-bold");
             Graph* graphDyn = layerDyn->add<Graph>("Dynamic function");

             graphDyn->setHeader("E = 2.35e-3");
             graphDyn->setFooter("Iteration " + to_string(index * counter));
             VectorXf& funcDyn = graphDyn->values();
             funcDyn.resize(100);
             for (int i = 0; i < 100; ++i)
                 funcDyn[i] = 0.5f *
                 std::abs((0.5f * std::sin(i / 10.f + counter) +
                     0.5f * std::cos(i / 23.f + 1 + counter)));
             ++counter;
             
             // We must invoke perform layout from the screen instance to keep everything in order.
             // This is essential when creating tabs dynamically.
             //BoxLayout 
             //performLayout();
             // Ensure that the newly added header is visible on screen
             tabWidget->ensureTabVisible(index);

         }
         });
     tabWidget->setActiveTab(0);

     
     Window* windowW= new Window(screen, "Grid of small widgets");
     windowW->setPosition(Vector2i(425, 300));

     auto cp = new ColorPicker(windowW, { 255, 120, 0, 255 });
     auto layout =
         new GridLayout(Orientation::Horizontal, 2,
             Alignment::Middle, 15, 5);
     layout->setColAlignment(
         { Alignment::Maximum, Alignment::Fill });
     layout->setSpacing(0, 10);
     windowW->setLayout(layout);
     windowW->setPosition(Vector2i(425, 500));
     new Label(windowW, "Combined: ");
     b = new Button(windowW, "ColorWheel", ENTYPO_ICON_500PX);
     new Label(windowW, "Red: ");
     auto redIntBox = new IntBox<int>(windowW);
     redIntBox->setEditable(false);
     new Label(windowW, "Green: ");
     auto greenIntBox = new IntBox<int>(windowW);
     greenIntBox->setEditable(false);
     new Label(windowW, "Blue: ");
     auto blueIntBox = new IntBox<int>(windowW);
     blueIntBox->setEditable(false);
     new Label(windowW, "Alpha: ");
     auto alphaIntBox = new IntBox<int>(windowW);
     cp->setCallback([b, redIntBox, blueIntBox, greenIntBox, alphaIntBox](const Color& c) {
         b->setBackgroundColor(c);
         b->setTextColor(c.contrastingColor());
         int red = (int)(c.r() * 255.0f);
         redIntBox->setValue(red);
         int green = (int)(c.g() * 255.0f);
         greenIntBox->setValue(green);
         int blue = (int)(c.b() * 255.0f);
         blueIntBox->setValue(blue);
         int alpha = (int)(c.w() * 255.0f);
         alphaIntBox->setValue(alpha);

         });

   
     // Checkbox widget 
     {
         
         new Label(windowW, "Checkbox :", "sans-bold");

         auto cb = new CheckBox(windowW, "Check me");
         cb->setFontSize(16);
         cb->setChecked(true);
     }

     new Label(windowW, "Combo box :", "sans-bold");
     ComboBox* cobo =
         new ComboBox(windowW, { "Item 1", "Item 2", "Item 3" });
     cobo->setFontSize(16);
     cobo->setFixedSize(Vector2i(100, 20));
     */
     //================================================================
    GLuint texture1;
    //-----------time---------------------------------
    double video_start_time = glfwGetTime();
    double video_end_time = 0.0;
    double frame_start_time = 0.0;

    Mat frameGL;
    // цикл рендеринга

   // Widget gts(cvWindow);
    

    //----------Freetype------------------------
    
    
    //---------------------------------------------
    while (!glfwWindowShouldClose(window))

    {
        frame_start_time= glfwGetTime();
       
        // ќбработка ввода
        processInput(window);
       
        screen->performLayout();

        if (!cap.read(frameGL)) {
            std::cout << "Cannot grad and frame." << std::endl;
            break;
        }
        
        //================ //наложение видео на GL=============================
       
       cv::resize(frameGL, image, Size(), 1, 1);
       mTextureId = bindCVMat2GLTexture(image);
       hui->imgView(mTextureId);
        
        //==============================================

        // ¬ыполнение рендеринга
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

       
       
        
        
        //=============================================
       
        //====================================================
        
        // –ендеринг видео
        flip(frameGL, frameGL, 0);
        texture1 = bindCVMat2GLTexture(frameGL);


        ourShader.use();
        glBindVertexArray(VAO);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT,0);
        glDeleteTextures(1, &texture1);
        //glDisable(GL_TEXTURE);
         
        //=======================================================

        

 
       
        screen->cursor();
       screen->drawContents();
       screen->drawWidgets();

        glfwSwapBuffers(window);
        glfwPollEvents();

      

       // cv:: imshow("Live", frameGL);
        if (waitKey(5) >= 0)
            break;
       
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    // glfw: завершение, освобождение всех ранее задействованных GLFW-ресурсов
    glfwTerminate();
    return 0;
}



//========================================================================================
// ќбработка всех событий ввода: запрос GLFW о нажатии/отпускании кнопки мыши в данном кадре и соответствующа€ обработка данных событий
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: вс€кий раз, когда измен€ютс€ размеры окна (пользователем или опер. системой), вызываетс€ данна€ функци€
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // убеждаемс€, что вьюпорт соответствует новым размерам окна; обратите внимание,
    // что ширина и высота будут значительно больше, чем указано на retina -диспле€х.
    glViewport(0, 0, width, height);
}
//===========================================================================================
