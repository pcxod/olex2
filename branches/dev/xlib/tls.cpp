//---------------------------------------------------------------------------//
// (c) James J Haestier, 2007
//---------------------------------------------------------------------------//
#include "tls.h"
#include "satom.h"
#include "asymmunit.h"
#include "svd.h"
#include "inv.h"
#include "math/mmath.h"

/*
TLS module:



*/

TLS::TLS(const TSAtomPList &atoms, const double *const cellParameters)
{
	/* Calculates TLS fit for group of atoms including 
		- intialise designMatrix for SVD
		- TLS matrices optimised by SVD to fit UijColumn
	// Input: 
	//   atoms is a list of atoms in rigid body (postion, adp)
	//   cellParameters[12], (a,b,c,alpha,beta,gamma, "6 errors in same order") in Ang/degrees
	//	 atomsTLS = copy of atoms to be overwritten with Uij calc from TLS
	// Output: atomsTLS with Uij calc from TLS.

	//std::cout << "\n a is: " << cellParameters[0] << std::endl; //verbose check

	*/

	TLSfreeParameters = 21;
	
	const double &a		= cellParameters[0], 
				 &b		= cellParameters[1], 
				 &c		= cellParameters[2],
				 &alpha = cellParameters[3]*M_PI/180, // alpha, beta, gamma converted to radians
				 &beta	= cellParameters[4]*M_PI/180, 
				 &gamma = cellParameters[5]*M_PI/180;

	const double cellVolume = a*b*c*sqrt(1 - cos(alpha)*cos(alpha) - cos(beta)*cos(beta) 
						- cos(gamma)*cos(gamma) + 2*cos(alpha)*cos(beta)*cos(gamma));

	{
		ematd temp(3,3);
		temp.I();
		RtoLaxes = temp; //intialise as identity for calcUij
	}

	//Use an atom as origin (reducing numerical errors on distances)
	TSAtom* anAtom = atoms[0]; //can be any atom or point; Recommend near rigid body
	origin.Assign(anAtom->crd(), 3); //in Cartesian


	// ADP 'Scaling' matrix (transfroming U^ij(Ang) to 2Pi^2 beta^ij(frac))
	{
		ematd adpScaleTemp(3,3); //scales ADPs from Ang to fraction of Xtal cell
		adpScaleTemp[0][0] = b*c*sin(alpha)/cellVolume;
		adpScaleTemp[1][1] = a*c*sin(beta)/cellVolume;
		adpScaleTemp[2][2] = a*b*sin(gamma)/cellVolume;
		adpScale = adpScaleTemp;
		
		for (int i =0 ; i<3 ; i++)
			adpScaleTemp[i][i] = 1/adpScaleTemp[i][i];

		adpScaleInv = adpScaleTemp;
	}


	 //initialise Transformations Rcart and Rcell
	{
		const mat3d& Rc  = anAtom->CAtom().GetParent()->GetCellToCartesian(),
					   &Rcc = anAtom->CAtom().GetParent()->GetCartesianToCell();
		//4x4 matrix for no good reason (Oleg's fault! :-)	)
		//change to 3x3, Oleg defn returns transpose
		ematd RcellTemp (3,3),
				 RcartTemp (3,3);
		for ( int p = 0; p < 3; p++ ){
			for ( int q = 0; q < 3; q++){
				RcellTemp[p][q] = Rc[p][q]; 
				RcartTemp[p][q] = Rcc[p][q]; 
			}
		}
			
		Rcart = RcartT = RcartTemp;
		Rcart.Transpose();
		Rcell = RcellTemp;
		Rcell.Transpose();
	}
/************************************************/
	UijErrors(atoms); //Calculate errors on Uij input - to be replaced be VcV when available from Olex
					  //currently unit weights
/************************************************************************/
	//Create design matrix and find TLS matrices using SVD
	createDM(designM, UijCol , atoms); //dm is function of atoms' coords in Cartesian.
										//UijCol create wrt same coord system
	calcTLS(designM,UijCol); //TLS wrt same frame/origin as UijCol
	printTLS();

/************************************************************************/
//Analysis :	1) rotate to L- principle axes
		//		2) shift origin; S symmetrix
		//		3) split axes; S diagonal
	RotateLaxes();     // Rotate TLS tensor to L principle axes: diagonalise L
	printTLS();

	calcUijEllipse (atoms, newElps); //Initialise newElps to TLS for atoms
	FigOfMerit (atoms, newElps); //Independant of coordinates, calculated with Uij in cartesian
	
	evecd rho(3); //Origin shift
	symS(rho); //Shift origin so S becomes symmetric
	printTLS();

	{
		ematd splitAxes(3,3), Tmatrix(3,3), Smatrix(3,3);
		diagS(splitAxes,Tmatrix,Smatrix);
		Tmatrix.Print();
		Smatrix.Print();
	}

/************************************************************************/

}
void TLS::printTLS(){
	// std::cout << "\nT matrix: ";
	Tmat.Print(); 
	// std::cout << "\nL matrix: ";
	Lmat.Print(); 
	// std::cout << "\nS matrix: ";
	Smat.Print();
}
void TLS::UijErrors(const TSAtomPList &atoms){
// Unit weights  - should be replaced by VcV matrix when available
	ematd temp (6*atoms.Count(), 6*atoms.Count() );
	for( size_t i= 0; i< (6*atoms.Count() ) ; i++)
		for( size_t j=0; j<(6*atoms.Count() ); j++)
			temp[i][j] = 10000.0; //weight = 1/sigma^2, sigma =0.01 Ang. 

	UijWeight = temp; //Diagonal weight matrix

	//Calculating errors on Uij from errors on cellParameters.
	//This should be replaced when Variance-Covariance matrix is available from Olex

	//Algorithym:
	//	1) Create Vector of cell errors; rotate to Cartesian frame.
	//	2)
/*
	&err_a = cellParameters[6],
	&err_b = cellParameters[7],
	&err_c = cellParameters[8],
	&err_alpha = cellParameters[9]*M_PI/180,
	&err_beta = cellParameters[10]*M_PI/180,
	&err_gamma = cellParameters[11]*M_PI/180;

	ematd errors(3);
	errors[0] = err_a;errors[1] = err_b;errors[2] = err_c;
	errors =  Rcart*errors; //Errors on Cart. cell axes to 1st order (errors on angles are 2nd order)
	std::cout << "\nErrors: " << errors[0] << ", "
		<< errors[1] << ", " << errors[2] <<"\n";
	Rcart.Print();*/
}

