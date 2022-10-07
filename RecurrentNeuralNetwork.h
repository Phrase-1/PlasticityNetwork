#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>
#include"SynapticPlasticityNetwork2.h"
//使用有向图表示神经网络
//行代表神经元，列代表与其连接的其他神经元
//0-3为输入 7-8为输出   其余为隐含
typedef struct RecurrentNeuralNetwork{
    float Neurons[9];
    float WeightMatrix[9][9];
    float BiasMatrix[9][9];
    int ConnectionMatrix[9][9];
    float InputWeights[4];
}RecurrentNeuralNetwork;

void InitiateRecurrentNeuralNetwork(RecurrentNeuralNetwork *rnn, Genome g){
    int geneindex = 0;
    for(int i = 0; i < 4; i++){
        rnn->InputWeights[i] = g.gene_info[geneindex];
        geneindex++;
    }
    geneindex = 60;
    for(int i = 0; i < 9; i++){
        rnn->Neurons[i] = 0;
        for(int j = 0; j < 9; j++){
            rnn->ConnectionMatrix[i][j] = g.gene_info[geneindex];
            geneindex++;
            rnn->WeightMatrix[i][j] = 0;
            rnn->BiasMatrix[i][j] = 0;
        }
    }
}
//大脑接收输入
void BrainReceiveInput(RecurrentNeuralNetwork *rnn ,int preaction1, int preaction2, float *sensoryarray){
    rnn->Neurons[0] = preaction1;
    rnn->Neurons[1] = preaction2;
    rnn->Neurons[2] = sensoryarray[0]*rnn->InputWeights[2];
    rnn->Neurons[3] = sensoryarray[1]*rnn->InputWeights[3];
}

void BrainFeedForward(RecurrentNeuralNetwork *rnn){
    float temp  = 0;
    //x遍历9个神经元，y遍历其上的连接
    for(int x = 0; x < 9; x++){
        temp = 0;
        for(int y = 0; y < 9; y++){
            if(rnn->ConnectionMatrix[x][y]==1 && x!=y){//若x y之间有连接则做运算
                temp += (rnn->Neurons[y]*rnn->WeightMatrix[x][y] + rnn->BiasMatrix[x][y]);
            }
        }
        rnn->Neurons[x] = temp;
        rnn->Neurons[x] = Sigmoid(rnn->Neurons[x]);//激活
    }
    for(int i = 7; i < 9; i++){
        if(rnn->Neurons[i] <= 0.5){
            rnn->Neurons[i] = 0;
        }
        else{
            rnn->Neurons[i] = 1;
        }
    }
}

void UpdateRecurrentBrainNetwork(RecurrentNeuralNetwork *rnn, PlasticityNetwork *pn){
    double update_w_b[2];
    for(int i = 0; i < 9; i++){
        for(int j = 0; j < 9; j++){
            if(i!=j && rnn->ConnectionMatrix[i][j]==1){
                pn->neuron_L1[0] = rnn->WeightMatrix[i][j];
                pn->neuron_L1[1] = rnn->Neurons[i];
                pn->neuron_L1[2] = rnn->Neurons[j];
                pn->neuron_L1[3] = rnn->BiasMatrix[i][j];
                pn->neuron_L1[4] = 0;
                PlasticityFeedForward(pn, update_w_b);
                rnn->WeightMatrix[i][j] += update_w_b[0];
                rnn->BiasMatrix[i][j] += update_w_b[1];
            }
        }
    }
}