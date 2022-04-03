package com.example.szocsata_android;

import android.opengl.GLSurfaceView;
import android.os.Build;

import androidx.annotation.RequiresApi;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class OpenGLRenderer implements GLSurfaceView.Renderer {

    Thread m_GameThread = null;
    GLSurfaceView m_ParentView;

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

        m_GameThread = new Thread(new Runnable() {
            public void run()
            {
                while (true)
                    GameLoop();
            }
        });

        m_GameThread.start();
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

    @Override
    public void onDrawFrame(GL10 gl) {
      //  GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        Render();
    }

    public void StopGameThread()
    {
        m_GameThread.interrupt();
    }

    public native void InitGameManager(int surfaceWidth, int surfaceHeight);
    public native boolean CreateGameManager();
    public native void Render();
    public native void GameLoop();
    public native void EndInitAndStart();
    public native void SetThisInGameManager(OpenGLRenderer obj);
}