void TLS::createDM(ematd &designM, evecd &UijC , const TSAtomPList &atoms )
{
	ematd dm( 6*atoms.Count(),TLSfreeParameters );	//TLS (6n x 21) design matrix
	evecd UijColumn(6*atoms.Count());	//6n vector of 'observed' Uij

	for( size_t i=0; i < atoms.Count(); i++ )  {  
		// for each atom calculate design matrix (dm) and UijColumn 'observed' (Uobs)
		// Append result to the existing dm / Uobs

		TSAtom* theAtom = atoms[i];

		if ( theAtom->GetEllipsoid() == NULL )
			throw TInvalidArgumentException(__OlxSourceInfo, "Isotropic atom: invalid TLS input"); 
		
		//Atom coords wrt origin, Cartesian axes
		double x = (theAtom->crd()[0] - origin[0]) ;
		double y = (theAtom->crd()[1] - origin[1]) ;
		double z = (theAtom->crd()[2] - origin[2]) ;

		//Creating design matrix for SVD
		//U11
		dm[i*6][0] = 1;	
		dm[i*6][10] = 2*z;			// 2z
		dm[i*6][14] = z*z;			// z^2
		dm[i*6][15] = -2*y;			// -2y
		dm[i*6][19] = -2*y*z;		// -2yz	
		dm[i*6][20] = y*y;			// y^2

		//U12
		dm[i*6+1][1] = 1;
		dm[i*6+1][6] = -z;		//-z
		dm[i*6+1][11] = z;		//z
		dm[i*6+1][13] = -z*z;	//-z^2
		dm[i*6+1][15] = x;		//x
		dm[i*6+1][16] = -y;		//-y
		dm[i*6+1][18] = y*z;	//yz
		dm[i*6+1][19] = x*z;	//xz
		dm[i*6+1][20] = -x*y;	//-xy
		
		//U22
		dm[i*6+2][2] = 1;	
		dm[i*6+2][7] = -2*z;	//-2z
		dm[i*6+2][9] = z*z;		//z^2
		dm[i*6+2][16] = 2*x;	//2x
		dm[i*6+2][18] = -2*x*z; //-2xz	
		dm[i*6+2][20] = x*x;	//x^2

		//U13
		dm[i*6+3][3] = 1;
		dm[i*6+3][6] = y;			//y
		dm[i*6+3][10] = -x;			//-x
		dm[i*6+3][12] = z;			//z
		dm[i*6+3][13] = y*z;		//yz
		dm[i*6+3][14] = -x*z;		//-xz
		dm[i*6+3][17] = -y;			//-y
		dm[i*6+3][18] = -y*y;		//-y^2
		dm[i*6+3][19] = x*y;		//xy

		//U23
		dm[i*6+4][4] = 1;
		dm[i*6+4][7] = y;			//y
		dm[i*6+4][8] = -z;			//-z
		dm[i*6+4][9] = -y*z;		//-yz
		dm[i*6+4][11] = -x;			//-x
		dm[i*6+4][13] = x*z;		//xz
		dm[i*6+4][17] = x;			//x
		dm[i*6+4][18] = x*y;		//xy
		dm[i*6+4][19] = -x*x;		//-x^2

		//U33
		dm[i*6+5][5] = 1;	
		dm[i*6+5][8] = 2*y;			//2y
		dm[i*6+5][9] = y*y;			//y^2
		dm[i*6+5][12] = -2*x;		//-2x
		dm[i*6+5][13] = -2*x*y;		//-2xy	
		dm[i*6+5][14] = x*x;		//x^2
		/************************************************************/
		// Create vector of 'observed' UijColumns, assign to matrix and
		// transform to cartesian frame
		/* Algorithym:	(1) Get ellipse quadratic form wrt Xtal axes
						(2) Create Uij matrix from quad
						(3) Transfrom to Cartesian
						(4) Create column of Cartesian Uij elements
		*/

		evecd quad(6);						//ellipse quadratic 
		 		 
		ematd UijCart(3,3);					// Uij wrt Cartesian axes
		theAtom->GetEllipsoid()->GetQuad( quad ); //ADP wrt xtal frame
		quadToCart(quad, UijCart);

		UijColumn[i*6]	 = UijCart[0][0];	//U11 in cartesian
		UijColumn[i*6+1] = UijCart[1][0];	//U21
		UijColumn[i*6+2] = UijCart[1][1];	//U22
		UijColumn[i*6+3] = UijCart[2][0];	//U31
		UijColumn[i*6+4] = UijCart[2][1];	//U32
		UijColumn[i*6+5] = UijCart[2][2];	//U33
			
	} //i loop
		
	//Multiply by weight matrix:
	//dm.Print();
	// std::cout<< "\ndm: " << dm.Vectors() << " x " <<dm.Elements();

	// std::cout<< "\nUijWeight: " << UijWeight.Vectors() << " x " << UijWeight.Elements();
	dm = UijWeight*dm;
	
	UijColumn = UijWeight * UijColumn; //matrix multiplication in Olex gives transpose!
	designM = dm;		//returning results
	UijC = UijColumn;
} 


