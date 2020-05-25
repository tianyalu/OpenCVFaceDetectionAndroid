package com.sty.opencv.facedetection.android;

import android.Manifest;
import android.content.Context;
import android.content.Intent;
import android.hardware.Camera;
import android.support.annotation.NonNull;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.text.PrecomputedText;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.TextView;

import com.sty.opencv.facedetection.android.util.PermissionUtils;

import org.opencv.core.Mat;
import org.opencv.samples.facedetect.FdActivity;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = MainActivity.class.getSimpleName();
    private String[] needPermissions = {Manifest.permission.CAMERA, Manifest.permission.WRITE_EXTERNAL_STORAGE};
    private SurfaceView surfaceView;
    private Button btnSwitchCamera;
    private Button btnDetect;
    private CameraHelper cameraHelper;

    private File mCascadeFile;

    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.activity_main);

        copyCascadeFile();
        initViews();

        if(!PermissionUtils.checkPermissions(this, needPermissions)) {
            PermissionUtils.requestPermissions(this, needPermissions);
        }else {
            initTracker(mCascadeFile.getAbsolutePath());
            cameraHelper.rePreview();
            cameraHelper.setPreviewCallback(new Camera.PreviewCallback() {
                @Override
                public void onPreviewFrame(byte[] data, Camera camera) {
                    getFaceData(data, cameraHelper.getmWidth(), cameraHelper.getmHeight(), cameraHelper.getmCameraID());
//                    camera.addCallbackBuffer(data);
                }
            });
        }
    }

    private void initViews() {
        surfaceView = findViewById(R.id.surface_view);
        btnSwitchCamera = findViewById(R.id.btn_switch_camera);
        btnDetect = findViewById(R.id.btn_detect);

        cameraHelper = new CameraHelper(MainActivity.this);
        cameraHelper.setPreviewDisplay(surfaceView.getHolder());


        btnSwitchCamera.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                cameraHelper.switchCamera();
            }
        });

        btnDetect.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                startActivity(new Intent(MainActivity.this, FdActivity.class));
            }
        });
    }

    @Override
    protected void onResume() {
        super.onResume();
//        initTracker(mCascadeFile.getAbsolutePath());
    }

    private void copyCascadeFile() {
        try {
            // load cascade file from application resources
            File cascadeDir = getDir("cascade", Context.MODE_PRIVATE);
            mCascadeFile = new File(cascadeDir, "lbpcascade_frontalface.xml");
//            if(mCascadeFile.exists()) {
//                return;
//            }
            InputStream is = getResources().openRawResource(R.raw.lbpcascade_frontalface);
            FileOutputStream os = new FileOutputStream(mCascadeFile);

            byte[] buffer = new byte[4096];
            int bytesRead;
            while ((bytesRead = is.read(buffer)) != -1) {
                os.write(buffer, 0, bytesRead);
            }
            is.close();
            os.close();
        } catch (IOException e) {
            e.printStackTrace();
            Log.e(TAG, "Failed to load cascade. Exception thrown: " + e);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions,
                                           @NonNull int[] grantResults) {
        if(requestCode == PermissionUtils.REQUEST_PERMISSIONS_CODE){
            if(!PermissionUtils.verifyPermissions(grantResults)) {
                PermissionUtils.showMissingPermissionDialog(this);
            }else {
                cameraHelper.rePreview();
                initTracker(mCascadeFile.getAbsolutePath());
            }
        }
    }

    /**
     * 初始化追踪器
     * @param cascadeFile
     */
    public native void initTracker(String cascadeFile);


    /**
     * 得到画出人脸图像的数据
     * @param data
     * @param w
     * @param h
     * @param cameraId
     * @return
     */
    public native byte[] getFacedData(byte[] data, int w, int h, int cameraId);

    public native void getFaceData(byte[] data, int w, int h, int cameraId);

    /**
     * 释放追踪器
     */
    public native void releaseTracker();

}
