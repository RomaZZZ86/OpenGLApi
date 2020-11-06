#pragma once

//#include <nanogui/widget.h>
//#include <nanogui/glutil.h>
#include"main.h"
#include <functional>


NAMESPACE_BEGIN(nanogui)



 //class ImageView imageview.h nanogui/imageview.h
 
 //brief Widget used to display images.
 
    class NANOGUI_EXPORT GLImageView : public Widget {
    public:
       // using namespace nanogui;

        GLImageView(Widget* parent);//  GLuint imageID
        ~GLImageView();

       void imgView(GLuint imageID);

       void init_rect();
       void bindImage(GLuint imageId);

        GLShader& imageShader() { return mShader; }

        Vector2f positionF() const { return mPos.cast<float>(); }
        Vector2f sizeF() const { return mSize.cast<float>(); }

        const Vector2i& imageSize() const { return mImageSize; }
        Vector2i scaledImageSize() const { return (mScale * mImageSize.cast<float>()).cast<int>(); }
        Vector2f imageSizeF() const { return mImageSize.cast<float>(); }
        Vector2f scaledImageSizeF() const { return (mScale * mImageSize.cast<float>()); }

        const Vector2f& offset() const { return mOffset; }
        void setOffset(const Vector2f& offset) { mOffset = offset; }
        float scale() const { return mScale; }
        void setScale(float scale) { mScale = scale > 0.01f ? scale : 0.01f; }

        bool fixedOffset() const { return mFixedOffset; }
        void setFixedOffset(bool fixedOffset) { mFixedOffset = fixedOffset; }
        bool fixedScale() const { return mFixedScale; }
        void setFixedScale(bool fixedScale) { mFixedScale = fixedScale; }

        float zoomSensitivity() const { return mZoomSensitivity; }
        void setZoomSensitivity(float zoomSensitivity) { mZoomSensitivity = zoomSensitivity; }

        float gridThreshold() const { return mGridThreshold; }
        void setGridThreshold(float gridThreshold) { mGridThreshold = gridThreshold; }

        float pixelInfoThreshold() const { return mPixelInfoThreshold; }
        void setPixelInfoThreshold(float pixelInfoThreshold) { mPixelInfoThreshold = pixelInfoThreshold; }

#ifndef DOXYGEN_SHOULD_SKIP_THIS
        void setPixelInfoCallback(const std::function<std::pair<std::string, Color>(const Vector2i&)>& callback) {
            mPixelInfoCallback = callback;
        }
        const std::function<std::pair<std::string, Color>(const Vector2i&)>& pixelInfoCallback() const {
            return mPixelInfoCallback;
        }
#endif // DOXYGEN_SHOULD_SKIP_THIS

        void setFontScaleFactor(float fontScaleFactor) { mFontScaleFactor = fontScaleFactor; }
        float fontScaleFactor() const { return mFontScaleFactor; }

        // Image transformation functions.

        /// Calculates the image coordinates of the given pixel position on the widget.
        Vector2f imageCoordinateAt(const Vector2f& position) const;

        
         // Calculates the image coordinates of the given pixel position on the widget.
         // If the position provided corresponds to a coordinate outside the range of
        // the image, the coordinates are clamped to edges of the image.
         
        Vector2f clampedImageCoordinateAt(const Vector2f& position) const;

        /// Calculates the position inside the widget for the given image coordinate.
        Vector2f positionForCoordinate(const Vector2f& imageCoordinate) const;

        
         // Modifies the internal state of the image viewer widget so that the pixel at the provided
         // position on the widget has the specified image coordinate. Also clamps the values of offset
        // to the sides of the widget.
        
        void setImageCoordinateAt(const Vector2f& position, const Vector2f& imageCoordinate);

        /// Centers the image without affecting the scaling factor.
        void center();

        /// Centers and scales the image so that it fits inside the widgets.
        void fit();

        /// Set the scale while keeping the image centered
        void setScaleCentered(float scale);

        /// Moves the offset by the specified amount. Does bound checking.
        void moveOffset(const Vector2f& delta);

        
         // Changes the scale factor by the provided amount modified by the zoom sensitivity member variable.
         // The scaling occurs such that the image coordinate under the focused position remains in
        // the same position before and after the scaling.
         
        void zoom(int amount, const Vector2f& focusPosition);

        bool keyboardEvent(int key, int scancode, int action, int modifiers) override;
        bool keyboardCharacterEvent(unsigned int codepoint) override;
        bool mouseDragEvent(const Vector2i& p, const Vector2i& rel, int button, int modifiers) override;
        bool scrollEvent(const Vector2i& p, const Vector2f& rel) override;

        /// Function indicating whether the grid is currently visible.
        bool gridVisible() const;

        /// Function indicating whether the pixel information is currently visible.
        bool pixelInfoVisible() const;

        /// Function indicating whether any of the overlays are visible.
        bool helpersVisible() const;

        Vector2i preferredSize(NVGcontext* ctx) const override;
        void performLayout(NVGcontext* ctx) override;
        void draw(NVGcontext* ctx) override;

        float mapMat(float x, float in_min, float in_max, float out_min, float out_max) {
            return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
        }

       

    private:
        // Helper image methods.
        void updateImageParameters();

        // Helper drawing methods.
        void drawWidgetBorder(NVGcontext* ctx) const;
        void drawImageBorder(NVGcontext* ctx) const;
        void drawHelpers(NVGcontext* ctx) const;
        static void drawPixelGrid(NVGcontext* ctx, const Vector2f& upperLeftCorner,
            const Vector2f& lowerRightCorner, float stride);
        void drawPixelInfo(NVGcontext* ctx, float stride) const;
        void writePixelInfo(NVGcontext* ctx, const Vector2f& cellPosition,
            const Vector2i& pixel, float stride, float fontSize) const;

        // Image parameters.
        GLShader mShader;
        GLuint mImageID;
        GLuint mProgramShader;
        Vector2i mImageSize;
        GLuint VBO, VAO, EBO;
        Shader ourShader;

        // Image display parameters.
        float mScale;
        Vector2f mOffset;
        bool mFixedScale;
        bool mFixedOffset;

        bool FlagView;

        // Fine-tuning parameters.
        float mZoomSensitivity = 1.1f;

        // Image info parameters.
        float mGridThreshold = -1;
        float mPixelInfoThreshold = -1;

        // Image pixel data display members.
        std::function<std::pair<std::string, Color>(const Vector2i&)> mPixelInfoCallback;
        float mFontScaleFactor = 0.5f;
    public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

NAMESPACE_END(nanogui);

/*
#ifndef H_FONT_H
#define H_FONT_H
#include <stdint.h>
#include <ft2build.h>
#include <string>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader_s.h"

#include <iostream>
#include <string>

#include FT_FREETYPE_H
#include FT_GLYPH_H


int render_text(const char* t, int x, int y, int fontSize, FT_Face f);
FT_Face  ftt(const char* libttf);

class Font {
public:
    Font() { }
    // инициализировать библиотеку freetype и загрузить ttf файл. 
    Font(const char* ttf_file);
   // задать позицию на экране 
    void setPos(int x, int y);
    // здесь происходит создание текстуры. Вот параметры
     //1 сам текст в широких символах.
     //2 размер шрифта.
     //3 расстояние между шрифтами по горизонтали в пикселях.
     //4 расстояние между шрифтами по вертикали в пикселях.
    //5 размер пробела в пикселях.
     //6 компонент цвет красный.
     //7 компонент цвет зеленый.
     //8 компонент цвет синий.
    // ну это значит что можно задать любой цвет тексту 
    void init(wchar_t* text, int fontSize, int align, int valign, int space, uint8_t r, uint8_t g, uint8_t b);
    // задать размер текстуры 
    void setSize(int w, int h);
    // рисовать текстуру 
    void draw();
private:
    FT_Face face = 0;
    //здесь текстурные координаты 
    float* texture;
    // здесь координаты вершин 
    float* vertices;
    // это размер текстуры : ширина 
    int width;
    //это размер текстуры : высота 
    int height;
    //это для шейдера надо 
    int sampler;
    // id текстуры 
    GLuint textureid;
    // координата x 
    int x;
    // координата y 
    int y;
    // это замена функции glOrtho 
    float ortho[4][4];
    // это для перемещения на экране 
    float translate[4][4];
    // здесь результат матрицы 
    float result[4][4];
    // шейдерная программа 
    unsigned int program;
    FT_Library ft_library;
    FT_Face ttf;
};

*/







