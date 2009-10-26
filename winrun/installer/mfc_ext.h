#pragma once

namespace mfc_ext  {
  struct wnd  {
    static olxstr get_text(CWnd *window, int id)  {
      CWnd *item = window->GetDlgItem(id);
      int tl = (int)item->SendMessage(WM_GETTEXTLENGTH);
      olxch * bf = new olxch[tl+1];
      item->SendMessage(WM_GETTEXT, tl+1, (LPARAM)bf);
      return olxstr::FromExternal(bf, tl);
    }
    static void set_text(CWnd * window, int id, const olxstr &val)  {
      window->GetDlgItem(id)->SendMessage(WM_SETTEXT, 0, (LPARAM)val.u_str());
    }
    static void set_text(CWnd * window, int id, const olxch *val)  {
      window->GetDlgItem(id)->SendMessage(WM_SETTEXT, 0, (LPARAM)val);
    }
    static void set_enabled(CWnd * window, int id, bool v)  {
      window->GetDlgItem(id)->EnableWindow(v);
    }
    static void set_visible(CWnd * window, int id, bool v)  {
      window->GetDlgItem(id)->ShowWindow(v ? SW_SHOW : SW_HIDE);
    }
  };
  struct progress_bar  {
    static int get_pos(CWnd *window, int id)  {
      return (int)window->GetDlgItem(id)->SendMessage(PBM_GETPOS);
    }
    static void set_pos(CWnd *window, int id, int pos)  {
      window->GetDlgItem(id)->SendMessage(PBM_SETPOS, (WPARAM)pos, 0);
    }
    static void set_range(CWnd *window, int id, int min, int max)  {
      window->GetDlgItem(id)->SendMessage(PBM_SETRANGE, 0, MAKELPARAM(min, max));
    }
  };
  struct check_box : public wnd {
    static bool is_checked(CWnd *window, int id)  {
      return window->GetDlgItem(id)->SendMessage(BM_GETCHECK) == BST_CHECKED;
    }
    static void set_checked(CWnd *window, int id, bool v)  {
      window->GetDlgItem(id)->SendMessage(BM_SETCHECK, (WPARAM)( v? BST_CHECKED : BST_UNCHECKED), 0);
    }
  };
  struct combo_box : public wnd {
    static void clear_items(CWnd *window, int id)  {
      CWnd *cb = window->GetDlgItem(id);
      int cnt = (int)cb->SendMessage(CB_GETCOUNT);
      while( cnt != 0 )
        cnt = (int)cb->SendMessage(CB_DELETESTRING, (WPARAM)0);
    }
    static void add_item(CWnd *window, int id, const olxstr &item)  {
      window->GetDlgItem(id)->SendMessage(CB_ADDSTRING, 0, (LPARAM)item.u_str());
    }
    static void add_items(CWnd *window, int id, const TStrList &items)  {
      CWnd *cb = window->GetDlgItem(id);
      for( int i=0; i < items.Count(); i++ )
        cb->SendMessage(CB_ADDSTRING, 0, (LPARAM)items[i].u_str());
    }
  };
};