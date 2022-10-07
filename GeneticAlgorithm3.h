#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<time.h>

#define GENE_NUMBER 960//不可低于32，否则选择算子将陷入死循环
#define MAX_GENERATION 200
#define CROSSP 0.9
#define MUTATIONP 0.01
#define MIN_NUMBER 0//取值区间左端
#define MAX_NUMBER 15//取值区间右端
#define TOURNAMENT_SIZE 32
#define GENE_LENGTH 141//由全部元素数量加和得出
#define BINARY_GENE_LENGTH 1281//二进制基因长度

typedef struct Genome
{
    // float brain_input_weight[4];//感知输入到大脑的权重0-3
    // float plasticity_L1L2_weight[5][4];4-23
    // float plasticity_L1L2_bias[5][4];24-43
    // float plasticity_L2L3_weight[4][2];44-51
    // float plasticity_L2L3_bias[4][2];52-59
    // int brain_connection_matrix[9][9];//i=j时赋0 60-140

    //若用二进制表示，则为60*20+81 = 1281个二进制数
    float gene_info[GENE_LENGTH];//以上信息按顺序写入此数组，作为基因
    float fitness;
    int once_ate;
}Genome;

//生成x到y的随机浮点数
float randfloat(int x, int y){
    return x+1.0*(rand()%RAND_MAX)/RAND_MAX *(y-(x));
}
//生成x到y的随机整数
int randint(int x, int y)
{
    int a, l;
    l = y - x + 1;
    a = x + rand() * l / 32768;
    return a;
}

void ConvertDtoB(float x, short *result){
    //转换单个浮点数为二进制
    //结果数组为逆序存放
    //result长度要求为20
    x = ((x-(-3))/6*pow(2,20));//浮点数转整数过程，解码时按相反顺序运算，会损失精度
    int p = x;
	int count=0;
	do{
        result[count] = (int)p%2;
		p=p/2;
        count++;
	}while(p!=0);//当商不为0时进行循环 
}
float ConvertBtoD(short *binary){
    float temp = 0;
    float result;
    for(int i = 0; i < 20; i++){
        temp += binary[i]*pow(2,i); 
    }
    result = (temp/pow(2,20)*6)+(-3);
    return result;
}
//将基因转化为二进制
void EncodeBinaryGene(Genome g, short *result){
    int gene_index = 0;
    short single_data[20] = {0};
    //编码前60个数据
    for(int i = 0; i < 60; i++){
        for(int x = 0; x < 20; x++){
            single_data[x] = 0;
        }
        ConvertDtoB(g.gene_info[i], single_data);
        for(int j = 0; j < 20; j++){
            result[gene_index] = single_data[j];
            gene_index++;
        }
    }
    //编码后面部分数据，连接矩阵部分
    short temp;
    for(int i = 60; i < 141; i++){
        temp = g.gene_info[i];
        result[gene_index] = temp;
        gene_index++;
    }
}

//将二进制基因转换回十进制
//存在转换不对的bug
void DecodeBinaryGene(Genome *g, short *binaryGene){
    short single_binary_data[20];
    int gene_index = 0;
    //解码前60个数据
    for(int i = 0; i < 60; i++){
        for(int j = 0; j < 20; j++){
            single_binary_data[j] = binaryGene[gene_index];
            gene_index++;
        }
        //至此得到单个数据的二进制编码
        g->gene_info[i] = ConvertBtoD(single_binary_data);
    }
    //解码连接矩阵部分
    for(int i = 60; i < 141; i++){
        g->gene_info[i] = binaryGene[gene_index];
        gene_index++;
    }
}


//基因组解码
//用于交叉变异操作之后
// void DecodeGene(Genome *geno){
//     int gene_index = 0;
//     for(int i = 0; i < 4; i++){
//         geno->brain_input_weight[i] = geno->gene_info[gene_index];
//         gene_index++;
//     }
//     for(int i = 0; i < 5; i++){
//         for(int j = 0; j < 4; j++){
//             geno->plasticity_L1L2_weight[i][j] = geno->gene_info[gene_index];
//             gene_index++;
//         }
//     }
//     for(int i = 0; i < 5; i++){
//         for(int j = 0; j < 4; j++){
//             geno->plasticity_L1L2_bias[i][j] = geno->gene_info[gene_index];
//             gene_index++;
//         }
//     }
//     for(int i = 0; i < 4; i++){
//         for(int j = 0; j < 2; j++){
//             geno->plasticity_L2L3_weight[i][j] = geno->gene_info[gene_index];
//             gene_index++;
//         }
//     }
//     for(int i = 0; i < 4; i++){
//         for(int j = 0; j < 2; j++){
//             geno->plasticity_L2L3_bias[i][j] = geno->gene_info[gene_index];
//             gene_index++;
//         }
//     }
//     for(int i = 0; i < 9; i++){
//         for(int j = 0; j < 9; j++){
//             geno->brain_connection_matrix[i][j] = geno->gene_info[gene_index];
//             gene_index++;
//         }
//     }
// }

