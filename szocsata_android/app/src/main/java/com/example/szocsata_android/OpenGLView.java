package com.example.szocsata_android;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;

import androidx.annotation.MainThread;

public class OpenGLView extends GLSurfaceView {

    public Renderer m_Renderer;
    public MainActivity m_Activity;

    public OpenGLView(Context context, AttributeSet attrs) {
        super(context, attrs);
        Init((MainActivity) context);
    }

    public OpenGLView(Context context) {
        super(context);
        Init((MainActivity) context);
    }

    public void StopGameLoop()
    {
        ((OpenGLRenderer)(m_Renderer)).StopGameThread();
    }

    private void Init(MainActivity activity) {
        setEGLContextClientVersion(2);
        setEGLConfigChooser(true);
        setEGLConfigChooser(8, 8, 8, 8, 24, 0);
        setPreserveEGLContextOnPause(true);
        m_Renderer = new OpenGLRenderer(this);
        setRenderer(m_Renderer);
    }
}
