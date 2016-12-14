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
    int** scoringMatrix = make2DIntArray(seqY.length()+1, seqX.length()+1);
    
    for(int i = 0; i < seqX.length()+1; i++){
        scoringMatrix[0][i] = 1;
    }
    for(int i = 1; i < seqY.length()+1; i++){
        scoringMatrix[i][0] = 1;
    }
    for(int j = 1; j < seqY.length()+1; j++){
        for(int i = 1; i < seqX.length()+1; i++){
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
// int** matchMatrix,
//returns scoring Matrix for each best alignment
//int** score(string seqX, string seqY, int gap_open, int gap_add, int mismatch){
//    int** scoringMatrix = make2DIntArray(seqY.length()+1, seqX.length()+1);
//    
//    scoringMatrix[0][0] = 0;
//    
//    for(int i = 1; i < seqX.length(); i++){
//        scoringMatrix[0][i] = scoringMatrix[0][i-1] - 1;
//    }
//    for(int i = 1; i < seqY.length(); i++){
//        scoringMatrix[i][0] = scoringMatrix[i-1][0] - 1;
//    }
//    for(int j = 1; j < seqY.length(); j++){
//        for(int i = 1; i < seqX.length(); i++){
//            int max = scoringMatrix[j-1][i-1];
//            int gap_tally = 0;
//            int gap_tallyX = 0;
//            int gap_tallyY = 0;
//            for(int h = i-2; h > -1; h--){
//                gap_tallyX++;
//                if(scoringMatrix[j-1][h]>max){
//                    max = scoringMatrix[j-1][h];
//                    gap_tally = gap_tallyX;
//                }
//                
//            }
//            for(int v = j-2; v > -1; v--){
//                gap_tallyY++;
//                if(scoringMatrix[v][i-1] > max){
//                    max = scoringMatrix[v][i-1];
//                    gap_tally = gap_tallyY;
//                }
//            }
//            
//            if (seqX[i-1] == seqY[j-1]){
//                scoringMatrix[j][i] = 1 + max - gap_tally;
//            }
//            else{
//                scoringMatrix[j][i] = -1 + max - gap_tally;
//            }
//        }
//    }
//    return scoringMatrix;
//}

int** score1(string seqX, string seqY, int match, int gap_open, int gap_add, int mismatch){
    int** scoringMatrix = make2DIntArray(seqY.length()+1, seqX.length()+1);
    
    scoringMatrix[0][0] = 0;
    scoringMatrix[0][1] = gap_open;
    scoringMatrix[1][0] = gap_open;
    
    for(int i = 2; i < seqX.length() + 1; i++){
        scoringMatrix[0][i] = scoringMatrix[0][i-1] + gap_add;
    }
    for(int i = 2; i < seqY.length() + 1; i++){
        scoringMatrix[i][0] = scoringMatrix[i-1][0] + gap_add;
    }
    for(int j = 1; j < seqY.length()+1; j++){
        for(int i = 1; i < seqX.length()+1; i++){
            int max = scoringMatrix[j-1][i-1];
            int gap_tally = 0;
            int gap_tallyX = 0;
            int gap_tallyY = 0;
            for(int h = i-2; h > -1; h--){
                gap_tallyX++;
                if(scoringMatrix[j-1][h]>max){
                    max = scoringMatrix[j-1][h];
                    gap_tally = gap_tallyX;
                }
                
            }
            for(int v = j-2; v > -1; v--){
                gap_tallyY++;
                if(scoringMatrix[v][i-1] > max){
                    max = scoringMatrix[v][i-1];
                    gap_tally = gap_tallyY;
                }
            }
            
//            if (seqX[i-1] == seqY[j-1] && seqX[i-1] == 'A'){
//                if(gap_tally > 0){
//                    scoringMatrix[j][i] = 100 + max - (gap_tally*gap_add) + gap_open;
//                }
//                else{
//                    scoringMatrix[j][i] = 100 + max;
//                }
//            }
//            else
            if (seqX[i-1] == seqY[j-1]){
                if(gap_tally > 0){
                    scoringMatrix[j][i] = match + max - (gap_tally*gap_add) + gap_open;
                }
                else{
                    scoringMatrix[j][i] = match + max;
                }
            }
            else{
                if(gap_tally > 0){
                    scoringMatrix[j][i] = mismatch + max - (gap_tally*gap_add) + gap_open;
                }
                else{
                    scoringMatrix[j][i] = mismatch + max;
                }
            }
        }
    }
    return scoringMatrix;
}


int main(int argc, const char * argv[]) {

    string seqX = "CGATTCGACCAG";
    string seqY = "TGAATCCG";
    
    int** matchMatrix = make2DIntArray(4,4); // ACGT*ACGT

    
    int** pos_align_Matrix = possibleAlignments(seqX, seqY);
    cout << pos_align_Matrix[seqX.length()][seqY.length()] << endl; //prints the maximum of alignments
    
    int** scoringMatrix = score1(seqX, seqY, 2, -10, 0, 0);
    
    
    for(int j = 0; j < seqY.length()+1; j++){
        for(int i = 0; i < seqX.length()+1; i++){
            cout << scoringMatrix[j][i] << ' ';
            if(i == seqX.length()  && j == seqY.length())
                cout << endl << scoringMatrix[j][i] << ' ';
        }
        cout << endl;
    }
}