bool TLS::calcTLS (const ematd &designM, const evecd &UijC)
{
	// Uses SVD.
	// Finds TLS wrt origin and frame used in design matrix and Uij.
	/*Algorhythm:
		(1) Decompose design matrix (dm)	
		(2) Find 'inverse' dm
		(3) calc tls = inverseDM.UijC
		(4) Assign TLS elements to TLS matrices

	*/

	const int nColumns	= (int)designM.Elements();	//No. of columns
	const int mRows		= (int)designM.Vectors();	//No. of Rows

	if (TLSfreeParameters!= nColumns)
		throw TFunctionFailedException(__OlxSourceInfo, 
		"TLS Failed: Number of free TLS parameters not same as design matrix width");

	if (nColumns> mRows)
		throw TFunctionFailedException(__OlxSourceInfo, 
		"TLS Failed: more tls parameters than equations.\nTry adding more atoms or constraints.");


	ap::template_2d_array<double> designMatrix; //copy dm for SVD
    designMatrix.setbounds(1, mRows,1, nColumns);

	for(int j = 0; j < mRows; j++) {	
			for(int i = 0; i < nColumns; i++) {
				designMatrix(j+1,i+1) = designM[j][i];  	
			}		
	}

	int uNeeded = 2; //Options of SVD  - calculate U, V and W 
	int vtNeeded = 2;
	int useAdditionalMemory = 0; //2;

	//SVD output matrices
	ap::template_1d_array<double> wOutput;
	wOutput.setbounds(1,nColumns);				//Nb. diagonal nxn 2d array, only diagonal terms outputed
	ap::template_2d_array<double> uOutput;
	uOutput.setbounds(1,mRows,1,nColumns);
	ap::template_2d_array<double> vtOutput;
	vtOutput.setbounds(1,nColumns,1,nColumns);  //Nb. transpose(v) outputed

	bool svdRes;
	svdRes = svddecomposition(designMatrix, mRows, nColumns,
		uNeeded, vtNeeded, useAdditionalMemory,
		wOutput, uOutput, vtOutput);

  ematd m(designM), vt, u;
  evecd w;
  math::SVD::Decompose(m, 2, 2, w, u, vt);
  math::alg::print1(vtOutput, nColumns, nColumns);
  math::alg::print0(vt, nColumns, nColumns);
	{//Calculate variance-covariance matrix before further changes to w, vt
		evecd w(nColumns);
		for (int j = 0; j<nColumns; j++)
			w[j] = wOutput(j+1);

		ematd V(nColumns,nColumns);
		for (int j = 0; j<nColumns; j++){
			for (int k = 0; k<nColumns; k++){
				V[j][k] = vtOutput(k+1,j+1);
			}
		}
		calcTLS_VcV(w,V);
	}

	// std::cout << "\nSVD success: " << svdRes << std::endl;
  /***************************************************************************/
	// Compute tls elements from decomposition matrices

	evecd tlsElements(TLSfreeParameters); // = v* 1/w * u^transpose . UijColumn(vector)
	/* Order: (t11, t12,t22,t13,t23,t33,s11,s12,s13,l11,s21,s22,s33,l12,l22,s31,s32,s33,l13,l23,l33) */

	
	// invert w, special case w=0 -> 1/w =0
	for (short i = 1; i <= TLSfreeParameters; i++){
		if ( wOutput(i) ) 
			wOutput(i) = 1.0/wOutput(i);
	}


	//compute  v[i][j]. 1/w[j][j]
	ap::template_2d_array<double> vInvw;
	vInvw.setbounds(1,nColumns,1,nColumns);  

	for (short i = 1; i <= TLSfreeParameters; i++){
		for (short j = 1; j<= TLSfreeParameters; j++) {
			vInvw(i,j) = vtOutput(j,i) * wOutput(j);
			//std::cout << vInvw(i,j) <<",  " ;  //debug statement
		}
		//std::cout << "\n" <<std::endl;			//debug statement
	}	

	//compute inverseDM = vInvW[i][j].transpose(u)[j][k], 

	ap::template_2d_array<double> inverseDM;
    inverseDM.setbounds(1, nColumns,1, mRows);

	for ( int i=1; i <= nColumns; i++){
		for ( int j = 1; j <= mRows; j++) {
			inverseDM(i,j) = 0 ;
		}
	}
	
	for ( int k=1; k <= mRows; k++){
		for ( int i = 1; i <= nColumns; i++) {	
			for ( int j = 1; j <= nColumns; j++) {
				inverseDM(i,k) = inverseDM(i,k) + vInvw(i,j)*uOutput(k,j);
			}
		}
	}


	//tls = v* 1/w * u^transpose . UijColumn(vector)
	for(int k = 0; k< nColumns; k++){
		for(int i = 0; i < mRows ; i++) {	 
			tlsElements[k] = tlsElements[k] + inverseDM(k+1,i+1)* UijCol[i];
		}
	}

	ematd Tmatrix(3,3);		 //TLS  matrices
	ematd Lmatrix(3,3);		 
	ematd Smatrix(3,3);		

	Tmatrix[0][0] =					tlsElements[0];
	Tmatrix[1][0] = Tmatrix[0][1] = tlsElements[1];
	Tmatrix[0][2] = Tmatrix[2][0] = tlsElements[3];
	Tmatrix[1][1] =					tlsElements[2];
	Tmatrix[1][2] = Tmatrix[2][1] = tlsElements[4];
	Tmatrix[2][2] =					tlsElements[5];

	Lmatrix[0][0] =					tlsElements[9];
	Lmatrix[1][0] = Lmatrix[0][1] = tlsElements[13];
	Lmatrix[0][2] = Lmatrix[2][0] = tlsElements[18];
	Lmatrix[1][1] =					tlsElements[14];
	Lmatrix[1][2] = Lmatrix[2][1] = tlsElements[19];
	Lmatrix[2][2] =					tlsElements[20];

	//(t11, t12,t22,t13,t23,t33,s11,s12,s13,l11,s21,s22,s23,l12,l22,s31,s32,s33,l13,l23,l33)

	Smatrix[0][0] =	tlsElements[6];
	Smatrix[0][1] = tlsElements[7];
	Smatrix[0][2] = tlsElements[8];
	Smatrix[1][0] = tlsElements[10];
	Smatrix[1][1] =	tlsElements[11];
	Smatrix[1][2] = tlsElements[12];
	Smatrix[2][0] = tlsElements[15];
	Smatrix[2][1] = tlsElements[16];
	Smatrix[2][2] =	tlsElements[17];

	Tmat = Tmatrix;
	Lmat = Lmatrix;
	Smat = Smatrix;

	Smatrix.Transpose();
	SmatT = Smatrix;

	return svdRes;
	// END of SVD call
}

