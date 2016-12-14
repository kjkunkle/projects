//
//  main.cpp
//  bio lab 4
//
//  Created by Kevin Kunkle on 10/14/15.
//  Copyright © 2015 Kevin Kunkle. All rights reserved.
//

#include <iostream>
#include <string.h>
#include <map>

using namespace std;

int** make2DIntArray(int sizeY, int sizeX) {
    int** array;
    array = (int**) calloc(sizeY, sizeof(int*));
    for (int i = 0; i < sizeY; i++){
        array[i] = (int*) calloc(sizeX, sizeof(int));
    }
    return array;
}

//finds all possible alignments and returns a matrix
//of the number of paths leading to each point
int** possibleAlignments(string seqX, string seqY){
    int** scoringMatrix = make2DIntArray(seqY.length(), seqX.length());
    
    for(int i = 0; i < seqX.length(); i++){
        scoringMatrix[0][i] = 1;
    }
    for(int i = 1; i < seqY.length(); i++){
        scoringMatrix[i][0] = 1;
    }
    for(int j = 1; j < seqY.length(); j++){
        for(int i = 1; i < seqX.length(); i++){
            scoringMatrix[j][i] = scoringMatrix[j-1][i-1];
            for(int h = i-2; h > -1; h--){
                scoringMatrix[j][i] += scoringMatrix[j-1][h];
            }
            for(int v = j-2; v > -1; v--){
                scoringMatrix[j][i] += scoringMatrix[v][i-1];
            }
        }
    }
    return scoringMatrix;
}

//returns scoring Matrix for each best alignment
int** score(string seqX, string seqY, int** matchMatrix, int gap_open, int gap_add, int mismatch){
    int** scoringMatrix = make2DIntArray(seqY.length(), seqX.length());
    
    for(int i = 0; i < seqX.length(); i++){
        scoringMatrix[0][i] = 1;
    }
    for(int i = 1; i < seqY.length(); i++){
        scoringMatrix[i][0] = 1;
    }
    for(int j = 1; j < seqY.length(); j++){
        for(int i = 1; i < seqX.length(); i++){
            scoringMatrix[j][i] = scoringMatrix[j-1][i-1];
            for(int h = i-2; h > -1; h--){
                scoringMatrix[j][i] += scoringMatrix[j-1][h];
            }
            for(int v = j-2; v > -1; v--){
                scoringMatrix[j][i] += scoringMatrix[v][i-1];
            }
        }
    }
    return scoringMatrix;\\\\
}



int main(int argc, const char * argv[]) {

    string seqX = "CCGGCCACC";
    string seqY = "CGCTAAT";
    
    int** matchMatrix = make2DIntArray(4,4); // ACGT*ACGT

    
    int** scoringMatrix = possibleAlignments(seqX, seqY);
    
    
    
    for(int j = 0; j < seqY.length(); j++){
        for(int i = 0; i < seqX.length(); i++){
            cout << scoringMatrix[j][i];
            if(i == seqX.length() -1  && j == seqY.length() - 1)
                cout << endl << scoringMatrix[j][i];
        }
        cout << endl;
    }
}
