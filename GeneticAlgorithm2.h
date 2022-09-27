#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<time.h>

#define GENE_NUMBER 20
#define MAX_GENERATION 50
#define CROSSP 0.9
#define MUTATIONP 0.01
#define MIN_NUMBER 0//取值区间左端
#define MAX_NUMBER 15//取值区间右端

typedef struct Genome
{
    double brain_input_weight[4][3];//1 2层之间
    double brain_input_bias[4][3];
    double brain_initial_conn_weight[3][2];//2 3层之间
    double brain_initial_conn_bias[3][2];
    double plasticity_L1L2_weight[5][4];
    double plasticity_L1L2_bias[5][4];
    double plasticity_L2L3_weight[4][2];
    double plasticity_L2L3_bias[4][2];
    int brain_conn_matrix_L2_L3[3][2];
    double gene_info[98];//以上信息按顺序写入此数组，作为基因
    int fitness;
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

void InitGenome(Genome *geno){
    int gene_index = 0;
    //geno->fitness = 0;

    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 3; j++){
            geno->brain_input_weight[i][j] = randfloat(-3,3);
            geno->gene_info[gene_index] = geno->brain_input_weight[i][j];
            gene_index++;
        }
    }
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 3; j++){
            geno->brain_input_bias[i][j] = randfloat(-3,3);
            geno->gene_info[gene_index] = geno->brain_input_bias[i][j];
            gene_index++;
        }
    }
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 2; j++){
            geno->brain_initial_conn_weight[i][j] = randfloat(-3,3);
            geno->gene_info[gene_index] = geno->brain_initial_conn_weight[i][j];
            gene_index++;
        }
    }
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 2; j++){
            geno->brain_initial_conn_bias[i][j] = randfloat(-3,3);
            geno->gene_info[gene_index] = geno->brain_initial_conn_bias[i][j];
            gene_index++;
        }
    }
    for(int i = 0; i < 5; i++){
        for(int j = 0; j < 4; j++){
            geno->plasticity_L1L2_weight[i][j] = randfloat(-3,3);
            geno->gene_info[gene_index] = geno->plasticity_L1L2_weight[i][j];
            gene_index++;
        }
    }
    for(int i = 0; i < 5; i++){
        for(int j = 0; j < 4; j++){
            geno->plasticity_L1L2_bias[i][j] = randfloat(-3,3);
            geno->gene_info[gene_index] = geno->plasticity_L1L2_bias[i][j];
            gene_index++;
        }
    }
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 2; j++){
            geno->plasticity_L2L3_weight[i][j] = randfloat(-3,3);
            geno->gene_info[gene_index] = geno->plasticity_L2L3_weight[i][j];
            gene_index++;
        }
    }
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 2; j++){
            geno->plasticity_L2L3_bias[i][j] = randfloat(-3,3);
            geno->gene_info[gene_index] = geno->plasticity_L2L3_bias[i][j];
            gene_index++;
        }
    }
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 2; j++){
            geno->brain_conn_matrix_L2_L3[i][j] = rand() % 2;//0或1
            geno->gene_info[gene_index] = geno->brain_conn_matrix_L2_L3[i][j];
            gene_index++;
        }
    }
    //-3+1.0*(rand()%RAND_MAX)/RAND_MAX *(3-(-3));
}

