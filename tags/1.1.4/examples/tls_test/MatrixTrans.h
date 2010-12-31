class TM { //helper class
public:
	static void o2a(const TMatrixD & om, ap::real_2d_array& a){ //olex to aglib matrix
		try {
			int const n =0; // dumy int, does nothing!

	//		std::cout << "om.Vectors: " << om.Vectors() <<
	//			"\na.gethighbound(n) - a.getlowbound(n) + 1: " <<
	//			a.gethighbound(n) - a.getlowbound(n) + 1 << std::endl; // debug statement
			
			if (om.Vectors() != a.gethighbound(n) - a.getlowbound(n) + 1)
				throw TFunctionFailedException(__OlxSourceInfo, 
				"Copy error: Olex matrix different size to aglib matrix"); 
		 
			for(int i = 0; i < om.Vectors(); i++) {
				for(int j = 0; j < om.Elements(); j++) {	
					a(i+1,j+1) = om[i][j]; // copy matrix 	
				}				
			}			
		
		} // try loop

		catch( TExceptionBase& exc )
		{
			printf("An exception occured: %s\n", EsdlObjectName(exc).c_str() );
			printf("details: %s\n", exc.GetFullMessage().c_str() );
		}
		printf("\n...");
		return;
	};

/*
static void a2o(const TMatrixD & om, ap::real_2d_array& a){ // aglib to olex matrix
		try {
			int const n =0; // dumy int, does nothing!

			std::cout << "om.Vectors: " << om.Vectors() <<
				"\na.gethighbound(n) - a.getlowbound(n) + 1: " <<
				a.gethighbound(n) - a.getlowbound(n) + 1 << std::endl; // debug statement
			
			if (om.Vectors() != a.gethighbound(n) - a.getlowbound(n) + 1)
				throw TFunctionFailedException(__OlxSourceInfo, 
				"Copy error: Olex matrix different size to aglib matrix"); 
		 
			for(int i = 0; i < om.Vectors(); i++) {
				for(int j = 0; j < om.Elements(); j++) {	
					a(i+1,j+1) = om[i][j]; // copy matrix 	
				}				
			}			
		
		} // try loop

		catch( TExceptionBase& exc )
		{
			printf("An exception occured: %s\n", EsdlObjectName(exc).c_str() );
			printf("details: %s\n", exc.GetFullMessage().c_str() );
		}
		printf("\n...");
		return;
	};
*/

};//TM helper class 

