#ifndef _TEST_H_
#define _TEST_H_


#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>
//#include"GeneticAlgorithm2.h"
#include"SynapticPlasticityNetwork.h"

typedef struct NeuralNetwork{

    float w_L1_L2[4][3];//一二层之间的连接权重矩阵
    float b_L1_L2[4][3];//一二层间bias矩阵
    float w_L2_L3[3][2];//二三层间连接权重矩阵
    float b_L2_L3[3][2];//二三层间bias矩阵
    int conn_matrix_L2_L3[3][2];//二三层间的连接矩阵，取值0或1
    float neuron_L1[4];//第一层 四个input
    float neuron_L2[3];//3个隐藏层
    float neuron_L3[2];//两个输出
}NeuralNetwork;
//sigmoid
//activation function
// double Sigmoid(double x){
//     return 1 / (1+exp(-x));
// }

void InitiateNeuralNetwork(NeuralNetwork *NN, Genome g){

    
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 3; j++){
            //此处应由遗传算法随机生成，并保持不变
            NN->b_L1_L2[i][j] = g.brain_input_bias[i][j];//-1+1.0*(rand()%RAND_MAX)/RAND_MAX *(1-(-1));
            NN->w_L1_L2[i][j] = g.brain_input_weight[i][j];//-1+1.0*(rand()%RAND_MAX)/RAND_MAX *(1-(-1));
        }
    }
    //由遗传算法初始化，后续由plasticity network更新
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 2; j++){
            NN->b_L2_L3[i][j] = g.brain_initial_conn_bias[i][j];//-1+1.0*(rand()%RAND_MAX)/RAND_MAX *(1-(-1));
            NN->w_L2_L3[i][j] = g.brain_initial_conn_weight[i][j];//-1+1.0*(rand()%RAND_MAX)/RAND_MAX *(1-(-1));
        }
    }
    //由遗传算法初始化连接矩阵
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 2; j++){
            NN->conn_matrix_L2_L3[i][j] = g.brain_conn_matrix_L2_L3[i][j];
        }
    }

    //重置为0
    for(int i = 0; i < 4; i++){
        NN->neuron_L1[i] = 0;
    }
    for(int i = 0; i < 3; i++){
        NN->neuron_L2[i] = 0;
    }
    for(int i = 0; i < 2; i++){
        NN->neuron_L3[i] = 0;
    }

}

//compute the output
void BrainFeedForward(NeuralNetwork *NN){//, float *outputs){

    //计算第二层
    for(int i = 0; i < 3; i++){
        NN->neuron_L2[i] = 0;//每次计算前要先重置为0
        for(int j = 0; j < 4; j++){
            NN->neuron_L2[i]+=(NN->neuron_L1[j]*NN->w_L1_L2[j][i] + NN->b_L1_L2[j][i]);
        }
        NN->neuron_L2[i]=Sigmoid(NN->neuron_L2[i]);
    }
    //计算第三层
    for(int i = 0; i < 2; i++){
        NN->neuron_L3[i] = 0;//每次计算前要先重置为0
        for(int j = 0; j < 3; j++){
            NN->neuron_L3[i] += ((NN->neuron_L2[j]*NN->w_L2_L3[j][i] + NN->b_L2_L3[j][i])*NN->conn_matrix_L2_L3[j][i]);
        }
        NN->neuron_L3[i] = Sigmoid(NN->neuron_L3[i]);
        if(NN->neuron_L3[i]>=0.5){
            NN->neuron_L3[i]=1;
        } 
        else {
            NN->neuron_L3[i] = 0;
        }
    }
    
}

void ShowResult(NeuralNetwork nn){
    printf("hidden0=%lf\n",nn.neuron_L2[0]);
    printf("hidden1=%lf\n",nn.neuron_L2[1]);
    printf("hidden2=%lf\n",nn.neuron_L2[2]);
    printf("output0=%lf\n",nn.neuron_L3[0]);
    printf("output1=%lf\n",nn.neuron_L3[1]);
}

void UpdateBrainNetwork(NeuralNetwork *nn, PlasticityNetwork *pn){
    //double delta_w_b[2];
    //PlasticityUpdate(delta_w_b);
    double update_w_b[2];

    //更新一二层
    // for(int i = 0; i < 4; i++){
    //     for(int j = 0; j < 3; j++){
    //         pn->neuron_L1[0] = nn->w_L1_L2[i][j];
    //         pn->neuron_L1[1] = nn->neuron_L1[i];
    //         pn->neuron_L1[2] = nn->neuron_L2[j];
    //         pn->neuron_L1[3] = nn->b_L1_L2[i][j];
    //         pn->neuron_L1[4] = 0;
    //         PlasticityFeedForward(pn, update_w_b);
    //         nn->w_L1_L2[i][j] += update_w_b[0];
    //         nn->b_L1_L2[i][j] += update_w_b[1];
    //     }
    // }
    //更新二三层
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 2; j++){
            pn->neuron_L1[0] = nn->w_L2_L3[i][j];
            pn->neuron_L1[1] = nn->neuron_L2[i];
            pn->neuron_L1[2] = nn->neuron_L3[j];
            pn->neuron_L1[3] = nn->b_L2_L3[i][j];
            pn->neuron_L1[4] = 0;
            PlasticityFeedForward(pn, update_w_b);
            nn->w_L2_L3[i][j] += update_w_b[0];
            nn->b_L2_L3[i][j] += update_w_b[1];
        }
    }

}

#endif