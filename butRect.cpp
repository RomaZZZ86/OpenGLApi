
#include"butRect.h"

NAMESPACE_BEGIN(nanogui)

ButRect::ButRect(Widget* parent)//, const std::string& caption, int icon
    : Widget(parent),           //mCaption(caption), mIcon(icon),
    mIconPosition(IconPosition::LeftCentered), mPushed(false),
    mFlags(NormalButton), mBackgroundColor(Color(0, 0)),
    mTextColor(Color(0, 0)) {
   // , mFixedScale(false), mFixedOffset(false),
    //    mPixelInfoCallback(nullptr
    Rx = 100;
    Ry =130;  
    Rw = 10.0;
    Rh = 10.0;  
    Rr=3.0;
  
   // SIZEu= Vector2i(400, 400);
   
    mImageSize = Vector2i(640, 480);

    // Set the width of the widget
    

    /// Return the height of the widget
   //setPosition( Vector2i (134,263));
   setFixedWidth(640);
   setFixedHeight(480);

   float degres = 0;
   float retreat = 20;
   float radius = 50;

   for (int8_t i = 0; i < 80; i++)pusheds[i] = 0;

   for (int8_t i = 0; i < 20; i++) {
       float ledX = Rx + radius * (float)cos(radians(degres + i * retreat));
       float ledY = Ry + radius * (float)sin(radians(degres + i * retreat));
       rects.push_back(Vector2f(ledX, ledY));
   }
    
   
}

bool ButRect::collidepoint(Vector2f pos, Vector2i posd, float w, float h)
{
    bool push = false;
    if(static_cast<float>(posd.x())>=pos.x() && 
        static_cast<float>(posd.x())<= pos.x()+w && 
        static_cast<float>(posd.y()) >= pos.y() &&
        static_cast<float>(posd.y()) <= pos.y()+h)
        push=true;

    return push;
}



Vector2i ButRect::preferredSize(NVGcontext* ctx) const {
    int fontSize = mFontSize == -1 ? mTheme->mButtonFontSize : mFontSize;
    nvgFontSize(ctx, fontSize);
    nvgFontFace(ctx, "sans-bold");
    float tw = nvgTextBounds(ctx, 0, 0, mCaption.c_str(), nullptr, nullptr);
    float iw = 0.0f, ih = fontSize;

    if (mIcon) {
        if (nvgIsFontIcon(mIcon)) {
            ih *= icon_scale();
            nvgFontFace(ctx, "icons");
            nvgFontSize(ctx, ih);
            iw = nvgTextBounds(ctx, 0, 0, utf8(mIcon).data(), nullptr, nullptr)
                + mSize.y() * 0.15f;
        }
        else {
            int w, h;
            ih *= 0.9f;
            nvgImageSize(ctx, mIcon, &w, &h);
            iw = w * ih / h;
        }
    }
    return Vector2i((int)(tw + iw) + 20, fontSize + 10);
}


bool ButRect::mouseDragEvent(const Vector2i& p, const Vector2i&  rel ,
    int  button , int  modifiers ) {
    //std::cout << "DRAG -- " << p << "-- " <<  std::endl;
    for (int8_t i = 0; i < 20; i++) {
        if (collidepoint(rects[i], p, Rw, Rh))
        {
            pusheds[i] = true;

        }
    }
    if (mCallback)
        mCallback(mValue);
    return true;
}

bool ButRect::mouseButtonEvent(const Vector2i& p, int button, bool down, int modifiers) {

     std::cout << "BUT -- " << p << "-- " <<  std::endl;

     Widget::mouseButtonEvent(p, button, down, modifiers);
    // Temporarily increase the reference count of the button in case the
      // button causes the parent window to be destructed 
    ref<ButRect> self = this;

    
    if (button == GLFW_MOUSE_BUTTON_1 && mEnabled) {
        
        
        for (int8_t i = 0; i < 20; i++) {

            //---------------------------------------------
           

            if (down) {
                if (collidepoint(rects[i], p, Rw, Rh))
                {
                    pusheds[i] = true;
                   
                       
                }
               
            }

            if (mCallback)
                mCallback(mValue);
            if (mFinalCallback && !down)
                mFinalCallback(mValue);
           
        }

     
       
    }
    
   
        
    return false;
    
}

