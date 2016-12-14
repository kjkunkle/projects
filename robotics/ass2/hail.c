/**************************************************************************
*  Hailstone Numbers
*  Pauses for 2 seconds then prints each hailstone integer to the debug stream.
*  Hailstone sequence terminates when n = 1
*  
***************************************************************************/

//take an integer and return the next integer in the hailstone sequence
int hailstone(int n){
	if(n % 2 == 1){ // if n is odd
		return 3*n + 1;
  	}
	else{ // if n is even
		return n/2;
  	}
}


task main(){

  	wait1Msec(2000); //Waits 2 seconds

  	int i = 1; //iterator
  	int n = 7; //beginning of hailstone sequence

	//loop until hailstone sequence reaches n = 1
	while(n != 1){
		//write each hailstone integer to debug stream
		writeDebugStream("Hailstone #%d: %d\n", i, n); 
  		n = hailstone(n); //update n to next hailstone integer
  		i++; //iterate
	}
	
	writeDebugStream("Hailstone #%d: %d\n", i, n); 
}
