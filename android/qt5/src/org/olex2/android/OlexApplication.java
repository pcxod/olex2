package org.olex2.android;

import android.app.Activity;
import android.app.Application;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import org.qtproject.qt5.android.bindings.QtApplication;

// runs KeepAlive while no activity is visible and the app was not closed
public class OlexApplication extends QtApplication {
    private int started;

    @Override
    public void onCreate() {
        super.onCreate();
        registerActivityLifecycleCallbacks(new Application.ActivityLifecycleCallbacks() {
            @Override
            public void onActivityStarted(Activity a) {
                if (started++ == 0) {
                    stopService(new Intent(a, KeepAlive.class));
                }
            }

            @Override
            public void onActivityStopped(Activity a) {
                if (--started == 0 && !a.isFinishing()) {
                    Intent i = new Intent(a, KeepAlive.class);
                    if (Build.VERSION.SDK_INT >= 26) {
                        startForegroundService(i);
                    } else {
                        startService(i);
                    }
                }
            }

            @Override
            public void onActivityDestroyed(Activity a) {
                if (a.isFinishing()) {
                    stopService(new Intent(a, KeepAlive.class));
                }
            }

            @Override public void onActivityCreated(Activity a, Bundle b) {}
            @Override public void onActivityResumed(Activity a) {}
            @Override public void onActivityPaused(Activity a) {}
            @Override public void onActivitySaveInstanceState(Activity a, Bundle b) {}
        });
    }
}
