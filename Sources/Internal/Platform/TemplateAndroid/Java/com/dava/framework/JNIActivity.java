package com.dava.framework;

import java.util.Arrays;
import java.util.List;
import java.util.HashSet;
import java.util.Set;

import org.fmod.FMODAudioDevice;

import android.app.Activity;
import android.os.PowerManager;
import android.content.Context;
import android.content.Intent;
import android.hardware.SensorManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.InputDevice;
import android.view.InputDevice.MotionRange;
import android.view.MotionEvent;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.Manifest.permission;
import android.content.pm.PackageManager;

import com.dava.framework.InputManagerCompat.InputDeviceListener;

public abstract class JNIActivity extends Activity implements JNIAccelerometer.JNIAccelerometerListener, InputDeviceListener
{
    public static boolean isPaused = true;
    // on start we will gain focus and know about it after onWindowFocusChanged
    // if user disable lock screen focus stay in our window
    public static boolean isFocused = false; // do not change
    
    public static boolean isSurfaceReady = false;

    protected JNISurfaceView surfaceView = null;

    private static int errorState = 0;

    private JNIAccelerometer accelerometer = null;
    private View splashView = null;
    
    private FMODAudioDevice fmodDevice = new FMODAudioDevice();
    
    private InputManagerCompat inputManager = null;
    
    private long mainLoopThreadID = 0;
    
    private native void nativeOnCreate();
    private native void nativeOnStart();
    private native void nativeOnStop();
    private native void nativeOnDestroy();
    private native void nativeFinishing();
    private native void nativeOnAccelerometer(float x, float y, float z);
    private native void nativeOnGamepadAvailable(boolean isAvailable);
    private native void nativeOnGamepadTriggersAvailable(boolean isAvailable);
    private native boolean nativeIsMultitouchEnabled();
    private native int nativeGetDesiredFPS();
    private native void nativeOnPause(boolean isLocked);
    private native void nativeOnResume();
    
    private static String commandLineParams = null;
    
    private volatile boolean mainThreadNeedExit = false;
    private volatile boolean mainThreadNeedResume = true;
    private volatile boolean mainThreadNeedSuspend = false;
    private final Object mainThreadSync = new Object();
    
    public abstract JNISurfaceView FindSurfaceView();
    
    private static JNIActivity activity = null;
    protected static SignalStrengthListener signalStrengthListener = null;
    protected static DataConnectionStateListener dataConnectionStateListener = null;

    public boolean GetIsPausing()
    {
        return isPaused;
    }

    public static JNIActivity GetActivity()
    {
        return activity;
    }
        
    public boolean HasPermission(String permission)
    {
        return activity.getPackageManager().checkPermission(permission, activity.getPackageName()) != PackageManager.PERMISSION_DENIED;
    }

    /**
     * Get instance of {@link JNISurfaceView} without loading content view
     * @return instance of {@link JNISurfaceView} or null
     */
    public JNISurfaceView GetSurfaceView() {
        return surfaceView;
    }
    
    // https://code.google.com/p/android/issues/detail?id=81083
    private void FixNoClassDefFoundError81083() {
        try {
            Class.forName("android.os.AsyncTask");
        }
        catch(Throwable e) {
            Log.e(JNIConst.LOG_TAG, e.toString());
        }
    }
    
    @Override
    public void onCreate(Bundle savedInstanceState) 
    {
        // The activity is being created.
        Log.d(JNIConst.LOG_TAG, "[Activity::onCreate] in");
        
        FixNoClassDefFoundError81083();
        
        activity = this;
        super.onCreate(savedInstanceState);
        
        commandLineParams = initCommandLineParams();
        
        //JNINotificationProvider.AttachToActivity();
        
        // initialize accelerometer
        SensorManager sensorManager = (SensorManager)getSystemService(Context.SENSOR_SERVICE);
        accelerometer = new JNIAccelerometer(this, sensorManager);

        getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_NOTHING | WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN | WindowManager.LayoutParams.FLAG_FULLSCREEN);
        getWindow().requestFeature(Window.FEATURE_ACTION_MODE_OVERLAY);
        getWindow().requestFeature(Window.FEATURE_ACTION_BAR_OVERLAY);
        getWindow().requestFeature(Window.FEATURE_NO_TITLE);
        
