/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "updateapi.h"
#include "log.h"
#include "efile.h"
#include "filesystem.h"
#include "settingsfile.h"
#include "url.h"
#include "datafile.h"
#include "dataitem.h"
#include "patchapi.h"
#include "cdsfs.h"
#include "zipfs.h"

#ifdef __WXWIDGETS__
  #include "wxftpfs.h"
#endif

using namespace updater;
using namespace patcher;
//..............................................................................
UpdateAPI::UpdateAPI()
  : f_lsnr(0), p_lsnr(0), Tag(patcher::PatchAPI::ReadRepositoryTag())
{
  TStrList files;
  files << GetSettingsFileName() << GetMirrorsFileName();
  for (size_t i = 0; i < files.Count(); i++) {
    if (!TEFile::Exists(files[i])) {
      olxstr fn = TBasicApp::GetBaseDir() +
        TEFile::ExtractFileName(files[i]);
      if (TEFile::Exists(fn)) {
        TEFile::Copy(fn, files[i]);
      }
    }
  }
  settings.Init(GetSettingsFileName());
}
//..............................................................................
UpdateAPI::~UpdateAPI() {}
//..............................................................................
void UpdateAPI::InitAQueues(TActionQueue& f, TActionQueue& p) {
  if (f_lsnr != 0) {
    f.Add(f_lsnr.release());
  }
  if (p_lsnr != 0) {
    p.Add(p_lsnr.release());
  }
}
//...........................................................................
short UpdateAPI::_Update(AFileSystem& SrcFS, const TStrList& properties,
  const TFSItem::SkipOptions* toSkip)
{
  try {
    TOSFileSystem DestFS(TBasicApp::GetBaseDir()); // local file system
    TFSIndex FI(SrcFS);
    InitAQueues(SrcFS.OnProgress, FI.OnProgress);
    if (FI.Synchronise(DestFS, properties, toSkip) == 0) {
      return uapi_UptoDate;
    }
    return uapi_OK;
  }
  catch (TExceptionBase& exc) {
    TStrList out;
    exc.GetException()->GetStackTrace(out);
    log.AddAll(out);
    return uapi_UpdateError;
  }
}
//.............................................................................
short UpdateAPI::_Update(AFileSystem& src, AFileSystem& dest) {
  try {
    TFSIndex FI(src);
    InitAQueues(src.OnProgress, FI.OnProgress);
    TStrList empty;
    if (FI.Synchronise(dest, empty, 0) == 0) {
      return uapi_UptoDate;
    }
    return uapi_OK;
  }
  catch (TExceptionBase& exc) {
    TStrList out;
    exc.GetException()->GetStackTrace(out);
    log.AddAll(out);
    return uapi_UpdateError;
  }
}
//.............................................................................
short UpdateAPI::DoUpdate(olx_object_ptr < AActionHandler> _f_lsnr,
  olx_object_ptr < AActionHandler> _p_lsnr)
{
  f_lsnr = _f_lsnr;
  p_lsnr = _p_lsnr;
  if (!settings.IsValid()) {
    log.Add("Invalid settings file: ") << settings.source_file;
    return uapi_NoSettingsFile;
  }
  if (Tag.IsEmpty()) {
    return updater::uapi_InvalidInstallation;
  }
  if (!PatchAPI::LockUpdater()) {
    return updater::uapi_Busy;
  }
  SettingsFile& sf = settings;
  TFSItem::SkipOptions toSkip;
  toSkip.extsToSkip = sf.extensions_to_skip.IsEmpty() ? 0
    : &sf.extensions_to_skip;
  toSkip.filesToSkip = sf.files_to_skip.IsEmpty() ? 0 : &sf.files_to_skip;

  short res = updater::uapi_NoSource;
  olx_object_ptr<AFileSystem> srcFS = FindActiveRepositoryFS(&res);
  if (srcFS == 0) {
    return res;
  }
  srcFS->SetBase(AddTagPart(srcFS->GetBase(), false));
  // evaluate properties
  TStrList props = EvaluateProperties();
  bool skip = (toSkip.extsToSkip == 0 && toSkip.filesToSkip == 0);
  res = _Update(*srcFS, props, skip ? 0 : &toSkip);
  if (res == updater::uapi_OK) {
    sf.last_updated = TETime::EpochTime();
    sf.Save();
  }
  PatchAPI::UnlockUpdater();
  return res;
}
//.............................................................................
short UpdateAPI::DoInstall(olx_object_ptr <AActionHandler> download_lsnr,
  olx_object_ptr <AActionHandler> extract_lsnr, const olxstr& repo)
{
  f_lsnr = extract_lsnr;
  p_lsnr = download_lsnr;
  if (!IsInstallRequired()) {
    return updater::uapi_OK;
  }
  if (repo.IsEmpty()) {
    return updater::uapi_InvaildRepository;
  }
  if (!TBasicApp::GetInstance().IsBaseDirWriteable()) {
    return updater::uapi_AccessDenied;
  }
  // check for local installation
  olxstr src_fn = GetInstallationFileName(),
    inst_zip_fn = TBasicApp::GetBaseDir() + src_fn;
  if (repo == inst_zip_fn && TEFile::Exists(inst_zip_fn)) {
    try {
      if (!PatchAPI::LockUpdater()) {
        return updater::uapi_Busy;
      }
      olx_object_ptr<AZipFS> zfs(
        ZipFSFactory::GetInstance(inst_zip_fn, false));
      if (p_lsnr != 0) {
        zfs->OnProgress.Add(p_lsnr.release());
      }
      if (!zfs->Exists(patcher::PatchAPI::GetTagFileName())) {
        return updater::uapi_InvaildRepository;
      }
      zfs->ExtractAll(TBasicApp::GetBaseDir());
      settings.repository = GetDefaultRepositories()[0];
      settings.last_updated = TETime::EpochTime();
      settings.Save();
      PatchAPI::UnlockUpdater();
      return updater::uapi_OK;
    }
    catch (const TExceptionBase& exc) {
      log.Add(exc.GetException()->GetFullMessage());
      PatchAPI::UnlockUpdater();
      return updater::uapi_InstallError;
    }
  }
  if (!PatchAPI::LockUpdater()) {
    return updater::uapi_Busy;
  }
  olx_object_ptr<AFileSystem> fs = FSFromString(repo, settings.proxy);
  if (fs == 0) {
    PatchAPI::UnlockUpdater();
    return updater::uapi_NoSource;
  }
  if (f_lsnr != 0) {
    fs->OnProgress.Add(f_lsnr.release());
  }
  olx_object_ptr<IInputStream> src_s = 0;
  short res = updater::uapi_OK;
  try {
    src_s = fs->OpenFile(fs->GetBase() + src_fn);
  }
  catch (const TExceptionBase& exc) {
    log.Add(exc.GetException()->GetFullMessage());
    res = updater::uapi_InvaildRepository;
  }
  if (src_s == 0) {
    res = updater::uapi_InvaildRepository;
  }

  if (res != updater::uapi_OK) {
    PatchAPI::UnlockUpdater();
    return res;
  }
  try {
    src_fn = TBasicApp::GetBaseDir() + src_fn;
    TEFile src_f(src_fn, "w+b");
    src_f << *src_s;
    src_s = 0;
    src_f.Close();
    {  // make sure the zipfs goes before deleting the file
      olx_object_ptr<AZipFS> zfs(ZipFSFactory::GetInstance(src_fn, false));
      if (p_lsnr != 0) {
        zfs->OnProgress.Add(p_lsnr.release());
      }
      zfs->ExtractAll(TBasicApp::GetBaseDir());
    }
    fs = 0;
    TEFile::DelFile(src_fn);
    settings.repository = TrimTagPart(repo);
    settings.last_updated = TETime::EpochTime();
    settings.update_interval = "Always";
    settings.Save();
  }
  catch (const TExceptionBase& exc) {
    log.Add(exc.GetException()->GetFullMessage());
    res = updater::uapi_InstallError;
  }
  PatchAPI::UnlockUpdater();
  return res;
}
//.............................................................................
short UpdateAPI::InstallPlugin(olx_object_ptr<AActionHandler> d_lsnr,
  olx_object_ptr<AActionHandler> e_lsnr, const olxstr& name)
{
  f_lsnr = e_lsnr;
  p_lsnr = d_lsnr;
  if (!TBasicApp::GetInstance().IsBaseDirWriteable()) {
    return updater::uapi_AccessDenied;
  }
  if (Tag.IsEmpty()) {
    return updater::uapi_InvalidInstallation;
  }
  if (!PatchAPI::LockUpdater()) {
    return updater::uapi_Busy;
  }
  olx_object_ptr<AFileSystem> fs = FindActiveRepositoryFS();
  if (fs == 0) {
    PatchAPI::UnlockUpdater();
    return updater::uapi_NoSource;
  }
  if (f_lsnr != 0) {
    fs->OnProgress.Add(f_lsnr.release());
  }
  fs->SetBase(AddTagPart(fs->GetBase(), false));
  TStrList names,
    sys_tags = GetSystemTags();
  names << (TEFile::UnixPath(olxstr(fs->GetBase()) << name << '-' <<
    sys_tags.GetLastString() << ".zip"));
  names << (TEFile::UnixPath(olxstr(fs->GetBase()) << name << ".zip"));

  olxstr zip_fn;
  olx_object_ptr<IInputStream> is;
  try {
    for (size_t i = 0; i < names.Count(); i++) {
      is = fs->OpenFile(names[i]);
      if (is != 0) {
        zip_fn = names[i];
        break;
      }
    }
  }
  catch (const TExceptionBase& exc) {
    log.Add(exc.GetException()->GetFullMessage());
    PatchAPI::UnlockUpdater();
    return updater::uapi_InvaildRepository;
  }
  if (is == 0) {
    PatchAPI::UnlockUpdater();
    return updater::uapi_NoSource;
  }
  try {
    zip_fn = (TBasicApp::GetBaseDir() + name) << ".zip";
    TEFile src_f(zip_fn, "w+b");
    src_f << *is;
    is = 0;
    src_f.Close();
    {  // make sure the zipfs goes before deleting the file
      olx_object_ptr<AZipFS> zfs(ZipFSFactory::GetInstance(zip_fn, false));
      TFSIndex fsi(zfs);
      TOSFileSystem osf(TBasicApp::GetBaseDir());
      osf.RemoveAccessRight(afs_DeleteAccess);
      TStrList props = GetPluginProperties(name);
      if (p_lsnr != 0) {
        fsi.OnProgress.Add(p_lsnr.release());
      }
      fsi.Synchronise(osf, props);
    }
    TEFile::DelFile(zip_fn);
    fs = 0;
    PatchAPI::UnlockUpdater();
  }
  catch (const TExceptionBase& exc) {
    log.Add(exc.GetException()->GetFullMessage());
    PatchAPI::UnlockUpdater();
    return updater::uapi_InstallError;
  }
  PatchAPI::UnlockUpdater();
  return updater::uapi_OK;
}
//.............................................................................
short UpdateAPI::DoSynch(olx_object_ptr<AActionHandler> _f_lsnr,
  olx_object_ptr<AActionHandler> _p_lsnr)
{
  f_lsnr = _f_lsnr;
  p_lsnr = _p_lsnr;
  if (!settings.IsValid()) {
    log.Add("Invalid settings file: ") << settings.source_file;
    return uapi_NoSettingsFile;
  }
  const SettingsFile& sf = settings;
  if (sf.dest_repository.IsEmpty() || sf.src_for_dest.IsEmpty())
    return updater::uapi_NoTask;
  olx_object_ptr<AFileSystem> srcFS;
  if (sf.src_for_dest.Equalsi("local")) {
    srcFS = new TOSFileSystem(TBasicApp::GetBaseDir());
  }
  else if (sf.src_for_dest.Equalsi("remote")) {
    srcFS = FSFromString(sf.repository, sf.proxy);
  }

  if (srcFS == 0) {
    log.Add("Could not locate source for synchronisation");
    return updater::uapi_NoSource;
  }
  olx_object_ptr<AFileSystem> destFS = FSFromString(sf.dest_repository, sf.proxy);
  if (destFS == 0) {
    return updater::uapi_NoDestination;
  }
  return _Update(*srcFS, *destFS);
}
//.............................................................................
const_strlist UpdateAPI::GetSystemTags() {
  TStrList res;
  olxstr os_str;
#if defined(__WIN32__)
  os_str = "win";
#elif defined(__MAC__)
  os_str = "mac";
#elif defined(__linux__)
  os_str = "linux";
#else
  os_str = "unknown";
#endif
  res << os_str << os_str;
  if (TBasicApp::Is64BitCompilation()) {
    res.GetLastString() << "-64";
  }
  else {
    res.GetLastString() << "-32";
  }
  return res;
}
//.............................................................................
const_strlist UpdateAPI::GetPluginProperties(const olxstr &p) {
  TStrList props,
    sys_tags = GetSystemTags();
  props.Add("plugin-") << p;
  for (size_t i = 0; i < sys_tags.Count(); i++) {
    props.Add("plugin-") << p << '-' << sys_tags[i];
  }
  return props;
}
//.............................................................................
TStrList::const_list_type UpdateAPI::EvaluateProperties() const {
  olxstr platform, py_ver;
#if defined(__WIN32__)
#  if defined(_DEBUG)
#    if !defined(_WIN64)
  platform = "win32";
#    else
  platform = "win64";
#    endif
#  elif _WIN64
  platform = "win64";
#  else
#    if _M_IX86_FP == 0
  platform = "win32-nosse";
#    elif _M_IX86_FP == 1
  platform = "win32-sse";
  // cannot change it! olex2 does not get updated and this is it...
#    elif _M_IX86_FP == 2
  platform = "win32";
#    endif
#  endif
#elif __MAC__
#  if defined(__LP64__) || defined(__x86_64__)
  platform = "mac64";
#  else
  platform = "mac32";
#  endif
#elif __linux__
#  if defined(__LP64__) || defined(__x86_64__)
  platform = "lin64";
#  else
  platform = "win32";
#  endif
#endif

#ifdef _PYTHON
  py_ver << "py" << PY_MAJOR_VERSION << PY_MINOR_VERSION;
#endif

  TStrList props;
  props.Add("olex-update");
  if (!py_ver.IsEmpty()) {
    props.Add(platform) << '-' << py_ver;
  }

  if (!settings.olex2_port.IsEmpty()) {
    props.Add(settings.olex2_port);
    log.Add("Portable executable update tag: ") << settings.olex2_port;
  }
  olxstr pluginFile = TBasicApp::GetBaseDir() + "plugins.xld";
  if (TEFile::Exists(pluginFile)) {
    try {
      TDataFile df;
      df.LoadFromXLFile(pluginFile, 0);
      TDataItem* PluginItem = df.Root().FindItem("Plugin");
      if (PluginItem != 0) {
        for (size_t i = 0; i < PluginItem->ItemCount(); i++) {
          props.AddAll(GetPluginProperties(
            PluginItem->GetItemByIndex(i).GetName()));
        }
      }
    }
    catch (...) {}  // unlucky
  }
  return props;
}
//.............................................................................
olx_object_ptr<AFileSystem> UpdateAPI::FSFromString(const olxstr& _repo,
  const olxstr& _proxy)
{
  if (_repo.IsEmpty()) {
    return 0;
  }
  try {
    AFileSystem *FS = 0;
    olxstr repo = _repo;
    if (TEFile::Exists(repo)) {
      if (TEFile::ExtractFileExt(repo).Equalsi("zip")) {
        if (!TEFile::IsAbsolutePath(repo)) {
          repo = TBasicApp::GetBaseDir() + repo;
        }
        FS = ZipFSFactory::GetInstance(repo, false);
      }
      else if (TEFile::IsDir(repo)) {
        FS = new TOSFileSystem(repo);
      }
    }
    else {
      TUrl url(_repo);
      if (!_proxy.IsEmpty()) {
        url.SetProxy(_proxy);
      }
      if (url.GetProtocol() == "http") {
        TSocketFS* _fs = new TSocketFS(url);
        _fs->SetExtraHeaders(httpHeaderPlatform);
        olxstr tfn = TBasicApp::GetSharedDir() + "app.token";
        if (TEFile::Exists(tfn)) {
          TCStrList sl = TEFile::ReadCLines(tfn);
          if (sl.Count() == 1) {
            _fs->SetSessionInfo(sl[0]);
            _fs->SetExtraHeaders(httpHeaderPlatform | httpHeaderESession);
          }
        }
        FS = _fs;
      }
      else if (url.GetProtocol() == "https") {
#ifdef __WIN32__
        TWinHttpFileSystem* _fs = new TWinHttpFileSystem(url);
#else
        TSSLHttpFileSystem* _fs = new TSSLHttpFileSystem(url);
#endif
        _fs->SetExtraHeaders(httpHeaderPlatform);
        olxstr tfn = TBasicApp::GetSharedDir() + "app.token";
        if (TEFile::Exists(tfn)) {
          TCStrList sl = TEFile::ReadCLines(tfn);
          if (sl.Count() == 1) {
            THttpFileSystem::SetSessionInfo(sl[0]);
            _fs->SetExtraHeaders(httpHeaderPlatform | httpHeaderESession);
          }
        }
        FS = _fs;
    }
#ifdef __WXWIDGETS__
      else if (url.GetProtocol() == "ftp") {
        FS = new TwxFtpFileSystem(url);
      }
#endif
    }
    return FS;
  }
  catch (const TExceptionBase&) {
    return 0;
  }
}
//.............................................................................
bool UpdateAPI::WillUpdate(bool force) const {
  bool update = force ? true : false;
  if (!force) {
    const SettingsFile& sf = settings;
    if (sf.update_interval.IsEmpty() || sf.update_interval.Equalsi("Always")) {
      update = true;
    }
    else if (sf.update_interval.Equalsi("Daily")) {
      update = ((TETime::EpochTime() - sf.last_updated) > SecsADay);
    }
    else if (sf.update_interval.Equalsi("Weekly")) {
      update = ((TETime::EpochTime() - sf.last_updated) > SecsADay * 7);
    }
    else if (sf.update_interval.Equalsi("Monthly")) {
      update = ((TETime::EpochTime() - sf.last_updated) > SecsADay * 30);
    }
  }
  return update;
}
//.............................................................................
olx_object_ptr<AFileSystem> UpdateAPI::FindActiveRepositoryFS(short* res,
  bool force, bool update) const
{
  if (Tag.IsEmpty()) {
    if (res != 0) {
      *res = updater::uapi_InvalidInstallation;
    }
    return 0;
  }
  if (res != 0) {
    *res = updater::uapi_UptoDate;
  }
  olxstr repo = settings.repository;
  if (TEFile::Exists(repo)) {
    if (TEFile::ExtractFileExt(repo).Equalsi("zip")) {
      if (TEFile::FileAge(repo) > settings.last_updated) {
        return ZipFSFactory::GetInstance(repo, false);
      }
    }
    else if (TEFile::IsDir(repo)) {
      return new TOSFileSystem(AddTagPart(repo, true));
    }
  }
  else {
    if (WillUpdate(force)) {
      olx_object_ptr<AFileSystem> FS = FindActiveRepositoryFS(&repo,
        (AddTagPart(EmptyString(), true)+"index.ind").SubStringFrom(1));
      if (!FS.ok()) {
        if (res != 0) {
          *res = updater::uapi_NoSource;
        }
        return 0;
      }
      FS->SetBase(AddTagPart(FS->GetBase(), update));
      return FS;
    }
  }
  if (res != 0) {
    *res = updater::uapi_UptoDate;
  }
  return 0;
}
//.............................................................................
olx_object_ptr<AFileSystem> UpdateAPI::FindActiveRepositoryFS(olxstr* repo_name,
  const olxstr& check_file) const
{
  TStrList repositories;
  GetAvailableMirrors(repositories);
  for (size_t i = 0; i < repositories.Count(); i++) {
    olx_object_ptr<AFileSystem> fs = FSFromString(repositories[i], settings.proxy);
    if (fs != 0) {
#ifdef _DEBUG
      if (!check_file.IsEmpty()) {
        TBasicApp::NewLogEntry() << "Checking repository: " <<
          repositories[i] << " for file: " << check_file;
      }
#endif
      if (!check_file.IsEmpty() &&
        !fs->Exists(fs->GetBase() + check_file, true))
      {
        continue;
      }
      if (repo_name != 0) {
        *repo_name = repositories[i];
      }
      return fs;
    }
  }
  return 0;
}
//.............................................................................
void UpdateAPI::GetAvailableMirrors(TStrList& res) const {
  olxstr mirrors_fn = GetMirrorsFileName();
  if (TEFile::Exists(mirrors_fn)) {
    TEFile::ReadLines(mirrors_fn, res);
  }
  TStrList defs = GetDefaultRepositories();
  for (size_t i = 0; i < defs.Count(); i++) {
    if (res.IndexOf(defs[defs.Count() - i - 1]) == InvalidIndex) {
      res.Insert(0, defs[defs.Count() - i - 1]);
    }
  }
  if (settings.IsValid() && !settings.repository.IsEmpty()) {
    const size_t ind = res.IndexOf(settings.repository);
    if (ind != InvalidIndex && ind != 0) {
      res.Delete(ind);
    }
    if (ind != 0) {
      res.Insert(0, settings.repository);
    }
  }
}
//.............................................................................
void UpdateAPI::GetAvailableRepositories(TStrList& res) const {
  olxstr repo_name,
    inst_zip_fn = TBasicApp::GetBaseDir() + GetInstallationFileName();
  if (TEFile::Exists(inst_zip_fn)) {
    res.Add(inst_zip_fn);
  }
  olx_object_ptr <AFileSystem> fs = FindActiveRepositoryFS(&repo_name, GetTagsFileName());
  if (fs == 0) {
    return;
  }
  olx_object_ptr<IInputStream> is;
  try {
    is = fs->OpenFile(fs->GetBase() + GetTagsFileName());
  }
  catch (const TExceptionBase& exc) {
    log.Add(exc.GetException()->GetFullMessage());
    return;
  }
  if (is == 0) {
    return;
  }
  res.LoadFromTextStream(*is);
  for (size_t i = 0; i < res.Count(); i++) {
    res[i] = repo_name + res[i];
  }
  // LoadFromTextStream clears the list...
  if (TEFile::Exists(inst_zip_fn)) {
    res.Insert(0, inst_zip_fn);
  }
}
//.............................................................................
void UpdateAPI::GetAvailableTags(TStrList& res, olxstr& repo_name) const {
  olx_object_ptr<AFileSystem> fs = FindActiveRepositoryFS(&repo_name, GetTagsFileName());
  if (fs == 0) {
    return;
  }
  olx_object_ptr<IInputStream> is;
  try {
    is = fs->OpenFile(fs->GetBase() + GetTagsFileName());
  }
  catch (const TExceptionBase& exc) {
    log.Add(exc.GetException()->GetFullMessage());
    return;
  }
  if (is == 0) {
    return;
  }
  res.LoadFromTextStream(*is);
}
//.............................................................................
olxstr UpdateAPI::TrimTagPart(const olxstr& path) const {
  if (Tag.IsEmpty()) {
    return path;
  }
  olxstr rv = TEFile::UnixPath(path);
  if (!rv.EndsWith('/')) {
    rv << '/';
  }
  if (rv.EndsWith("update/")) {
    rv.SetLength(rv.Length() - 7);
  }
  if (rv.EndsWith(Tag + '/')) {
    rv.SetLength(rv.Length() - Tag.Length() - 1);
  }
  return rv;
}
//.............................................................................
olxstr UpdateAPI::AddTagPart(const olxstr& path, bool Update) const {
  if (Tag.IsEmpty()) {
    return path;
  }
  olxstr rv = TrimTagPart(path);
  rv << Tag << '/';
  if (Update) {
    rv << "update/";
  }
  return rv;
}
//.............................................................................
const TStrList& UpdateAPI::GetDefaultRepositories() {
  static TStrList rv;
  if (rv.IsEmpty()) {
    rv.Add("https://secure.olex2.org/olex2-distro/");
    rv.Add("http://www.olex2.org/olex2-distro/");
    rv.Add("http://www2.olex2.org/olex2-distro/");
  }
  return rv;
}
//.............................................................................
//http://www.jorgon.freeserve.co.uk/TestbugHelp/XMMfpins2.htm
olxstr UpdateAPI::GetInstallationFileName() {
#if defined(__WIN32__) && !defined(__GNUC__)
#ifndef _WIN64
  try {
    if (IsWow64()) {
      return "olex2-win64.zip";
    }
  }
  catch (...) {}  // stay quiet (?)
  unsigned int cpu_features = 0;
  _asm {
    push EAX
    push EBX
    push ECX
    push EDX
    mov EAX, 1
    cpuid
    mov[cpu_features], EDX
    pop EDX
    pop ECX
    pop EBX
    pop EAX
  }
  bool has_sse2 = (cpu_features & (0x1 << 26)) != 0;
  bool has_sse = (cpu_features & (0x1 << 25)) != 0;
  if (has_sse2) {
    return "olex2-win32.zip";
  }
  else if (has_sse) {
    return "olex2-win32-sse.zip";
  }
  else {
    return "olex2-win32-nosse.zip";
}
#else
  return "olex2-win64.zip";
#endif
#else
  return "portable-gui.zip";
#endif
}