void TLS::RotateLaxes(){
	//Rotates TLS tensors to L principle axes and records
	//rotation matrix, RtoLaxes.

	ematd rotate_to_L_axes(3,3);	
	rotate_to_L_axes.I();
	Lmat.EigenValues(Lmat,rotate_to_L_axes); //diagonalise Lmatrix

	// std::cout << "L eigenvectors (rotation matrix to L-principle axes): " << std::endl;
	RtoLaxes = rotate_to_L_axes;
	RtoLaxes.Print();
	/*for (int k = 0; k<3; k++)
		std::cout << "\norigin " << k << ": " << origin[k];*/

	calcL_VcV();//compute L variance-covariance matrix

	origin = RtoLaxes * origin; // ~R.origin in Olex
	
	Tmat = rotate_to_L_axes * Tmat;
	Smat = rotate_to_L_axes * Smat;
	rotate_to_L_axes.Transpose();
	
	//Lcopy = Lcopy*rotate_to_L_axes;
	Tmat = Tmat*rotate_to_L_axes ;
	Smat = Smat*rotate_to_L_axes ;

	SmatT = Smat;
	SmatT.Transpose();

	// std::cout << "\nTLS tensors wrt L principle axes: \n";
	printTLS();
}
void TLS::calcL_VcV(){
// VcV_L = sum_(m,n,p,q) d L_Laxes(i,i) / d L_cart(m,n) * d L_Laxes(j,j) / d L_cart(p,q) *VcVLCart(mn,pq)

	//compute VcV_L_cart from TLS_VcV_cart  
	ematd VcV(6,6); //order: l11,l22,l33,l23,l13,l12 (shelx)
	for(short i=0; i<6; i++){
		for(short j=0; j<6; j++){
			short p,q;
			tlsToShelx(i,p);
			tlsToShelx(j,q);
			VcV[i][j] = TLS_VcV[p][q];
		}
	}
	//check
	for(short i=0; i<6; i++){
		for(short j=0; j<6; j++){
			if(VcV[i][j] != VcV[i][j])
				throw TFunctionFailedException(__OlxSourceInfo, 
		"TLS Failed: VcV matrix for L in Cartesian not symmetric");
		}
	}

	ematd diff(3,6);
	//differiential matrix,diff[i][j] dL_laxes[i][i]/dLcartesian[j], j= 11,22,33,32,31,21 (shelx)
	for(short i=0; i<3; i++){  //ith eigenvalue
			ematd temp(3,3);
			for(short m=0; m<3; m++){
				for(short n=m; n<3; n++){ //symmetric - only calc 6 elements
					temp[m][n] = RtoLaxes[i][m]*RtoLaxes[i][n] ; 
				}
			}
			diff[i][0]= temp[0][0];
			diff[i][1]= temp[1][1];
			diff[i][2]= temp[2][2];
			diff[i][3]= temp[1][2];
			diff[i][4]= temp[0][2];
			diff[i][5]= temp[0][1];
	}	

	ematd VcVtemp(3,3); //Not 6x6: Only 3 diagonal L_Laxes values
	for(short i=0; i<3; i++){
		for(short j=0; j<3; j++){ // for Covariance [L(i,i),L(j,j)]
			for(short m=0; m<6; m++){
				for(short n=0; n<6; n++){ 
//VCV (L(i),L_L(j))= dL_L(i)/dL_Cart(m) * dL_L(j)/ d LCart(n) *VCV(Lcart,Lcart)
					VcVtemp[i][j] =  VcVtemp[i][j] + diff[i][m]*diff[j][n] *VcV[m][n]; 
				}
			}
			if (i!=j){
				VcVtemp[i][j] = 2*VcVtemp[i][j]; 
				// off-diag elements count twice due to symmetry, [i][j] = [j][i]
			}
		}
	}
	LVcV=VcVtemp;
}
void TLS::calcUijEllipse (const TSAtomPList &atoms, TEllpList &Ellipsoids) {
	/* For each atom, calc U_ij from current TLS matrices */
	TEllpList Elps;
	for( size_t i=0; i < atoms.Count(); i++ )  {  
		// for each atom, calculate Utls in Cart
		
		TSAtom* theAtom = atoms[i];
		//Atom coords wrt origin, as fraction of unit cell axes

		evecd position(3);
		for (int ii = 0; ii < 3 ; ii++)
			position[ii] = (theAtom->crd()[ii]); // wrt Cartesian
		
		position =  RtoLaxes * position;  // = R.x in Olex, position wrt Laxes
		
		ematd UtlsLaxes(3,3); 
		calcUijCart(UtlsLaxes,position);

		ematd RtoLaxesInv = RtoLaxes;  //inverse
		inverse3x3(RtoLaxesInv);
		ematd RtoLaxesInvT = RtoLaxesInv; //inverse Transposed
		RtoLaxesInvT.Transpose();
		
		ematd UtlsCell(3,3);
		UtlsCell = adpScaleInv * Rcart *RtoLaxesInv * UtlsLaxes 
			* RtoLaxesInvT* RcartT * adpScaleInv ; // intially RtoLaxesInv = 1 before TLS matrixes are rotated
		
		
		/*
		std::cout << "\nUtlsLaxes";
		UtlsLaxes.Print();
		*/

		//Add to ellipse list
		evecd vec(6);
		vec[0]=	UtlsCell[0][0];
		vec[1]=	UtlsCell[1][1];
		vec[2]=	UtlsCell[2][2];
		vec[3]=	UtlsCell[2][1];
		vec[4]=	UtlsCell[2][0];
		vec[5]=	UtlsCell[0][1];
		
		/*std::cout <<"\nAtom " << i << ", Uij from structure refinement:";
		atoms[i]->CAtom()->GetEllipsoid()->GetMatrix().Print();
		std::cout <<"Uij  frome TLS:";*/
		
		Elps.AddNew( vec) ;//.GetMatrix().Print();
		
	}	
	Ellipsoids = Elps;
}



