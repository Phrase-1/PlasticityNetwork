#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>
#include<GeneticAlgorithm2.h>
#define BIAS_NUMBER 18
#define NEURON_NUMBER 9
#define CONNECTION_NUMBER 18
#define INPUT_NUMBER 4
typedef struct Neuron{

}Neuron;
typedef struct PlasticityNetwork{
    double w_L1_L2[5][4];//一二层之间的连接矩阵
    double b_L1_L2[5][4];//一二层间bias矩阵
    double w_L2_L3[4][2];//二三层间连接矩阵
    double b_L2_L3[4][2];//二三层间bias矩阵
    double neuron_L1[5];//第一层 四个input
    double neuron_L2[4];//3个隐藏层
    double neuron_L3[2];//两个输出
}PlasticityNetwork;
double Sigmoid(double x){
    return 1 / (1+exp(-x));
}
//根据基因组信息进行初始化
void InitializePlasticityNet(PlasticityNetwork *pn, Genome g){
    for(int i = 0; i < 5; i++){
        for(int j = 0; j < 4; j++){
            pn->w_L1_L2[i][j] = g.plasticity_L1L2_weight[i][j];
            pn->b_L1_L2[i][j] = g.plasticity_L1L2_bias[i][j];
        }
    }
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 2; j++){
            pn->w_L2_L3[i][j] = g.plasticity_L2L3_weight[i][j];
            pn->b_L2_L3[i][j] = g.plasticity_L2L3_bias[i][j];
        }
    }
    //不使用hidden variable的情况
    for(int j = 0; j < 4; j++){
        pn->w_L1_L2[4][j] = 0;
        pn->b_L1_L2[4][j] = 0;
    }
    for(int i = 0; i < 5; i++){
        pn->neuron_L1[i] = 0;
    }
    for(int i = 0; i < 4; i++){
        pn->neuron_L2[i] = 0;
    }
    for(int i = 0; i < 2; i++){
        pn->neuron_L3[i] = 0;
    }
}
//将输出映射到[-0.03, 0.03]
void PlasticityFeedForward(PlasticityNetwork *pn, double *update){
    //计算第二层
    for(int i = 0; i < 4; i++){
        pn->neuron_L2[i] = 0;//每次计算前要先重置为0
        for(int j = 0; j < 5; j++){
            pn->neuron_L2[i] += (pn->neuron_L1[j]*pn->w_L1_L2[j][i] + pn->b_L1_L2[j][i]);
        }
        pn->neuron_L2[i] = Sigmoid(pn->neuron_L2[i]);
    }
    //计算第三层
    for(int i = 0; i < 2; i++){
        pn->neuron_L3[i] = 0;
        for(int j = 0; j < 4; j++){
            pn->neuron_L3[i] += (pn->neuron_L2[j]*pn->w_L2_L3[j][i] + pn->b_L2_L3[j][i]);
        }
        pn->neuron_L3[i] = Sigmoid(pn->neuron_L3[i]);
    }
    //输出0为delta_w,输出1为delta_b
    //将输出线性映射到[-0.03, 0.03]
    //y = 0.006*x - 0.003
    update[0] = (0.006*pn->neuron_L3[0]) - 0.003;
    update[1] = (0.006*pn->neuron_L3[1]) - 0.003;
}