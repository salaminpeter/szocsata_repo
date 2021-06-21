package com.example.szocsata_android;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;

public class OpenGLView extends GLSurfaceView {

    public Renderer m_Renderer;

    public OpenGLView(Context context, AttributeSet attrs) {
        super(context, attrs);
        Init();
    }

    public OpenGLView(Context context) {
        super(context);
        Init();
    }

    private void Init() {
        setEGLContextClientVersion(2);
        setEGLConfigChooser(true);
        setEGLConfigChooser(8, 8, 8, 8, 24, 0);
        setPreserveEGLContextOnPause(true);
        m_Renderer = new OpenGLRenderer(this);
        setRenderer(m_Renderer);
    }
}