void TLS::calcUijCart (ematd &Uij,const evecd &position){
	// Calculates Uij from atom position wrt ANY Cartesian frame,
	// (position must be wrt same axes as current TLS tensors
	// uses current TLS tensors and origin
	
	double x = position[0] - origin[0];	
	double y = position[1] - origin[1];
	double z = position[2] - origin[2];

	/*
	std::cout << "\nposition x y x: " 
				<< position[0] <<", "
				<< position[1]<<", "
				<< position[2]<<".\nOrigin x y z:"	
				<< origin[0] <<", "
				<< origin[1] <<", "
				<< origin[2] ;
				*/

	ematd Atls(3,3);
	Atls[0][1] = z; Atls[1][0] = -z;
	Atls[0][2] = -y; Atls[2][0] = y;
	Atls[1][2] = x; Atls[2][1] = -x;
	//Atls.Print(); //A wrt initial cartesian cell

	ematd AtlsT = Atls;
	//std::cout << "Atls in cartesian:\n" ;
	//Atls.Print();
	AtlsT.Transpose();
	//std::cout << "\nAtls in L axes:";
	//Atls.Print();
	Uij = Tmat + Atls*Smat + SmatT*AtlsT + Atls*Lmat*AtlsT;
}
void TLS::calcTLS_VcV(evecd &w, ematd &v){
	//takes references to SVD matrix, w ,V
	//calculates Variance-covariance of output vector (TLS elements)
	ematd VcV(TLSfreeParameters,TLSfreeParameters);
	for (int j = 0; j < TLSfreeParameters; j++){
		for (int k = 0; k < TLSfreeParameters; k++){
			for (int i = 0; i < TLSfreeParameters; i++){
				if(w[i]){
					VcV[j][k] = VcV[j][k] 
					+ ( v[j][i]*v[k][i]	/ (w[i]*w[i])	); //sum_i v(j,i) v(k,i)/ w(i)^2
				}
			}
		}
	}
	
	//VcV.Print();
	TLS_VcV = VcV;
}
void TLS::inverse3x3(ematd &theMatrix) {
	// Only valid for 3x3 matrix
	if (theMatrix.Elements() != 3 || theMatrix.Vectors() !=3)
		throw TFunctionFailedException(__OlxSourceInfo, 
		"TLS Failed: Inverse of non 3x3 matrix called");
	//std::cout << "\naM before:";
	
  if(!math::LU<double>::Invert(theMatrix))
		throw TFunctionFailedException(__OlxSourceInfo, 
		"Inverse of singular matrix or other inverse problem: failed");
}


