
import java.awt.Graphics;
import java.awt.event.MouseEvent;
import java.awt.image.BufferedImage;

/*
 * olex2ja.java
 *
 * Created on 22 June 2008, 16:12
 */
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.util.Timer;
import java.util.TimerTask;
import javax.swing.ImageIcon;
import javax.swing.JFileChooser;
import javax.swing.JPanel;
import olex2j.GlWindow;

/**
 *
 * @author  Oleg
 */
public class olex2ja extends javax.swing.JApplet {
    GlWindow glWindow;
    BufferedImage bitmap;
//    Graphics2D graphics;
    int[] ImgBuffer;
    int iWidth, iHeight;
    JFileChooser dlgOpen;
    String FileName;
    Timer timer;
    
    public String getFileName()  {  return FileName;  }
    
    class OnListen extends TimerTask {
        olex2ja parent;
        long LastModified = 0;
        public OnListen(olex2ja parent)  {
          this.parent = parent;
        }
        @Override
        public void run() {
            if( FileName.isEmpty() || !parent.getGlWindow().isInitialised() )  return;
            File f = new File( parent.getFileName() );
            if( !f.exists() )  return;
            long lm = f.lastModified();
            if( lm != LastModified )  {
                LastModified = lm;
                parent.getGlWindow().fileChanged(parent.getGlWindow().getInstanceId(), 
                  parent.getFileName());
                parent.repaint();
            }
        }
        
    }

    /** Initializes the applet olex2ja */
    public void init() {
        try {
            java.awt.EventQueue.invokeAndWait(new Runnable() {

                public void run() {
                    initComponents();
                }
            });
        } catch (Exception ex) {
            ex.printStackTrace();
        }
        glWindow = new GlWindow();
        jiFrame.setSize(getWidth(), getHeight());
        //jiFrame.set
        iHeight = getClientHeight();
        iWidth = getClientWidth();
        bitmap = new BufferedImage(iWidth, iHeight, BufferedImage.TYPE_INT_BGR);
        ImgBuffer = new int[iWidth * iHeight];
        FileName = getParameter("filename");
        if( FileName == null )  FileName = "";
        else  {
          try  {
            URL url = new URL(FileName);
            String fn = url.getFile();
            int di = fn.lastIndexOf('.');
            if( di >= 0 )
                fn = fn.substring(di, fn.length());
            //
            InputStream is = url.openStream();
            File tmpf = File.createTempFile("olxvj", fn);
            tmpf.deleteOnExit();
            byte[] bf = new byte[1024*8];
            FileOutputStream os = new FileOutputStream(tmpf);
            int rc;
            while( (rc=is.read(bf)) != -1 )
                os.write(bf, 0, rc);
            os.close();
            FileName = tmpf.getCanonicalPath();
            jpDraw.repaint();
          }
          catch( IOException exc )  {
            lStatus.setText("Failed to load URL");
          }
          URL iurl = GlWindow.class.getResource("/olex2.jpg");
          if( iurl != null )  {
            ImageIcon icon = new ImageIcon(iurl, "Olex2 icon");
            jiFrame.setFrameIcon(icon);
          }
        }
        //timer = new Timer();
        //timer.schedule(new OnListen(this), 100);
    }
    GlWindow getGlWindow()  {  return glWindow;  }
    @Override
    public void stop()  {
        glWindow.doFinalise();
    }
    public void loadFile(String f_n)  {
        try {
            URL url = new URL(f_n);
            String fn = url.getFile();
            int di = fn.lastIndexOf('.');
            if (di >= 0) {
                fn = fn.substring(di, fn.length());
            //
            }
            InputStream is = url.openStream();
            File tmpf = File.createTempFile("olxvj", fn);
            tmpf.deleteOnExit();
            byte[] bf = new byte[1024 * 8];
            FileOutputStream os = new FileOutputStream(tmpf);
            int rc;
            while ((rc = is.read(bf)) != -1) {
                os.write(bf, 0, rc);
            }
            os.close();
            FileName = tmpf.getCanonicalPath();
            jpDraw.repaint();
        } catch (IOException exc) {
            lStatus.setText("Failed to load URL");
        }
    }
    void paintImage(Graphics g) {
        if (glWindow.isInitialised() ) {
            glWindow.paint(glWindow.getInstanceId(), ImgBuffer, iWidth, iHeight);
        }
        else  {
            lStatus.setText("Failed to load required dll");
            return;
        }
        if( !FileName.isEmpty() )  {
            glWindow.fileChanged(glWindow.getInstanceId(), FileName); 
            FileName = "";
        }
        byte[] status = glWindow.getStatus(glWindow.getInstanceId());
        if( status != null )
          lStatus.setText(new String(status));
        bitmap.setRGB(0, 0, iWidth, iHeight, ImgBuffer, 0, iWidth);
        
        g.drawImage(bitmap, 0, 0, null);
    }
    
