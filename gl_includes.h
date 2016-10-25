//
// Created by Martin Wickham on 10/11/2016.
//

#ifndef STUPIDSHTRICKS_GLERROR_H
#define STUPIDSHTRICKS_GLERROR_H

#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define checkError() _check_gl_error(__FILE__,__LINE__)

void _check_gl_error(const char *file, int line) {
    GLenum err (glGetError());

    while(err!=GL_NO_ERROR) {
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
}

#endif //STUPIDSHTRICKS_GLERROR_H
