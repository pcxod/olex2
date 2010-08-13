/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package cds;

import java.io.File;
import java.util.Comparator;

/**
 *
 * @author olegdolomanov
 */
class DirAlphaComparator implements Comparator<File> {

  public DirAlphaComparator() {}

  public int compare(File filea, File fileb) {
    if (filea.isDirectory() && !fileb.isDirectory()) {
      return -1;
    } else if (!filea.isDirectory() && fileb.isDirectory()) {
      return 1;
    } else {
      return filea.getName().compareToIgnoreCase(fileb.getName());
    }
  }
}
