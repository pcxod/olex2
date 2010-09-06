/*
 * Download manager server, (c) O. Dolomanov, 2010
 */

package cds;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashSet;

/**
 *
 * @author Oleg
 */
public class Main {
  static final int port_number = 8082;
  static boolean terminate = false;
  static int threadCount = 0;
  static String baseDir;
  static PrintWriter logFile=null;
  static HashSet<String> blocked = new HashSet();
  public synchronized static void doTerminate()  {  terminate = true;  }
  public static String getBaseDir()  {  return baseDir;  }
  public synchronized static void onThreadTerminate()  {
    threadCount--;
  }
  public static boolean shouldHandle(String src)  {
    return !blocked.contains(src);
  }
  public static void print(String s)  {
    if( logFile != null )  {
      logFile.println(s);
      logFile.flush();
    }
    System.out.println(s);
  }
  public static void main(String[] args) {
    print("Download manager server, (c) O. Dolomanov, 2010");
    String status = "unknown";
    // read current status
    try {
      Socket s = new Socket();
      s.connect(new InetSocketAddress("localhost", port_number));
      PrintWriter out = new PrintWriter(s.getOutputStream(), true);
      out.println("status\n");
      BufferedReader in = new BufferedReader(new InputStreamReader(s.getInputStream()));
      status = in.readLine();
      out.close();
      in.close();
      s.close();
    }
    catch (IOException ex) {}  // unknow status
    if( args.length == 0 )  {
      print("Status: " + status);
    }
    else if( args[0].equals("start") )  {
      if( status.equals("running") )  {
        print("Already running...");
        return;
      }
      if( args.length < 2 )  {
        print("Please provide the exposed root folder");
        return;
      }
      // set up the base dir
      File ef = new File(args[1]);
      if( !ef.exists() || !ef.isDirectory() )  {
        print("The exposed location must be an existing/valid folder");
        return;
      }
      try {
        baseDir = ef.getCanonicalPath();
      }
      catch (IOException e) {
        print("The exposed location must be an existing/valid folder");
        return;
      }
      if( !baseDir.endsWith(File.pathSeparator))
        baseDir += File.separatorChar;
      print("Exposing: " + baseDir);
      try  {
        if( args.length == 4 && args[2].equals("log") )  {
           File log = new File(args[3]);
           logFile = new PrintWriter(new FileWriter(log, true));
        }
        if( args.length == 6 && args[4].equals("blocked") )  {
           BufferedReader reader = new BufferedReader(new FileReader(args[5]));
           String line = null;
           while( (line = reader.readLine()) != null )
             blocked.add(line);
        }
        ServerSocket s = new ServerSocket(port_number);
        print("Server started at " + (new SimpleDateFormat("yyyy.MM.dd HH:mm:ss")).format(new Date()));
        while( true )  {
          try  {
            Socket c = s.accept();
            threadCount++;
            (new ClientHandler(c)).run();
            if( terminate )  {
              while( threadCount != 0 )
                Thread.sleep(1000);
              break;
            }
          }
          catch(Exception e)  {
            print("Failed to process the client");
          }
        }
        s.close();
        logFile.close();
        logFile = null;
      }
      catch(Exception e)  {
        if( logFile != null )  {
          logFile.close();
          logFile = null;
        }
        e.printStackTrace();
      }
    }
    else if( args[0].equals("stop") )  {
      if( !status.equals("running") )  {
        print("Not running...");
        return;
      }
      try {
        Socket s = new Socket();
        s.connect(new InetSocketAddress("localhost", port_number));
        PrintWriter out = new PrintWriter(s.getOutputStream(), true);
        out.println("stop\n");
        out.close();
        s.close();
        print("The server has been successfully stopped.");
      }
      catch (IOException ex) {
        ex.printStackTrace();
      }
    }
  }
}
