#pragma once

#include"main.h"
#define  PI  3.1416
#define radians(degres) (( degres * PI ) / 180 )


NAMESPACE_BEGIN(nanogui)
/**
 * \class Button button.h nanogui/button.h
 *
 * \brief [Normal/Toggle/Radio/Popup] Button widget.
 */
    class NANOGUI_EXPORT ButRect: public Widget {
    public:
        /// Flags to specify the button behavior (can be combined with binary OR)
        enum Flags {
            NormalButton = (1 << 0), ///< A normal Button.
            RadioButton = (1 << 1), ///< A radio Button.
            ToggleButton = (1 << 2), ///< A toggle Button.
            PopupButton = (1 << 3)  ///< A popup Button.
        };

        /// The available icon positions.
        enum class IconPosition {
            Left,         ///< Button icon on the far left.
            LeftCentered, ///< Button icon on the left, centered (depends on caption text length).
            RightCentered,///< Button icon on the right, centered (depends on caption text length).
            Right         ///< Button icon on the far right.
        };

        /**
         * \brief Creates a button attached to the specified parent.
         *
         * \param parent
         *     The \ref nanogui::Widget this Button will be attached to.
         *
         * \param caption
         *     The name of the button (default ``"Untitled"``).
         *
         * \param icon
         *     The icon to display with this Button.  See \ref nanogui::Button::mIcon.
         */
        ButRect(Widget* parent);//, const std::string& caption = "Untitled", int icon = 0

       


        void drawImageBorder(NVGcontext* ctx) const;

        /// Returns the caption of this Button.
        const std::string& caption() const { return mCaption; }

        /// Sets the caption of this Button.
        void setCaption(const std::string& caption) { mCaption = caption; }

        /// Returns the background color of this Button.
        const Color& backgroundColor() const { return mBackgroundColor; }

        /// Sets the background color of this Button.
        void setBackgroundColor(const Color& backgroundColor) { mBackgroundColor = backgroundColor; }

        /// Returns the text color of the caption of this Button.
        const Color& textColor() const { return mTextColor; }

        /// Sets the text color of the caption of this Button.
        void setTextColor(const Color& textColor) { mTextColor = textColor; }

        /// Returns the icon of this Button.  See \ref nanogui::Button::mIcon.
        int icon() const { return mIcon; }

        /// Sets the icon of this Button.  See \ref nanogui::Button::mIcon.
        void setIcon(int icon) { mIcon = icon; }

        /// The current flags of this Button (see \ref nanogui::Button::Flags for options).
        int flags() const { return mFlags; }

        /// Sets the flags of this Button (see \ref nanogui::Button::Flags for options).
        void setFlags(int buttonFlags) { mFlags = buttonFlags; }

        /// The position of the icon for this Button.
        IconPosition iconPosition() const { return mIconPosition; }

        /// Sets the position of the icon for this Button.
        void setIconPosition(IconPosition iconPosition) { mIconPosition = iconPosition; }

        /// Whether or not this Button is currently pushed.
        bool pushed() const { return mPushed; }

        bool collidepoint(Vector2f pos, Vector2i posd, float w, float h);
       
        //virtual bool mouseDragEvent(const Vector2i& p, const Vector2i& /* rel */,
        //    int /* button */, int /* modifiers */) override;

        /// Sets whether or not this Button is currently pushed.
        void setPushed(bool pushed) { mPushed = pushed; }

        /// The current callback to execute (for any type of button).
        //std::function<void()> callback() const { return mCallback; }
        std::function<void(float)> callback() const { return mCallback; }

        /// Set the push callback (for any type of button).
        //void setCallback(const std::function<void()>& callback) { mCallback = callback; }
        void setCallback(const std::function<void(float)>& callback) { mCallback = callback; }
        /// The current callback to execute (for toggle buttons).
        std::function<void(bool)> changeCallback() const { return mChangeCallback; }

        /// Set the change callback (for toggle buttons).
        void setChangeCallback(const std::function<void(bool)>& callback) { mChangeCallback = callback; }

        std::function<void(float)> finalCallback() const { return mFinalCallback; }

        /// Set the button group (for radio buttons).
        void setButtonGroup(const std::vector<ButRect*>& buttonGroup) { mButtonGroup = buttonGroup; }

        /// The current button group (for radio buttons).
        const std::vector<ButRect*>& buttonGroup() const { return mButtonGroup; }

        /// The preferred size of this Button.
        virtual Vector2i preferredSize(NVGcontext* ctx) const override;

        //void cursorCallback(float x, float y) { cursor.x() = x; cursor.y() = y;};

        /// The callback that is called when any type of mouse button event is issued to this Button.
        virtual bool mouseButtonEvent(const Vector2i& p, int button, bool down, int modifiers) override;

        virtual bool mouseDragEvent(const Vector2i& p, const Vector2i& rel, int button, int modifiers) override;
        /// Responsible for drawing the Button.
        virtual void draw(NVGcontext* ctx) override;

        /// Saves the state of this Button provided the given Serializer.
        virtual void save(Serializer& s) const override;

        /// Sets the state of this Button provided the given Serializer.
        virtual bool load(Serializer& s) override;

    protected:
        /// The caption of this Button.
        std::string mCaption;

        float Rx, Ry, Rw, Rh, Rr;

        Vector2i SIZEu; 
        Vector2i mImageSize;
        Vector2f mOffset;
        bool pusheds[80];
        Vector2f cursor;
        std::vector< Vector2f> rects;
        //GLFWwindow* mGLFWWindow;
        /**
         * \brief The icon of this Button (``0`` means no icon).
         *
         * \rst
         * The icon to display with this Button.  If not ``0``, may either be a
         * picture icon, or one of the icons enumerated in
         * :ref:`file_nanogui_entypo.h`.  The kind of icon (image or Entypo)
         * is determined by the functions :func:`nanogui::nvgIsImageIcon` and its
         * reciprocal counterpart :func:`nanogui::nvgIsFontIcon`.
         * \endrst
         */
        int mIcon;

        /// The position to draw the icon at.
        IconPosition mIconPosition;

        /// Whether or not this Button is currently pushed.
        bool mPushed;

        /// The current flags of this button (see \ref nanogui::Button::Flags for options).
        int mFlags;

        /// The background color of this Button.
        Color mBackgroundColor;

        /// The color of the caption text of this Button.
        Color mTextColor;

        /// The callback issued for all types of buttons.
        //std::function<void()> mCallback;
        float mValue;
        std::function<void(float)> mCallback;
        std::function<void(float)> mFinalCallback;

        /// The callback issued for toggle buttons.
        std::function<void(bool)> mChangeCallback;

        /// The button group for radio buttons.
        std::vector<ButRect*> mButtonGroup;

    public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

NAMESPACE_END(nanogui)

