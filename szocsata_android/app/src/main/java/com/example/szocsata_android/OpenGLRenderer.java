package com.example.szocsata_android;

import android.opengl.GLES20;
import android.opengl.GLSurfaceView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class OpenGLRenderer implements GLSurfaceView.Renderer {

    Thread GameThread = null;

     @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
         InitGameManager();

         GameThread = new Thread(new Runnable() {
             public void run()
             {
                while (true)
                    GameLoop();
             }
         });

         GameThread.start();
     }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        GLES20.glViewport(0,0, width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
      //  GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        Render();
    }

    public native void InitGameManager();
    public native void Render();
    public native void GameLoop();
}
