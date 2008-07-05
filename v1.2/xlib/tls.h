// TLS procedure, (c) J Haestier, 2007
#ifndef tlsH
#define tlsH

#include "xbase.h"
#include "ematrix.h"
#include "satom.h"
#include "ellipsoid.h"

BeginXlibNamespace()

//---------------------------------------------------------------------------
class TLS {
public:
	TLS(const TSAtomPList &atoms, const double *const cellParameters);
	~TLS(){}
	
	//accessors
	const TMatrixD& GetT() const {return Tmat;}
	const TMatrixD& GetL() const {return Lmat;}
	const TMatrixD& GetS() const {return Smat;}
	const TVectorD& GetOrigin() const {return origin;}
	const TVectorD& GetFoM() const {return FoM;}
	const TEllpList& GetElpList() const {return newElps;}
	const TMatrixD& GetRtoLaxes() const {return RtoLaxes;}
	const TMatrixD& GetLVcV() const {return LVcV;}
	const void printTLS();
	
	void calcUijEllipse (const TSAtomPList &atoms, TEllpList &Elps);
	bool calcTLS (const TMatrixD &designM, const TVectorD &UijC);
	void BondCorrect(TSAtom *atom1, TSAtom*  anAtom2, double &correctedlength,double &correctedError);
	void extrapolate(TSAtom *atom, TEllpList &ellipsoid) ; // Extapolate TLS motion to atom
	
private:
//	TSAtomPList RigidBody; //atomslist in constructor + additions to the body

//Frame definitions and transformations matrices
	TVectorD origin;
	TMatrixD Rcart,			//Transformation from Cartesian to Xtal cell
			 RcartT,		//Transpose Rcart
			 Rcell,			//Inverse of above: cell to Cartesian transf
			 RtoLaxes;		//Rotation matrix, transforms TLS to L-principle axes. Nb. Inverse = Transpose since orthog	
	TMatrixD adpScale,		//Scales ADPs in Xtal frame from fractions to Angstrom
			 adpScaleInv;	//Inverse adpScale

	//TLS wrt current frame: Updated through analysis
	TMatrixD Tmat,	
			 Lmat, 
			 Smat,
			 SmatT;		//Transpose of Smat
	
	TEllpList newElps;	//Ellipsoids calculated from TLS 
	unsigned short TLSfreeParameters; // 21, To be reduced by 1 per constraint 
									  //(unless enforced with Lagrange multipliers?).
	
	TMatrixD designM;	// designMatrix 
	TVectorD UijCol;    // Uij from atomlist: the 'observed Uijs' (6 per atom) 
	TMatrixD UijWeight; // 1/Errors on UijCol.  - 1/VcV weight matrix 
	TVectorD FoM;		// {R1,R2, sqrt(chi^2)}
	TMatrixD TLS_VcV;
	TMatrixD LVcV;

	void UijErrors(const TSAtomPList &atoms); // NOTE: To be replace when VcV matrix is available
	void createDM(TMatrixD &designM, TVectorD &UijC ,const TSAtomPList &atoms);
	void calcUijCart (TMatrixD &Uij,const TVectorD &atomPosition);
	void calcTLS_VcV(TVectorD &w, TMatrixD &v);
	void RotateLaxes();
	void calcL_VcV();
	void FigOfMerit(const TSAtomPList &atoms, const TEllpList Elps);
	void quadToCart(const TVectorD &quad, TMatrixD &UijCart);  // Converts Uij quadractic wrt Xtal basis to Catesian
	void symS(TVectorD &shifts); // Shifts origin - makes S symmetric
	void diagS(TMatrixD &split, TMatrixD &Tmatrix, TMatrixD &Smatrix); //Splits L axes to make S diagonal
	
	//helper mathods:
	void epsil(const short &i, const short &j,const short &k, short &ep); 
	void inverse3x3(TMatrixD &theMatrix);  
	void tlsToShelx(short &i, short &p);
};

EndXlibNamespace()
#endif
