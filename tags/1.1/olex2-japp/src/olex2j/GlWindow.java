/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package olex2j;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;

/**
 *
 * @author Oleg
 */
public class GlWindow {
    protected static int InstanceCount=0;
    protected static  boolean Initialised = false;
    protected int InstanceId;
    public GlWindow()  {
        InstanceId = InstanceCount++;
    }
    static {
        try {
            System.loadLibrary("olex2vj");
            Initialised = true;
        } catch (java.lang.UnsatisfiedLinkError e) {
        }
        if (!Initialised) {
            String dllFileName;
            try {
                File testFile = File.createTempFile("test", "~");
                testFile.deleteOnExit();
                String tmpPath = testFile.getPath();
                int lsc = tmpPath.lastIndexOf(File.separator);
                tmpPath = tmpPath.substring(0, lsc + 1);
                dllFileName = tmpPath + "olex2vj.dll";
                File dllFile = new File(dllFileName);
                InputStream is = GlWindow.class.getResourceAsStream("/olex2vj.dll");
                // !dllFile.exists(), dllFile.canWrite()
                boolean act = true;
                try  {
                if( dllFile.exists() )
                    dllFile.delete();
                }
                catch( Exception e)  {
                    act = false;
                }
                if( true ) {
                    if (dllFile.createNewFile()) {
                        byte[] bf = new byte[1024 * 64];
                        FileOutputStream os = new FileOutputStream(dllFile);
                        int rc;
                        while ((rc = is.read(bf)) != -1) {
                            os.write(bf, 0, rc);
                        }
                        os.close();
                    }
                }

            } catch (Exception exc) {
                dllFileName = "";
            }
            if (!dllFileName.isEmpty()) {
                System.load(dllFileName);
                Initialised = true;
            }
        }
    //System.loadLibrary("olex2vj");
    }
    public boolean isInitialised() {  return Initialised;  }
    public int getInstanceId()  {  return InstanceId;  }
    public void doFinalise()  {
        if( !Initialised )  return;
        Initialised = false;
        finalise(InstanceId);
    }
    public final static short olxv_MouseUp = 1,  olxv_MouseDown = 2,  olxv_MouseMove = 3,  olxv_MouseLeft = 1,  olxv_MouseMiddle = 2,  olxv_MouseRight = 4,  olxv_ShiftCtrl = 1,  olxv_ShiftShift = 2,  olxv_ShiftAlt = 4;
    
    public native void paint(int intstance, int[] buffer, int width, int height);
    
    public native boolean mouseEvt(int intstance, int x, int y, short evt, short btn, short shiftState);
    
    public native void fileChanged(int intstance, String fileName);
    
    public native byte[] getStatus(int intstance);
    
    public native byte[] getSelectionInfo(int intstance);

    public native byte[] getObjectLabelAt(int intstance, int x, int y);

    public native void showLabels(int intstance, boolean v);

    protected native void finalise(int intstance);
}
