//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#ifndef actionsH
#define actionsH
#include "ebase.h"
#include "estlist.h"
#include "tptrlist.h"
#include "typelist.h"

#undef GetObject
//---------------------------------------------------------------------------
BeginEsdlNamespace()

const short msiEnter      = 0x0001,
            msiExecute    = 0x0002,
            msiExit       = 0x0004,
            msiUpdateData = 0x0008,
            msiAll        = 0x000F;
class AActionHandler;
class AEventsDispatcher;
class TActionQueue;
class TActionQList;

struct TDispatchInfo  {
  int MsgId;
  short MsgSubId;
  AEventsDispatcher *Dispatcher;
  TDispatchInfo(int msgid, int subid, AEventsDispatcher *dispatcher)  {
    MsgId = msgid;
    MsgSubId = subid;
    Dispatcher = dispatcher;
  }
};
// abstract class;  an item of the actionqueue
class AActionHandler : public ACollectionItem  {
  bool  Enabled, // not executed if false
        ToDelete;  // default is true - the object will be deleted by the collection!!
public:
  AActionHandler();
  virtual ~AActionHandler();
  // handler before the event
  virtual bool Enter(const IEObject *Sender, const IEObject *Data=NULL) {  return false; }
  // handler after the event
  virtual bool Exit(const IEObject *Sender, const IEObject *Data=NULL)  {  return false; }
  /* handler during the event, does some operation when Obj is changed;
     the action depends on the type of queue
  */
  virtual void UpdateData(const IEObject *Sender, const IEObject *Data=NULL) {  ;  }
  virtual bool Execute(const IEObject *Sender, const IEObject *Data=NULL) {  return false; }
  // the function is called the object is removed from the queue
  virtual void OnRemove()  { ;  }
  DefPropB(Enabled)
  /* beware that it is set to true by default, which means that the object will
    be automatically deleted by the parent queue
  */
  DefPropB(ToDelete)
};
/*
 AEventsDispatcher allows to do events/actions hanling within one loop
 (AActionHandler is a single object ususally created  for one event)
 simply derive your class from this one and write the Dispartch method
 Register it in an ActionQueue object (see below)
 */
class AEventsDispatcher: public ACollectionItem  {
  bool Enabled;
public:
  AEventsDispatcher();
  virtual ~AEventsDispatcher();
  /* dispatch is called all time an event occures, MsgId is the Id passed when the
     object is registered in a queue, MsgSubId is one msiEnter, msiExecute, msiExit or
     msiUpdateData (see the discription of cirtain events). Sender is the object sending the message,
     Data -  is the object being changed or some data object
  */
  virtual bool Dispatch(int MsgId, short MsgSubId, const IEObject *Sender, const IEObject *Data=NULL)=0;
  DefPropB(Enabled)
};
class TActionQueue: public AActionHandler  {
  TPtrList<AActionHandler> Handlers;
  TTypeList<TDispatchInfo> Dispatchers;
  olxstr FName;
  TActionQList *FParent;
  bool FExecuteOnce; // set to true to stop of the first successful handler, false by default
public:
  TActionQueue(TActionQList *Parent, const olxstr & Name);
  virtual ~TActionQueue();

  inline int HandlerCount()             const {  return Handlers.Count(); };
  inline AActionHandler* Handler(int i)       {  return Handlers[i]; }

  inline int DispatcherCount()          const {  return Dispatchers.Count(); };
  inline AEventsDispatcher* Dispatcher(int i) {  return Dispatchers[i].Dispatcher; }
  // adds new handler
  void Add( AActionHandler* handler );
  /*
   inserts a new handler at the beginning of the list
   so that it will be executed first of all
  */
  void AddFirst(AActionHandler* handler);
  // adds new dispatcher
  void Add(AEventsDispatcher *dispatcher, int MsgId, short MsgSubId = msiAll);

  //makes subsequent calls to the handlers stored in the list
  bool Execute(const IEObject *Sender, const IEObject *Data=NULL);
  bool Enter(const IEObject *Sender, const IEObject *Data=NULL);
  bool Exit(const IEObject *Sender, const IEObject *Data=NULL);
  void UpdateData(const IEObject *Sender, const IEObject *Data);
  virtual void OnRemove()  {  return;  }
  // empties the queue
  void Clear();
  // removes specified handler from the queue
  void Remove(AActionHandler* handler);
  bool Contains(AActionHandler* handler);

  // removes specified dispatcher from the queue
  void Remove(AEventsDispatcher* dispatcher);
  // returns true if a specified handler belongs to the queue
  bool Contains(AEventsDispatcher* dispatcher);
  // returns the name of the queue
  inline const olxstr& Name() const  {  return FName;  }
};

class TActionQList: public IEObject  {
private:
  TCSTypeList<olxstr,TActionQueue*> Queues;
public:
  TActionQList()  {  }
  virtual ~TActionQList() {  Clear();  }
  /* is used to create a new named actions queue
    throws an exception if the queue already exists: use Exists to check
  */
  TActionQueue& NewQueue(const olxstr &Name);

  // executes a named queue
  bool Execute(const olxstr &Name, const IEObject *Sender, const IEObject *Data=NULL);

  inline bool QueueExists(const olxstr &Name)  const {  return Queues.IndexOfComparable(Name) != -1;  }
  /* throws exception if the queue does not exist */
  inline TActionQueue* FindQueue(const olxstr &Name)  const {  return Queues[Name];  }
  // queue by index
  inline TActionQueue& Queue(int index)                 {  return *Queues.Object(index);  }
  inline const TActionQueue& GetQueue(int index)  const {  return *Queues.GetObject(index);  }
  // returns the number of queues
  inline int Count()                           const {  return Queues.Count();  }
  // empties the list
  void Clear();
};

class TOnProgress: public IEObject {  // generic on progress data
  double Max, Pos;
  olxstr Action;
public:
  TOnProgress()  {  Max = Pos = 0; }
  virtual ~TOnProgress()  {  ;  }
  // set Max to a valid number before OnProgress is executed!
  DefPropP(double, Max)
  DefPropP(double, Pos)
  inline void IncPos(double v)   {  Pos += v;  }
  DefPropC(olxstr, Action)
};

EndEsdlNamespace()
#endif