void TLS::FigOfMerit(const TSAtomPList &atoms, const TEllpList Elps){	
	//sets FoM = {R1,R2,Sqrt[chi^2]}
	// R1 = sum |Uobs - Utls| / sum |Uobs|
	// R2 = Sqrt[ sum {weight_Uobs (Uobs - Utls)^2} / sum{weigh_Uobs Uobs^2}  ]
	// Sqrt[chi^2] = Sqrt[ sum {weight_Uobs (Uobs - Utls)^2}/ n ], n dof

	evecd quad(6);
	ematd UijCart(3,3);
	evecd diff (6* atoms.Count() ) ;
	for (size_t i =0; i < atoms.Count(); i++){
			Elps[i].GetQuad(quad);
			quadToCart(quad, UijCart);

			diff[6*i] = UijCol[6*i] - UijCart[0][0];  // Uobs - Utls in Cartesian
			diff[6*i+1] = UijCol[6*i+1] - UijCart[1][0];
			diff[6*i+2] = UijCol[6*i+2] - UijCart[1][1];
			diff[6*i+3] = UijCol[6*i+3] - UijCart[2][0];
			diff[6*i+4] = UijCol[6*i+4] - UijCart[2][1];
			diff[6*i+5] = UijCol[6*i+5] - UijCart[2][2];
	}

	//R1
	double sumUobs=0;
	double R1 = 0;
	for (size_t i=0; i<6*atoms.Count(); i++){
		if ( diff[i]>0 )
			R1 = R1 + diff[i];
		else
			R1 = R1 - diff[i];

		if (UijCol[i] > 0 )
			sumUobs = sumUobs + UijCol[i];
		else
			sumUobs = sumUobs - UijCol[i];
	}
	R1 = R1/sumUobs;

	//R2
	double R2=0;
	double sumUobsSq = 0;
	for (size_t i =0; i<6*atoms.Count(); i++){
		R2 =  R2 + UijWeight[i][i]* diff[i]*diff[i];
		sumUobsSq = sumUobsSq + UijWeight[i][i]*UijCol[i]*UijCol[i];
	}

	double chiSq = R2 / (6*atoms.Count() - TLSfreeParameters );

	R2 = sqrt( R2/sumUobsSq ) ;

	evecd FoMtemp(3);
	FoMtemp[0] = R1;
	FoMtemp[1] = R2;
	FoMtemp[2] = sqrt( chiSq );
	FoM = FoMtemp;
}

