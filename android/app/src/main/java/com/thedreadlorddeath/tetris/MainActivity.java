package com.thedreadlorddeath.tetris;

import android.app.NativeActivity;
import android.os.Bundle;

public class MainActivity extends NativeActivity {
    static {
        System.loadLibrary("c++_shared");
        System.loadLibrary("openal");
        System.loadLibrary("sfml-system");
        System.loadLibrary("sfml-window");
        System.loadLibrary("sfml-graphics");
        System.loadLibrary("sfml-audio");
        System.loadLibrary("Tetris");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }
}
