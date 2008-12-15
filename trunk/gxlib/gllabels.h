#ifndef gllabelsH
#define gllabelsH
#include "gxbase.h"

#include "glmouselistener.h"
#include "glfont.h"

#include "glmaterial.h"

// I am really tired of this bshyt
#undef AddAtom

BeginGxlNamespace()

// label modes
const short lmLabels   = 0x0001,  // atom label
            lmPart     = 0x0002,  // part
            lmAfix     = 0x0004,  // afix
            lmOVar     = 0x0008,  // occupancy variable
            lmOccp     = 0x0010,  // occupancy
            lmUiso     = 0x0020,  // Uiso
            lmUisR     = 0x0040,  // Uiso for riding atoms (negative)
            lmAOcc     = 0x0080,  // actuall occupancy (as read from ins )
            lmHydr     = 0x0100,  // include hydrogens
            lmQPeak    = 0x0200,  // include Q-peaks
            lmQPeakI   = 0x0400,  // Q-peaks intensity
            lmFixed    = 0x0800;  // fixed values

class TXGlLabels: public AGDrawObject  {
  TPtrList<class TXAtom> FAtoms;
  short  FFontIndex;
  TGlMaterial FMarkMaterial;
  TArrayList<bool> FMarks;
  short Mode;
public:
  TXGlLabels(const olxstr& collectionName, TGlRender *Render);
  void Create(const olxstr& cName = EmptyString, const CreationParams* cpar = NULL);
  virtual ~TXGlLabels();

  void Clear();
  void ClearLabelMarks();

  DefPropP(short, Mode)

  void Selected(bool On);

  bool Orient(TGlPrimitive *P);
  bool GetDimensions(vec3d &Max, vec3d &Min) {  return false;  }
  void AddAtom(class TXAtom *A);
  void MarkLabel(TXAtom *A, bool v);

  inline int AtomCount() const        {  return FAtoms.Count(); }
  inline TXAtom*  Atom(int i)  const  {  return FAtoms[i];  }

  TGlFont *Font() const;
  void FontIndex(short FntIndex)  {  FFontIndex = FntIndex; }
  short FontIndex() const         {  return FFontIndex; }

  TGlMaterial& MarkMaterial()     {  return FMarkMaterial; }
};

EndGxlNamespace()
#endif
 
