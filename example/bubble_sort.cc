#include<iostream>
using namespace std;
namespace abc = std;

typedef int int32_t;
typedef int32_t nature_num;

void print(int arr[], int n)
{  
    for(int j= 0; j<n; j++)
  {  
    cout<<arr[j] <<"  ";  
  }  
  cout<<endl;  
}  
 
void BubbleSort(int arr[], int n)
{
  for (int i = 0; i < n - 1; i++)
  {
    for (int j = 0; j < n - i - 1; j++)
      {
        if (arr[j] > arr[j + 1]) 
          {
            int temp = arr[j];
            arr[j] = arr[j + 1];
            arr[j + 1] = temp;
          }
      }
  }
}
 
int main()
{  
  nature_num s[10] = {8,1,9,7,2,4,5,6,10,3};  
  abc::cout<<"初始序列：";  
  print(s,10);  
  BubbleSort(s,10);  
  abc::cout<<"排序结果：";  
  print(s,10);  
}