        final View decorView = getWindow().getDecorView();
        // Try hide navigation bar for detect correct GL view size
        HideNavigationBar(decorView);
        // Subscribe listener on UI changing for hiding navigation bar after keyboard hiding
        decorView.setOnSystemUiVisibilityChangeListener(new View.OnSystemUiVisibilityChangeListener() {
            @Override
            public void onSystemUiVisibilityChange(int visibility) {
                if((visibility & View.SYSTEM_UI_FLAG_HIDE_NAVIGATION) == 0) {
                    HideNavigationBar(decorView);
                }
            }
        });
        
        // initialize GL VIEW
        surfaceView = FindSurfaceView();
        assert(surfaceView != null);
        surfaceView.setFocusableInTouchMode(true);
        surfaceView.setClickable(true);
        surfaceView.setFocusable(true);
        surfaceView.requestFocus();
        
        inputManager = InputManagerCompat.Factory.getInputManager(this);
        
        splashView = GetSplashView();

        TelephonyManager tm = (TelephonyManager)getSystemService(TELEPHONY_SERVICE);
        if (!HasPermission(permission.READ_PHONE_STATE))
        {
            Log.d("", "AndroidManifest.xml haven't READ_PHONE_STATE permission!");
        }
        if ((tm != null) & HasPermission(permission.READ_PHONE_STATE)) {
            signalStrengthListener = new SignalStrengthListener();
            dataConnectionStateListener = new DataConnectionStateListener();
            tm.listen(signalStrengthListener, SignalStrengthListener.LISTEN_SIGNAL_STRENGTHS);
            tm.listen(dataConnectionStateListener, DataConnectionStateListener.LISTEN_DATA_CONNECTION_STATE);
        } else {
            Log.d("", "no singalStrengthListner");
            Log.d("", "no dataConnectionStateListener");
        }

        JNIApplication.mainCPPThread = new Thread(new Runnable() 
        {
            long startTime;
            
            @Override
            public void run() 
            {
                Log.d(JNIConst.LOG_TAG, "[C++ main thread] started!");
                
                // Initialize native framework core         
                JNIApplication.GetApplication().InitFramework(commandLineParams);

                nativeOnCreate();
                
                UpdateGamepadAxises();
                
                startTime = System.currentTimeMillis();
                
                while(!mainThreadNeedExit)
                {
                    {
                        long elapsedTime = System.currentTimeMillis() - startTime;
                        long fpsLimit = nativeGetDesiredFPS();
                        if (fpsLimit > 0)
                        {
                            long averageFrameTime = 1000L / fpsLimit;
                            if(averageFrameTime > elapsedTime)
                            {
                                long sleepMs = averageFrameTime - elapsedTime;
                                try {
                                    Thread.sleep(sleepMs);
                                } catch (InterruptedException e) {
                                    e.printStackTrace();
                                }
                            }
                        }
                        startTime = System.currentTimeMillis();
                    }
                    
                    surfaceView.ProcessQueuedEvents();

                    if(JNIActivity.this.mainThreadNeedResume)
                    {
                        Log.d(JNIConst.LOG_TAG, "[C++ main thread] resume native in");

                        nativeOnResume();

                        Log.d(JNIConst.LOG_TAG, "[C++ main thread] resume native out");

                        synchronized(mainThreadSync) {
                            JNIActivity.this.mainThreadNeedResume = false;
                            mainThreadSync.notify();
                        }
                    }
                    else if(JNIActivity.this.mainThreadNeedSuspend)
                    {
                        Log.d(JNIConst.LOG_TAG, "[C++ main thread] suspend native in");

                        boolean isScreenLocked = isScreenLocked();
                
                        nativeOnPause(isScreenLocked);

                        Log.d(JNIConst.LOG_TAG, "[C++ main thread] suspend native out");

                        synchronized(mainThreadSync) {
                            JNIActivity.this.mainThreadNeedSuspend = false;
                            mainThreadSync.notify();
                        }
                    }
                    else
                    {
                        surfaceView.ProcessFrame();
                    }
                }

                Log.d(JNIConst.LOG_TAG, "[C++ main thread] destroying native...");
                nativeOnDestroy();
                Log.d(JNIConst.LOG_TAG, "[C++ main thread] finished!");
            }
        }, "cpp_main_thread");

