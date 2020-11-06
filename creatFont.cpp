#include "creatFont.h"

//#include <nanogui/window.h>
//#include <nanogui/screen.h>
//#include <nanogui/theme.h>
#include <cmath>
#include <Eigen/Geometry>


NAMESPACE_BEGIN(nanogui)


namespace {
    std::vector<std::string> tokenize(const std::string& string,
        const std::string& delim = "\n",
        bool includeEmpty = false) {
        std::string::size_type lastPos = 0, pos = string.find_first_of(delim, lastPos);
        std::vector<std::string> tokens;

        while (lastPos != std::string::npos) {
            std::string substr = string.substr(lastPos, pos - lastPos);
            if (!substr.empty() || includeEmpty)
                tokens.push_back(std::move(substr));
            lastPos = pos;
            if (lastPos != std::string::npos) {
                lastPos += 1;
                pos = string.find_first_of(delim, lastPos);
            }
        }

        return tokens;
    }

    constexpr char const* const defaultImageViewVertexShader =
        R"(#version 330
        uniform vec2 scaleFactor;
        uniform vec2 position;
        in vec2 vertex;
        out vec2 uv;
        void main() {
            uv = vertex;
            vec2 scaledVertex = (vertex * scaleFactor) + position;
            gl_Position  = vec4(2.0*scaledVertex.x - 1.0,
                                1.0 - 2.0*scaledVertex.y,
                                0.0, 1.0);

        })";

    constexpr char const* const defaultImageViewFragmentShader =
        R"(#version 330
        uniform sampler2D image;
        out vec4 color;
        in vec2 uv;
        void main() {
            color = texture(image, uv);
        })";

}

GLImageView::GLImageView(Widget* parent) : //, GLuint imageID
 
    Widget(parent), 
   // mImageID(imageID), 
    mScale(1.0f), mOffset(Vector2f::Zero()),
    mFixedScale(false), mFixedOffset(false), 
    mPixelInfoCallback(nullptr){



//
   
    

    //----------------------------------------------------------------
   
    ourShader.init("4.1.texture.vs", "4.1.texture.fs");
   
    init_rect();
    glUseProgram(mProgramShader);
    
    ourShader.use();
    ourShader.setInt("texture1", 0);

    FlagView = false;

    //std::cout << "SIZE--" << mImageSize.x() << std::endl;

   
}

void  GLImageView::imgView(GLuint imageID) 