// void InitGenome(Genome *geno){
//     geno->once_ate = 0;
//     int gene_index = 0;
//     for(int i = 0; i < 4; i++){
//         geno->brain_input_weight[i] = randfloat(-3,3);
//         geno->gene_info[gene_index] = geno->brain_input_weight[i];
//         gene_index++;
//     }
//     for(int i = 0; i < 5; i++){
//         for(int j = 0; j < 4; j++){
//             geno->plasticity_L1L2_weight[i][j] = randfloat(-3,3);
//             geno->gene_info[gene_index] = geno->plasticity_L1L2_weight[i][j];
//             gene_index++;
//         }
//     }
//     for(int i = 0; i < 5; i++){
//         for(int j = 0; j < 4; j++){
//             geno->plasticity_L1L2_bias[i][j] = randfloat(-3,3);
//             geno->gene_info[gene_index] = geno->plasticity_L1L2_bias[i][j];
//             gene_index++;
//         }
//     }
//     for(int i = 0; i < 4; i++){
//         for(int j = 0; j < 2; j++){
//             geno->plasticity_L2L3_weight[i][j] = randfloat(-3,3);
//             geno->gene_info[gene_index] = geno->plasticity_L2L3_weight[i][j];
//             gene_index++;
//         }
//     }
//     for(int i = 0; i < 4; i++){
//         for(int j = 0; j < 2; j++){
//             geno->plasticity_L2L3_bias[i][j] = randfloat(-3,3);
//             geno->gene_info[gene_index] = geno->plasticity_L2L3_bias[i][j];
//             gene_index++;
//         }
//     }
//     //初始化连接矩阵
//     for(int i = 0; i < 9; i++){
//         for(int j = 0; j < 9; j++){
//             if(i == j){
//                 geno->brain_connection_matrix[i][j] = 0;//神经元自身连接为0
//                 geno->gene_info[gene_index] = geno->brain_connection_matrix[i][j];
//                 gene_index++;
//             }
//             else{
//                 geno->brain_connection_matrix[i][j] = rand() % 2;//0或1
//                 geno->gene_info[gene_index] = geno->brain_connection_matrix[i][j];
//                 gene_index++;
//             }
//         }
//     }
// }

void InitGeno(Genome *geno){
    geno->once_ate = 0;
    int gene_index = 0;
    for(int i = 0; i < 60; i++){
        geno->gene_info[gene_index] = randfloat(-3,3);
        gene_index++;
    }
    //连接矩阵
    for(int i = 60; i < 141; i++){
        geno->gene_info[gene_index] = rand() % 2;
        gene_index++;
    }

}


void InitPopulation(Genome *Population){
    for(int i = 0; i < GENE_NUMBER; i++){
        InitGeno(&Population[i]);
        Population[i].fitness = 0;
    }
}

//冒泡排序，适应度高的个体靠前
void EvaluatePop(Genome *Population){
    Genome temp;
    for(int i = 0; i < GENE_NUMBER; i++){
        for(int j = 0; j < GENE_NUMBER-i-1; j++){
            if(Population[j].fitness < Population[j+1].fitness){
                temp = Population[j];
                Population[j] = Population[j+1];
                Population[j+1] = temp;
            }
        }
    }
}
//寻找最佳个体
//返回其下标
int FindBestIndividual(Genome *Population){
    int index = 0;
    for(int i = 1; i < GENE_NUMBER; i++){
        if(Population[index].fitness < Population[i].fitness){
            index = i;
        }
    }
    return index;
}
//选择算子
//stochastic tournament model
//随机联赛选择
void SelectPopulation(Genome *Population){
    //32锦标赛
    Genome NewPopulation[GENE_NUMBER];
    Genome temp;
    int randnum;
    int generated_times = 0, flag = 0;
    int tournament_member[TOURNAMENT_SIZE] = {0};
    int selected_number = 0;
    //进行960轮锦标赛
    for (int tournament_round = 0; tournament_round < GENE_NUMBER; tournament_round++)
    {
        generated_times = 0;flag = 0;
        //生成不重复随机数组
        while (1)
        {
            flag = 0;
            if (generated_times == TOURNAMENT_SIZE)
            {
                break;
            }
            randnum = rand() % GENE_NUMBER; //所有个体中随意抽签
            for (int i = 0; i < generated_times; i++)
            {
                if (randnum == tournament_member[i])
                {
                    flag = 1;
                }
            }
            if (flag != 1)
            {
                tournament_member[generated_times] = randnum;
                generated_times++;
            }
        }
        //到此已随机选出32个竞选者
        // for(int i = 0; i < TOURNAMENT_SIZE; i++){
        //     printf("%d ", tournament_member[i]);
        // }
        // printf("\n");
        //在一组32个中选择最好的
        temp = Population[tournament_member[0]];
        for(int i = 1; i < TOURNAMENT_SIZE; i++){
            if(Population[tournament_member[i]].fitness > temp.fitness){
                temp = Population[tournament_member[i]];
            }
        }
        NewPopulation[tournament_round] = temp;
    }
    //新种群替换旧种群
    for(int i = 0; i < GENE_NUMBER; i++){
        Population[i] = NewPopulation[i];
    }
}