void ButRect::draw(NVGcontext* ctx) {
   
    Widget::draw(ctx);
    //std::cout << "Callback key -- " << cursor.x() << "-- " << cursor.y() << std::endl;
    /*
    NVGcolor gradTop = mTheme->mButtonGradientTopUnfocused;
    NVGcolor gradBot = mTheme->mButtonGradientBotUnfocused;

    if (mPushed) {
        gradTop = mTheme->mButtonGradientTopPushed;
        gradBot = mTheme->mButtonGradientBotPushed;
    }
    else if (mMouseFocus && mEnabled) {
        gradTop = mTheme->mButtonGradientTopFocused;
        gradBot = mTheme->mButtonGradientBotFocused;
    }
    */
   
    

    for (float i = 0; i < 20; i++) {
        //----------------------------------------------------------------------------

       // std::cout << "Radians -- " << ledX<<"__"<< ledY<< std::endl;
        // nvgRoundedRect(ctx, mPos.x() + 1, mPos.y() + 1.0f, mSize.x() - 2,
         //    mSize.y() - 2, mTheme->mButtonCornerRadius - 1);

        if (pusheds[static_cast<int>(i)]) {


            nvgBeginPath(ctx);
            nvgRoundedRect(ctx, rects[i].x(), rects[i].y(), Rw, Rh, Rr);
            nvgFillColor(ctx, Color(1.f, 0.f, 0.f, 1.f));
            //nvgFillColor(ctx, nvgRGBA(0,0,0,128));
            nvgFill(ctx);
        }
        else {
            nvgBeginPath(ctx);
            nvgRoundedRect(ctx, rects[i].x(), rects[i].y(), Rw, Rh, Rr);
            nvgFillColor(ctx, Color(1.0, 1.0, 1.0, 1.f));
            nvgFill(ctx);

            nvgBeginPath(ctx);
            nvgRoundedRect(ctx, rects[i].x() + 0.5, rects[i].y(), Rw - 1.0f,
                Rh + 0.5f, Rr);
            nvgFillColor(ctx, Color(0.2, 0.2, 0.2, 1.f));
            nvgFill(ctx);
        }

        //for (uint16_t i = 0; i < 5000; i++);

    }
  
  
}

void ButRect::drawImageBorder(NVGcontext* ctx) const {
    nvgSave(ctx);
    nvgBeginPath(ctx);
    nvgScissor(ctx, mPos.x(), mPos.y(), mSize.x(), mSize.y());
    nvgStrokeWidth(ctx, 1.0f);
    Vector2i borderPosition = mPos + mOffset.cast<int>();
    Vector2i borderSize = mImageSize;
    nvgRect(ctx, borderPosition.x() - 0.5f, borderPosition.y() - 0.5f,
        borderSize.x() + 1, borderSize.y() + 1);
    nvgStrokeColor(ctx, Color(1.0f, 1.0f, 1.0f, 1.0f));
    nvgStroke(ctx);
    nvgResetScissor(ctx);
    nvgRestore(ctx);
}

void ButRect::save(Serializer& s) const {
    Widget::save(s);
    s.set("caption", mCaption);
    s.set("icon", mIcon);
    s.set("iconPosition", (int)mIconPosition);
    s.set("pushed", mPushed);
    s.set("flags", mFlags);
    s.set("backgroundColor", mBackgroundColor);
    s.set("textColor", mTextColor);
}

bool ButRect::load(Serializer& s) {
    if (!Widget::load(s)) return false;
    if (!s.get("caption", mCaption)) return false;
    if (!s.get("icon", mIcon)) return false;
    if (!s.get("iconPosition", mIconPosition)) return false;
    if (!s.get("pushed", mPushed)) return false;
    if (!s.get("flags", mFlags)) return false;
    if (!s.get("backgroundColor", mBackgroundColor)) return false;
    if (!s.get("textColor", mTextColor)) return false;
    return true;
}

NAMESPACE_END(nanogui)