void InitPopulation(Genome *Population){
    for(int i = 0; i < GENE_NUMBER; i++){
        InitGenome(&Population[i]);
        Population[i].fitness = randint(1,100);
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

//选择算子
//stochastic tournament model
//随机联赛选择
void SelectPopulation(Genome *Population){
    int index=0;
    Genome NewPopulation[GENE_NUMBER];
    //从前到后两两比较，比较GENE_NUMBER次，保证下一代种群数量不变
    for (int i = 0; i < GENE_NUMBER; i++)
    {
        if(Population[i].fitness>Population[i+1].fitness){
            NewPopulation[index] = Population[i];
            i++;index++;
        }
        else{
            NewPopulation[index] = Population[i+1];
            i++;index++;
        }
    }
    printf("\nindex=%d\n",index);
    //前后两两比较
    for(int i = 0, j = GENE_NUMBER-1; i <j; i++,j--){
        if(Population[i].fitness > Population[j].fitness){
            NewPopulation[index] = Population[i];
            index++;
        }
        else{
            NewPopulation[index] = Population[j];
            index++;
        }
    }
    printf("\nindex=%d\n",index);
    //新种群替换旧种群
    for(int i = 0; i < GENE_NUMBER; i++){
        Population[i] = NewPopulation[i];
    }

}

//交叉算子
//从前到后两两配对
//交换交叉点后的基因
void CrossPopulation(Genome *Population){
    for(int i = 0; i < GENE_NUMBER; i+=2){
        if(rand()%100 < 90){//以90%概率执行交叉操作
            int cross_point = randint(0,97);
            float info_temp;
            //交换基因
            for(int j = cross_point; j < 98; j++){
                info_temp = Population[i].gene_info[j];
                Population[i].gene_info[j] = Population[i+1].gene_info[j];
                Population[i+1].gene_info[j] = info_temp;
            }
        }
        else{
            continue;
        } 
    }
    //将geneinfo解码到各分组
    int gene_index;
    for (int x = 0; x < GENE_NUMBER; x++)
    {
        gene_index = 0;
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                Population[x].brain_input_weight[i][j] = Population[x].gene_info[gene_index];
                gene_index++;
            }
        }
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                Population[x].brain_input_bias[i][j] = Population[x].gene_info[gene_index];
                gene_index++;
            }
        }
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 2; j++)
            {
                Population[x].brain_initial_conn_weight[i][j] = Population[x].gene_info[gene_index];
                gene_index++;
            }
        }
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 2; j++)
            {
                Population[x].brain_initial_conn_bias[i][j] = Population[x].gene_info[gene_index];
                gene_index++;
            }
        }
        for (int i = 0; i < 5; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                Population[x].plasticity_L1L2_weight[i][j] = Population[x].gene_info[gene_index];
                gene_index++;
            }
        }
        for (int i = 0; i < 5; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                Population[x].plasticity_L1L2_bias[i][j] = Population[x].gene_info[gene_index];
                gene_index++;
            }
        }
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 2; j++)
            {
                Population[x].plasticity_L2L3_weight[i][j] = Population[x].gene_info[gene_index];
                gene_index++;
            }
        }
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 2; j++)
            {
                Population[x].plasticity_L2L3_bias[i][j] = Population[x].gene_info[gene_index];
                gene_index++;
            }
        }
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 2; j++)
            {
                Population[x].brain_conn_matrix_L2_L3[i][j] = Population[x].gene_info[gene_index];
                gene_index++;
            }
        }
    }

}