void TLS::quadToCart(const evecd &quad, ematd &UijCart){
	// Converts Uij quadratic wrt the crystal frame, to the Cartesian frame

	ematd UijCell(3,3);
	UijCell[0][0] = quad[0];
	UijCell[0][1] = UijCell[1][0] = quad[5];
	UijCell[0][2] = UijCell[2][0] = quad[4];
	UijCell[1][1] = quad[1];
	UijCell[1][2] = UijCell[2][1] = quad[3];
	UijCell[2][2] = quad[2];

	ematd RcellT = Rcell; 
	RcellT.Transpose();
	UijCart = Rcell * adpScale * UijCell * adpScale * RcellT;
}

void TLS::symS(evecd &rhoOut){
	evecd rho(3); //shifts of origin
	
	for(int i =0; i<3; i++){
		int j,k;
		if(i==0){ j=1;	k=2;	 }
		else if(i==1){ 	j=2;k=0; }
		else if(i==2){ j=0; k=1; }

		rho[i] = (Smat[j][k] - Smat[k][j]) / (Lmat[j][j] + Lmat[k][k]);
	}
	rhoOut = rho;

	ematd P(3,3);
	P[0][1] = rho[2]; P[1][0]=-rho[2];
	P[1][2] = rho[0]; P[2][1]=-rho[0];
	P[2][0] = rho[1]; P[0][2]=-rho[1];
	P.Print();
	ematd Pt = P;
	Pt.Transpose();

	Tmat = Tmat + P*Smat + SmatT*Pt + P*Lmat*Pt;
	Smat = Smat + Lmat*Pt;
	SmatT = Smat;
	SmatT.Transpose();

	origin = origin + rho; //Origin wrt L axes
}

void TLS::diagS(ematd &split, ematd &Tmatrix, ematd &Smatrix){
	// Calc split of L-principle axes
	short ep;
	for (short i=0; i<3; i++){
		for(short j=0; j<3 ; j++){
			for (short k =0; k <3; k++){ 
				epsil(i,j,k,ep);
				if( ep )
					split[i][j] = ep*Smat[j][k]/Lmat[j][j]; 
			}
		}
	}
	// Calc change to T matrix
	ematd temp(3,3);
	for (short i=0; i<3; i++){
		for(short j=0; j<3 ; j++){
			for (short k =0; k <3; k++){
				if(i!=j)
					temp[i][j] = temp[i][j]+ Smat[k][i]*Smat[k][j]/Lmat[k][k];
				else if(i!=k) //i==j !=k
					temp[i][j] = temp[i][j]+ Smat[k][i]*Smat[k][j]/Lmat[k][k];
				//no contrib from i=j=k
			}
			Tmatrix[i][j] = Tmat[i][j] - temp[i][j];
			temp[i][j]=0; // reset for Smat transf
		}
	}
	//Change to S matrix
	for (short i=0; i<3; i++){
		for(short j=0; j<3 ; j++){
			for (short m =0; m <3; m++){
				for (short n =0; n <3; n++){
					epsil(m,n,j,ep);
					if( i!=j && ep )
						temp[i][j] = temp[i][j] + ep *Lmat[i][m]*split[n][i];
				       // sum ( epsil * L *split ) see TLS notes
				}
			}
		}
	}
	Smatrix = Smat + temp;
}

