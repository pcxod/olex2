/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package cds;

import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.net.Socket;
import java.net.URL;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.UUID;

/**
 *
 * @author Oleg
 */
public class ClientHandler extends Thread {
  Socket client;
  static HashMap<String,String> fileMapping = new HashMap();
  static HashSet<String> TextExts = new HashSet<String>();
  public ClientHandler(Socket _client)  {
    client = _client;
    if( TextExts.isEmpty() )  {
      String[] exts = "txt py java cpp h html htm xml xld osp ind log".split("\\s");
      for( int i=0; i < exts.length; i++ )
        TextExts.add(exts[i]);
    }
    if( fileMapping.isEmpty() )  {
      fileMapping.put("olex2.zip", "Windows 32bit distribution");
      fileMapping.put("olex2-x64.zip", "Windows 64bit distribution");
      fileMapping.put("suse101x32-py26.zip", "Linux 32bit distribution");
      fileMapping.put("suse101x64-py26.zip", "Linux 64bit distribution");
      fileMapping.put("mac-intel-py26.zip", "Mac OS 32bit distribution");
      fileMapping.put("olex2-sse.zip", "Windows 32bit distribution for PIII");
    }
  }
  String getContentType(File f)  {
    int di = f.getName().indexOf(".");
    String ext = di == -1 ? "" : f.getName().substring(di+1);
    if( TextExts.contains(ext) )
      return "text/plain; charset=iso-8859-1";
    else
      return "application/octet-stream";
  }
  @Override
  public void run()  {
    try {
      BufferedReader in = new BufferedReader(new InputStreamReader(client.getInputStream()));
      DataOutputStream out = new DataOutputStream(client.getOutputStream());  // true - autoflush
      ArrayList<String> cmds = new ArrayList();
      String cmd, origin=null, platform = null;
      while( (cmd = in.readLine()) != null )  {
        if( cmd.length() == 0 )
          break;
        cmds.add(cmd);
        if( cmd.startsWith("X-Forwarded-For:") )
          origin = cmd.substring(16);
        else if( cmd.startsWith("Platform:") )
          platform = cmd.substring(10);
      }
      cmd = (cmds.isEmpty() ? null : cmds.get(0));
      if( cmd != null )  {
        String src = (origin == null ? client.getRemoteSocketAddress().toString() : origin).trim();
        if( !Main.shouldHandle(src) )  {
          String info_line = "Blocking ";
          info_line += src;
          info_line += (" at " + (new SimpleDateFormat("yyyy.MM.dd HH:mm:ss")).format(new Date()));
          info_line += (": " + cmd);
          if( platform != null )
            info_line += (" on " + platform);
          Main.print(info_line);
          out.writeBytes("HTTP/1.0 404 ERROR\n");
        }
        else  {
          String info_line = "Handling ";
          info_line += src;
          info_line += (" at " + (new SimpleDateFormat("yyyy.MM.dd HH:mm:ss")).format(new Date()));
          info_line += (": " + cmd);
          if (platform != null) {
            info_line += (" on " + platform);
          }
          Main.print(info_line);
          boolean handled = false;
          // accept some command only from localhost...
          if (client.getInetAddress().isLoopbackAddress() && origin == null) {
            if (cmd.equals("status")) {
              out.writeBytes("running");
              handled = true;
            } else if (cmd.equals("stop")) {
              Main.doTerminate();
              handled = true;
            }
          }
          if (!handled) {
            String[] toks = cmd.split("\\s");
            int offset = 0;
            if (toks.length > 1) {  // normalise file name
              int off_ind = toks[1].indexOf('#');
              if (off_ind != -1) {
                offset = Integer.parseInt(toks[1].substring(off_ind + 1));
                toks[1] = toks[1].substring(0, off_ind);
              }
              if (!toks[1].startsWith("/")) {
                URL url = new URL(toks[1]);
                toks[1] = url.getPath();
              }
              // make sure that the request does not end up outside the baseDir
              toks[1] = toks[1].replace("..", "").replace("%20", " ");
              if (toks[1].startsWith("/")) {
                toks[1] = toks[1].substring(1);
              }
            }
            if (toks[0].equals("GET")) {
              if (toks.length == 3) {
                String fn = Main.getBaseDir() + toks[1];
                File file = new File(fn);
                if (file.isDirectory()) {
                  listFolder(out, file);
                  out.close();
                  in.close();
                  client.close();
                  return;
                }
                if (!file.exists() || !file.isFile() || offset >= file.length()) {
                  out.writeBytes("HTTP/1.0 404 ERROR\n");
                } else {
                  out.writeBytes("HTTP/1.0 200 OK\n");
                  out.writeBytes("Server: Olex2-CDS\n");
                  FileInputStream fr = new FileInputStream(file);
                  fr.skip(offset);
                  out.writeBytes(("Content-Length: " + file.length()) + "\n");
                  out.writeBytes("Last-Modified: " + (new SimpleDateFormat()).format(new Date(file.lastModified())) + "\n");
                  out.writeBytes("ETag: \"" + UUID.randomUUID().toString() + "\"\n");
                  out.writeBytes("Connection: close\n");
                  out.writeBytes("Content-Type: " + getContentType(file) + "\n\n");
                  final int bf_len = 1024 * 64;
                  byte[] bf = new byte[bf_len];
                  try {
                    int read_len;
                    while ((read_len = fr.read(bf)) > 0) {
                      out.write(bf, 0, read_len);
                    }
                  } catch (Exception e) {
                    Main.print("Connection broken...");
                  }
                  fr.close();
                }
              }
            } else if (toks[0].equals("HEAD")) {
              if (toks.length == 3) {
                final String fn = Main.getBaseDir() + toks[1];
                File file = new File(fn);
                if (file.exists()) {
                  out.writeBytes("HTTP/1.0 200 OK\n");
                  out.writeBytes(("Content-Length: " + file.length()) + "\n");
                  out.writeBytes("Last-Modified: " + (new SimpleDateFormat()).format(new Date(file.lastModified())) + "\n");
                } else {
                  out.writeBytes("HTTP/1.0 404 ERROR\n");
                }
                out.writeBytes("Server: Olex2-CDS\n");
              }
            }
          }
        }
      }
      out.close();
      in.close();
      client.close();
    }
    catch (IOException ex) {
      ex.printStackTrace();
    }
    finally  {
      Main.onThreadTerminate();
    }
  }
  protected void listFolder(DataOutputStream out, File file) throws IOException  {
    File[] files = file.listFiles();
    Arrays.sort(files, new DirAlphaComparator());
    out.writeBytes("HTTP/1.0 200 OK\n");
    out.writeBytes("Server: Olex2-CDS\n");
    StringWriter _strw;
    PrintWriter pr = new PrintWriter(_strw = new StringWriter());
    pr.println("<html><head><title>CDS</title></head><body><table>");
    pr.println("<tr><td>Name</td><td align='center'>Description</td><td>Last Modified</td><td>Size</td></tr>");
    for( int i=0; i < files.length; i++ )  {
      String name = files[i].isDirectory() ? files[i].getName().toUpperCase() :
        files[i].getName().toLowerCase();
      String mapping = fileMapping.get(files[i].getName());
      if( mapping == null )
          mapping = ".";
      pr.println("<tr><td><a href=\"" +
        files[i].getName() + 
        (files[i].isDirectory() ? "/" : "") + "\">" +
        name + "</a></td><td>");
      pr.print(mapping + "</td><td>");
      pr.print((new java.text.SimpleDateFormat()).format(new Date(files[i].lastModified())) + "</td><td>");
      if( files[i].isFile() )
        pr.print(files[i].length());
      else
        pr.print(".");
      pr.print("</td></tr>");
    }
    pr.println("</table>");
    pr.println("<hr><p><font size=-2>Continuous Download Server, (c) O. Dolomanov, 2010</font></p>" +
            "</body></html>");
    String content = _strw.toString();
    out.writeBytes("Content-Length: " + content.length());
    out.writeBytes("\nConnection: close\n");
    out.writeBytes("Content-Type: text/html; charset=iso-8859-1\n\n" + content);
  }
}
