package com.sty.opencv.facedetection.android;

import android.Manifest;
import android.content.Intent;
import android.support.annotation.NonNull;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.sty.opencv.facedetection.android.util.PermissionUtils;

import org.opencv.samples.facedetect.FdActivity;

public class MainActivity extends AppCompatActivity {
    private String[] needPermissions = {Manifest.permission.CAMERA};
    private SurfaceView surfaceView;
    private Button btnSwitchCamera;
    private Button btnDetect;
    private CameraHelper cameraHelper;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        initViews();

        if(!PermissionUtils.checkPermissions(this, needPermissions)) {
            PermissionUtils.requestPermissions(this, needPermissions);
        }else {
            cameraHelper.rePreview();
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
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions,
                                           @NonNull int[] grantResults) {
        if(requestCode == PermissionUtils.REQUEST_PERMISSIONS_CODE){
            if(!PermissionUtils.verifyPermissions(grantResults)) {
                PermissionUtils.showMissingPermissionDialog(this);
            }else {
                cameraHelper.rePreview();
            }
        }
    }
}
