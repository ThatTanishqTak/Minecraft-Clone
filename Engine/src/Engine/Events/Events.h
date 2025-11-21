#pragma once

#include <functional>
#include <sstream>
#include <string>

namespace Engine
{
    // Enumerates the types of events the engine can produce.
    enum class EventType
    {
        None = 0,
        WindowResize,
        WindowClose,
        KeyPressed,
        KeyReleased,
        MouseMoved,
        MouseButtonPressed,
        MouseButtonReleased,
        MouseScrolled
    };

    // Categorize events so listeners can perform quick filtering using bitwise checks.
    enum EventCategory
    {
        EventCategoryApplication = 1 << 0,
        EventCategoryInput = 1 << 1,
        EventCategoryKeyboard = 1 << 2,
        EventCategoryMouse = 1 << 3,
        EventCategoryMouseButton = 1 << 4
    };

    // Base class for all events, providing runtime type information and handled state.
    class Event
    {
    public:
        virtual ~Event() = default;

        // Used by the dispatcher to query the concrete event type at runtime.
        virtual EventType GetEventType() const = 0;

        // The human-readable name of this event type, useful for logging.
        virtual const char* GetName() const = 0;

        // Bitwise category flags that allow coarse filtering (keyboard, mouse, etc.).
        virtual int GetCategoryFlags() const = 0;

        // Convert the event into a friendly string for diagnostics.
        virtual std::string ToString() const
        {
            return GetName();
        }

        // Helper to quickly check membership in an event category.
        bool IsInCategory(EventCategory category) const
        {
            return (GetCategoryFlags() & category) != 0;
        }

    public:
        // Whether a listener has consumed the event; dispatchers skip further processing when true.
        bool m_Handled = false;
    };

    // Holds the strongly-typed dispatch logic for a single event instance.
    class EventDispatcher
    {
    public:
        explicit EventDispatcher(Event& event) : m_Event(event)
        {

        }

        // Dispatches the event to the provided callable if the types match.
        // Returns true if the callable was invoked.
        template<typename T, typename F>
        bool Dispatch(F&& function)
        {
            if (m_Event.GetEventType() == T::GetStaticType())
            {
                m_Event.m_Handled = function(static_cast<T&>(m_Event));

                return true;
            }

            return false;
        }

    private:
        Event& m_Event;
    };

    // --------------------------------------
    // Application events
    // --------------------------------------
    struct WindowResizeEvent : public Event
    {
        WindowResizeEvent(int width, int height) : m_Width(width), m_Height(height)
        {

        }

        inline int GetWidth() const { return m_Width; }
        inline int GetHeight() const { return m_Height; }

        std::string ToString() const override
        {
            std::stringstream l_Stream;
            l_Stream << "WindowResize: " << m_Width << ", " << m_Height;

            return l_Stream.str();
        }

        static EventType GetStaticType() { return EventType::WindowResize; }
        EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "WindowResize"; }
        int GetCategoryFlags() const override { return EventCategoryApplication; }

    private:
        int m_Width;
        int m_Height;
    };

    struct WindowCloseEvent : public Event
    {
        WindowCloseEvent() = default;

        static EventType GetStaticType() { return EventType::WindowClose; }
        EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "WindowClose"; }
        int GetCategoryFlags() const override { return EventCategoryApplication; }
    };

    // --------------------------------------
    // Keyboard events
    // --------------------------------------
    struct KeyEvent : public Event
    {
        int GetKeyCode() const { return m_KeyCode; }

        int GetCategoryFlags() const override
        {
            return EventCategoryKeyboard | EventCategoryInput;
        }

    protected:
        explicit KeyEvent(int keycode) : m_KeyCode(keycode)
        {

        }

        int m_KeyCode;
    };

    struct KeyPressedEvent : public KeyEvent
    {
        KeyPressedEvent(int keycode, int repeatCount) : KeyEvent(keycode), m_RepeatCount(repeatCount)
        {

        }

        int GetRepeatCount() const { return m_RepeatCount; }

        std::string ToString() const override
        {
            std::stringstream l_Stream;
            l_Stream << "KeyPressed: " << m_KeyCode << " (" << m_RepeatCount << " repeats)";

            return l_Stream.str();
        }

        static EventType GetStaticType() { return EventType::KeyPressed; }
        EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "KeyPressed"; }

    private:
        int m_RepeatCount;
    };

    struct KeyReleasedEvent : public KeyEvent
    {
        explicit KeyReleasedEvent(int keycode) : KeyEvent(keycode)
        {

        }

        std::string ToString() const override
        {
            std::stringstream l_Stream;
            l_Stream << "KeyReleased: " << m_KeyCode;

            return l_Stream.str();
        }

        static EventType GetStaticType() { return EventType::KeyReleased; }
        EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "KeyReleased"; }
    };

    // --------------------------------------
    // Mouse events
    // --------------------------------------
    struct MouseMovedEvent : public Event
    {
        MouseMovedEvent(float x, float y) : m_MouseX(x), m_MouseY(y)
        {

        }

        float GetX() const { return m_MouseX; }
        float GetY() const { return m_MouseY; }

        std::string ToString() const override
        {
            std::stringstream l_Stream;
            l_Stream << "MouseMoved: " << m_MouseX << ", " << m_MouseY;

            return l_Stream.str();
        }

        static EventType GetStaticType() { return EventType::MouseMoved; }
        EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "MouseMoved"; }
        int GetCategoryFlags() const override { return EventCategoryMouse | EventCategoryInput; }

    private:
        float m_MouseX;
        float m_MouseY;
    };

    struct MouseScrolledEvent : public Event
    {
        MouseScrolledEvent(float xOffset, float yOffset)
            : m_XOffset(xOffset), m_YOffset(yOffset)
        {
        }

        float GetXOffset() const { return m_XOffset; }
        float GetYOffset() const { return m_YOffset; }

        std::string ToString() const override
        {
            std::stringstream l_Stream;
            l_Stream << "MouseScrolled: " << m_XOffset << ", " << m_YOffset;

            return l_Stream.str();
        }

        static EventType GetStaticType() { return EventType::MouseScrolled; }
        EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "MouseScrolled"; }
        int GetCategoryFlags() const override { return EventCategoryMouse | EventCategoryInput; }

    private:
        float m_XOffset;
        float m_YOffset;
    };

    struct MouseButtonEvent : public Event
    {
        int GetMouseButton() const { return m_Button; }

        int GetCategoryFlags() const override
        {
            return EventCategoryMouse | EventCategoryInput | EventCategoryMouseButton;
        }

    protected:
        explicit MouseButtonEvent(int button) : m_Button(button)
        {

        }

        int m_Button;
    };

    struct MouseButtonPressedEvent : public MouseButtonEvent
    {
        explicit MouseButtonPressedEvent(int button) : MouseButtonEvent(button)
        {

        }

        std::string ToString() const override
        {
            std::stringstream l_Stream;
            l_Stream << "MouseButtonPressed: " << m_Button;

            return l_Stream.str();
        }

        static EventType GetStaticType() { return EventType::MouseButtonPressed; }
        EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "MouseButtonPressed"; }
    };

    struct MouseButtonReleasedEvent : public MouseButtonEvent
    {
        explicit MouseButtonReleasedEvent(int button)
            : MouseButtonEvent(button)
        {
        }

        std::string ToString() const override
        {
            std::stringstream l_Stream;
            l_Stream << "MouseButtonReleased: " << m_Button;

            return l_Stream.str();
        }

        static EventType GetStaticType() { return EventType::MouseButtonReleased; }
        EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "MouseButtonReleased"; }
    };
}