        mainLoopThreadID = JNIApplication.mainCPPThread.getId();
        JNIApplication.mainCPPThread.start();

        // check if we are starting from android notification popup
        // and execute appropriate runnable
        {
            JNINotificationProvider.AttachToActivity(this);
            final Intent intent = getIntent();

            RunOnMainLoopThread(new Runnable() {
                @Override
                public void run() {
                    if (null != intent) {
                        String uid = intent.getStringExtra("uid");
                        if (uid != null) {
                            JNINotificationProvider.NotificationPressed(uid);
                        }
                    }
                }
            });
        }

        // The activity is being created.
        Log.d(JNIConst.LOG_TAG, "[Activity::onCreate] out");
    }
    
    private String initCommandLineParams() {
        String commandLine = "";
        Bundle extras = this.getIntent().getExtras();
        if (extras != null) {
            commandLine = "";
            for (String key : extras.keySet()) {
                String value = extras.getString(key);
                commandLine += key + " " + value + " ";
            }
            commandLine = commandLine.trim();
        }
        Log.i(JNIConst.LOG_TAG, "command line params: " + commandLine);
        return commandLine;
    }
    
    public long GetMainLoopThreadID()
    {
        return mainLoopThreadID;
    }
    
    @Override
    protected void onStart()
    {
        Log.d(JNIConst.LOG_TAG, "[Activity::onStart] in");
        super.onStart();
        fmodDevice.start();
        
        if (isFocused)
        {
            // we not lost focus, it can happen if 
            // user disable any lock screen in security settings
            // and turn off screen, then turn on screen, our window
            // steel has focus
            HideSplashScreenView();
        }

        RunOnMainLoopThread(new Runnable() {
            public void run()
            {
                nativeOnStart();
            }
        });

        Log.d(JNIConst.LOG_TAG, "[Activity::onStart] out");
    }
    
    @Override
    protected void onRestart()
    {
        Log.d(JNIConst.LOG_TAG, "[Activity::onRestart] in");
        super.onRestart();
        Log.d(JNIConst.LOG_TAG, "[Activity::onRestart] out");
    }

    protected void handleSuspend()
    {
        Log.d(JNIConst.LOG_TAG, "[Activity::handleSuspend] in");

        if(!isPaused && isSurfaceReady)
        {
            // set paused flag. this will forbid
            // main c++ thread to process frames
            isPaused = true;

            synchronized(mainThreadSync) {

                // set flag to suspend main thread
                mainThreadNeedSuspend = true;

                // now wait until mainThreadNeedSuspend becomes =false
                // this will mean that c++ thread was suspended
                while(mainThreadNeedSuspend) {
                    try {
                        mainThreadSync.wait();
                    } catch(InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }

            Log.d(JNIConst.LOG_TAG, "[Activity::handleSuspend] suspended");
        }

        Log.d(JNIConst.LOG_TAG, "[Activity::handleSuspend] out");
    }

    protected void handleResume()
    {
        Log.d(JNIConst.LOG_TAG, "[Activity::handleResume] in");

        if(isPaused && isSurfaceReady && isFocused)
        {
            // remove paused flag. this will allow
            // main c++ thread to process frames
            isPaused = false;

            synchronized(mainThreadSync) {
                // set flag to resume main thread
                mainThreadNeedResume = true;

                // now wait until mainThreadNeedResume becomes =false
                // this will mean that c++ thread was resumed
                while(mainThreadNeedResume) {
                    try {
                        mainThreadSync.wait();
                    } catch(InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }

            Log.d(JNIConst.LOG_TAG, "[Activity::handleResume] resumed");
        }

        Log.d(JNIConst.LOG_TAG, "[Activity::handleResume] out");
    }

    @Override
    protected void onPause()
    {
        // reverse order of onResume
        // Another activity is taking focus (this activity is about to be "paused").
        Log.d(JNIConst.LOG_TAG, "[Activity::onPause] in");
        
        inputManager.unregisterInputDeviceListener(this);
        
        // deactivate accelerometer
        if(accelerometer != null)
        {
            if(accelerometer.IsActive())
            {
                accelerometer.Stop();
            }
        }
        
        handleSuspend();

        DestroyKeyboardLayout();

        super.onPause();

        Log.d(JNIConst.LOG_TAG, "[Activity::onPause] out");
    }
    
    @Override
    protected void onResume() 
    {
        Log.d(JNIConst.LOG_TAG, "[Activity::onResume] in");
        // recreate eglContext (also eglSurface, eglScreen) should be first
        super.onResume();
         
        // activate accelerometer
        if(accelerometer != null)
        {
            if(accelerometer.IsSupported())
            {
                accelerometer.Start();
            }
        }

        inputManager.registerInputDeviceListener(this, null);
        // if we connect gamepad after start game and do not 
        // receive even onInputDeviceAdded double check gamepad axis
        UpdateGamepadAxises();
        JNIUtils.keepScreenOnOnResume();
        
        JNITextField.RelinkNativeControls();
        JNIWebView.RelinkNativeControls();

        handleResume();

        /*
         Start of workaround
         
         Here we need to hide navigation bar. 
         Activity is configured to hide the bar, but seems android shows the bar before activity. 
         In that case we need to hide the bar. 
         */
         // start of workaround ---->
        Runnable navigationBarHider = new Runnable() {
            @Override
            public void run() {
                HideNavigationBar(getWindow().getDecorView());
            }
        };
        
        Handler handler = new Handler(Looper.getMainLooper());
        handler.postDelayed(navigationBarHider, 500);
        // <----- end of workaround

        Log.d(JNIConst.LOG_TAG, "[Activity::onResume] out");
    }

    @Override
    protected void onStop()
    {
        Log.d(JNIConst.LOG_TAG, "[Activity::onStop] in");

        ShowSplashScreenView();
                
        //call native method
        RunOnMainLoopThread(new Runnable() {
            public void run()
            {
                nativeOnStop();
            }
        });
        
        fmodDevice.stop();
        super.onStop();
        
        // The activity is no longer visible (it is now "stopped")
        Log.d(JNIConst.LOG_TAG, "[Activity::onStop] out");
    }
    
    
    @Override
    protected void onDestroy()
    {
        Log.d(JNIConst.LOG_TAG, "[Activity::onDestroy] in");

        // set flag to notify C++ thread to finish
        mainThreadNeedExit = true;
        
        Log.d(JNIConst.LOG_TAG, "[Activity::onDestroy] c++ main thread join start");

        // Now wait for the CPP thread to quit
        if (JNIApplication.mainCPPThread != null) {
            try {
                JNIApplication.mainCPPThread.join();
            } catch(Exception e) {
                Log.v(JNIConst.LOG_TAG, "Problem stopping mainCPPThread: " + e);
            }
            JNIApplication.mainCPPThread = null;
        }

        Log.d(JNIConst.LOG_TAG, "[Activity::onDestroy] c++ main thread join end");

        super.onDestroy();

        Log.d(JNIConst.LOG_TAG, "[Activity::onDestroy] out");

        Log.d(JNIConst.LOG_TAG, "[Activity::onDestroy] System exit");
        System.exit(0);
    }
    
    @Override
    public void onBackPressed() {
    }
    
    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        Log.d(JNIConst.LOG_TAG, "[Activity::onWindowFocusChanged] in");
        // clear key tracking state, so should always be called
        // now we definitely shown on screen
        // http://developer.android.com/reference/android/app/Activity.html#onWindowFocusChanged(boolean)
        super.onWindowFocusChanged(hasFocus);

        isFocused = hasFocus;
        handleResume();
            
        if(hasFocus) {
            HideSplashScreenView();
            HideNavigationBar(getWindow().getDecorView());
            InitKeyboardLayout();
        }
        Log.d(JNIConst.LOG_TAG, "[Activity::onWindowFocusChanged] out");
    }
    
    // we have to call next function after initialization of glView
    void InitKeyboardLayout() {
        // first destroy if any keyboard layout
        final WindowManager windowManager = getWindowManager();
        JNITextField.DestroyKeyboardLayout(windowManager);
        
        // now initialize one more time
        // http://stackoverflow.com/questions/7776768/android-what-is-android-r-id-content-used-for
        final View v = findViewById(android.R.id.content);
        
        if (v == null)
        {
            throw new RuntimeException("findViewById returned null - strange null pointer view");
        }
        
        if(v.getWindowToken() != null)
        {
            JNITextField.InitializeKeyboardLayout(windowManager, v.getWindowToken());
        }
        else
        {
            // if there is no window token - seems view is still not attached to window.
            // wait for it.
            v.addOnAttachStateChangeListener(new View.OnAttachStateChangeListener() {
                
                @Override
                public void onViewDetachedFromWindow(View v) {
                    v.removeOnAttachStateChangeListener(this);
                }
                
                @Override
                public void onViewAttachedToWindow(View v) {
                    JNITextField.InitializeKeyboardLayout(windowManager, v.getWindowToken());
                    v.removeOnAttachStateChangeListener(this);
                }
            });
        }
    }

    void DestroyKeyboardLayout() {
        final WindowManager windowManager = getWindowManager();
        JNITextField.DestroyKeyboardLayout(windowManager);
    }
    
    protected final List<Integer> supportedAxises = Arrays.asList(
            MotionEvent.AXIS_X,
            MotionEvent.AXIS_Y,
            MotionEvent.AXIS_Z,
            MotionEvent.AXIS_RX,
            MotionEvent.AXIS_RY,
            MotionEvent.AXIS_RZ,
            MotionEvent.AXIS_LTRIGGER,
            MotionEvent.AXIS_RTRIGGER,
            MotionEvent.AXIS_BRAKE,
            MotionEvent.AXIS_GAS
    );
    
    protected void UpdateGamepadAxises()
    {
        RunOnMainLoopThread(new Runnable() 
        {
            @Override
            public void run() 
            {

                boolean isGamepadAvailable = false;
                int[] inputDevices = InputDevice.getDeviceIds();
                Set<Integer> avalibleAxises = new HashSet<Integer>();
                for (int id : inputDevices) {
                    InputDevice device = InputDevice.getDevice(id);
                    if ((device.getSources()
                            & InputDevice.SOURCE_CLASS_JOYSTICK) > 0) {
                        isGamepadAvailable = true;

                        List<MotionRange> ranges = device.getMotionRanges();
                        for (MotionRange r : ranges) {
                            int axisId = r.getAxis();
                            if (supportedAxises.contains(axisId)) {
                                avalibleAxises.add(axisId);
                            }
                        }
                        break; // only first connected device
                    }
                }

                Integer[] axisIds = new Integer[avalibleAxises.size()];
                surfaceView.SetAvailableGamepadAxises(
                        avalibleAxises.toArray(axisIds));

                nativeOnGamepadAvailable(isGamepadAvailable);
                nativeOnGamepadTriggersAvailable(avalibleAxises
                        .contains(MotionEvent.AXIS_LTRIGGER)
                        || avalibleAxises.contains(MotionEvent.AXIS_BRAKE));
            }
        });
    }
    
    @Override
    public void onInputDeviceAdded(int deviceId) 
    {
        UpdateGamepadAxises();
    }

    @Override
    public void onInputDeviceChanged(int deviceId) 
    {
        UpdateGamepadAxises();
    }

    @Override
    public void onInputDeviceRemoved(int deviceId) 
    {
        UpdateGamepadAxises();
    }
    
    public void onAccelerationChanged(float x, float y, float z)
    {
        nativeOnAccelerometer(x, y, z);
    }
    
    // execute on c++ main thread (GL, Game) not Java UI main thread
    public void RunOnMainLoopThread(Runnable event) {
        surfaceView.queueEvent(event);
    }

    public int GetNotificationIcon() {
        return android.R.drawable.sym_def_app_icon;
    }
    
    /**
     * Since API 19 we can hide Navigation bar (Immersive Full-Screen Mode)
     */
    public static void HideNavigationBar(View view) {
        // The UI options currently enabled are represented by a bitfield.
        // getSystemUiVisibility() gives us that bitfield.
        int uiOptions = view.getSystemUiVisibility();
        
        // Navigation bar hiding:  Backwards compatible to ICS.
        // Don't use View.SYSTEM_UI_FLAG_HIDE_NAVIGATION on API less that 19 because any
        // click on view shows navigation bar, and we must hide it manually only. It is
        // bad workflow.

        // Status bar hiding: Backwards compatible to Jellybean
        if (Build.VERSION.SDK_INT >= 16) {
            uiOptions |= 0x00000004; //View.SYSTEM_UI_FLAG_FULLSCREEN;
        }

        // Immersive mode: Backward compatible to KitKat.
        // Note that this flag doesn't do anything by itself, it only augments the behavior
        // of HIDE_NAVIGATION and FLAG_FULLSCREEN.  For the purposes of this sample
        // all three flags are being toggled together.
        // Note that there are two immersive mode UI flags, one of which is referred to as "sticky".
        // Sticky immersive mode differs in that it makes the navigation and status bars
        // semi-transparent, and the UI flag does not get cleared when the user interacts with
        // the screen.
        if (Build.VERSION.SDK_INT >= 19) {
            uiOptions |= View.SYSTEM_UI_FLAG_HIDE_NAVIGATION 
                    | 0x00000200 //View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                    | 0x00000100 //View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                    | 0x00000400 //View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                    | 0x00001000; //View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
        }
        
        view.setSystemUiVisibility(uiOptions);
    }
    
    public View GetSplashView() {
        return null;
    }
    
    public void ShowSplashScreenView() {
        if(null != splashView) {
            Log.d(JNIConst.LOG_TAG, "splashView set visible");
            splashView.setVisibility(View.VISIBLE);
        }

        JNITextField.HideAllTextFields();
        JNIWebView.HideAllWebViews();
    }
    
    public void HideSplashScreenView() {
        if(null != splashView) {
            Log.d(JNIConst.LOG_TAG, "splashView hide");
            splashView.setVisibility(View.GONE);
        }

        JNITextField.ShowVisibleTextFields();
        JNIWebView.ShowVisibleWebViews();
        surfaceView.SetMultitouchEnabled(nativeIsMultitouchEnabled());
    }
    
    @SuppressWarnings("deprecation")
    private boolean isScreenLocked() {
        PowerManager pm = (PowerManager) JNIApplication.GetApplication().getSystemService(Context.POWER_SERVICE);
        boolean isScreenLocked = false;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT_WATCH) {
            isScreenLocked = !pm.isInteractive();
        } else {
            isScreenLocked = !pm.isScreenOn();
        }
        return isScreenLocked;
    }

    public static void finishActivity()
    {
        Log.d(JNIConst.LOG_TAG, "[Activity::finishActivity] in");
        
        activity.runOnUiThread(new Runnable(){
            @Override
            public void run() {
                activity.finish();
            }
        });
        
        Log.d(JNIConst.LOG_TAG, "[Activity::finishActivity] out");
    }
}