{
    if (FlagView) {
        mImageID = imageID;
        updateImageParameters();
    }
    else
    {
        glDeleteTextures(1, &mImageID);
        glDeleteTextures(1, &imageID);
    }
    FlagView = false;
    
    /*
    mShader.init("ImageViewShader", defaultImageViewVertexShader,
        defaultImageViewFragmentShader);
   
    MatrixXu indices(3, 2);
    indices.col(0) << 0, 1, 2;
    indices.col(1) << 2, 3, 1;

    

    MatrixXf vertices(2, 4);
    vertices.col(0) << 0, 0;
    vertices.col(1) << 1, 0;
    vertices.col(2) << 0, 1;
    vertices.col(3) << 1, 1;
    */
   // mShader.bind();

   
    
   //mShader.uploadAttrib("indices", indices, 1); 
  // mShader.uploadAttrib("vertex", vertices,1);
    
  
  //  mShader.freeAttrib("ImageViewShader");
    
  
    
}

 void GLImageView::init_rect() {
    // задание вершин (и буфера(ов)) и настройка вершинных атрибутов
  // ------------------------------------------------------------------
     
    float vertices[] = {
        // координаты          // текстурные координаты
           1.0f,  1.0f, 0.0f,   1.0f, 1.0f, // верхняя правая вершина
           1.0f, -1.0f, 0.0f,   1.0f, 0.0f, // нижняя правая вершина
          -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, // нижняя левая вершина
          -1.0f,  1.0f, 0.0f,   0.0f, 1.0f  // верхняя левая вершина
          
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // цветовые атрибуты
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

GLImageView::~GLImageView() {
    mShader.free();
   glDeleteVertexArrays(1, &VAO);
   glDeleteBuffers(1, &VBO);
   glDeleteBuffers(1, &EBO);
}

void GLImageView::bindImage(GLuint imageId) {
    mImageID = imageId;
    updateImageParameters();
    fit();
}

Vector2f GLImageView::imageCoordinateAt(const Vector2f& position) const {
    auto imagePosition = position - mOffset;
    return imagePosition / mScale;
}

Vector2f GLImageView::clampedImageCoordinateAt(const Vector2f& position) const {
    auto imageCoordinate = imageCoordinateAt(position);
    return imageCoordinate.cwiseMax(Vector2f::Zero()).cwiseMin(imageSizeF());
}

Vector2f GLImageView::positionForCoordinate(const Vector2f& imageCoordinate) const {
    return mScale * imageCoordinate + mOffset;
}

void GLImageView::setImageCoordinateAt(const Vector2f& position, const Vector2f& imageCoordinate) {
    // Calculate where the new offset must be in order to satisfy the image position equation.
    // Round the floating point values to balance out the floating point to integer conversions.
    mOffset = position - (imageCoordinate * mScale);

    // Clamp offset so that the image remains near the screen.
    mOffset = mOffset.cwiseMin(sizeF()).cwiseMax(-scaledImageSizeF());
}

void GLImageView::center() {
    mOffset = (sizeF() - scaledImageSizeF()) / 2;
}

void GLImageView::fit() {
    // Calculate the appropriate scaling factor.
    mScale = (sizeF().cwiseQuotient(imageSizeF())).minCoeff();
    center();
}

void GLImageView::setScaleCentered(float scale) {
    auto centerPosition = sizeF() / 2;
    auto p = imageCoordinateAt(centerPosition);
    mScale = scale;
    setImageCoordinateAt(centerPosition, p);
}

void GLImageView::moveOffset(const Vector2f& delta) {
    // Apply the delta to the offset.
    mOffset += delta;

    // Prevent the image from going out of bounds.
    auto scaledSize = scaledImageSizeF();
    if (mOffset.x() + scaledSize.x() < 0)
        mOffset.x() = -scaledSize.x();
    if (mOffset.x() > sizeF().x())
        mOffset.x() = sizeF().x();
    if (mOffset.y() + scaledSize.y() < 0)
        mOffset.y() = -scaledSize.y();
    if (mOffset.y() > sizeF().y())
        mOffset.y() = sizeF().y();
}

void GLImageView::zoom(int amount, const Vector2f& focusPosition) {
    auto focusedCoordinate = imageCoordinateAt(focusPosition);
    float scaleFactor = std::pow(mZoomSensitivity, amount);
    mScale = std::max(0.01f, scaleFactor * mScale);
    setImageCoordinateAt(focusPosition, focusedCoordinate);
}

bool GLImageView::mouseDragEvent(const Vector2i& p, const Vector2i& rel, int button, int ) { //modifiers
    if ((button & (1 << GLFW_MOUSE_BUTTON_LEFT)) != 0 && !mFixedOffset) {
        setImageCoordinateAt((p + rel).cast<float>(), imageCoordinateAt(p.cast<float>()));
        return true;
    }
    return false;
}

bool GLImageView::gridVisible() const {
    return (mGridThreshold != -1) && (mScale > mGridThreshold);
}

bool GLImageView::pixelInfoVisible() const {
    return mPixelInfoCallback && (mPixelInfoThreshold != -1) && (mScale > mPixelInfoThreshold);
}

bool GLImageView::helpersVisible() const {
    return gridVisible() || pixelInfoVisible();
}

bool GLImageView::scrollEvent(const Vector2i& p, const Vector2f& rel) {
    if (mFixedScale)
        return false;
    float v = rel.y();
    if (std::abs(v) < 1)
        v = std::copysign(1.f, v);
    zoom(v, (p - position()).cast<float>());
    return true;
}
//scancode
bool GLImageView::keyboardEvent(int key, int , int action, int modifiers) {
    if (action) {
        switch (key) {
        case GLFW_KEY_LEFT:
            if (!mFixedOffset) {
                if (GLFW_MOD_CONTROL & modifiers)
                    moveOffset(Vector2f(30, 0));
                else
                    moveOffset(Vector2f(10, 0));
                return true;
            }
            break;
        case GLFW_KEY_RIGHT:
            if (!mFixedOffset) {
                if (GLFW_MOD_CONTROL & modifiers)
                    moveOffset(Vector2f(-30, 0));
                else
                    moveOffset(Vector2f(-10, 0));
                return true;
            }
            break;
        case GLFW_KEY_DOWN:
            if (!mFixedOffset) {
                if (GLFW_MOD_CONTROL & modifiers)
                    moveOffset(Vector2f(0, -30));
                else
                    moveOffset(Vector2f(0, -10));
                return true;
            }
            break;
        case GLFW_KEY_UP:
            if (!mFixedOffset) {
                if (GLFW_MOD_CONTROL & modifiers)
                    moveOffset(Vector2f(0, 30));
                else
                    moveOffset(Vector2f(0, 10));
                return true;
            }
            break;
        }
    }
    return false;
}

bool GLImageView::keyboardCharacterEvent(unsigned int codepoint) {
    switch (codepoint) {
    case '-':
        if (!mFixedScale) {
            zoom(-1, sizeF() / 2);
            return true;
        }
        break;
    case '+':
        if (!mFixedScale) {
            zoom(1, sizeF() / 2);
            return true;
        }
        break;
    case 'c':
        if (!mFixedOffset) {
            center();
            return true;
        }
        break;
    case 'f':
        if (!mFixedOffset && !mFixedScale) {
            fit();
            return true;
        }
        break;
    case '1': case '2': case '3': case '4': case '5':
    case '6': case '7': case '8': case '9':
        if (!mFixedScale) {
            setScaleCentered(1 << (codepoint - '1'));
            return true;
        }
        break;
    default:
        return false;
    }
    return false;
}

///ctx
Vector2i GLImageView::preferredSize(NVGcontext* ) const {
    return mImageSize;
}

void GLImageView::performLayout(NVGcontext* ctx) {
    Widget::performLayout(ctx);
    center();
}

void GLImageView::draw(NVGcontext* ctx) {
    FlagView = true;

    Widget::draw(ctx);
    nvgEndFrame(ctx); // Flush the NanoVG draw stack, not necessary to call nvgBeginFrame afterwards.

    drawImageBorder(ctx);
   

    // Calculate several variables that need to be send to OpenGL in order for the image to be
    // properly displayed inside the widget.
    const Screen* screen = dynamic_cast<const Screen*>(this->window()->parent());
    assert(screen);
    Vector2f screenSize = screen->size().cast<float>();
    Vector2f scaleFactor = mScale * imageSizeF().cwiseQuotient(screenSize);
    Vector2f positionInScreen = absolutePosition().cast<float>();
    Vector2f positionAfterOffset = positionInScreen + mOffset;

    Vector2f PositionF = positionAfterOffset.cwiseQuotient(screenSize);
    Vector2f imagePosition = Vector2f(PositionF.x(), PositionF.y());
    glEnable(GL_SCISSOR_TEST);
    float r = screen->pixelRatio();
    glScissor(positionInScreen.x() * r,
        (screenSize.y() - positionInScreen.y() - size().y()) * r,
        size().x() * r, size().y() * r);
    //mShader.bind();
   // glUseProgram(mProgramShader);
    //glBindVertexArray(mImageID);
    //-----------------------------------------------------------------------
   
    //ourShader.use();
    
   
   // std::cout << "GLUINT DRAW " << mSize.x()<< "--" << mSize.y() << std::endl;
    //------------------------------------------------------------------------
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mImageID);


    // создаем преобразование
    glm::mat4 transform = glm::mat4(1.0f); // сначала инициализируем единичную матрицу
    transform = glm::translate(transform, glm::vec3((float)imagePosition.x(), (float)imagePosition.y() , 0.0f));
    //transform = glm::rotate(transform, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
    transform = glm::scale(transform, glm::vec3((float)scaleFactor.x() , (float)scaleFactor.y(), 0.0));

    std::cout << "GLUINT DRAW POS" << imagePosition.x() << "--" << imagePosition.y() << std::endl;
    /*
    mShader.setUniform("image", 0);
    mShader.setUniform("scaleFactor", scaleFactor);
    mShader.setUniform("position", imagePosition);

   
    GLint vertexColorLocation = glGetUniformLocation(mProgramShader, "image");
    glUniform1i(vertexColorLocation, 0);
    
    vertexColorLocation = glGetUniformLocation(mProgramShader, "scaleFactor");
    glUniform2f(vertexColorLocation, (float)scaleFactor.x(), (float)scaleFactor.y());


    vertexColorLocation = glGetUniformLocation(mProgramShader, "position");
    glUniform2f(vertexColorLocation, (float)imagePosition.x(), (float)imagePosition.y());
    */
    //mShader.drawIndexed(GL_TRIANGLES, 0, 2);
    //================================================================
    // получаем location uniform-переменной матрицы и настраиваем её
    ourShader.use();
    unsigned int transformLoc = glGetUniformLocation(ourShader.ID, "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
    //=================================================================
     glBindVertexArray(VAO);
     glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
     glDeleteTextures(1, &mImageID);
    //======================================================================
     glDisable(GL_SCISSOR_TEST);

    if (helpersVisible())
        drawHelpers(ctx);

    drawWidgetBorder(ctx);

   //glDisable(GL_DEPTH_TEST);
   
}

void GLImageView::updateImageParameters() {
    // Query the width of the OpenGL texture.
    glBindTexture(GL_TEXTURE_2D, mImageID);
    GLint w, h;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
    mImageSize = Vector2i(w, h);
}

void GLImageView::drawWidgetBorder(NVGcontext* ctx) const {
    nvgBeginPath(ctx);
    nvgStrokeWidth(ctx, 1);
    nvgRoundedRect(ctx, mPos.x() + 0.5f, mPos.y() + 0.5f, mSize.x() - 1,
        mSize.y() - 1, 0);
    nvgStrokeColor(ctx, mTheme->mWindowPopup);
    nvgStroke(ctx);

    nvgBeginPath(ctx);
    nvgRoundedRect(ctx, mPos.x() + 0.5f, mPos.y() + 0.5f, mSize.x() - 1,
        mSize.y() - 1, mTheme->mButtonCornerRadius);
    nvgStrokeColor(ctx, mTheme->mBorderDark);
    nvgStroke(ctx);
}

void GLImageView::drawImageBorder(NVGcontext* ctx) const {
    nvgSave(ctx);
    nvgBeginPath(ctx);
    nvgScissor(ctx, mPos.x(), mPos.y(), mSize.x(), mSize.y());
    nvgStrokeWidth(ctx, 1.0f);
    Vector2i borderPosition = mPos + mOffset.cast<int>();
    Vector2i borderSize = scaledImageSizeF().cast<int>();
    nvgRect(ctx, borderPosition.x() - 0.5f, borderPosition.y() - 0.5f,
        borderSize.x() + 1, borderSize.y() + 1);
    nvgStrokeColor(ctx, Color(1.0f, 1.0f, 1.0f, 1.0f));
    nvgStroke(ctx);
    nvgResetScissor(ctx);
    nvgRestore(ctx);
}

void GLImageView::drawHelpers(NVGcontext* ctx) const {
    // We need to apply mPos after the transformation to account for the position of the widget
    // relative to the parent.
    Vector2f upperLeftCorner = positionForCoordinate(Vector2f::Zero()) + positionF();
    Vector2f lowerRightCorner = positionForCoordinate(imageSizeF()) + positionF();
    if (gridVisible())
        drawPixelGrid(ctx, upperLeftCorner, lowerRightCorner, mScale);
    if (pixelInfoVisible())
        drawPixelInfo(ctx, mScale);
}

void GLImageView::drawPixelGrid(NVGcontext* ctx, const Vector2f& upperLeftCorner,
    const Vector2f& lowerRightCorner, float stride) {
    nvgBeginPath(ctx);

    // Draw the vertical grid lines
    float currentX = upperLeftCorner.x();
    while (currentX <= lowerRightCorner.x()) {
        nvgMoveTo(ctx, std::round(currentX), std::round(upperLeftCorner.y()));
        nvgLineTo(ctx, std::round(currentX), std::round(lowerRightCorner.y()));
        currentX += stride;
    }

    // Draw the horizontal grid lines
    float currentY = upperLeftCorner.y();
    while (currentY <= lowerRightCorner.y()) {
        nvgMoveTo(ctx, std::round(upperLeftCorner.x()), std::round(currentY));
        nvgLineTo(ctx, std::round(lowerRightCorner.x()), std::round(currentY));
        currentY += stride;
    }

    nvgStrokeWidth(ctx, 1.0f);
    nvgStrokeColor(ctx, Color(1.0f, 1.0f, 1.0f, 0.2f));
    nvgStroke(ctx);
    
}

void GLImageView::drawPixelInfo(NVGcontext* ctx, float stride) const {
    // Extract the image coordinates at the two corners of the widget.
    Vector2i topLeft = clampedImageCoordinateAt(Vector2f::Zero())
        .unaryExpr([](float x) { return std::floor(x); })
        .cast<int>();

    Vector2i bottomRight = clampedImageCoordinateAt(sizeF())
        .unaryExpr([](float x) { return std::ceil(x); })
        .cast<int>();

    // Extract the positions for where to draw the text.
    Vector2f currentCellPosition =
        (positionF() + positionForCoordinate(topLeft.cast<float>()));

    float xInitialPosition = currentCellPosition.x();
    int xInitialIndex = topLeft.x();

    // Properly scale the pixel information for the given stride.
    auto fontSize = stride * mFontScaleFactor;
    static constexpr float maxFontSize = 30.0f;
    fontSize = fontSize > maxFontSize ? maxFontSize : fontSize;
    nvgBeginPath(ctx);
    nvgFontSize(ctx, fontSize);
    nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
    nvgFontFace(ctx, "sans");
    while (topLeft.y() != bottomRight.y()) {
        while (topLeft.x() != bottomRight.x()) {
            writePixelInfo(ctx, currentCellPosition, topLeft, stride, fontSize);
            currentCellPosition.x() += stride;
            ++topLeft.x();
        }
        currentCellPosition.x() = xInitialPosition;
        currentCellPosition.y() += stride;
        ++topLeft.y();
        topLeft.x() = xInitialIndex;
    }
}

void GLImageView::writePixelInfo(NVGcontext* ctx, const Vector2f& cellPosition,
    const Vector2i& pixel, float stride, float fontSize) const {
    auto pixelData = mPixelInfoCallback(pixel);
    auto pixelDataRows = tokenize(pixelData.first);

    // If no data is provided for this pixel then simply return.
    if (pixelDataRows.empty())
        return;

    nvgFillColor(ctx, pixelData.second);
    float yOffset = (stride - fontSize * pixelDataRows.size()) / 2;
    for (size_t i = 0; i != pixelDataRows.size(); ++i) {
        nvgText(ctx, cellPosition.x() + stride / 2, cellPosition.y() + yOffset,
            pixelDataRows[i].data(), nullptr);
        yOffset += fontSize;
    }
}

NAMESPACE_END(nanogui)

//================================================================================================================







/*

int render_text(const char* t, int x, int y,int fontSize ,FT_Face f) {//

	int ret = 0;
	GLuint* textures;
	size_t i;
	size_t length = strlen(t);

	FT_Set_Pixel_Sizes(f, 0, fontSize);

	FT_GlyphSlot g;
	if (f->glyph)  g = f->glyph;
	else {
		std::cout << "NO FONT" << std::endl;
		return 1;
	}


	//===========================================================================
	

	//===========================================================================
	textures = (GLuint*)malloc(sizeof(GLuint) * length);
	if (textures == NULL) {
		ret = -ENOMEM;
		goto done;
	}
	glGenTextures(length, textures);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	for (i = 0; i < length; ++i) {
		// if (FT_Load_Char(f, t[i], FT_LOAD_RENDER))continue;
		glActiveTexture(textures[i]);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, textures[i]);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//GL_NEAREST GL_LINEAR GL_LINEAR_MIPMAP_LINEAR
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//GL_LINEAR

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);//GL_CLAMP_TO_EDGE
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);//GL_CLAMP_TO_EDGE

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, g->bitmap.width, g->bitmap.rows, 0, GL_ALPHA, GL_UNSIGNED_BYTE, g->bitmap.buffer);
		//glBegin(GL_TRIANGLE_STRIP);
		glTexCoordP2ui(0, 0);
		glVertexP2ui(x + 0, y + 0);

		glTexCoordP2ui(g->bitmap.width, 0);
		glVertexP2ui(x + g->bitmap.width, y + 0);

		glTexCoordP2ui(0, g->bitmap.rows);
		glVertexP2ui(0, y + g->bitmap.rows);

		glTexCoordP2ui(g->bitmap.width, 0);
		glVertexP2ui(x + g->bitmap.width, y + 0);

		glTexCoordP2ui(g->bitmap.width, g->bitmap.rows);
		glVertexP2ui(x + g->bitmap.width, y + g->bitmap.rows);

		glTexCoordP2ui(0, g->bitmap.rows);
		glVertexP2ui(0, y + g->bitmap.rows);;
		// glEnd();
		glGenerateMipmap(GL_TEXTURE_2D);

	}

	//и рисуем текстуру 
	glDrawArrays(GL_TRIANGLES, 0, 12);

	glDeleteTextures(length, textures);
	free(textures);

done:
	return ret;

}

FT_Face  ftt(const char* libttf) {


	// здесь загружается файл шрифта 

	//char* path = (char*) new char[255];
	//sprintf(path, "assets/%s", libttf);
	//FT_New_Face(ft_library, path, 0, &face);
	


	FT_Library  library;   // handle to library     
	FT_Face     face=0;      // handle to face object 


	FT_Error error = FT_Init_FreeType(&library);


	//error = FT_New_Face(library,
	//	libttf //"/usr/share/fonts/truetype/arial.ttf"
	//	,
	//	0,
	//	&face);
	FT_New_Face(library, libttf, 0, &face);

	//delete[]  path;

	return face;
}


Font::Font(const char* ttf_file)
{
	 //Начнем с создании объекта и подготовки данных
	 // так как я делаю игру для android, я не использую cpp библиотеку glm.
	 // Я создал альтернативу, сишную библиотеку и пока там только три или четыре функции
	 // ну здесь идет очистка массивов в ноль 
	glm::clearMatrix4x4(&ortho[0]);
	glm::clearMatrix4x4(&translate[0]);
	glm::clearMatrix4x4(&result[0]);
	

	// получаю из глобальной структуры шейдерную программу 
	//program = global.programs["sprite"];
	/// также в глобальной структуре хранятся размеры экрана, их я тоже использую 
	int width = global.width;
	int height = global.height;
	// вот и пригодились размеры экрана, здесь я заполняю матрицу правильными значениями
	//  для 2d рисунков 
	glm::ortho(&ortho[0], 0.0f, width, 0.0f, height, 0.0f, 1.0f);
	//устанавливаю позицию в ноль 
	setPos(0, 0);

	// инициализация библиотеки freetype2. 
	FT_Init_FreeType(&ft_library);

	//здесь загружается файл шрифта 
#ifdef __ANDROID__
	FT_NewFace(ft_library, ttf_file, 0, &face);
#else
	char* path = (char*) new char[255];
	sprintf(path, "assets/%s", ttf_file);
	FT_New_Face(ft_library, path, 0, &face);
	delete[]  path;
#endif
}

// вот здесь самое интересное 
void Font::init(wchar_t* es, int fontSize, int align, int vert, int space, uint8_t r, uint8_t g, uint8_t b)
{
	//задать размер пикселя в высоту 
	FT_Set_Pixel_Sizes(face, 0, fontSize);

	FT_Glyph glyph;

	int w = 0;
	unsigned int h = 0;
	unsigned int maxh = 0;
	unsigned int toprow = 0;
	//эта функция возвращает сколько символов в широкой строке, если например в строке
	 //будут три буквы iаф, то функция вернет три символа. 
	int len = wcslen(es);

	//первое что я придумал это посчитать какую текстуру вообще надо создать, но для этого
	 //мне пришлось создать каждый символ и узнать его ширину. Так я вижу полную картину. Знаю
	 // какой массив создать 
	for (int i = 0; i < len; i++) {
		// итак получаем символ
		wchar_t charcode = es[i];
		// далее идут стандартные операции для создания bitmap символа 
		FT_Load_Char(face, charcode, FT_LOAD_RENDER);

		FT_UInt glyph_index = FT_Get_Char_Index(face, charcode) FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
		
		FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
		FT_Get_Glyph(face->glyph, &glyph);

		FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
		FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;
		FT_Bitmap bitmap = bitmap_glyph->bitmap;
		//теперь надо узнать ширину символа 
		w += bitmap.width;

		 //узнать разницу высоты шрифта и отступа от верха. 
		int resize = bitmap.rows > bitmap_glyph->top ? bitmap.rows - bitmap_glyph->top : bitmap_glyph->top - bitmap.rows;
		// теперь высота значиться как высота символа плюс отступ 
		if (h < bitmap.rows + resize) h = bitmap.rows + resize;
		// здесь надо знать самую большую высоту символа 
		if (toprow < bitmap.rows) toprow = bitmap.rows;
		if (maxh < bitmap.rows + bitmap_glyph->top) maxh = bitmap.rows + bitmap_glyph->top;

		// если символ равен пробелу, то увеличить w на столько пикселей, сколько задали при
		//  инициализации 
		if (charcode == ' ') w += space;
		// если встретился символ 'новая строка'
		 // то увеличить высоту включив туда вертикальный отступ и максимальную высоту 
		if (charcode == '\n') {
			h += vert + maxh;
			FT_Done_Glyph(glyph);
			continue;
		}
		// это расстояние между шрифтом, если align равен одному пикселю, то увеличиться на один 
		w += align;

		FT_Done_Glyph(glyph);
	}

	// теперь можно создать подготовительный двухмерный массив,
	 // он включает размер всего текста в пикселях 
	if (h <= 0) h = maxh;
	uint8_t im[h][w];
	
	// заполню нулями массив 
	memset(&im[0][0], 0, w * h * sizeof(uint8_t));

	int ih = 0;
	int iw = 0;
	int posy = 0;
	int topy = 0;
	int maxwidth = 0;
	for (int i = 0; i < len; i++) {
		wchar_t charcode = es[i];
		FT_Load_Char(face, charcode, FT_LOAD_RENDER);
		FT_UInt glyph_index = FT_Get_Char_Index(face, charcode);

		FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
		FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
		FT_Get_Glyph(face->glyph, &glyph);

		FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
		FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;
		FT_Bitmap bitmap = bitmap_glyph->bitmap;

		//получить отступ символа от верха 
		posy = bitmap_glyph->top;
		// это математика наверное, немогу объяснить как я тут высчитал 
		posy = bitmap.rows - posy;
		topy = toprow - bitmap.rows;

		//если новая строка, то ih - это высота от верха, то есть сверху это ноль,
		 // ниже увеличивается 
		if (charcode == '\n') {
			ih += maxh;
			iw = 0;
			FT_Done_Glyph(glyph);
			continue;
		}
		for (unsigned int y = 0, i = 0; y < bitmap.rows; y++) {
			for (unsigned int x = 0; x < bitmap.width; x++, i++) {
				if ((ih + posy + y + topy) > h) {
					if (posy < 0) posy = abs(posy);
				}

				//здесь заполняется в нужное место один компонент цвета
				 // пока массив из одного компонента gray, потом его перенесем в альфа канал 
				im[ih + posy + y + topy][iw + x] = bitmap.buffer[i];
			}
		}
		//увеличиваем ширину 
		iw += bitmap.width;
		// увеличиваем расстояние между символами 
		iw += align;
		if (maxwidth < iw) maxwidth = iw;

		if (charcode == ' ') {
			iw += space;
		}

		FT_Done_Glyph(glyph);

	}

	iw = maxwidth;
	width = iw;
	height = h;

	unsigned int size = width * height;
	// а вот это уже будущая текстура 
	uint8_t* image_data = new uint8_t[size * 4];
	// заполняет белым цветом всю текстуру 
	memset(image_data, 255, size * 4 * sizeof(uint8_t));

	for (unsigned int i = 0, y = 0; i < size; y++) {
		for (int x = 0; x < width; x++, i++) {
			//сюда помещаем из нашего массива значение в альфа канал 
			image_data[4 * i + 3] = im[y][x];
			//сюда цвет текста 
			image_data[4 * i + 0] = r;
			image_data[4 * i + 1] = g;
			image_data[4 * i + 2] = b;
		}
	}

	//стандартные действия для заполнения текстуры 
	glGenTextures(1, &textureid);
	glBindTexture(GL_TEXTURE_2D, textureid);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// теперь нужно задать размер текстуры 
	setSize(width, height);
	// и удалить текстуру, она уже загружена в буфер и image_data больше не требуется. 
	delete[] image_data;


}
void Font::setSize(int w, int h)
{
	// это я высчитал, где должны быть размеры ширины и высоты, чтобы отобразить треугольники правильно 
	if (vertices) delete[] vertices;
	if (texture) delete[] texture;
	vertices = new float[12];
	vertices[0] = 0;
	vertices[1] = 0;
	vertices[2] = 0;
	vertices[3] = h;
	vertices[4] = w;
	vertices[5] = 0;

	vertices[6] = w;
	vertices[7] = 0;
	vertices[8] = w;
	vertices[9] = h;
	vertices[10] = 0;
	vertices[11] = h;

	// для текстуры надо задавать полный размер в единицу, так она будет полностью наложена на
	// треугольники 
	texture = new float[12];
	texture[0] = 0;
	texture[1] = 1;
	texture[2] = 0;
	texture[3] = 0;
	texture[4] = 1;
	texture[5] = 1;

	texture[6] = 1;
	texture[7] = 1;
	texture[8] = 1;
	texture[9] = 0;
	texture[10] = 0;
	texture[11] = 0;
}

void Font::setPos(int x, int y)
{
	// ну здесь задается позиция, где отобразить текст 
	this->x = x;
	this->y = y;
	glm::translate(&translate[0], x, y, 0);
	glm::sumMatrix(&result[0], &translate[0], &ortho[0]);
}

void Font::draw()
{
	// стандартные действия для использования шейдера 
	glUseProgram(program);

	sampler = glGetUniformLocation(program, "s_texture");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureid);
	glUniform1i(sampler, 0);

	GLint projection_location = glGetUniformLocation(program, "transform");
	glUniformMatrix4fv(projection_location, 1, GL_FALSE, &result[0][0]);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	//сюда заноситься координаты вершин 
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vertices);
	//сюда заноситься координаты текстуры 
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, texture);

	//и рисуем текстуру 
	glDrawArrays(GL_TRIANGLES, 0, 12);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}
*/