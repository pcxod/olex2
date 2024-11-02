#pragma once
#include "bapp.h"
#include "olxth.h"

namespace olex2 {
  struct RefinementListener {
    static bool& Continue();

    static void DoBreak() {
      Continue() = false;
    }

    static bool OnProgress(size_t max, size_t pos);
    static void ResetInstance() {
      RefinementListener*& i = GetInstance();
      if (i != 0) {
        delete i;
        i = 0;
      }
    }
  private:
    RefinementListener();
    static RefinementListener*& GetInstance() {
      static RefinementListener* i = 0;
      return i;
    }
    olxstr fin_fn;
    static olx_critical_section& get_critical_section() {
      static olx_critical_section cs_;
      return cs_;
    }
  };
}