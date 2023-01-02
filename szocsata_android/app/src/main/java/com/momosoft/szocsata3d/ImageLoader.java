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
    private static  int m_ColorDepth  = 0;

    ImageLoader(Context ctx) {
        m_Context = ctx;
    }

    public static boolean LoadImage(String path) throws IOException {
        InputStream is = null;
        Bitmap bmp = null;

        try {
            AssetManager as = m_Context.getAssets();
            is = as.open(path);
            bmp = BitmapFactory.decodeStream(is);
        }
        catch (IOException e) {
            return false;
        }
        finally {
            if (is != null) {
                is.close();
            }
        }

        Matrix matrix = new Matrix();
        matrix.postScale(1, -1, bmp.getWidth() / 2f, bmp.getHeight() / 2f);
        bmp = Bitmap.createBitmap(bmp, 0, 0, bmp.getWidth(), bmp.getHeight(), matrix, true);

        int ByteCount = bmp.getRowBytes() * bmp.getHeight();
        m_ImageBytes = ByteBuffer.allocate(ByteCount);
        bmp.copyPixelsToBuffer(m_ImageBytes);
        m_Width = bmp.getWidth();
        m_Height = bmp.getHeight();

        if (bmp.getConfig() == Bitmap.Config.RGB_565) {
            m_ColorDepth = 3;
        }
        else if (bmp.getConfig() == Bitmap.Config.ARGB_8888) {
            m_ColorDepth = 4;
        }

        return true;
    }

    public static int GetWidth() {
        return m_Width;
    }

    public static int GetColorDepth() {
        return m_ColorDepth;
    }

    public static int GetHeight() {
        return m_Height;
    }

    public static byte[] GetImageBytes() {
        return  m_ImageBytes.array();
    }
}
