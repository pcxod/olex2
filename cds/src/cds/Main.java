/*
 * Download manager server, (c) O. Dolomanov, 2010
 */

package cds;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;

/**
 *
 * @author Oleg
 */
public class Main {
  static final int port_number = 8082;
  static boolean terminate = false;
  static int threadCount = 0;
  static String baseDir;
  public synchronized static void doTerminate()  {  terminate = true;  }
  public static String getBaseDir()  {  return baseDir;  }
  public synchronized static void onThreadTerminate()  {
    threadCount--;
  }
  public static void print(String s)  {  System.out.println(s);  }
  public static void main(String[] args) {
    print("Download manager server, (c) O. Dolomanov, 2010");
    String status = "unknown";
    // read current status
    try {
      Socket s = new Socket();
      s.connect(new InetSocketAddress("localhost", port_number));
      //s.connect(new InetSocketAddress("www.olex2.org", 80));
      PrintWriter out = new PrintWriter(s.getOutputStream(), true);
      out.println("status");
      //out.print("GET http://www.olex2.org/olex2-distro/tags.txt HTTP/1.0\n\n");
      out.flush();
      BufferedReader in = new BufferedReader(new InputStreamReader(s.getInputStream()));
      //while( (status=in.readLine()) != null )
      //  print(status);
      status = in.readLine();
      out.close();
      in.close();
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
      if( args.length != 2 )  {
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
        ServerSocket s = new ServerSocket(port_number);
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
      }
      catch(Exception e)  {
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
        out.println("stop");
        out.close();
        print("The server has been successfully stopped.");
      }
      catch (IOException ex) {
        print("Failed to stop the server.");
      }
    }
  }
}
