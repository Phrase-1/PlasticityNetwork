//#include"GeneticAlgorithm2.h"
//#include"PlasticyNetwork.h"
#include"NeuralNetwork.h"
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>
#include<windows.h>
#define MAP_SIZE 100
#define FOOD_NUMBER 50
#define ORIENT_UP 0
#define ORIENT_RIGHT 90
#define ORIENT_DOWN 180
#define ORIENT_LEFT 270
#define pi 3.1415926535898
#define DEC (pi/180) //弧长,sin(角度*DEC),角度=acos(x)/DEC
#define ORGAN_LIFE_SPAN 5000
#define DELIBRATION_STEP 10//最小为1
#define MOVING_COST -0.003
#define BRAIN_METABOLIC_COST -0.000005
#define LEARNING_COST -0.00002
#define FOOD_SUPPLY 1
typedef struct Point
{
    float x;
    float y;
}Point;
typedef struct Orientation{
    int orient_status;
}Orientation;
typedef struct Organism
{
    Point coordinate;
    Orientation orient;
    int is_alive;
    int eaten_number;
    float max_allowable_energy;//Fmax
    float brain_metabolism;
    float learning_cost;
    float moveing_cost;
    float current_energy;
    float surplus;
}Organism;
typedef struct Food
{
    Point coordinate;
    //被吃为1否则为0
    int isEaten;
}Food;
//在生物进食、移动、学习、生存的时候分别运行此函数，实时更新surplus
void UpdateOrganSurplus(Organism *o, float parameter){
    o->surplus += parameter;
}
//初始化与重置食物
//坐标范围为<0-99>
//貌似有重复生成的BUG
void InitFoods(Food *foods, Organism o){
    for(int i = 0; i < FOOD_NUMBER; i ++){
        foods[i].coordinate.x=foods[i].coordinate.y=0;
        foods[i].isEaten = 0;
    }
    //srand(time(NULL));
    int RandNum_x, RandNum_y;
    int generated_times = 0, flag = 0;
    //开始循环生成随机坐标
    while(1){
        flag = 0;
        if(generated_times == FOOD_NUMBER){
            break;
        }
        RandNum_x = (rand() % (MAP_SIZE - 1));
        RandNum_y = (rand() % (MAP_SIZE - 1));
        for(int i = 0; i <= generated_times; i++){
            //若生成坐标点与已有食物或生物冲突，则不生成
            if(((float)RandNum_x==foods->coordinate.x)&&((float)RandNum_y==foods->coordinate.y)){
                flag = 1;
            }
            if( ((float)RandNum_x==o.coordinate.x)&&((float)RandNum_y==o.coordinate.y)){
                flag = 1;
            }
        }
        if(flag != 1){
            foods[generated_times].coordinate.x = (float)RandNum_x;
            foods[generated_times].coordinate.y = (float)RandNum_y;
            generated_times++;

        }
    }

}
//初始化生物的坐标与朝向
void InitOrgan(Organism *o){
    o->coordinate.x = MAP_SIZE/2;
    o->coordinate.y = MAP_SIZE/2;
    o->orient.orient_status = ORIENT_UP;
    o->eaten_number = 0;
    //o->current_energy = 10;
    o->max_allowable_energy = ORGAN_LIFE_SPAN*0.02;
    //o->moveing_cost = 0.003;
    o->surplus = 0;
}
//生物进食
void OrganEatFood(Food *foods, Organism *o){
    for(int i = 0; i < FOOD_NUMBER; i++){
        //若生物与食物坐标相同，则吃掉该食物
        if(foods[i].coordinate.x==o->coordinate.x && foods[i].coordinate.y==o->coordinate.y && foods[i].isEaten==0){
            foods[i].isEaten = 1;
            o->eaten_number++;
            UpdateOrganSurplus(o, FOOD_SUPPLY);//吃到一个食物surplus+1
        }
    }
}
//边界检测
//坐标范围为<0-49>
//超出边界返回1否则0
int BorderDetection(int co_x, int co_y){
    if(co_x>MAP_SIZE-1 || co_x<0 || co_y>MAP_SIZE-1 || co_y<0){
        return 1;
    }
    else return 0;
}
//生物移动
//00不动01右转10左转11前进
void OrganMove(Organism *o, int command_code1, int command_code2){
    //左右前的操作均需参照当前生物体朝向
    //右转
    if(command_code1==0 && command_code2==1){
        UpdateOrganSurplus(o, MOVING_COST);
        if(o->orient.orient_status==ORIENT_LEFT){
            o->orient.orient_status = ORIENT_UP;
        }
        else{
            o->orient.orient_status+=90;
        }
    }
    //左转
    if(command_code1==1 && command_code2==0){
        UpdateOrganSurplus(o, MOVING_COST);
        if(o->orient.orient_status==ORIENT_UP){
            o->orient.orient_status = ORIENT_LEFT;
        }
        else{
            o->orient.orient_status-=90;
        }
    }
    //前进
    if(command_code1==1 && command_code2==1){
        UpdateOrganSurplus(o, MOVING_COST);
        //temp用于储存前进后的临时坐标
        float temp_x,temp_y;
        switch(o->orient.orient_status){
            case ORIENT_UP:
                temp_x = o->coordinate.x - 1;
                temp_y = o->coordinate.y;
                break;
            case ORIENT_RIGHT:
                temp_x = o->coordinate.x;
                temp_y = o->coordinate.y + 1;
                break;
            case ORIENT_DOWN:
                temp_x = o->coordinate.x + 1;
                temp_y = o->coordinate.y;
                break;
            case ORIENT_LEFT:
                temp_x = o->coordinate.x;
                temp_y = o->coordinate.y - 1;
                break;
        }
        //若不超出边界则更新位置
        if(!BorderDetection(temp_x, temp_y)){
            o->coordinate.x = temp_x;
            o->coordinate.y = temp_y;
        }
    }
    else{;}
}
//提示最近食物位置_笛卡尔坐标系
//轮流比较曼哈顿距离
//output_array最终输出最近的食物坐标在生物体的相对位置
//[5,12]代表右边5格下面12格
void Cartetian_Sensory(float *output_array, Food foods[], Organism o){
    output_array[0] = foods[0].coordinate.x;
    output_array[1] = foods[1].coordinate.y;
    float old_distance = (float)(abs)(o.coordinate.x-output_array[0])+(float)(abs)(o.coordinate.y-output_array[1]);
    float new_distance = 0;
    //查找最近食物的坐标
    for(int i = 1; i <= FOOD_NUMBER; i++){\
        //|生物体x-食物x|+|生物体y-食物y|
        new_distance = (float)(abs)(o.coordinate.x-foods[i].coordinate.x)+(float)(abs)(o.coordinate.y-foods[i].coordinate.y);
        if(new_distance < old_distance && foods[i].isEaten==0){
            output_array[0] = foods[i].coordinate.x;
            output_array[1] = foods[i].coordinate.y;
            old_distance = new_distance;
        }
    }
    //将最近食物坐标转化为egocentric坐标
    //食物坐标-生物坐标，并修改output_array
    output_array[0] -= o.coordinate.x;
    output_array[1] -= o.coordinate.y;

}
//打印场地，食物距离，行动步数
void PrintMap(Food foods[], Organism o, float sensory_array[], int action_steps, int generation, int individual)
{
    printf("this is %dth individual of %dth generation\n", individual, generation);
    int pointPrinted = 0;
    //x为行y为列
    // for (int x = 0; x < MAP_SIZE; x++)
    // {
    //     printf("%4drow ", x);
    //     //打印整个地图
    //     for (int y = 0; y < MAP_SIZE; y++)
    //     {
    //         pointPrinted = 0;
    //         for (int f = 0; f < FOOD_NUMBER; f++)
    //         {
    //             //打印食物
    //             if (foods[f].coordinate.x == (float)x && foods[f].coordinate.y == (float)y && foods[f].isEaten==0)
    //             {
    //                 printf("+");
    //                 pointPrinted = 1;
    //             }
    //         }
    //         //打印生物体
    //         if (o.coordinate.x == x && o.coordinate.y == y)
    //         {
    //             switch(o.orient.orient_status){
    //                 case ORIENT_UP:
    //                     printf("M");
    //                     pointPrinted = 1;
    //                     break;
    //                 case ORIENT_DOWN:
    //                     printf("V");
    //                     pointPrinted = 1;
    //                     break;
    //                 case ORIENT_LEFT:
    //                     printf("<");
    //                     pointPrinted = 1;
    //                     break;
    //                 case ORIENT_RIGHT:
    //                     printf(">");
    //                     pointPrinted = 1;
    //                     break;
    //                 default: 
    //                     printf("O");
    //                     break;
    //             }
    //         }
    //         if (pointPrinted == 0)
    //         {
    //             printf(" ");
    //         }
    //     }
    //     printf("\n");

    // }
    printf("the organism is at (%.0fx, %.0fy)\n", o.coordinate.x, o.coordinate.y);
    printf("the closest food item is at(%.0f,%.0f)(DOWN,RIGHT)\ndistance:%.0f\n",
                   sensory_array[0],sensory_array[1],(float)(abs)(sensory_array[0])+(float)(abs)(sensory_array[1]));
    printf("The organism has eaten %d food items\n", o.eaten_number);
    printf("Organism's energy surplus is %f", o.surplus);
    printf("\naction_steps is %d\n", action_steps);
}
//操纵行动函数
void Operate(Food *foods, Organism *o,int ch, int action_step){

}
int IsOrganAlive(Organism o){

}
float MaxFloat(float a, float b){
    if(a >= b){return a;}
    else return b;
}
float MinFloat(float a, float b){
    if(a <= b){return a;}
    else return b;
}

