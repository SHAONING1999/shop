#include "cJSON_Process.h"
#include "freertos.h"
#include "main.h"
#include "stdio.h"

/*******************************************************************
 *                          变量声明                               
 *******************************************************************/



cJSON* cJSON_Data_Init(void)
{
  cJSON* cJSON_Root = NULL;    //json根节点
  
  cJSON_Root = cJSON_CreateObject();   /*创建项目*/
  if(NULL == cJSON_Root)
  {
      return NULL;
  }
  cJSON_AddStringToObject(cJSON_Root, "name", "Water_analyzer");  /*添加元素  键值对*/
  cJSON_AddNumberToObject(cJSON_Root, "temp", 0);
  cJSON_AddNumberToObject(cJSON_Root, "humi", 0);
  
  char* p = cJSON_Print(cJSON_Root);  /*p 指向的字符串是json格式的*/
  
//  PRINT_DEBUG("%s\n",p);
  
  vPortFree(p);
  p = NULL;
  
  return cJSON_Root;
  
}
uint8_t cJSON_Update(const cJSON * const object,const char * const string,void *d)
{
  cJSON* node = NULL;    //json根节点
  node = cJSON_GetObjectItem(object,string);
  if(node == NULL)
    return NULL;
  if(cJSON_IsBool(node))
  {
    int *b = (int*)d;
//    printf ("d = %d",*b);
    cJSON_GetObjectItem(object,string)->type = *b ? cJSON_True : cJSON_False;
//    char* p = cJSON_Print(object);    /*p 指向的字符串是json格式的*/
    return 1;
  }
  else if(cJSON_IsString(node))
  {
    cJSON_GetObjectItem(object,string)->valuestring = (char*)d;
//    char* p = cJSON_Print(object);    /*p 指向的字符串是json格式的*/
    return 1;
  }
  else if(cJSON_IsNumber(node))
  {
    double *num = (double*)d;
//    printf ("num = %f",*num);
//    cJSON_GetObjectItem(object,string)->valueint = (double)*num;
    cJSON_GetObjectItem(object,string)->valuedouble = (double)*num;
//    char* p = cJSON_Print(object);    /*p 指向的字符串是json格式的*/
    return 1;
  }
  else
    return 1;
}

void Proscess(void* data)
{
  printf("开始解析JSON数据");
  cJSON *root,*json_name,*json_co_num,*json_co2_num,*json_ch4_num,*json_o2_num;
  root = cJSON_Parse((char*)data); //解析成json形式

  json_name = cJSON_GetObjectItem( root , "name");  //获取键值内容
  json_co_num = cJSON_GetObjectItem( root , "CO" );
  json_co2_num = cJSON_GetObjectItem( root , "CO2" );
  json_ch4_num = cJSON_GetObjectItem( root , "CH4" );
  json_o2_num = cJSON_GetObjectItem( root , "O2" );
  printf("name:%s\n co_num:%f\n co2_num:%f\n ch4_num:%f\n o2_num:%f\n",
              json_name->valuestring,
              json_co_num->valuedouble,
              json_co2_num->valuedouble,
							json_ch4_num->valuedouble,
              json_o2_num->valuedouble);

  cJSON_Delete(root);  //释放内存 
}








