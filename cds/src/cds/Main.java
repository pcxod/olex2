/*
 * Download manager server, (c) O. Dolomanov, 2010
 */

package cds;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.security.MessageDigest;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Map.Entry;

/**
 *
 * @author Oleg
 */
public class Main {
  static class FileHash  {
    long timestamp, size;
    String hash;
  }
  static final String digestChars = "0123456789abcdef";
  static int port_number = 8082;
  static boolean terminate = false;
  static int threadCount = 0;
  static String baseDir, hashesFileName = "";
  static PrintWriter logFile=null;
  static HashSet<String> blocked = new HashSet();
  static HashSet<String> unmounted = new HashSet();
  static HashMap<String,FileHash> FileHashes = new HashMap();
  public final static String versionInfo = "1.0";
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
  public static String getFileHash(String fileName)  {
    File file = new File(fileName);
    FileHash hash = FileHashes.get(fileName);
    if( hash != null )  {
      if( hash.size == file.length() && hash.timestamp == file.lastModified() )
        return hash.hash;
    }
    else
      FileHashes.put(fileName, hash = new FileHash());
    hash.size = file.length();
    hash.timestamp = file.lastModified();
    MessageDigest md = null;
    try {
      md = MessageDigest.getInstance("MD5");
      md.reset();
      FileInputStream fr = new FileInputStream(file);
      byte [] bf = new byte[1026*64];
      int read;
      while( (read=fr.read(bf)) > 0 )
        md.update(bf, 0, read);
      byte [] digest = md.digest();
      String str_digest = "";
      for( int i = 0; i < digest.length; i++ )  {
        str_digest += digestChars.charAt((digest[i]&0xf0) >> 4);
        str_digest += digestChars.charAt(digest[i]&0x0f);
      }
      return (hash.hash=str_digest);
    }
    catch(Exception ex) {
      return "";
    }
  }
  static boolean saveHashes(String fileName)  {
    if( fileName.isEmpty() )  return false;
    try  {
      File f = new File(fileName);
      PrintWriter pw = new PrintWriter(new FileOutputStream(f));
      Iterator<Entry<String,FileHash>> itr = FileHashes.entrySet().iterator();
      while( itr.hasNext() )  {
        Entry<String,FileHash> en = itr.next();
        String line = en.getKey().replaceAll("\\s", "%20");
        line += ' ';
        line += en.getValue().hash;  line += ' ';
        line += en.getValue().size;  line += ' ';
        line += en.getValue().timestamp;
        pw.println(line);
      }
      pw.close();
      return true;
    }
    catch(Exception e)  {  return false;  }
  }
  static boolean loadHashes(String fileName)  {
    if( fileName.isEmpty() )  return false;
    try  {
      File f = new File(fileName);
      BufferedReader br = new BufferedReader(new InputStreamReader(new FileInputStream(f)));
      String line;
      while( (line=br.readLine()) != null )  {
        String[] toks = line.split("\\s");
        FileHash hash = new FileHash();
        hash.hash = toks[1];
        hash.size = Long.parseLong(toks[2]);
        hash.timestamp = Long.parseLong(toks[3]);
        FileHashes.put(toks[0].replaceAll("%20", " "), hash);
      }
      br.close();
      return true;
    }
    catch(Exception e)  {  return false;  }
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
  public static void main(String[] _args) {
    print("Download manager server, (c) O. Dolomanov, 2010");
    ArrayList<String> args = new ArrayList();
    for( int i=0; i < _args.length; i++ )  {
      if( _args[i].equals("port") && i+1 < _args.length )  {
        port_number = Integer.parseInt(_args[++i]);
        continue;
      }
      args.add(_args[i]);
    }
    String status = "unknown", status_info = "none";
    ArrayList<String> st = doCall("status", "");
    if( st.size() == 2 )  {
      status = st.get(0);
      status_info = st.get(1);
    }
    if( args.isEmpty() )  {
      print("Status: " + status);
      print("Status Info: " + status_info);
      System.exit(0);
    }
    String command = args.get(0);
    if( command.equals("start") )  {
      if( status.equals("running") )  {
        print("Already running...");
        return;
      }
      if( args.size() < 2 )  {
        print("Please provide the exposed root folder");
        return;
      }
      // set up the base dir
      File ef = new File(args.get(1));
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
        for( int i=2; i < args.size(); i+=2 )  {
          final int val_i = i+1;
          if( val_i == args.size() )
            break;
          if( args.get(i).equals("log") )  {
            File log = new File(args.get(val_i));
            logFile = new PrintWriter(new FileWriter(log, true));
          }
          else if( args.get(i).equals("hashes") )  {
            hashesFileName = args.get(val_i);
          }
          else if( args.get(i).equals("blocked") )  {
            BufferedReader reader = new BufferedReader(new FileReader(args.get(val_i)));
            String line = null;
            while ((line = reader.readLine()) != null) {
              blocked.add(line);
            }
          }
        }
        loadHashes(hashesFileName);
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
        saveHashes(hashesFileName);
      }
      catch(Exception e)  {
        if( logFile != null )  {
          logFile.close();
          logFile = null;
        }
        e.printStackTrace();
      }
    }
    else if( command.equals("stop") )  {
      if( !status.equals("running") )  {
        print("Not running...");
        return;
      }
      st = doCall(args.get(0), "");
      st = doCall(args.get(0), "");  // have to call twice since Accept is blocking
      if( st.size() == 1 && st.get(0).equals("OK"))
        print("The server has been successfully stopped.");
      else
        print("Failed to stop server.");
    }
    else if( command.equals("block") && args.size() == 2 )  {
      if( !status.equals("running") )  {
        print("Not running...");
        return;
      }
      st = doCall("block", args.get(1));
      if( st.size() == 1 && st.get(0).equals("OK") )
        print("The address has been blocked for the session.");
      else
        print("Failed to block the address.");
    }
    else if( command.equals("unblock") && args.size() == 2 )  {
      if( !status.equals("running") )  {
        print("Not running...");
        return;
      }
      st = doCall(command, args.get(1));
      if( st.size() == 1 && st.get(0).equals("OK") )
        print("The address has been unblocked.");
      else
        print("Failed to unblock the address.");
    }
    else if( command.equals("mount") && args.size() == 2 )  {
      if( !status.equals("running") )  {
        print("Not running...");
        return;
      }
      st = doCall(command, args.get(1));
      if( st.size() == 1 && st.get(0).equals("OK") )
        print("Mounted: " + args.get(1));
      else
        print("Failed to mount the folder: " + args.get(1));
    }
    else if( command.equals("unmount") && args.size() == 2 )  {
      if( !status.equals("running") )  {
        print("Not running...");
        return;
      }
      st = doCall(command, args.get(1));
      if( st.size() >= 1 && st.get(0).equals("OK") )
        print("Unmounted for the session: " + args.get(1));
      else
        print("Failed to unmount the folder: " + args.get(1));
    }
  }
}
