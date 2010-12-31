/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package olex2j;

import java.io.File;
import java.lang.reflect.Method;
import java.net.URL;
import java.net.URLClassLoader;

/**
 * http://forum.java.sun.com/thread.jspa?threadID=300557&start=45
 * @author Oleg
 */
public class ClassPathHacker {

    private static final Class[] parameters = new Class[]{URL.class};

    public static void addFile(String s) {
        File f = new File(s);
        addFile(f);
    }

    /* File.toURL() was deprecated, so use File.toURI().toURL() */
    public static void addFile(File f) {
        try {
            addURL(f.toURI().toURL());
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void addURL(URL u) {
        URLClassLoader sysloader = (URLClassLoader) ClassLoader.getSystemClassLoader();
        try {
            /* Class was uncheched, so used URLClassLoader.class instead */
            Method method = URLClassLoader.class.getDeclaredMethod("addURL", parameters);
            method.setAccessible(true);
            method.invoke(sysloader, new Object[]{u});
            System.out.println("Dynamically added " + u.toString() + " to classLoader");
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
