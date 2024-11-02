#pragma once
#include "log.h"
#include "utf8file.h"

namespace esdl { namespace log {
  enum {
    EVT_INFO,
    EVT_WARNING,
    EVT_ERROR,
    EVT_EXCEPTION,
    EVT_POST,
  };

  class FileLog : public AEventsDispatcher {
    class LogFile : public TUtf8File {
    public:
      LogFile(const olxstr& fn) :
        TUtf8File(fn, "wb", false)
      {}
      virtual size_t Write(const void* Data, size_t size) {
        size_t r = TUtf8File::Write(Data, size);
        TUtf8File::Flush();
        return r;
      }
    };
    olx_object_ptr<LogFile> out;
    olxstr last_marker;
  public:
    FileLog(const olxstr& file_name)
      : out(new LogFile(file_name))
    {}

    void Register(TLog& l) {
      short flags = msiEnter | msiExit;
      l.OnInfo.Add(this, EVT_INFO, flags);
      l.OnWarning.Add(this, EVT_WARNING, flags);
      l.OnError.Add(this, EVT_ERROR, flags);
      l.OnException.Add(this, EVT_EXCEPTION, flags);
      l.OnPost.Add(this, EVT_POST, flags);
      l.AddStream(&out, false);
    }

    void Unregister(TLog& l) {
      l.OnInfo.Remove(this);
      l.OnWarning.Remove(this);
      l.OnError.Remove(this);
      l.OnException.Remove(this);
      l.OnPost.Remove(this);
      l.RemoveStream(&out);
    }

    bool Dispatch(int MsgId, short MsgSubId, const IOlxObject* Sender,
      const IOlxObject* Data, TActionQueue*)
    {
      if (MsgSubId == msiEnter) {
        olxstr marker;
        switch (MsgId) {
        case EVT_INFO:
          marker = "info";
          break;
        case EVT_WARNING:
          marker = "warning";
          break;
        case EVT_ERROR:
          marker = "error";
          break;
        case EVT_EXCEPTION:
          marker = "exception";
          break;
        default:
          marker = "post";
        }
        last_marker = marker;
        out->Writeln(olxstr(">>>") << last_marker << "<<<");
      }
      else if (MsgSubId == msiExit) {
        out->Writeln(olxstr("<<<") << last_marker << ">>>");
        out->Flush();
      }
      return false;
    }

  };

}} // esdl::log