float GetFitness(Organism o){
    float fitness;
    /*此下两句fitness公式二选一*/

    //此句可可输出最低为0的fitness
    fitness = MinFloat(o.max_allowable_energy, MaxFloat(0, o.surplus));
    //此句可输出无下限的fitness
    // fitness = MinFloat(o.max_allowable_energy, o.surplus);
    return fitness;
}


//求浮点数的绝对值
//精度为小数点后六位
float AbsFloat(float x){
    float result = 1.0*(abs(x*1000000))/1000000;
    return result;
}

//计算大脑新陈代谢能量消耗
float GetBrainCost(NeuralNetwork nn){
    float brain_cost = 0;
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 3; j++){
            brain_cost += (AbsFloat(nn.b_L1_L2[i][j]) + AbsFloat(nn.w_L1_L2[i][j]));
        }
    }
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 2; j++){
            brain_cost += ((AbsFloat(nn.b_L2_L3[i][j])+AbsFloat(nn.w_L2_L3[i][j]))*nn.conn_matrix_L2_L3[i][j]);
        }
    }
    return brain_cost;
}

//计算学习的能量消耗
float GetLearningCost(){

}

//初始化生物体时顺带生成偏置值，此值在生物体存活期间保持不变
//产生[-1.5,1.5]的随机偏置值
float Generate_Offset()
{
    float offset = randfloat(-1,2) - 0.5;
    return offset;
}
//笛卡尔坐标系偏置值
//output = (ρ cos(π(θ + φ)) , ρ sin(π(θ + φ)))
//s[0]为上下，s[1]为左右
void Cartetian_Offset(float *output_array, float offset){

    float offset_angle = offset;//φ:Phi [-1.5, 1.5]
    float tanx = (1.0*output_array[0])/(1.0*output_array[1]);
    float true_angular = atan(tanx)/DEC;//θ:Theta
    float Rho = sqrt(output_array[0]*output_array[0] + output_array[1]*output_array[1]);//ρ:Rho 
    output_array[0] = Rho*cos(pi*(true_angular + offset));
    output_array[1] = Rho*sin(pi*(true_angular + offset));
}

