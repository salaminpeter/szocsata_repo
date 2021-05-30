package com.example.szocsata_android;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;

import java.util.logging.Handler;

public class GameThread extends Thread {

    GameThread() {
        setName("GameThread");
    }

    @Override
    public void run() {

        while (true) {
            GameLoop();
        }
    }

    public native void GameLoop();
}