//变异算子
void MutatePopulation(Genome *Population){
    int mutate_point;
    float mutate_quantity;
    for(int i = 0; i < GENE_NUMBER; i++){
        if(rand()%100 < 1){//以0.01%概率变异    
            mutate_point = randint(0,91);////在连接矩阵部分前随机挑选一个变异点
            mutate_quantity = randfloat(-1,1);//在-1到1之间随机变化
            Population[i].gene_info[mutate_point] += mutate_quantity;

        }
        else{
            continue;
        }
        if(rand()%100 < 1){
            mutate_point = randint(92,97);//在连接矩阵部分随机变异
            if(Population[i].gene_info[mutate_point] == 1){
                Population[i].gene_info[mutate_point] = 0;
            }
            else{
                Population[i].gene_info[mutate_point] = 1;
            }
        }
        else{
            continue;
        }
    }
    //将geneinfo解码到各分组
    int gene_index;
    for (int x = 0; x < GENE_NUMBER; x++)
    {
        gene_index = 0;
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                Population[x].brain_input_weight[i][j] = Population[x].gene_info[gene_index];
                gene_index++;
            }
        }
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                Population[x].brain_input_bias[i][j] = Population[x].gene_info[gene_index];
                gene_index++;
            }
        }
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 2; j++)
            {
                Population[x].brain_initial_conn_weight[i][j] = Population[x].gene_info[gene_index];
                gene_index++;
            }
        }
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 2; j++)
            {
                Population[x].brain_initial_conn_bias[i][j] = Population[x].gene_info[gene_index];
                gene_index++;
            }
        }
        for (int i = 0; i < 5; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                Population[x].plasticity_L1L2_weight[i][j] = Population[x].gene_info[gene_index];
                gene_index++;
            }
        }
        for (int i = 0; i < 5; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                Population[x].plasticity_L1L2_bias[i][j] = Population[x].gene_info[gene_index];
                gene_index++;
            }
        }
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 2; j++)
            {
                Population[x].plasticity_L2L3_weight[i][j] = Population[x].gene_info[gene_index];
                gene_index++;
            }
        }
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 2; j++)
            {
                Population[x].plasticity_L2L3_bias[i][j] = Population[x].gene_info[gene_index];
                gene_index++;
            }
        }
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 2; j++)
            {
                Population[x].brain_conn_matrix_L2_L3[i][j] = Population[x].gene_info[gene_index];
                gene_index++;
            }
        }
    }
}


// void main()
// {
//     srand(time(0));
//     Genome g;
//     InitGenome(&g);
//     Genome Population[GENE_NUMBER];
//     InitPopulation(Population);
//     // for (int i = 0; i < GENE_NUMBER; i++)
//     // {
//     //     printf("%d ", Population[i].fitness);
//     // }
//     // printf("\n");
//     // for(int i = 0; i < GENE_NUMBER; i++){
//     //     for(int j = 0; j < 3; j++){
//     //         for(int k = 0; k < 2; k++){
//     //             printf("%d ",Population[i].brain_conn_matrix_L2_L3[j][k]);
//     //         }
//     //         printf("\n");
//     //     }
//     //     printf("\n");
//     // }

//     // printf("\n");
//     // printf("\n");
//     // for(int i = 0; i < GENE_NUMBER; i++){
//     //     printf("%d ",Population[i].fitness);
//     // }
//     // printf("\n");
//     // EvaluatePop(Population);
//     // SelectPopulation(Population);

//     // for(int i = 0; i < GENE_NUMBER; i++){
//     //     printf("%d ",Population[i].fitness);
//     // }
//     // printf("\n");
//     // for(int i = 0; i < 4; i++){
//     //     for(int j = 0; j < 3; j++){
//     //         printf("%lf ",Population[19].brain_input_weight[i][j]);
//     //     }
//     //     printf("\n");
//     // }
//     // printf("\n");
//     //     for(int i = 0; i < 4; i++){
//     //     for(int j = 0; j < 3; j++){
//     //         printf("%lf ",Population[17].brain_input_weight[i][j]);
//     //     }
//     //     printf("\n");
//     // }
//     // for (int i = 0; i < 4; i++)
//     // {
//     //     for (int j = 0; j < 3; j++)
//     //     {
//     //         printf("%lf ", g.brain_input_bias[i][j]);
//     //     }
//     //     printf("\n");
//     // }
//     // printf("\n");
//     // for(int i = 0; i < 3; i++){
//     //     for(int j = 0; j < 2; j++){
//     //         printf("%lf ", g.brain_initial_conn_weight[i][j]);
//     //     }
//     //     printf("\n");
//     // }
//     // printf("\n");
//     // for(int i = 0; i < 92; i++){
//     //     printf("%lf\n",g.gene_info[i]);
//     // }

//     CrossPopulation(Population);
//     printf("\n");
//     CrossPopulation(Population);
// }