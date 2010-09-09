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
import java.util.ArrayList;
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
  static HashSet<String> unmounted = new HashSet();
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
  static ArrayList<String> doCall(String function, String arg)  {
    ArrayList<String> rv = new ArrayList();
    try {
      Socket s = new Socket();
      s.connect(new InetSocketAddress("localhost", port_number));
      PrintWriter out = new PrintWriter(s.getOutputStream(), true);
      out.println(function);
      out.println(arg + '\n');
      BufferedReader in = new BufferedReader(new InputStreamReader(s.getInputStream()));
      String line = null;
      while ( (line = in.readLine()) != null && !line.isEmpty() )
        rv.add(line);
      out.close();
      in.close();
      s.close();
    }
    catch (IOException ex) {}  // unknow status
    return rv;
  }
  public static void main(String[] args) {
    print("Download manager server, (c) O. Dolomanov, 2010");
    String status = "unknown", status_info = "none";
    ArrayList<String> st = doCall("status", "");
    if( st.size() == 2 )  {
      status = st.get(0);
      status_info = st.get(1);
    }
    if( args.length == 0 )  {
      print("Status: " + status);
      print("Status Info: " + status_info);
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
            (new ClientHandler(c)).start();
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
        if( logFile != null )  {
          logFile.close();
          logFile = null;
        }
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
      st = doCall(args[0], "");
      st = doCall(args[0], "");  // have to call twice since Accept is blocking
      if( st.size() == 1 && st.get(0).equals("OK"))
        print("The server has been successfully stopped.");
      else
        print("Failed to stop server.");
    }
    else if( args[0].equals("block") && args.length == 2 )  {
      if( !status.equals("running") )  {
        print("Not running...");
        return;
      }
      st = doCall("block", args[1]);
      if( st.size() == 1 && st.get(0).equals("OK") )
        print("The address has been blocked for the session.");
      else
        print("Failed to block the address.");
    }
    else if( args[0].equals("unblock") && args.length == 2 )  {
      if( !status.equals("running") )  {
        print("Not running...");
        return;
      }
      st = doCall(args[0], args[1]);
      if( st.size() == 1 && st.get(0).equals("OK") )
        print("The address has been blocked for the session.");
      else
        print("Failed to unblock the address.");
    }
    else if( args[0].equals("mount") && args.length == 2 )  {
      if( !status.equals("running") )  {
        print("Not running...");
        return;
      }
      st = doCall(args[0], args[1]);
      if( st.size() == 1 && st.get(0).equals("OK") )
        print("Mounted: " + args[1]);
      else
        print("Failed to mount the folder: " + args[1]);
    }
    else if( args[0].equals("unmount") && args.length == 2 )  {
      if( !status.equals("running") )  {
        print("Not running...");
        return;
      }
      st = doCall(args[0], args[1]);
      if( st.size() >= 1 && st.get(0).equals("OK") )
        print("Unmounted for the session: " + args[1]);
      else
        print("Failed to unmount the folder: " + args[1]);
    }
  }
}