void TLS::epsil(const short &i, const short &j,const short &k, short &ep){
	//antiSym tensor Epsilon_(i,j,k)
	//		= +1, even perms of 0,1,2
	//		= -1, odd perms 
	//		= 0 for repeated index, eg 0,0,k 
		if( (i==0 && j==1 && k==2) 
		 || (i==1 && j==2 && k==0)
		 || (i==2 && j==0 && k==1) )
			ep = 1;		
		else if( (i==1 && j==0 && k==2) 
			  || (i==2 && j==1 && k==0)
			  || (i==0 && j==2 && k==1) )
			ep = -1;
		else if (i==j || i==k || k==j )
			ep = 0;  // returns zero eg for i==j > 2;
		else
			throw TInvalidArgumentException(__OlxSourceInfo, 
			"epsilon_i,j,k not permutation of 0,1,2"); 
	}
void TLS::BondCorrect(TSAtom *atom1, TSAtom* atom2, double &bondlength,double &error){
	evecd vec(3);
	vec.Assign(atom1->crd(), 3);
	evecd vec2(3);
	vec2.Assign(atom2->crd(), 3); 
	vec = vec - vec2; //bond wrt Cartesian axes
	vec = RtoLaxes * vec; //bond wrt L axes
	bondlength = 0;
	for (int i = 0; i<3; i++){
		bondlength = bondlength + vec[i]*vec[i];
		vec[i] = sqrt ( vec[i]*vec[i] ); //set vec[i] to |vec[i]| for calc below
	}
	bondlength = sqrt(bondlength); // uncorrected bondlength
	
	 //TLS correction
	bondlength = bondlength	+ vec[0]*(Lmat[1][1]+Lmat[2][2])/2
							+ vec[1]*(Lmat[0][0]+Lmat[2][2])/2
							+ vec[2]*(Lmat[1][1]+Lmat[0][0])/2;

	error = 0. ; //Propagate error from error on L
	evecd dBdL(3); //derivative: d BondCorrection / d L[i][i], i = 1,2,3
	dBdL[0] = (1./2.)* ( vec[1] + vec[2] );
	dBdL[1] = (1./2.)* ( vec[0] + vec[2] );
	dBdL[2] = (1./2.)* ( vec[1] + vec[0] );

	for(short i=0; i<3; i++)
		for(short j=0; j<3; j++)
			error = error + dBdL[i]*dBdL[j]*LVcV[i][j]; 
	
	error = sqrt(error );
}
void TLS::extrapolate(TSAtom *atom, TEllpList &Elps) {
	evecd position(3);
	for (int ii = 0; ii < 3 ; ii++)
		position[ii] = (atom->crd()[ii]); // wrt Cartesian
		
	position =  RtoLaxes * position;  // = R.x in Olex, position wrt Laxes
	
	ematd UtlsLaxes(3,3); 
	calcUijCart(UtlsLaxes,position); 

	ematd RtoLaxesInv = RtoLaxes;  //inverse
	RtoLaxesInv.Transpose(); //inverse of Orthog matrix is the transpose
	
	ematd UtlsCell(3,3);
	UtlsCell = adpScaleInv * Rcart *RtoLaxesInv * UtlsLaxes * RtoLaxes* RcartT * adpScaleInv ; 

	//Add to ellipse list
	evecd vec(6);
	vec[0]=	UtlsCell[0][0];
	vec[1]=	UtlsCell[1][1];
	vec[2]=	UtlsCell[2][2];
	vec[3]=	UtlsCell[2][1];
	vec[4]=	UtlsCell[2][0];
	vec[5]=	UtlsCell[0][1];
	
	Elps.AddNew( vec) ;
}
void TLS::tlsToShelx(short &i, short &p){
	//recall TLS elements order:  
	//(t11, t12,t22,t13,t23,t33,   s11,s12,s13,	   l11,s21,s22,s33,    l12,l22,s31,s32,s33,	    l13,l23,l33) 
	//	0	1	2	3	4	5		6	7	8		9	10	11	12		13	14	15	16	17		18	19	20			
	
	//shelx order: l11,l22,l33,l32,l31,l21
	switch(i){
				case 0: 
					p=9; //l11
					break;
				case 1:
					p=14; //l22
					break;
				case 2:
					p=20; //l33			
					break;
				case 3:
					p=19;
					break;
				case 4:
					p=18;
					break;
				case 5:
					p=13;
					break;
				default:
					throw TInvalidArgumentException(__OlxSourceInfo, 
						"TLS parameters (l11,l22,l33,l23,l13,l12 etc) conversion to shelx convention: failed"); 	
	}
}
