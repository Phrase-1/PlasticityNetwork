#include "PlasticyNetwork.h"
#include"NeuralNetwork.c"
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>
#include<conio.h>
#define MAP_SIZE 50
#define FOOD_NUMBER 50
#define ORIENT_UP 0
#define ORIENT_RIGHT 90
#define ORIENT_DOWN 180
#define ORIENT_LEFT 270
#define PI 3.1415926
#define ORGAN_LIFE_SPAN 200
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
    float max_storage_energy;
    float brain_metabolism;
    float move_cost;
    float current_energy;
}Organism;
typedef struct Food
{
    Point coordinate;
    //被吃为1否则为0
    int isEaten;
}Food;
//初始化与重置食物
//坐标范围为<0-49>
//貌似有重复生成的BUG
void InitFoods(Food *foods, Organism o){
    for(int i = 0; i < FOOD_NUMBER; i ++){
        foods[i].coordinate.x=foods[i].coordinate.y=0;
        foods[i].isEaten = 0;
    }
    srand(time(NULL));
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
    o->coordinate.x = 25;
    o->coordinate.y = 25;
    o->orient.orient_status = ORIENT_UP;
    o->eaten_number = 0;
    o->current_energy = 10;
    o->max_storage_energy = 0;
}
//生物进食
void OrganEatFood(Food *foods, Organism *o){
    for(int i = 0; i < FOOD_NUMBER; i++){
        //若生物与食物坐标相同，则吃掉该食物
        if(foods[i].coordinate.x==o->coordinate.x && foods[i].coordinate.y==o->coordinate.y && foods[i].isEaten==0){
            foods[i].isEaten = 1;
            o->eaten_number++;
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
        if(o->orient.orient_status==ORIENT_LEFT){
            o->orient.orient_status = ORIENT_UP;
        }
        else{
            o->orient.orient_status+=90;
        }
    }
    //左转
    if(command_code1==1 && command_code2==0){
        if(o->orient.orient_status==ORIENT_UP){
            o->orient.orient_status = ORIENT_LEFT;
        }
        else{
            o->orient.orient_status-=90;
        }
    }
    //前进
    if(command_code1==1 && command_code2==1){
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
void PrintMap(Food foods[], Organism o, float sensory_array[], int action_steps)
{
    int pointPrinted = 0;
    // x为行y为列
    for (int x = 0; x < MAP_SIZE; x++)
    {
        printf("%4drow ", x);
        //打印整个地图
        for (int y = 0; y < MAP_SIZE; y++)
        {
            pointPrinted = 0;
            for (int f = 0; f < FOOD_NUMBER; f++)
            {
                //打印食物
                if (foods[f].coordinate.x == (float)x && foods[f].coordinate.y == (float)y && foods[f].isEaten==0)
                {
                    printf("+");
                    pointPrinted = 1;
                }
            }
            //打印生物体
            if (o.coordinate.x == x && o.coordinate.y == y)
            {
                switch(o.orient.orient_status){
                    case ORIENT_UP:
                        printf("M");
                        pointPrinted = 1;
                        break;
                    case ORIENT_DOWN:
                        printf("V");
                        pointPrinted = 1;
                        break;
                    case ORIENT_LEFT:
                        printf("<");
                        pointPrinted = 1;
                        break;
                    case ORIENT_RIGHT:
                        printf(">");
                        pointPrinted = 1;
                        break;
                    default: 
                        printf("O");
                        break;
                }
            }
            if (pointPrinted == 0)
            {
                printf(" ");
            }
        }
        printf("\n");

    }
    printf("the closest food item is at(%.0f,%.0f)(RIGHT,DOWN)\ndistance:%.0f\n",
                   sensory_array[1],sensory_array[0],(float)(abs)(sensory_array[0])+(float)(abs)(sensory_array[1]));
    printf("The organism has eaten %d food items\n", o.eaten_number);
    printf("\naction_steps is %d\n", action_steps);
}
//操纵行动函数
void Operate(){

}
int IsOrganAlive(Organism o){

}
//初始化生物体时顺带生成偏置值，此值在生物体存活期间保持不变
float Generate_Offset()
{
    srand(time(NULL));
    float offset = 1.0 * rand() / RAND_MAX * (3);
    offset -= 1.5;
    return offset;
}
//笛卡尔坐标系偏置值
//output = (ρ cos(π(θ + φ)) , ρ sin(π(θ + φ)))
void Cartetian_Offset(float *output_array, float offset){

    float offset_angle = offset;//φ:Phi [-1.5, 1.5]
    float true_angular;//θ:Theta
    float Rho;//ρ:Rho 

}
void main(){
    Organism o;
    InitOrgan(&o);
    NeuralNetwork nn;
    InitiateNeuralNetwork(&nn);
    Food foods[FOOD_NUMBER] = {0};
    InitFoods(foods, o);
    float offset = Generate_Offset();
    int action_steps = 0;//记录行动步数，每50步重新生成食物
    int ch;//接收键盘输入
    float sensory_array[2];//s[0]为上下，s[1]为左右
    Cartetian_Sensory(sensory_array, foods, o);
    Cartetian_Offset(sensory_array, offset);
    PrintMap(foods, o, sensory_array, action_steps);
    if()
    while ((ch = getch()) != 0x1B && action_steps<ORGAN_LIFE_SPAN) /* Press ESC to quit... */
    {
        switch (ch = getch())
        {
        case 72:
            printf("UP\n");
            OrganMove(&o, 1, 1);
            OrganEatFood(foods,&o);
            break;
        case 80:
            printf("DOWN\n");
            break;
        case 75:
            printf("LEFT\n");
            OrganMove(&o, 1, 0);
            break;
        case 77:
            printf("RIGHT\n");
            OrganMove(&o, 0, 1);
            break;
        default:
            break;   
        }
        action_steps++;
        system("cls");
        Cartetian_Sensory(sensory_array, foods, o);
        Cartetian_Offset(sensory_array,offset);
        PrintMap(foods, o, sensory_array, action_steps);
        if(action_steps % 50 == 0){
            InitFoods(foods,o);
        }

    }
}