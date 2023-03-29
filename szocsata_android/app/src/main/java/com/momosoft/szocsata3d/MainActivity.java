package com.momosoft.szocsata3d;

import androidx.appcompat.app.AppCompatActivity;
import android.content.Context;
import android.content.Intent;
import android.content.res.AssetManager;
import android.content.res.Configuration;
import android.graphics.BitmapFactory;
import android.graphics.Point;
import android.os.Bundle;
import android.view.View;
import android.os.Debug;

import com.momosoft.szocsata3d.TouchInputManager;

import com.momosoft.szocsata3d.R;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.Buffer;

public class MainActivity extends AppCompatActivity {

    private OpenGLView m_OpenGLView;
    private TouchInputManager m_TouchInputManager;

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    public native void ClearResources();
    public native void StopThreads();
    public native void SaveState();

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

        m_TouchInputManager = new TouchInputManager(this);
        m_OpenGLView.setOnTouchListener(m_TouchInputManager.m_TouchInputListener);
        InstallDB();
    }

    @Override
    protected void onStart() {
        super.onStart();
        m_PauseHandled = false;
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig)
    {
        super.onConfigurationChanged(newConfig);
    }

    @Override
    protected void onResume(){
        super.onResume();
        hideStatusBar();

//       Debug.waitForDebugger();
        ImageLoader.m_Context = this;
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
    }

    @Override
    protected void onPause(){
        super.onPause();

        if (m_PauseHandled)
            return;

        m_PauseHandled = true;

        StopThreads();
        m_OpenGLView.onPause();
        SaveState();
        ClearResources();
    }
    @Override
    public void onPointerCaptureChanged(boolean hasCapture) {
    }

    public Point GetGLViewSize() {
        return new Point(m_OpenGLView.getWidth(), m_OpenGLView.getHeight());
    }

    public void hideStatusBar() {
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

    public void InstallDB()
    {
        File DBFile = new File(getFilesDir() + "/db.dat");

        if (!DBFile.exists()) {
            AssetManager am = getAssets();
            InputStream is = null;
            OutputStream os = null;

            try {
                byte[] Buffer = new byte[4096];
                int BytesRead = -1;

                is = am.open("db.dat");
                os = new FileOutputStream(getFilesDir() + "/db.dat");

                while ((BytesRead = is.read(Buffer)) != -1) {
                    os.write(Buffer, 0, BytesRead);
                }
            }
            catch (IOException e) {
                e.printStackTrace();
            }
            finally {
                try {
                    if (is != null)
                        is.close();
                    if (os != null)
                        os.close();
                }
                catch (IOException e) {
                        e.printStackTrace();
                }
            }
        }
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