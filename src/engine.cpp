/*
    TETRIS FROM SCRATCH
    (C) livingcreative, 2015

    feel free to use and modify
*/

// platform independend engine functions

#include <gl/GL.h>
#include "platform/platform.h"


static InputEvent *new_event(Input &input)
{
    if (input.event_count == INPUT_EVENT_COUNT) {
        return nullptr;
    } else {
        return input.events + input.event_count++;
    }
}


// since OpenGL is cross-platform by itself - most of GraphicsAPI implemented here
class OpenGLAPI : public GraphicsAPI
{
public:
    OpenGLAPI()
    {
        // basic OpenGL set-up
        glFrontFace(GL_CW);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    ~OpenGLAPI()
    {}

    void Clear(const Color &color) override
    {
        const float k = 1.0f / 255.0f;
        glClearColor(color.r * k, color.g * k, color.b * k, color.a * k);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void Viewport(int left, int top, int width, int height) override
    {
        int rt_width, rt_height;
        GetRenderTargetSize(rt_width, rt_height);

        // OpenGL's viewport origin is located at bottom left
        glViewport(left, rt_height - top, width, height);
    }

    void Rectangle(float left, float top, float width, float height, const Color &color) override
    {
        // just for test - render with glBegin/glEnd, later this will be changed
        // to more practical solution
        glBegin(GL_TRIANGLES);

        glColor4ub(color.r, color.g, color.b, color.a);

        glVertex2f(left, top);
        glVertex2f(left + width, top);
        glVertex2f(left, top + height);

        glVertex2f(left + width, top);
        glVertex2f(left + width, top + height);
        glVertex2f(left, top + height);

        glEnd();
    }
};
