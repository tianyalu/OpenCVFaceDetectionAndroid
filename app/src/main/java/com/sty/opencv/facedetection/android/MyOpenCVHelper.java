package com.sty.opencv.facedetection.android;

import android.app.Activity;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class MyOpenCVHelper implements CameraHelper.PreviewCallback, CameraHelper.OnChangedSizeListener
        , SurfaceHolder.Callback {

    static {
        System.loadLibrary("native-lib");
    }

    private static final String TAG = MyOpenCVHelper.class.getSimpleName();
    private CameraHelper cameraHelper;
    private SurfaceHolder surfaceHolder;
    private Activity activity;
    private int width;
    private int height;
    private ExecutorService singleExecutorService; //单线程池

    public MyOpenCVHelper(Activity activity) {
        this.activity = activity;
        singleExecutorService = Executors.newSingleThreadExecutor();
    }


    public void createCamera(SurfaceHolder surfaceHolder) {
        cameraHelper = new CameraHelper(activity, surfaceHolder);
        cameraHelper.setPreviewCallback(this);
        cameraHelper.setOnChangedSizeListener(this);
    }

    public void setSurfacePreview(SurfaceHolder surfaceHolder) {
        if(surfaceHolder != null) {
            surfaceHolder.removeCallback(this);
        }
        this.surfaceHolder = surfaceHolder;
        this.surfaceHolder.addCallback(this);
    }

    //surface创建回调
    @Override
    public void surfaceCreated(SurfaceHolder surfaceHolder) {
        setSurfaceNative(surfaceHolder.getSurface());
    }

    //surface发送改变回调，一般是横竖屏切换
    @Override
    public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int i1, int i2) {

    }

    //surface销毁回调
    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {

    }

    //CameraHelper方向大小发生改变时回调，一般是横竖屏切换
    @Override
    public void onChanged(int w, int h) {
        this.width = w;
        this.height = h;
    }

    //CameraHelper摄像头预览获取到数据回调
    @Override
    public void onPreviewFrame(byte[] bytes) {
        renderFrameNative(bytes, width, height);
    }


    /**
     * native 方法
     */
    //初始化：人脸模型路径，追踪器
    public native void initNative(String faceModelPath);
    //设置native surface
    private native void setSurfaceNative(Surface surface);
    //渲染
    private native void renderFrameNative(byte[] bytes,int width,int height);
}