//保存每代平均适应度
void SaveAverageResult(Genome *Population, int current_generation){
    float average_fitness = 0;
    for(int i = 0; i < GENE_NUMBER; i++){
        average_fitness += Population[i].fitness;
    }
    average_fitness = average_fitness/GENE_NUMBER;
    FILE *fp;
    fp = fopen("AverageResult.txt","a");
    fprintf(fp, "%d\t", current_generation);
    fprintf(fp, "%f\t", average_fitness);
    char ch = '\n';
    fputc(ch, fp);
    fclose(fp);
}

//将结果输出到文件
void SaveEachResult(Genome *Population, int current_generation){
    FILE *fp;
    fp = fopen("result.txt","a");
    
    for(int i = 0; i < GENE_NUMBER; i++){
        fprintf(fp, "%d\t",current_generation);
        fprintf(fp, "%f\t", Population[i].fitness);
        for(int j = 0; j < 98; j++){
            fprintf(fp, "%lf\t", Population[i].gene_info[j]);
        }
        char ch = '\n';
        fputc(ch, fp);
    }
    
    fclose(fp);
}

//保存最好个体
void SaveBestResult(Genome *best_individual, int generation_of_best_individual){
    FILE *fp;
    fp = fopen("bestresult.txt", "w");
    fprintf(fp, "%d\t",generation_of_best_individual);//generation
    fprintf(fp, "%f\t", best_individual->fitness);//fitness
    for (int j = 0; j < 98; j++)
    {
        fprintf(fp, "%lf\t", best_individual->gene_info[j]);
    }
    char ch = '\n';
    fputc(ch, fp);
    
    fclose(fp);
}
//保存每代最好个体
void SaveBestOfEachGeneration(Genome *Population, int current_generation){
    FILE *fp;
    fp = fopen("BestResultOfEachGeneration.txt","a");
    fprintf(fp, "%d\t",current_generation);//generation
    fprintf(fp, "%f\t", Population[0].fitness);//fitness
    for(int i = 0; i < 98; i++){
        fprintf(fp, "%lf\t", Population[0].gene_info[i]);//gene
    }
    char ch = '\n';
    fputc(ch, fp);
    
    fclose(fp);
}
void main(){
    srand(time(NULL));
    Organism o;
    NeuralNetwork nn;
    Food foods[FOOD_NUMBER] = {0};
    PlasticityNetwork pn;
    Genome Population[GENE_NUMBER];
    float sensory_array[2];//s[0]为上下，s[1]为左右
    int previous_action[2] = {0,0};//存储前一步操作，初始化为00
    int current_generation = 0;
    int current_individual = 0;
    int action_steps = 0;//记录行动步数，每50步重新生成食物
    InitPopulation(Population);//初始化种群
    Genome best_individual;
    best_individual.fitness = 0;
    int generation_of_best_individual = 0;
    while(current_generation < MAX_GENERATION){
        current_individual = 0;
        //从第二代开始不初始化，而进行选择交叉变异
        while(current_individual < GENE_NUMBER){
            //对每个生物体初始化状态，大脑，可塑性网络
            action_steps = 0;
            InitOrgan(&o);
            InitFoods(foods,o);
            InitiateNeuralNetwork(&nn,Population[current_individual]);
            InitializePlasticityNet(&pn, Population[current_individual]);
            Cartetian_Sensory(sensory_array, foods, o);
            //PrintMap(foods, o, sensory_array, action_steps, current_generation, current_individual);
            while(action_steps < ORGAN_LIFE_SPAN){
                nn.neuron_L1[0] = previous_action[0];//为大脑的sensory_input赋值
                nn.neuron_L1[1] = previous_action[1];
                nn.neuron_L1[2] = sensory_array[0];
                nn.neuron_L1[3] = sensory_array[1];
                for(int i = 0; i < DELIBRATION_STEP; i++){//Delibration Steps
                    UpdateBrainNetwork(&nn, &pn);
                    UpdateOrganSurplus(&o, LEARNING_COST);
                }
                BrainFeedForward(&nn);
                OrganMove(&o,nn.neuron_L3[0],nn.neuron_L3[1]);
                previous_action[0] = nn.neuron_L3[0];
                previous_action[1] = nn.neuron_L3[1];
                OrganEatFood(foods,&o);
                UpdateOrganSurplus(&o, BRAIN_METABOLIC_COST);
                action_steps++;
                
                //system("cls");
                Cartetian_Sensory(sensory_array, foods, o);
                //应加入offset
                //PrintMap(foods, o, sensory_array, action_steps, current_generation, current_individual);
                //printf("output1=%lf\noutput2=%lf\n", nn.neuron_L3[0], nn.neuron_L3[1]);
                if (action_steps % 50 == 0)
                {
                    InitFoods(foods, o);
                }
                //Sleep(3000);

                

                //action_steps++;
            }
            Population[current_individual].fitness = GetFitness(o);
            //printf("generation %d individual %d finished\n", current_generation, current_individual);
            
            
            current_individual++;
        }
        SaveAverageResult(Population, current_generation);
        //每代全部个体游戏完毕后，执行选择、交叉、变异
        //printf("1\n");
        EvaluatePop(Population);
        //printf("2\n");
        SaveEachResult(Population,current_generation);//保存结果
        //printf("3\n");
        //保存最佳个体基因
        if(best_individual.fitness < Population[0].fitness){
            best_individual = Population[0];
            generation_of_best_individual = current_generation;
            SaveBestResult(&best_individual,generation_of_best_individual);
        }
        //printf("4\n");
        SaveBestOfEachGeneration(Population, current_generation);
        //printf("5\n");
        SelectPopulation(Population);
        //printf("6\n");
        CrossPopulation(Population);
        //printf("7\n");
        MutatePopulation(Population);
        //printf("8\n");
        printf("generation %d finished\n", current_generation);
        //printf("9\n");
            
    
        
        
        
        current_generation++;
        //printf("10\n");
    }


}

//下步工作任务;
// 1.完成可塑性网络的feedforward
//2.完成大脑网络的初始化
// 3.完成大脑网络的各权重更新函数
//4.feedforward不能用+=
//  5.优化offset随机值
// 6.将每代训练结果输出到文件
//7.增加监视函数
// 8.考虑能量消耗
//9.结果写入CSV文件
//10.保存平均适应度