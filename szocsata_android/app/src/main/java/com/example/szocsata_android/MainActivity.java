package com.example.szocsata_android;

import androidx.annotation.MainThread;
import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;
import androidx.constraintlayout.widget.ConstraintLayout;
import androidx.core.view.MotionEventCompat;

import android.accessibilityservice.FingerprintGestureController;
import android.annotation.SuppressLint;
import android.content.Context;
import android.content.Intent;
import android.content.res.AssetManager;
import android.graphics.Point;
import android.graphics.PointF;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.TextView;
import com.example.szocsata_android.R;

import java.io.File;
import java.io.IOException;

public class MainActivity extends AppCompatActivity {

    private OpenGLView m_OpenGLView;

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    public native void ClearResources();

    private static Point m_FingerPos0 = new Point();
    private static Point m_FingerPos1 = new Point();
    private static boolean m_MultyTouch = false;
    private static double Distance = 0;

    private boolean m_PauseHandled = true;

    private static MainActivity obj;

    public void setPauseDone(boolean done) {m_PauseHandled = done; }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        obj = this;
        hideStatusBar();
        setContentView(R.layout.activity_main);
        m_OpenGLView = findViewById(R.id.openGLView);

        AssetManager am = getAssets();
        SetAssetManager(am);

        m_OpenGLView.setOnTouchListener(new View.OnTouchListener() {
                                            @Override
                                            public boolean onTouch(View v, MotionEvent event) {

                                                int Action = event.getActionMasked();

                                                if (Action == MotionEvent.ACTION_DOWN) {
                                                    Log.i("fos", "finger one down : " + 0);
                                                    hideStatusBar();
                                                    int x = (int)event.getX(0);
                                                    int y = (int)event.getY(0);
                                                    m_FingerPos0.x = x;
                                                    m_FingerPos0.y = y;

                                                    if (!m_MultyTouch)
                                                        HandleTouchEvent(x, y);
                                                    return true;
                                                }

                                                if (Action == MotionEvent.ACTION_POINTER_DOWN) {
                                                        Log.i("fos", "finger 2 down : " + 1);
                                                    m_MultyTouch = true;
                                                    int x = (int)event.getX(1);
                                                    int y = (int)event.getY(1);
                                                    m_FingerPos1.x = x;
                                                    m_FingerPos1.y = y;
                                                    Distance = Math.sqrt((double)((m_FingerPos0.x - m_FingerPos1.x) * (m_FingerPos0.x - m_FingerPos1.x) + (m_FingerPos0.y - m_FingerPos1.y) * (m_FingerPos0.y - m_FingerPos1.y)));
                                                    Log.i("dist", "start Distance : " + Distance);
                                                    HandleMultyTouchStartEvent(m_FingerPos0.x, m_OpenGLView.getHeight() - m_FingerPos0.y, m_FingerPos1.x, m_OpenGLView.getHeight() - m_FingerPos1.y);
                                                    return  true;
                                                }

                                                if (Action == MotionEvent.ACTION_UP) {
                                                    Log.i("fos", "finger 1 up : " + 1);
                                                    if (m_MultyTouch) {
                                                        HandleMultyTouchEndEvent();
                                                        m_MultyTouch = false;
                                                    }
                                                    else {
                                                        int x = (int)event.getX(0);
                                                        int y = (int)event.getY(0);
                                                        Log.i("fos", "HandleRelease x : " + x + " y : " + y );
                                                        HandleReleaseEvent(x, y);
                                                    }
                                                        return true;
                                                 }

                                                if (Action == MotionEvent.ACTION_POINTER_UP) {
                                                    Log.i("fos", "finger 2 up : " + 1);
                                                    HandleMultyTouchEndEvent();
                                                    m_MultyTouch = false;
                                                    return true;
                                                }

                                                if (Action == MotionEvent.ACTION_MOVE) {
  //                                                 Log.i("fos", "finger move : " + event.getPointerCount() + " kk " + m_MultyTouch);
                                                    if (m_MultyTouch) {

                                                        int pointerCount = event.getPointerCount();
                                                        int dif = 0;

                                                        for(int i = 0; i < pointerCount; ++i)
                                                        {
                                                            int pointerIndex = i;
                                                            int pointerId = event.getPointerId(pointerIndex);
                                                            Log.d("pointer id - move",Integer.toString(pointerId));
                                                            if(pointerId == 0)
                                                            {
                                                                dif += Math.abs(m_FingerPos0.x - (int)event.getX(pointerIndex)) + Math.abs(m_FingerPos0.y - (int)event.getX(pointerIndex));
                                                                m_FingerPos0.x = (int)event.getX(pointerIndex);
                                                                m_FingerPos0.y = (int)event.getY(pointerIndex);
                                                                Log.i("fos", "finger 0 moved :  x - " + m_FingerPos0.x + " y - " + m_FingerPos0.y);
                                                            }
                                                            if(pointerId == 1)
                                                            {
                                                                dif += Math.abs(m_FingerPos1.x - (int)event.getX(pointerIndex)) + Math.abs(m_FingerPos1.y - (int)event.getX(pointerIndex));
                                                                m_FingerPos1.x = (int)event.getX(pointerIndex);
                                                                m_FingerPos1.y = (int)event.getY(pointerIndex);
                                                                Log.i("fos", "finger 1 moved :  x - " + m_FingerPos1.x + " y - " + m_FingerPos1.y);
                                                            }
                                                        }

                                                        //if (dif > 4) {
                                                            double Dist = Math.sqrt((double)((m_FingerPos0.x - m_FingerPos1.x) * (m_FingerPos0.x - m_FingerPos1.x) + (m_FingerPos0.y - m_FingerPos1.y) * (m_FingerPos0.y - m_FingerPos1.y)));
                                                            Log.i("dist", "Distance : " + (Dist - Distance));
                                                            Distance = Dist;
                                                            HandleMultyTouchEvent(m_FingerPos0.x, m_OpenGLView.getHeight() - m_FingerPos0.y, m_FingerPos1.x, m_OpenGLView.getHeight() - m_FingerPos1.y);
                                                        //}
                                                    }

                                                    else {
                                                        int x = (int)event.getX(0);
                                                        int y = (int)event.getY(0);
                                                        Log.i("fos", "HandleDrag x : " + x + " y : " + y );
                                                       HandleDragEvent(x, y);
                                                    }

                                                    return true;
                                                }

                                                return v.onTouchEvent(event);
                                            }
                                        }

        );
    }

    @Override
    protected void onStart() {
        super.onStart();
        m_PauseHandled = false;
    }

    @Override
    protected void onResume(){
        super.onResume();

        String Path = getFilesDir() + "/secondstart";
        File SecondStartFile = new File(Path);

        if (SecondStartFile.exists() == false) {
            try {
                SecondStartFile.createNewFile();
            } catch (IOException e) {
                e.printStackTrace();
            }
            Intent i = new Intent(getApplicationContext(), MainActivity.class);
            i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
            startActivity(i);
            android.os.Process.killProcess(android.os.Process.myPid());
            return;
        }

        m_OpenGLView.onResume();

        SecondStartFile.delete();

/*
        hideStatusBar();
        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        m_OpenGLView.onResume();*/
    }

    @Override
    protected void onPause(){
        super.onPause();

        if (m_PauseHandled)
            return;

        m_PauseHandled = true;
        m_OpenGLView.onPause();
        ClearResources();
    }
    @Override
    public void onPointerCaptureChanged(boolean hasCapture) {
    }

    private void hideStatusBar() {
        View decorView = getWindow().getDecorView();
        decorView.setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_IMMERSIVE
                        // Set the content to appear under the system bars so that the
                        // content doesn't resize when the system bars hide and show.
                        | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                        | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                        // Hide the nav bar and status bar
                        | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_FULLSCREEN);
    }

    public static String GetWorkingDir()
    {
        return ((Context) obj).getFilesDir().toString();
    }

    public static void showLoadingScreen(boolean show) {

        obj.runOnUiThread(new Runnable() {
            public void run() {
                View openGlView  = (View) obj.findViewById(R.id.openGLView);
                View textView  = (View) obj.findViewById(R.id.textView);

                openGlView.setScaleX(show ? 0 : 1);
                openGlView.setScaleY(show ? 0 : 1);
                textView.setVisibility(show ? View.VISIBLE : View.INVISIBLE);
            }
        });
    }

        /**
         * A native method that is implemented by the 'native-lib' native library,
         * which is packaged with this application.
         */
    public native void SetAssetManager(Object assetManager);
    public native void HandleTouchEvent(int x, int y);
    public native void HandleReleaseEvent(int x, int y);
    public native void HandleDragEvent(int x, int y);
    public native void HandleMultyTouchStartEvent(int x0, int y0, int x1, int y1);
    public native void HandleMultyTouchEvent(int x0, int y0, int x1, int y1);
    public native void HandleMultyTouchEndEvent();
}