// //交叉算子
// //从前到后两两配对
// //交换交叉点后的基因
// void CrossPopulation(Genome *Population){
//     for(int i = 0; i < GENE_NUMBER; i+=2){
//         if(rand()%100 < 90){//以90%概率执行交叉操作
//             int cross_point = randint(0,GENE_LENGTH-1);//随机选择交叉点
//             float info_temp;
//             //交换基因
//             for(int j = cross_point; j < GENE_LENGTH; j++){
//                 info_temp = Population[i].gene_info[j];
//                 Population[i].gene_info[j] = Population[i+1].gene_info[j];
//                 Population[i+1].gene_info[j] = info_temp;
//             }
//         }
//         else{
//             continue;
//         } 
//     }
// }

void BinaryCrossPopulation(Genome *Population){
    short binarygene1[BINARY_GENE_LENGTH],binarygene2[BINARY_GENE_LENGTH];
    for(int i = 0; i < GENE_NUMBER; i+=2){
        if(rand()%100 < 90){//以90%概率执行交叉操作
            int cross_point = randint(0,BINARY_GENE_LENGTH-1);//随机选择交叉点
            EncodeBinaryGene(Population[i],binarygene1);
            EncodeBinaryGene(Population[i+1],binarygene2);
            float info_temp;
            //交换基因
            for(int j = cross_point; j < BINARY_GENE_LENGTH; j++){
                info_temp = binarygene1[j];
                binarygene1[j] = binarygene2[j];
                binarygene2[j] = info_temp;
            }
            DecodeBinaryGene(&Population[i], binarygene1);
            DecodeBinaryGene(&Population[i+1], binarygene2);
        }
        else{
            continue;
        } 
    }
    //将geneinfo解码到各分组
    // for(int i = 0; i < GENE_NUMBER; i++){
    //     DecodeGene(&Population[i]);
    // }
}

//变异算子
// void MutatePopulation(Genome *Population){
//     int mutate_point;
//     float mutate_quantity;
//     for(int i = 0; i < GENE_NUMBER; i++){
//         if(rand()%100 < 1){//以1%概率变异    
//             mutate_point = randint(0,1199);////在连接矩阵部分前随机挑选一个变异点
//             mutate_quantity = randfloat(-2,2);//在-1到1之间随机变化
//             Population[i].gene_info[mutate_point] += mutate_quantity;

//         }
//         else{
//             continue;
//         }
//         if(rand()%100 < 1){
//             mutate_point = randint(60,140);//在连接矩阵部分随机变异
//             if(Population[i].gene_info[mutate_point] == 1){
//                 Population[i].gene_info[mutate_point] = 0;
//             }
//             else{
//                 Population[i].gene_info[mutate_point] = 1;
//             }
//         }
//         else{
//             continue;
//         }
//         // //将神经元自身的连接重置为0
//         // for(int x = 0; x < 9; x++){
//         //     for(int y = 0; y < 9; y++){
//         //         if(x == y){
//         //             Population[i].brain_connection_matrix[x][y] = 0;
//         //         }
//         //     }
//         // }
//         //解码基因组
//         // DecodeGene(&Population[i]);
//     }
// }

//二进制变异算子
//每个点位都有可能变异
void BinaryMutatePopulation(Genome *Population){
    int mutate_point;
    short binarygene[BINARY_GENE_LENGTH] = {0};
    for(int i = 0; i < GENE_NUMBER; i++){
        //数组置零
        for(int index = 0; index < BINARY_GENE_LENGTH; index++){
            binarygene[index] = 0;
        }
        EncodeBinaryGene(Population[i], binarygene);
        //遍历整条基因
        for(int index = 0; index < BINARY_GENE_LENGTH; index++){
            if(rand()%100 < 1){
                if(binarygene[index] == 1){binarygene[index] = 0;}
                else{binarygene[index] = 1;}
            }
        }
        DecodeBinaryGene(&Population[i], binarygene);
        // //将神经元自身的连接重置为0
        // for(int x = 0; x < 9; x++){
        //     for(int y = 0; y < 9; y++){
        //         if(x == y){
        //             Population[i].brain_connection_matrix[x][y] = 0;
        //         }
        //     }
        // }
        //解码基因组
        // DecodeGene(&Population[i]);
    }
}

