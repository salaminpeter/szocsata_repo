package com.example.szocsata_android;

import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.os.Build;

import androidx.annotation.RequiresApi;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class OpenGLRenderer implements GLSurfaceView.Renderer {

    GLSurfaceView m_ParentView;
    static boolean m_DoRender = false; //TODO ha ez nincs akkor segv_maperr-t kapok a renderframe-ben ???

    OpenGLRenderer(GLSurfaceView surfaceView) {
        super();
        m_ParentView = surfaceView;
    }

     @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
     }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {

        if (CreateGameManager()) {
            SetThisInGameManager(this);
            InitGameManager(width, height);
        }
    }
    
    public void FinishRenderInit()
    {
        m_ParentView.queueEvent(new Runnable(){
            @Override
            public void run() {
                EndInitAndStart();
            }
        });
    }

    public void RunTaskOnRenderThreadMain(String id) {
        m_ParentView.queueEvent(new Runnable(){
            @Override
            public void run() {
                RunTaskOnRenderThread(id);
                if (id.equals("show_startscreen_task") || id.equals("resume_on_saved_screen_task")) {
                    m_DoRender = true;
                }
            }
        });
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        if (m_DoRender) {
            Render();
        }
        else {
            gl.glClearColor(0, 1, 0, 1);
            gl.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        }
    }

    public native void InitGameManager(int surfaceWidth, int surfaceHeight);
    public native boolean CreateGameManager();
    public native void Render();
    public native void EndInitAndStart();
    public native void RunTaskOnRenderThread(String id);
    public native void SetThisInGameManager(OpenGLRenderer obj);
}