    public int getClientHeight() {
        return jpDraw.getHeight();
    }

    public int getClientWidth() {
        return jpDraw.getWidth();
    }

    public int getY(int y) {
        return  y;
    }
    public int getX(int x) {
        return x;
    }

    /** This method is called from within the init() method to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jiFrame = new javax.swing.JInternalFrame();
        jpDraw = new JPanel() {
            public void paint(Graphics g)  {
                super.paint(g);
                paintImage(g);
            }
        };
        lStatus = new javax.swing.JLabel();
        jPanel1 = new javax.swing.JPanel();
        bOpen = new javax.swing.JButton();
        lInfo = new javax.swing.JLabel();
        jcbLabels = new javax.swing.JCheckBox();

        setName("Olex2 Applet"); // NOI18N

        jiFrame.setVisible(true);

        jpDraw.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mousePressed(java.awt.event.MouseEvent evt) {
                jpDrawMousePressed(evt);
            }
            public void mouseReleased(java.awt.event.MouseEvent evt) {
                jpDrawMouseReleased(evt);
            }
        });
        jpDraw.addMouseMotionListener(new java.awt.event.MouseMotionAdapter() {
            public void mouseDragged(java.awt.event.MouseEvent evt) {
                jpDrawMouseDragged(evt);
            }
            public void mouseMoved(java.awt.event.MouseEvent evt) {
                jpDrawMouseMoved(evt);
            }
        });

        javax.swing.GroupLayout jpDrawLayout = new javax.swing.GroupLayout(jpDraw);
        jpDraw.setLayout(jpDrawLayout);
        jpDrawLayout.setHorizontalGroup(
            jpDrawLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 367, Short.MAX_VALUE)
        );
        jpDrawLayout.setVerticalGroup(
            jpDrawLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 238, Short.MAX_VALUE)
        );

        lStatus.setText("Running...");

        bOpen.setText("Open");
        bOpen.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                bOpenActionPerformed(evt);
            }
        });

        jcbLabels.setText("Labels");
        jcbLabels.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent evt) {
                jcbLabelsItemStateChanged(evt);
            }
        });

        javax.swing.GroupLayout jPanel1Layout = new javax.swing.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(bOpen)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jcbLabels, javax.swing.GroupLayout.PREFERRED_SIZE, 79, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(lInfo, javax.swing.GroupLayout.PREFERRED_SIZE, 217, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(bOpen)
                    .addComponent(lInfo, javax.swing.GroupLayout.PREFERRED_SIZE, 19, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(jcbLabels, javax.swing.GroupLayout.PREFERRED_SIZE, 23, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(3, Short.MAX_VALUE))
        );

        javax.swing.GroupLayout jiFrameLayout = new javax.swing.GroupLayout(jiFrame.getContentPane());
        jiFrame.getContentPane().setLayout(jiFrameLayout);
        jiFrameLayout.setHorizontalGroup(
            jiFrameLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jiFrameLayout.createSequentialGroup()
                .addGroup(jiFrameLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addComponent(jPanel1, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 367, Short.MAX_VALUE)
                    .addComponent(lStatus, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 367, Short.MAX_VALUE)
                    .addComponent(jpDraw, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        jiFrameLayout.setVerticalGroup(
            jiFrameLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jiFrameLayout.createSequentialGroup()
                .addComponent(jPanel1, javax.swing.GroupLayout.PREFERRED_SIZE, 26, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jpDraw, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(lStatus, javax.swing.GroupLayout.PREFERRED_SIZE, 19, javax.swing.GroupLayout.PREFERRED_SIZE))
        );

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(jiFrame)
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(jiFrame)
        );
    }// </editor-fold>//GEN-END:initComponents

private void bOpenActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_bOpenActionPerformed
    if( !glWindow.isInitialised() )  return;
    if (dlgOpen == null) {
        dlgOpen = new JFileChooser();
    }
    if (dlgOpen.showOpenDialog(this) != JFileChooser.ERROR_OPTION) {
        glWindow.fileChanged(glWindow.getInstanceId(), dlgOpen.getSelectedFile().getAbsolutePath());
    }
}//GEN-LAST:event_bOpenActionPerformed

private void jpDrawMouseMoved(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_jpDrawMouseMoved
    if( !glWindow.isInitialised() )  return;
    short btn = 0,  shift = 0;
    if (evt.getButton() == MouseEvent.BUTTON1) {
        btn = GlWindow.olxv_MouseLeft;
    } else if (evt.getButton() == MouseEvent.BUTTON2) {
        btn = GlWindow.olxv_MouseMiddle;
    } else if (evt.getButton() == MouseEvent.BUTTON3) {
        btn = GlWindow.olxv_MouseRight;
    }
    if ((evt.getModifiers() & MouseEvent.SHIFT_MASK) != 0) {
        shift |= GlWindow.olxv_ShiftShift;
    }
    if ((evt.getModifiers() & MouseEvent.ALT_MASK) != 0) {
        shift |= GlWindow.olxv_ShiftAlt;
    }
    if ((evt.getModifiers() & MouseEvent.CTRL_MASK) != 0) {
        shift |= GlWindow.olxv_ShiftCtrl;
    }
    if( glWindow.mouseEvt(glWindow.getInstanceId(), getX(evt.getX()), getY(evt.getY()), GlWindow.olxv_MouseMove, btn, shift) )  {
        repaint();
    }
    else  {
        jpDraw.setToolTipText( 
          new String(glWindow.getObjectLabelAt(
          glWindow.getInstanceId(), getX(evt.getX()), getY(evt.getY()))));
    }
}//GEN-LAST:event_jpDrawMouseMoved

private void jpDrawMousePressed(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_jpDrawMousePressed
    if( !glWindow.isInitialised() )  return;
    short btn = 0,  shift = 0;
    if (evt.getButton() == MouseEvent.BUTTON1) {
        btn = GlWindow.olxv_MouseLeft;
    } else if (evt.getButton() == MouseEvent.BUTTON2) {
        btn = GlWindow.olxv_MouseMiddle;
    } else if (evt.getButton() == MouseEvent.BUTTON3) {
        btn = GlWindow.olxv_MouseRight;
    }
    if ((evt.getModifiers() & MouseEvent.SHIFT_MASK) != 0) {
        shift |= GlWindow.olxv_ShiftShift;
    }
    if ((evt.getModifiers() & MouseEvent.ALT_MASK) != 0) {
        shift |= GlWindow.olxv_ShiftAlt;
    }
    if ((evt.getModifiers() & MouseEvent.CTRL_MASK) != 0) {
        shift |= GlWindow.olxv_ShiftCtrl;
    }
    glWindow.mouseEvt(glWindow.getInstanceId(), getX(evt.getX()), getY(evt.getY()), 
      GlWindow.olxv_MouseDown, btn, shift);
    repaint();
}//GEN-LAST:event_jpDrawMousePressed

private void jpDrawMouseReleased(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_jpDrawMouseReleased
    if( !glWindow.isInitialised() )  return;
    short btn = 0,  shift = 0;
    if (evt.getButton() == MouseEvent.BUTTON1) {
        btn = GlWindow.olxv_MouseLeft;
    } else if (evt.getButton() == MouseEvent.BUTTON2) {
        btn = GlWindow.olxv_MouseMiddle;
    } else if (evt.getButton() == MouseEvent.BUTTON3) {
        btn = GlWindow.olxv_MouseRight;
    }
    if ((evt.getModifiers() & MouseEvent.SHIFT_MASK) != 0) {
        shift |= GlWindow.olxv_ShiftShift;
    }
    if ((evt.getModifiers() & MouseEvent.ALT_MASK) != 0) {
        shift |= GlWindow.olxv_ShiftAlt;
    }
    if ((evt.getModifiers() & MouseEvent.CTRL_MASK) != 0) {
        shift |= GlWindow.olxv_ShiftCtrl;
    }
    glWindow.mouseEvt(glWindow.getInstanceId(), getX(evt.getX()), getY(evt.getY()), 
      GlWindow.olxv_MouseUp, btn, shift);
    lInfo.setText( new String(glWindow.getSelectionInfo(glWindow.getInstanceId())) );
    repaint();
}//GEN-LAST:event_jpDrawMouseReleased

private void jpDrawMouseDragged(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_jpDrawMouseDragged
    jpDrawMouseMoved(evt);
}//GEN-LAST:event_jpDrawMouseDragged

private void jcbLabelsItemStateChanged(java.awt.event.ItemEvent evt) {//GEN-FIRST:event_jcbLabelsItemStateChanged
  if( !glWindow.isInitialised() )  return;
  glWindow.showLabels(glWindow.getInstanceId(), jcbLabels.isSelected() );
  repaint();
}//GEN-LAST:event_jcbLabelsItemStateChanged

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton bOpen;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JCheckBox jcbLabels;
    private javax.swing.JInternalFrame jiFrame;
    private javax.swing.JPanel jpDraw;
    private javax.swing.JLabel lInfo;
    private javax.swing.JLabel lStatus;
    // End of variables declaration//GEN-END:variables
}
