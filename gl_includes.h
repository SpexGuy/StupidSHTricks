//
// Created by Martin Wickham on 10/11/2016.
//

#ifndef GLERROR_H
#define GLERROR_H

#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLSL(src) "#version 400\n" #src

#define checkError() _check_gl_error(__FILE__,__LINE__)

static bool _check_gl_error(const char *file, int line) {
    bool hasError = false;
    GLenum err (glGetError());

    while(err!=GL_NO_ERROR) {
        hasError = true;
        const char *error;

        switch(err) {
            case GL_INVALID_OPERATION:      error="INVALID_OPERATION";      break;
            case GL_INVALID_ENUM:           error="INVALID_ENUM";           break;
            case GL_INVALID_VALUE:          error="INVALID_VALUE";          break;
            case GL_OUT_OF_MEMORY:          error="OUT_OF_MEMORY";          break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:  error="INVALID_FRAMEBUFFER_OPERATION";  break;
        }

        std::cerr << "GL_" << error <<" - "<<file<<":"<<line<<std::endl;
        err=glGetError();
    }
    return hasError;
}

#endif //GLERROR_H
