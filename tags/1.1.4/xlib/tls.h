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
	const ematd& GetT() const {return Tmat;}
	const ematd& GetL() const {return Lmat;}
	const ematd& GetS() const {return Smat;}
	const evecd& GetOrigin() const {return origin;}
	const evecd& GetFoM() const {return FoM;}
	const TEllpList& GetElpList() const {return newElps;}
	const ematd& GetRtoLaxes() const {return RtoLaxes;}
	const ematd& GetLVcV() const {return LVcV;}
	void printTLS();
	
	void calcUijEllipse (const TSAtomPList &atoms, TEllpList &Elps);
	bool calcTLS (const ematd& designM, const evecd& UijC);
	void BondCorrect(TSAtom *atom1, TSAtom*  anAtom2, double &correctedlength,double &correctedError);
	void extrapolate(TSAtom *atom, TEllpList &ellipsoid) ; // Extapolate TLS motion to atom
	
private:
//	TSAtomPList RigidBody; //atomslist in constructor + additions to the body

//Frame definitions and transformations matrices
	evecd origin;
	ematd Rcart,			//Transformation from Cartesian to Xtal cell
			 RcartT,		//Transpose Rcart
			 Rcell,			//Inverse of above: cell to Cartesian transf
			 RtoLaxes;		//Rotation matrix, transforms TLS to L-principle axes. Nb. Inverse = Transpose since orthog	
	ematd adpScale,		//Scales ADPs in Xtal frame from fractions to Angstrom
			 adpScaleInv;	//Inverse adpScale

	//TLS wrt current frame: Updated through analysis
	ematd Tmat,	
			 Lmat, 
			 Smat,
			 SmatT;		//Transpose of Smat
	
	TEllpList newElps;	//Ellipsoids calculated from TLS 
	unsigned short TLSfreeParameters; // 21, To be reduced by 1 per constraint 
									  //(unless enforced with Lagrange multipliers?).
	
	ematd designM;	// designMatrix 
	evecd UijCol;    // Uij from atomlist: the 'observed Uijs' (6 per atom) 
	ematd UijWeight; // 1/Errors on UijCol.  - 1/VcV weight matrix 
	evecd FoM;		// {R1,R2, sqrt(chi^2)}
	ematd TLS_VcV;
	ematd LVcV;

	void UijErrors(const TSAtomPList &atoms); // NOTE: To be replace when VcV matrix is available
	void createDM(ematd &designM, evecd &UijC ,const TSAtomPList &atoms);
	void calcUijCart (ematd &Uij,const evecd &atomPosition);
	void calcTLS_VcV(evecd &w, ematd &v);
	void RotateLaxes();
	void calcL_VcV();
	void FigOfMerit(const TSAtomPList &atoms, const TEllpList Elps);
	void quadToCart(const evecd &quad, ematd &UijCart);  // Converts Uij quadractic wrt Xtal basis to Catesian
	void symS(evecd &shifts); // Shifts origin - makes S symmetric
	void diagS(ematd &split, ematd &Tmatrix, ematd &Smatrix); //Splits L axes to make S diagonal
	
	//helper mathods:
	void epsil(const short &i, const short &j,const short &k, short &ep); 
	void inverse3x3(ematd &theMatrix);  
	void tlsToShelx(short &i, short &p);
};

EndXlibNamespace()
#endif
