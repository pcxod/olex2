package org.olex2.android;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Intent;
import android.os.Build;
import android.os.IBinder;

/* Foreground service while Olex2 is in the background: the process moves from
cached (adj 900+) to perceptible (adj 200), so the low-memory killer takes the
cached apps first and a reopen is a 0.2 s resume, not a 9 s cold start
(Python, cctbx, GUI). Started and stopped by OlexApplication. */
public class KeepAlive extends Service {
    static final String CHANNEL = "olex2_keepalive";

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Notification.Builder b;
        if (Build.VERSION.SDK_INT >= 26) {
            NotificationManager nm = getSystemService(NotificationManager.class);
            nm.createNotificationChannel(new NotificationChannel(CHANNEL,
                "Olex2 in the background", NotificationManager.IMPORTANCE_LOW));
            b = new Notification.Builder(this, CHANNEL);
        } else {
            b = new Notification.Builder(this);
        }
        Intent open = getPackageManager().getLaunchIntentForPackage(getPackageName());
        b.setSmallIcon(android.R.drawable.ic_dialog_info)
            .setContentTitle("Olex2")
            .setContentText("Kept in memory for a quick return")
            .setContentIntent(PendingIntent.getActivity(this, 0, open, 0));
        startForeground(1, b.build());
        return START_NOT_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
}
