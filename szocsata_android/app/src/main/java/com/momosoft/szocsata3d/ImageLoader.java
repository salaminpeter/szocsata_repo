package com.momosoft.szocsata3d;

import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;

public class ImageLoader {

    public static Context m_Context; //TODO
    private static  ByteBuffer m_ImageBytes = null;
    private static  int m_Width = 0;
    private static  int m_Height = 0;
    private static  int m_ByteCount  = 0;

    ImageLoader(Context ctx) {
        m_Context = ctx;
    }

    public static void LoadImage(String path) throws IOException {
        InputStream is = null;
        Bitmap bmp = null;

        try {
            AssetManager as = m_Context.getAssets();
            is = as.open(path);
            bmp = BitmapFactory.decodeStream(is);
        }
        catch (IOException e) {
            Log.d("", "f");
        }
        finally {
            if (is != null) {
                is.close();
            }
        }

        Matrix matrix = new Matrix();
        matrix.postScale(1, -1, bmp.getWidth() / 2f, bmp.getHeight() / 2f);
        bmp = Bitmap.createBitmap(bmp, 0, 0, bmp.getWidth(), bmp.getHeight(), matrix, true);

        m_ByteCount = bmp.getRowBytes() * bmp.getHeight();
        m_ImageBytes = ByteBuffer.allocate(m_ByteCount);
        bmp.copyPixelsToBuffer(m_ImageBytes);
        m_Width = bmp.getWidth();
        m_Height = bmp.getHeight();
    }

    public static int GetWidth() {
        return m_Width;
    }

    public static int GetByteCount() {
        return m_ByteCount;
    }

    public static int GetHeight() {
        return m_Height;
    }

    public static byte[] GetImageBytes() {
        return  m_ImageBytes.array();
    }
}
